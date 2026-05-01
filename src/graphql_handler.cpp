#include <string>
#include <vector>
#include <sstream>
#include <regex>
#include <iostream> 
#include <map> 
#include <ctime> 
#include <cstring> 

#include <postgresql/libpq-fe.h>
#include <jwt.h>
#include <curl/curl.h>

#include "graphql_handler.h"
#include "user_manager.h"
#include "book_manager.h"
#include "order_manager.h"
#include "extra_features.h"
#include "utils.h"
#include "network_manager.h"
#include "db_manager.h"
#include "payment_handler.h"

// External caches and DB connection
extern PGconn *dbConn;
extern std::map<std::string, User> usersCache;
extern std::map<int, Book> booksCache;
extern std::map<int, Author> authorsCache;
extern std::map<std::string, std::vector<CartItem>> cartCache;
extern std::map<std::string, Order> ordersCache;
extern std::map<int, Review> reviewsCache;
extern std::map<std::string, Webhook> webhooksCache;


// GraphQL Helper Functions
std::string extractJsonString(const std::string& body, size_t startPos) {
    size_t braceCount = 0;
    size_t endPos = startPos;
    bool inString = false;
    bool escaped = false;

    for (size_t i = startPos; i < body.length(); ++i) {
        char c = body[i];
        if (escaped) {
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '"') {
            inString = !inString;
        } else if (!inString) {
            if (c == '{') {
                braceCount++;
            } else if (c == '}') {
                braceCount--;
            }
        }
        if (braceCount == 0 && !inString && c == '}') {
            endPos = i;
            break;
        }
    }
    return body.substr(startPos, endPos - startPos + 1);
}

std::string extractQueryFromBody(const std::string& body) {
    size_t queryPos = body.find("\"query\":");
    if (queryPos == std::string::npos) return "";
    size_t start = body.find('"', queryPos + 8); // Skip \"query\":
    if (start == std::string::npos) return "";
    start++; // Move past the opening quote

    std::string queryString;
    bool escaped = false;
    for (size_t i = start; i < body.length(); ++i) {
        if (escaped) {
            queryString += body[i];
            escaped = false;
        } else if (body[i] == '\\') {
            escaped = true;
            // queryString += body[i]; // Keep the backslash for proper parsing later
        } else if (body[i] == '"') {
            // End of query string
            break;
        } else {
            queryString += body[i];
        }
    }
    return queryString;
}

std::string extractValue(const std::string& query, const std::string& key) {
    std::string searchKey = key + ":";
    size_t keyPos = query.find(searchKey);
    if (keyPos == std::string::npos) return "";
    
    size_t searchStart = keyPos + searchKey.length();
    
    // Skip whitespace
    while (searchStart < query.length() && 
           (query[searchStart] == ' ' || query[searchStart] == '\t')) {
        searchStart++;
    }
    
    if (searchStart >= query.length()) return "";
    
    // Skip opening quote (may be escaped with backslash like \")
    if (query[searchStart] == '"') {
        searchStart++;
    } else if (query[searchStart] == '\\' && 
               searchStart + 1 < query.length() && 
               query[searchStart + 1] == '"') {
        // Skip escaped quote: \"
        searchStart += 2;
    }
    
    std::string value;
    bool escaped = false;
    
    for (size_t i = searchStart; i < query.length(); i++) {
        char c = query[i];
        
        if (escaped) {
            // If we're escaped and see a quote, it's an escaped quote - skip it
            if (c != '"') {
                value += c;
            }
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '"') {
            // End of string
            return value;
        } else if (c == ' ' || c == ',' || c == ')' || c == '{' || c == '}') {
            // End of value (unquoted)
            return value;
            
        } else {
            value += c;
        }
    }
    
    return value;
}

std::string extractIntValue(const std::string& query, const std::string& key) {
    std::string searchKey = key + ":";
    size_t keyPos = query.find(searchKey);
    if (keyPos == std::string::npos) return "";
    size_t start = keyPos + searchKey.length();
    while (start < query.length() && (query[start] == ' ' || query[start] == '\t')) {
        start++;
    }
    size_t end = start;
    while (end < query.length() && isdigit(query[end])) {
        end++;
    }
    return query.substr(start, end - start);
}

bool isFieldRequested(const std::string& query, const std::string& fieldName) {
    size_t pos = query.find(fieldName);
    while (pos != std::string::npos) {
        char before = (pos > 0) ? query[pos - 1] : ' ';
        char after = (pos + fieldName.length() < query.length()) ? query[pos + fieldName.length()] : ' ';
        
        bool validStart = (before == ' ' || before == '{' || before == '\n' || before == '\t' || before == ',');
        bool validEnd = (after == ' ' || after == ',' || after == '}' || after == '{' || after == '\r' || after == '\t' || after == '\n');
        
        if (validStart && validEnd) return true;
        
        pos = query.find(fieldName, pos + 1);
    }
    return false;
}

std::string extractSubQuery(const std::string& query, const std::string& fieldName) {
    size_t fieldPos = query.find(fieldName + " {");
    if (fieldPos == std::string::npos) {
        fieldPos = query.find(fieldName + "{");
    }
    if (fieldPos == std::string::npos) return "";
    
    size_t braceStart = query.find("{", fieldPos);
    if (braceStart == std::string::npos) return "";
    
    int braceCount = 1;
    size_t pos = braceStart + 1;
    while (pos < query.length() && braceCount > 0) {
        if (query[pos] == '{') braceCount++;
        else if (query[pos] == '}') braceCount--;
        pos++;
    }
    
    return query.substr(braceStart + 1, pos - braceStart - 2);
}

bool isFieldRequestedInContext(const std::string& query, const std::string& contextName, const std::string& fieldName) {
    std::string subQuery = extractSubQuery(query, contextName);
    if (subQuery.empty()) {
        return isFieldRequested(query, fieldName);
    }
    return isFieldRequested(subQuery, fieldName);
}

bool hasAnyUserField(const std::string& query) {
    return isFieldRequested(query, "id") || isFieldRequested(query, "username") ||
           isFieldRequested(query, "firstName") || isFieldRequested(query, "lastName") ||
           isFieldRequested(query, "role") || isFieldRequested(query, "phone") ||
           isFieldRequested(query, "address") || isFieldRequested(query, "city") ||
           isFieldRequested(query, "state") || isFieldRequested(query, "zipCode") ||
           isFieldRequested(query, "country") || isFieldRequested(query, "email") ||
           isFieldRequested(query, "isActive") || isFieldRequested(query, "lastLogin");
}

// JSON Conversion Functions
std::string userToJson(const User& user, const std::string& query) {
    std::stringstream ss;
    ss << "{";
    bool firstField = true;
    bool includeAll = query.empty() || !hasAnyUserField(query);
    if (includeAll || isFieldRequested(query, "id")) {
        ss << "\"id\":\"" << user.id << "\"";
        firstField = false;
    }
    if (includeAll || isFieldRequested(query, "username")) {
        if (!firstField) ss << ",";
        ss << "\"username\":\"" << escapeJson(user.username) << "\"";
        firstField = false;
    }
    if (includeAll || isFieldRequested(query, "firstName")) {
        if (!firstField) ss << ",";
        ss << "\"firstName\":\"" << escapeJson(user.firstName) << "\"";
        firstField = false;
    }
    if (includeAll || isFieldRequested(query, "lastName")) {
        if (!firstField) ss << ",";
        ss << "\"lastName\":\"" << escapeJson(user.lastName) << "\"";
        firstField = false;
    }
    if (includeAll || isFieldRequested(query, "role")) {
        if (!firstField) ss << ",";
        ss << "\"role\":\"" << escapeJson(user.role) << "\"";
        firstField = false;
    }

    if (isFieldRequested(query, "passwordHash")) {
        if (!firstField) ss << ",";
        ss << "\"passwordHash\":\"" << escapeJson(user.passwordHash) << "\"";
        firstField = false;
    }

    if (includeAll || isFieldRequested(query, "phone")) {
        if (!firstField) ss << ",";
        ss << "\"phone\":\"" << escapeJson(user.phone) << "\"";
        firstField = false;
    }
    if (includeAll || isFieldRequested(query, "address")) {
        if (!firstField) ss << ",";
        ss << "\"address\":\"" << escapeJson(user.address) << "\"";
        firstField = false;
    }
    if (includeAll || isFieldRequested(query, "city")) {
        if (!firstField) ss << ",";
        ss << "\"city\":\"" << escapeJson(user.city) << "\"";
        firstField = false;
    }
    if (includeAll || isFieldRequested(query, "state")) {
        if (!firstField) ss << ",";
        ss << "\"state\":\"" << escapeJson(user.state) << "\"";
        firstField = false;
    }
    if (includeAll || isFieldRequested(query, "zipCode")) {
        if (!firstField) ss << ",";
        ss << "\"zipCode\":\"" << escapeJson(user.zipCode) << "\"";
        firstField = false;
    }
    if (includeAll || isFieldRequested(query, "country")) {
        if (!firstField) ss << ",";
        ss << "\"country\":\"" << escapeJson(user.country) << "\"";
        firstField = false;
    }
    if (includeAll || isFieldRequested(query, "isActive")) {
        if (!firstField) ss << ",";
        ss << "\"isActive\":" << (user.isActive ? "true" : "false");
        firstField = false;
    }
    ss << "}";
    return ss.str();
}

std::string bookToJson(const Book& book, const std::string& query) {
    std::stringstream ss;
    ss << "{";
    bool firstField = true;
    if (query.empty() || isFieldRequested(query, "id")) {
        ss << "\"id\":" << book.id;
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "title")) {
        if (!firstField) ss << ",";
        ss << "\"title\":\"" << escapeJson(book.title) << "\"";
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "isbn")) {
        if (!firstField) ss << ",";
        ss << "\"isbn\":\"" << escapeJson(book.isbn) << "\"";
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "description")) {
        if (!firstField) ss << ",";
        ss << "\"description\":\"" << escapeJson(book.description) << "\"";
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "price")) {
        if (!firstField) ss << ",";
        ss << "\"price\":" << book.price;
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "salePrice")) {
        if (!firstField) ss << ",";
        ss << "\"salePrice\":" << book.salePrice;
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "stockQuantity")) {
        if (!firstField) ss << ",";
        ss << "\"stockQuantity\":" << book.stockQuantity;
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "author")) {
        if (!firstField) ss << ",";
        Author* author = getAuthorById(book.authorId);
        if (author) {
            ss << "\"author\":{";
            bool firstAuthorField = true;
            if (isFieldRequested(query, "id") && query.find("author {") != std::string::npos) {
                ss << "\"id\":" << author->id;
                firstAuthorField = false;
            }
            if (isFieldRequested(query, "firstName")) {
                if (!firstAuthorField) ss << ",";
                ss << "\"firstName\":\"" << escapeJson(author->firstName) << "\"";
                firstAuthorField = false;
            }
            if (isFieldRequested(query, "lastName")) {
                if (!firstAuthorField) ss << ",";
                ss << "\"lastName\":\"" << escapeJson(author->lastName) << "\"";
                firstAuthorField = false;
            }
            if (isFieldRequested(query, "bio")) {
                if (!firstAuthorField) ss << ",";
                ss << "\"bio\":\"" << escapeJson(author->bio) << "\"";
                firstAuthorField = false;
            }
            ss << "}";
        } else {
            ss << "\"author\":null";
        }
        firstField = false;
    }
    ss << "}";
    return ss.str();
}

std::string cartItemToJson(const CartItem& item, const std::string& query) {
    std::stringstream ss;
    ss << "{";
    bool firstField = true;
    if (query.empty() || isFieldRequested(query, "bookId")) {
        ss << "\"bookId\":" << item.bookId;
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "quantity")) {
        if (!firstField) ss << ",";
        ss << "\"quantity\":" << item.quantity;
        firstField = false;
    }
    // Nested book object
    if (query.find("book {") != std::string::npos) {
        if (!firstField) ss << ",";
        Book* book = getBookById(item.bookId);
        if (book) {
            ss << "\"book\":" << bookToJson(*book, query.substr(query.find("book {")));
        } else {
            ss << "\"book\":null";
        }
        firstField = false;
    }
    ss << "}";
    return ss.str();
}

std::string orderItemToJson(const OrderItem& item, const std::string& query) {
    std::stringstream ss;
    ss << "{";
    bool firstField = true;
    if (query.empty() || isFieldRequested(query, "bookId")) {
        ss << "\"bookId\":" << item.bookId;
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "bookTitle")) {
        if (!firstField) ss << ",";
        ss << "\"bookTitle\":\"" << escapeJson(item.bookTitle) << "\"";
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "quantity")) {
        if (!firstField) ss << ",";
        ss << "\"quantity\":" << item.quantity;
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "unitPrice")) {
        if (!firstField) ss << ",";
        ss << "\"unitPrice\":" << item.unitPrice;
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "totalPrice")) {
        if (!firstField) ss << ",";
        ss << "\"totalPrice\":" << item.totalPrice;
        firstField = false;
    }
    ss << "}";
    return ss.str();
}

