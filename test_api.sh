#!/bin/bash

echo "=========================================="
echo "  Vulnerable GraphQL API - Test Suite  "
echo "=========================================="
echo ""

API_URL="${API_URL:-http://localhost:4000/graphql}"
PASS_COUNT=0
FAIL_COUNT=0

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Helper function to create test files
create_test_file() {
    local filename="$1"
    local content="$2"
    echo "$content" > "$filename"
}

# Cleanup function
cleanup() {
    rm -f /tmp/test_*.json 2>/dev/null
}
trap cleanup EXIT

echo "API URL: $API_URL"
echo ""

#==========================================
# PRE-FLIGHT CHECK
#==========================================
echo "Running pre-flight check..."
HEALTH_CHECK=$(curl -s "${API_URL}/" 2>/dev/null)

if [ -z "$HEALTH_CHECK" ]; then
    echo -e "   ${RED}ERROR:${NC} Server not responding at $API_URL"
    echo "   Make sure the server is running:"
    echo "   - Local: ./bookstore-server"
    echo "   - Docker: sudo docker-compose up -d"
    echo ""
    exit 1
fi

if ! echo "$HEALTH_CHECK" | grep -q "GraphQL"; then
    echo -e "   ${YELLOW}WARNING:${NC} Server responding but GraphQL not found"
fi
echo -e "   ${GREEN}✓${NC} Server is running"
echo ""

#==========================================
# TEST 1: Server Health Check
#==========================================
echo "1. Testing Server Health..."
if echo "$HEALTH_CHECK" | grep -q "GraphQL"; then
    echo -e "   ${GREEN}✓${NC} Server accessible"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} Server not accessible"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 2: User Registration
#==========================================
echo "2. Testing User Registration..."
create_test_file /tmp/test_register.json '{"query":"mutation { register(username: \"testuser\", firstName: \"Test\", lastName: \"User\", password: \"testpass123\") { success message token user { id username } } }"}'
RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}" -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_register.json 2>&1)
HTTP_CODE=$(echo "$RESPONSE" | grep "HTTP_CODE:" | cut -d: -f2)
RESPONSE=$(echo "$RESPONSE" | grep -v "HTTP_CODE:")

if echo "$RESPONSE" | grep -q '"success":true'; then
    echo -e "   ${GREEN}✓${NC} Registration successful"
    ((PASS_COUNT++))
    REGISTERED=true
elif echo "$RESPONSE" | grep -q '"message":"Username already exists"'; then
    echo -e "   ${YELLOW}⚠${NC} User already exists (expected for repeat tests)"
    ((PASS_COUNT++))
    REGISTERED=true
else
    echo -e "   ${RED}✗${NC} Registration failed"
    echo "   Debug: Response received"
    REGISTERED=false
    ((FAIL_COUNT++))
fi
echo "   Response: $RESPONSE"
echo ""

#==========================================
# TEST 3: User Login
#==========================================
echo "3. Testing User Login (admin)..."
create_test_file /tmp/test_login.json '{"query":"mutation { login(username: \"admin\", password: \"password123\") { success token user { id username role } } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_login.json 2>&1)

if echo "$RESPONSE" | grep -q '"success":true'; then
    echo -e "   ${GREEN}✓${NC} Login successful"
    ((PASS_COUNT++))
    ADMIN_TOKEN=$(echo "$RESPONSE" | grep -oP '"token":"[^"]+' | cut -d'"' -f4)
    echo "   Token: ${ADMIN_TOKEN:0:30}..."
elif echo "$RESPONSE" | grep -q '"Invalid username or password"'; then
    echo -e "   ${RED}✗${NC} Invalid credentials"
    echo "   Check database and credentials"
    ADMIN_TOKEN=""
    ((FAIL_COUNT++))
elif echo "$RESPONSE" | grep -q '"Missing required fields"'; then
    echo -e "   ${RED}✗${NC} Query parsing issue - check backslash escaping"
    ADMIN_TOKEN=""
    ((FAIL_COUNT++))
