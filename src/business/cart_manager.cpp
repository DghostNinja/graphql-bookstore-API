#include "business/cart_manager.h"
#include "database/connection.h"
#include <iostream>

CartManager& CartManager::getInstance() {
    static CartManager instance;
    return instance;
}

std::string CartManager::getCart(const std::string& userId) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string cartQuery = "SELECT id, user_id FROM shopping_carts WHERE user_id = $1";
    auto cartResult = conn->executeQuery(cartQuery, {userId});
    
    std::string cartId;
    if (PQntuples(cartResult) == 0) {
        conn->clearResult(cartResult);
        
        std::string createCart = "INSERT INTO shopping_carts (user_id) VALUES ($1) RETURNING id";
        auto newCartResult = conn->executeQuery(createCart, {userId});
        cartId = PQgetvalue(newCartResult, 0, 0);
        conn->clearResult(newCartResult);
    } else {
        cartId = PQgetvalue(cartResult, 0, 0);
        conn->clearResult(cartResult);
    }
    
    std::string itemsQuery = "SELECT ci.id, ci.book_id, ci.quantity, ci.added_at, "
                            "b.title, b.price, b.sale_price "
                            "FROM cart_items ci JOIN books b ON ci.book_id = b.id "
                            "WHERE ci.cart_id = $1";
    
    auto itemsResult = conn->executeQuery(itemsQuery, {cartId});
    
    json cart;
    cart["id"] = cartId;
    cart["user"]["id"] = userId;
    cart["items"] = json::array();
    
    double subtotal = 0.0;
    int numItems = PQntuples(itemsResult);
    
    for (int i = 0; i < numItems; i++) {
        json item;
        item["id"] = PQgetvalue(itemsResult, i, 0);
        item["book"]["id"] = PQgetvalue(itemsResult, i, 1);
        item["book"]["title"] = PQgetvalue(itemsResult, i, 4);
        item["quantity"] = std::stoi(PQgetvalue(itemsResult, i, 2));
        item["addedAt"] = PQgetvalue(itemsResult, i, 3);
        
        double price = std::stod(PQgetvalue(itemsResult, i, 5));
        std::string salePrice = PQgetvalue(itemsResult, i, 6);
        if (!salePrice.empty() && salePrice != "") {
            price = std::stod(salePrice);
        }
        
        item["price"] = price;
        item["total"] = price * item["quantity"];
        subtotal += item["total"];
        
        cart["items"].push_back(item);
    }
    
    conn->clearResult(itemsResult);
    
    double tax = subtotal * 0.0825;
    double discount = 0.0;
    
    cart["subtotal"] = subtotal;
    cart["tax"] = tax;
    cart["discount"] = discount;
    cart["total"] = subtotal + tax - discount;
    cart["createdAt"] = "";
    cart["updatedAt"] = "";
    
    return cart.dump();
}

std::string CartManager::addToCart(const std::string& userId, const std::string& bookId, int quantity) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string cartQuery = "SELECT id FROM shopping_carts WHERE user_id = $1";
    auto cartResult = conn->executeQuery(cartQuery, {userId});
    
    std::string cartId;
    if (PQntuples(cartResult) == 0) {
        conn->clearResult(cartResult);
        
        std::string createCart = "INSERT INTO shopping_carts (user_id) VALUES ($1) RETURNING id";
        auto newCartResult = conn->executeQuery(createCart, {userId});
        cartId = PQgetvalue(newCartResult, 0, 0);
        conn->clearResult(newCartResult);
    } else {
        cartId = PQgetvalue(cartResult, 0, 0);
        conn->clearResult(cartResult);
    }
    
    std::string bookQuery = "SELECT price, sale_price, stock_quantity FROM books WHERE id = $1";
    auto bookResult = conn->executeQuery(bookQuery, {bookId});
    
    if (PQntuples(bookResult) == 0) {
        conn->clearResult(bookResult);
        json error;
        error["success"] = false;
        error["message"] = "Book not found";
        return error.dump();
    }
    
    double price = std::stod(PQgetvalue(bookResult, 0, 0));
    std::string salePrice = PQgetvalue(bookResult, 0, 1);
    int stockQuantity = std::stoi(PQgetvalue(bookResult, 0, 2));
    conn->clearResult(bookResult);
    
    if (!salePrice.empty() && salePrice != "") {
        price = std::stod(salePrice);
    }
    
    if (quantity > stockQuantity) {
        json error;
        error["success"] = false;
        error["message"] = "Insufficient stock";
        return error.dump();
    }
    
    std::string checkItem = "SELECT id, quantity FROM cart_items WHERE cart_id = $1 AND book_id = $2";
    auto checkResult = conn->executeQuery(checkItem, {cartId, bookId});
    
    if (PQntuples(checkResult) > 0) {
        std::string itemId = PQgetvalue(checkResult, 0, 0);
        int currentQuantity = std::stoi(PQgetvalue(checkResult, 0, 1));
        conn->clearResult(checkResult);
        
        std::string updateQuery = "UPDATE cart_items SET quantity = $1 WHERE id = $2";
        conn->executeQuery(updateQuery, {std::to_string(currentQuantity + quantity), itemId});
    } else {
        conn->clearResult(checkResult);
        
        std::string insertQuery = "INSERT INTO cart_items (cart_id, book_id, quantity) VALUES ($1, $2, $3)";
        conn->executeQuery(insertQuery, {cartId, bookId, std::to_string(quantity)});
    }
    
    std::string updateCart = "UPDATE shopping_carts SET updated_at = NOW() WHERE id = $1";
    conn->executeQuery(updateCart, {cartId});
    
    return getCart(userId);
}

