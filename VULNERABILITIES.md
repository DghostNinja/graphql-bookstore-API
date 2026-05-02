# Vulnerabilities in GraphQL Bookstore API - OWASP API Security Top 10

This document outlines intentional security vulnerabilities designed for educational purposes. All vulnerabilities are designed to be realistic, requiring knowledge of GraphQL, API security, and creative thinking to exploit.

## 1. Broken Object Level Authorization (BOLA) - Critical

### Location: `src/resolvers/user_resolvers.cpp:73-95`
- **Vulnerability**: The `resolveUser` function allows staff and admin users to access ANY user's data without additional validation.
- **Realism**: Staff often need view access for customer support, but authorization doesn't distinguish between necessary access and unrestricted access.
- **Exploit Chain**:
  1. Use `users` query to enumerate all users
  2. Use `user` query with specific IDs to access detailed profiles
  3. Access nested fields including orders, reviews, payment details
- **Difficulty**: Medium - Requires staff account, then straightforward data access

### Location: `src/resolvers/order_resolvers.cpp:46-58`
- **Vulnerability**: Order resolver checks ownership on top level but nested fields (items, payments) don't re-validate.
- **Realism**: Developers often add authorization at query level but forget nested field resolvers.
- **Exploit**: Query `order` with ID of another user's order, then request `paymentTransactions` nested field.
- **Difficulty**: Medium - Requires finding valid order IDs

### Location: `src/resolvers/admin_resolvers.cpp:59-130`
- **Vulnerability**: `_internalOrdersByDate` exposes ALL orders in date range with user PII including password hashes.
- **Realism**: Internal endpoints for reporting often have overly broad data access.
- **Exploit**: Use broad date ranges to dump all user data including passwords.
- **Difficulty**: Low - Once you discover the endpoint, data is freely accessible

## 2. Broken User Authentication - Critical

### Location: `src/auth/jwt_handler.cpp:1-150`
- **Vulnerability**: JWT secret is hardcoded and weak (`CHANGE_ME_IN_PRODUCTION_real_jwt_secret_key_2024`).
- **Realism**: Development secrets often make it to production.
- **Exploit**: Forge JWTs for any user ID and role, including admin.
- **Difficulty**: Low - If you can access the source or guess the secret

### Location: `src/auth/authorization.cpp:70-90`
- **Vulnerability**: Staff users can elevate their role by modifying their own profile (mass assignment vulnerability).
- **Realism**: Role field exists in UpdateProfileInput but developers forget to block it.
- **Exploit**:
  ```graphql
  mutation {
    updateProfile(input: {role: "admin"}) {
      role
    }
  }
  ```
- **Difficulty**: Medium - Need to find the mass assignment vulnerability first

### Location: `src/main.cpp:42`
- **Vulnerability**: JWT tokens never expire (or have very long expiration).
- **Realism**: Developers set long expiration times to improve UX.
- **Exploit**: Stolen tokens remain valid indefinitely.
- **Difficulty**: Low - Once you have a token, it's valid forever

### Location: `src/auth/jwt_handler.cpp:54-63`
- **Vulnerability**: JWT verification only checks signature, doesn't validate token revocation.
- **Realism**: Stateless JWTs can't be revoked without implementing a blocklist.
- **Exploit**: Even after logout or account deletion, tokens remain valid.
- **Difficulty**: Medium - Requires understanding JWT statelessness

## 3. Excessive Data Exposure - High

### Location: `src/graphql/schema.cpp:128-150`
- **Vulnerability**: User type includes sensitive fields (phone, address, password hash) that can be bulk queried.
- **Realism**: GraphQL makes it easy to accidentally expose all fields.
- **Exploit**: Use `users` query with high limit to dump all user PII.
- **Difficulty**: Low - GraphQL's introspection reveals all fields

### Location: `src/graphql/schema.cpp:199-230`
- **Vulnerability**: Order type exposes full payment details including card type and last four digits.
- **Realism**: Developers don't consider nested field exposure.
- **Exploit**:
  ```graphql
  query {
    order(id: "ORDER_ID") {
      paymentTransactions {
        lastFourDigits
        cardType
        gatewayResponse
      }
    }
  }
  ```
- **Difficulty**: Medium - Requires finding a valid order ID

