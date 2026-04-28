# AGENTS.md - Codebase Guide for AI Agents

## CRITICAL RULES

### NEVER use git commands without explicit permission
- NEVER run `git checkout`, `git restore`, `revert`, `reset`, or any git operations that modify the codebase
- If something is broken, ASK the user how they want to proceed
- Do not assume you can revert changes - the user may have local work they haven't pushed yet
- Always ask before making any git operations

## Quick Start

```bash
# Clone and setup (installs deps, builds, sets up database)
git clone <repo-url>
cd GraphQL-Bookstore
./build.sh

# Run the server
./bookstore-server
```

### Rebuilding the Server
**IMPORTANT**: After making any changes to the C++ source code (`src/main.cpp`), you must rebuild the server executable before running it. Running `./bookstore-server` only executes the *last built* version. To rebuild, use the following command:
```bash
./build.sh
```

Server runs on http://localhost:4000/

## Default Credentials
| Username | Password | Role |
|----------|----------|-------|
| admin    | password123 | admin |
| staff    | password123 | staff |
| user     | password123 | user |

## Difficulty Rating

This API is designed for security education with progressive difficulty:

| Aspect | Difficulty |
|--------|-----------|
| GraphQL complexity | 7/10 |
| Number of vulnerabilities | 10+ |
| Realism | 9/10 |
| Finding vulnerabilities | 4/10 (some are hidden) |
| Exploiting them | 5-8/10 (varies) |

**Difficulty Breakdown:**
- **Easy (1-3)**: Basic SQL injection, IDOR, BOLA, weak JWT secret
- **Medium (4-6)**: SSRF, mass assignment, timing attacks, information disclosure
- **Hard (7-8)**: XXE, JWT algorithm confusion, race conditions, cache poisoning
- **Expert (9-10)**: Hidden pro vulnerabilities, batch query bypass

The API starts easy and progressively gets harder with hidden chapters and pro-level challenges.

## Manual Build (if needed)

```bash
# Build only
g++ -std=c++17 -o bookstore-server src/main.cpp -lpq -ljwt -lcurl -lssl -lcrypto

# Or use build script
./build.sh
```

## Testing

### Test Scripts

The project has two test scripts:

1. **`test_api.sh`** - Security/vulnerability test suite (tests intentional vulnerabilities)
2. **`flow.sh`** - User flow test suite (tests complete user journey)

### Security Test Suite (test_api.sh)

```bash
./test_api.sh
```

Tests for intentional vulnerabilities:
1. ✓ Server health / GraphQL Playground
2. ✓ User registration
3. ✓ User login
4. ✓ Books query (no auth)
5. ✓ 'me' query (with auth)
6. ✓ SSRF endpoint
7. ✓ BOLA vulnerability (intentional)
8. ✓ SQL Injection endpoint
9. ✓ Admin endpoints (intentional - no auth)
10. ✓ Complex nested queries
11. ✓ GraphQL introspection
12. ✓ Mass Assignment vulnerability
13. ✓ IDOR vulnerabilities
14. ✓ CORS headers

### User Flow Test Suite (flow.sh)

```bash
./flow.sh
```

Tests the complete user journey through the API:

| Step | Test | Description |
|------|------|-------------|
| 1 | Registration | Create new user account |
| 2 | Login | Authenticate and get JWT token |
| 3 | me query | Get current user profile |
| 4 | Browse books | List all available books |
| 5 | View book | Get single book details |
| 6 | Add to cart | Add book to shopping cart |
| 7 | View cart | Verify cart contents |
| 8 | Remove from cart | Remove item from cart |
| 9 | Add to cart again | Add item back |
| 10 | Create review | Write book review |
| 11 | View book reviews | See all reviews for a book |
| 12 | View my reviews | See current user's reviews |
| 13 | Register webhook | Create webhook subscription |
| 14 | View webhooks | List user's webhooks |
| 15 | Update profile | Change user profile info |
| 16 | View orders (empty) | Check orders before purchase |
| 17 | Purchase cart | Complete checkout with payment |
| 18 | View orders (with purchase) | Verify order created |
| 19 | Cancel order | Cancel the order |
| 20 | Verify cancelled | Confirm order status changed |
| 21 | Delete review | Remove review |
| 22 | Cart empty after purchase | Verify cart cleared |
| 23 | Search books | Test search functionality |
| 24 | Introspection | GraphQL schema discovery |
| 25 | Logout | Logout and clear token |
| 26 | Verify token invalidated | JWT is stateless (token still works) |
| 27 | Access without token | Returns 401 Authentication required |

**Flow Test Output Example:**
```
==========================================
  STEP 17: PURCHASE CART (PAYMENT)       
==========================================
Request: Purchase cart contents
Response: {"data":{"checkout":{"success":true,"orderId":"465e4d82..."...
Order ID: 465e4d82-1949-4497-b9c4-8c8287ab970d
Total Amount: $52.4192
   PASS: Purchase successful (Order ID: 465e4d82...)
```

### Server Logging

The server logs all operations to stderr. When running `./bookstore-server`, you'll see:

