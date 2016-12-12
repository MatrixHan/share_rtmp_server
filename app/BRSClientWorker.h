#pragma once 

#include "../common/BRSCommon.h"
#include "BRSClientContext.h"
#include "../network/BRSCoroutine.h"

namespace BRS 
{
  typedef std::vector< BRSWorker> BRSClientContexts;
  typedef std::map<int , BRSWorker>	BRSClientContextMaps;
  class BRSClientWorker: public BRSWorker
  {
  public:
    virtual void do_something();
    
  public:
    BRSClientWorker();
    BRSClientWorker(int socketfd,int coroutine_id);
    struct BRSClientContext mContext;
  };
  
}