#!/bin/bash

API_URL="${API_URL:-https://api.graphqlbook.org/graphql}"
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "=========================================="
echo "  Functional Test Suite"
echo "  URL: $API_URL"
echo "=========================================="

pass() { echo -e "${GREEN}✓ PASS${NC}: $1"; }
fail() { echo -e "${RED}✗ FAIL${NC}: $1"; }
info() { echo -e "${YELLOW}→${NC} $1"; }

TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

test_query() {
    local query="$1"
    local token="${2:-}"
    
    printf '{"query":"%s"}' "$query" > "$TMPDIR/q.json"
    
    if [ -n "$token" ]; then
        curl -s -X POST "$API_URL" \
            -H 'Content-Type: application/json' \
            -H "Authorization: Bearer $token" \
            --data-binary @"$TMPDIR/q.json"
    else
        curl -s -X POST "$API_URL" \
            -H 'Content-Type: application/json' \
            --data-binary @"$TMPDIR/q.json"
    fi
}

echo ""
info "TEST 1: Health Check"
health=$(curl -s -o /dev/null -w "%{http_code}" "$API_URL/../health")
if [ "$health" = "200" ]; then
    pass "Server is healthy"
else
    fail "Server not responding ($health)"
fi

echo ""
info "TEST 2: User Login"
login_resp=$(test_query 'mutation { login(username: \"user\", password: \"password123\") { token } }')
TOKEN=$(echo "$login_resp" | grep -oP '"token":"[^"]+' | cut -d'"' -f4)
if [ -n "$TOKEN" ]; then
    pass "Login successful"
else
    fail "Login failed"
    exit 1
fi

echo ""
info "TEST 3: Get Current User (me)"
me_resp=$(test_query 'query { me { username role } }' "$TOKEN")
if echo "$me_resp" | grep -q '"username":"user"'; then
    pass "me query works"
else
    fail "me query failed"
fi

echo ""
info "TEST 4: List Books"
books_resp=$(test_query 'query { books { id title price } }')
if echo "$books_resp" | grep -q "Clean Code"; then
    pass "Books query works"
else
    fail "Books query failed"
fi

echo ""
info "TEST 5: Get Single Book"
book_resp=$(test_query 'query { book(id: 1) { id title price } }')
if echo "$book_resp" | grep -q '"title":"Clean Code"'; then
    pass "Book query works"
else
    fail "Book query failed"
fi

echo ""
info "TEST 6: Add to Cart"
add_resp=$(test_query 'mutation { addToCart(bookId: 1, quantity: 1) { success } }' "$TOKEN")
if echo "$add_resp" | grep -q '"success":true'; then
    pass "Add to cart works"
else
    fail "Add to cart failed"
fi

echo ""
info "TEST 7: View Cart"
cart_resp=$(test_query 'query { cart { items { bookId title quantity } } }' "$TOKEN")
if echo "$cart_resp" | grep -q "Clean Code"; then
    pass "Cart query works"
else
    fail "Cart query failed"
fi

echo ""
info "TEST 8: Remove from Cart"
remove_resp=$(test_query 'mutation { removeFromCart(bookId: 1) { success } }' "$TOKEN")
if echo "$remove_resp" | grep -q '"success":true'; then
    pass "Remove from cart works"
else
    fail "Remove from cart failed"
fi

echo ""
info "TEST 9: Add Back to Cart"
add_resp2=$(test_query 'mutation { addToCart(bookId: 1, quantity: 1) { success } }' "$TOKEN")
if echo "$add_resp2" | grep -q '"success":true'; then
    pass "Add to cart works"
else
    fail "Add to cart failed"
fi

echo ""
info "TEST 10: Checkout"
checkout_resp=$(test_query 'mutation { checkout(cardNumber: \"4111111111111111\", expiry: \"12/25\", cvv: \"123\") { success orderId totalAmount } }' "$TOKEN")
if echo "$checkout_resp" | grep -q '"success":true'; then
    pass "Checkout works"
else
    fail "Checkout failed"
fi

