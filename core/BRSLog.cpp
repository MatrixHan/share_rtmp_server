#include "BRSLog.h"
#include <sys/time.h>
#include <coroutine.h>

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

void LogBS(const BString str)
{
  Log(str.c_str());
}


void CloseLog()
{
      GLogStream->close();
      SafeDelete(GLogStream);
}

ILogContext::ILogContext()
{
}

ILogContext::~ILogContext()
{
}



class LogContext : public ILogContext
{
private:
	class DateTime
	{
	private:
	    // %d-%02d-%02d %02d:%02d:%02d.%03d
	    #define DATE_LEN 24
	    char time_data[DATE_LEN];
	public:
	    DateTime();
	    virtual ~DateTime();
	public:
	    virtual const char* format_time();
	};
private:
    DateTime time;
    //std::map<int, int> cache;
public:
    LogContext();
    virtual ~LogContext();
public:
    virtual void generate_id();
    virtual int get_id();
public:
    virtual const char* format_time();
};

ILogContext* log_context = new LogContext();

LogContext::DateTime::DateTime()
{
    memset(time_data, 0, DATE_LEN);
}

LogContext::DateTime::~DateTime()
{
}

const char* LogContext::DateTime::format_time()
{
    // clock time
    timeval tv;
    if (gettimeofday(&tv, NULL) == -1) {
        return "";
    }
    // to calendar time
    struct tm* tm;
    if ((tm = localtime(&tv.tv_sec)) == NULL) {
        return "";
    }
    
    // log header, the time/pid/level of log
    // reserved 1bytes for the new line.
    snprintf(time_data, DATE_LEN, "%d-%02d-%02d %02d:%02d:%02d.%03d", 
        1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, 
        (int)(tv.tv_usec / 1000));
        
    return time_data;
}

LogContext::LogContext()
{
}

LogContext::~LogContext()
{
}

void LogContext::generate_id()
{
	
}

int LogContext::get_id()
{
    return 0;
}

const char* LogContext::format_time()
{
    return time.format_time();
}

}