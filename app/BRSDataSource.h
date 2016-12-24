#pragma once 

#include <BRSCommon.h>
#include <BRSKernelCodec.h>
#include <BRSFlv.h>
#include <BRSRtmpPackets.h>

namespace BRS 
{
  
#define BRS_PURE_AUDIO_GUESS_COUNT 115
  
extern int64_t brs_generate_id();

class BRSGopCache
{
private:
  bool enable_cache_gop;
  
  bool cached_video_count;
  
  bool audio_after_last_video_count;
  
  std::vector<BrsSharedPtrMessage*> gop_cache;
public:
  BRSGopCache();
  virtual ~BRSGopCache();
public:
  virtual void dispose();
  
  virtual void set(bool enabled);
  
  virtual int cache(BrsSharedPtrMessage* shared_mg);
  
  virtual void clear();
  
  virtual bool empty();
  
  virtual int64_t start_time();
  
  virtual bool pure_audio();
};

class BRSQuque
{
private:
  std::multimap<uint64_t,BrsSharedPtrMessage*> minQuque;
  int nb_video;
  int nb_audio;
  typedef std::multimap<int64_t,BrsSharedPtrMessage*>::iterator minItr;
  
public:
  BRSQuque();
  virtual ~BRSQuque();
public:
  
  virtual void push(BrsSharedPtrMessage*);
  
  virtual BrsSharedPtrMessage* pop();
  
  virtual void clear();
};

}