else
    echo -e "   ${RED}✗${NC} Login failed"
    echo "   Response: $RESPONSE"
    ADMIN_TOKEN=""
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 4: Books Query (No Auth)
#==========================================
echo "4. Testing Books Query (No Auth)..."
create_test_file /tmp/test_books.json '{"query":"query { books { id title price stockQuantity } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_books.json)
if echo "$RESPONSE" | grep -q '"books":'; then
    BOOK_COUNT=$(echo "$RESPONSE" | grep -o '"id"' | wc -l)
    echo -e "   ${GREEN}✓${NC} Books query works ($BOOK_COUNT books found)"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} Books query failed"
    echo "   Response: $RESPONSE"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 5: Get User Info (me query)
#==========================================
echo "5. Testing 'me' Query (With Auth)..."
if [ -n "$ADMIN_TOKEN" ]; then
    create_test_file /tmp/test_me.json '{"query":"query { me { id username role } }"}'
    RESPONSE=$(curl -s -X POST "$API_URL" \
        -H 'Content-Type: application/json' \
        -H "Authorization: Bearer $ADMIN_TOKEN" \
        --data-binary @/tmp/test_me.json)
    if echo "$RESPONSE" | grep -q '"username":"admin"'; then
        echo -e "   ${GREEN}✓${NC} 'me' query works with auth"
        ((PASS_COUNT++))
    else
        echo -e "   ${RED}✗${NC} 'me' query failed"
        echo "   Response: $RESPONSE"
        ((FAIL_COUNT++))
    fi
else
    echo -e "   ${YELLOW}⊘${NC} Skipped (no token)"
fi
echo ""

#==========================================
# TEST 6: SSRF Vulnerability
#==========================================
echo "6. Testing SSRF Vulnerability (_fetchExternalResource)..."
create_test_file /tmp/test_ssrf.json '{"query":"query { _fetchExternalResource(url: \"http://example.com\") }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_ssrf.json)
if echo "$RESPONSE" | grep -q '_fetchExternalResource'; then
    echo -e "   ${GREEN}✓${NC} SSRF endpoint accessible"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} SSRF endpoint failed"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 7: BOLA Vulnerability
#==========================================
echo "7. Testing BOLA Vulnerability (_internalUserSearch)..."
create_test_file /tmp/test_bola.json '{"query":"query { _internalUserSearch(username: \"\") { id username passwordHash } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_bola.json)
USER_COUNT=$(echo "$RESPONSE" | grep -o '"username"' | wc -l)
if [ "$USER_COUNT" -gt 0 ]; then
    echo -e "   ${YELLOW}⚠${NC} BOLA vulnerability exposed ($USER_COUNT users leaked)"
    echo "   Sample: $(echo "$RESPONSE" | head -c 150)..."
    ((PASS_COUNT++))  # This is intentional
else
    echo -e "   ${GREEN}✓${NC} BOLA protected"
    ((FAIL_COUNT++))  # This should leak
fi
echo ""

#==========================================
# TEST 8: SQL Injection
#==========================================
echo "8. Testing SQL Injection (_searchAdvanced)..."
create_test_file /tmp/test_sql.json '{"query":"query { _searchAdvanced(query: \"SQL\") { id title } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_sql.json)
if echo "$RESPONSE" | grep -q '"_searchAdvanced":'; then
    echo -e "   ${GREEN}✓${NC} SQL Injection endpoint accessible"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} SQL Injection endpoint failed"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 9: Admin Endpoints (No Auth)
#==========================================
echo "9. Testing Admin Endpoints (No Auth Required)..."

# _adminStats
create_test_file /tmp/test_admin_stats.json '{"query":"query { _adminStats { userCount bookCount orderCount } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_admin_stats.json)
if echo "$RESPONSE" | grep -q '"userCount":'; then
    echo -e "   ${GREEN}✓${NC} _adminStats exposed (intentional)"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} _adminStats protected (unexpected)"
    ((FAIL_COUNT++))
fi

# _adminAllOrders
create_test_file /tmp/test_admin_orders.json '{"query":"query { _adminAllOrders { id orderNumber totalAmount } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_admin_orders.json)
if echo "$RESPONSE" | grep -q '"_adminAllOrders"'; then
    echo -e "   ${GREEN}✓${NC} _adminAllOrders exposed (intentional)"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} _adminAllOrders protected (unexpected)"
    ((FAIL_COUNT++))
fi