```
[REGISTER] username='testuser', firstName='Test', lastName='User'
[LOGIN] username='testuser', password='testpass123'
[QUERY] me (user: testuser)
[QUERY] books(search: "", categoryId: 0)
[QUERY] book(id: 1)
[ADDTOCART] user='testuser', bookId=1, quantity=2
[QUERY] cart (user: testuser)
[REMOVEFROMCART] user='testuser', bookId=1
[CREATEREVIEW] user='testuser', bookId=1, rating=5
[QUERY] bookReviews(bookId: 1)
[QUERY] myReviews (user: testuser)
[REGISTERWEBHOOK] user='testuser', url='http://example.com/webhook'
[QUERY] webhooks (user: testuser)
[UPDATEPROFILE] user='testuser', firstName='Updated', lastName='User'
[QUERY] orders (user: testuser)
[PURCHASECART] user='testuser', cardNumber='4111111111111111'
[PURCHASECART] order created, orderId='xxx', total=52.42
[CANCELORDER] user='testuser', orderId='xxx'
[CANCELORDER] order 'xxx' cancelled successfully
[DELETEREVIEW] user='testuser', reviewId='22'
[DELETEREVIEW] review '22' deleted successfully
```

**Log Format:**
- `[QUERY]` - GraphQL query operations
- `[MUTATION]` - GraphQL mutation operations (e.g., `[REGISTER]`, `[LOGIN]`)
- Includes relevant parameters for debugging

### Testing Commands

```bash
# Run security test suite (requires server running)
./test_api.sh

# Run user flow test suite (requires server running)
./flow.sh

# Test with custom API URL
API_URL=http://localhost:4000/graphql ./test_api.sh
API_URL=http://localhost:4000/graphql ./flow.sh

# Test Docker container
API_URL=http://localhost:4001/graphql ./test_api.sh
API_URL=http://localhost:4001/graphql ./flow.sh
```

### Clean Up
```bash
# Kill server on port 4000
pkill -f bookstore-server

# Or kill port 4000
lsof -ti:4000 | xargs kill -9
```

### Test File Format (Avoid Bash Escaping)
Always use files for JSON payloads to avoid bash escaping issues:

```bash
# Create test file
cat > /tmp/test_login.json << 'EOF'
{"query":"mutation { login(username: \"admin\", password: \"password123\") { success token } }"}
EOF

# Use file input
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  --data-binary @/tmp/test_login.json
```

### Docker Deployment
```bash
# Build and run with docker-compose
docker-compose up --build

# Server will be available at http://localhost:4000
# Database runs automatically in container

# Stop containers
docker-compose down

# Rebuild after code changes
docker-compose up --build --force-recreate
```

### Environment Variables
The following environment variables can be set to configure the server:
- `PORT` - Server port (default: 4000)
- `JWT_SECRET` - JWT signing secret (default: hardcoded weak secret)
- `DB_CONNECTION_STRING` - PostgreSQL connection string
- `DATABASE_URL` - PostgreSQL connection URL (preferred)

Example:
```bash
export PORT=4000
export JWT_SECRET="your-secret-here"
export DATABASE_URL="postgresql://user:pass@host:5432/dbname?sslmode=require"
./bookstore-server
```

### Docker Deployment (CRITICAL)
**ALWAYS set both `DATABASE_URL` and `DB_CONNECTION_STRING` when deploying with Docker!**

The server checks `DATABASE_URL` first, then falls back to `DB_CONNECTION_STRING`. In Docker Compose, use:
```yaml
environment:
  DATABASE_URL: "postgresql://bookstore_user:bookstore_password@postgres:5432/bookstore_db"
  DB_CONNECTION_STRING: "dbname=bookstore_db user=bookstore_user password=bookstore_password host=postgres port=5432"
```
Note: Use `postgres` as the hostname (Docker service name), not `localhost`.

### GitHub Actions
The workflow (`.github/workflows/fly-deploy.yml`) runs:
1. Build Docker image
2. Test with Docker Compose (local PostgreSQL)
3. Push to Docker Hub
4. Deploy to Fly.io

Required GitHub Secrets:
- `DOCKER_USERNAME` - Docker Hub username
- `DOCKER_PASSWORD` - Docker Hub password/access token
- `FLY_API_TOKEN` - Fly.io API token (get with `flyctl auth token`)

## Code Style Guidelines

### Language & Standards
- Language: C++17
- Compiler: g++
- No external web frameworks (raw socket HTTP server)

### Naming Conventions
- **Defines**: `UPPER_CASE_WITH_UNDERSCORES` (e.g., `PORT`, `BUFFER_SIZE`, `JWT_SECRET`)
- **Structs**: `PascalCase` (e.g., `User`, `Book`, `Order`)
- **Struct Members**: `camelCase` (e.g., `username`, `passwordHash`, `firstName`)
- **Functions**: `camelCase` (e.g., `loadUsersCache`, `handleQuery`, `escapeJson`)
- **Variables**: `camelCase` (e.g., `cartId`, `targetOrderId`, `paramValues`)
- **Global Variables**: `camelCase` with descriptive names (e.g., `usersCache`, `booksCache`)

### Import/Include Order
System headers first, then third-party libs:
```cpp
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <ctime>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <postgresql/libpq-fe.h>
#include <jwt.h>
#include <curl/curl.h>
```

### Type Usage
- Use `string` for text fields
- Use `int` for numeric IDs (books, reviews, cart items)
- Use `string` for UUID IDs (users, orders, webhooks)
- Use `double` for prices/monetary values
- Use `bool` for flags
- Use `map<K, V>` for caches keyed by ID
- Use `vector<T>` for lists

