#include <BRSDataSource.h>
#include <BRSKernelError.h>
#include <BRSMath.h>
namespace BRS 
{
  
int64_t brs_gvid = getpid();

int64_t brs_generate_id()
{
    return brs_gvid++;
}
  
BRSGopCache::BRSGopCache()
{
    cached_video_count = 0;
    enable_cache_gop = true;
    audio_after_last_video_count = 0;
}

BRSGopCache::~BRSGopCache()
{
    clear();
}

void BRSGopCache::dispose()
{
  clear();
}

void BRSGopCache::set(bool enabled)
{
    enable_cache_gop = enabled;
    if(!enabled){
      brs_info("disable gop cache, clear %d packets.", (int)gop_cache.size());
        clear();
        return;
    }
    brs_info("enable gop cache");
}

int BRSGopCache::cache(BrsSharedPtrMessage* shared_mg)
{
    int ret = ERROR_SUCCESS;
    if(!enable_cache_gop)
    {
      brs_verbose("gop cache disabled!");
      return ret;
    }
    
    BrsSharedPtrMessage * ms=shared_mg;
    if(ms->is_video())
    {
      if(!BrsFlvCodec::video_is_h264(ms->payload,ms->size))
      {
	  brs_info("gop cache drop video for none h.264");
	  return ret;
      }
      cached_video_count++;
      audio_after_last_video_count = 0;
    }
    if(pure_audio())
    {
	brs_verbose("ignore any frame util got a h264 video frame.");
        return ret;
    }
    
    if(ms->is_audio())
    {
      audio_after_last_video_count++;
    }
    
    if(audio_after_last_video_count > BRS_PURE_AUDIO_GUESS_COUNT)
    {
	brs_warn("clear gop cache for guess pure audio overflow");
        clear();
        return ret;
    }
    // clear gop cache when got key frame
    if (ms->is_video() && BrsFlvCodec::video_is_keyframe(ms->payload, ms->size)) {
        brs_info("clear gop cache when got keyframe. vcount=%d, count=%d",
            cached_video_count, (int)gop_cache.size());
            
        clear();
        
        // curent msg is video frame, so we set to 1.
        cached_video_count = 1;
    }
    
    // cache the frame.
    gop_cache.push_back(ms->copy());
    
    return ret;
}

bool BRSGopCache::empty()
{
    return gop_cache.empty();
}


void BRSGopCache::clear()
{
    std::vector<BrsSharedPtrMessage*>::iterator itr;
    for(itr = gop_cache.begin();itr!=gop_cache.end();++itr)
    {
	   BrsSharedPtrMessage *msg = *itr;
	   SafeDelete(msg);
    }
    gop_cache.clear();
    
    cached_video_count = 0;
    audio_after_last_video_count = 0;
}

bool BRSGopCache::pure_audio()
{
    return cached_video_count==0;
}

int64_t BRSGopCache::start_time()
{
    if(empty())
    {
      return 0;
    }
    BrsSharedPtrMessage * msg = gop_cache[0];
    assert(msg);
    return msg->timestamp;
}


BRSQueue::BRSQueue()
{
    nb_video = nb_audio = 0;
    
}

BRSQueue::~BRSQueue()
{

}

void BRSQueue::push(BrsSharedPtrMessage* msgp)
{
      if(!msgp)
	return;
      minQueue.insert(std::make_pair<int64_t,BrsSharedPtrMessage*>(msgp->timestamp,msgp));
      if(msgp->is_video())
	nb_video++;
      else
	nb_audio++;
}

BrsSharedPtrMessage* BRSQueue::pop()
{
      minItr itr = minQueue.begin();
      BrsSharedPtrMessage * msg = itr->second;
      minQueue.erase(itr);
      if(msg->is_video())
	nb_video--;
      else
	nb_audio--;
      return msg;
}

void BRSQueue::clear()
{
    minItr itr;
    for(itr=minQueue.begin();itr!=minQueue.end();++itr)
    {
      BrsSharedPtrMessage * msg = itr->second;
      SafeDelete(msg);
    }
    minQueue.clear();
}


BRSMessageQueue::BRSMessageQueue(bool ignore_shrink)
{
    _ignore_shrink = ignore_shrink;
    queue_size_ms = 0;
    av_start_time = av_end_time =-1;
}

BRSMessageQueue::~BRSMessageQueue()
{
    clear();
}

int BRSMessageQueue::size()
{
    return (int)msgs.size();
}

int BRSMessageQueue::duration()
{
    return (int)(av_end_time - av_start_time);
}

void BRSMessageQueue::set_queue_size(double queue_size)
{
     queue_size_ms = (int)(queue_size*1000);
}

int BRSMessageQueue::enqueue(BrsSharedPtrMessage* msg, bool* is_overflow)
{
      int ret = ERROR_SUCCESS;
      
      if(msg->is_av())
      {
	if(av_start_time==-1)
	{
	  av_start_time = msg->timestamp;
	}
	av_end_time = msg->timestamp;
      }
      
      msgs.push_back(msg);
      
      if(av_end_time - av_start_time > queue_size_ms)
      {
	if(is_overflow)
	{
	  *is_overflow = true;
	}
	shrink();
      }
      
      return ret;
}

int BRSMessageQueue::dump_packets(int max_count, BrsSharedPtrMessage** pmsgs, int& count)
{
    int ret = ERROR_SUCCESS;
    
    int nb_msgs = (int)msgs.size();
    if(nb_msgs<=0)
    {
      return ret;
    }
    
    assert(max_count>0);
    count = Min(max_count,nb_msgs);
    
    BrsSharedPtrMessage ** omsgs = msgs.data();
    
    for(int i = 0;i < count ; i++)
    {
      pmsgs[i] = omsgs[i];
    }
    
    BrsSharedPtrMessage * msg = omsgs[count-1];
    av_start_time = msg->timestamp;
    
    if(count >= nb_msgs)
    {
       msgs.clear();
    }
    else
    {
      msgs.erase(msgs.begin(),msgs.begin()+count);
    }
    return ret;
}

void BRSMessageQueue::shrink()
{
    BrsSharedPtrMessage * video_sh = NULL;
    BrsSharedPtrMessage * audio_sh = NULL;
    int msgs_size = (int)msgs.size();
    
    for (int i = 0;i < (int)msgs.size();i++)
    {
	BrsSharedPtrMessage * msg = msgs.at(i);
	if(msg->is_video() && BrsFlvCodec::video_is_sequence_header(msg->payload,msg->size))
	{
	  SafeDelete(video_sh);
	  video_sh = msg;
	  continue;
	}
	if(msg->is_audio() && BrsFlvCodec::audio_is_sequence_header(msg->payload,msg->size))
	{
	  SafeDelete(audio_sh);
	  audio_sh = msg;
	  continue;
	}
	
	SafeDelete(msg);
    }
    
    msgs.clear();
    
    av_start_time = av_end_time;
    
    if(video_sh)
    {
      video_sh->timestamp = av_end_time;
      msgs.push_back(video_sh);
    }
    
    if(audio_sh)
    {
      audio_sh->timestamp = av_end_time;
      msgs.push_back(audio_sh);
    }
    
    if (_ignore_shrink) {
        brs_info("shrink the cache queue, size=%d, removed=%d, max=%.2f", 
            (int)msgs.size(), msgs_size - (int)msgs.size(), queue_size_ms / 1000.0);
    } else {
        brs_trace("shrink the cache queue, size=%d, removed=%d, max=%.2f", 
            (int)msgs.size(), msgs_size - (int)msgs.size(), queue_size_ms / 1000.0);
    }
    
}

void BRSMessageQueue::clear()
{
    std::vector<BrsSharedPtrMessage*>::iterator it;

    for (it = msgs.begin(); it != msgs.end(); ++it) {
        BrsSharedPtrMessage* msg = *it;
        SafeDelete(msg);
    }
    
     msgs.clear();
    
     av_start_time = av_end_time = -1;
}

BRSConsumer::BRSConsumer(BRSSource* _source)
{
    source = _source;
    paused = false;
    queue = new BRSMessageQueue();
    should_update_source_id = false;
}

BRSConsumer::~BRSConsumer()
{
    //source->on_consumer_destroy(this);
      SafeDelete(queue);
}

int BRSConsumer::get_time()
{

}

void BRSConsumer::set_queue_size(double queue_size)
{
    queue->set_queue_size(queue_size);
}

void BRSConsumer::update_source_id()
{
    should_update_source_id = true;
}

int BRSConsumer::enqueue(BrsSharedPtrMessage* shared_ms)
{
    int ret = ERROR_SUCCESS;
    
    BrsSharedPtrMessage *msg = shared_ms->copy();
    
    if((ret = queue->enqueue(msg,NULL)) != ERROR_SUCCESS)
    {
      return ret;
    }
    
    return ret;
}

int BRSConsumer::dump_packets(BrsMessageArray* msgs, int& count)
{
    int ret =ERROR_SUCCESS;
    
    assert(count >= 0);
    assert(msgs->max > 0);
    
    // the count used as input to reset the max if positive.
    int max = count? Min(count, msgs->max) : msgs->max;
    
    // the count specifies the max acceptable count,
    // here maybe 1+, and we must set to 0 when got nothing.
    count = 0;
    
    if (should_update_source_id) {
        brs_trace("update source_id=%d[%d]", source->source_id(), source->source_id());
        should_update_source_id = false;
    }
    
    // paused, return nothing.
    if (paused) {
        return ret;
    }

    // pump msgs from queue.
    if ((ret = queue->dump_packets(max, msgs->msgs, count)) != ERROR_SUCCESS) {
        return ret;
    }
    
    return ret;
}

int BRSConsumer::on_play_client_pause(bool is_pause)
{
    int ret = ERROR_SUCCESS;
    
    brs_trace("stream consumer change pause state %d=>%d", paused, is_pause);
    paused = is_pause;
    
    return ret;
}


}