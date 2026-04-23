#!/bin/bash

API_URL="${API_URL:-http://localhost:4000}"

echo "=========================================="
echo "  GraphQL Bookstore - Functional Test    "
echo "=========================================="
echo ""

PASS=0
FAIL=0

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

pass() { echo -e "   ${GREEN}✓ PASS${NC}: $1"; ((PASS++)) || true; }
fail() { echo -e "   ${RED}✗ FAIL${NC}: $1"; ((FAIL++)) || true; }

echo "Checking server..."
HEALTH=$(curl -s -o /dev/null -w "%{http_code}" "$API_URL/" || echo "000")
if [ "$HEALTH" = "000" ]; then
    echo "Server not running at $API_URL"
    exit 1
fi
echo "Server OK (HTTP $HEALTH)"
echo ""

echo "=== 1. Public: Browse Books ==="
RESPONSE=$(curl -s -X POST "$API_URL/graphql" -H 'Content-Type: application/json' \
    -d '{"query":"{ books { id title author { firstName lastName } price } }"}')
BOOK_ID=$(echo "$RESPONSE" | grep -oP '"id":\s*[0-9]+' | head -1 | grep -oP '[0-9]+')
BOOK_TITLE=$(echo "$RESPONSE" | grep -oP '"title":"[^"]+' | head -1 | cut -d'"' -f4)
[ -n "$BOOK_ID" ] && pass "List books (ID: $BOOK_ID)" || fail "List books"

echo ""
echo "=== 2. Public: Book Details ==="
RESPONSE=$(curl -s -X POST "$API_URL/graphql" -H 'Content-Type: application/json' \
    -d "{\"query\":\"{ book(id: $BOOK_ID) { id title price } }\"}")
echo "$RESPONSE" | grep -q '"title"' && pass "Book details" || fail "Book details"

echo ""
echo "=== 3. Public: Search ==="
RESPONSE=$(curl -s -X POST "$API_URL/graphql" -H 'Content-Type: application/json' \
    -d '{"query":"{ books(search: \"Code\") { id title } }"}')
echo "$RESPONSE" | grep -q '"title"' && pass "Search books" || fail "Search books"

echo ""
echo "=== 4. Auth: Login ==="
RESPONSE=$(curl -s -X POST "$API_URL/graphql" -H 'Content-Type: application/json' \
    -d '{"query":"mutation { login(username: \"user\", password: \"password123\") { success token user { id username role } } }"}')
TOKEN=$(echo "$RESPONSE" | grep -oP '"token":"[^"]+' | cut -d'"' -f4)
[ -n "$TOKEN" ] && pass "Login (token received)" || fail "Login"

if [ -z "$TOKEN" ]; then
    echo "Can't continue without token"
    exit 1
fi

echo ""
echo "=== 5. Auth: Me ==="
RESPONSE=$(curl -s -X POST "$API_URL/graphql" -H 'Content-Type: application/json' \
    -H "Authorization: Bearer $TOKEN" \
    -d '{"query":"{ me { id username role } }"}')
echo "$RESPONSE" | grep -q '"username"' && pass "Get current user" || fail "Get current user"

echo ""
echo "=== 6. Cart: Add to Cart ==="
RESPONSE=$(curl -s -X POST "$API_URL/graphql" -H 'Content-Type: application/json' \
    -H "Authorization: Bearer $TOKEN" \
    -d "{\"query\":\"mutation { addToCart(bookId: $BOOK_ID, quantity: 1) { success message } }\"}")
echo "$RESPONSE" | grep -q '"success":true' && pass "Add to cart" || fail "Add to cart"

echo ""
echo "=== 7. Cart: View Cart ==="
RESPONSE=$(curl -s -X POST "$API_URL/graphql" -H 'Content-Type: application/json' \
    -H "Authorization: Bearer $TOKEN" \
    -d '{"query":"{ cart { id items { book { title } quantity } } }"}')
echo "$RESPONSE" | grep -q '"cart"' && pass "View cart" || fail "View cart"

echo ""
echo "=== 8. Cart: Apply Coupon ==="
RESPONSE=$(curl -s -X POST "$API_URL/graphql" -H 'Content-Type: application/json' \
    -H "Authorization: Bearer $TOKEN" \
    -d '{"query":"mutation { applyCoupon(code: \"SUMMER25\") { success discountAmount } }"}')
echo "$RESPONSE" | grep -q '"success":true' && pass "Apply coupon" || fail "Apply coupon"

echo ""
echo "=== 9. Checkout: Pay ==="
RESPONSE=$(curl -s -X POST "$API_URL/graphql" -H 'Content-Type: application/json' \
    -H "Authorization: Bearer $TOKEN" \
    -d '{"query":"mutation { checkout(cardNumber: \"4111111111111111\", expiry: \"12/25\", cvv: \"123\") { success orderId totalAmount } }"}')
echo "$RESPONSE" | grep -q '"success":true' && pass "Checkout with payment" || fail "Checkout"

echo ""
echo "=== 10. Orders: View ==="
RESPONSE=$(curl -s -X POST "$API_URL/graphql" -H 'Content-Type: application/json' \
    -H "Authorization: Bearer $TOKEN" \
    -d '{"query":"{ orders { id orderNumber status } }"}')
ORDER_NUM=$(echo "$RESPONSE" | grep -o '"orderNumber"' | wc -l)
[ "$ORDER_NUM" -gt 0 ] && pass "View orders" || fail "View orders"

echo ""
echo "=== 11. Reviews: Create ==="
RESPONSE=$(curl -s -X POST "$API_URL/graphql" -H 'Content-Type: application/json' \
    -H "Authorization: Bearer $TOKEN" \
    -d "{\"query\":\"mutation { createReview(bookId: $BOOK_ID, rating: 5, comment: \\\"Great book!\\\") { success } }\"}")
echo "$RESPONSE" | grep -q '"success":true' && pass "Create review" || fail "Create review"

echo ""
echo "=== 12. Reviews: My Reviews ==="
RESPONSE=$(curl -s -X POST "$API_URL/graphql" -H 'Content-Type: application/json' \
    -H "Authorization: Bearer $TOKEN" \
    -d '{"query":"{ myReviews { id rating } }"}')
echo "$RESPONSE" | grep -q '"myReviews"' && pass "View my reviews" || fail "View my reviews"

echo ""
echo "=========================================="
echo "  Results: $PASS passed, $FAIL failed"
echo "=========================================="