### Function Return Types
- `string` for JSON responses
- `bool` for success/failure operations
- `T*` for pointer returns (e.g., `User* getUserByUsername()`)
- `void` for procedures with no return

### Error Handling
- Database operations: Check `PQresultStatus(res)` against `PGRES_TUPLES_OK` or `PGRES_COMMAND_OK`
- JWT operations: Check return codes, return empty objects on failure
- CURL operations: Check `CURLcode res == CURLE_OK`
- Always `PQclear(res)` after use
- Return meaningful error messages in JSON responses

### JSON Building Pattern
Use `stringstream` for building responses:
```cpp
stringstream ss;
ss << "{";
ss << "\"id\":\"" << id << "\",";
ss << "\"username\":\"" << escapeJson(username) << "\"";
ss << "}";
return ss.str();
```

### Database Query Pattern
Use parameterized queries (when not intentionally vulnerable):
```cpp
string sql = "SELECT id, name FROM table WHERE id = $1";
const char* paramValues[1] = {value.c_str()};
PGresult* res = PQexecParams(dbConn, sql.c_str(), 1, nullptr, paramValues, nullptr, nullptr, 0);
// Process result
PQclear(res);
```

### Debug Output
- Use `cerr << "[OPERATION] message"` for operation logging
- Log format: `[UPPERCASE_OPERATION] key='value', key2='value2'`
- Examples:
  - `cerr << "[REGISTER] username='" << username << "', firstName='" << firstName << "'" << endl;`
  - `cerr << "[ADDTOCART] user='" << user << "', bookId=" << bookId << ", quantity=" << quantity << endl;`
  - `cerr << "[QUERY] me (user: " << username << ")" << endl;`
- All GraphQL queries and mutations should log their invocation

### Project Structure
```
src/main.cpp           # Main server
src/payment_handler.cpp  # Vulnbank checkout handling
src/utils.h              # Shared utility function declarations (escapeJson, WriteCallback)
src/utils.cpp            # Shared utility function definitions
src/user_manager.h       # User struct and user-related function declarations
src/user_manager.cpp     # User-related function definitions
src/book_manager.h       # Book/Author structs and book/author-related function declarations
src/book_manager.cpp     # Book/Author-related function definitions
src/order_manager.h      # Cart/Order structs and cart/order-related function declarations
src/order_manager.cpp    # Cart/Order-related function definitions
src/extra_features.h     # Review/Webhook structs and related function declarations
src/extra_features.cpp   # Review/Webhook-related function definitions
src/graphql_handler.h    # GraphQL handler declarations
src/graphql_handler.cpp  # GraphQL handler definitions
src/db_manager.h         # Database manager declarations
src/db_manager.cpp       # Database manager definitions
src/network_manager.h    # Network manager declarations
src/network_manager.cpp  # Network manager definitions
src/html_generator.h     # HTML generator declarations
src/html_generator.cpp   # HTML generator definitions
src/rate_limiter.h       # Rate limiter declarations
src/rate_limiter.cpp     # Rate limiter definitions
scripts/init_database.sql  # Database schema and seed data
build.sh              # Build script
test_api.sh           # Security/vulnerability test suite
flow.sh               # User flow test suite
docker-compose.yml     # Docker deployment
Dockerfile            # Container image
fly.toml              # Fly.io config
deploy-fly.sh         # Fly.io deployment script
```

### Vulnerability Implementation Notes
- Vulnerabilities are INTENTIONAL and realistic
- Follow existing patterns for new vulnerabilities
- No "backdoors" - use subtle implementation errors
- Use `{"name": "_internalName"}` pattern for internal/debug endpoints
- Document vulnerabilities in VULNERABILITIES.md

### Important Constants
```cpp
PORT 4000
JWT_SECRET "CHANGE_ME_IN_PRODUCTION_real_jwt_secret_key_2024"
DB_CONN "dbname=bookstore_db user=bookstore_user password=bookstore_password host=localhost port=5432"
```

### CRITICAL: Database Schema Requirements
When adding new features that use database tables, ensure the schema in `init_database.sql` matches what the code expects:

**shopping_carts table MUST have these columns:**
- `id` (UUID, primary key)
- `user_id` (UUID, foreign key to users)
- `subtotal` (DECIMAL)
- `tax` (DECIMAL)
- `discount` (DECIMAL)
- `coupon_code` (VARCHAR)
- `total` (DECIMAL)

If code expects columns that don't exist in the schema, operations will silently fail or return incorrect results. Always verify schema matches code.

### Default Credentials
| Username | Password | Role |
|----------|----------|-------|
| admin    | password123 | admin |
| staff    | password123 | staff |
| user     | password123 | user |

### GraphQL Request Handling
1. Parse Authorization header to extract JWT token
2. Verify JWT with `verifyJWT()`, get User object
3. Parse query string from POST body
4. Check for "mutation" or "query" keyword
5. Route to `handleMutation()` or `handleQuery()`
6. Return JSON response wrapped in `{"data":{...}}`

### Adding New Queries/Mutations
1. Add struct for new data type if needed
2. Add cache variable: `map<K, T> newCache;`
3. Add `loadNewCache()` function
4. Call `loadNewCache()` in `main()` after db connect
5. Add handler in `handleQuery()` or `handleMutation()` - **CRITICAL: Must add handler implementation!**
6. Add introspection entry in `__schema` query handler (NO descriptions!)
7. Add JSON builder function if needed: `TToJson(const T& t)`

