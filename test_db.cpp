#include <iostream>
#include <cstring>
#include <postgresql/libpq-fe.h>

int main() {
    const char* connString = "dbname=bookstore_db user=bookstore_user password=bookstore_password host=localhost port=5432";
    
    std::cout << "Attempting to connect with: " << connString << std::endl;
    
    PGconn* conn = PQconnectdb(connString);
    
    if (PQstatus(conn) == CONNECTION_OK) {
        std::cout << "✓ Database connection successful!" << std::endl;
        
        PGresult* res = PQexec(conn, "SELECT COUNT(*) FROM users");
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            std::cout << "Users in database: " << PQgetvalue(res, 0, 0) << std::endl;
        } else {
            std::cout << "Query failed: " << PQerrorMessage(conn) << std::endl;
        }
        PQclear(res);
    } else {
        std::cout << "✗ Connection failed: " << PQerrorMessage(conn) << std::endl;
    }
    
    PQfinish(conn);
    return 0;
}
