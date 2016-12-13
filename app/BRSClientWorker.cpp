#include "BRSClientWorker.h"


namespace BRS 
{


BRSClientWorker::BRSClientWorker(int pfd)
{
      protocol = new BRSProtocol(pfd);
}


  
BRSClientWorker::~BRSClientWorker()
{

}



void BRSClientWorker::do_something()
{
      
     
     int result;
     char buf[1024];
     while((result = read(this->mContext.client_socketfd,buf,1024))>0)
     {
       LogBS("do_something"+IntToString(this->mContext.coroutine_fd)); 
       coroutine_yield(this->mContext.menv);
    }
}
  
}