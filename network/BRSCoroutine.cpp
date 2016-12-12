#include "BRSCoroutine.h"


namespace BRS {
  
  
void BRSCoroutine::fun(schedule* S, void* ud)
{
    BRSWorker * brsw = static_cast<BRSWorker*>(ud);
    brsw->do_something();
}

  
BRSCoroutine* BRSCoroutine::BRSCoroutine_open(void)
{
  return env = coroutine_open();
}

int BRSCoroutine::BRSCoroutine_new(BRSCoroutine* benv,BRSWorker* worker)
{
   return coroutine_new(env,fun,worker);
}

void BRSCoroutine::BRSCoroutine_resume(BRSCoroutine* benv, int id)
{
   coroutine_resume(env,id);
}

void BRSCoroutine::BRSCoroutine_close(BRSCoroutine* benv)
{
   coroutine_close(env);
}

int BRSCoroutine::BRSCoroutine_running(BRSCoroutine * benv)
{
  return coroutine_running(env);
}

int BRSCoroutine::BRSCoroutine_status(BRSCoroutine* benv, int id)
{
  return coroutine_status(env,id);
}

void BRSCoroutine::BRSCoroutine_yield(BRSCoroutine* benv)
{
    coroutine_yield(env);
}

  
  
}