### Location: `src/resolvers/admin_resolvers.cpp:59-130`
- **Vulnerability**: `_internalUserSearch` returns password hashes in search results.
- **Realism**: Internal search endpoints sometimes return full user objects.
- **Exploit**: Search for common email patterns to dump password hashes.
- **Difficulty**: Low - Search endpoint accessible to staff

## 4. Lack of Resources & Rate Limiting - High

### Location: `src/main.cpp:150-200`
- **Vulnerability**: No query depth, complexity, or rate limiting implemented.
- **Realism**: GraphQL servers often skip performance optimization in early deployments.
- **Exploit 1 - Deep Nesting**:
  ```graphql
  query {
    user(id: "ID") {
      orders {
        items {
          book {
            author {
              books {
                reviews {
                  user {
                    orders { ... }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  ```
- **Exploit 2 - Alias Abuse**:
  ```graphql
  query {
    u1: user(id: "1") { id email phone address }
    u2: user(id: "2") { id email phone address }
    u3: user(id: "3") { id email phone address }
    # ... 100 more users
  }
  ```
- **Difficulty**: Medium - Requires crafting malicious queries

### Location: `src/resolvers/user_resolvers.cpp:102-117`
- **Vulnerability**: `users` query accepts arbitrary high limit values.
- **Realism**: Developers add pagination but don't enforce maximum limits.
- **Exploit**: Query with `limit: 100000` to dump entire database.
- **Difficulty**: Low - Change limit parameter

## 5. Broken Object Property Level Authorization - High

### Location: `src/graphql/schema.cpp:415-425`
- **Vulnerability**: UpdateProfileInput allows `role` field modification.
- **Realism**: Input types often include all fields for flexibility.
- **Exploit**: Update your own profile to admin role.
- **Difficulty**: Medium - Requires discovering the mass assignment vulnerability

### Location: `src/graphql/schema.cpp:427-446`
- **Vulnerability**: BookInput allows modifying all book fields including internal ones.
- **Realism**: Admin inputs often mirror database schema exactly.
- **Exploit**: Modify `isFeatured`, `isBestseller`, or `ratingAverage` arbitrarily.
- **Difficulty**: Low - Once you have staff/admin access

### Location: `src/resolvers/order_resolvers.cpp:212-272`
- **Vulnerability**: `updateOrderAddress` doesn't verify user owns the order.
- **Realism**: Staff can update orders, but the mutation doesn't distinguish between own orders and others'.
- **Exploit**: Staff can change shipping addresses of other users' orders.
- **Difficulty**: Medium - Requires staff access

## 6. Mass Assignment - Medium

### Location: `src/resolvers/user_resolvers.cpp:122-159`
- **Vulnerability**: `updateProfile` iterates through ALL input fields and updates them blindly.
- **Realism**: Generic update functions are common for DRY code.
- **Exploit**:
  ```graphql
  mutation {
    updateProfile(input: {
      firstName: "Admin",
      lastName: "User",
      role: "admin",
      isActive: true,
      email: "newadmin@bookstore.com"
    }) {
      id
      role
      email
    }
  }
  ```
- **Difficulty**: Medium - Requires testing different input combinations

### Location: `src/resolvers/admin_resolvers.cpp:280-340`
- **Vulnerability**: `_bulkUpdateUsers` applies updates to all user IDs in list without individual authorization.
- **Realism**: Bulk operations skip per-item validation for performance.
- **Exploit**: Update multiple users' roles or other fields in one mutation.
- **Difficulty**: Low - Once you find the endpoint

## 7. Security Misconfiguration - High

### Location: `src/main.cpp:95-100`
- **Vulnerability**: GraphQL Playground is enabled in production and shows full schema.
- **Realism**: Playgrounds left enabled for debugging in production.
- **Exploit**: Use introspection to discover all queries, mutations, and types.
- **Difficulty**: Low - Access the playground URL

### Location: `src/resolvers/admin_resolvers.cpp:187-210`
- **Vulnerability**: `_debugQuery` returns raw SQL queries and execution details.
- **Realism**: Debug endpoints often left enabled for troubleshooting.
- **Exploit**:
  ```graphql
  mutation {
    _debugQuery(query: "query { user(id: \"1\") { id } }") {
      databaseQueries
      context
      executionTime
    }
  }
  ```
- **Difficulty**: Low - Query is documented in schema

