#include "resolvers/order_resolvers.h"
#include "database/connection.h"
#include "auth/authorization.h"
#include "business/payment_processor.h"
#include "business/inventory_manager.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

std::map<std::string, QueryResolver> queryOrderResolvers;
std::map<std::string, MutationResolver> mutationOrderResolvers;

void OrderResolvers::registerResolvers() {
    queryOrderResolvers["order"] = QueryResolver("order", resolveOrder);
    queryOrderResolvers["order"].setRequireAuth(true);
    queryOrderResolvers["order"].setRequireOwnership(true);
    
    queryOrderResolvers["orders"] = QueryResolver("orders", resolveOrders);
    queryOrderResolvers["orders"].setRequireAuth(true);
    queryOrderResolvers["orders"].setRequiredRole(UserRole::STAFF);
    
    queryOrderResolvers["myOrders"] = QueryResolver("myOrders", resolveMyOrders);
    queryOrderResolvers["myOrders"].setRequireAuth(true);
    
    queryOrderResolvers["_internalOrdersByDate"] = QueryResolver("_internalOrdersByDate", resolveInternalOrdersByDate);
    queryOrderResolvers["_internalOrdersByDate"].setRequireAuth(true);
    
    mutationOrderResolvers["createOrder"] = MutationResolver("createOrder", resolveCreateOrder);
    mutationOrderResolvers["createOrder"].setRequireAuth(true);
    
    mutationOrderResolvers["cancelOrder"] = MutationResolver("cancelOrder", resolveCancelOrder);
    mutationOrderResolvers["cancelOrder"].setRequireAuth(true);
    
    mutationOrderResolvers["requestRefund"] = MutationResolver("requestRefund", resolveRequestRefund);
    mutationOrderResolvers["requestRefund"].setRequireAuth(true);
    
    mutationOrderResolvers["updateOrderStatus"] = MutationResolver("updateOrderStatus", resolveUpdateOrderStatus);
    mutationOrderResolvers["updateOrderStatus"].setRequireAuth(true);
    
    mutationOrderResolvers["updateOrderAddress"] = MutationResolver("updateOrderAddress", resolveUpdateOrderAddress);
    mutationOrderResolvers["updateOrderAddress"].setRequireAuth(true);
}

ResolverResult OrderResolvers::resolveOrder(const ResolverParams& params) {
    std::string orderId = params.arguments.at("id");
    
    std::string data = getOrderById(orderId, params.authContext);
    
    if (data == "null") {
        return ResolverResult::errorResult("Order not found");
    }
    
    return ResolverResult::successResult(data);
}

ResolverResult OrderResolvers::resolveOrders(const ResolverParams& params) {
    int limit = 20;
    int offset = 0;
    std::string statusFilter;
    std::string userIdFilter;
    
    if (params.arguments.count("limit")) {
        limit = std::stoi(params.arguments.at("limit"));
    }
    if (params.arguments.count("offset")) {
        offset = std::stoi(params.arguments.at("offset"));
    }
    if (params.arguments.count("status")) {
        statusFilter = params.arguments.at("status");
    }
    if (params.arguments.count("userId")) {
        userIdFilter = params.arguments.at("userId");
    }
    
    std::string data = getAllOrders(limit, offset, statusFilter, userIdFilter, params.authContext);
    
    return ResolverResult::successResult(data);
}

ResolverResult OrderResolvers::resolveMyOrders(const ResolverParams& params) {
    int limit = 20;
    int offset = 0;
    
    if (params.arguments.count("limit")) {
        limit = std::stoi(params.arguments.at("limit"));
    }
    if (params.arguments.count("offset")) {
        offset = std::stoi(params.arguments.at("offset"));
    }
    
    std::string data = getAllOrders(limit, offset, "", params.authContext.userId, params.authContext);
    
    return ResolverResult::successResult(data);
}

