#pragma once 

#include "../common/BRSCommon.h"
#include "../core/BRSLog.h"
#include "BRSClientContext.h"
#include "../network/BRSCoroutine.h"

namespace BRS 
{
  typedef std::vector< BRSWorker*> BRSClientContexts;
  typedef std::map<int ,  BRSWorker*>	BRSClientContextMaps;
  typedef std::map<int ,  BRSWorker*>::iterator	BCCMItor;
  class BRSClientWorker: public BRSWorker
  {
  public:
    void do_something();
    
  public:
    BRSClientWorker();
    BRSClientWorker(int socketfd,int coroutine_id);
  };
  
}