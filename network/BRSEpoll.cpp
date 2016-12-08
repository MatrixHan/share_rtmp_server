#include "BRSEpoll.h"


namespace BRS 
{
  
BRSEpoll::BRSEpoll(BRSSocket psocket)
{
      brsSocket = psocket;
}

  
int BRSEpoll::initEpoll()
{
    epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    sEvent.data.fd = brsSocket.sfd;
    sEvent.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD,brsSocket.sfd,  &sEvent);
    clients.resize(16);
}

int BRSEpoll::initEpoll(int clients_size)
{
    epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    sEvent.data.fd = brsSocket.sfd;
    sEvent.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD,brsSocket.sfd,  &sEvent);
    clients.resize(clients_size);
}

bool BRSEpoll::isAccept(int i)
{
    return clients[i].data.fd==brsSocket.sfd;
}

bool BRSEpoll::epoll_in(int i)
{
    return clients[i].events&EPOLLIN;
}



int BRSEpoll::waitEpoll()
{
   return epoll_wait(epoll_fd,&*clients.begin(), static_cast<int>(clients.size()), -1);
}

  
}