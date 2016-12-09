#include "BRSServer.h"


namespace BRS 
{
  
BRSServer::BRSServer():server_epoll(NULL)
{

}

int BRSServer::initServer()
{
   int result;
   if(server_socket.initSocket(5888)<0)
     return -1;
   if( server_socket.optionSocket(SOL_SOCKET, SO_REUSEADDR)<0)
      return -1;
   if( server_socket.bindSocket()<0)
     return -1;
   if(server_socket.listenSocket(SOMAXCONN)<0)
     return -1;
   server_epoll = new BRSEpoll(server_socket);
   server_epoll->initEpoll(16);
   return 0;
}

void BRSServer::start()
{
  while(1){
    int nready=server_epoll->waitEpoll();
    if (nready == -1)
	{
	    if (errno == EINTR)
		continue;
	    ERR_EXIT("epoll_wait");
	}
	if (nready == 0)
	    continue;
	if ((size_t)nready == server_epoll->client_size())
	    server_epoll->resize_client();
	for(int i = 0; i < nready; i++)
	{
	  if(server_epoll->isAccept(i))
	  {
	    
	  }
	}
  }
}

  
}