std::string orderToJson(const Order& order, const std::string& query) {
    std::stringstream ss;
    ss << "{";
    bool firstField = true;
    if (query.empty() || isFieldRequested(query, "id")) {
        ss << "\"id\":\"" << order.id << "\"";
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "orderNumber")) {
        if (!firstField) ss << ",";
        ss << "\"orderNumber\":\"" << escapeJson(order.orderNumber) << "\"";
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "userId")) {
        if (!firstField) ss << ",";
        ss << "\"userId\":\"" << order.userId << "\"";
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "status")) {
        if (!firstField) ss << ",";
        ss << "\"status\":\"" << escapeJson(order.status) << "\"";
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "totalAmount")) {
        if (!firstField) ss << ",";
        ss << "\"totalAmount\":" << order.totalAmount;
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "items")) {
        if (!firstField) ss << ",";
        ss << "\"items\":[";
        bool firstItem = true;
        for (const auto& item : order.items) {
            if (!firstItem) ss << ",";
            ss << orderItemToJson(item, query);
            firstItem = false;
        }
        ss << "]";
        firstField = false;
    }
    ss << "}";
    return ss.str();
}

std::string reviewToJson(const Review& review, const std::string& query) {
    std::stringstream ss;
    ss << "{";
    bool firstField = true;
    if (query.empty() || isFieldRequested(query, "id")) {
        ss << "\"id\":\"" << review.id << "\"";
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "bookId")) {
        if (!firstField) ss << ",";
        ss << "\"bookId\":" << review.bookId;
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "userId")) {
        if (!firstField) ss << ",";
        ss << "\"userId\":\"" << review.userId << "\"";
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "rating")) {
        if (!firstField) ss << ",";
        ss << "\"rating\":" << review.rating;
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "comment")) {
        if (!firstField) ss << ",";
        ss << "\"comment\":\"" << escapeJson(review.comment) << "\"";
        firstField = false;
    }
    ss << "}";
    return ss.str();
}

std::string webhookToJson(const Webhook& webhook, const std::string& query) {
    std::stringstream ss;
    ss << "{";
    bool firstField = true;
    if (query.empty() || isFieldRequested(query, "id")) {
        ss << "\"id\":\"" << webhook.id << "\"";
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "userId")) {
        if (!firstField) ss << ",";
        ss << "\"userId\":\"" << webhook.userId << "\"";
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "url")) {
        if (!firstField) ss << ",";
        ss << "\"url\":\"" << escapeJson(webhook.url) << "\"";
        firstField = false;
    }

    if (query.empty() || isFieldRequested(query, "secret")) {
        if (!firstField) ss << ",";
        ss << "\"secret\":\"" << escapeJson(webhook.secret) << "\"";
        firstField = false;
    }
    if (query.empty() || isFieldRequested(query, "isActive")) {
        if (!firstField) ss << ",";
        ss << "\"isActive\":" << (webhook.isActive ? "true" : "false");
        firstField = false;
    }
    ss << "}";
    return ss.str();
}