### CRITICAL: Always Implement the Handler
When adding a new mutation or query to the schema, you MUST implement the handler in `graphql_handler.cpp`. Simply adding it to introspection is NOT enough.

**Bug Example (applyCoupon):**
- The mutation was added to schema but handler was missing
- Requests returned `{"data":{}}` with no error - silent failure
- Always test new mutations end-to-end

### Mutation Syntax
**IMPORTANT**: Mutations require parentheses syntax:
```graphql
# WRONG - returns empty response
mutation { createOrder { success orderId } }

# CORRECT - works properly
mutation { createOrder() { success orderId } }
```

### Comment Policy
- DO NOT add code comments
- Keep code concise and self-explanatory
- Comments in documentation only (README, VULNERABILITIES.md)

### Available Queries
| Query | Description | Auth Required |
|-------|-------------|---------------|\n| `me` | Get current authenticated user | Yes |\n| `books` | List all books with optional search and category filter | No |\n| `book(id)` | Get a specific book by ID | No |\n| `cart` | Get user\'s shopping cart | Yes |\n| `orders` | Get user\'s orders | Yes |\n| `bookReviews(bookId)` | Get reviews for a specific book | No |\n| `myReviews` | Get current user\'s reviews | Yes |\n| `webhooks` | Get user\'s registered webhooks | Yes |\n| `_internalUserSearch(username)` | Internal user search | No |\n| `_fetchExternalResource(url)` | Fetch external resource by URL | No |\n| `_searchAdvanced(query)` | Advanced search | No |\n| `_adminStats` | Admin statistics | No |\n| `_adminAllOrders` | All orders | No |\n| `_adminAllPayments` | All payment transactions | No |\n| `_batchQuery` | GraphQL batch queries bypass rate limiting | No |\n| `processXMLData` | XXE vulnerability in XML processing | No |\n| `applyCoupon` | Race condition in coupon application | No |\n| `decodeJWT` | JWT algorithm confusion attack | No |\n| `manageCache` | HTTP cache poisoning via headers | No |\n| `handleRecursiveQuery` | Deep recursion attack via nested queries | No |\n
### Available Mutations
| Mutation | Description | Auth Required |\n|----------|-------------|---------------|\n| `register(username, firstName, lastName, password)` | Register a new user | No |\n| `login(username, password)` | Login and get JWT token | No |\n| `updateProfile(...)` | Update user profile | Yes |\n| `addToCart(bookId, quantity)` | Add item to shopping cart | Yes |\n| `removeFromCart(bookId)` | Remove item from shopping cart | Yes |\n| `applyCoupon(code)` | Apply coupon code to cart | Yes |\n| `createOrder()` | Create order from cart (without payment) | Yes |
| `checkout(cardNumber, expiry, cvv)` | Create order and process payment | Yes |
| `cancelOrder(orderId)` | Cancel an order | Yes |\n| `createReview(bookId, rating, comment)` | Create a review | Yes |\n| `deleteReview(reviewId)` | Delete a review | Yes |\n| `registerWebhook(url, events, secret)` | Register a webhook URL | Yes |\n| `testWebhook(webhookId)` | Test a webhook | Yes |\n
### Recent Features Added
- **Field Selection**: All queries now return only requested fields (e.g., `{ books { id title } }` returns only id and title). Fixed proper nested field selection for cart, orders, and other nested queries.
- **JWT Enhancements**: Tokens now include `iat` (issued at) and `exp` (expires in 6 hours), with proper expiration validation. Expired tokens are now rejected.
- **JSON Validation**: Invalid JSON requests are now rejected with proper error messages (400 Bad Request). Validates for unquoted keys, single quotes, missing/extra braces, and trailing commas.
- **Nested Author Field**: Books can include author details via `author { firstName lastName }`
- **Authors Cache**: Authors are loaded at startup for nested queries
- **Shopping Cart System**: Full cart functionality with add/remove items
- **Order Management**: Create orders from cart, cancel orders
- **Unified Checkout**: New `checkout` mutation creates order and processes payment in one atomic operation
- **Review System**: Create and delete reviews
- **Webhook System**: Register webhooks to receive real-time notifications for order and payment events (order.created, order.paid, order.cancelled, payment.completed, payment.failed, review.created). Includes SSRF vulnerability via testWebhook
- **Logout Mutation**: New `logout` mutation that tells client to discard JWT token (note: JWT is stateless so server cannot invalidate existing tokens)
- **HTTP Status Codes**: Proper 401 for authentication errors, 403 for authorization errors
- **Admin Queries**: Stats, orders, and payments accessible without auth
- **Rate Limiting**: IP-based rate limiting (100 requests/minute, 5-min block)
- **API Documentation Page**: Beautiful glass-morphism landing page with query runner
- **Hidden Pro Inventory**: `_proInventory` query reveals 6 hidden expert-level books
- **Hidden Endpoints**: Additional hidden API endpoints for advanced security testing
- **Pro Vulnerabilities**: 6 additional expert-level security vulnerabilities

### Hidden Pro Vulnerabilities
The server contains 6 hidden expert-level vulnerabilities:

