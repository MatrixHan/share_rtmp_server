#include "BRSLog.h"


namespace BRS 
{
    std::ofstream *GLogStream = NULL;
    
bool InitLog(const BString& filename)
{
      if (GLogStream == NULL)
      {
	GLogStream = new std::ofstream();
	GLogStream->open(filename.c_str());
	if(GLogStream->good())
	{
	  return true;
	}
	return false;
      }
      return true;
}

void Log(const BChar* string, ...)
{
    if(GLogStream == NULL)
       return ;
    BChar buffer[256];
    if (!string || !GLogStream)
      return ;
    
    va_list arglist;
    va_start(arglist,string);
    vsprintf(buffer,string,arglist);
    va_end(arglist);
    
    BString info(buffer);
    *GLogStream <<now()<<":"<< info << std::endl;
    GLogStream->flush();
}

void CloseLog()
{
      GLogStream->close();
      SafeDelete(GLogStream);
}

}