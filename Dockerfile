FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libpq-dev \
    libssl-dev \
    libcurl4-openssl-dev \
    libjwt-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY src/ src/
COPY graphql.json .
COPY scripts/ scripts/

RUN g++ -std=c++17 -pthread -o bookstore-server src/*.cpp -lpq -ljwt -lcurl -lssl -lcrypto

EXPOSE 4000

CMD ["./bookstore-server"]
