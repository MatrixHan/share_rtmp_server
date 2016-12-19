#include <BRSEpoll.h>


namespace BRS 
{
  
BRSEpoll::BRSEpoll():epoll_fd(0)
{
     epoll_fd = epoll_create1(EPOLL_CLOEXEC);
}

  
int BRSEpoll::initEpoll()
{
    clients.resize(16);
    return 0;
}

int BRSEpoll::initEpoll(int clients_size)
{
    clients.resize(clients_size);
    return 0;
}

bool BRSEpoll::isAccept(int sfd,int i)
{
    return clients[i].data.fd==sfd;
}

bool BRSEpoll::epoll_in(int i)
{
    return clients[i].events&EPOLLIN;
}



int BRSEpoll::waitEpoll()
{
   return epoll_wait(epoll_fd,&*clients.begin(), static_cast<int>(clients.size()), -1);
}

int BRSEpoll::addEpoll(int cfd,int op)
{
	    struct epoll_event sEvent;
	    sEvent.data.fd = cfd;
	    sEvent.events = op;
	return  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cfd, &sEvent);
}


int BRSEpoll::delEpoll(int cfd)
{
	    struct epoll_event sEvent;
	    sEvent.data.fd = cfd;
	    sEvent.events = EPOLLIN;
	 return    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cfd, &sEvent);
}

int BRSEpoll::getCfd(int index)
{
  return clients[index].data.fd;
}


  
}