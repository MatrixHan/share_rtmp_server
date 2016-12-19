#pragma once 

#include <BRSCommon.h>
#include <BRSServer.h>
#include <BRSLog.h>
#include <BRSUtils.h>
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