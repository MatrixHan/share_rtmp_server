#include "BRSServer.h"


namespace BRS 
{


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
   env= BRSCoroutine_open();
   return 0;
}

void BRSServer::start()
{
   int connfd,coroutineid;
   int result=0;
   BRSClientWorker  *clientWorker;
   struct BRSClientContext bcc;
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
	      clientWorker = new BRSClientWorker();
	      connfd = server_socket.accept4Socket();
	      coroutineid=BRSCoroutine_new(env,(BRSWorker*)clientWorker);
	      clientWorker->mContext.client_socketfd = connfd;
	      clientWorker->mContext.coroutine_fd = coroutineid;
	      brsClientContextMaps[connfd] = *clientWorker;
	    if(connfd<0) continue;
	    result = server_epoll->addEpoll(connfd);
	  }else if(server_epoll->epoll_in())
	  {
	     connfd = server_epoll->getCfd(i);
	     *clientWorker = brsClientContextMaps[connfd];
	     if(BRSCoroutine_status(bcc.coroutine_fd))//if 协程状态为真 则分配时间运行
	      {
		BRSCoroutine_resume(bcc.coroutine_fd);//重新分配协程时间
	      }
	  }
	}
  }
  BRSCoroutine_close(env);
}

  
}