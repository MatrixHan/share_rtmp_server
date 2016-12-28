#pragma once 

#include <BRSCommon.h>
#include <BRSKernelCodec.h>
#include <BRSFlv.h>
#include <BRSRtmpPackets.h>
#include <BRSRtmpMsgArray.h>

using namespace std;

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

class BRSQueue
{
private:
  std::multimap<int64_t,BrsSharedPtrMessage*> minQueue;
  int nb_video;
  int nb_audio;
  typedef std::multimap<int64_t,BrsSharedPtrMessage*>::iterator minItr;
  
public:
  BRSQueue();
  virtual ~BRSQueue();
public:
  
  virtual void push(BrsSharedPtrMessage*);
  
  virtual BrsSharedPtrMessage* pop();
  
  virtual void clear();
};

class BRSMessageQueue
{
private:
   bool _ignore_shrink;
  int64_t av_start_time;
  int64_t av_end_time;
  int	  queue_size_ms;
  std::vector<BrsSharedPtrMessage*> msgs;
public:
  BRSMessageQueue(bool ignore_shrink = false);
  virtual ~BRSMessageQueue();
  
public:
  //
  virtual int size();
  //队列时长
  virtual int duration();
  
  virtual void set_queue_size(double queue_size);
  
public:
  
  virtual int enqueue(BrsSharedPtrMessage* msg,bool * is_overflow = NULL);
  
  virtual int dump_packets(int max_count,BrsSharedPtrMessage **pmsgs,int & count);
  
private:
    /**
    * remove a gop from the front.
    * if no iframe found, clear it.
    * last packet push this queue
    */
    virtual void shrink();
public:
    /**
     * clear all messages in queue.
     */
    virtual void clear();
};
class BRSConsumer;
class BrsRtmpServer;
class BRSSource
{
private:
  static std::map<std::string,BRSSource*> pool;
public:
  static int create(BrsRequest *req,BRSSource **pps);
  
  static BRSSource * fetch(BrsRequest *req);
  
  static BRSSource * fetch(std::string vhost,std::string app,std::string stream);
  
  static void dispose_all();
  
  static int cycle_all();
    /**
    * when system exit, destroy the sources,
    * for gmc to analysis mem leaks.
    */
  static void destroy();
private:
    // source id,
    // for publish, it's the publish client id.
    // for edge, it's the edge ingest id.
    // when source id changed, for example, the edge reconnect,
    // invoke the on_source_id_changed() to let all clients know.
    int _source_id;
    // deep copy of client request.
    BrsRequest* _req;
    // to delivery stream to clients.
    std::vector<BRSConsumer*> consumers;
    // the time jitter algorithm for vhost.
  //  BrsRtmpJitterAlgorithm jitter_algorithm;
    // whether use interlaced/mixed algorithm to correct timestamp.
    bool mix_correct;
    BRSQueue* mix_queue;
    // whether stream is monotonically increase.
    bool is_monotonically_increase;
    int64_t last_packet_time;
public:
  BRSSource();
  virtual ~BRSSource();
public:
  
  virtual int initialize(BrsRequest *r);
  
  virtual void dispose();
  
  virtual int onMetaData(BrsCommonMessage *msg ,BrsOnMetaDataPacket *metadata);
  
  virtual int onVideo(BrsCommonMessage *msg);
  
  virtual int onVideoImp(BrsSharedPtrMessage *videoMsg);
  
  virtual int onAudio(BrsCommonMessage *msg);
  
  virtual int onAudioImp(BrsSharedPtrMessage *audioMsg);
private:
    BrsSharedPtrMessage* cache_metadata;
    // the cached video sequence header.
    BrsSharedPtrMessage* cache_sh_video;
    // the cached audio sequence header.
    BrsSharedPtrMessage* cache_sh_audio;
    
public:
  virtual int pushConsumer(BRSConsumer * consumer);
    
  virtual int delConsumer(BRSConsumer * consumer);
  
  virtual int forwardAll();
};


class BRSConsumer
{
private:
    BRSSource * source;
    BRSMessageQueue *queue;
    
    BrsRtmpServer *rtmp;
    
    int  clientFd;
    
   BrsResponse * res;
    
    bool paused;
    
    // when source id changed, notice all consumers
    bool should_update_source_id;
    
public:
  BRSConsumer(BRSSource *_source,int _clientFd, BrsRtmpServer *rtmp,BrsResponse *_res);
  virtual ~BRSConsumer();
public:
  
  virtual void set_queue_size(double queue_size);
  
  virtual void update_source_id();
  
public:
  
  virtual int getClientFd();
  
  virtual int get_time();
  
  virtual int enqueue(BrsSharedPtrMessage * shared_ms);
  
  virtual int dump_packets(BrsMessageArray * msgs,int &count);
  
  virtual int get_queue_size();
  
  virtual int on_play_client_pause(bool is_pause);
  
  virtual int forward();

};



}