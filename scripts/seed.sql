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
('Alice Williams', 'Mystery thriller writer'),
('Michael Chen', 'Data science and AI researcher'),
('Sarah Williams', 'Historical fiction writer'),
('David Brown', 'Fantasy and mythology author'),
('Emily Davis', 'Romance novelist'),
('Robert Miller', 'Business and finance author'),
('Jennifer Wilson', 'Self-help and motivational speaker'),
('Christopher Taylor', 'Cybersecurity expert'),
('Jessica Anderson', 'Children books author'),
('Matthew Thomas', 'Philosophical writer'),
('Amanda White', 'Poet and literary critic'),
('Daniel Martinez', 'DevOps and cloud computing'),
('Ashley Thompson', 'Young adult fiction writer'),
('Kevin Garcia', 'Software architecture expert'),
('Nicole Robinson', 'Graphic novel creator'),
('Brian Lee', 'Blockchain and crypto author'),
('Rachel Clark', 'Environmental science writer'),
('Jason Lewis', 'Game development author'),
('Stephanie Hall', 'Health and wellness writer'),
('Andrew Walker', 'Embedded systems engineer'),
('Michelle Allen', 'Thriller and suspense writer'),
('Justin Young', 'Network security specialist'),
('Laura King', 'Science communication writer'),
('Ryan Wright', 'Mobile development expert'),
('Kimberly Scott', 'Horror fiction author'),
('Brandon Adams', 'Database administrator author'),
('Megan Baker', 'Biographical writer'),
('Eric Nelson', 'Computer vision researcher'),
('Hannah Hill', 'Cooking and culinary author'),
('Austin Carter', 'Web security expert'),
('Samantha Mitchell', 'Travel and adventure writer'),
('Jordan Perez', 'Cryptography author'),
('Olivia Roberts', 'Paranormal fiction writer'),
('Tyler Turner', 'Blockchain developer'),
('Victoria Phillips', 'Drama and theater writer'),
('Dylan Campbell', 'Augmented reality expert'),
('Grace Mitchell', 'Memoir and autobiography author'),
('Austin Cooper', 'API design specialist'),
('Isabella Ross', 'Contemporary fiction writer'),
('Ethan Foster', 'Containerization expert'),
('Ava Thompson', 'Dystopian fiction author'),
('Jacob Davis', 'Machine learning engineer'),
('Chloe Evans', 'Art and photography book author'),
('Lucas Edwards', 'Serverless architecture expert'),
('Penelope Collins', 'Literary fiction writer'),
('Nathan Carter', 'Quantum computing researcher'),
('Zoey Rogers', 'Comedy and humor writer'),
('Adam Peterson', 'GraphQL API specialist'),
('Madison Bennett', 'Western fiction author'),
('Gabriel Torres', 'Edge computing expert'),
('Lily Hughes', 'Cozy mystery writer'),
('Justin Richardson', 'Kubernetes author'),
('Avery Jackson', 'Magical realism author'),
('Cameron Morris', 'API gateway specialist'),
('Taylor Morgan', 'Sports biography writer'),
('Hunter Rogers', 'Load balancing expert'),
('Hayley Stewart', 'Legal thriller author'),
('Cole Griffin', 'WebSocket specialist'),
('Aria Stone', 'Satirical fiction writer'),
('Wyatt Brooks', 'Microservices expert'),
('Jade Porter', 'True crime writer'),
('Blake Simpson', 'Service mesh specialist'),
('Riley Cooper', 'Crime fiction writer'),
('Jace Wallace', 'CI/CD pipeline expert'),
('Skyler Jensen', 'Adventure fiction writer'),
('Tristan Hayes', 'Observability tools author'),
('Autumn Lane', 'Southern fiction writer'),
('Spencer Reid', 'Chaos engineering expert'),
('Violet Reynolds', 'Gothic fiction writer'),
('Colton Brooks', 'Distributed systems author'),
('Quinn Fletcher', 'Urban fantasy writer'),
('Derek Wells', 'Site reliability engineer'),
('Sage Patterson', 'Comics and graphic novels');