std::string CartManager::removeFromCart(const std::string& userId, const std::string& bookId) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string cartQuery = "SELECT id FROM shopping_carts WHERE user_id = $1";
    auto cartResult = conn->executeQuery(cartQuery, {userId});
    
    if (PQntuples(cartResult) == 0) {
        conn->clearResult(cartResult);
        return getCart(userId);
    }
    
    std::string cartId = PQgetvalue(cartResult, 0, 0);
    conn->clearResult(cartResult);
    
    std::string deleteQuery = "DELETE FROM cart_items WHERE cart_id = $1 AND book_id = $2";
    conn->executeQuery(deleteQuery, {cartId, bookId});
    
    std::string updateCart = "UPDATE shopping_carts SET updated_at = NOW() WHERE id = $1";
    conn->executeQuery(updateCart, {cartId});
    
    return getCart(userId);
}

std::string CartManager::updateCartItem(const std::string& userId, const std::string& bookId, int quantity) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string cartQuery = "SELECT id FROM shopping_carts WHERE user_id = $1";
    auto cartResult = conn->executeQuery(cartQuery, {userId});
    
    if (PQntuples(cartResult) == 0) {
        conn->clearResult(cartResult);
        json error;
        error["success"] = false;
        error["message"] = "No cart found. Add items to cart first with addToCart mutation.";
        return error.dump();
    }
    
    std::string cartId = PQgetvalue(cartResult, 0, 0);
    conn->clearResult(cartResult);
    
    std::string bookQuery = "SELECT stock_quantity FROM books WHERE id = $1";
    auto bookResult = conn->executeQuery(bookQuery, {bookId});
    
    if (PQntuples(bookResult) == 0) {
        conn->clearResult(bookResult);
        json error;
        error["success"] = false;
        error["message"] = "Book not found";
        return error.dump();
    }
    
    int stockQuantity = std::stoi(PQgetvalue(bookResult, 0, 0));
    conn->clearResult(bookResult);
    
    if (quantity > stockQuantity) {
        json error;
        error["success"] = false;
        error["message"] = "Insufficient stock";
        return error.dump();
    }
    
    std::string updateQuery = "UPDATE cart_items SET quantity = $1 WHERE cart_id = $2 AND book_id = $3";
    conn->executeQuery(updateQuery, {std::to_string(quantity), cartId, bookId});
    
    std::string updateCart = "UPDATE shopping_carts SET updated_at = NOW() WHERE id = $1";
    conn->executeQuery(updateCart, {cartId});
    
    return getCart(userId);
}

std::string CartManager::clearCart(const std::string& userId) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string cartQuery = "SELECT id FROM shopping_carts WHERE user_id = $1";
    auto cartResult = conn->executeQuery(cartQuery, {userId});
    
    if (PQntuples(cartResult) == 0) {
        conn->clearResult(cartResult);
        return getCart(userId);
    }
    
    std::string cartId = PQgetvalue(cartResult, 0, 0);
    conn->clearResult(cartResult);
    
    std::string deleteQuery = "DELETE FROM cart_items WHERE cart_id = $1";
    conn->executeQuery(deleteQuery, {cartId});
    
    std::string updateCart = "UPDATE shopping_carts SET subtotal = 0, tax = 0, discount = 0, total = 0, updated_at = NOW() WHERE id = $1";
    conn->executeQuery(updateCart, {cartId});
    
    return getCart(userId);
}