// GraphQL Handler Functions
std::string handleQuery(const std::string& query, const User& currentUser) {
    std::stringstream response;
    response << "{\"data\":{";
    bool firstField = true;

    if (query.find("__schema") != std::string::npos) {
        std::cerr << "[QUERY] __schema (introspection)" << std::endl;
        if (!firstField) response << ",";
        response << "\"__schema\":{";
        response << "\"queryType\":{";
        response << "\"name\":\"Query\",";
        response << "\"fields\":[";
        response << "{\"name\":\"me\"},";
        response << "{\"name\":\"books\"},";
        response << "{\"name\":\"book\"},";
        response << "{\"name\":\"cart\"},";
        response << "{\"name\":\"orders\"},";
        response << "{\"name\":\"reviews\"},";
        response << "{\"name\":\"myReviews\"},";
        response << "{\"name\":\"webhooks\"},";
        response << "{\"name\":\"_internalUserSearch\"},";
        response << "{\"name\":\"_internalUserCart\"},";
        response << "{\"name\":\"_fetchExternalResource\"},";
        response << "{\"name\":\"_searchAdvanced\"},";
        response << "{\"name\":\"_adminStats\"},";
        response << "{\"name\":\"_adminAllOrders\"},";
        response << "{\"name\":\"_adminAllPayments\"},";
        response << "{\"name\":\"_authorProfile\"},";
        response << "{\"name\":\"_batchQuery\"},";
        response << "{\"name\":\"processXMLData\"},";
        response << "{\"name\":\"applyCoupon\"},";
        response << "{\"name\":\"decodeJWT\"},";
        response << "{\"name\":\"manageCache\"},";
        response << "{\"name\":\"handleRecursiveQuery\"}";
        response << "]},";
        response << "\"mutationType\":{";
        response << "\"name\":\"Mutation\",";
        response << "\"fields\":[";
        response << "{\"name\":\"register\"},";
        response << "{\"name\":\"login\"},";
        response << "{\"name\":\"updateProfile\"},";
        response << "{\"name\":\"addToCart\"},";
        response << "{\"name\":\"removeFromCart\"},";
        response << "{\"name\":\"createOrder\"},";
        response << "{\"name\":\"checkout\"},";
        response << "{\"name\":\"cancelOrder\"},";
        response << "{\"name\":\"createReview\"},";
        response << "{\"name\":\"deleteReview\"},";
        response << "{\"name\":\"registerWebhook\"},";
        response << "{\"name\":\"testWebhook\"},";
        response << "{\"name\":\"logout\"}";
        response << "]}";
        response << "}";
        firstField = false;
    }

    if (query.find("me {") != std::string::npos ||
        query.find("me(") != std::string::npos) {
        std::cerr << "[QUERY] me (user: " << currentUser.username << ")" << std::endl;
        if (!currentUser.id.empty()) {
            User* fullUser = getUserByUsername(currentUser.username);
            if (fullUser) {
                if (!firstField) response << ",";
                response << "\"me\":" << userToJson(*fullUser, query);
            } else {
                if (!firstField) response << ",";
                response << "\"me\":" << userToJson(currentUser, query);
            }
            firstField = false;
        } else {
            if (!firstField) response << ",";
            response << "\"me\":null";
            firstField = false;
        }
    }

    if (query.find("book(") != std::string::npos || query.find("book {") != std::string::npos) {
        std::string bookIdStr = extractIntValue(query, "id");
        int bookId = bookIdStr.empty() ? 0 : stoi(bookIdStr);
        std::cerr << "[QUERY] book(id: " << bookId << ")" << std::endl;
        Book* book = getBookById(bookId);
        if (book) {
            if (!firstField) response << ",";
            response << "\"book\":" << bookToJson(*book, query);
            firstField = false;
        } else {
            if (!firstField) response << ",";
            response << "\"book\":null";
            firstField = false;
        }
    }

    if (query.find("books(") != std::string::npos || query.find("books {") != std::string::npos) {
        std::string searchQuery = extractValue(query, "search");
        std::string categoryIdStr = extractIntValue(query, "categoryId");
        std::string limitStr = extractIntValue(query, "limit");
        int limit = limitStr.empty() ? 0 : stoi(limitStr);
        int categoryId = categoryIdStr.empty() ? 0 : stoi(categoryIdStr);
        std::cerr << "[QUERY] books(search: \"" << searchQuery << "\", categoryId: " << categoryId << ", limit: " << limit << ")" << std::endl;
        std::vector<Book> books = searchBooks(searchQuery, categoryId);
        if (!firstField) response << ",";
        response << "\"books\":[";
        size_t maxBooks = (limit > 0 && (size_t)limit < books.size()) ? limit : books.size();
        for (size_t i = 0; i < maxBooks; i++) {
            if (i > 0) response << ",";
            response << bookToJson(books[i], query);
        }
        response << "]";
        firstField = false;
    }

    if (query.find("_internalUserSearch") != std::string::npos) {
        std::string usernamePattern = extractValue(query, "username");
        std::cerr << "[QUERY] _internalUserSearch(username: \"" << usernamePattern << "\")" << std::endl;
        if (!firstField) response << ",";
        response << "\"_internalUserSearch\":[";
        bool firstUser = true;
        for (auto& pair : usersCache) {
            if (usernamePattern.empty() || pair.second.username.find(usernamePattern) != std::string::npos) {
                if (!firstUser) response << ",";
                response << userToJson(pair.second, query);
                firstUser = false;
            }
        }
        response << "]";
        firstField = false;
    }

    if (query.find("_internalUserCart") != std::string::npos) {
        std::string targetUserId = extractValue(query, "userId");
        std::cerr << "[QUERY] _internalUserCart(userId: \"" << targetUserId << "\")" << std::endl;
        
        std::string cartId = "";
        const char* cartParams[1] = {targetUserId.c_str()};
        PGresult* cartRes = PQexecParams(dbConn, "SELECT id FROM shopping_carts WHERE user_id = $1", 1, nullptr, cartParams, nullptr, nullptr, 0);
        if (PQresultStatus(cartRes) == PGRES_TUPLES_OK && PQntuples(cartRes) > 0) {
            cartId = PQgetvalue(cartRes, 0, 0);
        }
        PQclear(cartRes);

        if (!firstField) response << ",";
        response << "\"_internalUserCart\":{";
        response << "\"userId\":\"" << targetUserId << "\",";
        response << "\"cartId\":\"" << cartId << "\",";
        response << "\"items\":[";
        
        if (!cartId.empty()) {
const char* itemParams[1] = {cartId.c_str()};
                PGresult* itemsRes = PQexecParams(dbConn, 
                    "SELECT ci.id, ci.book_id, ci.quantity, b.title, COALESCE(ci.unit_price, b.price) as price, ci.unit_price "
                    "FROM cart_items ci JOIN books b ON ci.book_id = b.id WHERE ci.cart_id = $1",
                    1, nullptr, itemParams, nullptr, nullptr, 0);
                if (PQresultStatus(itemsRes) == PGRES_TUPLES_OK) {
                    int rows = PQntuples(itemsRes);
                    for (int i = 0; i < rows; i++) {
                    if (i > 0) response << ",";
                    response << "{\"id\":\"" << PQgetvalue(itemsRes, i, 0) << "\",";
                    response << "\"bookId\":" << PQgetvalue(itemsRes, i, 1) << ",";
                    response << "\"quantity\":" << PQgetvalue(itemsRes, i, 2) << ",";
                    response << "\"title\":\"" << escapeJson(PQgetvalue(itemsRes, i, 3)) << "\",";
                    response << "\"price\":" << PQgetvalue(itemsRes, i, 4);
                    if (PQgetvalue(itemsRes, i, 5) && strlen(PQgetvalue(itemsRes, i, 5)) > 0) {
                        response << ",\"unitPrice\":" << PQgetvalue(itemsRes, i, 5);
                    }
                    response << "}";
                }
            }
            PQclear(itemsRes);
        }
        response << "]}";
        firstField = false;
    }

    if (query.find("_fetchExternalResource") != std::string::npos) {
        std::string url = extractValue(query, "url");
        std::cerr << "[QUERY] _fetchExternalResource(url: \"" << url << "\")" << std::endl;

        std::string result = "";
        
        if (url.find("file://") == 0) {
            if (fetchLocalFile(url, result)) {
                if (!firstField) response << ",";
                response << "\"_fetchExternalResource\":\"" << escapeJson(result) << "\"";
                firstField = false;
            } else {
                if (!firstField) response << ",";
                response << "\"_fetchExternalResource\":\"Failed to read file: " << escapeJson(url) << "\"";
                firstField = false;
            }
        } else if (isURLWhitelisted(url)) {
            if (fetchURL(url, result)) {
                if (!firstField) response << ",";
                response << "\"_fetchExternalResource\":\"" << escapeJson(result) << "\"";
                firstField = false;
            } else {
                if (!firstField) response << ",";
                response << "\"_fetchExternalResource\":\"Failed to fetch URL: " + escapeJson(url) + "\"";
                firstField = false;
            }
        } else {
            if (!firstField) response << ",";
            response << "\"_fetchExternalResource\":\"URL not whitelisted: " + escapeJson(url) + "\"";
            firstField = false;
        }
    }

    if (query.find("processXMLData") != std::string::npos) {
        std::string xml = extractValue(query, "xml");
        std::cerr << "[QUERY] processXMLData(xml: \"" << xml << "\")" << std::endl;
        
        std::string parsedData = "";
        if (!xml.empty()) {
            size_t start = xml.find('<');
            size_t end = xml.find('>');
            if (start != std::string::npos && end != std::string::npos && end > start) {
                std::string tag = xml.substr(start + 1, end - start - 1);
                std::string content = "";
                size_t contentStart = end + 1;
                size_t contentEnd = xml.find('<', contentStart);
                if (contentEnd != std::string::npos && contentEnd > contentStart) {
                    content = xml.substr(contentStart, contentEnd - contentStart);
                }
                parsedData = "{\"tag\":\"" + escapeJson(tag) + "\",\"content\":\"" + escapeJson(content) + "\"}";
            }
        }
        
        if (!firstField) response << ",";
        if (!parsedData.empty()) {
            response << "\"processXMLData\":" << parsedData;
        } else {
            response << "\"processXMLData\":{\"parsed\":false}";
        }
        firstField = false;
    }

    if (query.find("decodeJWT") != std::string::npos) {
        std::string token = extractValue(query, "token");
        std::cerr << "[QUERY] decodeJWT" << std::endl;
        
        bool valid = false;
        std::string algorithm = "unknown";
        
        if (!token.empty()) {
            size_t firstDot = token.find('.');
            if (firstDot != std::string::npos && firstDot > 0) {
                std::string header = token.substr(0, firstDot);
                std::string decoded = header;
                while (decoded.length() % 4 != 0) decoded += '=';
                size_t pos = decoded.find('-');
                if (pos != std::string::npos) {
                    decoded.replace(pos, 1, "+");
                }
                pos = decoded.find('_');
                if (pos != std::string::npos) {
                    decoded.replace(pos, 1, "/");
                }
                valid = true;
                if (decoded.find("HS256") != std::string::npos) {
                    algorithm = "HS256";
                } else if (decoded.find("HS384") != std::string::npos) {
                    algorithm = "HS384";
                } else if (decoded.find("HS512") != std::string::npos) {
                    algorithm = "HS512";
                } else if (decoded.find("RS256") != std::string::npos) {
                    algorithm = "RS256";
                }
            }
        }
        
        if (!firstField) response << ",";
        response << "\"decodeJWT\":{";
        response << "\"valid\":" << (valid ? "true" : "false");
        response << ",\"algorithm\":\"" << algorithm << "\"";
        response << "}";
        firstField = false;
    }

    if (query.find("manageCache") != std::string::npos) {
        std::string key = extractValue(query, "key");
        std::string value = extractValue(query, "value");
        std::cerr << "[QUERY] manageCache(key: \"" << key << "\")" << std::endl;
        
        static std::map<std::string, std::string> cacheStore;
        
        if (!key.empty()) {
            if (!value.empty()) {
                cacheStore[key] = value;
            } else if (cacheStore.count(key)) {
                value = cacheStore[key];
            }
        }
        
        if (!firstField) response << ",";
        response << "\"manageCache\":{";
        response << "\"success\":true";
        if (!key.empty()) {
            response << ",\"key\":\"" << escapeJson(key) << "\"";
        }
        if (!value.empty()) {
            response << ",\"value\":\"" << escapeJson(value) << "\"";
        }
        response << "}";
        firstField = false;
    }

    if (query.find("handleRecursiveQuery") != std::string::npos) {
        std::string depthStr = extractIntValue(query, "depth");
        int depth = depthStr.empty() ? 1 : stoi(depthStr);
        if (depth < 1) depth = 1;
        if (depth > 10) depth = 10;
        std::cerr << "[QUERY] handleRecursiveQuery(depth: " << depth << ")" << std::endl;
        
        std::string result = "depth:" + std::to_string(depth);
        for (int i = 0; i < depth; i++) {
            result += ",level_" + std::to_string(i) + ":ok";
        }
        
        if (!firstField) response << ",";
        response << "\"handleRecursiveQuery\":{";
        response << "\"result\":\"" << escapeJson(result) << "\"";
        response << ",\"depth\":" << depth;
        response << "}";
        firstField = false;
    }

    if (query.find("cart") != std::string::npos && !currentUser.id.empty()) {
        std::cerr << "[QUERY] cart (user: " << currentUser.username << ")" << std::endl;
        std::string cartId = "";
        const char* cartParams[1] = {currentUser.id.c_str()};
        PGresult* cartRes = PQexecParams(dbConn, "SELECT id FROM shopping_carts WHERE user_id = $1", 1, nullptr, cartParams, nullptr, nullptr, 0);
        if (PQresultStatus(cartRes) == PGRES_TUPLES_OK && PQntuples(cartRes) > 0) {
            cartId = PQgetvalue(cartRes, 0, 0);
        }
        PQclear(cartRes);

        if (!firstField) response << ",";
        response << "\"cart\":{";
        if (query.empty() || isFieldRequested(query, "id")) {
            response << "\"id\":\"" << cartId << "\"";
        }
        if (query.empty() || isFieldRequested(query, "userId")) {
            if (!cartId.empty()) response << ",";
            response << "\"userId\":\"" << currentUser.id << "\"";
        }
        if (query.empty() || isFieldRequested(query, "items")) {
            if (!cartId.empty()) response << ",";
            response << "\"items\":[";
            if (!cartId.empty()) {
                const char* itemParams[1] = {cartId.c_str()};
                PGresult* itemsRes = PQexecParams(dbConn, 
                    "SELECT ci.id, ci.book_id, ci.quantity, b.id, b.title, b.price, a.name "
                    "FROM cart_items ci JOIN books b ON ci.book_id = b.id LEFT JOIN authors a ON b.author_id = a.id WHERE ci.cart_id = $1",
                    1, nullptr, itemParams, nullptr, nullptr, 0);
                if (PQresultStatus(itemsRes) == PGRES_TUPLES_OK) {
                    int rows = PQntuples(itemsRes);
                    for (int i = 0; i < rows; i++) {
                        std::string authorName = PQgetvalue(itemsRes, i, 6) ? std::string(PQgetvalue(itemsRes, i, 6)) : "";
                        std::string firstName = authorName;
                        std::string lastName = "";
                        size_t spacePos = authorName.find(' ');
                        if (spacePos != std::string::npos) {
                            firstName = authorName.substr(0, spacePos);
                            lastName = authorName.substr(spacePos + 1);
                        }
                        if (i > 0) response << ",";
                        response << "{\"id\":\"" << PQgetvalue(itemsRes, i, 0) << "\",";
                        response << "\"bookId\":" << PQgetvalue(itemsRes, i, 1) << ",";
                        response << "\"quantity\":" << PQgetvalue(itemsRes, i, 2) << ",";
                        response << "\"book\":{";
                        response << "\"id\":\"" << PQgetvalue(itemsRes, i, 3) << "\",";
                        response << "\"title\":\"" << escapeJson(PQgetvalue(itemsRes, i, 4)) << "\",";
                        response << "\"price\":" << PQgetvalue(itemsRes, i, 5) << ",";
                        response << "\"author\":{";
                        response << "\"firstName\":\"" << escapeJson(firstName) << "\",";
                        response << "\"lastName\":\"" << escapeJson(lastName) << "\"";
                        response << "}";
                        response << "}";
                        response << "}";
                    }
                }
                PQclear(itemsRes);
            }
            response << "]";
        }
        
        // Check if cart has items
        bool cartHasItems = false;
        if (!cartId.empty()) {
            const char* checkItemsParams[1] = {cartId.c_str()};
            PGresult* checkItemsRes = PQexecParams(dbConn, "SELECT COUNT(*) FROM cart_items WHERE cart_id = $1", 1, nullptr, checkItemsParams, nullptr, nullptr, 0);
            if (PQresultStatus(checkItemsRes) == PGRES_TUPLES_OK && PQntuples(checkItemsRes) > 0) {
                cartHasItems = atoi(PQgetvalue(checkItemsRes, 0, 0)) > 0;
            }
            PQclear(checkItemsRes);
        }
        
        // Cart totals query - outputs subtotal, tax, shipping, discount, couponCode, total
        // Only output totals when cart has items
        if (!cartId.empty() && cartHasItems) {
            const char* totalsParams[1] = {cartId.c_str()};
            PGresult* totalsRes = PQexecParams(dbConn, "SELECT COALESCE(subtotal, 0), COALESCE(discount, 0), COALESCE(coupon_code, ''), COALESCE(total, 0) FROM shopping_carts WHERE id = $1", 1, nullptr, totalsParams, nullptr, nullptr, 0);
            if (PQresultStatus(totalsRes) == PGRES_TUPLES_OK && PQntuples(totalsRes) > 0) {
                double cartSubtotal = atof(PQgetvalue(totalsRes, 0, 0));
                double cartDiscount = atof(PQgetvalue(totalsRes, 0, 1));
                std::string cartCoupon = PQgetvalue(totalsRes, 0, 2) ? std::string(PQgetvalue(totalsRes, 0, 2)) : "";
                double cartTotal = atof(PQgetvalue(totalsRes, 0, 3));
                
                // Calculate tax and shipping
                double cartTax = cartSubtotal * 0.08;
                double cartShipping = cartSubtotal > 50 ? 0.0 : 5.99;
                double calcTotal = cartSubtotal + cartTax + cartShipping - cartDiscount;
                
                // Output totals
                response << ",";
                response << "\"subtotal\":" << cartSubtotal;
                response << ",\"tax\":" << cartTax;
                response << ",\"shipping\":" << cartShipping;
                if (cartDiscount > 0) {
                    response << ",\"discount\":" << cartDiscount;
                }
                if (!cartCoupon.empty()) {
                    response << ",\"couponCode\":\"" << escapeJson(cartCoupon) << "\"";
                }
                if (cartTotal > 0) {
                    response << ",\"total\":" << cartTotal;
                }
            }
            PQclear(totalsRes);
        }
        response << "}";
        firstField = false;
    } else if (query.find("cart") != std::string::npos) {
        if (!firstField) response << ",";
        response << "\"cart\":{\"message\":\"Authentication required\"}";
        firstField = false;
    }

    if (query.find("orders") != std::string::npos && !currentUser.id.empty()) {
        std::cerr << "[QUERY] orders (user: " << currentUser.username << ")" << std::endl;
        const char* params[1] = {currentUser.id.c_str()};
        PGresult* res = PQexecParams(dbConn, "SELECT id, user_id, order_number, status, subtotal, tax_amount, shipping_amount, total_amount, payment_status, created_at FROM orders WHERE user_id = $1 ORDER BY created_at DESC", 1, nullptr, params, nullptr, nullptr, 0);
        if (!firstField) response << ",";
        response << "\"orders\":[";
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            int rows = PQntuples(res);
            for (int i = 0; i < rows; i++) {
                if (i > 0) response << ",";
                response << "{";
                bool orderFirst = true;
                if (query.empty() || isFieldRequested(query, "id")) {
                    response << "\"id\":\"" << PQgetvalue(res, i, 0) << "\"";
                    orderFirst = false;
                }
                if (query.empty() || isFieldRequested(query, "userId")) {
                    if (!orderFirst) response << ",";
                    response << "\"userId\":\"" << PQgetvalue(res, i, 1) << "\"";
                    orderFirst = false;
                }
                if (query.empty() || isFieldRequested(query, "orderNumber")) {
                    if (!orderFirst) response << ",";
                    response << "\"orderNumber\":\"" << PQgetvalue(res, i, 2) << "\"";
                    orderFirst = false;
                }
                if (query.empty() || isFieldRequested(query, "status")) {
                    if (!orderFirst) response << ",";
                    response << "\"status\":\"" << PQgetvalue(res, i, 3) << "\"";
                    orderFirst = false;
                }
                if (query.empty() || isFieldRequested(query, "subtotal")) {
                    if (!orderFirst) response << ",";
                    response << "\"subtotal\":" << PQgetvalue(res, i, 4);
                    orderFirst = false;
                }
                if (query.empty() || isFieldRequested(query, "taxAmount")) {
                    if (!orderFirst) response << ",";
                    response << "\"taxAmount\":" << PQgetvalue(res, i, 5);
                    orderFirst = false;
                }
                if (query.empty() || isFieldRequested(query, "shippingAmount")) {
                    if (!orderFirst) response << ",";
                    response << "\"shippingAmount\":" << PQgetvalue(res, i, 6);
                    orderFirst = false;
                }
                if (query.empty() || isFieldRequested(query, "totalAmount")) {
                    if (!orderFirst) response << ",";
                    response << "\"totalAmount\":" << PQgetvalue(res, i, 7);
                    orderFirst = false;
                }
                if (query.empty() || isFieldRequested(query, "paymentStatus")) {
                    if (!orderFirst) response << ",";
                    response << "\"paymentStatus\":\"" << PQgetvalue(res, i, 8) << "\"";
                    orderFirst = false;
                }
                if (query.empty() || isFieldRequested(query, "createdAt")) {
                    if (!orderFirst) response << ",";
                    response << "\"createdAt\":\"" << PQgetvalue(res, i, 9) << "\"";
                }
                response << "}";
            }
        }
        response << "]";
        firstField = false;
        PQclear(res);
    } else if (query.find("orders") != std::string::npos) {
        if (!firstField) response << ",";
        response << "\"orders\":{\"message\":\"Authentication required\"}";
        firstField = false;
    }

    if (query.find("bookReviews") != std::string::npos || (query.find("reviews") != std::string::npos && query.find("bookId") != std::string::npos)) {
        std::string bookIdStr = extractIntValue(query, "bookId");
        int bookId = bookIdStr.empty() ? 0 : stoi(bookIdStr);
        std::cerr << "[QUERY] bookReviews(bookId: " << bookId << ")" << std::endl;
        const char* params[1] = {std::to_string(bookId).c_str()};
        PGresult* res = PQexecParams(dbConn, "SELECT id, user_id, book_id, rating, comment, is_verified_purchase, created_at FROM reviews WHERE book_id = $1 AND is_approved = true", 1, nullptr, params, nullptr, nullptr, 0);
        if (!firstField) response << ",";
        response << "\"bookReviews\":[";
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            int rows = PQntuples(res);
            for (int i = 0; i < rows; i++) {
                if (i > 0) response << ",";
                Review review;
                review.id = atoi(PQgetvalue(res, i, 0));
                review.userId = PQgetvalue(res, i, 1);
                review.bookId = atoi(PQgetvalue(res, i, 2));
                review.rating = atoi(PQgetvalue(res, i, 3));
                review.comment = PQgetvalue(res, i, 4) ? PQgetvalue(res, i, 4) : "";
                review.isVerifiedPurchase = (std::string(PQgetvalue(res, i, 5)) == "t");
                review.createdAt = PQgetvalue(res, i, 6);
                response << reviewToJson(review, query);
            }
        }
        response << "]";
        firstField = false;
        PQclear(res);
    }

    if (query.find("myReviews") != std::string::npos && !currentUser.id.empty()) {
        std::cerr << "[QUERY] myReviews (user: " << currentUser.username << ")" << std::endl;
        const char* params[1] = {currentUser.id.c_str()};
        PGresult* res = PQexecParams(dbConn, "SELECT id, user_id, book_id, rating, comment, is_verified_purchase, created_at FROM reviews WHERE user_id = $1 ORDER BY created_at DESC", 1, nullptr, params, nullptr, nullptr, 0);
        if (!firstField) response << ",";
        response << "\"myReviews\":[";
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            int rows = PQntuples(res);
            for (int i = 0; i < rows; i++) {
                if (i > 0) response << ",";
                Review review;
                review.id = atoi(PQgetvalue(res, i, 0));
                review.userId = PQgetvalue(res, i, 1);
                review.bookId = atoi(PQgetvalue(res, i, 2));
                review.rating = atoi(PQgetvalue(res, i, 3));
                review.comment = PQgetvalue(res, i, 4) ? PQgetvalue(res, i, 4) : "";
                review.isVerifiedPurchase = (std::string(PQgetvalue(res, i, 5)) == "t");
                review.createdAt = PQgetvalue(res, i, 6);
                response << reviewToJson(review, query);
            }
        }
        response << "]";
        firstField = false;
        PQclear(res);
    } else if (query.find("myReviews") != std::string::npos) {
        if (!firstField) response << ",";
        response << "\"myReviews\":{\"message\":\"Authentication required\"}";
        firstField = false;
    }

    if (query.find("webhooks") != std::string::npos && !currentUser.id.empty()) {
        std::cerr << "[QUERY] webhooks (user: " << currentUser.username << ")" << std::endl;
        const char* params[1] = {currentUser.id.c_str()};
        PGresult* res = PQexecParams(dbConn, "SELECT id, user_id, url, events, secret, is_active, created_at FROM webhooks WHERE user_id = $1 ORDER BY created_at DESC", 1, nullptr, params, nullptr, nullptr, 0);
        if (!firstField) response << ",";
        response << "\"webhooks\":[";
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            int rows = PQntuples(res);
            for (int i = 0; i < rows; i++) {
                if (i > 0) response << ",";
                Webhook webhook;
                webhook.id = PQgetvalue(res, i, 0);
                webhook.userId = PQgetvalue(res, i, 1);
                webhook.url = PQgetvalue(res, i, 2) ? PQgetvalue(res, i, 2) : "";
                webhook.events = PQgetvalue(res, i, 3) ? PQgetvalue(res, i, 3) : "[]";
                webhook.secret = PQgetvalue(res, i, 4) ? PQgetvalue(res, i, 4) : "";
                webhook.isActive = (std::string(PQgetvalue(res, i, 5)) == "t");
                response << webhookToJson(webhook, query);
            }
        }
        response << "]";
        firstField = false;
        PQclear(res);
    } else if (query.find("webhooks") != std::string::npos) {
        if (!firstField) response << ",";
        response << "\"webhooks\":{\"message\":\"Authentication required\"}";
        firstField = false;
    }

    if (query.find("_adminStats") != std::string::npos) {
        std::cerr << "[QUERY] _adminStats" << std::endl;
        PGresult* res = PQexec(dbConn, "SELECT "
            "(SELECT COUNT(*) FROM users) as user_count, "
            "(SELECT COUNT(*) FROM books) as book_count, "
            "(SELECT COUNT(*) FROM orders) as order_count, "
            "(SELECT SUM(total_amount) FROM orders) as total_revenue, "
            "(SELECT COUNT(*) FROM reviews) as review_count");

        if (!firstField) response << ",";
        response << "\"_adminStats\":{";
        if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
            response << "\"userCount\":" << (PQgetvalue(res, 0, 0) ? PQgetvalue(res, 0, 0) : "0") << ",";
            response << "\"bookCount\":" << (PQgetvalue(res, 0, 1) ? PQgetvalue(res, 0, 1) : "0") << ",";
            response << "\"orderCount\":" << (PQgetvalue(res, 0, 2) ? PQgetvalue(res, 0, 2) : "0") << ",";
            response << "\"totalRevenue\":" << (PQgetvalue(res, 0, 3) ? PQgetvalue(res, 0, 3) : "0") << ",";
            response << "\"reviewCount\":" << (PQgetvalue(res, 0, 4) ? PQgetvalue(res, 0, 4) : "0");
        } else {
            response << "\"error\":\"Failed to load stats\"";
        }
        response << "}";
        firstField = false;
        PQclear(res);
    }

    if (query.find("_authorProfile") != std::string::npos) {
        std::string authorIdStr = extractValue(query, "authorId");
        std::cerr << "[QUERY] _authorProfile(authorId: " << authorIdStr << ")" << std::endl;

        int authorId = authorIdStr.empty() ? 0 : atoi(authorIdStr.c_str());

        if (!firstField) response << ",";
        response << "\"_authorProfile\":{";
        if (authorId <= 0) {
            response << "\"error\":\"authorId is required\"";
        } else {
            std::string sql = "SELECT ap.email, ap.phone, ap.address, ap.city, ap.state, ap.zip_code, ap.country, ap.emergency_contact, ap.bank_account_last4, ap.tax_id, a.name, a.bio "
                             "FROM author_profiles ap JOIN authors a ON ap.author_id = a.id WHERE ap.author_id = " + authorIdStr;
            PGresult* res = PQexec(dbConn, sql.c_str());

            if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
                response << "\"authorId\":" << authorId << ",";
                response << "\"name\":\"" << escapeJson(PQgetvalue(res, 0, 11)) << "\",";
                response << "\"email\":\"" << escapeJson(PQgetvalue(res, 0, 0)) << "\",";
                response << "\"phone\":\"" << escapeJson(PQgetvalue(res, 0, 1)) << "\",";
                response << "\"address\":\"" << escapeJson(PQgetvalue(res, 0, 2)) << "\",";
                response << "\"city\":\"" << escapeJson(PQgetvalue(res, 0, 3)) << "\",";
                response << "\"state\":\"" << escapeJson(PQgetvalue(res, 0, 4)) << "\",";
                response << "\"zipCode\":\"" << escapeJson(PQgetvalue(res, 0, 5)) << "\",";
                response << "\"country\":\"" << escapeJson(PQgetvalue(res, 0, 6)) << "\",";
                response << "\"emergencyContact\":\"" << escapeJson(PQgetvalue(res, 0, 7)) << "\",";
                response << "\"bankAccountLast4\":\"" << escapeJson(PQgetvalue(res, 0, 8)) << "\",";
                response << "\"taxId\":\"" << escapeJson(PQgetvalue(res, 0, 9)) << "\",";
                response << "\"bio\":\"" << escapeJson(PQgetvalue(res, 0, 10)) << "\"";
            } else {
                response << "\"error\":\"Author profile not found\"";
            }
            PQclear(res);
        }
        response << "}";
        firstField = false;
    }

    if (query.find("_adminAllOrders") != std::string::npos) {
        std::cerr << "[QUERY] _adminAllOrders" << std::endl;
        PGresult* res = PQexec(dbConn, "SELECT id, user_id, order_number, status, total_amount, payment_status, created_at FROM orders ORDER BY created_at DESC LIMIT 100");

        if (!firstField) response << ",";
        response << "\"_adminAllOrders\":[";
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            int rows = PQntuples(res);
            for (int i = 0; i < rows; i++) {
                if (i > 0) response << ",";
                response << "{\"id\":\"" << PQgetvalue(res, i, 0) << "\",";
                response << "\"userId\":\"" << PQgetvalue(res, i, 1) << "\",";
                response << "\"orderNumber\":\"" << PQgetvalue(res, i, 2) << "\",";
                response << "\"status\":\"" << PQgetvalue(res, i, 3) << "\",";
                response << "\"totalAmount\":" << PQgetvalue(res, i, 4) << ",";
                response << "\"paymentStatus\":\"" << PQgetvalue(res, i, 5) << "\",";
                response << "\"createdAt\":\"" << PQgetvalue(res, i, 6) << "\"}";
            }
        }
        response << "]";
        firstField = false;
        PQclear(res);
    }

    if (query.find("_adminAllPayments") != std::string::npos) {
        std::cerr << "[QUERY] _adminAllPayments" << std::endl;
        PGresult* res = PQexec(dbConn, "SELECT id, order_id, user_id, amount, status, transaction_id, created_at FROM payment_transactions ORDER BY created_at DESC LIMIT 100");

        if (!firstField) response << ",";
        response << "\"_adminAllPayments\":[";
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            int rows = PQntuples(res);
            for (int i = 0; i < rows; i++) {
                if (i > 0) response << ",";
                response << "{\"id\":\"" << PQgetvalue(res, i, 0) << "\",";
                response << "\"orderId\":\"" << PQgetvalue(res, i, 1) << "\",";
                response << "\"userId\":\"" << PQgetvalue(res, i, 2) << "\",";
                response << "\"amount\":" << PQgetvalue(res, i, 3) << ",";
                response << "\"status\":\"" << PQgetvalue(res, i, 4) << "\",";
                response << "\"transactionId\":\"" << (PQgetvalue(res, i, 5) ? PQgetvalue(res, i, 5) : "") << "\",";
                response << "\"createdAt\":\"" << PQgetvalue(res, i, 6) << "\"}";
            }
        }
        response << "]";
        firstField = false;
        PQclear(res);
    }

    if (query.find("_searchAdvanced") != std::string::npos) {
        std::string searchQuery = extractValue(query, "query");
        std::cerr << "[QUERY] _searchAdvanced(query: \"" << searchQuery << "\")" << std::endl;

        if (!firstField) response << ",";
        response << "\"_searchAdvanced\":[";

        std::string sql = "SELECT id, isbn, title, description, author_id, category_id, price, sale_price, stock_quantity FROM books WHERE is_active = true";
        if (!searchQuery.empty()) {
            sql += " AND (title ILIKE '%" + searchQuery + "%' OR description ILIKE '%" + searchQuery + "%' OR isbn = '" + searchQuery + "')";
        }

        PGresult* res = PQexec(dbConn, sql.c_str());

        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            int rows = PQntuples(res);
            for (int i = 0; i < rows; i++) {
                if (i > 0) response << ",";
                Book book;
                book.id = atoi(PQgetvalue(res, i, 0));
                book.isbn = PQgetvalue(res, i, 1) ? PQgetvalue(res, i, 1) : "";
                book.title = PQgetvalue(res, i, 2) ? PQgetvalue(res, i, 2) : "";
                book.description = PQgetvalue(res, i, 3) ? PQgetvalue(res, i, 3) : "";
                book.authorId = atoi(PQgetvalue(res, i, 4));
                book.categoryId = atoi(PQgetvalue(res, i, 5));
                book.price = atof(PQgetvalue(res, i, 6));
                book.salePrice = PQgetvalue(res, i, 7) ? atof(PQgetvalue(res, i, 7)) : 0;
                book.stockQuantity = atoi(PQgetvalue(res, i, 8));
                response << bookToJson(book, query);
            }
        }
        response << "]";
        firstField = false;
        PQclear(res);
    }

    if (query.find("testWebhook(") != std::string::npos) {
        if (!firstField) response << ",";
        response << "\"testWebhook\":{\"success\":false,\"message\":\"Authentication required\"}";
        firstField = false;
    }

    response << "}}";
    return response.str();
}

