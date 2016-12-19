#pragma once 

#include <BRSCommon.h>
#include <BRSEpoll.h>
#include <BRSSocket.h>
#include <BRSStream.h>
#include <BRSClientWorker.h>
#include <BRSPublicEntity.h>
#include <BRSCoroutine.h>
namespace BRS 
{
  
  class BRSServer:public BRSCoroutine
  {
    
  private:
    BRSSocket server_socket;
    BCCMItor 		bccmitor;
    BRSWorker  *clientWorker;
    BRSClientContextMaps  * brsClientContextMaps;
    BRSEpoll		 * server_epoll;
  public:
    
    BRSServer();
    int initServer(int mport);
    
    void start();
    
    int closeClient(int fd);
  };
  
}