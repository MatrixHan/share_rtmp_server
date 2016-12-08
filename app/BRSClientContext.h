#pragma once
#include "../common/BRSCommon.h"

namespace BRS 
{
  struct BRSClientContext
  {
    int client_socketfd;
    int coroutine_fd;
    
  }__attribute__((packed));
  
}