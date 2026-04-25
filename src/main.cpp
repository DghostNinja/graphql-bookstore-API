#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <sstream>
#include <fstream>
#include <vector>
#include <ctime>
#include <csignal>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <pthread.h>
#include <curl/curl.h>

#include "utils.h"
#include "user_manager.h"
#include "book_manager.h"
#include "order_manager.h"
#include "extra_features.h"
#include "graphql_handler.h"
#include "db_manager.h"
#include "database/connection.h"
#include "network_manager.h"
#include "html_generator.h"
#include "rate_limiter.h"
#include "payment_handler.h"

#define PORT (getenv("PORT") ? atoi(getenv("PORT")) : 4000)
#define BUFFER_SIZE 65536
#define READ_TIMEOUT_SEC 5
#define JWT_SECRET (getenv("JWT_SECRET") ? getenv("JWT_SECRET") : "CHANGE_ME_IN_PRODUCTION_real_jwt_secret_key_2024")
#define DB_CONN (getenv("DATABASE_URL") ? getenv("DATABASE_URL") : (getenv("DB_CONNECTION_STRING") ? getenv("DB_CONNECTION_STRING") : "dbname=bookstore_db user=bookstore_user password=bookstore_password host=localhost port=5432"))

using namespace std;

string findFile(const string& filename) {
    vector<string> searchPaths = {
        filename,
        "./" + filename,
        "/app/" + filename,
        "/app/../" + filename,
    };
    
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
        searchPaths.push_back(string(cwd) + "/" + filename);
        searchPaths.push_back(string(cwd) + "/../" + filename);
    }
    
    for (const auto& path : searchPaths) {
        ifstream testFile(path);
        if (testFile) {
            return path;
        }
    }
    return "";
}

void signalHandler(int signal) {
    cout << "\nShutting down server..." << endl;
    if (dbConn) PQfinish(dbConn);
    exit(0);
}

void cleanupPort() {
    cout << "Checking for existing processes on port " << PORT << "..." << endl;
    string cmd = "lsof -ti:" + to_string(PORT) + " 2>/dev/null | xargs kill -9 2>/dev/null";
    system(cmd.c_str());
    string cmd2 = "fuser -k " + to_string(PORT) + "/tcp 2>/dev/null";
    system(cmd2.c_str());
    sleep(1);
    cout << "Port cleanup complete." << endl;
}

void* keepAliveThread(void* arg) {
    int port = PORT;
    CURL* curl = curl_easy_init();
    if (!curl) return NULL;
    
    while (true) {
        sleep(480);
        if (curl) {
            string fullUrl = string("http://127.0.0.1:") + to_string(port) + "/graphql";
            curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
            CURLcode res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                cerr << "[KEEPALIVE] Ping successful" << endl;
            }
        }
    }
    curl_easy_cleanup(curl);
    return NULL;
}

bool readFullRequest(int clientSocket, string& request, int timeoutSec) {
    char buffer[BUFFER_SIZE];
    request.clear();
    
    fd_set readfds;
    struct timeval tv;
    
    while (true) {
        FD_ZERO(&readfds);
        FD_SET(clientSocket, &readfds);
        tv.tv_sec = timeoutSec;
        tv.tv_usec = 0;
        
        int ready = select(clientSocket + 1, &readfds, NULL, NULL, &tv);
        if (ready <= 0) {
            return !request.empty();
        }
        
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived <= 0) {
            return !request.empty();
        }
        
        buffer[bytesReceived] = 0;
        request += buffer;
        
        size_t headerEnd = request.find("\r\n\r\n");
        if (headerEnd == string::npos) {
            headerEnd = request.find("\n\n");
        }
        
        if (headerEnd != string::npos) {
            size_t contentLengthPos = request.find("Content-Length:");
            if (contentLengthPos == string::npos) {
                contentLengthPos = request.find("content-length:");
            }
            
            if (contentLengthPos != string::npos) {
                size_t lineEnd = request.find("\r\n", contentLengthPos);
                if (lineEnd == string::npos) lineEnd = request.find("\n", contentLengthPos);
                
                if (lineEnd != string::npos) {
                    string lengthStr = request.substr(contentLengthPos + 15, lineEnd - contentLengthPos - 15);
                    while (!lengthStr.empty() && (lengthStr[0] == ' ' || lengthStr[0] == '\t')) {
                        lengthStr = lengthStr.substr(1);
                    }
                    
                    int contentLength = atoi(lengthStr.c_str());
                    size_t bodyStart = headerEnd + 4;
                    size_t currentBodyLength = request.length() - bodyStart;
                    
                    if (contentLength > 0 && (int)currentBodyLength < contentLength) {
                        continue;
                    }
                }
            }
            
            return true;
        }
        
        if (request.length() > BUFFER_SIZE * 2) {
            return true;
        }
    }
    
    return !request.empty();
}

