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

  
  
}