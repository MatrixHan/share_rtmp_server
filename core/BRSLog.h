#pragma once 

#include <BRSCommon.h>
#include <BRSUtils.h>




namespace BRS
{
  extern std::ofstream *GLogStream;
  
  bool InitLog(const BString &filename);
  
  void Log(const BChar * string , ...);
  
  void LogBS(const BString str);
  
  void CloseLog();
  
  // the context for multiple clients.
class ILogContext
{
public:
    ILogContext();
    virtual ~ILogContext();
public:
    virtual void generate_id() = 0;
    virtual int get_id() = 0;
public:
    virtual const char* format_time() = 0;
};

// user must implements the LogContext and define a global instance.
extern ILogContext* log_context;

// donot print method
#if 0
    #define brs_verbose(msg, ...) printf("[%s][%d][verbs] ", log_context->format_time(), log_context->get_id());printf(msg, ##__VA_ARGS__);printf("\n")
    #define brs_info(msg, ...)    printf("[%s][%d][infos] ", log_context->format_time(), log_context->get_id());printf(msg, ##__VA_ARGS__);printf("\n")
    #define brs_trace(msg, ...)   printf("[%s][%d][trace] ", log_context->format_time(), log_context->get_id());printf(msg, ##__VA_ARGS__);printf("\n")
    #define brs_warn(msg, ...)    printf("[%s][%d][warns] ", log_context->format_time(), log_context->get_id());printf(msg, ##__VA_ARGS__);printf(" errno=%d(%s)", errno, strerror(errno));printf("\n")
    #define brs_error(msg, ...)   printf("[%s][%d][error] ", log_context->format_time(), log_context->get_id());printf(msg, ##__VA_ARGS__);printf(" errno=%d(%s)", errno, strerror(errno));printf("\n")
// use __FUNCTION__ to print c method
#elif 1
    #define brs_verbose(msg, ...) printf("[%s][%d][verbs][%s] ", log_context->format_time(), log_context->get_id(), __FUNCTION__);printf(msg, ##__VA_ARGS__);printf("\n")
   // #define brs_info(msg, ...)    printf("[%s][%d][infos][%s] ", log_context->format_time(), log_context->get_id(), __FUNCTION__);printf(msg, ##__VA_ARGS__);printf("\n")
     #define brs_info(msg, ...)
    #define brs_trace(msg, ...)   printf("[%s][%d][trace][%s] ", log_context->format_time(), log_context->get_id(), __FUNCTION__);printf(msg, ##__VA_ARGS__);printf("\n")
    #define brs_warn(msg, ...)    printf("[%s][%d][warns][%s] ", log_context->format_time(), log_context->get_id(), __FUNCTION__);printf(msg, ##__VA_ARGS__);printf(" errno=%d(%s)", errno, strerror(errno));printf("\n")
    #define brs_error(msg, ...)   printf("[%s][%d][error][%s] ", log_context->format_time(), log_context->get_id(), __FUNCTION__);printf(msg, ##__VA_ARGS__);printf(" errno=%d(%s)", errno, strerror(errno));printf("\n")
// use __PRETTY_FUNCTION__ to print c++ class:method
#else
    #define brs_verbose(msg, ...) printf("[%s][%d][verbs][%s] ", log_context->format_time(), log_context->get_id(), __PRETTY_FUNCTION__);printf(msg, ##__VA_ARGS__);printf("\n")
    #define brs_info(msg, ...)    printf("[%s][%d][infos][%s] ", log_context->format_time(), log_context->get_id(), __PRETTY_FUNCTION__);printf(msg, ##__VA_ARGS__);printf("\n")
    #define brs_trace(msg, ...)   printf("[%s][%d][trace][%s] ", log_context->format_time(), log_context->get_id(), __PRETTY_FUNCTION__);printf(msg, ##__VA_ARGS__);printf("\n")
    #define brs_warn(msg, ...)    printf("[%s][%d][warns][%s] ", log_context->format_time(), log_context->get_id(), __PRETTY_FUNCTION__);printf(msg, ##__VA_ARGS__);printf(" errno=%d(%s)", errno, strerror(errno));printf("\n")
    #define brs_error(msg, ...)   printf("[%s][%d][error][%s] ", log_context->format_time(), log_context->get_id(), __PRETTY_FUNCTION__);printf(msg, ##__VA_ARGS__);printf(" errno=%d(%s)", errno, strerror(errno));printf("\n")
#endif
  
}
