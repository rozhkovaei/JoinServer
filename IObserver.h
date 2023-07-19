#pragma once

struct IObserver
{
    virtual ~IObserver() {};
    
    virtual void update( std::string&& message ) = 0;
};

