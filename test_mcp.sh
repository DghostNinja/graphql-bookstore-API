#!/bin/bash

set -e

API_URL="${API_URL:-http://localhost:4000/graphql}"

if [[ "$API_URL" == *"localhost"* ]]; then
    API_URL="${API_URL/localhost/127.0.0.1}"
fi
MCP_DIR="$(cd "$(dirname "$0")/mcp" && pwd)"

run_mcp() {
    echo "$1" | API_URL="$API_URL" node mcp_server.mjs 2>&1 | grep -v "Bookstore MCP Server"
}

echo "=========================================="
echo "  MCP Server Test Suite"
echo "=========================================="
echo ""
echo "API URL: $API_URL"
echo ""

cd "$MCP_DIR"

echo "[1/7] Checking Node.js..."
if ! command -v node &> /dev/null; then
    echo "ERROR: Node.js is not installed"
    exit 1
fi
echo "  Node.js version: $(node --version)"
echo "  PASS"

echo ""
echo "[2/7] Checking MCP dependencies..."
if [ ! -d "node_modules" ]; then
    echo "  Installing dependencies..."
    npm install --silent 2>/dev/null || npm install
fi
echo "  PASS"

echo ""
echo "[3/7] Testing tools/list..."
run_mcp '{"jsonrpc":"2.0","id":1,"method":"tools/list"}' | grep -q 'tools' && echo "  PASS" || { echo "  ERROR"; exit 1; }

echo ""
echo "[4/7] Testing bookstore_books..."
run_mcp '{"jsonrpc":"2.0","id":2,"method":"tools/call","params":{"name":"bookstore_books","arguments":{"search":"","categoryId":0}}}' | grep -q 'books' && echo "  PASS" || { echo "  ERROR"; exit 1; }

echo ""
echo "[5/7] Testing bookstore_login..."
run_mcp '{"jsonrpc":"2.0","id":3,"method":"tools/call","params":{"name":"bookstore_login","arguments":{"username":"admin","password":"password123"}}}' | grep -q 'success' && echo "  PASS" || { echo "  ERROR"; exit 1; }

echo ""
echo "[6/7] Testing bookstore_admin_stats..."
run_mcp '{"jsonrpc":"2.0","id":4,"method":"tools/call","params":{"name":"bookstore_admin_stats","arguments":{}}}' | grep -q 'userCount' && echo "  PASS" || { echo "  ERROR"; exit 1; }

echo ""
echo "[7/7] Testing bookstore_search..."
if run_mcp '{"jsonrpc":"2.0","id":5,"method":"tools/call","params":{"name":"bookstore_search","arguments":{"query":"Clean"}}}' | grep -q 'Clean Code'; then
    echo "  PASS"
else
    echo "  ERROR"
    exit 1
fi

echo ""
echo "=========================================="
echo "  All MCP Tests Passed!"
echo "=========================================="