### Location: `src/main.cpp:42`
- **Vulnerability**: Weak JWT secret and no rotation mechanism.
- **Realism**: Default secrets in configuration files.
- **Exploit**: Forge admin tokens using the weak secret.
- **Difficulty**: Medium - Requires access to secret or guessing

### Location: `src/resolvers/admin_resolvers.cpp:246-268`
- **Vulnerability**: `_exportSchema` reveals internal endpoints and hidden mutations.
- **Realism**: Schema export for documentation often exposes internal APIs.
- **Exploit**: Discover all `_` prefixed endpoints and their purposes.
- **Difficulty**: Low - Single query reveals everything

## 8. Injection - Critical

### Location: `src/resolvers/admin_resolvers.cpp:187-210`
- **Vulnerability**: `_debugQuery` doesn't sanitize input, returns raw query strings.
- **Realism**: Debug endpoints often log queries without sanitization.
- **Exploit**:
  ```graphql
  mutation {
    _debugQuery(query: "query { user(id: \"1') OR '1'='1\") { id } }") {
      databaseQueries
    }
  }
  ```
- **Difficulty**: High - Requires understanding GraphQL query structure

### Location: `src/resolvers/user_resolvers.cpp:122-159`
- **Vulnerability**: Direct string concatenation in SQL queries (potential SQLi).
- **Realism**: Dynamic query building without parameterization.
- **Exploit**: While queries use parameters, some edge cases may be vulnerable.
- **Difficulty**: High - Requires finding the vulnerable parameter

### Location: `src/graphql/schema.cpp:415-425`
- **Vulnerability**: GraphQL input validation insufficient, allows unexpected field types.
- **Realism**: GraphQL's flexible typing makes strict validation difficult.
- **Exploit**: Send unexpected types (e.g., arrays for strings) to bypass validation.
- **Difficulty**: High - Requires deep GraphQL knowledge

## 9. Improper Assets Management - Medium

### Location: `src/graphql/schema.cpp:67-119`
- **Vulnerability**: Deprecated mutations still accessible (e.g., old auth methods).
- **Realism**: Old APIs rarely removed for backward compatibility.
- **Exploit**: Find and use deprecated but more permissive endpoints.
- **Difficulty**: Medium - Requires testing all mutations

### Location: `src/resolvers/admin_resolvers.cpp:246-268`
- **Vulnerability**: `_exportSchema` shows internal mutations not meant for public use.
- **Realism**: Internal endpoints exposed for admin tools.
- **Exploit**: Access `_importUsers`, `_bulkUpdateUsers` which shouldn't be exposed.
- **Difficulty**: Low - Export reveals internal APIs

### Location: `src/graphql/schema.cpp:115-119`
- **Vulnerability**: Admin mutations with `_` prefix are accessible via GraphQL schema.
- **Realism**: Prefixes used to "hide" endpoints but they're still in the schema.
- **Exploit**: Use `_internalUserSearch`, `_importUsers` etc.
- **Difficulty**: Low - Schema introspection reveals all

## 10. Insufficient Logging & Monitoring - Medium

### Location: `src/auth/authorization.cpp:129-132`
- **Vulnerability**: Authorization failures only logged to console, not persisted.
- **Realism**: Console logs often lost in production.
- **Exploit**: Attack without detection by monitoring.
- **Difficulty**: Low - No logging to bypass

### Location: `src/resolvers/admin_resolvers.cpp:59-130`
- **Vulnerability**: `_internalOrdersByDate` doesn't log who accessed which data.
- **Realism**: Admin operations often skip audit logging.
- **Exploit**: Bulk export user data without leaving audit trail.
- **Difficulty**: Low - No audit logs to worry about

### Location: `src/main.cpp:150-200`
- **Vulnerability**: No logging of failed authentication attempts or rate limit violations.
- **Realism**: Logging often an afterthought.
- **Exploit**: Brute force authentication without triggering alerts.
- **Difficulty**: Low - Unlimited attempts

## 11. Server-Side Request Forgery (SSRF) - Critical

### Location: `src/resolvers/admin_resolvers.cpp:146-160`
- **Vulnerability**: `_fetchExternalResource` makes HTTP requests to user-provided URLs.
- **Realism**: Integrations with external services often need to fetch URLs.
- **Exploit 1 - Internal Network Scanning**:
  ```graphql
  query {
    _fetchExternalResource(url: "http://169.254.169.254/latest/meta-data/iam/security-credentials/") {
      body
    }
  }
  ```