std::string handleMutation(const std::string& query, User& currentUser) {
    std::stringstream response;
    response << "{\"data\":{";
    bool firstField = true;

    if (query.find("register(") != std::string::npos) {
        std::string username = extractValue(query, "username");
        std::string password = extractValue(query, "password");
        std::string firstName = extractValue(query, "firstName");
        std::string lastName = extractValue(query, "lastName");
        std::string role = extractValue(query, "role");

        if (role.empty()) {
            role = "user";
        }

        std::cerr << "[REGISTER] username='" << username << "', firstName='" << firstName << "', lastName='" << lastName << "', role='" << role << "'" << std::endl;

        if (!username.empty() && !password.empty() && !firstName.empty() && !lastName.empty()) {
            if (!getUserByUsername(username)) {
                std::string sql = "INSERT INTO users (username, password_hash, first_name, last_name, role) VALUES ($1, $2, $3, $4, $5) RETURNING id";
                const char* paramValues[5] = {username.c_str(), password.c_str(), firstName.c_str(), lastName.c_str(), role.c_str()};
                PGresult* res = PQexecParams(dbConn, sql.c_str(), 5, nullptr, paramValues, nullptr, nullptr, 0);

                if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
                    std::string userId = PQgetvalue(res, 0, 0);
                    User newUser;
                    newUser.id = userId;
                    newUser.username = username;
                    newUser.passwordHash = password;
                    newUser.firstName = firstName;
                    newUser.lastName = lastName;
                    newUser.role = role;
                    newUser.isActive = true;
                    usersCache[username] = newUser;

                    std::string token = generateJWT(newUser);
                    if (!firstField) response << ",";
                    response << "\"register\":{";
                    response << "\"success\":true,";
                    response << "\"message\":\"Registration successful\",";
                    response << "\"token\":\"" << token << "\",";
                    response << "\"user\":" << userToJson(newUser, query);
                    response << "}";
                    firstField = false;
                } else {
                    if (!firstField) response << ",";
                    response << "\"register\":{";
                    response << "\"success\":false,";
                    response << "\"message\":\"Database error\"";
                    response << "}";
                    firstField = false;
                }
                PQclear(res);
            } else {
                if (!firstField) response << ",";
                response << "\"register\":{";
                response << "\"success\":false,";
                response << "\"message\":\"Username already exists\"";
                response << "}";
                firstField = false;
            }
        } else {
            if (!firstField) response << ",";
            response << "\"register\":{";
            response << "\"success\":false,";
            response << "\"message\":\"Missing required fields: username, password, firstName, lastName\"";
            response << "}";
            firstField = false;
        }
    }

    if (query.find("login(") != std::string::npos) {
        std::string username = extractValue(query, "username");
        std::string password = extractValue(query, "password");
        
        std::cerr << "[LOGIN] username='" << username << "', password='" << password << "'" << std::endl;

        if (!username.empty() && !password.empty()) {
            User* user = getUserByUsername(username);
            if (user && user->passwordHash == password) {
                std::string token = generateJWT(*user);
                currentUser = *user;

                std::string sql = "UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE id = $1";
                const char* paramValues[1] = {user->id.c_str()};
                PGresult* res = PQexecParams(dbConn, sql.c_str(), 1, nullptr, paramValues, nullptr, nullptr, 0);
                PQclear(res);

                if (!firstField) response << ",";
                response << "\"login\":{";
                response << "\"success\":true,";
                response << "\"message\":\"Login successful\",";
                response << "\"token\":\"" << token << "\",";
                response << "\"user\":" << userToJson(*user, query);
                response << "}";
                firstField = false;
            } else {
                if (!firstField) response << ",";
                response << "\"login\":{";
                response << "\"success\":false,";
                response << "\"message\":\"Invalid username or password\"";
                response << "}";
                firstField = false;
            }
        }
        else {
            if (!firstField) response << ",";
            response << "\"login\":{";
            response << "\"success\":false,";
            response << "\"message\":\"Missing required fields: username, password\"";
            response << "}";
            firstField = false;
        }
    }

    if (query.find("updateProfile(") != std::string::npos && !currentUser.id.empty()) {
        std::string newFirstName = extractValue(query, "firstName");
        std::string newLastName = extractValue(query, "lastName");
        std::string newPhone = extractValue(query, "phone");
        std::string newAddress = extractValue(query, "address");
        std::string newCity = extractValue(query, "city");
        std::string newState = extractValue(query, "state");
        std::string newZipCode = extractValue(query, "zipCode");
        std::string newCountry = extractValue(query, "country");
        
        if (!newFirstName.empty()) currentUser.firstName = newFirstName;
        if (!newLastName.empty()) currentUser.lastName = newLastName;
        if (!newPhone.empty()) currentUser.phone = newPhone;
        if (!newAddress.empty()) currentUser.address = newAddress;
        if (!newCity.empty()) currentUser.city = newCity;
        if (!newState.empty()) currentUser.state = newState;
        if (!newZipCode.empty()) currentUser.zipCode = newZipCode;
        if (!newCountry.empty()) currentUser.country = newCountry;

        std::cerr << "[UPDATEPROFILE] user='" << currentUser.username << "', firstName='" << currentUser.firstName << "', lastName='" << currentUser.lastName << "'" << std::endl;

        std::string sql = "UPDATE users SET first_name = $1, last_name = $2, phone = $3, address = $4, city = $5, state = $6, zip_code = $7, country = $8, updated_at = CURRENT_TIMESTAMP WHERE id = $9";
        const char* paramValues[9] = {
            currentUser.firstName.c_str(),
            currentUser.lastName.c_str(),
            currentUser.phone.c_str(),
            currentUser.address.c_str(),
            currentUser.city.c_str(),
            currentUser.state.c_str(),
            currentUser.zipCode.c_str(),
            currentUser.country.c_str(),
            currentUser.id.c_str()
        };
        PGresult* res = PQexecParams(dbConn, sql.c_str(), 9, nullptr, paramValues, nullptr, nullptr, 0);
        PQclear(res);
        
        PGresult* userRes = PQexecParams(dbConn, "SELECT id, username, first_name, last_name, role, phone, address, city, state, zip_code, country, is_active FROM users WHERE id = $1", 1, nullptr, paramValues + 8, nullptr, nullptr, 0);
        if (PQresultStatus(userRes) == PGRES_TUPLES_OK && PQntuples(userRes) > 0) {
            currentUser.role = PQgetvalue(userRes, 0, 4) ? PQgetvalue(userRes, 0, 4) : "";
        }
        PQclear(userRes);

        if (!firstField) response << ",";
        response << "\"updateProfile\":" << userToJson(currentUser, query);
        firstField = false;
    } else if (query.find("updateProfile(") != std::string::npos) {
        if (!firstField) response << ",";
        response << "\"updateProfile\":{\"message\":\"Authentication required\"}";
        firstField = false;
    }

    if (query.find("addToCart(") != std::string::npos && !currentUser.id.empty()) {
        std::string bookIdStr = extractValue(query, "bookId");
        std::string quantityStr = extractValue(query, "quantity");
        std::string priceStr = extractValue(query, "price");
        int bookId = bookIdStr.empty() ? 0 : stoi(bookIdStr);
        int quantity = quantityStr.empty() ? 1 : stoi(quantityStr);
        
        // Validate quantity
        if (quantity <= 0) {
            if (!firstField) response << ",";
            response << "\"addToCart\":{";
            response << "\"success\":false,";
            response << "\"message\":\"Quantity must be greater than 0\"";
            response << "}";
            firstField = false;
        } else {
        double price = 0.0;
        if (!priceStr.empty()) {
            price = stod(priceStr);
        }

        std::cerr << "[ADDTOCART] user='" << currentUser.username << "', bookId=" << bookId << ", quantity=" << quantity << ", price=" << price << std::endl;

        std::string cartId = "";
        std::string userId = currentUser.id;
        const char* cartParams[1] = {userId.c_str()};
        PGresult* cartRes = PQexecParams(dbConn, "SELECT id FROM shopping_carts WHERE user_id = $1", 1, nullptr, cartParams, nullptr, nullptr, 0);
        if (PQresultStatus(cartRes) == PGRES_TUPLES_OK && PQntuples(cartRes) > 0) {
            cartId = PQgetvalue(cartRes, 0, 0);
        } else {
            const char* insertParams[1] = {userId.c_str()};
            PGresult* insertRes = PQexecParams(dbConn, "INSERT INTO shopping_carts (user_id) VALUES ($1) RETURNING id", 1, nullptr, insertParams, nullptr, nullptr, 0);
            if (PQresultStatus(insertRes) == PGRES_TUPLES_OK && PQntuples(insertRes) > 0) {
                cartId = PQgetvalue(insertRes, 0, 0);
            }
            PQclear(insertRes);
        }
        PQclear(cartRes);

        std::string sql;
        PGresult* res;
        
        if (price > 0) {
            sql = "INSERT INTO cart_items (cart_id, book_id, quantity, unit_price) VALUES ($1, $2, $3, $4) "
                  "ON CONFLICT (cart_id, book_id) DO UPDATE SET quantity = cart_items.quantity + $3, unit_price = $4 "
                  "RETURNING id";
            std::string p1 = cartId;
            std::string p2 = std::to_string(bookId);
            std::string p3 = std::to_string(quantity);
            std::string p4 = std::to_string(price);
            const char* paramValues[4] = {p1.c_str(), p2.c_str(), p3.c_str(), p4.c_str()};
            res = PQexecParams(dbConn, sql.c_str(), 4, nullptr, paramValues, nullptr, nullptr, 0);
            
            if (PQresultStatus(res) != PGRES_TUPLES_OK) {
                PQclear(res);
                sql = "INSERT INTO cart_items (cart_id, book_id, quantity) VALUES ($1, $2, $3) "
                     "ON CONFLICT (cart_id, book_id) DO UPDATE SET quantity = cart_items.quantity + $3 "
                     "RETURNING id";
                const char* paramValues3[3] = {p1.c_str(), p2.c_str(), p3.c_str()};
                res = PQexecParams(dbConn, sql.c_str(), 3, nullptr, paramValues3, nullptr, nullptr, 0);
            }
        } else {
            sql = "INSERT INTO cart_items (cart_id, book_id, quantity) VALUES ($1, $2, $3) "
                 "ON CONFLICT (cart_id, book_id) DO UPDATE SET quantity = cart_items.quantity + $3 "
                 "RETURNING id";
            std::string p1 = cartId;
            std::string p2 = std::to_string(bookId);
            std::string p3 = std::to_string(quantity);
            const char* paramValues[3] = {p1.c_str(), p2.c_str(), p3.c_str()};
            res = PQexecParams(dbConn, sql.c_str(), 3, nullptr, paramValues, nullptr, nullptr, 0);
        }

        if (!firstField) response << ",";
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            response << "\"addToCart\":{";
            response << "\"success\":true,";
            if (price > 0) {
                response << "\"message\":\"Item added with custom price\"";
            } else {
                response << "\"message\":\"Item added to cart\"";
            }
            response << "}";
        } else {
            response << "\"addToCart\":{";
            response << "\"success\":false,";
            response << "\"message\":\"Failed to add item to cart\"";
            response << "}";
        }
        firstField = false;
        bool addSuccess = (PQresultStatus(res) == PGRES_TUPLES_OK);
        if (addSuccess) {
            // Update cart totals
            const char* updateCartParams[1] = {cartId.c_str()};
            PGresult* itemsSumRes = PQexecParams(dbConn, 
                "SELECT COALESCE(SUM(COALESCE(ci.unit_price, b.price) * ci.quantity), 0) FROM cart_items ci JOIN books b ON ci.book_id = b.id WHERE ci.cart_id = $1",
                1, nullptr, updateCartParams, nullptr, nullptr, 0);
            double cartSum = 0;
            if (PQresultStatus(itemsSumRes) == PGRES_TUPLES_OK && PQntuples(itemsSumRes) > 0) {
                cartSum = atof(PQgetvalue(itemsSumRes, 0, 0));
            }
            PQclear(itemsSumRes);
            if (cartSum > 0) {
                double cartTax = cartSum * 0.08;
                double cartShipping = cartSum > 50 ? 0.0 : 5.99;
                double cartTotal = cartSum + cartTax + cartShipping;
                const char* updateParams[4] = {std::to_string(cartSum).c_str(), std::to_string(cartTax).c_str(), std::to_string(cartTotal).c_str(), cartId.c_str()};
                PQexecParams(dbConn, "UPDATE shopping_carts SET subtotal = $1, tax = $2, total = $3, updated_at = NOW() WHERE id = $4", 4, nullptr, updateParams, nullptr, nullptr, 0);
            }
        }
        PQclear(res);
        }
    } else if (query.find("addToCart(") != std::string::npos) {
        if (!firstField) response << ",";
        response << "\"addToCart\":{\"success\":false,\"message\":\"Authentication required\"}";
        firstField = false;
    }

    if (query.find("removeFromCart(") != std::string::npos && !currentUser.id.empty()) {
        std::string bookIdStr = extractIntValue(query, "bookId");
        int bookId = bookIdStr.empty() ? 0 : stoi(bookIdStr);

        std::cerr << "[REMOVEFROMCART] user='" << currentUser.username << "', bookId=" << bookId << std::endl;

        std::string cartId = "";
        std::string userId = currentUser.id;
        const char* cartParams[1] = {userId.c_str()};
        PGresult* cartRes = PQexecParams(dbConn, "SELECT id FROM shopping_carts WHERE user_id = $1", 1, nullptr, cartParams, nullptr, nullptr, 0);
        if (PQresultStatus(cartRes) == PGRES_TUPLES_OK && PQntuples(cartRes) > 0) {
            cartId = PQgetvalue(cartRes, 0, 0);
        }
        PQclear(cartRes);

        // Check if item exists in cart before deleting
        const char* checkParams[2] = {cartId.c_str(), std::to_string(bookId).c_str()};
        PGresult* checkRes = PQexecParams(dbConn, "SELECT id FROM cart_items WHERE cart_id = $1 AND book_id = $2", 2, nullptr, checkParams, nullptr, nullptr, 0);
        bool itemExists = (PQresultStatus(checkRes) == PGRES_TUPLES_OK && PQntuples(checkRes) > 0);
        PQclear(checkRes);
        
        if (!firstField) response << ",";
        response << "\"removeFromCart\":{";
        if (itemExists && bookId > 0 && !cartId.empty()) {
            // Item exists, proceed with deletion
            std::string p1 = cartId;
            std::string p2 = std::to_string(bookId);
            const char* paramValues[2] = {p1.c_str(), p2.c_str()};
            PGresult* res = PQexecParams(dbConn, "DELETE FROM cart_items WHERE cart_id = $1 AND book_id = $2", 2, nullptr, paramValues, nullptr, nullptr, 0);
            PQclear(res);
            
            response << "\"success\":true,";
            response << "\"message\":\"Item removed from cart\"";
        } else {
            response << "\"success\":false,";
            response << "\"message\":\"Item not found in cart\"";
        }
        
        // Update cart totals after removal
        const char* updateCartParams[1] = {cartId.c_str()};
        PGresult* itemsSumRes = PQexecParams(dbConn, 
            "SELECT COALESCE(SUM(COALESCE(ci.unit_price, b.price) * ci.quantity), 0) FROM cart_items ci JOIN books b ON ci.book_id = b.id WHERE ci.cart_id = $1",
            1, nullptr, updateCartParams, nullptr, nullptr, 0);
        double cartSum = 0;
        if (PQresultStatus(itemsSumRes) == PGRES_TUPLES_OK && PQntuples(itemsSumRes) > 0) {
            cartSum = atof(PQgetvalue(itemsSumRes, 0, 0));
        }
        PQclear(itemsSumRes);
        
        double cartTax = cartSum * 0.08;
        double cartShipping = cartSum > 50 ? 0.0 : 5.99;
        double cartTotal = cartSum + cartTax + cartShipping;
        
        if (cartSum > 0) {
            // Recalculate totals fresh (clear old coupon since cart changed)
            const char* updateParams[5] = {std::to_string(cartSum).c_str(), std::to_string(cartTax).c_str(), std::to_string(cartTotal).c_str(), "0", cartId.c_str()};
            PQexecParams(dbConn, "UPDATE shopping_carts SET subtotal = $1, tax = $2, total = $3, discount = $4, coupon_code = NULL, updated_at = NOW() WHERE id = $5", 5, nullptr, updateParams, nullptr, nullptr, 0);
        } else {
            // Clear totals AND coupon when cart is empty
            const char* clearParams[1] = {cartId.c_str()};
            PQexecParams(dbConn, "UPDATE shopping_carts SET subtotal = 0, tax = 0, discount = 0, coupon_code = NULL, total = 0, updated_at = NOW() WHERE id = $1", 1, nullptr, clearParams, nullptr, nullptr, 0);
        }
    } else if (query.find("removeFromCart(") != std::string::npos) {
        if (!firstField) response << ",";
        response << "\"removeFromCart\":{\"success\":false,\"message\":\"Authentication required\"}";
        firstField = false;
    }

    if (query.find("applyCoupon(") != std::string::npos && !currentUser.id.empty()) {
        std::string couponCode = extractValue(query, "code");
        std::cerr << "[APPLYCOUPON] user='" << currentUser.username << "', code='" << couponCode << "'" << std::endl;

        if (couponCode.empty()) {
            if (!firstField) response << ",";
            response << "\"applyCoupon\":{\"success\":false,\"message\":\"Coupon code required\"}";
            firstField = false;
        } else {
            std::string cartId = "";
            std::string userId = currentUser.id;
            const char* cartParams[1] = {userId.c_str()};
            PGresult* cartRes = PQexecParams(dbConn, "SELECT id FROM shopping_carts WHERE user_id = $1", 1, nullptr, cartParams, nullptr, nullptr, 0);
            if (PQresultStatus(cartRes) == PGRES_TUPLES_OK && PQntuples(cartRes) > 0) {
                cartId = PQgetvalue(cartRes, 0, 0);
            }
            PQclear(cartRes);

            if (cartId.empty()) {
                if (!firstField) response << ",";
                response << "\"applyCoupon\":{\"success\":false,\"message\":\"No cart found. Add items to cart first with addToCart mutation.\"}";
                firstField = false;
            } else {
                const char* couponParams[1] = {couponCode.c_str()};
                PGresult* couponRes = PQexecParams(dbConn, "SELECT code, discount_type, discount_value FROM coupons WHERE code = $1 AND is_active = true", 1, nullptr, couponParams, nullptr, nullptr, 0);
                if (PQresultStatus(couponRes) != PGRES_TUPLES_OK || PQntuples(couponRes) == 0) {
                    PQclear(couponRes);
                    if (!firstField) response << ",";
                    response << "\"applyCoupon\":{\"success\":false,\"message\":\"Invalid or inactive coupon code\"}";
                    firstField = false;
                } else {
                    std::string code = PQgetvalue(couponRes, 0, 0);
                    std::string discountType = PQgetvalue(couponRes, 0, 1);
                    double discountValue = atof(PQgetvalue(couponRes, 0, 2));
                    PQclear(couponRes);

                    double subtotal = 0;
                    const char* cartParam[1] = {cartId.c_str()};
                    PGresult* itemsRes = PQexecParams(dbConn, "SELECT ci.quantity, COALESCE(ci.unit_price, b.price) as price FROM cart_items ci JOIN books b ON ci.book_id = b.id WHERE ci.cart_id = $1", 1, nullptr, cartParam, nullptr, nullptr, 0);
if (PQresultStatus(itemsRes) == PGRES_TUPLES_OK) {
                    int rows = PQntuples(itemsRes);
                    std::cerr << "[CART] cartId='" << cartId << "', rows=" << rows << ", query='" << query.substr(0, 50) << "'" << std::endl;
                    for (int i = 0; i < rows; i++) {
                            double price = atof(PQgetvalue(itemsRes, i, 1));
                            int qty = atoi(PQgetvalue(itemsRes, i, 0));
                            subtotal += price * qty;
                        }
                    }
                    PQclear(itemsRes);

                    double discountAmount = 0;
                    
                    // Check min_order_amount for fixed discounts
                    if (discountType == "fixed" && subtotal < discountValue) {
                        if (!firstField) response << ",";
                        response << "\"applyCoupon\":{";
                        response << "\"success\":false,";
                        response << "\"message\":\"Minimum order amount is $" << std::to_string((int)discountValue) << " to use this coupon\"";
                        response << "}";
                        firstField = false;
                    } else if (discountType == "percentage") {
                        discountAmount = subtotal * (discountValue / 100.0);
                    } else {
                        discountAmount = discountValue;
                    }

                    double taxAmount = subtotal * 0.08;
                    double shippingAmount = subtotal > 50 ? 0.0 : 5.99;
                    double totalAmount = subtotal + taxAmount + shippingAmount - discountAmount;
                    if (totalAmount < 0) totalAmount = 0;

                    const char* updateParams[5] = {std::to_string(subtotal).c_str(), std::to_string(discountAmount).c_str(), couponCode.c_str(), std::to_string(totalAmount).c_str(), cartId.c_str()};
                    PGresult* updateRes = PQexecParams(dbConn, "UPDATE shopping_carts SET subtotal = $1, discount = $2, coupon_code = $3, total = $4, updated_at = NOW() WHERE id = $5", 5, nullptr, updateParams, nullptr, nullptr, 0);
                    PQclear(updateRes);

                    if (!firstField) response << ",";
                    response << "\"applyCoupon\":{";
                    response << "\"success\":true,";
                    response << "\"message\":\"Coupon applied successfully\",";
                    response << "\"discountAmount\":" << discountAmount << ",";
                    response << "\"couponCode\":\"" << escapeJson(code) << "\"";
                    response << "}";
                    firstField = false;
                }
            }
        }
    } else if (query.find("applyCoupon(") != std::string::npos) {
        if (!firstField) response << ",";
        response << "\"applyCoupon\":{\"success\":false,\"message\":\"Authentication required\"}";
        firstField = false;
    }

    if ((query.find("createOrder(") != std::string::npos || query.find("createOrder ") != std::string::npos) && !currentUser.id.empty()) {
        std::cerr << "[CREATEORDER] user='" << currentUser.username << "'" << std::endl;
        std::string cartId = "";
        std::string userId = currentUser.id;
        double discount = 0;
        std::string couponCode = "";
        const char* cartParams[1] = {userId.c_str()};
        PGresult* cartRes = PQexecParams(dbConn, "SELECT id, COALESCE(discount, 0), COALESCE(coupon_code, '') FROM shopping_carts WHERE user_id = $1", 1, nullptr, cartParams, nullptr, nullptr, 0);
        if (PQresultStatus(cartRes) == PGRES_TUPLES_OK && PQntuples(cartRes) > 0) {
            cartId = PQgetvalue(cartRes, 0, 0);
            discount = atof(PQgetvalue(cartRes, 0, 1));
            couponCode = PQgetvalue(cartRes, 0, 2);
        } else {
            PGresult* insertRes = PQexecParams(dbConn, "INSERT INTO shopping_carts (user_id) VALUES ($1) RETURNING id", 1, nullptr, cartParams, nullptr, nullptr, 0);
            if (PQresultStatus(insertRes) == PGRES_TUPLES_OK && PQntuples(insertRes) > 0) {
                cartId = PQgetvalue(insertRes, 0, 0);
            }
            PQclear(insertRes);
        }
        PQclear(cartRes);

        std::string orderNumber = "ORD-" + std::to_string(time(nullptr));

        double subtotal = 0;
        int itemCount = 0;
        const char* cartParam[1] = {cartId.c_str()};
        PGresult* itemsRes = PQexecParams(dbConn, "SELECT ci.book_id, b.title, b.isbn, ci.quantity, COALESCE(ci.unit_price, b.price) as price "
                                                      "FROM cart_items ci JOIN books b ON ci.book_id = b.id WHERE ci.cart_id = $1",
                                          1, nullptr, cartParam, nullptr, nullptr, 0);
        if (PQresultStatus(itemsRes) == PGRES_TUPLES_OK) {
            itemCount = PQntuples(itemsRes);
            for (int i = 0; i < itemCount; i++) {
                double price = atof(PQgetvalue(itemsRes, i, 4));
                int qty = atoi(PQgetvalue(itemsRes, i, 3));
                subtotal += price * qty;
            }
        }
        PQclear(itemsRes);

        if (itemCount == 0) {
            if (!firstField) response << ",";
            response << "\"createOrder\":{";
            response << "\"success\":false,";
            response << "\"message\":\"Cart is empty\"";
            response << "}";
            firstField = false;
        } else {
            double tax = subtotal * 0.08;
            double shipping = subtotal > 50 ? 0 : 5.99;
            double total = subtotal + tax + shipping - discount;
            if (total < 0) total = 0;

            std::string sql = "INSERT INTO orders (user_id, order_number, status, subtotal, tax_amount, shipping_amount, discount_amount, total_amount, shipping_address, billing_address, payment_status) "
                         "VALUES ($1, $2, 'pending', $3, $4, $5, $6, $7, '123 Test St', '123 Test St', 'pending') RETURNING id";
            const char* paramValues[7] = {userId.c_str(), orderNumber.c_str(),
                                          std::to_string(subtotal).c_str(), std::to_string(tax).c_str(),
                                          std::to_string(shipping).c_str(), std::to_string(discount).c_str(),
                                          std::to_string(total).c_str()};
            PGresult* res = PQexecParams(dbConn, sql.c_str(), 7, nullptr, paramValues, nullptr, nullptr, 0);

            if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
                std::string orderId = PQgetvalue(res, 0, 0);
                std::cerr << "[CREATEORDER] created orderId='" << orderId << "', orderNumber='" << orderNumber << "', total=" << total << ", discount=" << discount << ", couponCode='" << couponCode << "'" << std::endl;
                PGresult* itemsRes2 = PQexecParams(dbConn, "SELECT ci.book_id, b.title, b.isbn, ci.quantity, b.price "
                                                              "FROM cart_items ci JOIN books b ON ci.book_id = b.id WHERE ci.cart_id = $1",
                                                  1, nullptr, cartParam, nullptr, nullptr, 0);
                if (PQresultStatus(itemsRes2) == PGRES_TUPLES_OK) {
                    int rows = PQntuples(itemsRes2);
                    for (int i = 0; i < rows; i++) {
                        double price = atof(PQgetvalue(itemsRes2, i, 4));
                        int qty = atoi(PQgetvalue(itemsRes2, i, 3));
                        std::string itemSql = "INSERT INTO order_items (order_id, book_id, book_title, book_isbn, quantity, unit_price, total_price) "
                                          "VALUES ($1, $2, $3, $4, $5, $6, $7)";
                        const char* itemParams[7] = {orderId.c_str(), PQgetvalue(itemsRes2, i, 0),
                                                       PQgetvalue(itemsRes2, i, 1), PQgetvalue(itemsRes2, i, 2),
                                                       PQgetvalue(itemsRes2, i, 3), PQgetvalue(itemsRes2, i, 4),
                                                       std::to_string(price * qty).c_str()};
                        PQexecParams(dbConn, itemSql.c_str(), 7, nullptr, itemParams, nullptr, nullptr, 0);
                    }
                }
                PQclear(itemsRes2);

                PQexecParams(dbConn, "DELETE FROM cart_items WHERE cart_id = $1", 1, nullptr, cartParam, nullptr, nullptr, 0);

                std::string orderPayload = "{\"order_id\":\"" + orderId + "\",\"order_number\":\"" + orderNumber + "\",\"total\":" + std::to_string(total) + ",\"status\":\"pending\"}";
                triggerWebhooks("order.created", orderPayload);

                if (!firstField) response << ",";
                response << "\"createOrder\":{";
                response << "\"success\":true,";
                response << "\"orderId\":\"" << orderId << "\",";
                response << "\"orderNumber\":\"" << orderNumber << "\",";
                response << "\"totalAmount\":" << total;
                response << "}";
                firstField = false;
            } else {
                if (!firstField) response << ",";
                response << "\"createOrder\":{";
                response << "\"success\":false,";
                response << "\"message\":\"Failed to create order\"";
                response << "}";
                firstField = false;
            }
            PQclear(res);
        }
    } else if (query.find("createOrder") != std::string::npos && query.find("createOrder(") == std::string::npos) {
        if (!firstField) response << ",";
        response << "\"createOrder\":{\"success\":false,\"message\":\"Authentication required\"}";
        firstField = false;
    }

    if (query.find("checkout(") != std::string::npos && !currentUser.id.empty()) {
        std::string cardNumber = extractValue(query, "cardNumber");
        std::string expiry = extractValue(query, "expiry");
        std::string cvv = extractValue(query, "cvv");

        std::cerr << "[CHECKOUT] user='" << currentUser.username << "', cardNumber='" << cardNumber << "'" << std::endl;

        if (cardNumber.empty() || expiry.empty() || cvv.empty()) {
            if (!firstField) response << ",";
            response << "\"checkout\":{";
            response << "\"success\":false,";
            response << "\"message\":\"cardNumber, expiry, and cvv are required\"";
            response << "}";
            firstField = false;
        } else {
            std::string cartId = "";
            std::string userId = currentUser.id;
            double discount = 0;
            double subtotal = 0;
            std::string couponCode = "";
            
            const char* cartParams[1] = {userId.c_str()};
            PGresult* cartRes = PQexecParams(dbConn, "SELECT id, COALESCE(discount, 0), COALESCE(coupon_code, ''), COALESCE(subtotal, 0) FROM shopping_carts WHERE user_id = $1", 1, nullptr, cartParams, nullptr, nullptr, 0);
            if (PQresultStatus(cartRes) == PGRES_TUPLES_OK && PQntuples(cartRes) > 0) {
                cartId = PQgetvalue(cartRes, 0, 0);
                discount = atof(PQgetvalue(cartRes, 0, 1));
                couponCode = PQgetvalue(cartRes, 0, 2);
                subtotal = atof(PQgetvalue(cartRes, 0, 3));
            } else {
                PGresult* insertRes = PQexecParams(dbConn, "INSERT INTO shopping_carts (user_id) VALUES ($1) RETURNING id", 1, nullptr, cartParams, nullptr, nullptr, 0);
                if (PQresultStatus(insertRes) == PGRES_TUPLES_OK && PQntuples(insertRes) > 0) {
                    cartId = PQgetvalue(insertRes, 0, 0);
                }
                PQclear(insertRes);
            }
            PQclear(cartRes);

            if (cartId.empty()) {
                if (!firstField) response << ",";
                response << "\"checkout\":{";
                response << "\"success\":false,";
                response << "\"message\":\"No active cart found\"";
                response << "}";
                firstField = false;
            } else {
                struct CheckoutItemLine {
                    std::string bookId;
                    std::string title;
                    std::string isbn;
                    int quantity;
                    double price;
                };

                std::vector<CheckoutItemLine> checkoutItems;
                double subtotal = 0;
                const char* cartParam[1] = {cartId.c_str()};
                PGresult* itemsRes = PQexecParams(dbConn,
                    "SELECT ci.book_id, b.title, b.isbn, ci.quantity, COALESCE(ci.unit_price, b.price) as price "
                    "FROM cart_items ci JOIN books b ON ci.book_id = b.id WHERE ci.cart_id = $1",
                    1, nullptr, cartParam, nullptr, nullptr, 0);
                if (PQresultStatus(itemsRes) == PGRES_TUPLES_OK) {
                    int rows = PQntuples(itemsRes);
                    for (int i = 0; i < rows; i++) {
                        CheckoutItemLine line;
                        line.bookId = PQgetvalue(itemsRes, i, 0);
                        line.title = PQgetvalue(itemsRes, i, 1) ? PQgetvalue(itemsRes, i, 1) : "";
                        line.isbn = PQgetvalue(itemsRes, i, 2) ? PQgetvalue(itemsRes, i, 2) : "";
                        line.quantity = atoi(PQgetvalue(itemsRes, i, 3));
                        line.price = atof(PQgetvalue(itemsRes, i, 4));
                        subtotal += line.price * line.quantity;
                        checkoutItems.push_back(line);
                    }
                }
                PQclear(itemsRes);

                if (checkoutItems.empty()) {
                    if (!firstField) response << ",";
                    response << "\"checkout\":{";
                    response << "\"success\":false,";
                    response << "\"message\":\"Cart is empty\"";
                    response << "}";
                    firstField = false;
                } else {
                    double cartSubtotal = subtotal;
                    if (cartSubtotal == 0) {
                        for (const auto& item : checkoutItems) {
                            cartSubtotal += item.price * item.quantity;
                        }
                    }
                    double tax = 0;
                    double shipping = 0;
                    double total = 0;
                    
                    // Fetch stored totals from cart
                    const char* cartTotalParams[1] = {cartId.c_str()};
                    PGresult* cartTotalRes = PQexecParams(dbConn, 
                        "SELECT COALESCE(subtotal, 0), COALESCE(discount, 0), COALESCE(total, 0) FROM shopping_carts WHERE id = $1",
                        1, nullptr, cartTotalParams, nullptr, nullptr, 0);
                    double storedSubtotal = 0;
                    double storedDiscount = 0;
                    double storedTotal = 0;
                    if (PQresultStatus(cartTotalRes) == PGRES_TUPLES_OK && PQntuples(cartTotalRes) > 0) {
                        storedSubtotal = atof(PQgetvalue(cartTotalRes, 0, 0));
                        storedDiscount = atof(PQgetvalue(cartTotalRes, 0, 1));
                        storedTotal = atof(PQgetvalue(cartTotalRes, 0, 2));
                        std::cerr << "[CHECKOUT] cartId='" << cartId << "', storedSubtotal=" << storedSubtotal << ", storedDiscount=" << storedDiscount << ", storedTotal=" << storedTotal << std::endl;
                    }
                    PQclear(cartTotalRes);
                    
                    // Use stored cart total if available (coupon was applied), otherwise calculate
                    if (storedTotal > 0 && storedDiscount > 0) {
                        // Cart has coupon applied - use stored values
                        total = storedTotal;  // Already includes discount, tax, shipping
                    } else {
                        // No coupon - calculate fresh
                        tax = cartSubtotal * 0.08;
                        shipping = cartSubtotal > 50 ? 0 : 5.99;
                        total = cartSubtotal + tax + shipping;
                    }
                    if (total < 0) total = 0;

                    std::string orderNumber = "ORD-" + std::to_string(time(nullptr));
                    std::string subtotalStr = std::to_string(subtotal);
                    std::string taxStr = std::to_string(tax);
                    std::string shippingStr = std::to_string(shipping);
                    std::string discountStr = std::to_string(discount);
                    std::string totalStr = std::to_string(total);
                    
                    const char* orderParams[7] = {userId.c_str(), orderNumber.c_str(),
                                                  subtotalStr.c_str(), taxStr.c_str(),
                                                  shippingStr.c_str(), discountStr.c_str(), totalStr.c_str()};
                    PGresult* orderRes = PQexecParams(dbConn,
                        "INSERT INTO orders (user_id, order_number, status, subtotal, tax_amount, shipping_amount, discount_amount, total_amount, shipping_address, billing_address, payment_status) "
                        "VALUES ($1, $2, 'pending', $3, $4, $5, $6, $7, '123 Test St', '123 Test St', 'pending') RETURNING id",
                        7, nullptr, orderParams, nullptr, nullptr, 0);

                    if (PQresultStatus(orderRes) == PGRES_TUPLES_OK && PQntuples(orderRes) > 0) {
                        std::string orderId = PQgetvalue(orderRes, 0, 0);
                        std::cerr << "[CHECKOUT] order created, orderId='" << orderId << "', total=" << total << std::endl;
                        
                        for (size_t i = 0; i < checkoutItems.size(); i++) {
                            const auto& item = checkoutItems[i];
                            std::string qtyStr = std::to_string(item.quantity);
                            std::string priceStr = std::to_string(item.price);
                            std::string totalLineStr = std::to_string(item.price * item.quantity);
                            const char* itemParams[7] = {orderId.c_str(), item.bookId.c_str(),
                                                          item.title.c_str(), item.isbn.c_str(), qtyStr.c_str(),
                                                          priceStr.c_str(), totalLineStr.c_str()};
                            PGresult* itemInsertRes = PQexecParams(dbConn,
                                "INSERT INTO order_items (order_id, book_id, book_title, book_isbn, quantity, unit_price, total_price) "
                                "VALUES ($1, $2, $3, $4, $5, $6, $7)",
                                7, nullptr, itemParams, nullptr, nullptr, 0);
                            PQclear(itemInsertRes);
                        }

                        PGresult* clearCartRes = PQexecParams(dbConn, "DELETE FROM cart_items WHERE cart_id = $1", 1, nullptr, cartParam, nullptr, nullptr, 0);
                        PQclear(clearCartRes);
                        
                        // Clear cart totals
                        PGresult* clearCartTotals = PQexecParams(dbConn, "UPDATE shopping_carts SET subtotal = 0, tax = 0, discount = 0, coupon_code = NULL, total = 0, updated_at = NOW() WHERE id = $1", 1, nullptr, cartParam, nullptr, nullptr, 0);
                        PQclear(clearCartTotals);

                        std::string paymentResult = processPayment(userId, orderId, total, cardNumber, expiry, cvv);
                        
                        // Check if payment was successful
                        bool paymentSuccess = paymentResult.find("\"success\":true") != std::string::npos;
                        
                        if (!firstField) response << ",";
                        response << "\"checkout\":{";
                        response << "\"success\":" << (paymentSuccess ? "true" : "false") << ",";
                        response << "\"orderId\":\"" << orderId << "\",";
                        response << "\"orderNumber\":\"" << orderNumber << "\",";
                        response << "\"totalAmount\":" << total << ",";
                        response << "\"warning\":\"Do not use real card details - this is a test environment\",";
                        response << "\"payment\":" << paymentResult;
                        response << "}";
                        firstField = false;
                        
                        // If payment failed, delete the order
                        if (!paymentSuccess) {
                            const char* deleteOrderParams[1] = {orderId.c_str()};
                            PGresult* deleteOrderRes = PQexecParams(dbConn, "DELETE FROM orders WHERE id = $1", 1, nullptr, deleteOrderParams, nullptr, nullptr, 0);
                            PQclear(deleteOrderRes);
                        }
                    } else {
                        if (!firstField) response << ",";
                        response << "\"checkout\":{";
                        response << "\"success\":false,";
                        response << "\"message\":\"Failed to create order\"";
                        response << "}";
                        firstField = false;
                    }
                    PQclear(orderRes);
                }
            }
        }
    } else if (query.find("checkout(") != std::string::npos) {
        if (!firstField) response << ",";
        response << "\"checkout\":{\"success\":false,\"message\":\"Authentication required\"}";
        firstField = false;
    }

    if (query.find("cancelOrder(") != std::string::npos && !currentUser.id.empty()) {
        std::string orderId = extractValue(query, "orderId");
        std::string targetOrderId = orderId;

        std::cerr << "[CANCELORDER] user='" << currentUser.username << "', orderId='" << orderId << "'" << std::endl;

        const char* params[1] = {targetOrderId.c_str()};
        PGresult* res = PQexecParams(dbConn, "SELECT id, user_id FROM orders WHERE id = $1", 1, nullptr, params, nullptr, nullptr, 0);

        if (!firstField) response << ",";
        if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
            std::string actualUserId = PQgetvalue(res, 0, 1);
            if (actualUserId == currentUser.id || currentUser.role == "admin" || currentUser.role == "staff") {
                PQexecParams(dbConn, "UPDATE orders SET status = 'cancelled', payment_status = 'refunded' WHERE id = $1", 1, nullptr, params, nullptr, nullptr, 0);
                std::cerr << "[CANCELORDER] order '" << orderId << "' cancelled successfully" << std::endl;

                std::string orderPayload = "{\"order_id\":\"" + orderId + "\",\"status\":\"cancelled\",\"refund_status\":\"processed\"}";
                triggerWebhooks("order.cancelled", orderPayload);
                response << "\"cancelOrder\":{";
                response << "\"success\":true,";
                response << "\"message\":\"Order cancelled successfully\"";
                response << "}";
            } else {
                response << "\"cancelOrder\":{";
                response << "\"success\":false,";
                response << "\"message\":\"You can only cancel your own orders\"";
                response << "}";
            }
        } else {
            response << "\"cancelOrder\":{";
            response << "\"success\":false,";
            response << "\"message\":\"Order not found\"";
            response << "}";
        }
        firstField = false;
        PQclear(res);
    } else if (query.find("cancelOrder(") != std::string::npos) {
        if (!firstField) response << ",";
        response << "\"cancelOrder\":{\"success\":false,\"message\":\"Authentication required\"}";
        firstField = false;
    }

    if (query.find("createReview(") != std::string::npos && !currentUser.id.empty()) {
        std::string bookIdStr = extractIntValue(query, "bookId");
        std::string ratingStr = extractIntValue(query, "rating");
        std::string comment = extractValue(query, "comment");
        int bookId = bookIdStr.empty() ? 0 : stoi(bookIdStr);
        int rating = ratingStr.empty() ? 5 : stoi(ratingStr);

        // Validate rating
        if (rating < 1 || rating > 5) {
            if (!firstField) response << ",";
            response << "\"createReview\":{";
            response << "\"success\":false,";
            response << "\"message\":\"Rating must be between 1 and 5\"";
            response << "}";
            firstField = false;
        } else {
        std::cerr << "[CREATEREVIEW] user='" << currentUser.username << "', bookId=" << bookId << ", rating=" << rating << std::endl;

        std::string sql = "INSERT INTO reviews (user_id, book_id, rating, comment, is_verified_purchase) "
                     "VALUES ($1, $2, $3, $4, true) ON CONFLICT (user_id, book_id) DO UPDATE SET rating = $3, comment = $4 RETURNING id";
        const char* paramValues[4] = {currentUser.id.c_str(), std::to_string(bookId).c_str(),
                                      std::to_string(rating).c_str(), comment.c_str()};
        PGresult* res = PQexecParams(dbConn, sql.c_str(), 4, nullptr, paramValues, nullptr, nullptr, 0);

        if (!firstField) response << ",";
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            std::string reviewPayload = "{\"book_id\":" + std::to_string(bookId) + ",\"rating\":" + std::to_string(rating) + ",\"user_id\":\"" + currentUser.id + "\"}";
            triggerWebhooks("review.created", reviewPayload);

            response << "\"createReview\":{";
            response << "\"success\":true,";
            response << "\"message\":\"Review created successfully\"";
            response << "}";
        } else {
            response << "\"createReview\":{";
            response << "\"success\":false,";
            response << "\"message\":\"Failed to create review\"";
            response << "}";
        }
        firstField = false;
        PQclear(res);
        }
    } else if (query.find("createReview(") != std::string::npos) {
        if (!firstField) response << ",";
        response << "\"createReview\":{\"success\":false,\"message\":\"Authentication required\"}";
        firstField = false;
    }

    if (query.find("deleteReview(") != std::string::npos && !currentUser.id.empty()) {
        std::string reviewId = extractValue(query, "reviewId");
        std::string targetReviewId = reviewId;

        std::cerr << "[DELETEREVIEW] user='" << currentUser.username << "', reviewId='" << reviewId << "'" << std::endl;

        const char* params[1] = {targetReviewId.c_str()};
        PGresult* res = PQexecParams(dbConn, "SELECT id, user_id FROM reviews WHERE id = $1", 1, nullptr, params, nullptr, nullptr, 0);

        if (!firstField) response << ",";
        if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
            std::string actualUserId = PQgetvalue(res, 0, 1);
            if (actualUserId == currentUser.id || currentUser.role == "admin" || currentUser.role == "staff") {
                PQexecParams(dbConn, "DELETE FROM reviews WHERE id = $1", 1, nullptr, params, nullptr, nullptr, 0);
                std::cerr << "[DELETEREVIEW] review '" << reviewId << "' deleted successfully" << std::endl;
                response << "\"deleteReview\":{";
                response << "\"success\":true,";
                response << "\"message\":\"Review deleted successfully\"";
                response << "}";
            } else {
                response << "\"deleteReview\":{";
                response << "\"success\":false,";
                response << "\"message\":\"You can only delete your own reviews\"";
                response << "}";
            }
        } else {
            response << "\"deleteReview\":{";
            response << "\"success\":false,";
            response << "\"message\":\"Review not found\"";
            response << "}";
        }
        firstField = false;
        PQclear(res);
    } else if (query.find("deleteReview(") != std::string::npos) {
        if (!firstField) response << ",";
        response << "\"deleteReview\":{\"success\":false,\"message\":\"Authentication required\"}";
        firstField = false;
    }

    if (query.find("registerWebhook(") != std::string::npos && !currentUser.id.empty()) {
        std::string url = extractValue(query, "url");
        std::string events = extractValue(query, "events");
        std::string secret = extractValue(query, "secret");

        std::cerr << "[REGISTERWEBHOOK] user='" << currentUser.username << "', url='" << url << "'" << std::endl;

        if (!url.empty()) {
            std::string eventsJson;
            
            // Extract events - handle GraphQL array format ["event1", "event2"]
            if (events.empty()) {
                eventsJson = "[\"*\"]";
            } else if (events.find('[') != std::string::npos) {
                // Has brackets - extract content between them
                size_t bracketStart = events.find('[');
                size_t bracketEnd = events.find(']');
                if (bracketEnd > bracketStart) {
                    std::string arrayContent = events.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
                    // Parse each quoted item
                    eventsJson = "[";
                    size_t pos = 0;
                    size_t start = 0;
                    bool firstItem = true;
                    while ((pos = arrayContent.find(',', start)) != std::string::npos) {
                        std::string item = arrayContent.substr(start, pos - start);
                        // Find quoted string and extract it
                        size_t q1 = item.find('"');
                        size_t q2 = item.find('"', q1 + 1);
                        if (q1 != std::string::npos && q2 != std::string::npos && q2 > q1) {
                            if (!firstItem) eventsJson += ",";
                            eventsJson += "\"" + item.substr(q1 + 1, q2 - q1 - 1) + "\"";
                            firstItem = false;
                        }
                        start = pos + 1;
                    }
                    // Last item
                    std::string item = arrayContent.substr(start);
                    size_t q1 = item.find('"');
                    size_t q2 = item.find('"', q1 + 1);
                    if (q1 != std::string::npos && q2 != std::string::npos && q2 > q1) {
                        if (!firstItem) eventsJson += ",";
                        eventsJson += "\"" + item.substr(q1 + 1, q2 - q1 - 1) + "\"";
                    }
                    eventsJson += "]";
                } else {
                    eventsJson = "[\"*\"]";
                }
            } else if (!events.empty() && events.find(',') != std::string::npos) {
                // Comma-separated without brackets
                eventsJson = "[";
                size_t start = 0;
                size_t pos = events.find(',');
                bool firstItem = true;
                while (pos != std::string::npos) {
                    std::string item = events.substr(start, pos - start);
                    // Trim whitespace
                    size_t first = item.find_first_not_of(" \t\"");
                    size_t last = item.find_last_not_of(" \t\"");
                    if (first != std::string::npos) {
                        if (!firstItem) eventsJson += ",";
                        eventsJson += "\"" + item.substr(first, last - first + 1) + "\"";
                        firstItem = false;
                    }
                    start = pos + 1;
                    pos = events.find(',', start);
                }
                // Last item
                std::string item = events.substr(start);
                size_t first = item.find_first_not_of(" \t\"");
                size_t last = item.find_last_not_of(" \t\"");
                if (first != std::string::npos) {
                    if (!firstItem) eventsJson += ",";
                    eventsJson += "\"" + item.substr(first, last - first + 1) + "\"";
                }
                eventsJson += "]";
            } else {
                // Single event - just wrap in quotes
                eventsJson = "[\"" + events + "\"]";
            }

            std::string sql = "INSERT INTO webhooks (user_id, url, events, secret) VALUES ($1, $2, $3::jsonb, $4) RETURNING id";
            const char* paramValues[4] = {currentUser.id.c_str(), url.c_str(),
                                         eventsJson.c_str(),
                                         secret.empty() ? "default_secret" : secret.c_str()};
            PGresult* res = PQexecParams(dbConn, sql.c_str(), 4, nullptr, paramValues, nullptr, nullptr, 0);

            if (!firstField) response << ",";
            if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
                std::string webhookId = PQgetvalue(res, 0, 0);
                std::cerr << "[REGISTERWEBHOOK] webhook created, id='" << webhookId << "'" << std::endl;
                response << "\"registerWebhook\":{";
                response << "\"success\":true,";
                response << "\"message\":\"Webhook registered successfully\",";
                response << "\"webhook\":{";
                response << "\"id\":\"" << webhookId << "\",";
                response << "\"url\":\"" << escapeJson(url) << "\",";
                response << "\"events\":" << eventsJson << ",";
                response << "\"isActive\":true";
                response << "}";
                response << "}";
            } else {
                response << "\"registerWebhook\":{";
                response << "\"success\":false,";
                response << "\"message\":\"Failed to register webhook\"";
                response << "}";
            }
            firstField = false;
            PQclear(res);
        } else {
            if (!firstField) response << ",";
            response << "\"registerWebhook\":{";
            response << "\"success\":false,";
            response << "\"message\":\"URL is required\"";
            response << "}";
            firstField = false;
        }
    } else if (query.find("registerWebhook(") != std::string::npos) {
        if (!firstField) response << ",";
        response << "\"registerWebhook\":{\"success\":false,\"message\":\"Authentication required\"}";
        firstField = false;
    }

    if (query.find("testWebhook(") != std::string::npos && !currentUser.id.empty()) {
        std::string webhookId = extractValue(query, "webhookId");
        std::string targetWebhookId = webhookId;

        std::cerr << "[TESTWEBHOOK] user='" << currentUser.username << "', webhookId='" << webhookId << "'" << std::endl;

        const char* params[1] = {targetWebhookId.c_str()};
        PGresult* res = PQexecParams(dbConn, "SELECT id, user_id, url, secret FROM webhooks WHERE id = $1 AND is_active = true", 1, nullptr, params, nullptr, nullptr, 0);

        if (!firstField) response << ",";
        if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
            std::string actualUserId = PQgetvalue(res, 0, 1);
            if (actualUserId == currentUser.id || currentUser.role == "admin" || currentUser.role == "staff") {
                std::string url = PQgetvalue(res, 0, 2);
                std::string secret = PQgetvalue(res, 0, 3);

                std::string testPayload = "{\"event\":\"test\",\"timestamp\":\"" + std::to_string(time(nullptr)) + "\",\"secret\":\"" + secret + "\"}";
                std::string responseBody;
                bool success = fetchURL(url, responseBody);

                if (success) {
                    std::cerr << "[TESTWEBHOOK] webhook triggered successfully" << std::endl;
                    response << "\"testWebhook\":{";
                    response << "\"success\":true,";
                    response << "\"message\":\"Webhook triggered successfully\",";
                    response << "\"response\":\"" << escapeJson(responseBody) << "\"";
                    response << "}";
                } else {
                    response << "\"testWebhook\":{";
                    response << "\"success\":false,";
                    response << "\"message\":\"Failed to trigger webhook\"";
                    response << "}";
                }
            } else {
                response << "\"testWebhook\":{";
                response << "\"success\":false,";
                response << "\"message\":\"You can only test your own webhooks\"";
                response << "}";
            }
        } else {
            response << "\"testWebhook\":{";
            response << "\"success\":false,";
            response << "\"message\":\"Webhook not found or inactive\"";
            response << "}";
        }
        firstField = false;
        PQclear(res);
    }

    if ((query.find("logout(") != std::string::npos || query.find("logout {") != std::string::npos) && !currentUser.id.empty()) {
        if (!firstField) response << ",";
        response << "\"logout\":{";
        response << "\"success\":true,";
        response << "\"message\":\"Logged out successfully\"";
        response << "}";
        firstField = false;
    } else if (query.find("logout(") != std::string::npos || query.find("logout {") != std::string::npos) {
        if (!firstField) response << ",";
        response << "\"logout\":{\"success\":false,\"message\":\"Authentication required\"}";
        firstField = false;
    }

    response << "}}";
    return response.str();
}

User extractAuthUser(const std::string& authHeader) {
    AuthResult result = extractAuthUserWithError(authHeader);
    return result.user;
}

AuthResult extractAuthUserWithError(const std::string& authHeader) {
    if (authHeader.empty()) {
        AuthResult result;
        result.valid = false;
        result.error = "No token provided";
        return result;
    }
    
    std::string token = authHeader;
    if (token.find("Bearer ") == 0) {
        token = token.substr(7);
    }
    
    return verifyJWTWithError(token);
}

std::string handleRequest(const std::string& query, User& currentUser, bool isMutation) {
    if (query.empty()) {
        return "{\"errors\":[{\"message\":\"Empty query\"}]}";
    }
    
    if (isMutation) {
        return handleMutation(query, currentUser);
    } else {
        return handleQuery(query, currentUser);
    }
}