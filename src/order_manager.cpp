#include "order_manager.h"
#include "book_manager.h"
#include "user_manager.h"
#include "utils.h"
#include "payment_handler.h"

#include <iostream>
#include <postgresql/libpq-fe.h>
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib> // For rand(), srand()
#include <sstream>

extern PGconn* dbConn;

std::map<std::string, std::vector<CartItem>> cartCache;
std::map<std::string, Order> ordersCache;

void loadCartCache() {
    PGresult* res = PQexec(dbConn, "SELECT ci.id, ci.cart_id, ci.book_id, ci.quantity, COALESCE(b.price, 0) as price "
                                     "FROM cart_items ci "
                                     "JOIN shopping_carts sc ON ci.cart_id = sc.id "
                                     "JOIN users u ON sc.user_id = u.id "
                                     "JOIN books b ON ci.book_id = b.id");

    if (PQresultStatus(res) == PGRES_TUPLES_OK) {
        int rows = PQntuples(res);
        for (int i = 0; i < rows; i++) {
            CartItem item;
            item.id = atoi(PQgetvalue(res, i, 0));
            item.cartId = PQgetvalue(res, i, 1);
            item.bookId = atoi(PQgetvalue(res, i, 2));
            item.quantity = atoi(PQgetvalue(res, i, 3));
            item.price = atof(PQgetvalue(res, i, 4));
            cartCache[item.cartId].push_back(item);
        }
    }
    PQclear(res);
}

void loadOrdersCache() {
    PGresult* res = PQexec(dbConn, "SELECT o.id, o.user_id, o.order_number, o.status, o.subtotal, o.tax_amount, "
                                     "o.shipping_amount, o.discount_amount, o.total_amount, o.shipping_address, "
                                     "o.billing_address, o.payment_status, o.created_at "
                                     "FROM orders o");

    if (PQresultStatus(res) == PGRES_TUPLES_OK) {
        int rows = PQntuples(res);
        for (int i = 0; i < rows; i++) {
            Order order;
            order.id = PQgetvalue(res, i, 0);
            order.userId = PQgetvalue(res, i, 1);
            order.orderNumber = PQgetvalue(res, i, 2);
            order.status = PQgetvalue(res, i, 3);
            order.subtotal = atof(PQgetvalue(res, i, 4));
            order.taxAmount = atof(PQgetvalue(res, i, 5));
            order.shippingAmount = atof(PQgetvalue(res, i, 6));
            order.discountAmount = atof(PQgetvalue(res, i, 7));
            order.totalAmount = atof(PQgetvalue(res, i, 8));
            order.shippingAddress = PQgetvalue(res, i, 9) ? PQgetvalue(res, i, 9) : "";
            order.billingAddress = PQgetvalue(res, i, 10) ? PQgetvalue(res, i, 10) : "";
            order.paymentStatus = PQgetvalue(res, i, 11);
            order.createdAt = PQgetvalue(res, i, 12);
            ordersCache[order.id] = order;
        }
    }
    PQclear(res);

    PGresult* itemsRes = PQexec(dbConn, "SELECT id, order_id, book_id, book_title, book_isbn, quantity, unit_price, total_price FROM order_items");
    if (PQresultStatus(itemsRes) == PGRES_TUPLES_OK) {
        int rows = PQntuples(itemsRes);
        for (int i = 0; i < rows; i++) {
            OrderItem item;
            item.id = atoi(PQgetvalue(itemsRes, i, 0));
            item.orderId = PQgetvalue(itemsRes, i, 1);
            item.bookId = atoi(PQgetvalue(itemsRes, i, 2));
            item.bookTitle = PQgetvalue(itemsRes, i, 3) ? PQgetvalue(itemsRes, i, 3) : "";
            item.bookIsbn = PQgetvalue(itemsRes, i, 4) ? PQgetvalue(itemsRes, i, 4) : "";
            item.quantity = atoi(PQgetvalue(itemsRes, i, 5));
            item.unitPrice = atof(PQgetvalue(itemsRes, i, 6));
            item.totalPrice = atof(PQgetvalue(itemsRes, i, 7));
            if (ordersCache.find(item.orderId) != ordersCache.end()) {
                ordersCache[item.orderId].items.push_back(item);
            }
        }
    }
    PQclear(itemsRes);
}
