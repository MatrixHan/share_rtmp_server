#include "BRSClientWorker.h"


namespace BRS 
{
BRSClientWorker::BRSClientWorker()
{

}

  
BRSClientWorker::BRSClientWorker(int socketfd, int coroutine_id)
{
    mContext.client_socketfd= socketfd;
    mContext.coroutine_fd = coroutine_id;
}

void BRSClientWorker::do_something()
{
      Log("do_something",this->mContext.coroutine_fd);
}
  
}