#!/bin/bash
# Stress/Flaw Test Script for GraphQL Bookstore API
# Usage: ./stress_test.sh [API_URL]
# Default: http://localhost:4000/graphql
# Production: ./stress_test.sh https://api.graphqlbook.org/graphql

API_URL="${1:-http://localhost:4000/graphql}"
echo "=== STRESS/FLAW TEST ==="
echo "API: $API_URL"
echo ""

get_token() {
    curl -s -X POST "$API_URL" -H 'Content-Type: application/json' \
        -d '{"query":"mutation { login(username: \"admin\", password: \"password123\") { token } }"}' | \
        python3 -c "import sys,json; print(json.load(sys.stdin)['data']['login']['token'])"
}

# Test Public Endpoints
echo "=== PUBLIC QUERIES ==="
echo "1. Books:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' \
    -d '{"query":"{ books { id title } }"}' | head -c 100

echo -e "\n\n2. Book by ID:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' \
    -d '{"query":"{ book(id: 1) { id title } }"}'

echo -e "\n\n3. Book Reviews:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' \
    -d '{"query":"{ bookReviews(bookId: 1) { id } }"}'

echo -e "\n\n4. Introspection:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' \
    -d '{"query":"{ __schema { queryType { name } } }"}'

# Get token for authenticated tests
TOKEN=$(get_token)
echo -e "\n\n=== AUTHENTICATED QUERIES ==="
echo "Token: ${TOKEN:0:30}..."

echo -e "\n5. Me:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' -H "Authorization: Bearer $TOKEN" \
    -d '{"query":"{ me { username role } }"}'

echo -e "\n\n6. Cart:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' -H "Authorization: Bearer $TOKEN" \
    -d '{"query":"{ cart { id total } }"}'

echo -e "\n\n7. Orders:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' -H "Authorization: Bearer $TOKEN" \
    -d '{"query":"{ orders { id } }"}'

echo -e "\n\n=== MUTATIONS ==="
echo "8. Add to Cart:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' -H "Authorization: Bearer $TOKEN" \
    -d '{"query":"mutation { addToCart(bookId: 1, quantity: 1) { success } }"}'

echo -e "\n\n9. Apply Coupon:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' -H "Authorization: Bearer $TOKEN" \
    -d '{"query":"mutation { applyCoupon(code: \"SUMMER25\") { success } }"}'

echo -e "\n\n10. Checkout:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' -H "Authorization: Bearer $TOKEN" \
    -d '{"query":"mutation { checkout(cardNumber: \"8763259044315935\", expiry: \"05/27\", cvv: \"034\") { success orderId } }"}'

echo -e "\n\n11. Update Profile:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' -H "Authorization: Bearer $TOKEN" \
    -d '{"query":"mutation { updateProfile(firstName: \"Test\") { username role } }"}'

echo -e "\n\n12. Create Review:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' -H "Authorization: Bearer $TOKEN" \
    -d '{"query":"mutation { createReview(bookId: 1, rating: 5, comment: \"Test\") { success } }"}'

echo -e "\n\n13. Register Webhook:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' -H "Authorization: Bearer $TOKEN" \
    -d '{"query":"mutation { registerWebhook(url: \"http://example.com/wh\", events: [\"order.created\"]) { success webhook { id } }"}'

echo -e "\n\n14. Logout:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' -H "Authorization: Bearer $TOKEN" \
    -d '{"query":"mutation { logout { success } }"}'

# Hidden/Pro endpoints
echo -e "\n\n=== HIDDEN ENDPOINTS ==="
echo "15. processXMLData:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' \
    -d '{"query":"query { processXMLData(xml: \"<test>hi</test>\") }"}'

echo -e "\n\n16. decodeJWT:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' \
    -d '{"query":"query { decodeJWT(token: \"eyJhbGciOiJIUzI1NiJ9.test\") { valid algorithm } }"}'

echo -e "\n\n17. manageCache:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' \
    -d '{"query":"query { manageCache(key: \"k\", value: \"v\") { success } }"}'

echo -e "\n\n18. handleRecursiveQuery:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' \
    -d '{"query":"query { handleRecursiveQuery(depth: 3) { result } }"}'

echo -e "\n\n19. _fetchExternalResource:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' \
    -d '{"query":"query { _fetchExternalResource(url: \"http://example.com\") }"}' | head -c 50

echo -e "\n\n20. _searchAdvanced:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' \
    -d '{"query":"query { _searchAdvanced(query: \"code\") { id title } }"}'

echo -e "\n\n21. _adminStats:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' \
    -d '{"query":"query { _adminStats { userCount bookCount } }"}'

echo -e "\n\n22. _adminAllOrders:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' \
    -d '{"query":"query { _adminAllOrders { id } }"}'

echo -e "\n\n23. _batchQuery:"
curl -s -X POST "$API_URL" -H 'Content-Type: application/json' \
    -d '{"query":"query { _batchQuery(queries: [\"{ books { id } }\\\"]) }"}'

# Stress test
echo -e "\n\n=== STRESS TEST ==="
echo "24. 20 Concurrent requests:"
START=$(date +%s)
for i in {1..20}; do curl -s -X POST "$API_URL" -H 'Content-Type: application/json' \
    -d '{"query":"{ books { id } }"}' >/dev/null & done
wait
END=$(date +%s)
echo "   Completed in $((END-START)) seconds"

echo -e "\n=== TEST COMPLETE ==="