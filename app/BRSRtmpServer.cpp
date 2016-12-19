#include <BRSRtmpServer.h>

namespace BRS 
{
  
BRSRtmpServer::BRSRtmpServer(int port):mport(port)
{
   
    
}

int BRSRtmpServer::initRtmpServer()
{
    return initServer(mport);
}


void BRSRtmpServer::loop()
{
   start();
}
  
}
