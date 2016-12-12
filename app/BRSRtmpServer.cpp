#include "BRSRtmpServer.h"

namespace BRS 
{
  
BRSRtmpServer::BRSRtmpServer(int port)
{
   
    initServer(port);
}

void BRSRtmpServer::loop()
{
   start();
}
  
}