- **Exploit 2 - Port Scanning**:
  ```graphql
  query {
    _fetchExternalResource(url: "http://localhost:6379") {
      body
    }
  }
  ```
- **Exploit 3 - Cloud Metadata Access**:
  ```graphql
  query {
    _fetchExternalResource(url: "http://169.254.169.254/latest/api/token") {
      body
    }
  }
  ```
- **Difficulty**: Medium - Requires knowledge of SSRF and internal network

### Location: `src/resolvers/admin_resolvers.cpp:179-186`
- **Vulnerability**: `_validateWebhookUrl` makes real HTTP requests to validate URLs.
- **Realism**: Webhook validation often tests endpoints to ensure they're reachable.
- **Exploit**: Use validation endpoint to SSRF into internal services.
- **Difficulty**: Low - Purpose is to make the request

### Location: `src/resolvers/admin_resolvers.cpp:342-378`
- **Vulnerability**: `_importUsers` fetches user data from user-provided URL.
- **Realism**: Bulk user import from external files is common admin feature.
- **Exploit**:
  ```graphql
  mutation {
    _importUsers(fileUrl: "http://169.254.169.254/latest/meta-data/identity-credentials/ec2/security-credentials/ec2-instance") {
      importedCount
    }
  }
  ```
- **Difficulty**: Low - Explicit URL input accepts any URL

### Location: `src/resolvers/admin_resolvers.cpp:187-210`
- **Vulnerability**: `_fetchBookMetadata` can fetch from any source URL.
- **Realism**: Book metadata often fetched from external APIs (ISBNdb, Google Books, etc).
- **Exploit**: SSRF to internal services by providing malicious sourceUrl.
- **Difficulty**: Medium - Requires finding a book ID first

### Location: `src/utils/webhook_manager.cpp:50-65`
- **Vulnerability**: `performHttpRequest` doesn't validate URL, sets `SSL_VERIFYPEER` to 0.
- **Realism**: Developers disable SSL verification for testing and forget to re-enable.
- **Exploit**: Bypass SSL validation and make requests to internal HTTPS services.
- **Difficulty**: Medium - SSL bypass required for some internal services

## 12. Business Logic Flaws - High

### Location: `src/business/cart_manager.cpp:295-380`
- **Vulnerability**: Coupon validation doesn't check per-user usage limits.
- **Realism**: Coupon systems are complex; developers often miss edge cases.
- **Exploit**: Use same coupon multiple times by placing orders separately.
- **Difficulty**: Medium - Requires creating multiple orders

### Location: `src/resolvers/order_resolvers.cpp:98-138`
- **Vulnerability**: `requestRefund` doesn't verify order age, payment status, or previous refunds.
- **Realism**: Refund logic requires multiple checks often incompletely implemented.
- **Exploit**: Refund old orders, already refunded orders, or pending orders.
- **Difficulty**: Low - Mutation doesn't validate much

### Location: `src/business/inventory_manager.cpp:11-27`
- **Vulnerability**: Stock updates read current value, calculate new, write - no atomic operation.
- **Realism**: Race conditions common in e-commerce inventory management.
- **Exploit**: Concurrent order placement leads to overselling.
- **Difficulty**: High - Requires timing and concurrency

### Location: `src/resolvers/order_resolvers.cpp:212-272`
- **Vulnerability**: `updateOrderAddress` doesn't verify order status (can update shipped/delivered orders).
- **Realism**: Address updates often allowed for customer convenience.
- **Exploit**: Change address of shipped order to redirect delivery.
- **Difficulty**: Low - Staff access allows this

### Location: `src/business/payment_processor.cpp:10-50`
- **Vulnerability**: Payment validation is superficial (just checks card length/format).
- **Realism**: Real payment gateway integration often mocked in development.
- **Exploit**: Use fake card numbers that pass validation.
- **Difficulty**: Low - Luhn algorithm is easy to pass

## Realism Factors

### Why These Vulnerabilities Are Realistic:

1. **GraphQL-Specific**: Many vulnerabilities exploit GraphQL's unique features (introspection, nested queries, aliases).

2. **Subtle Implementation Errors**: Most look like honest mistakes rather than intentional backdoors.

3. **Inconsistent Authorization**: Authorization exists but is incompletely or inconsistently applied.