bool requiresAuthentication(const string& query) {
    vector<string> authRequiredMutations = {
        "addToCart", "removeFromCart", "applyCoupon", "createOrder",
        "checkout", "cancelOrder", "createReview",
        "deleteReview", "registerWebhook", "testWebhook", "updateProfile",
        "logout"
    };
    
    for (const auto& keyword : authRequiredMutations) {
        if (query.find(keyword + "(") != string::npos || query.find(keyword + " {") != string::npos) {
            return true;
        }
    }
    
    if (query.find("query { cart") != string::npos || 
        query.find("query{cart") != string::npos ||
        query.find("query { orders") != string::npos ||
        query.find("query{orders") != string::npos ||
        query.find("query { myReviews") != string::npos ||
        query.find("query{myReviews") != string::npos ||
        query.find("query { webhooks") != string::npos ||
        query.find("query{webhooks") != string::npos ||
        query.find("query { me") != string::npos ||
        query.find("query{me") != string::npos ||
        query.find("mutation { cart") != string::npos ||
        query.find("mutation{cart") != string::npos) {
        return true;
    }
    
    return false;
}

bool isAuthorizationError(const string& responseBody) {
    vector<string> authzPatterns = {
        "only", "forbidden", "access denied", "not authorized", 
        "permission denied", "insufficient privileges", "unauthorized access",
        "admin required", "admins only", "not allowed"
    };
    
    string lowerResponse = responseBody;
    for (char& c : lowerResponse) {
        c = tolower(c);
    }
    
    for (const auto& pattern : authzPatterns) {
        if (lowerResponse.find(pattern) != string::npos) {
            return true;
        }
    }
    return false;
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    
    cout << "Connecting to database..." << endl;
    
    int maxRetries = 5;
    int retryDelay = 3;
    for (int attempt = 1; attempt <= maxRetries; attempt++) {
        if (connectDatabase()) {
            break;
        }
        cerr << "Database connection attempt " << attempt << " failed, retrying in " << retryDelay << "s..." << endl;
        sleep(retryDelay);
        retryDelay *= 2;
    }
    
    if (!isDbConnected()) {
        cerr << "WARNING: Could not connect to database after " << maxRetries << " attempts" << endl;
        cerr << "Server will start in degraded mode - some features may not work" << endl;
    } else {
        DatabasePool::getInstance().initialize(DB_CONN, 10);
    }
    
    cleanupPort();
    
    loadUsersCache();
    loadAuthorsCache();
    loadBooksCache();
    loadCartCache();
    loadOrdersCache();
    loadReviewsCache();
    loadWebhooksCache();

    cout << "Loaded " << usersCache.size() << " users from database" << endl;
    cout << "Loaded " << authorsCache.size() << " authors from database" << endl;
    cout << "Loaded " << booksCache.size() << " books from database" << endl;
    cout << "Loaded " << cartCache.size() << " carts from database" << endl;
    cout << "Loaded " << ordersCache.size() << " orders from database" << endl;
    cout << "Loaded " << reviewsCache.size() << " reviews from database" << endl;
    cout << "Loaded " << webhooksCache.size() << " webhooks from database" << endl;
    
    pthread_t keepalive_id;
    pthread_create(&keepalive_id, NULL, keepAliveThread, NULL);
    pthread_detach(keepalive_id);

    pthread_t ratelimit_id;
    pthread_create(&ratelimit_id, NULL, rateLimitCleanup, NULL);
    pthread_detach(ratelimit_id);
    
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << "Failed to create socket" << endl;
        return 1;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Bind failed" << endl;
        return 1;
    }
    
    if (listen(serverSocket, 10) < 0) {
        cerr << "Listen failed" << endl;
        return 1;
    }
    
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
    
    cout << "========================================" << endl;
    cout << "  Vulnerable GraphQL Bookstore API     " << endl;
    cout << "  Security Learning Environment          " << endl;
    cout << "========================================" << endl;
    cout << "Starting server on port " << PORT << endl;
    cout << "GraphQL endpoint: http://localhost:" << PORT << "/graphql" << endl;
    cout << endl;
    cout << "WARNING: This API contains intentional security vulnerabilities." << endl;
    cout << "           DO NOT USE IN PRODUCTION!" << endl;
    cout << "========================================" << endl;
    cout << endl;
    
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    
    while (true) {
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
        if (clientSocket < 0) {
            continue;
        }
        
        string request;
        if (!readFullRequest(clientSocket, request, READ_TIMEOUT_SEC)) {
            close(clientSocket);
            continue;
        }
        
        bool isGetRequest = (request.find("GET ") == 0);
        bool isHealthRequest = (request.find("GET /health") == 0);
        bool isPostmanRequest = (request.find("GET /graphql.json") == 0);
        bool isPostRequest = (request.find("POST ") == 0 && request.find("/graphql") != string::npos);
        bool isOptionsRequest = (request.find("OPTIONS") == 0);

        if (isOptionsRequest) {
            string response = "HTTP/1.1 204 No Content\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: POST, GET, OPTIONS\r\nAccess-Control-Allow-Headers: Content-Type, Authorization\r\nContent-Length: 0\r\n\r\n";
            send(clientSocket, response.c_str(), response.length(), 0);
            close(clientSocket);
            continue;
        }

        if (isHealthRequest) {
            string status = isDbConnected() ? "OK" : "DEGRADED";
            string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nX-DB-Status: " + status + "\r\nContent-Length: 2\r\n\r\nOK";
            send(clientSocket, response.c_str(), response.length(), 0);
            close(clientSocket);
            continue;
        }

        if (isPostmanRequest) {
            string filePath = findFile("graphql.json");
            ifstream postmanFile(filePath);
            if (!postmanFile) {
                cerr << "[POSTMAN] File not found, searched paths for graphql.json" << endl;
                string response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot Found";
                send(clientSocket, response.c_str(), response.length(), 0);
                close(clientSocket);
                continue;
            }
            stringstream buffer;
            buffer << postmanFile.rdbuf();
            string postmanContent = buffer.str();
            string response = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\nContent-Disposition: attachment; filename=\"graphql.json\"\r\nContent-Length: " + to_string(postmanContent.length()) + "\r\n\r\n" + postmanContent;
            send(clientSocket, response.c_str(), response.length(), 0);
            close(clientSocket);
            continue;
        }

        if (isGetRequest) {
            string html = generateLandingHTML();
            string response = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/html\r\nContent-Length: " + to_string(html.length()) + "\r\n\r\n";
            response += html;
            send(clientSocket, response.c_str(), response.length(), 0);
            close(clientSocket);
            continue;
        }
        
        if (isPostRequest) {
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            string clientIPStr(clientIP);

            string retryAfter;
            if (!checkRateLimit(clientIPStr, retryAfter)) {
                cerr << "[RATELIMIT] Rate limit exceeded for IP: " << clientIPStr << endl;
                string responseBody = "{\"errors\":[{\"message\":\"Rate limit exceeded. Try again in " + retryAfter + " seconds.\"}]}";
                string response = "HTTP/1.1 429 Too Many Requests\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\nRetry-After: " + retryAfter + "\r\nContent-Length: " + to_string(responseBody.length()) + "\r\n\r\n" + responseBody;
                send(clientSocket, response.c_str(), response.length(), 0);
                close(clientSocket);
                continue;
            }

            string authHeaderStr = "";
            size_t authPos = request.find("Authorization:");
            if (authPos == string::npos) authPos = request.find("authorization:");
            if (authPos != string::npos) {
                size_t lineEnd = request.find("\r\n", authPos);
                if (lineEnd == string::npos) lineEnd = request.find("\n", authPos);
                if (lineEnd != string::npos) {
                    authHeaderStr = request.substr(authPos + 14, lineEnd - authPos - 14);
                }
            }

            if (!authHeaderStr.empty() && authHeaderStr[0] == ' ') {
                authHeaderStr = authHeaderStr.substr(1);
            }

            AuthResult authResult = extractAuthUserWithError(authHeaderStr);
            User currentUser = authResult.user;

            size_t headerEnd = request.find("\r\n\r\n");
            if (headerEnd == string::npos) headerEnd = request.find("\n\n");

            string queryStr = "";

            if (headerEnd != string::npos) {
                string body = request.substr(headerEnd + 4);

                cerr << "[DEBUG] Raw body: " << body << endl;

                if (!isValidJson(body)) {
                    string errorResponse = "{\"errors\":[{\"message\":\"Invalid JSON: Malformed request body\"}]}";
                    cerr << "[DEBUG] Response: " << errorResponse << endl;
                    string response = "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\nContent-Length: " +
                        to_string(errorResponse.length()) + "\r\n\r\n" + errorResponse;
                    send(clientSocket, response.c_str(), response.length(), 0);
                    close(clientSocket);
                    continue;
                }

                size_t queryPos = body.find("\"query\"");
                if (queryPos != string::npos) {
                    size_t colonPos = body.find(":", queryPos);
                    if (colonPos != string::npos) {
                        size_t valueStart = colonPos + 1;
                        while (valueStart < body.length() && 
                               (body[valueStart] == ' ' || body[valueStart] == '\t' || body[valueStart] == '\n' || body[valueStart] == '\r')) {
                            valueStart++;
                        }
                        if (valueStart < body.length() && body[valueStart] == '"') {
                            string value;
                            bool escaped = false;
                            for (size_t i = valueStart + 1; i < body.length(); i++) {
                                char c = body[i];
                                if (escaped) {
                                    if (c == 'n') value += '\n';
                                    else if (c == 't') value += '\t';
                                    else if (c == 'r') value += '\r';
                                    else if (c == '\\') value += '\\';
                                    else if (c == '"') value += '"';
                                    else value += c;
                                    escaped = false;
                                } else if (c == '\\') {
                                    escaped = true;
                                } else if (c == '"') {
                                    break;
                                } else {
                                    value += c;
                                }
                            }
                            queryStr = value;
                        }
                    }
                }
            }

            cerr << "[DEBUG] Raw query: " << queryStr << endl;

            bool hasAuthHeader = !authHeaderStr.empty();
            if (!hasAuthHeader && requiresAuthentication(queryStr)) {
                string errorResponse = "{\"errors\":[{\"message\":\"Authentication required\"}]}";
                string response = "HTTP/1.1 401 Unauthorized\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\nContent-Length: " +
                    to_string(errorResponse.length()) + "\r\n\r\n" + errorResponse;
                send(clientSocket, response.c_str(), response.length(), 0);
                close(clientSocket);
                continue;
            }

            if (hasAuthHeader && !authResult.valid) {
                string errorResponse = "{\"errors\":[{\"message\":\"" + authResult.error + "\"}]}";
                string response = "HTTP/1.1 401 Unauthorized\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\nContent-Length: " +
                    to_string(errorResponse.length()) + "\r\n\r\n" + errorResponse;
                send(clientSocket, response.c_str(), response.length(), 0);
                close(clientSocket);
                continue;
            }

            bool isMutation = (queryStr.find("mutation {") != string::npos || 
                               queryStr.find("mutation(") != string::npos ||
                               queryStr.find("mutation{") != string::npos);

            string responseBody = handleRequest(queryStr, currentUser, isMutation);

            cerr << "[DEBUG] Response: " << responseBody << endl;

            string httpStatus = "200 OK";
            if (isAuthorizationError(responseBody)) {
                httpStatus = "403 Forbidden";
            }

            string response = "HTTP/1.1 " + httpStatus + "\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Headers: Content-Type, Authorization\r\nAccess-Control-Allow-Methods: POST, GET, OPTIONS\r\nContent-Type: application/json\r\nContent-Length: " +
                to_string(responseBody.length()) + "\r\nX-Content-Type-Options: nosniff\r\n\r\n" + responseBody;
            send(clientSocket, response.c_str(), response.length(), 0);
        }

        close(clientSocket);
    }
    
    if (dbConn) PQfinish(dbConn);
    curl_global_cleanup();
    
    return 0;
}
