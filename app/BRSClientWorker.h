#pragma once 

#include <BRSCommon.h>
#include <BRSLog.h>
#include <BRSClientContext.h>
#include <BRSCoroutine.h>
#include <BRSHandShake.h>
#include <BRSProtocol.h>
#include <BRSRtmpStack.h>
#include <BRSAppUtility.h>
namespace BRS 
{
  
// when stream is busy, for example, streaming is already
// publishing, when a new client to request to publish,
// sleep a while and close the connection.
#define BRS_STREAM_BUSY_SLEEP_US (int64_t)(3*1000*1000LL)

// the timeout to wait encoder to republish
// if timeout, close the connection.
#define BRS_REPUBLISH_SEND_TIMEOUT_US (int64_t)(3*60*1000*1000LL)
// if timeout, close the connection.
#define BRS_REPUBLISH_RECV_TIMEOUT_US (int64_t)(3*60*1000*1000LL)

// the timeout to wait client data, when client paused
// if timeout, close the connection.
#define BRS_PAUSED_SEND_TIMEOUT_US (int64_t)(30*60*1000*1000LL)
// if timeout, close the connection.
#define BRS_PAUSED_RECV_TIMEOUT_US (int64_t)(30*60*1000*1000LL)

// when edge timeout, retry next.
#define BRS_EDGE_TOKEN_TRAVERSE_TIMEOUT_US (int64_t)(3*1000*1000LL)
  
  class BRSServer;
  class BRSSource;
  class BRSClientWorker:virtual public BRSWorker 
  {
  private:
    bool disposed;
    std::string ip;
  private:
   BRSComplexHandShake * complexHandshake;
   BrsRequest			*req;
   BrsResponse			*res;
   BRSReadWriter                *skt;
   BrsRtmpServer                *rtmp;
  public:
    BRSClientWorker(int pfd,BRSServer *mserver);
    virtual ~BRSClientWorker();
    virtual  void do_something();
    
    virtual  int  rtmpHandshake();

    
    virtual  int  service_cycle();
    virtual  int  stream_service_cycle();
    
    virtual int publish(BRSSource * source);
    
    virtual int playing(BRSSource * source);
    
    virtual int do_playing(BRSSource * source);
    
  };
  
}