#pragma once 

#include "../common/BRSCommon.h"
#include "../core/BRSLog.h"
#include "BRSClientContext.h"
#include "../network/BRSCoroutine.h"
#include "../protocol/BRSProtocol.h"

namespace BRS 
{
  typedef std::vector< BRSWorker*> BRSClientContexts;
  typedef std::map<int ,  BRSWorker*>	BRSClientContextMaps;
  typedef std::map<int ,  BRSWorker*>::iterator	BCCMItor;
  class BRSWorker;
  class BRSReaderWriterStream;
  class BRSClientWorker:virtual public BRSWorker 
  {
  private:
   BRSProtocol  *protocol;
  public:
    BRSClientWorker(int pfd);
    virtual ~BRSClientWorker();
    virtual  void do_something();
    
  };
  
}