| Query | Description |
|-------|-------------|
| `_batchQuery` | GraphQL batch queries bypass rate limiting |
| `processXMLData` | XXE vulnerability in XML processing |
| `applyCoupon` | Race condition in coupon application |
| `decodeJWT` | JWT algorithm confusion attack |
| `manageCache` | HTTP cache poisoning via headers |
| `handleRecursiveQuery` | Deep recursion attack via nested queries |

### Hidden Pro Books (via `_proInventory`)
The server contains 6 hidden books with advanced security research content:

| Book Key | Title | Difficulty | Hint |
|----------|-------|------------|------|
| quantum_cryptography | Quantum Cryptography: The Next Frontier | master | Look for patterns in the API rate limiting headers |
| zero_day_exploits | Zero-Day Exploits: Offensive Security | master | Check for timing attacks in authentication |
| ai_red_team | AI Red Teaming: Advanced Adversarial ML | master | GraphQL batch queries bypass rate limits |
| blockchain_hacking | Blockchain Hacking: DeFi Vulnerabilities | master | Look for second-order vulnerabilities |
| memory_forensics | Advanced Memory Forensics | master | Check XML parsing for XXE |
| apt_analysis | APT Analysis: Nation-State Threats | master | WebSocket connections may leak data |

### Hidden API Endpoints
These endpoints provide advanced API functionality:

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/webhooks/subscribe` | POST | Subscribe to webhook events |
| `/api/batch` | POST | Batch GraphQL queries |
| `/api/coupons/apply` | POST | Apply coupon codes |
| `/api/debug/timing` | GET | Debug timing information |

### Advanced Features
The server contains additional advanced features for expert-level testing:

1. **Timing Analysis**: Login response times differ based on username validity (25ms vs 50ms delay)
2. **Second-Order Effects**: Review comments are logged and queried for analytics
3. **XML Processing**: XML webhook subscription endpoint handles XML payloads
4. **Batch Queries**: GraphQL batch queries for efficient data loading
5. **Concurrent Operations**: Coupon application handles concurrent requests
6. **Debug Endpoints**: Timing information available for debugging

### Rate Limiting
The server includes built-in rate limiting to prevent abuse:
- **100 requests per 60 seconds** per IP address
- **5-minute block** when limit exceeded
- **Automatic cleanup** of old entries every 60 seconds

Rate limit configuration constants in `src/main.cpp`:
```cpp
#define RATE_LIMIT_WINDOW_SECONDS 60
#define RATE_LIMIT_MAX_REQUESTS 100
#define RATE_LIMIT_BLOCK_DURATION 300
```

### Cold Start & Idle Connection Handling
The server is configured for optimal performance when traffic resumes after idle periods:

**fly.toml Configuration:**
```toml
[http_service]
  auto_stop_machines = false
  auto_start_machines = false
  min_machines_running = 1

[[http_service.checks]]
  path = "/health"
  interval = "30s"
  timeout = "5s"
  grace_period = "10s"
```

**Database Auto-Reconnect (src/db_manager.cpp):**
- `checkDatabaseConnection()` - Auto-detects lost connections and reconnects
- `safeExec()` / `safeExecParams()` - Wrapper functions that ensure connection before queries
- `getConnection()` - Returns live connection, auto-reconnects if needed

**How It Works:**
1. Fly.io machine stays warm → no cold start latency (~15s delay eliminated)
2. Neon DB goes idle → connection drops
3. Next request triggers `checkDatabaseConnection()` → auto-reconnect
4. No manual intervention required
368: 
369: ### API Documentation Page
370: The landing page (`generateLandingHTML()` in `src/main.cpp`) provides:
371: - Glass-morphism UI design with animated backgrounds
372: - **API Link Bar**: Small glass icon with pulse animation and copyable link to `api.graphqlbook.org/graphql`
373: - Query Runner panel for testing GraphQL queries
374: - Login and Registration panels with JWT token storage
375: - Quick examples and available endpoints grid
376: - Click-to-load endpoint examples
377: - Vulnerability chapter slideshow
378: 
379: **NOTE**: No built-in GraphQL Playground. Use external tools like:
380: - https://studio.apollographql.com/
381: - Postman, Insomnia, curl, Burp Suite

### Web UI Documentation Structure
The web documentation is organized as follows:

| Section | Description |
|---------|-------------|
| **Welcome** | Introduction to the API, purpose, business flow, default credentials |
| **Local Installation** | Step-by-step guide to run the API locally |
| **Docker Installation** | Quick Docker setup guide |
| **How To Send Requests** | API endpoints, request structure, cURL, JavaScript, Postman examples, download collection |
| **Data Types & Schema** | Main data types, field selection, GraphQL introspection |
| **Queries** | All available queries (public and protected) |
| **Mutations** | All available mutations grouped by category |
| **Vulnerabilities** | Security considerations (no exploitation details) |
| **Feedback** | Anonymous feedback form connected to Google Sheets |

**API Endpoints:**
- Local: `http://localhost:4000/graphql`
- Live: `https://api.graphqlbook.org/graphql`

**Postman Collection:**
A ready-to-use Postman collection is available at `/graphql.json`. Download it directly from the UI in the "How To Send Requests" section, or access it at:
- Local: `http://localhost:4000/graphql.json`
- Live: `https://api.graphqlbook.org/graphql.json`

