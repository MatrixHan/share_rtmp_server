#include "BRSProtocol.h"

namespace BRS 
{
  
BRSProtocol::BRSProtocol(int clientfd)
{
    skt = new BRSReadWriter(clientfd);
}

  
  
BRSProtocol::~BRSProtocol()
{
  
}


}