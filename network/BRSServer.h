#pragma once 

#include "../common/BRSCommon.h"
#include "BRSEpoll.h"
#include "BRSSocket.h"
#include "BRSStream.h"
namespace BRS 
{
  class BRSServer:public BRSInputStream,BRSOutputStream
  {
    typedef std::vector<struct BRSClientContext> brsClientContexts;
  private:
    struct BRSSocket server_socket;
    BRSEpoll		*server_epoll;
  public:
    BRSServer();
    int initServer();
    
    void start();
  };
  
}