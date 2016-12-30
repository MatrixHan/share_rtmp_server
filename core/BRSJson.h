#pragma once

#include <BRSCommon.h>
#include <fstream>
#include <json/json.h>

namespace BRS 
{
class BRSConfig;
  
extern BRSConfig * conf;
int initConfig();
struct BRSConfig
{
  int 			gopsize;
  int 			port;
  std::string 		movieDir;
  std::string 		vhost;
  int 			maxconnect;
  
  BRSConfig();
  ~BRSConfig();
  
  static BRSConfig* parse(std::string confName);
  static int 	    writeConf(BRSConfig* config);
};

  
}