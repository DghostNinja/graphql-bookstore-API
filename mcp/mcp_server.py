#!/usr/bin/env python3
"""
MCP Server for GraphQL Bookstore API
Allows AI/LLM models to interact with the Bookstore GraphQL API via MCP protocol.
"""

import os
import json
import requests
from typing import Any, Optional
from contextlib import contextmanager

API_URL = os.environ.get("API_URL", "http://localhost:4000/graphql")
DEFAULT_USERNAME = "admin"
DEFAULT_PASSWORD = "password123"


class BookstoreMCPClient:
    """Client for interacting with the GraphQL Bookstore API."""
    
    def __init__(self, api_url: str = API_URL):
        self.api_url = api_url
        self.token: Optional[str] = None
        self.session = requests.Session()
        self.session.headers.update({"Content-Type": "application/json"})
    
    def _execute(self, query: str, variables: Optional[dict] = None) -> dict:
        """Execute a GraphQL query."""
        body = {"query": query}
        if variables:
            body["variables"] = variables
        
        headers = {}
        if self.token:
            headers["Authorization"] = f"Bearer {self.token}"
        
        response = self.session.post(
            self.api_url,
            json=body,
            headers=headers
        )
        response.raise_for_status()
        return response.json()
    
    def login(self, username: str, password: str) -> dict:
        """Login and store JWT token."""
        query = '''
        mutation Login($username: String!, $password: String!) {
            login(username: $username, password: $password) {
                success
                token
                message
            }
        }
        '''
        result = self._execute(query, {"username": username, "password": password})
        if result.get("data", {}).get("login", {}).get("success"):
            self.token = result["data"]["login"]["token"]
        return result
    
    def register(self, username: str, first_name: str, last_name: str, password: str) -> dict:
        """Register a new user."""
        query = '''
        mutation Register($username: String!, $firstName: String!, $lastName: String!, $password: String!) {
            register(username: $username, firstName: $firstName, lastName: $lastName, password: $password) {
                success
                message
                userId
            }
        }
        '''
        return self._execute(query, {
            "username": username,
            "firstName": first_name,
            "lastName": last_name,
            "password": password
        })
    
    def me(self) -> dict:
        """Get current authenticated user."""
        query = '''
        query Me {
            me {
                id
                username
                firstName
                lastName
                role
            }
        }
        '''
        return self._execute(query)
    
    def books(self, search: str = "", category_id: int = 0) -> dict:
        """List all books with optional search."""
        query = '''
        query Books($search: String, $categoryId: Int) {
            books(search: $search, categoryId: $categoryId) {
                id
                title
                author
                price
                category
                description
            }
        }
        '''
        return self._execute(query, {"search": search, "categoryId": category_id})
    
    def book(self, book_id: int) -> dict:
        """Get a specific book by ID."""
        query = '''
        query Book($id: Int!) {
            book(id: $id) {
                id
                title
                author
                price
                category
                description
                stock
            }
        }
        '''
        return self._execute(query, {"id": book_id})
    
    def cart(self) -> dict:
        """Get user's shopping cart."""
        query = '''
        query Cart {
            cart {
                id
                items {
                    bookId
                    title
                    quantity
                    price
                }
                subtotal
                tax
                total
            }
        }
        '''
        return self._execute(query)
    
    def add_to_cart(self, book_id: int, quantity: int = 1) -> dict:
        """Add item to shopping cart."""
        query = '''
        mutation AddToCart($bookId: Int!, $quantity: Int!) {
            addToCart(bookId: $bookId, quantity: $quantity) {
                success
                message
            }
        }
        '''
        return self._execute(query, {"bookId": book_id, "quantity": quantity})
    
    def remove_from_cart(self, book_id: int) -> dict:
        """Remove item from shopping cart."""
        query = '''
        mutation RemoveFromCart($bookId: Int!) {
            removeFromCart(bookId: $bookId) {
                success
                message
            }
        }
        '''
        return self._execute(query, {"bookId": book_id})
    
    def orders(self) -> dict:
        """Get user's orders."""
        query = '''
        query Orders {
            orders {
                id
                status
                totalAmount
                createdAt
                items {
                    bookId
                    title
                    quantity
                    price
                }
            }
        }
        '''
        return self._execute(query)
    
    def create_order(self) -> dict:
        """Create order from cart."""
        query = '''
        mutation CreateOrder {
            createOrder {
                success
                orderId
                totalAmount
                message
            }
        }
        '''
        return self._execute(query)
    
    def checkout(self, card_number: str, expiry: str, cvv: str) -> dict:
        """Process checkout with payment."""
        query = '''
        mutation Checkout($cardNumber: String!, $expiry: String!, $cvv: String!) {
            checkout(cardNumber: $cardNumber, expiry: $expiry, cvv: $cvv) {
                success
                orderId
                message
            }
        }
        '''
        return self._execute(query, {
            "cardNumber": card_number,
            "expiry": expiry,
            "cvv": cvv
        })
    
    def cancel_order(self, order_id: str) -> dict:
        """Cancel an order."""
        query = '''
        mutation CancelOrder($orderId: String!) {
            cancelOrder(orderId: $orderId) {
                success
                message
            }
        }
        '''
        return self._execute(query, {"orderId": order_id})
    
    def create_review(self, book_id: int, rating: int, comment: str) -> dict:
        """Create a book review."""
        query = '''
        mutation CreateReview($bookId: Int!, $rating: Int!, $comment: String!) {
            createReview(bookId: $bookId, rating: $rating, comment: $comment) {
                success
                reviewId
                message
            }
        }
        '''
        return self._execute(query, {
            "bookId": book_id,
            "rating": rating,
            "comment": comment
        })
    
    def delete_review(self, review_id: int) -> dict:
        """Delete a review."""
        query = '''
        mutation DeleteReview($reviewId: Int!) {
            deleteReview(reviewId: $reviewId) {
                success
                message
            }
        }
        '''
        return self._execute(query, {"reviewId": review_id})
    
    def book_reviews(self, book_id: int) -> dict:
        """Get reviews for a book."""
        query = '''
        query BookReviews($bookId: Int!) {
            bookReviews(bookId: $bookId) {
                id
                userId
                rating
                comment
                createdAt
            }
        }
        '''
        return self._execute(query, {"bookId": book_id})
    
    def my_reviews(self) -> dict:
        """Get current user's reviews."""
        query = '''
        query MyReviews {
            myReviews {
                id
                bookId
                rating
                comment
                createdAt
            }
        }
        '''
        return self._execute(query)
    
    def update_profile(self, first_name: str = None, last_name: str = None) -> dict:
        """Update user profile."""
        query = '''
        mutation UpdateProfile($firstName: String, $lastName: String) {
            updateProfile(firstName: $firstName, lastName: $lastName) {
                success
                message
            }
        }
        '''
        return self._execute(query, {
            "firstName": first_name,
            "lastName": last_name
        })
    
    def search_books(self, query: str) -> dict:
        """Search for books."""
        return self.books(search=query)
    
    def apply_coupon(self, code: str) -> dict:
        """Apply coupon code to cart."""
        query = '''
        mutation ApplyCoupon($code: String!) {
            applyCoupon(code: $code) {
                success
                message
                discount
            }
        }
        '''
        return self._execute(query, {"code": code})
    
    def register_webhook(self, url: str, events: list, secret: str = "") -> dict:
        """Register a webhook."""
        query = '''
        mutation RegisterWebhook($url: String!, $events: [String]!, $secret: String) {
            registerWebhook(url: $url, events: $events, secret: $secret) {
                success
                webhookId
                message
            }
        }
        '''
        return self._execute(query, {
            "url": url,
            "events": events,
            "secret": secret
        })
    
    def webhooks(self) -> dict:
        """Get user's webhooks."""
        query = '''
        query Webhooks {
            webhooks {
                id
                url
                events
                createdAt
            }
        }
        '''
        return self._execute(query)
    
    def test_webhook(self, webhook_id: int) -> dict:
        """Test a webhook."""
        query = '''
        mutation TestWebhook($webhookId: Int!) {
            testWebhook(webhookId: $webhookId) {
                success
                message
            }
        }
        '''
        return self._execute(query, {"webhookId": webhook_id})
    
    def admin_stats(self) -> dict:
        """Get admin statistics."""
        query = '''
        query AdminStats {
            _adminStats {
                userCount
                bookCount
                totalRevenue
            }
        }
        '''
        return self._execute(query)
    
    def admin_orders(self) -> dict:
        """Get all orders (admin)."""
        query = '''
        query AdminOrders {
            _adminAllOrders {
                id
                userId
                status
                totalAmount
                createdAt
            }
        }
        '''
        return self._execute(query)
    
    def admin_payments(self) -> dict:
        """Get all payments (admin)."""
        query = '''
        query AdminPayments {
            _adminAllPayments {
                id
                orderId
                amount
                status
                createdAt
            }
        }
        '''
        return self._execute(query)
    
    def pro_inventory(self) -> dict:
        """Get pro/hidden inventory."""
        query = '''
        query ProInventory {
            _proInventory {
                id
                title
                author
                price
                difficulty
                hint
            }
        }
        '''
        return self._execute(query)


