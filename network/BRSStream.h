#pragma once

#include "../common/BRSCommon.h"


namespace BRS 
{
  
  class BRSInputStream
  {
    
      virtual void Reader(int fd,int size)=0;
  };
  
  class BRSOutputStream
  {
      virtual void Writer(int fd,void *arg,int size)=0;
  };
  
  
}