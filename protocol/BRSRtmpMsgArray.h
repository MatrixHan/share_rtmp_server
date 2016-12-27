#pragma once 

#include <BRSCommon.h>
#include <BRSFlv.h>
namespace BRS 
{

class BrsMessageArray
{
public:
  
  BrsSharedPtrMessage ** msgs;
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