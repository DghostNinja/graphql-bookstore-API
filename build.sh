#!/bin/bash

set -e

echo "========================================"
echo "  GraphQL Bookstore API                 "
echo "  Build & Setup Script                 "
echo "========================================"

echo ""
echo "[1/5] Checking dependencies..."
if ! command -v g++ >/dev/null 2>&1; then
    echo "  ERROR: g++ not found. Install build-essential."
    exit 1
fi
if ! pkg-config --exists libpq 2>/dev/null; then
    echo "  WARNING: libpq-dev may not be installed"
fi
echo "  Dependencies OK"

echo ""
echo "[2/5] Building server..."
g++ -std=c++17 -I./include -I/usr/include/postgresql -pthread -o bookstore-server \
    src/main.cpp \
    src/utils.cpp \
    src/user_manager.cpp \
    src/book_manager.cpp \
    src/order_manager.cpp \
    src/extra_features.cpp \
    src/db_manager.cpp \
    src/network_manager.cpp \
    src/graphql_handler.cpp \
    src/html_generator.cpp \
    src/rate_limiter.cpp \
    src/payment_handler.cpp \
    src/database/connection.cpp \
    -lpq -ljwt -lcurl -lssl -lcrypto
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi
echo "  Build OK"

echo ""
echo "[3/5] Docker database check..."
CONTAINER=$(docker ps --format "{{.Names}}" 2>/dev/null | grep -E "postgres" | head -1)
if [ -n "$CONTAINER" ]; then
    echo "  Docker DB found: $CONTAINER"
else
    echo "  No Docker database running"
fi

echo ""
echo "========================================"
echo "  Build complete!                     "
echo "========================================"
echo ""
echo "Run options:"
echo "  docker-compose up -d          # Use Docker database (recommended)"
echo "  ./bookstore-server        # Run binary (needs local Postgres)"
echo ""
echo "Server at: http://localhost:4000/"
echo "Default: admin/password123, staff/password123, user/password123"
echo ""