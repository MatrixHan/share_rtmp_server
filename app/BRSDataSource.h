#pragma once 

#include "../common/BRSCommon.h"

namespace BRS 
{
  typedef std::vector<BRSPacket>  BRSPackets;
  typedef std::map<int,BRSGop*>   BRSGops;
  
  
  struct BRSPacket
  {
      int index;
      int type;
      int dataSize;
      unsigned char *data;
  };
  
  struct BRSGop
  {
    int gopMaxSize;
    int offset;
    BRSPackets pks;
  };
}