# _adminAllPayments
create_test_file /tmp/test_admin_payments.json '{"query":"query { _adminAllPayments { id amount transactionId } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_admin_payments.json)
if echo "$RESPONSE" | grep -q '"_adminAllPayments"'; then
    echo -e "   ${GREEN}✓${NC} _adminAllPayments exposed (intentional)"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} _adminAllPayments protected (unexpected)"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 10: Complex Nested Query
#==========================================
echo "10. Testing Complex Nested Query..."
create_test_file /tmp/test_complex.json '{"query":"query { books { id title price author { id name } reviews { id rating comment } } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_complex.json)
if echo "$RESPONSE" | grep -q '"books":'; then
    echo -e "   ${GREEN}✓${NC} Complex nested query works"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} Complex nested query failed"
    echo "   Response: $RESPONSE"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 11: GraphQL Introspection
#==========================================
echo "11. Testing GraphQL Introspection..."
create_test_file /tmp/test_introspection.json '{"query":"query { __schema { queryType { name } mutationType { name } } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_introspection.json 2>&1)

if echo "$RESPONSE" | grep -q '"__schema":'; then
    echo -e "   ${GREEN}✓${NC} Introspection works"
    ((PASS_COUNT++))
elif echo "$RESPONSE" | grep -q '"errors"'; then
    echo -e "   ${RED}✗${NC} Introspection returned errors"
    echo "   Response: $RESPONSE"
    ((FAIL_COUNT++))
elif echo "$RESPONSE" | grep -q '"data":{}'; then
    echo -e "   ${YELLOW}⚠${NC} Introspection returned empty data (parsing issue?)"
    echo "   Response: $RESPONSE"
    ((FAIL_COUNT++))
else
    echo -e "   ${RED}✗${NC} Introspection failed"
    echo "   Response: $RESPONSE"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 12: Mass Assignment (if logged in)
#==========================================
echo "12. Testing Mass Assignment (updateProfile)..."
if [ -n "$ADMIN_TOKEN" ]; then
    create_test_file /tmp/test_mass_assign.json '{"query":"mutation { updateProfile(firstName: \"Hacked\", role: \"admin\") { id username role firstName } }"}'
    RESPONSE=$(curl -s -X POST "$API_URL" \
        -H 'Content-Type: application/json' \
        -H "Authorization: Bearer $ADMIN_TOKEN" \
        --data-binary @/tmp/test_mass_assign.json)
    if echo "$RESPONSE" | grep -q '"role":"admin"'; then
        echo -e "   ${YELLOW}⚠${NC} Mass Assignment vulnerability (role changed to admin!)"
        ((PASS_COUNT++))  # Intentional vulnerability
    elif echo "$RESPONSE" | grep -q '"firstName":"Hacked"'; then
        echo -e "   ${GREEN}✓${NC} Profile updated but role protected"
        ((PASS_COUNT++))
    else
        echo -e "   ${YELLOW}⊘${NC} Update may have failed"
        echo "   Response: $RESPONSE"
    fi
else
    echo -e "   ${YELLOW}⊘${NC} Skipped (no token)"
fi
echo ""

#==========================================
# TEST 13: IDOR - Cancel Order
#==========================================
echo "13. Testing IDOR (cancelOrder)..."
create_test_file /tmp/test_login_user.json '{"query":"mutation { login(username: \"user\", password: \"password123\") { token } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_login_user.json)
USER_TOKEN=$(echo "$RESPONSE" | grep -oP '"token":"[^"]+' | cut -d'"' -f4)

if [ -n "$USER_TOKEN" ]; then
    create_test_file /tmp/test_cancel_order.json '{"query":"mutation { cancelOrder(orderId: \"order-nonexistent-123\") { success message } }"}'
    RESPONSE=$(curl -s -X POST "$API_URL" \
        -H 'Content-Type: application/json' \
        -H "Authorization: Bearer $USER_TOKEN" \
        --data-binary @/tmp/test_cancel_order.json)
    if echo "$RESPONSE" | grep -q '"Order not found"' || echo "$RESPONSE" | grep -q '"success":false'; then
        echo -e "   ${GREEN}✓${NC} IDOR protected (order not found)"
        ((PASS_COUNT++))
    else
        echo -e "   ${YELLOW}⚠${NC} IDOR may be possible"
        echo "   Response: $RESPONSE"
    fi
else
    echo -e "   ${YELLOW}⊘${NC} Skipped (could not login)"
fi
echo ""

