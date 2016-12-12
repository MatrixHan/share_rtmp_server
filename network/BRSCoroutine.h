#pragma once 

#include "../common/BRSCommon.h"

extern "C"{
#include "../objs/coroutine/include/coroutine.h"
}

namespace BRS {

class BRSWorker
{
public:
    virtual void do_something()=0;
};

class BRSCoroutine
{
private:
  struct schedule 	*env;
public:
   
   static void fun(struct schedule *, void *ud);
   
  BRSCoroutine * BRSCoroutine_open();
  void BRSCoroutine_close(BRSCoroutine *);

  int BRSCoroutine_new(BRSCoroutine *,BRSWorker* worker);
  void BRSCoroutine_resume(BRSCoroutine *, int id);
  int BRSCoroutine_status(BRSCoroutine *, int id);
  int BRSCoroutine_running(BRSCoroutine *);
  void BRSCoroutine_yield(BRSCoroutine *);
};

}