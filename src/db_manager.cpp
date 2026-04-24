#include "db_manager.h"
#include <iostream>

PGconn* dbConn = nullptr;
std::map<std::string, PGresult*> preparedStatements;

bool connectDatabase() {
    std::string connStr = getenv("DATABASE_URL") ? getenv("DATABASE_URL") : 
                          (getenv("DB_CONNECTION_STRING") ? getenv("DB_CONNECTION_STRING") : 
                          "postgresql://bookstore_user:bookstore_password@localhost:5432/bookstore_db");
    
    std::cerr << "[DB] Connecting to: " << connStr << std::endl;

    dbConn = PQconnectdb(connStr.c_str());
    if (PQstatus(dbConn) != CONNECTION_OK) {
        std::cerr << "[DB] Connection FAILED: " << PQerrorMessage(dbConn) << std::endl;
        return false;
    }

    std::cerr << "[DB] Connected successfully" << std::endl;
    return true;
}

bool checkDatabaseConnection() {
    if (dbConn == nullptr) {
        return connectDatabase();
    }
    ConnStatusType status = PQstatus(dbConn);
    if (status != CONNECTION_OK) {
        std::cerr << "[DB] Connection lost, reconnecting..." << std::endl;
        PQfinish(dbConn);
        dbConn = nullptr;
        return connectDatabase();
    }
    return true;
}

bool isDbConnected() {
    if (dbConn == nullptr) {
        return false;
    }
    return PQstatus(dbConn) == CONNECTION_OK;
}

// Auto-reconnect wrapper for queries
PGresult* safeExec(const char* sql) {
    checkDatabaseConnection();
    return PQexec(dbConn, sql);
}

PGresult* safeExecParams(const char* sql, int nParams, const char* const* paramValues) {
    checkDatabaseConnection();
    return PQexecParams(dbConn, sql, nParams, nullptr, paramValues, nullptr, nullptr, 0);
}

PGconn* getConnection() {
    checkDatabaseConnection();
    return dbConn;
}

PGresult* executePrepared(const char* name, const char* sql, int nParams, const char* const* paramValues) {
    auto it = preparedStatements.find(name);
    if (it == preparedStatements.end()) {
        PGresult* res = PQprepare(dbConn, name, sql, nParams, nullptr);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "[DB] Failed to prepare statement " << name << ": " << PQerrorMessage(dbConn) << std::endl;
            PQclear(res);
            return nullptr;
        }
        preparedStatements[name] = res;
    }
    return PQexecPrepared(dbConn, name, nParams, paramValues, nullptr, nullptr, 0);
}