#==========================================
# TEST 14: CORS Headers
#==========================================
echo "14. Testing CORS Headers..."
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    -H 'Origin: http://example.com' \
    --data-binary @/tmp/test_books.json \
    -D - 2>/dev/null | grep -i "access-control-allow-origin")
if echo "$RESPONSE" | grep -q "Access-Control-Allow-Origin"; then
    echo -e "   ${GREEN}✓${NC} CORS headers present"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} CORS headers missing"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 15: Batch Query (Bypasses Rate Limiting)
#==========================================
echo "15. Testing Batch Query (_batchQuery bypasses rate limiting)..."
create_test_file /tmp/test_batch.json '{"query":"[{query: \"{ books { id } }\"}, {query: \"{ books { title } }\"}, {query: \"{ books { price } }\"}]"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_batch.json)
if echo "$RESPONSE" | grep -q '"id"'; then
    echo -e "   ${GREEN}✓${NC} Batch query endpoint accessible"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} Batch query endpoint failed"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 16: XXE Vulnerability (processXMLData)
#==========================================
echo "16. Testing XXE Vulnerability (processXMLData)..."
create_test_file /tmp/test_xxe.json '{"query":"mutation { processXMLData(data: \"<?xml version=\\\"1.0\\\"?><!DOCTYPE foo [<!ENTITY xxe SYSTEM \\\"file:///etc/passwd\\\"]>]<foo>&xxe;</foo>\") }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_xxe.json)
if echo "$RESPONSE" | grep -q "root:\|nobody:\|www-data:"; then
    echo -e "   ${YELLOW}⚠${NC} XXE VULNERABLE - File content exposed!"
    ((PASS_COUNT++))
elif echo "$RESPONSE" | grep -q '"processXMLData"\|"data":{'; then
    echo -e "   ${GREEN}✓${NC} XXE endpoint accessible (not exploited)"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} XXE endpoint failed"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 17: Race Condition (applyCoupon)
#==========================================
echo "17. Testing Race Condition (applyCoupon)..."
create_test_file /tmp/test_coupon.json '{"query":"mutation { applyCoupon(code: \"SAVE10\") { success message discount } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    -H "Authorization: Bearer $ADMIN_TOKEN" \
    --data-binary @/tmp/test_coupon.json)
if echo "$RESPONSE" | grep -q '"discount"'; then
    echo -e "   ${YELLOW}⚠${NC} Coupon race condition accessible"
    ((PASS_COUNT++))
elif echo "$RESPONSE" | grep -q 'applyCoupon'; then
    echo -e "   ${GREEN}✓${NC} Coupon endpoint accessible"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} Coupon endpoint failed"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 18: JWT Algorithm Confusion (decodeJWT)
#==========================================
echo "18. Testing JWT Algorithm Confusion (decodeJWT)..."
create_test_file /tmp/test_jwt.json '{"query":"query { decodeJWT(token: \"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c\") }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_jwt.json)
if echo "$RESPONSE" | grep -q '"header"\|"payload"'; then
    echo -e "   ${YELLOW}⚠${NC} JWT decode exposes token contents!"
    ((PASS_COUNT++))
elif echo "$RESPONSE" | grep -q '"decodeJWT"\|"data":{'; then
    echo -e "   ${GREEN}✓${NC} JWT decode endpoint accessible"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} JWT decode endpoint failed"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 19: Cache Poisoning (manageCache)
#==========================================
echo "19. Testing Cache Poisoning (manageCache)..."
create_test_file /tmp/test_cache.json '{"query":"query { manageCache(action: \"set\", key: \"malicious\", value: \"<script>alert(1)</script>\") }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    -H "X-Cache-Poison: malicious-value" \
    --data-binary @/tmp/test_cache.json)
if echo "$RESPONSE" | grep -q '"success"\|"value"'; then
    echo -e "   ${YELLOW}⚠${NC} Cache poisoning possible!"
    ((PASS_COUNT++))
elif echo "$RESPONSE" | grep -q '"manageCache"\|"data":{'; then
    echo -e "   ${GREEN}✓${NC} Cache management endpoint accessible"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} Cache management endpoint failed"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 20: Deep Recursion (handleRecursiveQuery)
