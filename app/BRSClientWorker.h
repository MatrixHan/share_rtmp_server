#pragma once 

#include <BRSCommon.h>
#include <BRSLog.h>
#include <BRSClientContext.h>
#include <BRSCoroutine.h>
#include <BRSHandShake.h>
#include <BRSProtocol.h>

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
    virtual  int  connect_app();
    
  };
  
}