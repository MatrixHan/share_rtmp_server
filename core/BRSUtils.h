#pragma once

#include <BRSCommon.h>
#include <BRSLog.h>
#define BUFLEN 255   

namespace BRS 
{
 
    long long getTimeDec();
 
  
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
  
  inline BString LongToString(long long num)
  {
    std::ostringstream ss;
    ss << num;
    BString result;  
    std::istringstream is(ss.str());  
    is>>result; 
    return result;
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
      return Abs(l-r)<=EPSILON_E6;
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
  
  BString timeFormat(long long timedec);
  
  
  BString now();
  
  long long getCurrentTime();
  
}