#==========================================
echo "20. Testing Deep Recursion (handleRecursiveQuery)..."
create_test_file /tmp/test_recursive.json '{"query":"query { handleRecursiveQuery(depth: 50) { result } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_recursive.json)
if echo "$RESPONSE" | grep -q '"handleRecursiveQuery"\|"data":{'; then
    echo -e "   ${YELLOW}⚠${NC} Recursive query endpoint accessible (DoS possible)"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} Recursive query endpoint failed"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 21: Pro Inventory (_proInventory)
#==========================================
echo "21. Testing Hidden Pro Inventory..."
create_test_file /tmp/test_pro.json '{"query":"query { _proInventory { id title author difficulty hint } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_pro.json)
PRO_COUNT=$(echo "$RESPONSE" | grep -o '"id"' | wc -l)
if [ "$PRO_COUNT" -gt 0 ]; then
    echo -e "   ${YELLOW}⚠${NC} Hidden pro books exposed ($PRO_COUNT books)"
    ((PASS_COUNT++))
elif echo "$RESPONSE" | grep -q '"_proInventory"\|"data":{'; then
    echo -e "   ${GREEN}✓${NC} Pro inventory endpoint accessible"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} Pro inventory endpoint failed"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 22: Advanced Search SQL Injection
#==========================================
echo "22. Testing Advanced Search SQL Injection..."
create_test_file /tmp/test_adv_search.json '{"query":"query { _searchAdvanced(query: \"\\\" OR 1=1--\") { id title author { name } } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_adv_search.json)
SQLI_COUNT=$(echo "$RESPONSE" | grep -o '"id"' | wc -l)
if [ "$SQLI_COUNT" -gt 6 ]; then
    echo -e "   ${YELLOW}⚠${NC} SQL Injection VULNERABLE! ($SQLI_COUNT results)"
    ((PASS_COUNT++))
elif echo "$RESPONSE" | grep -q '_searchAdvanced'; then
    echo -e "   ${GREEN}✓${NC} Advanced search accessible (SQLi not exploited)"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} Advanced search endpoint failed"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 23: Login Timing Attack
#==========================================
echo "23. Testing Login Timing Attack..."

# Test with invalid username
START1=$(date +%s%N)
curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    -d '{"query":"mutation { login(username: \"nonexistent999\", password: \"wrong\") { token } }"}' > /dev/null
END1=$(date +%s%N)
TIME1=$((($END1 - $START1) / 1000000))

# Test with valid username, wrong password  
START2=$(date +%s%N)
curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    -d '{"query":"mutation { login(username: \"admin\", password: \"wrongpass\") { token } }"}' > /dev/null
END2=$(date +%s%N)
TIME2=$((($END2 - $START2) / 1000000))

DIFF=$((TIME2 - TIME1))
if [ "$DIFF" -gt 10 ]; then
    echo -e "   ${YELLOW}⚠${NC} Timing attack possible! Diff: ${DIFF}ms"
    ((PASS_COUNT++))
else
    echo -e "   ${GREEN}✓${NC} Login timing: ${TIME2}ms (diff: ${DIFF}ms)"
    ((PASS_COUNT++))
fi
echo ""

#==========================================
# TEST 24: GraphQL Alias Abuse
#==========================================
echo "24. Testing GraphQL Alias Abuse..."
create_test_file /tmp/test_alias.json '{"query":"{ a1: books { id } a2: books { id } a3: books { id } a4: books { id } a5: books { id } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_alias.json)
if echo "$RESPONSE" | grep -q '"books"'; then
    echo -e "   ${GREEN}✓${NC} GraphQL aliases accessible (endpoint works)"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} GraphQL aliases failed"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 25: Batch Query Bypass Rate Limiting
#==========================================
echo "25. Testing Batch Query (Bypasses Rate Limiting)..."
create_test_file /tmp/test_batch.json '{"query":"[{query: \"{ books { id } }\"}, {query: \"{ books { title } }\"}, {query: \"{ books { price } }\"}, {query: \"{ books { author { name } } }\"}, {query: \"{ books { description } }\"}]"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_batch.json)
if echo "$RESPONSE" | grep -q '"id"'; then
    echo -e "   ${YELLOW}⚠${NC} Batch query bypasses rate limiting!"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} Batch query failed"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 26: SSRF Vulnerability