_client: Optional[BookstoreMCPClient] = None


def get_client() -> BookstoreMCPClient:
    """Get or create the MCP client."""
    global _client
    if _client is None:
        _client = BookstoreMCPClient()
    return _client


def login(username: str = DEFAULT_USERNAME, password: str = DEFAULT_PASSWORD) -> dict:
    """Login to the bookstore API."""
    client = get_client()
    return client.login(username, password)


def logout() -> dict:
    """Logout (clears token locally)."""
    client = get_client()
    client.token = None
    return {"success": True, "message": "Logged out locally"}


def register(username: str, first_name: str, last_name: str, password: str) -> dict:
    """Register a new user."""
    client = get_client()
    return client.register(username, first_name, last_name, password)


def me() -> dict:
    """Get current authenticated user."""
    client = get_client()
    return client.me()


def books(search: str = "", category_id: int = 0) -> dict:
    """List all books with optional search."""
    client = get_client()
    return client.books(search, category_id)


def book(book_id: int) -> dict:
    """Get a specific book by ID."""
    client = get_client()
    return client.book(book_id)


def cart() -> dict:
    """Get user's shopping cart."""
    client = get_client()
    return client.cart()


def add_to_cart(book_id: int, quantity: int = 1) -> dict:
    """Add item to shopping cart."""
    client = get_client()
    return client.add_to_cart(book_id, quantity)


