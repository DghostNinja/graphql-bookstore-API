FROM debian:bookworm-slim

# Install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    g++ \
    make \
    libpq-dev \
    libssl-dev \
    libcurl4-openssl-dev \
    libjwt-dev \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source code FIRST (for better Docker layer caching)
COPY src/ src/
COPY include/ include/
COPY graphql.json .
COPY scripts/ scripts/
COPY build.sh .
COPY *.sh .

# Build the server - fail if build fails
RUN set -e && \
    echo "=== Building Bookstore Server ===" && \
    g++ -std=c++17 -I include -pthread -o bookstore-server src/main.cpp \
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
        -lpq -ljwt -lcurl -lssl -lcrypto \
        && echo "=== Build SUCCESS ===" \
        && ls -la bookstore-server

# Verify binary exists and is executable
RUN test -x /app/bookstore-server && echo "Binary is executable"

EXPOSE 4000

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:4000/ || exit 1

CMD ["./bookstore-server"]