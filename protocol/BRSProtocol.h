#pragma once 

#include "../common/BRSCommon.h"
#include "../network/BRSStream.h"
#include "../network/BRSReadWriter.h"


namespace BRS 
{
  class BRSProtocol:virtual public BRSStream,virtual public BRSBitStream
  {
  private:
    BRSReadWriter  *skt;
    
  public:
      BRSProtocol(int clientfd);
      virtual ~BRSProtocol();
  };
  
  
}