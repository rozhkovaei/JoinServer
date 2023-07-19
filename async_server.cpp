#include <iostream>

#include <fstream>
#include <memory>
#include "async_server.h"

#include "waiting_queue.h"

using namespace std;

void query_producer( session* ss, const string& db_name, WaitingQueue< string >& queue )
{
    string query;
    
    while( queue.pop( query ) )
    {
        DBManager db_manager;
        db_manager.ExecuteQuery( ss, db_name, query );
        query.clear();
    }
}

class CommandProcessor
{
public:
    
    static void ProcessCommand( WaitingQueue< string >& queue, const string& input )
    {
        std::istringstream parse_input;
        parse_input.str( input );
        
        string command;
        parse_input >> command;
        
        if( command == "INTERSECTION" )
        {
            queue.push( "SELECT A.id, A.name as A, B.name as B FROM A,B where A.id = B.id ORDER BY 1" );
        }
        
        if( command == "SYMMETRIC_DIFFERENCE" )
        {
            queue.push( "SELECT id, name as A, NULL as B FROM A WHERE id NOT IN (SELECT id FROM B ) UNION SELECT id, NULL, name FROM B WHERE id NOT IN (SELECT id FROM A ) ORDER BY 1" );
        }
        
        if( command == "INSERT" )
        {
            string table_name, name;
            int id;
            parse_input >> table_name >> id >> name;
            
            std::stringstream query_ss;
            query_ss << "INSERT INTO " << table_name << "(id, name) VALUES (" << id << ", \"" << name << "\")";
            
            queue.push( query_ss.str() );
        }
        
        if( command == "TRUNCATE" )
        {
            string table_name;
            parse_input >> table_name;
            
            std::stringstream query_ss;
            query_ss << "DELETE FROM " << table_name;
            
            queue.push( query_ss.str() );
        }
    }
};

session::session( boost::asio::ip::tcp::socket socket )
  : socket_( std::move( socket ) )
{
    DBManager manager;
    
    stringstream ss;
    ss << std::chrono::system_clock::now().time_since_epoch().count() << "_db.sqlite";
  
    db_name_ = ss.str();
    manager.CreateDatabase( this, db_name_ );
    
    executer_ = std::thread( &query_producer, this, std::ref(db_name_), std::ref( queue_ ) );
}

void session::do_read()
{
auto self( shared_from_this() );
  socket_.async_read_some( boost::asio::buffer(data_, max_length ),
    [ this, self ]( boost::system::error_code ec, std::size_t length )
    {
      if ( !ec )
      {
          std::istringstream input;
          input.str( data_ );
                    
          for ( string line; getline( input, line ); )
          {
              CommandProcessor::ProcessCommand(queue_, line);
          }
          do_read();
      }
      else
      {
          std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
          
          queue_.stop();
          executer_.join();
      }
    });
}

void session::update(std::string&& message)
{
    do_write(std::move(message));
}

void session::do_write( std::string&& message )
{
    auto self( shared_from_this() );
    boost::asio::async_write( socket_, boost::asio::buffer( message.data(), message.length() ),
      [ this, self ]( boost::system::error_code ec, std::size_t /*length*/ )
      {
            if ( !ec )
            {
                // do_read();
            }
        }
    );
}

server::server( boost::asio::io_context& io_context, short port )
: acceptor_( io_context, boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), port ) )
, socket_( io_context )
{
    do_accept();
}

void server::do_accept()
{
    acceptor_.async_accept( socket_, [ this ]( boost::system::error_code ec )
    {
      if ( !ec )
      {
        std::make_shared< session >( std::move( socket_ ) )->start();
      }

      do_accept();
    } );
}
