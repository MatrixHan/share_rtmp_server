#pragma once


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#ifndef _WIN32
#include <inttypes.h>
#endif

#include <signal.h>

#include <math.h>
#include <algorithm>
#include <assert.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <queue>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdarg.h>
#include "BRSHeader.h"

namespace BRS
{
  typedef char 		BChar;
  typedef short 	BShort;
  typedef float 	BFloat;
  typedef int 		BInt;
  typedef bool  	BBool;
  typedef long 		BLong;
  typedef double	BDouble;
  
  typedef std::string	BString;
  typedef std::wstring 	BWString;
  
  typedef unsigned char 	BUChar;
  typedef unsigned short 	BUShort;
  typedef unsigned int 		BUInt;
  
  
#define Abs(a) ((a) > 0 ? (a) : -(a))

#define STRLENGTH(str) (sizeof(str))
  
#define SafeDelete(p) if(p) {delete p;p=0x0;}
  
#define SafeDeleteArray(p) if(p) {delete [] p;p=0x0;}
  
#define COUT(a) std::cout<< #a <<" : "<< (a) <<std:endl;
  
#define BCOLOR_16BIT(r,g,b) ( ((r) & 0xff)<<16 |  ((g) & 0xff)<<8  | ((b) & 0xff) )
  
#define COLOR255(rgb) ((rgb)>255?255:(rgb))
  
#define EPSILON_E6 (BFloat)(1E-6)
  
#define ERR_EXIT(m) \
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while(0)

/**
* disable copy constructor of class,
* to avoid the memory leak or corrupt by copy instance.
*/
#define disable_default_copy(className)\
    private:\
        /** \
        * disable the copy constructor and operator=, donot allow directly copy. \
        */ \
        className(const className&); \
        className& operator= (const className&)

        
// stable major version
#define VERSION_STABLE 1
#define VERSION_STABLE_BRANCH BRS_XSTR(VERSION_STABLE)".0release"        

#define BRS_XSTR(v) BRS_INTERNAL_STR(v)
#define BRS_INTERNAL_STR(v) #v
        
// server info.
#define RTMP_SIG_BRS_KEY "BRS"
#define RTMP_SIG_BRS_CODE "Brave Han"
#define RTMP_SIG_BRS_ROLE "origin/edge server"
#define RTMP_SIG_BRS_NAME RTMP_SIG_BRS_KEY"(Brave RTMP Server)"
#define RTMP_SIG_BRS_URL_SHORT "http://fxother.com/"
#define RTMP_SIG_BRS_URL "https://"RTMP_SIG_BRS_URL_SHORT
#define RTMP_SIG_BRS_WEB "http://fxother.com/"
#define RTMP_SIG_BRS_EMAIL "7248329hy@163.com"
#define RTMP_SIG_BRS_LICENSE "The MIT License (MIT)"
#define RTMP_SIG_BRS_COPYRIGHT "Copyright (c) 2016-2018"
#define RTMP_SIG_BRS_PRIMARY "BRS/"VERSION_STABLE_BRANCH
#define RTMP_SIG_BRS_AUTHROS "brave han"
#define RTMP_SIG_BRS_CONTRIBUTORS_URL RTMP_SIG_BRS_URL"/blob/master/AUTHORS.txt"
#define RTMP_SIG_BRS_HANDSHAKE RTMP_SIG_BRS_KEY"("RTMP_SIG_BRS_VERSION")"
#define RTMP_SIG_BRS_RELEASE RTMP_SIG_BRS_URL"/master"
#define RTMP_SIG_BRS_ISSUES(id) RTMP_SIG_BRS_URL"/issues/"#id
#define RTMP_SIG_BRS_VERSION BRS_XSTR(VERSION_MAJOR)"."BRS_XSTR(VERSION_MINOR)"."BRS_XSTR(VERSION_REVISION)
#define RTMP_SIG_BRS_SERVER RTMP_SIG_BRS_KEY"/"RTMP_SIG_BRS_VERSION"("RTMP_SIG_BRS_CODE")"
  
}