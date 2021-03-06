#include <BRSServer.h>


namespace BRS 
{

BRSServer::BRSServer():clientWorker(NULL)
{
    brsClientContextMaps = new BRSClientContextMaps();
    server_epoll	 = new BRSEpoll();
}


int BRSServer::initServer(int mport)
{
   int result=0;
   if((result=server_socket.initSocket(mport))<0)
     return -1;
   if( (result=server_socket.optionSocket(SOL_SOCKET, SO_REUSEADDR))<0)
      return -1;
   if((result= server_socket.bindSocket())<0)
     return -1;
   if((result=server_socket.listenSocket(SOMAXCONN))<0)
     return -1;
   result = server_epoll->initEpoll(16);
   result = BRSCoroutine_open();
   result = server_epoll->addEpoll(server_socket.sfd,EPOLLIN | EPOLLET);
   return result;
}

void BRSServer::start()
{
    
   int connfd,coroutineid;
   int result=0;
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
	if (nready == server_epoll->client_size())
	    server_epoll->resize_client();
	for(int i = 0; i < nready; i++)
	{
	  if(server_epoll->isAccept(server_socket.sfd,i))
	  {
	      connfd = server_socket.accept4Socket();
	      clientWorker =new BRSClientWorker(connfd,this);
	      coroutineid  = BRSCoroutine_new(clientWorker);
	      clientWorker->mContext.client_socketfd = connfd;
	      clientWorker->mContext.coroutine_fd = coroutineid;
	      brsClientContextMaps->insert(std::make_pair(connfd,clientWorker));
	    if(connfd<0) continue;
	   server_epoll->addEpoll(connfd,EPOLLIN);
	  }else if(server_epoll->epoll_in(i))
	  {
	     connfd = server_epoll->getCfd(i);
	     bccmitor = brsClientContextMaps->find(connfd);
	     if(bccmitor==brsClientContextMaps->end())
		  continue; 
	      else
		clientWorker=bccmitor->second;
	      
	     //if 协程状态为真 则分配时间运行
	     if(BRSCoroutine_status( clientWorker->mContext.coroutine_fd))
	      {
		//重新分配协程时间
		BRSCoroutine_resume( clientWorker->mContext.coroutine_fd);
	      }
	     
	  }
	}
	
  }
  BRSCoroutine_close();
}


int BRSServer::addEpoll(int fd, int opt)
{
    return server_epoll->addEpoll(fd,opt);
}

int BRSServer::delEpoll(int fd)
{
   return server_epoll->delEpoll(fd);
}




BRSWorker* BRSServer::getWork(int clientFd)
{
    BRSWorker  *clientWorker;
    BCCMItor 		bccmitor;
    bccmitor = brsClientContextMaps->find(clientFd);
	     if(bccmitor!=brsClientContextMaps->end())
		clientWorker=bccmitor->second;
    if(clientWorker)
    {
      return clientWorker;
    }
    return NULL;
}


int BRSServer::closeClient(int fd)
{
    BRSWorker  *clientWorker;
    BCCMItor 		bccmitor;
    bccmitor = brsClientContextMaps->find(fd);
	     if(bccmitor!=brsClientContextMaps->end())
		clientWorker=bccmitor->second;
    if(clientWorker){
    SafeDelete(clientWorker);
    server_epoll->delEpoll(fd);
    brsClientContextMaps->erase(bccmitor);
    }
    return 0;
}

  
}