The collection includes all public queries and mutations with input variables. Import it into Postman and set the `base_url` variable, then fill in input variables (username, password, bookId, etc.) before each request.
382: 
383: **API Link Bar Features:**
384: - Glass-styled icon with green gradient and pulse animation effect
385: - Clickable API URL that copies to clipboard when clicked
386: - Shows "Copied!" feedback for 2 seconds after clicking
387: - Styled with italic serif font for "Access the API at:" label
388: 
389: **Query Runner Features:**
390: - Textarea for entering GraphQL queries
391: - "Load Sample" button loads books query example
392: - "Run Query" button executes query via XMLHttpRequest
393: - Response displayed with JSON formatting
394: - Optional username/password for authenticated requests
395: 
396: **Authentication Panel Features:**
397: - Separate Login and Register panels
398: - JWT tokens stored in localStorage
399: - Tokens automatically used for authenticated requests

### Feedback System
The web UI includes an anonymous feedback form connected to Google Sheets:

**Features:**
- Anonymous comment submission (no login required)
- Simple math CAPTCHA to prevent bot spam
- Input sanitization to prevent XSS attacks
- Rate limiting (30-second cooldown between submissions)
- Honeypot field to catch automated bots

**Floating Feedback Button:**
- Green gradient chat icon fixed to bottom-right corner
- Click to open feedback form from any page
- Responsive design (smaller on mobile devices)

**Google Sheets Integration:**
- Submissions are sent to a Google Apps Script web app
- Data is stored in Google Sheets with columns: Timestamp, Name, Comment

**Spreadsheet Setup:**
The Google Sheet should have these headers in row 1:
- Column A: Timestamp
- Column B: Name
- Column C: Comment

**Google Apps Script Code:**
```javascript
function doPost(e) {
  const sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName('Sheet1');
  const data = JSON.parse(e.postData.contents);
  sheet.appendRow([
    new Date(),
    data.name || '',
    data.comment || ''
  ]);
  return ContentService.createTextOutput(JSON.stringify({ success: true }))
    .setMimeType(ContentService.MimeType.JSON);
}
```

Deploy as Web App with "Anyone" access to allow anonymous submissions.400: 
401: ### Fly.io Deployment
402: App name: `graphql-bookstore`
403: URL: https://graphql-bookstore.fly.dev
404: 
405: ### Testing New Features
406: ```bash
407: # Test cart with authentication
408: cat > /tmp/test_login.json << 'EOF'
409: {"query":"mutation { login(username: \"admin\", password: \"password123\") { token } }"}
410: EOF
411: TOKEN=$(curl -s -X POST http://localhost:4000/graphql \
412:   -H 'Content-Type: application/json' \
413:   -H "Authorization: Bearer $TOKEN" \
414:   --data-binary @/tmp/test_login.json | grep -oP '"token":"[^"]+' | cut -d'"' -f4)
415: 
416: cat > /tmp/test_cart.json << 'EOF'
417: {"query":"mutation { addToCart(bookId: 1, quantity: 2) { success message } }"}
418: EOF
419: curl -X POST http://localhost:4000/graphql \
420:   -H 'Content-Type: application/json' \
421:   -H "Authorization: Bearer $TOKEN" \
422:   --data-binary @/tmp/test_cart.json
423: 
424: # Test order creation
425: cat > /tmp/test_order.json << 'EOF'
426: {"query":"mutation { createOrder { success orderId totalAmount } }"}
427: EOF
428: curl -X POST http://localhost:4000/graphql \
429:   -H 'Content-Type: application/json' \
430:   -H "Authorization: Bearer $TOKEN" \
431:   --data-binary @/tmp/test_order.json
432: 
433: # Test admin queries (no auth required!)
434: cat > /tmp/test_admin.json << 'EOF'
435: {"query":"query { _adminStats { userCount bookCount totalRevenue } }"}
436: EOF
437: curl -X POST http://localhost:4000/graphql \
438:   -H 'Content-Type: application/json' \
439:   --data-binary @/tmp/test_admin.json
440: 
441: # Test SQL injection
442: cat > /tmp/test_sql.json << 'EOF'
443: {"query":"query { _searchAdvanced(query: \"%\" OR 1=1) { id title } }"}
444: EOF
445: curl -X POST http://localhost:4000/graphql \
446:   -H 'Content-Type: application/json' \
447:   --data-binary @/tmp/test_sql.json
448: ```
449: 
**Alternative: Use Postman Collection**
Instead of curl, download the Postman collection from `/graphql.json` and import it into Postman. Set the `base_url` variable to your server URL, fill in the input variables for each request, and set `token` after logging in.

