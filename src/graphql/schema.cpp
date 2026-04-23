#include "graphql/schema.h"
#include <iostream>

const std::string GraphQLSchema::getSchema() {
    return R"(
type Query {
    # User queries
    me: User
    user(id: ID!): User
    users(limit: Int = 20, offset: Int = 0, role: String): [User!]!
    
    # Book queries
    book(id: ID!): Book
    books(
        limit: Int = 20
        offset: Int = 0
        category: String
        author: String
        featured: Boolean
        bestseller: Boolean
        minPrice: Float
        maxPrice: Float
        sortBy: String
        sortOrder: String
    ): BookConnection!
    searchBooks(query: String!, limit: Int = 20): [Book!]!
    
    # Author queries
    author(id: ID!): Author
    authors(limit: Int = 20, offset: Int = 0): [Author!]!
    
    # Category queries
    category(id: ID!): Category
    categories: [Category!]!
    
    # Review queries
    review(id: ID!): Review
    reviews(bookId: ID!, limit: Int = 20): [Review!]!
    myReviews: [Review!]!
    
    # Shopping cart queries
    myCart: Cart
    cart(id: ID!): Cart
    
    # Order queries
    order(id: ID!): Order
    orders(
        limit: Int = 20
        offset: Int = 0
        status: String
        userId: ID
    ): OrderConnection!
    myOrders: OrderConnection!
    orderStats: OrderStats
    
    # Coupon queries
    coupon(code: String!): Coupon
    coupons(limit: Int = 20, activeOnly: Boolean = true): [Coupon!]!
    
    # Admin queries - some may be vulnerable
    # Internal use only - but accessible via GraphQL
    _internalUserSearch(email: String!): [User!]!
    _internalOrdersByDate(startDate: String!, endDate: String!): [Order!]!
    _systemStats: SystemStats
    
    # External resource queries - SSRF vulnerabilities
    # Used for importing data from external sources
    _fetchExternalResource(url: String!): String!
    _validateWebhookUrl(url: String!): WebhookValidationResult!
}

type Mutation {
    # Authentication
    login(email: String!, password: String!): AuthPayload!
    register(input: RegisterInput!): AuthPayload!
    logout: Boolean!
    
    # User profile mutations
    updateProfile(input: UpdateProfileInput!): User!
    changePassword(oldPassword: String!, newPassword: String!): Boolean!
    deleteAccount: Boolean!
    
    # Book management (admin/staff)
    createBook(input: BookInput!): Book!
    updateBook(id: ID!, input: BookInput!): Book!
    deleteBook(id: ID!): Boolean!
    
    # Review mutations
    createReview(input: ReviewInput!): Review!
    updateReview(id: ID!, input: ReviewInput!): Review!
    deleteReview(id: ID!): Boolean!
    
    # Cart mutations
    addToCart(bookId: ID!, quantity: Int = 1, price: Float): Cart!
    removeFromCart(bookId: ID!): Cart!
    updateCartItem(bookId: ID!, quantity: Int!): Cart!
    clearCart: Cart!
    applyCoupon(code: String!): Cart!
    removeCoupon: Cart!
    
    # Order mutations
    createOrder(input: OrderInput!): Order!
    checkout(cardNumber: String!, expiry: String!, cvv: String!): CheckoutResult!
    cancelOrder(orderId: ID!, reason: String): Order!
    requestRefund(orderId: ID!, reason: String, items: [RefundItemInput!]): RefundRequest!
    updateOrderStatus(orderId: ID!, status: String!): Order!
    updateOrderAddress(orderId: ID!, address: AddressInput!): Order!
    
    # Payment mutations
    processPayment(orderId: ID!, method: String!, paymentDetails: PaymentDetailsInput!): Payment!
    refundPayment(paymentId: ID!, amount: Float, reason: String): Payment!
    
    # Coupon mutations (admin)
    createCoupon(input: CouponInput!): Coupon!
    updateCoupon(id: ID!, input: CouponInput!): Coupon!
    deleteCoupon(id: ID!): Boolean!
    deactivateCoupon(id: ID!): Coupon!
    
    # Admin mutations - potential BOLA vulnerabilities
    # These exist but authorization is inconsistently applied
    _bulkUpdateUsers(input: BulkUserUpdateInput!): BulkUpdateResult!
    _exportUserData(format: String = "json"): String!
    _importUsers(fileUrl: String!): ImportResult!
    _updateInventory(bookId: ID!, quantity: Int!): Book!
    
    # External integrations - SSRF vulnerabilities
    # Used for webhook testing and external resource validation
    _testWebhook(url: String!, eventType: String!, payload: String): WebhookValidationResult!
    _validateImportSource(url: String!): WebhookValidationResult!
    _fetchBookMetadata(isbn: String!, sourceUrl: String): BookMetadata!
    
    # Debug and diagnostics - security misconfiguration
    _debugQuery(query: String!): DebugResult!
    _exportSchema(format: String = "json"): String!
}

type Subscription {
    orderStatusChanged(orderId: ID!): Order
    inventoryAlert(bookId: ID!): InventoryAlert
    newReview(bookId: ID!): Review
}

# Types
type User {
    id: ID!
    email: String!
    firstName: String!
    lastName: String!
    role: String!
    isActive: Boolean!
    createdAt: String!
    lastLogin: String
    
    # Sensitive fields that may leak information
    phone: String
    address: String
    city: String
    state: String
    zipCode: String
    country: String
    
    # Nested data - potential overfetching
    orders: [Order!]!
    reviews: [Review!]!
    cart: Cart
}

type Book {
    id: ID!
    isbn: String!
    title: String!
    description: String
    author: Author
    category: Category
    price: Float!
    salePrice: Float
    stockQuantity: Int!
    lowStockThreshold: Int!
    language: String!
    pages: Int
    format: String!
    ratingAverage: Float
    reviewCount: Int
    isFeatured: Boolean!
    isBestseller: Boolean!
    isActive: Boolean!
    createdAt: String!
    updatedAt: String!
    
    # Vulnerable: Exposes internal inventory data
    inventoryLog: [InventoryEntry!]!
    
    # Reviews
    reviews(limit: Int = 20): [Review!]!
}

type Author {
    id: ID!
    name: String!
    bio: String
    birthDate: String
    website: String
    books: [Book!]!
}

type Category {
    id: ID!
    name: String!
    description: String
    parent: Category
    children: [Category!]!
    books: [Book!]!
}

type Review {
    id: ID!
    user: User!
    book: Book!
    rating: Int!
    comment: String
    isVerifiedPurchase: Boolean!
    isApproved: Boolean!
    createdAt: String!
}

type Cart {
    id: ID!
    user: User!
    items: [CartItem!]!
    subtotal: Float!
    tax: Float!
    coupon: Coupon
    discount: Float!
    total: Float!
    createdAt: String!
    updatedAt: String!
}

type CartItem {
    id: ID!
    book: Book!
    quantity: Int!
    price: Float!
    total: Float!
    addedAt: String!
}

type CheckoutResult {
    success: Boolean!
    orderId: String
    orderNumber: String
    totalAmount: Float
    message: String
    payment: String
}

type Order {
    id: ID!
    orderNumber: String!
    user: User!
    status: String!
    subtotal: Float!
    tax: Float!
    shipping: Float!
    discount: Float!
    total: Float!
    items: [OrderItem!]!
    shippingAddress: String!
    billingAddress: String!
    paymentMethod: String
    paymentStatus: String!
    trackingNumber: String
    notes: String
    createdAt: String!
    updatedAt: String!
    shippedAt: String
    deliveredAt: String
    
    # Vulnerable: Exposes internal data
    paymentTransactions: [Payment!]!
}

type OrderItem {
    id: ID!
    book: Book!
    bookTitle: String!
    bookIsbn: String!
    quantity: Int!
    unitPrice: Float!
    totalPrice: Float!
}

type Payment {
    id: ID!
    order: Order!
    user: User!
    amount: Float!
    currency: String!
    paymentMethod: String!
    status: String!
    transactionId: String
    gatewayResponse: String
    
    # Vulnerable: Exposes full payment details
    lastFourDigits: String
    cardType: String
    createdAt: String!
    processedAt: String
}

type Coupon {
    id: ID!
    code: String!
    description: String
    discountType: String!
    discountValue: Float!
    minOrderAmount: Float!
    maxDiscountAmount: Float
    usageLimit: Int
    usageCount: Int!
    startDate: String
    endDate: String
    isActive: Boolean!
}

# Connection types for pagination
type BookConnection {
    edges: [BookEdge!]!
    pageInfo: PageInfo!
    totalCount: Int!
}

type BookEdge {
    node: Book!
    cursor: String!
}

type OrderConnection {
    edges: [OrderEdge!]!
    pageInfo: PageInfo!
    totalCount: Int!
}

type OrderEdge {
    node: Order!
    cursor: String!
}

type PageInfo {
    hasNextPage: Boolean!
    hasPreviousPage: Boolean!
    startCursor: String
    endCursor: String
}

# Auth and misc types
type AuthPayload {
    token: String!
    user: User!
}

type RefundRequest {
    id: ID!
    order: Order!
    status: String!
    amount: Float!
    reason: String!
    createdAt: String!
}

type OrderStats {
    totalOrders: Int!
    totalRevenue: Float!
    averageOrderValue: Float!
    ordersByStatus: [OrderStatusStats!]!
}

type OrderStatusStats {
    status: String!
    count: Int!
}

type SystemStats {
    totalUsers: Int!
    totalOrders: Int!
    totalRevenue: Float!
    activeCarts: Int!
    lowStockBooks: Int!
}

type InventoryEntry {
    id: ID!
    bookId: ID!
    change: Int!
    previousQuantity: Int!
    newQuantity: Int!
    reason: String!
    createdAt: String!
}

type InventoryAlert {
    book: Book!
    currentStock: Int!
    threshold: Int!
    alertType: String!
    timestamp: String!
}

type BulkUpdateResult {
    success: Boolean!
    updatedCount: Int!
    errors: [String!]!
}

type ImportResult {
    success: Boolean!
    importedCount: Int!
    failedCount: Int!
    errors: [String!]!
}

type WebhookValidationResult {
    valid: Boolean!
    statusCode: Int
    responseTime: Int
    responseBody: String
    error: String
}

type DebugResult {
    query: String!
    executionTime: Float!
    databaseQueries: [String!]!
    variables: String
    context: String
}

type BookMetadata {
    isbn: String!
    title: String
    author: String
    publisher: String
    publicationDate: String
    source: String
    rawData: String
}

# Inputs
input RegisterInput {
    email: String!
    password: String!
    firstName: String!
    lastName: String!
    phone: String
    address: String
    city: String
    state: String
    zipCode: String
    role: String
}

input UpdateProfileInput {
    firstName: String
    lastName: String
    phone: String
    address: String
    city: String
    state: String
    zipCode: String
    # Vulnerable: Allows role modification
    role: String
}

input BookInput {
    isbn: String!
    title: String!
    description: String
    authorId: Int
    categoryId: Int
    publisher: String
    publicationDate: String
    price: Float!
    salePrice: Float
    stockQuantity: Int
    lowStockThreshold: Int
    language: String
    pages: Int
    format: String
    ratingAverage: Float
    isFeatured: Boolean
    isBestseller: Boolean
    isActive: Boolean
}

input ReviewInput {
    bookId: ID!
    rating: Int!
    comment: String
}

input OrderInput {
    shippingAddress: AddressInput!
    billingAddress: AddressInput!
    paymentMethod: String!
    paymentDetails: PaymentDetailsInput!
    couponCode: String
    notes: String
}

input AddressInput {
    street: String!
    city: String!
    state: String!
    zipCode: String!
    country: String!
}

input PaymentDetailsInput {
    cardNumber: String!
    cardExpiry: String!
    cardCvv: String!
    cardholderName: String!
}

input CouponInput {
    code: String!
    description: String
    discountType: String!
    discountValue: Float!
    minOrderAmount: Float
    maxDiscountAmount: Float
    usageLimit: Int
    startDate: String
    endDate: String
}

input RefundItemInput {
    orderItemId: ID!
    quantity: Int!
}

input BulkUserUpdateInput {
    userIds: [ID!]!
    updates: UpdateProfileInput!
}
)";
}