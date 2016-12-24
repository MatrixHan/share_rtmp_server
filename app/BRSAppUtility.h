#pragma once 

#include <BRSCommon.h>

namespace BRS 
{
  
extern std::string brs_get_local_ip(int fd);

extern std::string brs_get_peer_ip(int fd);
}