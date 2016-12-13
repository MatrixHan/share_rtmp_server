#pragma once
#include "../common/BRSCommon.h"
extern "C"{
#include "../objs/coroutine/include/coroutine.h"
}
namespace BRS 
{
  struct BRSClientContext
  {
    int client_socketfd;
    int coroutine_fd;
    struct schedule * menv;
  }__attribute__((packed));
  
}