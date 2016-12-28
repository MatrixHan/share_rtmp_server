#pragma once 

#include <BRSCommon.h>
#include <BRSEpoll.h>
#include <BRSSocket.h>
#include <BRSStream.h>
#include <BRSClientWorker.h>
#include <BRSPublicEntity.h>
#include <BRSCoroutine.h>
#include <BRSDataSource.h>
namespace BRS 
{

  class BRSServer:public BRSCoroutine
  {
    
  private:
    BRSSocket 			server_socket;
    BCCMItor 			bccmitor;
    BRSWorker  			*clientWorker;
    BRSClientContextMaps  	*brsClientContextMaps;
    BRSEpoll		     	*server_epoll;
  public:
    
    BRSServer();
    int initServer(int mport);
    
    void start();
    
    
    int addEpoll(int fd,int opt);
    
    int delEpoll(int fd);
    
    
    int closeClient(int fd);
    
    BRSWorker* getWork(int clientFd);
  };
  
}