#pragma once
#include <math.h>
#include <algorithm>
#include <assert.h>
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <iostream>
#include "BRSHeader.h"

namespace BRS
{
  typedef char 		BChar;
  typedef short 	BShort;
  typedef float 	BFloat;
  typedef int 		EInt;
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
  
  
}