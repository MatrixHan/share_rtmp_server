#pragma once 

#include <BRSCommon.h>
#include <BRSReadWriter.h>
#include <BRSAutoFree.h>

#define BRS_S0   0x03;
#define BRS_S0_ssl   0x06;

namespace BRS 
{
  class  BRSComplexHandShake
  {
  public:
    BRSComplexHandShake();
    ~BRSComplexHandShake();
  public:
    virtual int handshake(BRSReadWriter *srw,char *c1);
  };
  

  class BRSSimpleHandShake
  {
  public:
    BRSSimpleHandShake();
    ~BRSSimpleHandShake();
  public:
    virtual int handshake(BRSReadWriter  *srw,char *c1);
    
  };
  
}