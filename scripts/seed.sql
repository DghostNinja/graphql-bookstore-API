-- Seed data for bookstore database (idempotent)
-- Run this after init_database.sql to add sample data

-- Clear existing data (in reverse order due to foreign keys)
DELETE FROM order_items;
DELETE FROM orders;
DELETE FROM cart_items;
DELETE FROM shopping_carts;
DELETE FROM reviews;
DELETE FROM webhooks;
DELETE FROM search_logs;
DELETE FROM audit_logs;
DELETE FROM payment_transactions;
DELETE FROM coupons;
DELETE FROM coupon_usage;
DELETE FROM hidden_books;
DELETE FROM books;
DELETE FROM categories;
DELETE FROM authors;
DELETE FROM system_config;
DELETE FROM users;

-- Reset sequences to start from 1
ALTER SEQUENCE authors_id_seq RESTART WITH 1;
ALTER SEQUENCE categories_id_seq RESTART WITH 1;
ALTER SEQUENCE books_id_seq RESTART WITH 1;

-- Insert default users (plaintext passwords for educational server)
INSERT INTO users (username, password_hash, first_name, last_name, role) VALUES
('admin', 'password123', 'Admin', 'User', 'admin'),
('staff', 'password123', 'Staff', 'Member', 'staff'),
('user', 'password123', 'Regular', 'User', 'user'),
('testuser', 'testpass123', 'Test', 'User', 'user');

-- Insert sample categories
INSERT INTO categories (name, description) VALUES
('Fiction', 'Fictional literature'),
('Non-Fiction', 'Non-fictional works'),
('Science Fiction', 'Science fiction and fantasy'),
('Mystery', 'Mystery and thriller novels'),
('Biography', 'Biographical works'),
('Technical', 'Technical and programming books');

-- Insert sample authors
INSERT INTO authors (name, bio) VALUES
('John Smith', 'Bestselling fiction author'),
('Jane Doe', 'Technical writer and developer'),
('Bob Johnson', 'Science fiction novelist'),
('Alice Williams', 'Mystery thriller writer');

-- Insert sample books
INSERT INTO books (isbn, title, description, author_id, category_id, price, stock_quantity, is_featured, is_bestseller) VALUES
('9780132350884', 'Clean Code', 'A handbook of agile software craftsmanship', 2, 6, 42.99, 25, true, true),
('9780201633610', 'Design Patterns', 'Elements of Reusable Object-Oriented Software', 2, 6, 54.99, 15, true, false),
('9780321125217', 'Domain-Driven Design', 'Tackling Complexity in Heart of Software', 2, 6, 49.99, 20, false, false),
('9780735619678', 'Code Complete', 'A Practical Handbook of Software Construction', 2, 6, 39.99, 30, false, false),
('9780345391803', 'The Hitchhiker''s Guide to the Galaxy', 'A sci-fi comedy classic', 3, 3, 14.99, 50, true, true),
('9780345391802', 'The Restaurant at the End of the Universe', 'Second book in trilogy', 3, 3, 14.99, 45, false, false);

-- Insert hidden books (for pro-level challenges)
INSERT INTO hidden_books (book_key, isbn, title, description, author_id, category_id, price, stock_quantity, content, difficulty_level, hint_text) VALUES
('quantum_cryptography', '9781593277501', 'Quantum Cryptography: The Next Frontier', 'Advanced quantum computing security', 2, 6, 299.99, 3, 'HIDDEN_CONTENT: Quantum key distribution protocols and post-quantum cryptography techniques. Contains classified research.', 'master', 'Look for patterns in the API rate limiting headers'),
('zero_day_exploits', '9781593277502', 'Zero-Day Exploits: Offensive Security', 'Offensive security research', 2, 6, 499.99, 1, 'HIDDEN_CONTENT: Advanced buffer overflow techniques, kernel exploitation, and reverse engineering methodologies.', 'master', 'Check for timing attacks in authentication'),
('ai_red_team', '9781593277503', 'AI Red Teaming: Advanced Adversarial ML', 'Adversarial machine learning', 2, 6, 349.99, 2, 'HIDDEN_CONTENT: Deep learning model extraction, adversarial examples, and model inversion attacks.', 'master', 'GraphQL batch queries bypass rate limits'),
('blockchain_hacking', '9781593277504', 'Blockchain Hacking: DeFi Vulnerabilities', 'DeFi security research', 2, 6, 399.99, 2, 'HIDDEN_CONTENT: Smart contract exploitation, flash loan attacks, and oracle manipulation.', 'master', 'Look for second-order vulnerabilities'),
('memory_forensics', '9781593277505', 'Advanced Memory Forensics', 'RAM analysis techniques', 2, 6, 279.99, 3, 'HIDDEN_CONTENT: Volatility framework mastery, malware detection in RAM, and incident response.', 'master', 'Check XML parsing for XXE'),
('apt_analysis', '9781593277506', 'APT Analysis: Nation-State Threats', 'Advanced persistent threat research', 2, 6, 449.99, 1, 'HIDDEN_CONTENT: APT group tactics, techniques and procedures, and threat hunting methodologies.', 'master', 'WebSocket connections may leak data');

