#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <string>
#include <map>
#include <postgresql/libpq-fe.h>

extern PGconn* dbConn;
extern std::map<std::string, PGresult*> preparedStatements;

bool connectDatabase();
bool checkDatabaseConnection();
bool isDbConnected();
PGconn* getConnection();
PGresult* executePrepared(const char* name, const char* sql, int nParams, const char* const* paramValues);
PGresult* safeExec(const char* sql);
PGresult* safeExecParams(const char* sql, int nParams, const char* const* paramValues);

#endif // DB_MANAGER_H