ResolverResult OrderResolvers::resolveCreateOrder(const ResolverParams& params) {
    json input;
    try {
        input = json.Parse(params.arguments.at("input"));
    } catch (...) {
        return ResolverResult::errorResult("Invalid input");
    }
    
    auto conn = DatabasePool::getInstance().getConnection();
    
    conn->beginTransaction();
    
    try {
        std::string cartQuery = "SELECT id, subtotal, tax, discount, total "
                                "FROM shopping_carts WHERE user_id = $1";
        auto cartResult = conn->executeQuery(cartQuery, {params.authContext.userId});
        
        if (PQntuples(cartResult) == 0) {
            conn->rollbackTransaction();
            conn->clearResult(cartResult);
            return ResolverResult::errorResult("No cart found. Add items to cart first with addToCart mutation.");
        }
        
        std::string cartId = PQgetvalue(cartResult, 0, 0);
        double subtotal = std::stod(PQgetvalue(cartResult, 0, 1));
        double tax = std::stod(PQgetvalue(cartResult, 0, 2));
        double discount = std::stod(PQgetvalue(cartResult, 0, 3));
        double total = std::stod(PQgetvalue(cartResult, 0, 4));
        
        conn->clearResult(cartResult);
        
        std::string orderNumber = "ORD-" + std::to_string(std::time(nullptr));
        
        std::string insertOrder = "INSERT INTO orders (user_id, order_number, status, subtotal, "
                                  "tax, shipping_amount, discount_amount, total_amount, "
                                  "shipping_address, billing_address, payment_method, notes) "
                                  "VALUES ($1, $2, 'pending', $3, $4, $5, $6, $7, $8, $9, $10, $11) "
                                  "RETURNING id";
        
        std::string shippingAddr = input["shippingAddress"]["street"] + ", " + 
                                  input["shippingAddress"]["city"] + ", " +
                                  input["shippingAddress"]["state"] + " " +
                                  input["shippingAddress"]["zipCode"];
        
        std::string billingAddr = input["billingAddress"]["street"] + ", " + 
                                 input["billingAddress"]["city"] + ", " +
                                 input["billingAddress"]["state"] + " " +
                                 input["billingAddress"]["zipCode"];
        
        auto orderResult = conn->executeQuery(insertOrder, {
            params.authContext.userId,
            orderNumber,
            std::to_string(subtotal),
            std::to_string(tax),
            "5.99",
            std::to_string(discount),
            std::to_string(total + 5.99),
            shippingAddr,
            billingAddr,
            input["paymentMethod"],
            input.count("notes") ? input["notes"].get<std::string>() : ""
        });
        
        std::string orderId = PQgetvalue(orderResult, 0, 0);
        conn->clearResult(orderResult);
        
        std::string cartItemsQuery = "SELECT book_id, quantity, price FROM cart_items WHERE cart_id = $1";
        auto cartItemsResult = conn->executeQuery(cartItemsQuery, {cartId});
        
        int numItems = PQntuples(cartItemsResult);
        for (int i = 0; i < numItems; i++) {
            std::string bookId = PQgetvalue(cartItemsResult, i, 0);
            std::string quantity = PQgetvalue(cartItemsResult, i, 1);
            std::string price = PQgetvalue(cartItemsResult, i, 2);
            
            std::string bookInfoQuery = "SELECT title, isbn FROM books WHERE id = $1";
            auto bookInfoResult = conn->executeQuery(bookInfoQuery, {bookId});
            
            std::string bookTitle = PQgetvalue(bookInfoResult, 0, 0);
            std::string bookIsbn = PQgetvalue(bookInfoResult, 0, 1);
            conn->clearResult(bookInfoResult);
            
            std::string insertItem = "INSERT INTO order_items (order_id, book_id, book_title, book_isbn, "
                                     "quantity, unit_price, total_price) "
                                     "VALUES ($1, $2, $3, $4, $5, $6, $7)";
            
            double unitPrice = std::stod(price);
            double totalPrice = unitPrice * std::stoi(quantity);
            
            conn->executeQuery(insertItem, {
                orderId, bookId, bookTitle, bookIsbn, quantity, price, std::to_string(totalPrice)
            });
            
            InventoryManager::updateStock(bookId, -std::stoi(quantity), "Order fulfillment");
        }
        
        conn->clearResult(cartItemsResult);
        
        std::string clearCart = "DELETE FROM cart_items WHERE cart_id = $1";
        conn->executeQuery(clearCart, {cartId});
        
        std::string updateCart = "UPDATE shopping_carts SET subtotal = 0, tax = 0, discount = 0, total = 0 WHERE id = $1";
        conn->executeQuery(updateCart, {cartId});
        
        conn->commitTransaction();
        
        return resolveOrder(ResolverParams{params.arguments, params.authContext, "", ""});
        
    } catch (const std::exception& e) {
        conn->rollbackTransaction();
        return ResolverResult::errorResult(std::string("Failed to create order: ") + e.what());
    }
}

