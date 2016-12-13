#pragma once

#include "../common/BRSCommon.h"


#include <arpa/inet.h>
#include <netinet/in.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <sys/socket.h>


namespace BRS {
  
  
  
  class BRSSocket
  {
  public:
    int sfd;//server clr fd
    int idlefd;//open system file fd
    struct sockaddr_in servaddr, peeraddr;
    
  public:
    BRSSocket();
    ~BRSSocket();
    int initSocket(int port);
    int optionSocket(int level,int optname);
    int bindSocket();
    int listenSocket(int maxListen);
    
    int acceptSocket();
    int accept4Socket();//return client fd

  };
  
  
}