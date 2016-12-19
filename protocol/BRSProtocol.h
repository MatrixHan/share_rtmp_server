#pragma once 

#include <BRSCommon.h>
#include <BRSStream.h>
#include <BRSReadWriter.h>


namespace BRS 
{
  
  #define CLASS_NAME_STRING(className) #className

/**
* max rtmp header size:
* 	1bytes basic header,
* 	11bytes message header,
* 	4bytes timestamp header,
* that is, 1+11+4=16bytes.
*/
#define RTMP_MAX_FMT0_HEADER_SIZE 16
/**
* max rtmp header size:
* 	1bytes basic header,
* 	4bytes timestamp header,
* that is, 1+4=5bytes.
*/
#define RTMP_MAX_FMT3_HEADER_SIZE 5

/**
* the protocol provides the rtmp-message-protocol services,
* to recv RTMP message from RTMP chunk stream,
* and to send out RTMP message over RTMP chunk stream.
*/
  class BRSProtocol:virtual public BRSStream,virtual public BRSBitStream
  {
  private:
    BRSReadWriter  *skt;
    
  public:
      BRSProtocol(int clientfd);
      virtual ~BRSProtocol();
  };
  
  
}