ResolverResult OrderResolvers::resolveCancelOrder(const ResolverParams& params) {
    std::string orderId = params.arguments.at("orderId");
    std::string reason = params.arguments.count("reason") ? params.arguments.at("reason") : "";
    
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT user_id, status FROM orders WHERE id = $1";
    auto result = conn->executeQuery(query, {orderId});
    
    if (PQntuples(result) == 0) {
        conn->clearResult(result);
        return ResolverResult::errorResult("Order not found");
    }
    
    std::string orderUserId = PQgetvalue(result, 0, 0);
    std::string status = PQgetvalue(result, 0, 1);
    conn->clearResult(result);
    
    if (params.authContext.userId != orderUserId && 
        params.authContext.role < UserRole::STAFF) {
        return ResolverResult::errorResult("Not authorized");
    }
    
    std::string updateQuery = "UPDATE orders SET status = 'cancelled', notes = COALESCE(notes, '') || ' - Cancelled: " + 
                              reason + "' WHERE id = $1";
    conn->executeQuery(updateQuery, {orderId});
    
    return resolveOrder(ResolverParams{params.arguments, params.authContext, "", ""});
}

ResolverResult OrderResolvers::resolveRequestRefund(const ResolverParams& params) {
    std::string orderId = params.arguments.at("orderId");
    std::string reason = params.arguments.count("reason") ? params.arguments.at("reason") : "Customer request";
    
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string orderQuery = "SELECT user_id, status, total_amount, created_at FROM orders WHERE id = $1";
    auto orderResult = conn->executeQuery(orderQuery, {orderId});
    
    if (PQntuples(orderResult) == 0) {
        conn->clearResult(orderResult);
        return ResolverResult::errorResult("Order not found");
    }
    
    std::string orderUserId = PQgetvalue(orderResult, 0, 0);
    std::string status = PQgetvalue(orderResult, 0, 1);
    double totalAmount = std::stod(PQgetvalue(orderResult, 0, 2));
    std::string createdAt = PQgetvalue(orderResult, 0, 3);
    conn->clearResult(orderResult);
    
    if (params.authContext.userId != orderUserId && 
        params.authContext.role < UserRole::STAFF) {
        return ResolverResult::errorResult("Not authorized");
    }
    
    if (status == "cancelled" || status == "refunded") {
        return ResolverResult::errorResult("Order already cancelled or refunded");
    }
    
    std::string refundQuery = "INSERT INTO payment_transactions (order_id, user_id, amount, currency, "
                              "payment_method, status, gateway_response) "
                              "VALUES ($1, $2, $3, 'USD', 'refund', 'pending', 'Refund requested: " + reason + "') "
                              "RETURNING id";
    
    auto refundResult = conn->executeQuery(refundQuery, {
        orderId,
        orderUserId,
        std::to_string(totalAmount)
    });
    
    std::string refundId = PQgetvalue(refundResult, 0, 0);
    conn->clearResult(refundResult);
    
    std::string updateOrder = "UPDATE orders SET status = 'refunded', payment_status = 'refunded' WHERE id = $1";
    conn->executeQuery(updateOrder, {orderId});
    
    json response;
    response["id"] = refundId;
    response["order"] = json.Parse(getOrderById(orderId, params.authContext));
    response["status"] = "approved";
    response["amount"] = totalAmount;
    response["reason"] = reason;
    response["createdAt"] = std::to_string(std::time(nullptr));
    
    return ResolverResult::successResult(response.dump());
}