std::string CartManager::applyCoupon(const std::string& userId, const std::string& couponCode) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string cartQuery = "SELECT id, subtotal FROM shopping_carts WHERE user_id = $1";
    auto cartResult = conn->executeQuery(cartQuery, {userId});
    
    if (PQntuples(cartResult) == 0) {
        conn->clearResult(cartResult);
        json error;
        error["success"] = false;
        error["message"] = "No cart found. Add items to cart first with addToCart mutation.";
        return error.dump();
    }
    
    std::string cartId = PQgetvalue(cartResult, 0, 0);
    double subtotal = std::stod(PQgetvalue(cartResult, 0, 1));
    conn->clearResult(cartResult);
    
    std::string couponResult = validateCoupon(couponCode, subtotal);
    json couponData = json.Parse(couponResult);
    
    if (!couponData["success"]) {
        return couponResult;
    }
    
    double discountAmount = couponData["discountAmount"];
    
    std::string updateQuery = "UPDATE shopping_carts SET discount = $1, coupon_code = $2, updated_at = NOW() WHERE id = $3";
    conn->executeQuery(updateQuery, {std::to_string(discountAmount), couponCode, cartId});
    
    return getCart(userId);
}

std::string CartManager::removeCoupon(const std::string& userId) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string cartQuery = "SELECT id FROM shopping_carts WHERE user_id = $1";
    auto cartResult = conn->executeQuery(cartQuery, {userId});
    
    if (PQntuples(cartResult) == 0) {
        conn->clearResult(cartResult);
        return getCart(userId);
    }
    
    std::string cartId = PQgetvalue(cartResult, 0, 0);
    conn->clearResult(cartResult);
    
    std::string updateQuery = "UPDATE shopping_carts SET discount = 0, coupon_code = NULL, updated_at = NOW() WHERE id = $1";
    conn->executeQuery(updateQuery, {cartId});
    
    return getCart(userId);
}

std::string CartManager::calculateCartTotals(const std::string& cartId) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string itemsQuery = "SELECT ci.quantity, b.price, b.sale_price "
                            "FROM cart_items ci JOIN books b ON ci.book_id = b.id "
                            "WHERE ci.cart_id = $1";
    
    auto itemsResult = conn->executeQuery(itemsQuery, {cartId});
    
    double subtotal = 0.0;
    int numItems = PQntuples(itemsResult);
    
    for (int i = 0; i < numItems; i++) {
        int quantity = std::stoi(PQgetvalue(itemsResult, i, 0));
        double price = std::stod(PQgetvalue(itemsResult, i, 1));
        std::string salePrice = PQgetvalue(itemsResult, i, 2);
        
        if (!salePrice.empty() && salePrice != "") {
            price = std::stod(salePrice);
        }
        
        subtotal += price * quantity;
    }
    
    conn->clearResult(itemsResult);
    
    json totals;
    totals["subtotal"] = subtotal;
    totals["tax"] = subtotal * 0.0825;
    totals["total"] = subtotal * 1.0825;
    
    return totals.dump();
}

std::string CartManager::validateCoupon(const std::string& couponCode, double subtotal) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT code, discount_type, discount_value, min_order_amount, "
                       "max_discount_amount, usage_limit, usage_count, start_date, end_date, is_active "
                       "FROM coupons WHERE code = $1";
    
    auto result = conn->executeQuery(query, {couponCode});
    
    if (PQntuples(result) == 0) {
        conn->clearResult(result);
        json response;
        response["success"] = false;
        response["message"] = "Invalid coupon code";
        return response.dump();
    }
    
    std::string discountType = PQgetvalue(result, 0, 1);
    double discountValue = std::stod(PQgetvalue(result, 0, 2));
    double minOrderAmount = std::stod(PQgetvalue(result, 0, 3));
    std::string maxDiscountStr = PQgetvalue(result, 0, 4);
    int usageLimit = std::stoi(PQgetvalue(result, 0, 5));
    int usageCount = std::stoi(PQgetvalue(result, 0, 6));
    std::string startDate = PQgetvalue(result, 0, 7);
    std::string endDate = PQgetvalue(result, 0, 8);
    bool isActive = std::string(PQgetvalue(result, 0, 9)) == "t";
    
    conn->clearResult(result);
    
    json response;
    
    if (!isActive) {
        response["success"] = false;
        response["message"] = "Coupon is not active";
        return response.dump();
    }
    
    if (usageLimit > 0 && usageCount >= usageLimit) {
        response["success"] = false;
        response["message"] = "Coupon usage limit reached";
        return response.dump();
    }
    
    if (subtotal < minOrderAmount) {
        response["success"] = false;
        response["message"] = "Minimum order amount not met";
        return response.dump();
    }
    
    double discountAmount = 0.0;
    
    if (discountType == "percentage") {
        discountAmount = subtotal * (discountValue / 100.0);
    } else {
        discountAmount = discountValue;
    }
    
    if (!maxDiscountStr.empty() && maxDiscountStr != "") {
        double maxDiscount = std::stod(maxDiscountStr);
        if (discountAmount > maxDiscount) {
            discountAmount = maxDiscount;
        }
    }
    
    response["success"] = true;
    response["discountAmount"] = discountAmount;
    
    return response.dump();
}