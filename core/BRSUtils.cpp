#include "BRSUtils.h"


namespace BRS 
{
long long int getTimeDec()
{
	register uint32_t lo, hi;
	register unsigned long long o;
    __asm__ __volatile__ (
        "rdtscp" : "=a"(lo), "=d"(hi)
        );
	o = hi;
	o <<= 32;
	return (o | lo);
}

BString timeFormat(long long int timedec)
{
      time_t tTime = time(NULL) ;   
      struct tm *tmTime;   //定义tm类型指针
      tmTime = localtime(&tTime);  //获取时间
      
      char tmpBuf[BUFLEN];   
      strftime(tmpBuf, BUFLEN, "%F %T", tmTime); //format date and time. 
      return BString(tmpBuf);
      
}



BString now()
{
    return timeFormat(getTimeDec());
}
 
long long getCurrentTime()
{
   time_t tTime = time(NULL) ; 
   return tTime;
}

}