450: ### SSRF URL Whitelist
451: Allowed prefixes for `_fetchExternalResource`:
452: - `http://example.com`
453: - `http://httpbin.org`
454: - `http://api.github.com`, `https://api.github.com`
455: - `http://169.254.169.254` (cloud metadata)
456: - `http://localhost:`, `http://127.0.0.1:`
457: 
458: ---
459: 
460: ## CRITICAL: GraphQL Query Parsing Guidelines
461: 
462: ### The Backslash-Escaped Quote Problem
463: 
464: When bash receives curl commands with `\"` inside single-quoted JSON, bash adds additional backslashes. For example:
465: 
466: ```bash
467: # What you TYPE:
468: -d '{"query":"mutation { login(username: \"admin\", password: \"password123\") { success } }"}'
469: 
470: # What the SERVER receives:
471: {"query":"mutation { login(username: \\"admin\\", password: \\"password123\\") { success } }"}
472: ```
473: 
474: The `\"` becomes `\\"` - the backslash is LITERAL in the string.
475: 
476: ### Correct extractValue() Implementation
477: 
478: This is the CORRECT implementation that handles escaped quotes:
479: 
480: ```cpp
481: string extractValue(const string& query, const string& key) {
482:     string searchKey = key + ":";
483:     size_t keyPos = query.find(searchKey);
484:     if (keyPos == string::npos) return "";
485:     
486:     size_t searchStart = keyPos + searchKey.length();
487:     
488:     // Skip whitespace
489:     while (searchStart < query.length() && 
490:            (query[searchStart] == ' ' || query[searchStart] == '\\t')) {
491:         searchStart++;
492:     }
493:     
494:     if (searchStart >= query.length()) return "";
495:     
496:     // Skip opening quote (may be escaped with backslash like \")
497:     if (query[searchStart] == '"') {
498:         searchStart++;
499:     } else if (query[searchStart] == '\\' && 
500:                searchStart + 1 < query.length() && 
501:                query[searchStart + 1] == '"') {
502:         // Skip escaped quote: \"
503:         searchStart += 2;
504:     }
505:     
506:     string value;
507:     bool escaped = false;
508:     
509:     for (size_t i = searchStart; i < query.length(); i++) {
510:         char c = query[i];
511:         
512:         if (escaped) {
513:             // If we're escaped and see a quote, it's an escaped quote - skip it
514:             if (c != '"') {
515:                 value += c;
516:             }
517:             escaped = false;
518:         } else if (c == '\\') {
519:             escaped = true;
520:         } else if (c == '"') {
521:             // End of string
522:             return value;
523:         } else if (c == ' ' || c == ',' || c == ')' || c == '{' || c == '}') {
524:             // End of value (unquoted)
525:             return value;
526:             
527:         } else {
528:             value += c;
529:         }
530:     }
531:     
532:     return value;
533: }
534: ```
535: 
536: ### Key Points:
537: 1. Check for both `"` (unescaped) AND `\"` (escaped) as opening quotes
538: 2. When `escaped=true`, a `"` means an escaped quote - skip it, don\'t add to value
539: 3. Only return when you hit an UNESCAPED closing quote
540: 4. Handle whitespace, commas, parentheses, and braces as value delimiters
541: 
542: ### Testing Query Parsing

The server logs both incoming requests and responses for debugging:
```
[DEBUG] Raw query: mutation { login(username: "admin", password: "password123") { success token } }
[LOGIN] username='admin', password='password123'
[DEBUG] Response: {"data":{"login":{"success":true,"token":"eyJhbG..."}}}
```

### Authentication Error Handling

The API returns proper HTTP status codes for authentication and authorization errors:

| Scenario | HTTP Status | Response |
|----------|-------------|----------|
| No auth token on protected endpoint | 401 | `{"errors":[{"message":"Authentication required"}]}` |
| Invalid/malformed token | 401 | `{"errors":[{"message":"Invalid token"}]}` |
| Expired token | 401 | `{"errors":[{"message":"Token expired"}]}` |
| Authenticated but insufficient permissions | 403 | `{"data":{...,"message":"You can only..."}}` |

**Error Responses by Endpoint (when not authenticated):**

| Endpoint | Error Response |
|----------|---------------|
| `cart` | `{"cart":{"message":"Authentication required"}}` |
| `orders` | `{"orders":{"message":"Authentication required"}}` |
| `myReviews` | `{"myReviews":{"message":"Authentication required"}}` |
| `webhooks` | `{"webhooks":{"message":"Authentication required"}}` |
| `updateProfile` | `{"updateProfile":{"message":"Authentication required"}}` |
| `addToCart` | `{"addToCart":{"success":false,"message":"Authentication required"}}` |
| `removeFromCart` | `{"removeFromCart":{"success":false,"message":"Authentication required"}}` |
| `createOrder` | `{"createOrder":{"success":false,"message":"Authentication required"}}` |
| `checkout` | `{"checkout":{"success":false,"message":"Authentication required"}}` |
| `cancelOrder` | `{"cancelOrder":{"success":false,"message":"Authentication required"}}` |
| `createReview` | `{"createReview":{"success":false,"message":"Authentication required"}}` |
| `deleteReview` | `{"deleteReview":{"success":false,"message":"Authentication required"}}` |
| `registerWebhook` | `{"registerWebhook":{"success":false,"message":"Authentication required"}}` |
| `testWebhook` | `{"testWebhook":{"success":false,"message":"Authentication required"}}` |

**Authorization Error Examples (403 Forbidden):**

When authenticated users try to access resources they don't own/have permission for:

| Scenario | Response |
|----------|----------|
| User tries to delete another user's review | `{"deleteReview":{"success":false,"message":"You can only delete your own reviews"}}` |
| User tries to cancel another user's order | `{"cancelOrder":{"success":false,"message":"You can only cancel your own orders"}}` |
| User tries to test another user's webhook | `{"testWebhook":{"success":false,"message":"You can only test your own webhooks"}}` |

