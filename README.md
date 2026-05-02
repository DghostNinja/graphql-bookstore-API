# GraphQL Bookstore API - Security Learning Environment

A realistic GraphQL API server built in C++ designed for security education and practice, covering all OWASP API Security Top 10 vulnerabilities.

## Features

- **Full Database Integration** - PostgreSQL with real user/book/order management
- **JWT Authentication** - Token-based auth with role-based access control (user, staff, admin)
- **Username-based Login** - Register and login using username/password (no email required)
- **SSRF with Whitelisted URLs** - Real HTTP fetching to allowed endpoints
- **Field-Level Response Control** - Specify exactly which fields you want returned
- **Interactive Playground** - Built-in web UI for testing queries and mutations
- **Complete CRUD Operations** - Users, books, orders, reviews, coupons
- **All OWASP API Security Top 10 vulnerabilities** implemented realistically

## Overview

### MCP Server - AI/LLM Integration

The Bookstore API includes an MCP (Model Context Protocol) server that exposes all API operations as tools for AI and LLM models.

#### Quick Installation

```bash
cd mcp
npm install
```

#### Usage

**Local Development:**
```bash
# Start the Bookstore API server (from project root)
./bookstore-server

# In another terminal, start MCP server
cd mcp
npm start
```

**Production (Live API):**
```bash
cd mcp
API_URL=https://api.graphqlbook.org/graphql npm start
```

#### Claude Desktop Integration

Add to your Claude Desktop config:

```json
{
  "mcpServers": {
    "bookstore": {
      "command": "node",
      "args": ["/path/to/GraphQL-Bookstore/mcp/mcp_server.mjs"],
      "env": {
        "API_URL": "http://localhost:4000/graphql"
      }
    }
  }
}
```

See `mcp/README.md` for detailed documentation.

## Installation & Setup

### Prerequisites

- **Build Tools**: g++, make
- **Libraries**: libpq-dev, libssl-dev, libjwt-dev, libcurl4-openssl-dev
- **Database**: PostgreSQL (local or Docker)
- **Optional**: Docker & Docker Compose

### Option 1: Docker (Recommended)

```bash
# Build and run with docker-compose
docker compose up -d

# Server: http://localhost:4000
# Database: localhost:5433 (Postgres 16)
```

### Verify Setup

Open http://localhost:4000 in your browser, or run:

```bash
curl -s http://localhost:4000/health
```

Expected response: `OK`

### Option 2: Direct Build

```bash
# Install dependencies
sudo apt-get install build-essential libpq-dev libssl-dev libjwt-dev libcurl4-openssl-dev

# Build server
./build.sh

# Initialize database (requires sudo/postgres access)
sudo -u postgres psql -f scripts/init_database.sql

# Run server
./bookstore-server
```


## API Usage

### Accessing the API

- **Local URL**: http://localhost:4000/graphql
- **Live URL**: https://api.graphqlbook.org/graphql

### Default Users

All users have password: **password123**

| Username | Role | Description |
|----------|-------|-------------|
| admin    | admin | Full system access |
| staff    | staff | Can manage users and orders |
| user     | user  | Regular customer access |

### Registration

```graphql
mutation {
  register(username: "newuser", firstName: "John", lastName: "Doe", password: "mypassword") {
    success
    message
    token
    user {
      id
      username
      role
    }
  }
}
```

### Login

```graphql
mutation {
  login(username: "admin", password: "password123") {
    success
    message
    token
    user {
      id
      username
      role
    }
  }
}
```


## OWASP API Security Top 10 Coverage

This environment includes realistic implementations of all OWASP API Security Top 10 vulnerabilities:

1. **Broken Object Level Authorization (BOLA)**
2. **Broken User Authentication**
3. **Excessive Data Exposure**
4. **Lack of Resources & Rate Limiting**
5. **Broken Object Property Level Authorization**
6. **Mass Assignment**
7. **Security Misconfiguration**
8. **Injection**
9. **Improper Assets Management**
10. **Insufficient Logging & Monitoring**

**Bonus Vulnerabilities:**
- **Server-Side Request Forgery (SSRF)** - HTTP fetching to whitelisted URLs
- **Business Logic Flaws** - Coupon abuse, refund bypass, inventory manipulation

## Project Structure

```
.
├── src/                      # Source code
│   ├── main.cpp              # Main server implementation
│   ├── user_manager.cpp      # User authentication & management
│   ├── book_manager.cpp      # Book & author management
│   ├── order_manager.cpp     # Cart & order management
│   ├── extra_features.cpp    # Reviews & webhooks
│   ├── graphql_handler.cpp   # GraphQL query/mutation handlers
│   ├── db_manager.cpp        # Database connection handling
│   ├── network_manager.cpp   # HTTP fetching (SSRF)
│   ├── html_generator.cpp    # Web UI generation
│   ├── rate_limiter.cpp      # Rate limiting logic
│   └── payment_handler.cpp   # Payment processing
├── scripts/
│   ├── init_database.sql     # Database schema
│   └── seed.sql              # Seed data (books, users)
├── mcp/                      # MCP server for AI/LLM integration
├── build.sh                  # Build script
├── docker-compose.yml        # Docker deployment
├── Dockerfile                # Docker image
├── test_api.sh              # Security test suite
└── flow.sh                   # User flow test suite
```

## Deployment

### Production Readiness

**This is a DELIBERATELY VULNERABLE API. DO NOT USE IN PRODUCTION.**

To make this production-ready, you would need to:
1. Fix all OWASP API Security Top 10 vulnerabilities
2. Add rate limiting and query depth restrictions
3. Implement proper password hashing with bcrypt
4. Add input validation and sanitization
5. Remove debug endpoints and disable introspection
6. Add comprehensive logging and monitoring
7. Implement proper authorization checks on all endpoints
8. Use strong, rotated JWT secrets with expiration
9. Add HTTPS/TLS support
10. Implement request size limits

## Support This Project

If you find this API useful for learning GraphQL security, consider supporting its hosting costs:

[![Donate via Flutterwave](https://img.shields.io/badge/Donate-Support%20the%20Server-green?style=for-the-badge)](https://flutterwave.com/pay/r7srcerzvxsw)

Any amount helps keep this API running for security researchers and students.

## Contributing

Contributions are welcome! This is a learning environment where the structure and patterns can be adapted for production use with proper security measures.

### Contributors

- [kizerh](https://github.com/kizerh)

## License

Educational use only.
