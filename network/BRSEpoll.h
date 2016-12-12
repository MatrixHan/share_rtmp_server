#pragma once

#include "../common/BRSCommon.h"
#include "BRSSocket.h"

namespace BRS 
{
  typedef std::vector<struct epoll_event> EventList;
  
  class  BRSEpoll{
  private:
    int epoll_fd;
    struct epoll_event sEvent;
    struct BRSSocket brsSocket;
    EventList clients;
  public:
   
    
    BRSEpoll(struct BRSSocket psocket);
    
    int initEpoll();
    
    int initEpoll(int clients_size);
    
    int waitEpoll();
    
    int addEpoll(int cfd);
    
    int delEpoll(int cfd);
    
    int getCfd(int index);
    
    bool isAccept(int i);
    
    bool epoll_in(int i);
    
    inline int client_size(){return clients.size();}
    
    inline void resize_client(){clients.resize(client_size()*2);}
  };
  
  
  
  
}

