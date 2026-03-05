#include <iostream>
#include <string>

int main() {
    // Test how the query extraction works
    std::string body = "{\"query\": \"{ __schema { queryType { name fields { name description } } } }\"}";
    
    std::cout << "Body: " << body << std::endl;
    std::cout << std::endl;
    
    // Simulate the extraction logic
    size_t queryPos = body.find("\"query\"");
    std::cout << "Found 'query' at position: " << queryPos << std::endl;
    
    if (queryPos != std::string::npos) {
        size_t colonPos = body.find(":", queryPos);
        std::cout << "Found ':' at position: " << colonPos << std::endl;
        
        if (colonPos != std::string::npos) {
            size_t quoteStart = body.find("\"", colonPos + 1);
            std::cout << "Found opening quote at: " << quoteStart << std::endl;
            
            if (quoteStart != std::string::npos) {
                size_t actualStart = quoteStart + 1;
                size_t quoteEnd = body.find("\"", actualStart);
                std::cout << "Found closing quote at: " << quoteEnd << std::endl;
                
                // Handle escaped quotes
                while (quoteEnd != std::string::npos && quoteEnd > 0 && body[quoteEnd - 1] == '\\') {
                    std::cout << "Skipping escaped quote at: " << quoteEnd << std::endl;
                    quoteEnd = body.find("\"", quoteEnd + 1);
                    std::cout << "Next quote at: " << quoteEnd << std::endl;
                }
                
                if (quoteEnd != std::string::npos) {
                    std::string queryStr = body.substr(actualStart, quoteEnd - actualStart);
                    std::cout << std::endl;
                    std::cout << "Extracted query: " << queryStr << std::endl;
                    
                    // Check if it contains __schema
                    if (queryStr.find("__schema") != std::string::npos) {
                        std::cout << "✓ Query contains __schema" << std::endl;
                    } else {
                        std::cout << "✗ Query does NOT contain __schema" << std::endl;
                    }
                }
            }
        }
    }
    
    return 0;
}
