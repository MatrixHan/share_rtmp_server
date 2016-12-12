#include "../common/BRSCommon.h"
#include "../core/BRSLog.h"
#include "../app/BRSRtmpServer.h"
using namespace BRS;

int main(void)
{
  InitLog("BRS.log");
  
  Log("project init ");
  BRSRtmpServer server(5888);
  server.loop();
  CloseLog();
  return 0;
}