#pragma once 

#include "../common/BRSCommon.h"
#include "../core/BRSLog.h"
#include "BRSClientContext.h"
#include "../network/BRSCoroutine.h"
#include "../core/BRSHandShake.h"
#include "../protocol/BRSProtocol.h"

namespace BRS 
{
  class BRSServer;
  class BRSClientWorker:virtual public BRSWorker 
  {
  private:
   BRSProtocol  *protocol;
   BRSComplexHandShake * complexHandshake;
  public:
    BRSClientWorker(int pfd,BRSServer *mserver);
    virtual ~BRSClientWorker();
    virtual  void do_something();
    virtual  int  rtmpHandshake();
    
  };
  
}