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
    if(setsockopt(sfd, level, optname, &on, sizeof(on))<0)
      return -1;
    return 0;
}


int BRSSocket::bindSocket()
{
    if (bind(sfd , (struct sockaddr*)&servaddr, sizeof(struct sockaddr)) < 0)
	    return -1;
    return 0;
}

int BRSSocket::listenSocket(int maxListen)
{
    if (listen(sfd ,maxListen) < 0)
	    return -1;
    return 0;
}

int BRSSocket::acceptSocket(sockaddr* saddr, socklen_t* len)
{	

   int connfd = accept(sfd, saddr, len);
   return connfd;
}

int BRSSocket::accept4Socket(sockaddr* saddr, socklen_t* len, int flags)
{

      int connfd = accept(sfd, saddr, len,flags);
      return connfd;
}
  
  
}