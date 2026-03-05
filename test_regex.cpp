#include <iostream>
#include <string>
#include <regex>

using namespace std;

string extractValue(const string& query, const string& key) {
    string pattern = key + "\\s*:\\s*\\\"([^\\\"]+)\\\"";
    regex re(pattern);
    smatch match;
    if (regex_search(query, match, re) && match.size() > 1) {
        return match[1].str();
    }
    return "";
}

int main() {
    // Test query like what comes from the HTTP body
    string query1 = "mutation { login(username: \"admin\", password: \"password123\") { success } }";
    string query2 = "mutation { login(username: \\\"admin\\\", password: \\\"password123\\\") { success } }";
    
    cout << "Test 1 (normal quotes):" << endl;
    cout << "  Query: " << query1 << endl;
    cout << "  Username: '" << extractValue(query1, "username") << "'" << endl;
    cout << "  Password: '" << extractValue(query1, "password") << "'" << endl;
    
    cout << "\nTest 2 (escaped quotes):" << endl;
    cout << "  Query: " << query2 << endl;
    cout << "  Username: '" << extractValue(query2, "username") << "'" << endl;
    cout << "  Password: '" << extractValue(query2, "password") << "'" << endl;
    
    return 0;
}