-- Insert author profiles (secondary data with sensitive information - PRODUCTION MISTAKE: exposed via hidden query)
INSERT INTO author_profiles (author_id, email, phone, address, city, state, zip_code, country, emergency_contact, bank_account_last4, tax_id) VALUES
(5, 'michael.chen@authoremail.com', '+1-555-0101', '142 Oak Street', 'Seattle', 'WA', '98101', 'USA', 'Mary Chen: +1-555-0102', '4521', 'TAX-785412963'),
(6, 'sarah.williams@authoremail.com', '+1-555-0103', '89 Maple Avenue', 'Boston', 'MA', '02101', 'USA', 'Tom Williams: +1-555-0104', '7832', 'TAX-963214587'),
(7, 'david.brown@authoremail.com', '+1-555-0105', '267 Pine Road', 'Denver', 'CO', '80201', 'USA', 'Linda Brown: +1-555-0106', '2156', 'TAX-741258963'),
(8, 'emily.davis@authoremail.com', '+1-555-0107', '45 Cedar Lane', 'Austin', 'TX', '78701', 'USA', 'Mark Davis: +1-555-0108', '9043', 'TAX-852147369'),
(9, 'robert.miller@authoremail.com', '+1-555-0109', '156 Birch Boulevard', 'Chicago', 'IL', '60601', 'USA', 'Susan Miller: +1-555-0110', '6789', 'TAX-963852741'),
(10, 'jennifer.wilson@authoremail.com', '+1-555-0111', '78 Elm Street', 'Portland', 'OR', '97201', 'USA', 'John Wilson: +1-555-0112', '3412', 'TAX-147852963'),
(11, 'christopher.taylor@authoremail.com', '+1-555-0113', '321 Willow Way', 'San Francisco', 'CA', '94101', 'USA', 'Rachel Taylor: +1-555-0114', '8256', 'TAX-258963741'),
(12, 'jessica.anderson@authoremail.com', '+1-555-0115', '54 Spruce Court', 'Miami', 'FL', '33101', 'USA', 'Brian Anderson: +1-555-0116', '1579', 'TAX-369741852'),
(13, 'matthew.thomas@authoremail.com', '+1-555-0117', '189 Ash Drive', 'Philadelphia', 'PA', '19101', 'USA', 'Emma Thomas: +1-555-0118', '4862', 'TAX-471852963'),
(14, 'amanda.white@authoremail.com', '+1-555-0119', '92 Cherry Lane', 'Nashville', 'TN', '37201', 'USA', 'David White: +1-555-0120', '7234', 'TAX-582963147'),
(15, 'daniel.martinez@authoremail.com', '+1-555-0121', '234 Walnut Street', 'Phoenix', 'AZ', '85001', 'USA', 'Maria Martinez: +1-555-0122', '9108', 'TAX-693147258'),
(16, 'ashley.thompson@authoremail.com', '+1-555-0123', '67 Cypress Avenue', 'Atlanta', 'GA', '30301', 'USA', 'Ryan Thompson: +1-555-0124', '2645', 'TAX-714852369'),
(17, 'kevin.garcia@authoremail.com', '+1-555-0125', '178 Redwood Road', 'Dallas', 'TX', '75201', 'USA', 'Laura Garcia: +1-555-0126', '8371', 'TAX-825963471'),
(18, 'nicole.robinson@authoremail.com', '+1-555-0127', '43 Magnolia Boulevard', 'San Diego', 'CA', '92101', 'USA', 'Jason Robinson: +1-555-0128', '5093', 'TAX-936741852'),
(19, 'brian.lee@authoremail.com', '+1-555-0129', '156 Dogwood Lane', 'Minneapolis', 'MN', '55401', 'USA', 'Ashley Lee: +1-555-0130', '6728', 'TAX-147963285'),
(20, 'rachel.clark@authoremail.com', '+1-555-0131', '89 Hickory Street', 'Detroit', 'MI', '48201', 'USA', 'Kevin Clark: +1-555-0132', '3156', 'TAX-258147963'),
(21, 'jason.lewis@authoremail.com', '+1-555-0133', '267 Chestnut Avenue', 'Charlotte', 'NC', '28201', 'USA', 'Megan Lewis: +1-555-0134', '9482', 'TAX-369258741'),
(22, 'stephanie.hall@authoremail.com', '+1-555-0135', '45 Poplar Way', 'Indianapolis', 'IN', '46201', 'USA', 'Eric Hall: +1-555-0136', '7219', 'TAX-471369852'),
(23, 'andrew.walker@authoremail.com', '+1-555-0137', '134 Sycamore Drive', 'St. Louis', 'MO', '63101', 'USA', 'Nicole Walker: +1-555-0138', '2854', 'TAX-582471963'),
(24, 'michelle.allen@authoremail.com', '+1-555-0139', '78 Beech Court', 'Baltimore', 'MD', '21201', 'USA', 'Brandon Allen: +1-555-0140', '8167', 'TAX-693582147'),
(25, 'justin.young@authoremail.com', '+1-555-0141', '198 Sequoia Lane', 'Las Vegas', 'NV', '89101', 'USA', 'Rachel Young: +1-555-0142', '4531', 'TAX-714693258'),
(26, 'laura.king@authoremail.com', '+1-555-0143', '56 Cottonwood Street', 'Portland', 'OR', '97201', 'USA', 'Ryan King: +1-555-0144', '9278', 'TAX-825714369'),
(27, 'ryan.wright@authoremail.com', '+1-555-0145', '145 Juniper Avenue', 'Memphis', 'TN', '38101', 'USA', 'Kimberly Wright: +1-555-0146', '1805', 'TAX-936825471'),
(28, 'kimberly.scott@authoremail.com', '+1-555-0147', '92 Fir Road', 'Louisville', 'KY', '40201', 'USA', 'Justin Scott: +1-555-0148', '5632', 'TAX-147936582'),
(29, 'brandon.adams@authoremail.com', '+1-555-0149', '234 Aspen Boulevard', 'Milwaukee', 'WI', '53201', 'USA', 'Megan Adams: +1-555-0150', '8149', 'TAX-258147693'),
(30, 'megan.baker@authoremail.com', '+1-555-0151', '67 Alder Way', 'Albuquerque', 'NM', '87101', 'USA', 'Eric Baker: +1-555-0152', '3476', 'TAX-369258714'),
(31, 'eric.nelson@authoremail.com', '+1-555-0153', '178 Willow Street', 'Tucson', 'AZ', '85701', 'USA', 'Ashley Nelson: +1-555-0154', '9203', 'TAX-471369825'),
(32, 'hannah.hill@authoremail.com', '+1-555-0155', '43 Magnolia Drive', 'Fresno', 'CA', '93650', 'USA', 'Daniel Hill: +1-555-0156', '6851', 'TAX-582471936'),
(33, 'austin.carter@authoremail.com', '+1-555-0157', '156 Redwood Lane', 'Sacramento', 'CA', '95814', 'USA', 'Samantha Carter: +1-555-0158', '4287', 'TAX-693582147'),
(34, 'samantha.mitchell@authoremail.com', '+1-555-0159', '89 Spruce Avenue', 'Kansas City', 'MO', '64101', 'USA', 'Jordan Mitchell: +1-555-0160', '7614', 'TAX-714693258'),
(35, 'jordan.perez@authoremail.com', '+1-555-0161', '267 Pine Street', 'Mesa', 'AZ', '85201', 'USA', 'Victoria Perez: +1-555-0162', '2139', 'TAX-825714369'),
(36, 'olivia.roberts@authoremail.com', '+1-555-0163', '45 Cedar Road', 'Virginia Beach', 'VA', '23450', 'USA', 'Dylan Roberts: +1-555-0164', '8762', 'TAX-936825471'),
(37, 'tyler.turner@authoremail.com', '+1-555-0165', '134 Birch Boulevard', 'Omaha', 'NE', '68101', 'USA', 'Grace Turner: +1-555-0166', '5208', 'TAX-147936582'),
(38, 'victoria.phillips@authoremail.com', '+1-555-0167', '78 Maple Lane', 'Colorado Springs', 'CO', '80901', 'USA', 'Ethan Phillips: +1-555-0168', '1935', 'TAX-258147693'),
(39, 'dylan.campbell@authoremail.com', '+1-555-0169', '189 Elm Street', 'Raleigh', 'NC', '27601', 'USA', 'Ava Campbell: +1-555-0170', '6471', 'TAX-369258714'),
(40, 'grace.mitchell@authoremail.com', '+1-555-0171', '92 Ash Way', 'Long Beach', 'CA', '90801', 'USA', 'Jacob Mitchell: +1-555-0172', '9046', 'TAX-471369825'),
(41, 'austin.cooper@authoremail.com', '+1-555-0173', '234 Cherry Drive', 'Miami', 'FL', '33101', 'USA', 'Chloe Cooper: +1-555-0174', '5713', 'TAX-582471936'),
(42, 'isabella.ross@authoremail.com', '+1-555-0175', '67 Walnut Avenue', 'Oakland', 'CA', '94601', 'USA', 'Lucas Ross: +1-555-0176', '3289', 'TAX-693582147'),
(43, 'ethan.foster@authoremail.com', '+1-555-0177', '156 Cedar Court', 'Minneapolis', 'MN', '55401', 'USA', 'Penelope Foster: +1-555-0178', '8157', 'TAX-714693258'),
(44, 'ava.thompson@authoremail.com', '+1-555-0179', '43 Sycamore Street', 'Tulsa', 'OK', '74101', 'USA', 'Nathan Thompson: +1-555-0180', '4824', 'TAX-825714369'),
(45, 'jacob.davis@authoremail.com', '+1-555-0181', '178 Pine Road', 'Arlington', 'TX', '76001', 'USA', 'Zoey Davis: +1-555-0182', '1598', 'TAX-936825471'),
(46, 'chloe.evans@authoremail.com', '+1-555-0183', '89 Birch Lane', 'Bakersfield', 'CA', '93301', 'USA', 'Adam Evans: +1-555-0184', '7265', 'TAX-147936582'),
(47, 'lucas.edwards@authoremail.com', '+1-555-0185', '234 Maple Avenue', 'Anaheim', 'CA', '92801', 'USA', 'Madison Edwards: +1-555-0186', '9031', 'TAX-258147693'),
(48, 'penelope.collins@authoremail.com', '+1-555-0187', '56 Elm Boulevard', 'Santa Ana', 'CA', '92701', 'USA', 'Gabriel Collins: +1-555-0188', '2748', 'TAX-369258714'),
(49, 'nathan.carter@authoremail.com', '+1-555-0189', '145 Willow Way', 'Corpus Christi', 'TX', '78401', 'USA', 'Lily Carter: +1-555-0190', '8312', 'TAX-471369825'),
(50, 'zoey.rogers@authoremail.com', '+1-555-0191', '92 Cedar Street', 'Riverside', 'CA', '92501', 'USA', 'Justin Rogers: +1-555-0192', '5679', 'TAX-582471936'),
(51, 'adam.peterson@authoremail.com', '+1-555-0193', '267 Spruce Road', 'St. Petersburg', 'FL', '33701', 'USA', 'Avery Peterson: +1-555-0194', '3104', 'TAX-693582147'),
(52, 'madison.bennett@authoremail.com', '+1-555-0195', '78 Ash Lane', 'Laredo', 'TX', '78040', 'USA', 'Cameron Bennett: +1-555-0196', '8756', 'TAX-714693258'),
(53, 'gabriel.torres@authoremail.com', '+1-555-0197', '134 Pine Avenue', 'Stockton', 'CA', '95201', 'USA', 'Taylor Torres: +1-555-0198', '4283', 'TAX-825714369'),
(54, 'lily.hughes@authoremail.com', '+1-555-0199', '45 Maple Drive', 'St. Louis', 'MO', '63101', 'USA', 'Hunter Hughes: +1-555-0200', '7912', 'TAX-936825471'),
(55, 'justin.richardson@authoremail.com', '+1-555-0201', '189 Walnut Street', 'Pittsburgh', 'PA', '15201', 'USA', 'Hayley Richardson: +1-555-0202', '2547', 'TAX-147936582'),
(56, 'avery.jackson@authoremail.com', '+1-555-0203', '67 Birch Court', 'Anchorage', 'AK', '99501', 'USA', 'Cole Jackson: +1-555-0204', '8193', 'TAX-258147693'),
(57, 'cameron.morris@authoremail.com', '+1-555-0205', '178 Cherry Way', 'Plano', 'TX', '75023', 'USA', 'Aria Morris: +1-555-0206', '5861', 'TAX-369258714'),
(58, 'taylor.morgan@authoremail.com', '+1-555-0207', '43 Cedar Lane', 'Newark', 'NJ', '07101', 'USA', 'Wyatt Morgan: +1-555-0208', '1238', 'TAX-471369825'),
(59, 'hunter.rogers@authoremail.com', '+1-555-0209', '156 Elm Street', 'Lincoln', 'NE', '68501', 'USA', 'Jade Rogers: +1-555-0210', '7604', 'TAX-582471936'),
(60, 'hayley.stewart@authoremail.com', '+1-555-0211', '92 Ash Avenue', 'Toledo', 'OH', '43601', 'USA', 'Blake Stewart: +1-555-0212', '4175', 'TAX-693582147'),
(61, 'cole.griffin@authoremail.com', '+1-555-0213', '234 Pine Road', 'Henderson', 'NV', '89011', 'USA', 'Riley Griffin: +1-555-0214', '9829', 'TAX-714693258'),
(62, 'aria.stone@authoremail.com', '+1-555-0215', '56 Maple Boulevard', 'Chandler', 'AZ', '85224', 'USA', 'Jace Stone: +1-555-0216', '6352', 'TAX-825714369'),
(63, 'wyatt.brooks@authoremail.com', '+1-555-0217', '145 Birch Way', 'Glendale', 'AZ', '85301', 'USA', 'Skyler Brooks: +1-555-0218', '2087', 'TAX-936825471'),
(64, 'jade.porter@authoremail.com', '+1-555-0219', '89 Walnut Lane', 'Scottsdale', 'AZ', '85251', 'USA', 'Tristan Porter: +1-555-0220', '8714', 'TAX-147936582'),
(65, 'blake.simpson@authoremail.com', '+1-555-0221', '267 Cedar Avenue', 'Boise', 'ID', '83701', 'USA', 'Autumn Simpson: +1-555-0222', '5461', 'TAX-258147693'),
(66, 'riley.cooper@authoremail.com', '+1-555-0223', '78 Spruce Street', 'Irving', 'TX', '75014', 'USA', 'Spencer Cooper: +1-555-0224', '9138', 'TAX-369258714'),
(67, 'jace.wallace@authoremail.com', '+1-555-0225', '134 Ash Road', 'Chesapeake', 'VA', '23320', 'USA', 'Violet Wallace: +1-555-0226', '3792', 'TAX-471369825'),
(68, 'skyler.jensen@authoremail.com', '+1-555-0227', '45 Pine Lane', 'North Las Vegas', 'NV', '89030', 'USA', 'Colton Jensen: +1-555-0228', '8267', 'TAX-582471936'),
(69, 'tristan.hayes@authoremail.com', '+1-555-0229', '178 Maple Drive', 'Winston-Salem', 'NC', '27101', 'USA', 'Quinn Hayes: +1-555-0230', '5943', 'TAX-693582147'),
(70, 'autumn.lane@authoremail.com', '+1-555-0231', '92 Cedar Way', 'Gilbert', 'AZ', '85233', 'USA', 'Derek Lane: +1-555-0232', '2618', 'TAX-714693258'),
(71, 'spencer.reid@authoremail.com', '+1-555-0233', '189 Elm Avenue', 'Reno', 'NV', '89501', 'USA', 'Sage Reid: +1-555-0234', '8375', 'TAX-825714369'),
(72, 'violet.reynolds@authoremail.com', '+1-555-0235', '67 Birch Street', 'Hialeah', 'FL', '33010', 'USA', 'Colton Reynolds: +1-555-0236', '4102', 'TAX-936825471'),
(73, 'colton.brooks@authoremail.com', '+1-555-0237', '156 Cherry Lane', 'Garland', 'TX', '75040', 'USA', 'Quinn Brooks: +1-555-0238', '9856', 'TAX-147936582'),
(74, 'quinn.fletcher@authoremail.com', '+1-555-0239', '43 Walnut Road', 'Durham', 'NC', '27701', 'USA', 'Derek Fletcher: +1-555-0240', '5234', 'TAX-258147693'),
(75, 'derek.wells@authoremail.com', '+1-555-0241', '234 Cedar Boulevard', 'St. Petersburg', 'FL', '33701', 'USA', 'Sage Wells: +1-555-0242', '8701', 'TAX-369258714'),
(76, 'sage.patterson@authoremail.com', '+1-555-0243', '56 Pine Street', 'Irving', 'TX', '75060', 'USA', 'Wyatt Patterson: +1-555-0244', '3169', 'TAX-471369825');

