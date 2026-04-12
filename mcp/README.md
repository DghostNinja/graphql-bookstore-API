# Bookstore MCP Server

MCP (Model Context Protocol) server that exposes the Bookstore GraphQL API as tools for AI/LLM models.

## Prerequisites

### 1. Install Node.js
```bash
# On Ubuntu/Debian
curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
sudo apt-get install -y nodejs

# On macOS
brew install node@18

# Verify installation
node --version  # Should be v18.x or higher
```

### 2. Install Dependencies
```bash
cd mcp
npm install
```

### 3. Start the Bookstore API Server
The MCP server connects to the Bookstore GraphQL API. You have two options:

#### Option A: Local Development
Run the API server locally:
```bash
# From the project root
./bookstore-server
```
The API runs on `http://localhost:4000/graphql` by default.

#### Option B: Production
Use the live API endpoint:
```
https://api.graphqlbook.org/graphql
```

## Installation

### Option A: Local Development
```bash
cd mcp
npm install

# Run with local API (default)
npm start
```

### Option B: Production
```bash
cd mcp
npm install

# Run with production API
API_URL=https://api.graphqlbook.org/graphql npm start
```

### Option C: Claude Desktop Integration
1. Copy `mcp_config.json` to your Claude Desktop config:
   - macOS: `~/Library/Application Support/Claude/claude_desktop_config.json`
   - Linux: `~/.config/Claude/claude_desktop_config.json`
   - Windows: `%APPDATA%/Claude/claude_desktop_config.json`

2. Or add to existing config:

**Local:**
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

**Production:**
```json
{
  "mcpServers": {
    "bookstore": {
      "command": "node",
      "args": ["/path/to/GraphQL-Bookstore/mcp/mcp_server.mjs"],
      "env": {
        "API_URL": "https://api.graphqlbook.org/graphql"
      }
    }
  }
}
  }
}
```

3. Restart Claude Desktop

## Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `API_URL` | `http://localhost:4000/graphql` | Bookstore API endpoint |

## Available Tools

### Authentication
| Tool | Description |
|------|-------------|
| `bookstore_login` | Login with username and password |
| `bookstore_register` | Register a new user account |
| `bookstore_me` | Get current user profile |

### Books
| Tool | Description |
|------|-------------|
| `bookstore_books` | List all books with optional search |
| `bookstore_book` | Get a specific book by ID |
| `bookstore_search` | Search books by title or author |

### Shopping Cart
| Tool | Description |
|------|-------------|
| `bookstore_cart` | Get current user shopping cart |
| `bookstore_add_to_cart` | Add a book to cart |
| `bookstore_remove_from_cart` | Remove a book from cart |

### Orders
| Tool | Description |
|------|-------------|
| `bookstore_orders` | Get user order history |
| `bookstore_create_order` | Create order from cart |
| `bookstore_purchase_cart` | Purchase cart with payment |
| `bookstore_cancel_order` | Cancel an order |

### Reviews
| Tool | Description |
|------|-------------|
| `bookstore_create_review` | Create a book review |
| `bookstore_delete_review` | Delete a review |
| `bookstore_book_reviews` | Get reviews for a book |
| `bookstore_my_reviews` | Get current user reviews |

### Profile
| Tool | Description |
|------|-------------|
| `bookstore_update_profile` | Update user profile |

### Coupons
| Tool | Description |
|------|-------------|
| `bookstore_apply_coupon` | Apply coupon code to cart |

### Webhooks
| Tool | Description |
|------|-------------|
| `bookstore_register_webhook` | Register a webhook URL |
| `bookstore_webhooks` | List user webhooks |
| `bookstore_test_webhook` | Test a webhook |

### Admin
| Tool | Description |
|------|-------------|
| `bookstore_admin_stats` | Get admin statistics |
| `bookstore_admin_orders` | Get all orders |
| `bookstore_admin_payments` | Get all payments |

### Special
| Tool | Description |
|------|-------------|
| `bookstore_pro_inventory` | Get hidden inventory |

## Usage Examples

### Login
```json
{
  "username": "admin",
  "password": "password123"
}
```

### Get Books
```json
{
  "search": "",
  "categoryId": 0
}
```

### Get Single Book
```json
{
  "id": 1
}
```

### Add to Cart
```json
{
  "bookId": 1,
  "quantity": 2
}
```

### Create Review
```json
{
  "bookId": 1,
  "rating": 5,
  "comment": "Great book!"
}
```

### Purchase Cart
```json
{
  "cardNumber": "4111111111111111",
  "expiry": "12/25",
  "cvv": "123"
}
```

## Testing

### Test MCP Server
```bash
cd mcp
echo '{"jsonrpc":"2.0","id":1,"method":"tools/list"}' | npm start
```

### Test with API
```bash
# Login first
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  -d '{"query":"mutation { login(username: \"admin\", password: \"password123\") { success token } }"}'

# Get books
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  -d '{"query":"query { books { id title author price } }"}'
```

## Troubleshooting

### Connection Refused
- Ensure the Bookstore API server is running: `./bookstore-server`
- Check API_URL environment variable

### Module Not Found
- Run `npm install` in the mcp directory

### Permission Denied
- Make the script executable: `chmod +x mcp_server.mjs`

## Default Credentials

| Username | Password | Role |
|----------|----------|-------|
| admin    | password123 | admin |
| staff    | password123 | staff |
| user     | password123 | user |
