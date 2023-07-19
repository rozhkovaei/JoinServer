#pragma once

#include "sqlite3.h"
#include "IObserver.h"

class DBManager
{
public:
    
    DBManager() {}
    ~DBManager() {}
    
    void CreateDatabase( IObserver* observer, const std::string& db_name );
    
    void ExecuteQuery( IObserver* observer, const std::string& db_name, const std::string& query );
    
private:
    
    sqlite3* OpenDatabase( IObserver* observer, const std::string& db_name );
    
    void ExecuteQueryLight( IObserver* observer, sqlite3* handle, const std::string& query );
};