-- Insert sample coupons
INSERT INTO coupons (code, description, discount_type, discount_value, min_order_amount, usage_limit) VALUES
('WELCOME10', 'Welcome discount for new users', 'percentage', 10.0, 0, 100),
('FLAT20', 'Flat $20 off orders over $100', 'fixed', 20.0, 100.0, 50),
('SUMMER25', 'Summer sale - 25% off', 'percentage', 25.0, 50.0, 200),
('DISCOUNT10', 'General discount code', 'percentage', 10.0, 0, 100);

-- Insert sample reviews
INSERT INTO reviews (user_id, book_id, rating, comment, is_verified_purchase, is_approved)
SELECT u.id, b.id, 5, 'Excellent book! Highly recommended.', true, true
FROM users u, books b
WHERE u.username = 'user' AND b.id IN (1, 2);

INSERT INTO reviews (user_id, book_id, rating, comment, is_verified_purchase, is_approved)
SELECT u.id, b.id, 4, 'Good read but a bit technical.', true, true
FROM users u, books b
WHERE u.username = 'admin' AND b.id = 1;

-- Create shopping carts for users
INSERT INTO shopping_carts (user_id)
SELECT id FROM users WHERE username = 'user';

-- Add items to cart
INSERT INTO cart_items (cart_id, book_id, quantity)
SELECT sc.id, b.id, 2
FROM shopping_carts sc, books b, users u
WHERE u.username = 'user' AND sc.user_id = u.id AND b.id = 1;

-- Create sample orders
INSERT INTO orders (user_id, order_number, status, subtotal, total_amount, shipping_address, billing_address, payment_status)
SELECT u.id, 'ORD-001', 'delivered', 42.99, 45.99, '123 Test St', '123 Test St', 'completed'
FROM users u WHERE u.username = 'user';

-- Add order items
INSERT INTO order_items (order_id, book_id, book_title, book_isbn, quantity, unit_price, total_price)
SELECT o.id, b.id, b.title, b.isbn, 1, b.price, b.price
FROM orders o, books b, users u
WHERE u.username = 'user' AND o.user_id = u.id AND b.id = 1;

-- Add audit logs
INSERT INTO audit_logs (user_id, action, entity_type, entity_id, ip_address)
SELECT id, 'LOGIN', 'user', id::text, '127.0.0.1'::inet
FROM users;

-- Insert sample webhook for testing SSRF
INSERT INTO webhooks (user_id, url, events, secret)
SELECT id, 'http://example.com/webhook', '["ORDER_CREATED"]', 'webhook_secret_123'
FROM users WHERE username = 'admin';

-- Insert sensitive config
INSERT INTO system_config (key, value, is_sensitive) VALUES
('jwt_secret', 'CHANGE_ME_IN_PRODUCTION_real_jwt_secret_key_2024', true),
('db_password', 'bookstore_password', true),
('stripe_api_key', 'sk_test_123456789', true),
('aws_access_key', 'AKIAIOSFODNN7EXAMPLE', true);

-- Grant privileges
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO public;
GRANT USAGE, SELECT ON ALL SEQUENCES IN SCHEMA public TO public;
