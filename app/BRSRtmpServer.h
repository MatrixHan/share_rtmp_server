#pragma once 

#include "../common/BRSCommon.h"
#include "../network/BRSServer.h"

namespace BRS 
{
  class BRSRtmpServer : public BRSServer
  {
  public:
    BRSRtmpServer(int port);
    void loop();
  };
  
}