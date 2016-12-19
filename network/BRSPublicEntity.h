#pragma once 
#include <BRSCoroutine.h>
#include <BRSEpoll.h>

namespace BRS 
{
  typedef std::vector< BRSWorker*> BRSClientContexts;
  typedef std::map<int ,  BRSWorker*>	BRSClientContextMaps;
  typedef std::map<int ,  BRSWorker*>::iterator	BCCMItor;
}