ALWAYS test with debug logging enabled:
547: cerr << "[DEBUG] Raw body: " << body << endl;
548: cerr << "[DEBUG] Extracted query: " << queryStr << endl;
549: cerr << "[DEBUG] username='" << username << "', password='" << password << "'" << endl;
550: ```
551: 
552: If you see `username='"admin"'` (with quotes included), the parsing is broken.
553: 
554: ### Never Use Regex for This
555: 
556: Regex is fragile with escaped strings. Use the simple character-by-character parsing shown above.
557: 
558: ### If You Break This Again...
559: 
560: Symptoms to watch for:
561: - `{\"data\":{\"login\":{\"success\":false,\"message\":\"Missing required fields: username, password\"}}}`
562: - Logs show `username='"admin"'` or `username=''`
563: - Server receives `\\"` in the raw body
564: 
565: Fix: Use the extractValue() implementation above.
566: 
567: ---
568: 
569: ## Fly.io Deployment
570: 
571: ### Why Fly.io?
572: - Faster cold starts than Render
573: - Better performance for low-traffic apps
574: - Generous free tier
575: - Docker-based deployments
576: - Automatic HTTPS
577: - Edge caching
578: 
579: ### Initial Setup
580: 
581: ```bash
582: # Install flyctl
583: curl -L https://fly.io/install.sh | sh
584: export FLYCTL_INSTALL="$HOME/.fly"
585: export PATH="$FLYCTL_INSTALL/bin:$PATH"
586: 
587: # Authenticate
588: flyctl auth login
589: 
590: # Launch app (creates fly.toml and deploys)
591: ./deploy-fly.sh
592: ```
593: 
594: ### Manual Deployment
595: ```bash
596: # Deploy to Fly.io
597: fly deploy
598: 
599: # Check status
600: fly status
601: 
602: # View logs
603: fly logs
604: 
605: # Open app
606: fly open
607: ```
608: 
609: ### Environment Variables
610: Set these in Fly.io dashboard or via CLI:
611: - `DATABASE_URL`: Neon PostgreSQL connection string (postgresql://...)
612: - `JWT_SECRET`: JWT signing secret
613: 
614: Example:
615: ```bash
616: fly secrets set DATABASE_URL="postgresql://user:pass@host.neon.tech/dbname?sslmode=require"
617: fly secrets set JWT_SECRET="your-jwt-secret"
618: ```
619: 
620: ### Scale Down (Free Tier)
621: ```bash
622: # Scale to 0 machines when not in use (saves credits)
623: fly scale count 0
624: 
625: # Scale back up
626: fly scale count 1
627: ```
628: 
629: ### Troubleshooting
630: ```bash
631: # Check deployed machines
632: fly machine list
633: 
634: # Restart a machine
635: fly machine restart <machine-id>
636: 
637: # Check connection to database
638: fly ssh console
639: psql "$DATABASE_URL" -c "SELECT 1"
640: 
641: # View recent logs
642: fly logs
643: 
644: # Redeploy after config changes
645: fly deploy
646: ```
647: 
648: ### URLs After Deployment
649: - **App**: https://graphql-bookstore.fly.dev
650: - **GraphQL Playground**: https://graphql-bookstore.fly.dev/
651: - **GraphQL Endpoint**: https://graphql-bookstore.fly.dev/graphql
652: - **Health Check**: https://graphql-bookstore.fly.dev/health

### Modularization Complete
The `src/main.cpp` file has been fully modularized into smaller, feature-specific modules:

| Module | Files | Description |
|--------|-------|-------------|
| **Shared Utilities** | `utils.h`, `utils.cpp` | `escapeJson`, `WriteCallback` |
| **User Management** | `user_manager.h`, `user_manager.cpp` | User struct, JWT, auth functions |
| **Book/Author Management** | `book_manager.h`, `book_manager.cpp` | Book, Author structs, cache loading |
| **Cart/Order Management** | `order_manager.h`, `order_manager.cpp` | Cart, Order structs, order logic |
| **Review/Webhook Management** | `extra_features.h`, `extra_features.cpp` | Review, Webhook structs |
| **Database Management** | `db_manager.h`, `db_manager.cpp` | PostgreSQL connection handling |
| **GraphQL Handler** | `graphql_handler.h`, `graphql_handler.cpp` | Query/mutation handlers, JSON converters |
| **Network Manager** | `network_manager.h`, `network_manager.cpp` | `fetchURL`, `isURLWhitelisted` |
| **HTML Generator** | `html_generator.h`, `html_generator.cpp` | `generateLandingHTML`, `generatePlaygroundHTML` |
| **Rate Limiter** | `rate_limiter.h`, `rate_limiter.cpp` | Rate limiting logic |
| **Payment Handler** | `payment_handler.h`, `payment_handler.cpp` | Vulnbank card processing |

**Result**: `main.cpp` reduced from ~3359 lines to ~370 lines.

### HTTP Request Handling
The server now properly handles TCP packet fragmentation with `readFullRequest()`:

```cpp
bool readFullRequest(int clientSocket, string& request, int timeoutSec);
```

**Features**:
- Uses `select()` with 5-second timeout for non-blocking reads
- Parses `Content-Length` header to determine exact body size
- Loops until full body is received
- Handles slow networks, large payloads, and TCP fragmentation
- Prevents memory exhaustion with buffer limits
