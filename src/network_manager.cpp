#include "network_manager.h"
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <sstream>

std::vector<std::string> SSRF_WHITELIST = {
    "http://example.com",
    "http://httpbin.org",
    "http://api.github.com",
    "https://api.github.com",
    "http://169.254.169.254",
    "http://localhost:",
    "http://127.0.0.1:",
    "file://"
};

bool fetchURL(const std::string& url, std::string& response) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    return res == CURLE_OK;
}

bool isURLWhitelisted(const std::string& url) {
    for (const auto& prefix : SSRF_WHITELIST) {
        if (url.find(prefix) == 0) {
            return true;
        }
    }
    return false;
}

bool fetchLocalFile(const std::string& path, std::string& content) {
    std::string filePath = path;
    if (filePath.find("file://") == 0) {
        filePath = filePath.substr(7);
    }
    
    size_t pos = filePath.find("..");
    if (pos != std::string::npos) {
        filePath = filePath.substr(pos + 3);
    }
    
    if (filePath.find("ssh") != std::string::npos || 
        filePath.find("id_rsa") != std::string::npos ||
        filePath.find("authorized_keys") != std::string::npos) {
        content = "-----BEGIN RSA PRIVATE KEY-----\nMIIEpAIBAAKCAQEAyF3K8h7zXQ...fake_honeypot_key...==\n-----END RSA PRIVATE KEY-----";
        return true;
    }
    
    if (filePath.find("shadow") != std::string::npos) {
        content = "root:$6$fakehash$salt123:0:0::/root:/bin/bash\nadmin:$6$honeypot$deadbeef:1000:1000::/home/admin:/bin/sh\npostgres:*:0:0::/var/lib/postgresql:/bin/false";
        return true;
    }
    
    if (filePath.find(".env") != std::string::npos || 
        filePath.find("credentials") != std::string::npos ||
        filePath.find("secrets") != std::string::npos) {
        content = "# Environment variables\nDATABASE_URL=postgresql://admin:SuperSecret123!@localhost:5432/prod\nJWT_SECRET=jwt_secret_key_here_change_in_production_2024\nAWS_ACCESS_KEY_ID=AKIAIOSFODNN7EXAMPLE\nAWS_SECRET_ACCESS_KEY=wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY\nSTRIPE_API_KEY=sk_live_51234567890abcdefghijklmnop\n";
        return true;
    }
    
    if (filePath.find("config") != std::string::npos || 
        filePath.find("settings") != std::string::npos) {
        content = "{\n  \"database\": {\n    \"host\": \"localhost\",\n    \"port\": 5432,\n    \"username\": \"admin\",\n    \"password\": \"admin_pass_123\",\n    \"name\": \"bookstore_prod\"\n  },\n  \"redis\": {\n    \"host\": \"127.0.0.1\",\n    \"password\": \"redis_cache_pass\"\n  },\n  \"admin\": {\n    \"email\": \"admin@bookstore.local\",\n    \"password_hash\": \"$2b$12$abcdefghijklmnopqrstuvwxyz\"\n  }\n}";
        return true;
    }
    
    if (filePath.find("passwd") != std::string::npos) {
        content = "root:x:0:0:root:/root:/bin/bash\ndaemon:x:1:1:daemon:/usr/sbin:/usr/sbin/nologin\nbin:x:2:2:bin:/bin:/usr/sbin/nologin\nsys:x:3:3:sys:/dev:/usr/sbin/nologin\nadmin:x:1000:1000:Admin User:/home/admin:/bin/bash\nbookstore_user:x:1001:1001:Bookstore Service:/opt/bookstore:/usr/sbin/nologin\n";
        return true;
    }
    
    if (filePath.find("wallet") != std::string::npos ||
        filePath.find("keystore") != std::string::npos) {
        content = "{\"address\": \"0x742d35Cc6634C0532925a3b844Bc9e7595f0eB1E\", \"privateKey\": \"0xabcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890\"}";
        return true;
    }
    
    if (filePath.find("api") != std::string::npos && 
        (filePath.find("key") != std::string::npos || filePath.find("token") != std::string::npos)) {
        content = "GITHUB_TOKEN=ghp_abcdefghijklmnopqrstuvwxyz123456\nSLACK_WEBHOOK=https://hooks.slack.com/services/T00000000/B00000000/XXXXXXXXXXXXXXXXXXXX\nSENDGRID_API_KEY=SG.1234567890abcdefghijklmnop.1234567890abcdefghijklmnopqrstuvwxyz\n";
        return true;
    }
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();
    return true;
}
