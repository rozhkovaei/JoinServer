#pragma once

#include <list>
#include <thread>
#include <boost/asio.hpp>

#include "db_manager.h"
#include "waiting_queue.h"
#include "IObserver.h"

class session;

class session
  : public std::enable_shared_from_this< session >, public IObserver
{
public:
    session( boost::asio::ip::tcp::socket socket );
    
    void start() { do_read(); }

    virtual void update( std::string&& message ) override;
    
private:
    
    void do_read();

    void do_write( std::string&& message );
    
    boost::asio::ip::tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[ max_length ];
    
    WaitingQueue< std::string > queue_;
    std::thread executer_;
    std::string db_name_;
};

class server
{
public:
    server( boost::asio::io_context& io_context, short port );

    ~server() {}
    
private:
    
  void do_accept();
  
  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::ip::tcp::socket socket_;
};
