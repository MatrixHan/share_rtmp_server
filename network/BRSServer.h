#pragma once 

#include "../common/BRSCommon.h"
#include "BRSEpoll.h"
#include "BRSSocket.h"
#include "BRSStream.h"
#include "../app/BRSClientContext.h"
#include "../app/BRSClientWorker.h"
#include "BRSCoroutine.h"
namespace BRS 
{
  class BRSServer:public BRSCoroutine
  {
    
  private:
    struct BRSSocket server_socket;
    BRSEpoll		*server_epoll;
    BRSClientContextMaps   brsClientContextMaps;
    BCCMItor 		bccmitor;
    BRSWorker  *clientWorker;
    
  public:
    int initServer(int mport);
    
    void start();
    
    int closeClient(int fd);
  };
  
}