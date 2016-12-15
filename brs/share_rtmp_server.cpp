#include "../common/BRSCommon.h"
#include "../app/BRSRtmpServer.h"
using namespace BRS;
/**
 * share_rtmp_server.cpp
 */
int main(int argc,char *argv[])
{
  InitLog("BRS.log");
  
  Log("project init ");
  BRSRtmpServer server(1935);
  Log("server init ");
  if(server.initRtmpServer()==0)
  server.loop();
  CloseLog();
  return 0;
}