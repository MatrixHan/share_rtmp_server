#pragma once

#include <BRSCommon.h>

namespace BRS 
{
    template<class T>
    const T& Min(const T& a,const T& b)
    {
      
      return (a	< b) ? a : b;
    }
    
    template<class T>
    const T& Max(const T & a,const T& b)
    {
      return (a < b) ? b : a;
    }
  
    template<class T>
    const T& Clamp(const T& val,const T& minV,const T& maxV)
    {
      return Min(Max(val,maxV),minV);
    }
  
}