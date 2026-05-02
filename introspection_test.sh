#!/bin/bash

echo "Quick Introspection Query"
echo "=========================="
echo ""
echo "Try this exact query in the Playground:"
echo ""
cat << 'EOF'
query IntrospectionQuery {
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
EOF

echo ""
echo ""
echo "Or use curl:"
echo 'curl -X POST http://localhost:4000/graphql \\
  -H "Content-Type: application/json" \\
  -d '"'"'{"query": "query { __schema { queryType { name fields { name description } } mutationType { name fields { name description } } } }"}'"'"'