#pragma once 

#include "../common/BRSCommon.h"
#include "../app/BRSClientContext.h"
extern "C"{
#include "../objs/coroutine/include/coroutine.h"
}


namespace BRS {


class BRSWorker
{
public:
    struct BRSClientContext mContext;
    virtual void do_something()=0;
    virtual ~BRSWorker(){}
};

class BRSCoroutine
{
 
public:
  BRSCoroutine(); 
  
  static void fun(struct schedule *, void *ud);
  int BRSCoroutine_open();
  void BRSCoroutine_close();
  int BRSCoroutine_new(BRSWorker* worker);
  void BRSCoroutine_resume( int id);
  int BRSCoroutine_status( int id);
  int BRSCoroutine_running();
  void BRSCoroutine_yield();
 
  
public:
  struct schedule * env;
};

}
