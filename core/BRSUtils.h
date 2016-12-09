#pragma once

#include "../common/BRSCommon.h"
#define BUFLEN 255   

namespace BRS 
{
 
  static long long getTimeDec();
 
  
  inline BString FloatToString(BFloat num)
  {
    std::stringstream ss;
    ss << num;
    BString re;
    ss >> re;
    return re;
  }
  
  inline BString IntToString(BInt num)
  {
    std::stringstream ss;
    ss << num;
    BString re;
    ss >> re;
    return re;
  }
  
  inline BInt RandomInt(BInt from = 0, BInt to = 10)
  {
      BInt ran = rand() % (to - from +1) + from;
      return ran;
  }
  
  inline BInt StringToInt(const BString &str)
  {
      return atoi(str.c_str());
  }
  
  inline BFloat StringToFloat(const BString &str)
  {
      return (BFloat)atof(str.c_str());
  }
  
  inline BBool EqualFloat(BFloat l,BFloat r)
  {
      return abs(l-r)<=EPSILON_E6;
  }
  
  inline BString getNameFromPath(const BString &path)
  {
      std::size_t beg = path.find_last_of("\\/");
      std::size_t end = path.find_last_of(".");
      if(beg == BString::npos)
      {
	
	beg = -1;
      }
      return path.substr(beg+1,end-beg-1);
  }
  
  inline BString Trim(const BString &msg)
  {
      const static BString SPACE_CHAR = " \t\f\v\n\r";
      std::size_t beg = msg.find_first_not_of(SPACE_CHAR);
      
      if(beg > msg.length())
      {
	return BString();
      }
      BString result = msg.substr(beg);
      
      std::size_t end = result.find_last_not_of(SPACE_CHAR);
      
      if(end != BString::npos)
	  end++;
      return result.substr(0,end);
    
  }
  
  inline BString timeFormat(long long timedec)
  {
      time_t tTime = time(NULL) ;   
      struct tm *tmTime;   //定义tm类型指针
      tmTime = localtime(&tTime);  //获取时间
      
      char tmpBuf[BUFLEN];   
      strftime(tmpBuf, BUFLEN, "%F %T", tmTime); //format date and time. 
      return BString(tmpBuf);
      
  }
  
  BString now();
  
}