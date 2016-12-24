#include <BRSDataSource.h>
#include <BRSKernelError.h>
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


BRSQuque::BRSQuque()
{
    nb_video = nb_audio = 0;
    
}

BRSQuque::~BRSQuque()
{

}

void BRSQuque::push(BrsSharedPtrMessage* msgp)
{
      if(!msgp)
	return;
      minQuque.insert(std::make_pair<int64_t,BrsSharedPtrMessage*>(msgp->timestamp,msgp));
      if(msgp->is_video())
	nb_video++;
      else
	nb_audio++;
}

BrsSharedPtrMessage* BRSQuque::pop()
{
      minItr itr = minQuque.begin();
      BrsSharedPtrMessage * msg = itr->second;
      minQuque.erase(itr);
      if(msg->is_video())
	nb_video--;
      else
	nb_audio--;
      return msg;
}

void BRSQuque::clear()
{
    minItr itr;
    for(itr=minQuque.begin();itr!=minQuque.end();++itr)
    {
      BrsSharedPtrMessage * msg = itr->second;
      SafeDelete(msg);
    }
    minQuque.clear();
}


}