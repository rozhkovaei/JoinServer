#include <iostream>
#include <sstream>
#include "db_manager.h"

using namespace std;

int print_results( void* observer, int columns, char **data, char ** /*names*/ )
{
    string result;
    
    if( columns )
        result = data[0]; // данные id
    
    for ( int i = 1; i < columns; ++i )
    {
        result += ", ";
        result += data[i] ? data[i] : "";
      //  std::cout << names[i] << " = " << (data[i] ? data[i] : "NULL") << std::endl;
    }
    result += "\n";
    
    if( observer )
        reinterpret_cast < IObserver * >( observer )->update( move( result ) );
    
    return 0;
}

sqlite3* DBManager::OpenDatabase( IObserver* observer, const std::string& db_name )
{
    sqlite3* handle;
    if ( sqlite3_open( db_name.c_str(), &handle ) )
    {
        stringstream ss;
        ss << "ERR " << sqlite3_errmsg( handle ) << "\n";
        observer->update( ss.str() );

        sqlite3_close( handle );
        return nullptr;
    }
    return handle;
}

void DBManager::CreateDatabase( IObserver* observer, const std::string& db_name )
{
    sqlite3* handle = OpenDatabase( observer, db_name );
    
    if( handle == nullptr )
        return;
    
    ExecuteQueryLight( observer, handle, "DROP TABLE IF EXISTS A" );
    ExecuteQueryLight( observer, handle, "DROP TABLE IF EXISTS B" );
                      
    ExecuteQueryLight( observer, handle, "CREATE TABLE A(id int NOT NULL, name varchar(255), PRIMARY KEY (id))" );
    ExecuteQueryLight( observer, handle, "CREATE TABLE B(id int NOT NULL, name varchar(255), PRIMARY KEY (id))" );
    
    sqlite3_close( handle );
}

void DBManager::ExecuteQuery( IObserver* observer, const string& db_name, const string& query )
{
    sqlite3* handle = OpenDatabase( observer, db_name );
    if( handle == nullptr )
    {
        return;
    }
    
    char *errmsg;
    int rc = sqlite3_exec( handle, query.data(), print_results, observer, &errmsg );
    if ( rc != SQLITE_OK )
    {
        stringstream ss;
        ss << "ERR " << errmsg << "\n";
        
        sqlite3_free( errmsg );
        observer->update( ss.str() );
    }
    else
    {
        observer->update( "OK\n" );
    }
}

void DBManager::ExecuteQueryLight( IObserver* observer, sqlite3* handle, const string& query )
{
    char *errmsg;
    int rc = sqlite3_exec( handle, query.data(), 0, 0, &errmsg );
    if ( rc != SQLITE_OK )
    {
        stringstream ss;
        ss << "ERR " << errmsg << "\n";
        
        sqlite3_free( errmsg );
        observer->update( ss.str() );
    }
}