ResolverResult OrderResolvers::resolveUpdateOrderStatus(const ResolverParams& params) {
    std::string orderId = params.arguments.at("orderId");
    std::string newStatus = params.arguments.at("status");
    
    auto conn = DatabasePool::getInstance().getConnection();
    
    if (params.authContext.role < UserRole::STAFF) {
        return ResolverResult::errorResult("Not authorized");
    }
    
    std::string query = "UPDATE orders SET status = $1";
    
    if (newStatus == "shipped") {
        query += ", shipped_at = NOW()";
    } else if (newStatus == "delivered") {
        query += ", delivered_at = NOW()";
    }
    
    query += " WHERE id = $" + std::to_string(2);
    
    conn->executeQuery(query, {newStatus, orderId});
    
    return resolveOrder(ResolverParams{params.arguments, params.authContext, "", ""});
}

ResolverResult OrderResolvers::resolveUpdateOrderAddress(const ResolverParams& params) {
    std::string orderId = params.arguments.at("orderId");
    json address;
    try {
        address = json.Parse(params.arguments.at("address"));
    } catch (...) {
        return ResolverResult::errorResult("Invalid address");
    }
    
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string addressStr = address["street"] + ", " + address["city"] + ", " + 
                            address["state"] + " " + address["zipCode"];
    
    std::string query = "UPDATE orders SET shipping_address = $1 WHERE id = $2";
    conn->executeQuery(query, {addressStr, orderId});
    
    return resolveOrder(ResolverParams{params.arguments, params.authContext, "", ""});
}

ResolverResult OrderResolvers::resolveInternalOrdersByDate(const ResolverParams& params) {
    std::string startDate = params.arguments.at("startDate");
    std::string endDate = params.arguments.at("endDate");
    
    std::string data = getOrdersByDate(startDate, endDate, params.authContext);
    
    return ResolverResult::successResult(data);
}

std::string OrderResolvers::getOrderById(const std::string& orderId, const RequestContext& ctx) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT o.id, o.order_number, o.user_id, o.status, o.subtotal, o.tax, "
                        "o.shipping_amount, o.discount_amount, o.total_amount, "
                        "o.shipping_address, o.billing_address, o.payment_method, o.payment_status, "
                        "o.tracking_number, o.notes, o.created_at, o.updated_at, o.shipped_at, o.delivered_at, "
                        "u.email, u.first_name, u.last_name "
                        "FROM orders o JOIN users u ON o.user_id = u.id WHERE o.id = $1";
    
    auto result = conn->executeQuery(query, {orderId});
    
    if (PQntuples(result) == 0) {
        conn->clearResult(result);
        return "null";
    }
    
    json order;
    order["id"] = PQgetvalue(result, 0, 0);
    order["orderNumber"] = PQgetvalue(result, 0, 1);
    order["user"]["id"] = PQgetvalue(result, 0, 2);
    order["user"]["email"] = PQgetvalue(result, 0, 19);
    order["user"]["firstName"] = PQgetvalue(result, 0, 20);
    order["user"]["lastName"] = PQgetvalue(result, 0, 21);
    order["status"] = PQgetvalue(result, 0, 3);
    order["subtotal"] = std::stod(PQgetvalue(result, 0, 4));
    order["tax"] = std::stod(PQgetvalue(result, 0, 5));
    order["shipping"] = std::stod(PQgetvalue(result, 0, 6));
    order["discount"] = std::stod(PQgetvalue(result, 0, 7));
    order["total"] = std::stod(PQgetvalue(result, 0, 8));
    order["shippingAddress"] = PQgetvalue(result, 0, 9);
    order["billingAddress"] = PQgetvalue(result, 0, 10);
    order["paymentMethod"] = PQgetvalue(result, 0, 11);
    order["paymentStatus"] = PQgetvalue(result, 0, 12);
    order["trackingNumber"] = PQgetvalue(result, 0, 13);
    order["notes"] = PQgetvalue(result, 0, 14);
    order["createdAt"] = PQgetvalue(result, 0, 15);
    order["updatedAt"] = PQgetvalue(result, 0, 16);
    order["shippedAt"] = PQgetvalue(result, 0, 17);
    order["deliveredAt"] = PQgetvalue(result, 0, 18);
    order["items"] = json::array();
    
    conn->clearResult(result);
    
    return order.dump();
}

