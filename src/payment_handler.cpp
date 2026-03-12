#include <string>
#include <iostream>
#include <curl/curl.h>
#include <postgresql/libpq-fe.h>
#include <ctime>
#include <sstream>
#include <regex> // For std::regex
#include "utils.h"

extern PGconn* dbConn;

// Function to simulate charging a card at vulnbank.org
bool chargeVulnBankCard(const std::string& cardNumber, const std::string& expiry, const std::string& cvv, double amount, std::string& transactionId, std::string& message) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        std::string postFields = "card_number=" + cardNumber + "&expiry=" + expiry + "&cvv=" + cvv + "&amount=" + std::to_string(amount);
        std::string url = "http://vulnbank.org/api/charge"; // Simulating interaction

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // Add timeout

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            message = "curl_easy_perform() failed: " + std::string(curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return false;
        }

        // Simulate parsing vulnbank.org response
        // For simplicity, we'll look for a success string.
        // In a real scenario, you'd parse JSON/XML from readBuffer.
        if (readBuffer.find("\"success\":true") != std::string::npos) {
            transactionId = "VULN-" + std::to_string(time(0)) + "-" + cardNumber.substr(cardNumber.length() - 4);
            message = "Payment successful at vulnbank.org";
            curl_easy_cleanup(curl);
            return true;
        } else {
            message = "Payment failed at vulnbank.org. Response: " + readBuffer;
            curl_easy_cleanup(curl);
            return false;
        }
    }
    message = "Failed to initialize CURL";
    return false;
}

// Function to record payment in the database
bool recordPayment(const std::string& orderId, const std::string& userId, double amount, const std::string& transactionId, const std::string& status, std::string& errorMessage) {
    if (dbConn == nullptr) {
        errorMessage = "Database connection not available.";
        return false;
    }

    std::string sql = "INSERT INTO payment_transactions (order_id, user_id, amount, currency, status, transaction_id, created_at) VALUES ($1, $2, $3, $4, $5, $6, NOW())";
    const char* paramValues[6];
    paramValues[0] = orderId.c_str();
    paramValues[1] = userId.c_str();
    std::string amountStr = std::to_string(amount);
    paramValues[2] = amountStr.c_str();
    paramValues[3] = "USD"; // Assuming USD for now
    paramValues[4] = status.c_str();
    paramValues[5] = transactionId.c_str();

    PGresult* res = PQexecParams(dbConn, sql.c_str(), 6, nullptr, paramValues, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        errorMessage = "Failed to record payment in DB: " + std::string(PQerrorMessage(dbConn));
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}

// Main function to process payment for checkout
std::string processPayment(const std::string& userId, const std::string& orderId, double totalAmount, const std::string& cardNumber, const std::string& expiry, const std::string& cvv) {
    std::string transactionId = "";
    std::string paymentMessage = "";
    std::string paymentStatus = "failed";

    if (chargeVulnBankCard(cardNumber, expiry, cvv, totalAmount, transactionId, paymentMessage)) {
        paymentStatus = "completed";
    }

    std::string dbErrorMessage;
    if (!recordPayment(orderId, userId, totalAmount, transactionId, paymentStatus, dbErrorMessage)) {
        std::cerr << "[ERROR] Failed to record payment in DB: " << dbErrorMessage << std::endl;
        // If vulnbank.org charge was successful but DB record failed, we notify user but consider payment potentially done
        if (paymentStatus == "completed") {
             return "{\"success\":false, \"message\":\"Payment processed, but failed to record in DB. Please contact support.\", \"transactionId\":\"" + escapeJson(transactionId) + "\"}";
        }
        return "{\"success\":false, \"message\":\"" + escapeJson(dbErrorMessage) + "\"}";
    }

    if (paymentStatus == "completed") {
        // Update order status in main database to 'paid'
        std::string updateSql = "UPDATE orders SET status = $1, payment_status = $2 WHERE id = $3 AND user_id = $4";
        const char* updateParamValues[4];
        updateParamValues[0] = "paid";
        updateParamValues[1] = "paid";
        updateParamValues[2] = orderId.c_str();
        updateParamValues[3] = userId.c_str();
        PGresult* updateRes = PQexecParams(dbConn, updateSql.c_str(), 4, nullptr, updateParamValues, nullptr, nullptr, 0);
        if (PQresultStatus(updateRes) != PGRES_COMMAND_OK) {
            std::cerr << "[ERROR] Failed to update order status to paid for order " << orderId << ": " << PQerrorMessage(dbConn) << std::endl;
            // Continue, as payment might still be successful despite order update failure
        }
        PQclear(updateRes);

        return "{\"success\":true, \"message\":\"Payment successful and recorded. WARNING: Do not use real card details - this is a test environment.\", \"transactionId\":\"" + escapeJson(transactionId) + "\"}";
    } else {
        return "{\"success\":false, \"message\":\"" + escapeJson(paymentMessage) + "\"}";
    }
}
