#include <BRSJson.h>
using namespace std;
namespace BRS 
{
 BRSConfig *conf = NULL;
  
 int initConfig()
{	
  conf = new BRSConfig();
  conf = BRSConfig::parse(CONFIG_DEFAULT_FILE_NAME);
  return 0;
}

 
BRSConfig::BRSConfig()
{
    gopsize = port = maxconnect = 0;
}

BRSConfig::~BRSConfig()
{

}


BRSConfig* BRSConfig::parse(std::string confName)
{
      ifstream ifs;
      ifs.open(confName.c_str());
      assert(ifs.is_open());
      Json::Reader reader;
      Json::Value  root;
      if(!reader.parse(ifs,root,false))
      {
	return NULL;
      }
      int size = root.size();
      BRSConfig * cf = new BRSConfig();
      
	cf->gopsize = root["gopsize"].asInt();
	cf->maxconnect = root["maxconnect"].asInt();
	cf->port   = root["port"].asInt();
	cf->movieDir = root["movieDir"].asString();
	cf->vhost   = root["vhost"].asString();
     
      
      return cf;
}

int BRSConfig::writeConf(BRSConfig* config)
{
    Json::Value root;
    Json::FastWriter writer;
    Json::Value person;
    std::string json_file = writer.write(root);
    
    person["gopsize"] = config->gopsize;
    person["maxconnect"] = config->maxconnect;
    person["port"] = config->port;
    person["movieDIr"] = config->movieDir;
    person["vhost"] = config->vhost;
 
    ofstream ofs;
    ofs.open(CONFIG_DEFAULT_FILE_NAME);
    assert(ofs.is_open());
    ofs<<json_file;
 
    return 0;
    
}

  
}