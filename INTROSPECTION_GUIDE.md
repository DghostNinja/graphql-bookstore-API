# GraphQL Introspection Queries - Vulnerable Bookstore API

## ⚠️ Note

This server has **partial introspection support**. The queries below show you the available endpoints, but the schema is implemented manually (not using a full GraphQL schema library).

## Correct Introspection Queries

### 1. Get All Available Queries

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
  }
}
```

### 2. Get All Mutations

```graphql
{
  __schema {
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

### 3. Get All Types

```graphql
{
  __schema {
    types {
      name
      kind
    }
  }
}
```

## Available Queries (What Actually Works)

### Public Queries (No Authentication Required)
- `books` - List all books
- `book(id: ID!)` - Get specific book
- `_internalUserSearch(username: String!)` - ⚠️ VULNERABLE: Searches all users
- `_fetchExternalResource(url: String!)` - ⚠️ VULNERABLE: SSRF endpoint

### Authenticated Queries (Requires JWT Token)
- `me` - Get current user info

### All Mutations
- `register(username, firstName, lastName, password)` - Create new user
- `login(username, password)` - Authenticate and get JWT token
- `updateProfile(...)` - ⚠️ VULNERABLE: Mass assignment

## Working Examples

### List Books (No Auth)
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

### Get Current User (Requires Auth)
```graphql
query {
  me {
    id
    username
    role
    passwordHash  # ⚠️ Excessive data exposure!
  }
}
```

### Search Users (BOLA Vulnerability)
```graphql
query {
  _internalUserSearch(username: "") {
    id
    username
    passwordHash  # ⚠️ Password hash exposed!
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

### Register New User
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

### Mass Assignment (Privilege Escalation)
```graphql
mutation {
  updateProfile(
    username: "currentuser",
    role: "admin"  # ⚠️ Escalates privileges!
  ) {
    id
    username
    role
  }
}
```

## Important Notes

1. **Introspection is limited** - This is a custom GraphQL implementation, not a full schema
2. **No schema validation** - The server accepts any query structure
3. **Vulnerable by design** - Internal endpoints and sensitive data are exposed
4. **Field selection works** - You can specify which fields to return in responses

## Default Test Users

| Username | Password | Role |
|----------|----------|------|
| admin | password123 | admin |
| staff | password123 | staff |
| user | password123 | user |

## Using the Playground

1. Open http://localhost:4000/
2. Use the query tester textarea
3. Enter your GraphQL query
4. Click "Execute Query"
5. View the response

## Common Issues

### "Unknown error" or empty response
- Make sure all required fields are provided
- Check that the server is running: `ps aux | grep bookstore-server`
- Test via curl first to isolate browser issues

### Introspection not working
- This server has simplified introspection
- Use the examples above instead of standard introspection queries
- The schema is documented in the code and this file
