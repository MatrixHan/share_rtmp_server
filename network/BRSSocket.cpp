#include "BRSSocket.h"

namespace BRS 
{
  
BRSSocket::BRSSocket()
{
      signal(SIGPIPE ,SIG_IGN);
      signal(SIGCHLD ,SIG_IGN);
      idlefd = open("dev/null" ,O_RDONLY | O_CLOEXEC);//处理EMFILE 文件描述符满了 没法触发高电平问题
}

BRSSocket::~BRSSocket()
{
  close(sfd);
  close(idlefd);
}

int BRSSocket::initSocket(int port)
{
      
  if ((sfd=socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)) < 0)
		return -1;
      memset(&servaddr, 0, sizeof(servaddr));
      servaddr.sin_family = AF_INET;
      servaddr.sin_port = htons(port);
      servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  return 0;
}

int BRSSocket::optionSocket(int level, int optname)
{
    int on=1;
    return setsockopt(sfd, level, optname, &on, sizeof(on));
}


int BRSSocket::bindSocket()
{
    return bind(sfd , (struct sockaddr*)&servaddr, sizeof(struct sockaddr));
}

int BRSSocket::listenSocket(int maxListen)
{
    return listen(sfd ,maxListen);
}

int BRSSocket::acceptSocket()
{	
   unsigned int len = sizeof(peeraddr);
   int connfd = accept(sfd, (struct sockaddr*)&peeraddr, &len);
   return connfd;
}

int BRSSocket::accept4Socket()
{
      unsigned int len = sizeof(peeraddr);
      int connfd = accept4(sfd, (struct sockaddr*)&peeraddr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
       if(connfd == -1)
	    {
	       if(errno == EMFILE)
	       {
		 close(idlefd);
		 idlefd = accept(sfd, NULL, NULL);
		 close(idlefd);
		 idlefd = open("dev/null" ,O_RDONLY | O_CLOEXEC);
		 return -2;
	      }else
		return -1;
	    }
      return connfd;
}
  
  
}
