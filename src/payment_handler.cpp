#include <string>
#include <iostream>
#include <curl/curl.h>
#include <postgresql/libpq-fe.h>
#include <ctime>
#include <sstream>
#include <regex>
#include <cstring>
#include "utils.h"

extern PGconn* dbConn;

const std::string VULNBANK_API_KEY = "vk_fe675fe7aaee830b6fed09b64e034f84dcbdaeb429d9cccd4ebb90e15af8dd71";
const std::string VULNBANK_API_URL = "https://vulnbank.org/api/v1/payments/charge";

struct MemoryStruct {
    char* memory;
    size_t size;
};

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;
    char* ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == nullptr) return 0;
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

bool chargeWithVulnbank(double amount, const std::string& cardNumber, const std::string& expiry, const std::string& cvv, const std::string& orderId, std::string& transactionId, std::string& message) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        message = "Failed to initialize CURL";
        return false;
    }

    struct MemoryStruct chunk;
    chunk.memory = (char*)malloc(1);
    chunk.size = 0;

    std::string jsonPayload = "{";
    jsonPayload += "\"amount\":" + std::to_string(amount) + ",";
    jsonPayload += "\"currency\":\"USD\",";
    jsonPayload += "\"card_number\":\"" + cardNumber + "\",";
    jsonPayload += "\"cvv\":\"" + cvv + "\",";
    jsonPayload += "\"expiry_date\":\"" + expiry + "\",";
    jsonPayload += "\"merchant_order_id\":\"" + orderId + "\",";
    jsonPayload += "\"description\":\"GraphQL Bookstore purchase\"";
    jsonPayload += "}";

    struct curl_slist* headers = nullptr;
    std::string apiKeyHeader = "X-Merchant-Api-Key: " + VULNBANK_API_KEY;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, apiKeyHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, VULNBANK_API_URL.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    CURLcode res = curl_easy_perform(curl);
    bool success = false;

    if (res == CURLE_OK) {
        std::string response(chunk.memory, chunk.size);
        
        if (response.find("\"status\":\"completed\"") != std::string::npos || 
            response.find("\"status\": \"completed\"") != std::string::npos) {
            success = true;
            
            size_t idPos = response.find("\"id\":");
            if (idPos != std::string::npos) {
                size_t idStart = idPos + 5;
                size_t idEnd = response.find(",", idStart);
                if (idEnd == std::string::npos) idEnd = response.find("}", idStart);
                transactionId = response.substr(idStart, idEnd - idStart);
                transactionId = std::regex_replace(transactionId, std::regex("[\" ]"), "");
            }
            
            if (transactionId.empty()) {
                transactionId = "VULN-" + std::to_string(time(0));
            }
            message = "Payment successful";
        } else {
            size_t msgPos = response.find("\"message\":");
            if (msgPos != std::string::npos) {
                size_t msgStart = msgPos + 9;
                size_t msgEnd = response.find("}", msgStart);
                if (msgEnd == std::string::npos) msgEnd = response.size();
                message = response.substr(msgStart, msgEnd - msgStart);
                message = std::regex_replace(message, std::regex("[\"{}]"), "");
                
                if (response.find("invalid_card") != std::string::npos) {
                    message += ". Please register a virtual card at vulnbank.org";
                }
            } else {
                message = "Payment failed: " + response;
            }
        }
    } else {
        message = "Failed to connect to payment gateway: " + std::string(curl_easy_strerror(res));
    }

    free(chunk.memory);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return success;
}

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
    paramValues[3] = "USD";
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

std::string processPayment(const std::string& userId, const std::string& orderId, double totalAmount, const std::string& cardNumber, const std::string& expiry, const std::string& cvv) {
    std::string transactionId = "";
    std::string paymentMessage = "";
    std::string paymentStatus = "failed";

    std::cerr << "[PAYMENT] Charging card " << cardNumber.substr(cardNumber.length() - 4) << " for $" << totalAmount << " via Vulnbank" << std::endl;

    if (chargeWithVulnbank(totalAmount, cardNumber, expiry, cvv, orderId, transactionId, paymentMessage)) {
        paymentStatus = "completed";
    }

    std::string dbErrorMessage;
    if (!recordPayment(orderId, userId, totalAmount, transactionId, paymentStatus, dbErrorMessage)) {
        std::cerr << "[ERROR] Failed to record payment in DB: " << dbErrorMessage << std::endl;
        if (paymentStatus == "completed") {
             return "{\"success\":false, \"message\":\"Payment processed, but failed to record in DB. Please contact support.\", \"transactionId\":\"" + escapeJson(transactionId) + "\"}";
        }
        return "{\"success\":false, \"message\":\"" + escapeJson(dbErrorMessage) + "\"}";
    }

    if (paymentStatus == "completed") {
        std::string updateSql = "UPDATE orders SET status = $1, payment_status = $2 WHERE id = $3 AND user_id = $4";
        const char* updateParamValues[4];
        updateParamValues[0] = "processing";
        updateParamValues[1] = "completed";
        updateParamValues[2] = orderId.c_str();
        updateParamValues[3] = userId.c_str();
        PGresult* updateRes = PQexecParams(dbConn, updateSql.c_str(), 4, nullptr, updateParamValues, nullptr, nullptr, 0);
        if (PQresultStatus(updateRes) != PGRES_COMMAND_OK) {
            std::cerr << "[ERROR] Failed to update order status to paid for order " << orderId << ": " << PQerrorMessage(dbConn) << std::endl;
        }
        PQclear(updateRes);

        return "{\"success\":true, \"message\":\"Payment successful\", \"transactionId\":\"" + escapeJson(transactionId) + "\"}";
    } else {
        return "{\"success\":false, \"message\":\"" + escapeJson(paymentMessage) + "\"}";
    }
}