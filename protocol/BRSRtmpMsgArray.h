#pragma once 

#include <BRSCommon.h>

namespace BRS 
{

class BrsSharePtrMessage;  
class BrsMessageArray
{
private:
  
  BrsSharePtrMessage ** msgs;
  int max;
  
public:
  BrsMessageArray(int argMax);
  virtual ~BrsMessageArray();
public:
  virtual void free(int count);
  
private:
  virtual void zero(int count);
};
  
}