def remove_from_cart(book_id: int) -> dict:
    """Remove item from shopping cart."""
    client = get_client()
    return client.remove_from_cart(book_id)


def orders() -> dict:
    """Get user's orders."""
    client = get_client()
    return client.orders()


def create_order() -> dict:
    """Create order from cart."""
    client = get_client()
    return client.create_order()


def purchase_cart(card_number: str = "4111111111111111", expiry: str = "12/25", cvv: str = "123") -> dict:
    """Purchase cart with payment."""
    client = get_client()
    return client.purchase_cart(card_number, expiry, cvv)


def cancel_order(order_id: str) -> dict:
    """Cancel an order."""
    client = get_client()
    return client.cancel_order(order_id)


def create_review(book_id: int, rating: int, comment: str) -> dict:
    """Create a book review."""
    client = get_client()
    return client.create_review(book_id, rating, comment)


def delete_review(review_id: int) -> dict:
    """Delete a review."""
    client = get_client()
    return client.delete_review(review_id)


def book_reviews(book_id: int) -> dict:
    """Get reviews for a book."""
    client = get_client()
    return client.book_reviews(book_id)


def my_reviews() -> dict:
    """Get current user's reviews."""
    client = get_client()
    return client.my_reviews()


def update_profile(first_name: str = None, last_name: str = None) -> dict:
    """Update user profile."""
    client = get_client()
    return client.update_profile(first_name, last_name)


def search_books(query: str) -> dict:
    """Search for books by title or author."""
    client = get_client()
    return client.search_books(query)


def apply_coupon(code: str) -> dict:
    """Apply coupon code to cart."""
    client = get_client()
    return client.apply_coupon(code)


def register_webhook(url: str, events: list, secret: str = "") -> dict:
    """Register a webhook URL."""
    client = get_client()
    return client.register_webhook(url, events, secret)


def webhooks() -> dict:
    """Get user's webhooks."""
    client = get_client()
    return client.webhooks()


def test_webhook(webhook_id: int) -> dict:
    """Test a webhook."""
    client = get_client()
    return client.test_webhook(webhook_id)


def logout() -> dict:
    """Logout and clear token."""
    client = get_client()
    return client.logout()


def admin_stats() -> dict:
    """Get admin statistics."""
    client = get_client()
    return client.admin_stats()


def admin_orders() -> dict:
    """Get all orders (admin)."""
    client = get_client()
    return client.admin_orders()


def admin_payments() -> dict:
    """Get all payments (admin)."""
    client = get_client()
    return client.admin_payments()


def pro_inventory() -> dict:
    """Get pro/hidden inventory (expert-level books)."""
    client = get_client()
    return client.pro_inventory()


if __name__ == "__main__":
    import sys
    
    client = get_client()
    print("Bookstore MCP Client")
    print("=" * 40)
    print(f"API URL: {client.api_url}")
    print("\nLogging in with default credentials...")
    
    result = client.login(DEFAULT_USERNAME, DEFAULT_PASSWORD)
    if result.get("data", {}).get("login", {}).get("success"):
        print("Logged in successfully!")
        me = client.me()
        print(f"User: {me.get('data', {}).get('me', {}).get('username')}")
        
        books = client.books()
        book_list = books.get("data", {}).get("books", [])
        print(f"Available books: {len(book_list)}")
    else:
        print("Login failed:", result)
        sys.exit(1)
