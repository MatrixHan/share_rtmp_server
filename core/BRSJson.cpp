#include <BRSJson.h>
using namespace std;
namespace BRS 
{
 BRSConfig *conf = NULL;
  
BRSConfig::BRSConfig()
{
    gopsize = port = maxconnect = 0;
}

BRSConfig::~BRSConfig()
{

}
int BRSConfig::initConfig()
{	
  conf = new BRSConfig();
  conf = BRSConfig::parse(CONFIG_DEFAULT_FILE_NAME);
  return 0;
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
      for(int i =0 ;i<size ;i++)
      {
	cf->gopsize = root[i]["gopsize"].asInt();
	cf->maxconnect = root[i]["maxconnect"].asInt();
	cf->port   = root[i]["port"].asInt();
	cf->movieDir = root[i]["movieDir"].asString();
	cf->vhost   = root[i]["vhost"].asString();
      }
      
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