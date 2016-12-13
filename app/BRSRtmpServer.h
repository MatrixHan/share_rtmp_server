#pragma once 

#include "../common/BRSCommon.h"
#include "../network/BRSServer.h"
#include "../core/BRSLog.h"
#include "../core/BRSUtils.h"
namespace BRS 
{
  class BRSRtmpServer : public BRSServer
  {
  public:
    BRSRtmpServer(int port);
    int initRtmpServer();
    void loop();
  private:
    BInt mport;
  };
  
}