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
  
  
  
  struct BRSSocket
  {
    int sfd;//server clr fd
    int idlefd;//open system file fd
    struct sockaddr_in servaddr, peeraddr;
    
    
    BRSSocket();
    ~BRSSocket();
    int initSocket(int port);
    int optionSocket(int level,int optname);
    int bindSocket();
    int listenSocket(int maxListen);
    
    int acceptSocket(sockaddr* saddr,socklen_t *len);
    int accept4Socket(sockaddr* saddr,socklen_t *len,int flags);//return client fd

  };
  
  
}