-- Insert sample books
INSERT INTO books (isbn, title, description, author_id, category_id, price, stock_quantity, is_featured, is_bestseller) VALUES
('9780132350884', 'Clean Code', 'A handbook of agile software craftsmanship', 2, 6, 42.99, 25, true, true),
('9780201633610', 'Design Patterns', 'Elements of Reusable Object-Oriented Software', 2, 6, 54.99, 15, true, false),
('9780321125217', 'Domain-Driven Design', 'Tackling Complexity in Heart of Software', 2, 6, 49.99, 20, false, false),
('9780735619678', 'Code Complete', 'A Practical Handbook of Software Construction', 2, 6, 39.99, 30, false, false),
('9780345391803', 'The Hitchhiker''s Guide to the Galaxy', 'A sci-fi comedy classic', 3, 3, 14.99, 50, true, true),
('9780345391802', 'The Restaurant at the End of the Universe', 'Second book in trilogy', 3, 3, 14.99, 45, false, false),
('9780123456789', 'Machine Learning Fundamentals', 'Comprehensive guide to ML algorithms', 5, 6, 49.99, 35, true, true),
('9780234567890', 'The Last Dynasty', 'Historical tale of fallen empire', 6, 1, 24.99, 60, false, false),
('9780345456789', 'Dragonborn Chronicles', 'Epic fantasy adventure', 7, 3, 18.99, 80, true, true),
('9780456123456', 'Love in the Time of Coffee', 'Modern romance novel', 8, 2, 14.99, 100, false, true),
('9780678901234', 'Financial Freedom', 'Investment strategies for beginners', 9, 5, 29.99, 45, false, false),
('9780785123456', 'Mindset Mastery', 'Transform your thinking', 10, 4, 19.99, 70, true, false),
('9780898678901', 'The Hacker''s Playbook', 'Practical penetration testing', 11, 6, 39.99, 25, true, true),
('9780123456780', 'The Magic Tree House', 'Children adventure series', 12, 1, 12.99, 150, true, true),
('9780234567891', 'Philosophy of Existence', 'Exploring life''s meaning', 13, 1, 22.99, 30, false, false),
('9780345678902', 'Poems from the Heart', 'Collection of heartfelt poetry', 14, 1, 15.99, 40, false, false),
('9780456789012', 'Kubernetes in Production', 'Container orchestration guide', 15, 6, 44.99, 20, true, false),
('9780567890123', 'The Forgotten Realm', 'Young adult fantasy epic', 16, 3, 16.99, 90, false, true),
('9780678901235', 'System Design Interview', 'Architecture patterns for scale', 17, 6, 38.99, 35, true, true),
('9780789012345', 'The Graphic Novel Journey', 'Visual storytelling guide', 18, 2, 24.99, 50, false, false),
('9780890123456', 'Mastering Blockchain', 'Cryptocurrency development', 19, 6, 42.99, 30, true, true),
('9780901234567', 'Our Planet Earth', 'Environmental conservation guide', 20, 2, 18.99, 55, false, false),
('9780912345678', 'Game Development Mastery', 'Unity and Unreal引擎', 21, 6, 49.99, 25, false, false),
('9780923456789', 'The Wellness Guide', 'Holistic health approach', 22, 4, 21.99, 65, false, false),
('9780934567890', 'Embedded Systems Programming', 'Microcontroller development', 23, 6, 46.99, 20, false, false),
('9780945678901', 'The Midnight Library', 'Psychological thriller', 24, 2, 17.99, 85, true, true),
('9780956789012', 'Network Security Essentials', 'Defensive security guide', 25, 6, 35.99, 40, false, false),
('9780967890123', 'Science for Everyone', 'Making complex topics simple', 26, 2, 19.99, 50, false, false),
('9780978901234', 'Mobile App Architecture', 'iOS and Android patterns', 27, 6, 41.99, 30, true, false),
('9780989012345', 'Shadows in the Dark', 'Horror collection', 28, 2, 14.99, 75, false, true),
('9780990123456', 'SQL Performance Tuning', 'Database optimization guide', 29, 6, 37.99, 25, false, false),
('9780991234567', 'A Life in Words', 'Biography of a writer', 30, 1, 23.99, 35, false, false),
('9780992345678', 'Deep Learning Bible', 'Neural networks deep dive', 31, 6, 54.99, 20, true, true),
('9780993456789', 'The Chef''s Kitchen', 'Culinary masterclass', 32, 2, 28.99, 45, false, false),
('9780994567890', 'Web Security Testing', 'OWASP top 10 guide', 33, 6, 33.99, 30, false, false),
('9780995678901', 'Wanderlust Adventures', 'Travel stories around world', 33, 2, 19.99, 60, false, false),
('9780996789012', 'Zero to Blockchain', 'Crypto development tutorial', 34, 6, 39.99, 35, false, false),
('9780997890123', 'The Drama Queen', 'Theatrical memoir', 35, 1, 16.99, 40, false, false),
('9780998901234', 'AR/VR Development', 'Immersive tech guide', 36, 6, 47.99, 20, false, false),
('9780999012345', 'My Life Story', 'Personal journey memoir', 37, 1, 21.99, 50, false, false),
('9780900123456', 'API Design Patterns', 'RESTful best practices', 38, 6, 36.99, 30, false, false),
('9780901234568', 'The City of Dreams', 'Contemporary fiction', 39, 2, 15.99, 70, false, true),
('9780902345678', 'Serverless Architectures', 'Cloud-native patterns', 40, 6, 43.99, 25, false, false),
('9780903456789', 'Dystopian Future', 'YA dystopian novel', 41, 3, 13.99, 95, true, true),
('9780904567890', 'Machine Learning in Production', 'MLOps guide', 42, 6, 51.99, 20, true, false),
('9780905678901', 'Photography Masterclass', 'Camera techniques', 43, 2, 29.99, 40, false, false),
('9780906789012', 'Microservices Patterns', 'Distributed systems design', 44, 6, 48.99, 20, false, false),
('9780907890123', 'Crime Scene Investigation', 'Forensic science guide', 45, 2, 20.99, 45, false, false),
('9780908901234', 'Observability Engineering', 'Monitoring and logging', 46, 6, 44.99, 25, false, false),
('9780909012345', 'The Western Frontier', 'Western genre novel', 47, 1, 17.99, 35, false, false),
('9780900123457', 'Chaos Engineering', 'Resilience testing', 48, 6, 40.99, 25, false, false),
('9780911234567', 'The Gothic House', 'Gothic horror novel', 49, 2, 18.99, 55, false, false),
('9780912345679', 'Distributed Systems', 'Scalable architectures', 50, 6, 52.99, 20, false, false),
('9780913456789', 'Urban Magic', 'Urban fantasy series', 51, 3, 14.99, 80, false, true),
('9780914567890', 'Site Reliability Engineering', 'SRE practices', 52, 6, 46.99, 20, false, false),
('9780915678901', 'The Comic Collection', 'Graphic novel anthology', 53, 2, 27.99, 40, false, false),
('9780916789012', 'API Gateway Patterns', 'Edge services design', 54, 6, 38.99, 30, false, false);

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