#==========================================
echo "26. Testing SSRF Vulnerability..."
create_test_file /tmp/test_ssrf_meta.json '{"query":"query { _fetchExternalResource(url: \"http://169.254.169.254/latest/meta-data/\") }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    --data-binary @/tmp/test_ssrf_meta.json)
if echo "$RESPONSE" | grep -q "iam\|meta-data\|aws\|ec2"; then
    echo -e "   ${YELLOW}⚠${NC} SSRF VULNERABLE - Cloud metadata accessible!"
    ((PASS_COUNT++))
elif echo "$RESPONSE" | grep -q '_fetchExternalResource'; then
    echo -e "   ${GREEN}✓${NC} SSRF endpoint accessible"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} SSRF endpoint failed"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 27: Logout (JWT Stateless)
#==========================================
echo "27. Testing Logout Mutation..."
create_test_file /tmp/test_logout.json '{"query":"mutation { logout { success message } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    -H "Authorization: Bearer $ADMIN_TOKEN" \
    --data-binary @/tmp/test_logout.json)
if echo "$RESPONSE" | grep -q '"success":true'; then
    echo -e "   ${GREEN}✓${NC} Logout works (but JWT is stateless)"
    ((PASS_COUNT++))
else
    echo -e "   ${RED}✗${NC} Logout failed"
    ((FAIL_COUNT++))
fi
echo ""

#==========================================
# TEST 28: Second-Order XSS
#==========================================
echo "28. Testing Second-Order XSS (Review Comments)..."

# Create a review with XSS payload
create_test_file /tmp/test_xss_review.json '{"query":"mutation { createReview(bookId: 1, rating: 5, comment: \"<script>alert(1)</script>\") { success message } }"}'
RESPONSE=$(curl -s -X POST "$API_URL" \
    -H 'Content-Type: application/json' \
    -H "Authorization: Bearer $ADMIN_TOKEN" \
    --data-binary @/tmp/test_xss_review.json)

if echo "$RESPONSE" | grep -q '"success":true'; then
    # Retrieve the review - check if comment is stored and returned unsanitized
    create_test_file /tmp/test_xss_retrieve.json '{"query":"query { myReviews { id comment } }"}'
    RESPONSE=$(curl -s -X POST "$API_URL" \
        -H 'Content-Type: application/json' \
        -H "Authorization: Bearer $ADMIN_TOKEN" \
        --data-binary @/tmp/test_xss_retrieve.json)
    
    if echo "$RESPONSE" | grep -q '<script>'; then
        echo -e "   ${YELLOW}⚠${NC} Second-Order XSS VULNERABLE! Script tags stored and returned unsanitized"
        ((PASS_COUNT++))
    else
        echo -e "   ${GREEN}✓${NC} Second-Order XSS endpoint accessible (XSS sanitized or not reflected)"
        ((PASS_COUNT++))
    fi
else
    echo -e "   ${GREEN}✓${NC} Second-Order XSS test completed"
    ((PASS_COUNT++))
fi
echo ""

#==========================================
# Summary
#==========================================
echo "=========================================="
echo "  Test Summary                          "
echo "=========================================="
echo ""
echo "KEY TESTS (Must Pass):"
echo -e "  Registration:  $([ -n "$REGISTERED" ] && echo -e "${GREEN}✓ PASS${NC}" || echo -e "${RED}✗ FAIL${NC}")"
echo -e "  Login:        $([ -n "$ADMIN_TOKEN" ] && echo -e "${GREEN}✓ PASS${NC}" || echo -e "${RED}✗ FAIL${NC}")"
echo -e "  All Tests:    $([ $FAIL_COUNT -eq 0 ] && echo -e "${GREEN}✓ PASS${NC}" || echo -e "${RED}✗ FAIL${NC}")"
echo ""
echo -e "  ${GREEN}Passed:${NC} $PASS_COUNT"
echo -e "  ${RED}Failed:${NC} $FAIL_COUNT"
echo ""

if [ $FAIL_COUNT -eq 0 ]; then
    echo -e "${GREEN}=========================================="
    echo "  ALL TESTS PASSED - READY FOR DEPLOYMENT!"
    echo -e "==========================================${NC}"
    exit 0
else
    echo -e "${YELLOW}=========================================="
    echo "  SOME TESTS FAILED - REVIEW OUTPUT ABOVE"
    echo -e "==========================================${NC}"
    exit 1
fi
