#pragma once 

#include <BRSCommon.h>
#include <BRSClientContext.h>
extern "C"{
#include <coroutine.h>
}


namespace BRS {


class BRSWorker
{
public:
    struct BRSClientContext mContext;
    virtual void do_something()=0;
    virtual ~BRSWorker(){}
protected:
  friend class BRSServer;
  BRSWorker(BRSServer * mserver);
protected:
  BRSServer *brsServer;
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
  static void BRSCoroutine_yield(struct schedule *);
 
 
public:
  struct schedule * env;
};

}
