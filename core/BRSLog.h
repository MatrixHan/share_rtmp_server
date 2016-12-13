#pragma once 

#include "../common/BRSCommon.h"
#include "BRSUtils.h"


namespace BRS
{
  extern std::ofstream *GLogStream;
  
  bool InitLog(const BString &filename);
  
  void Log(const BChar * string , ...);
  
  void LogBS(const BString str);
  
  void CloseLog();
  
}