std::string OrderResolvers::getAllOrders(int limit, int offset, const std::string& statusFilter, 
                                          const std::string& userIdFilter, const RequestContext& ctx) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT o.id, o.order_number, o.user_id, o.status, o.total_amount, "
                        "o.payment_status, o.created_at, u.email "
                        "FROM orders o JOIN users u ON o.user_id = u.id ";
    
    std::vector<std::string> conditions;
    if (!statusFilter.empty()) {
        conditions.push_back("o.status = '" + statusFilter + "'");
    }
    if (!userIdFilter.empty()) {
        conditions.push_back("o.user_id = '" + userIdFilter + "'");
    }
    
    if (!conditions.empty()) {
        query += "WHERE ";
        for (size_t i = 0; i < conditions.size(); i++) {
            if (i > 0) query += " AND ";
            query += conditions[i];
        }
    }
    
    query += " ORDER BY o.created_at DESC LIMIT $" + std::to_string(1) + " OFFSET $" + std::to_string(2);
    
    auto result = conn->executeQuery(query, {std::to_string(limit), std::to_string(offset)});
    
    json orders = json::array();
    int numRows = PQntuples(result);
    
    for (int i = 0; i < numRows; i++) {
        json order;
        order["id"] = PQgetvalue(result, i, 0);
        order["orderNumber"] = PQgetvalue(result, i, 1);
        order["userId"] = PQgetvalue(result, i, 2);
        order["status"] = PQgetvalue(result, i, 3);
        order["total"] = std::stod(PQgetvalue(result, i, 4));
        order["paymentStatus"] = PQgetvalue(result, i, 5);
        order["createdAt"] = PQgetvalue(result, i, 6);
        order["userEmail"] = PQgetvalue(result, i, 7);
        
        orders.push_back(order);
    }
    
    conn->clearResult(result);
    
    return orders.dump();
}

std::string OrderResolvers::getOrdersByDate(const std::string& startDate, const std::string& endDate, 
                                             const RequestContext& ctx) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT o.id, o.order_number, o.user_id, o.status, o.total_amount, "
                        "o.shipping_address, o.billing_address, o.notes, o.created_at, "
                        "u.email, u.first_name, u.last_name, u.phone, u.address "
                        "FROM orders o JOIN users u ON o.user_id = u.id "
                        "WHERE o.created_at >= $1 AND o.created_at <= $2 "
                        "ORDER BY o.created_at DESC";
    
    auto result = conn->executeQuery(query, {startDate, endDate});
    
    json orders = json::array();
    int numRows = PQntuples(result);
    
    for (int i = 0; i < numRows; i++) {
        json order;
        order["id"] = PQgetvalue(result, i, 0);
        order["orderNumber"] = PQgetvalue(result, i, 1);
        order["userId"] = PQgetvalue(result, i, 2);
        order["status"] = PQgetvalue(result, i, 3);
        order["totalAmount"] = std::stod(PQgetvalue(result, i, 4));
        order["shippingAddress"] = PQgetvalue(result, i, 5);
        order["billingAddress"] = PQgetvalue(result, i, 6);
        order["notes"] = PQgetvalue(result, i, 7);
        order["createdAt"] = PQgetvalue(result, i, 8);
        order["userEmail"] = PQgetvalue(result, i, 9);
        order["userFirstName"] = PQgetvalue(result, i, 10);
        order["userLastName"] = PQgetvalue(result, i, 11);
        order["userPhone"] = PQgetvalue(result, i, 12);
        order["userAddress"] = PQgetvalue(result, i, 13);
        
        orders.push_back(order);
    }
    
    conn->clearResult(result);
    
    return orders.dump();
}