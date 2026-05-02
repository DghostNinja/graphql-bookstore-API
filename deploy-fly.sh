#!/bin/bash

echo "=== Deploying to Fly.io ==="

# Check if flyctl is installed
if ! command -v flyctl &> /dev/null; then
    echo "Installing flyctl..."
    curl -L https://fly.io/install.sh | sh
    export FLYCTL_INSTALL="$HOME/.fly"
    export PATH="$FLYCTL_INSTALL/bin:$PATH"
fi

# Authenticate (if not already authenticated)
if ! flyctl auth whoami &> /dev/null; then
    echo "Please run: flyctl auth login"
    exit 1
fi

# Launch app (creates if not exists)
echo "Launching app..."
fly launch --no-deploy --name graphql-bookstore

# Set secrets
echo "Setting secrets..."
fly secrets set JWT_SECRET="CHANGE_ME_IN_PRODUCTION_real_jwt_secret_key_2024"

# If DATABASE_URL is set, use it
if [ -n "$DATABASE_URL" ]; then
    fly secrets set DATABASE_URL="$DATABASE_URL"
fi

# Deploy
echo "Deploying..."
fly deploy

echo ""
echo "=== Deployment Complete ==="
echo "App URL: https://graphql-bookstore.fly.dev"
