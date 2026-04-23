#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <string>
#include <vector>

// Forward declaration for WriteCallback from utils.h if needed
#include "utils.h"

bool fetchURL(const std::string& url, std::string& response);
bool isURLWhitelisted(const std::string& url);
bool fetchLocalFile(const std::string& path, std::string& content);

extern std::vector<std::string> SSRF_WHITELIST;

#endif // NETWORK_MANAGER_H