4. **Production Patterns**: Reflect common patterns seen in real GraphQL deployments.

5. **No "IsAdmin" Flags**: Vulnerabilities require understanding the system, not just toggling a flag.

6. **Multi-Step Exploits**: Most require chaining multiple vulnerabilities or queries.

### Difficulty Levels:

- **Low**: Discoverable via introspection, single query/mutation
- **Medium**: Requires combining multiple operations or understanding authorization flow
- **High**: Requires timing attacks, race conditions, or deep GraphQL knowledge

## Exploit Chains

### Chain 1: Full Account Takeover
1. Login as regular user
2. Use mass assignment in `updateProfile` to escalate to staff
3. Use `_internalUserSearch` to find admin users
4. Use SSRF in `_fetchExternalResource` to access admin panel
5. Create new admin account via backend API

### Chain 2: Complete Data Exfiltration
1. Login as staff (or escalate via mass assignment)
2. Use `_exportUserData` to download all user data including password hashes
3. Use `_internalOrdersByDate` to get all orders with payment details
4. Use SSRF to access internal database backups

### Chain 3: Free Books via Business Logic Flaws
1. Create order with coupon code
2. Receive books
3. Request refund (bypasses age verification)
4. Keep books and get money back
5. Repeat with same coupon (no per-user limit)

### Chain 4: SSRF to Internal Services
1. Login as staff
2. Use `_validateWebhookUrl` to scan internal network
3. Discover admin dashboard on localhost:8080
4. Use `_fetchExternalResource` with file:// protocol to read local files
5. Access application secrets and configuration

### Chain 5: SSRF Cloud Metadata Access
1. Login as staff
2. Use `_fetchExternalResource` to access cloud instance metadata
3. Extract temporary credentials
4. Access internal S3 buckets or other cloud services
5. Exfiltrate sensitive data

## Mitigation Strategies

### For Each Vulnerability Type:

1. **BOLA**: Implement consistent authorization at both query and field levels
2. **Broken Auth**: Use strong secrets, short token expiration, token revocation
3. **Excessive Data**: Implement field-level authorization and query complexity limits
4. **Resource Exhaustion**: Enforce query depth, complexity, and rate limits
5. **Property Level Auth**: Use allow-lists for mutation inputs
6. **Mass Assignment**: Explicitly list allowed fields, validate each
7. **Security Config**: Disable introspection in production, remove debug endpoints
8. **Injection**: Use parameterized queries, validate all inputs
9. **Improper Assets**: Remove deprecated endpoints, version API properly
10. **Insufficient Logging**: Comprehensive audit logging for all operations
11. **SSRF**: Validate URLs against allow-list, block internal IPs, don't disable SSL verification
12. **Business Logic**: Comprehensive validation for all operations

## Detection and Prevention

### For Security Teams:

1. **Introspection Monitoring**: Alert on large introspection queries
2. **Rate Limiting**: Detect and block rapid queries from same user
3. **Anomaly Detection**: Flag unusual access patterns (e.g., querying many different user IDs)
4. **SSRF Detection**: Monitor for requests to internal IPs or unusual ports
5. **Audit Log Review**: Regular review of admin operations
6. **Cloud Metadata Monitoring**: Block or monitor access to 169.254.169.254

## Usage for Learning

This API is designed to be exploitable entirely through GraphQL Playground. Learning objectives:

1. GraphQL-specific vulnerabilities and exploitation techniques
2. Building multi-step exploit chains
3. Understanding authorization bypass patterns
4. Practicing SSRF exploitation
5. Business logic flaw identification and exploitation
6. Cloud service interaction via SSRF
7. Race condition exploitation

**DO NOT USE IN PRODUCTION**

## Additional Practice Scenarios

1. **Race Condition**: Create concurrent orders to trigger inventory overselling
2. **SSRF Chain**: Use SSRF to access internal metadata service, extract credentials
3. **Mass Assignment**: Find and exploit hidden mass assignment vulnerabilities
4. **Authorization Bypass**: Combine multiple authorization gaps to access admin data
5. **Business Logic**: Find and exploit subtle business logic flaws in ordering/refunding
6. **Cloud Access**: Use SSRF to access cloud instance metadata and escalate privileges
7. **Internal Scanning**: Use multiple SSRF endpoints to map internal network topology