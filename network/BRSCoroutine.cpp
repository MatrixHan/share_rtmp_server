#include "BRSCoroutine.h"


namespace BRS {
  
 // struct schedule * env = NULL;
  
BRSCoroutine::BRSCoroutine()
{

}

  
void BRSCoroutine::fun(schedule* S, void* ud)
{
    BRSWorker * brsw = static_cast<BRSWorker*>(ud);
    brsw->do_something();
}

  
int  BRSCoroutine::BRSCoroutine_open()
{
  env = coroutine_open();
  if(env)
    return 0;
  return -1;
}

int BRSCoroutine::BRSCoroutine_new(BRSWorker* worker)
{
   return coroutine_new(env,fun,worker);
}

void BRSCoroutine::BRSCoroutine_resume(int id)
{
   coroutine_resume(env,id);
}

void BRSCoroutine::BRSCoroutine_close()
{
   coroutine_close(env);
}

int BRSCoroutine::BRSCoroutine_running()
{
  return coroutine_running(env);
}

int BRSCoroutine::BRSCoroutine_status( int id)
{
  return coroutine_status(env,id);
}

void BRSCoroutine::BRSCoroutine_yield()
{
    coroutine_yield(env);
}

  
  
}