echo ""
info "TEST 11: View Orders"
orders_resp=$(test_query 'query { orders { id status totalAmount } }' "$TOKEN")
if echo "$orders_resp" | grep -q "totalAmount"; then
    pass "Orders query works"
else
    fail "Orders query failed"
fi

echo ""
info "TEST 12: Cancel Order"
order_id=$(echo "$orders_resp" | grep -oP '"id":"[^"]+' | head -1 | cut -d'"' -f4)
if [ -n "$order_id" ]; then
    cancel_resp=$(test_query "mutation { cancelOrder(orderId: \"$order_id\") { success } }" "$TOKEN")
    if echo "$cancel_resp" | grep -q '"success":true'; then
        pass "Cancel order works"
    else
        fail "Cancel order failed"
    fi
else
    info "No orders to cancel"
fi

echo ""
info "TEST 13: Create Review"
review_resp=$(test_query 'mutation { createReview(bookId: 1, rating: 5, comment: \"Great book!\") { success } }' "$TOKEN")
if echo "$review_resp" | grep -q '"success":true'; then
    pass "Create review works"
else
    fail "Create review failed"
fi

echo ""
info "TEST 14: View Book Reviews"
book_reviews_resp=$(test_query 'query { bookReviews(bookId: 1) { rating comment } }')
if echo "$book_reviews_resp" | grep -q "rating"; then
    pass "Book reviews query works"
else
    fail "Book reviews query failed"
fi

echo ""
info "TEST 15: View My Reviews"
my_reviews_resp=$(test_query 'query { myReviews { id rating comment } }' "$TOKEN")
if echo "$my_reviews_resp" | grep -q "rating"; then
    pass "My reviews query works"
else
    fail "My reviews query failed"
fi

echo ""
info "TEST 16: Delete Review"
review_id=$(echo "$my_reviews_resp" | grep -oP '"id":[0-9]+' | head -1 | grep -oP '[0-9]+')
if [ -n "$review_id" ]; then
    delete_resp=$(test_query "mutation { deleteReview(reviewId: $review_id) { success } }" "$TOKEN")
    if echo "$delete_resp" | grep -q '"success":true'; then
        pass "Delete review works"
    else
        fail "Delete review failed"
    fi
else
    info "No reviews to delete"
fi

echo ""
info "TEST 17: Register Webhook"
webhook_resp=$(test_query 'mutation { registerWebhook(url: \"https://example.com/webhook\", events: [\"order.created\"]) { success } }' "$TOKEN")
if echo "$webhook_resp" | grep -q '"success":true'; then
    pass "Register webhook works"
else
    fail "Register webhook failed"
fi

echo ""
info "TEST 18: View Webhooks"
webhooks_resp=$(test_query 'query { webhooks { id url } }' "$TOKEN")
if echo "$webhooks_resp" | grep -q "url"; then
    pass "Webhooks query works"
else
    fail "Webhooks query failed"
fi

echo ""
info "TEST 19: Update Profile"
update_resp=$(test_query 'mutation { updateProfile(phone: \"1234567890\") { success } }' "$TOKEN")
if echo "$update_resp" | grep -q '"success":true'; then
    pass "Update profile works"
else
    fail "Update profile failed"
fi

echo ""
info "TEST 20: Logout"
logout_resp=$(test_query 'mutation { logout }' "$TOKEN")
if echo "$logout_resp" | grep -q 'true'; then
    pass "Logout works"
else
    fail "Logout failed"
fi

echo ""
info "TEST 21: Introspection"
introsp_resp=$(test_query '{ __schema { queryType { name } } }')
if echo "$introsp_resp" | grep -q '"queryType"'; then
    pass "Introspection works"
else
    fail "Introspection failed"
fi

echo ""
info "TEST 22: GraphQL POST Health"
post_health=$(curl -s -X POST "$API_URL" -H 'Content-Type: application/json' -d '{"query":"{ __typename }"}' | grep -q "__typename" && echo "OK")
if [ "$post_health" = "OK" ]; then
    pass "GraphQL POST works"
else
    fail "GraphQL POST failed"
fi

echo ""
echo "=========================================="
echo "  Test Suite Complete"
echo "=========================================="