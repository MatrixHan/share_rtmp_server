#pragma once
#include <BRSCommon.h>


namespace BRS 
{
  
  struct BRSUser
  {
    BChar		keys[64];
    BChar		url[64];
    BInt		ip;
    BInt		port; 
  }__attribute__((packed));
  
}