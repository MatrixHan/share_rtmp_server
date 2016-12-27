#include <BRSDataSource.h>
#include <BRSKernelError.h>
#include <BRSMath.h>
#include <BRSRtmpUtility.h>
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
      bool mix_ok = false;
     // pure video
    if (nb_video >= 10 && nb_audio == 0) {
        mix_ok = true;
    }
    
    // pure audio
    if (nb_audio >= 10 && nb_video == 0) {
        mix_ok = true;
    }
    
    // got 1 video and 1 audio, mix ok.
    if (nb_video >= 1 && nb_audio >= 1) {
        mix_ok = true;
    }
    
    if (!mix_ok) {
        return NULL;
    }
  
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

BRSConsumer::BRSConsumer(BRSSource* _source,int _clientFd)
{
    source = _source;
    clientFd = _clientFd;
    paused = false;
    queue = new BRSMessageQueue();
    should_update_source_id = false;
}

BRSConsumer::~BRSConsumer()
{
    //source->on_consumer_destroy(this);
      SafeDelete(queue);
}

int BRSConsumer::getClientFd()
{
    return clientFd;
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
        //brs_trace("update source_id=%d[%d]", source->source_id(), source->source_id());
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


std::map<std::string,BRSSource*> BRSSource::pool;

int BRSSource::create(BrsRequest* req, BRSSource** pps)
{
    int ret = ERROR_SUCCESS;
    
    string stream_utl = req->get_stream_url();
    
    string vhost = req->vhost;

    assert(pool.find(stream_utl) == pool.end());
    
    BRSSource* source = new BRSSource();
    if((ret = source->initialize(req))!=ERROR_SUCCESS)
    {
       SafeDelete(source);
       return ret;
    }
    
    pool[stream_utl] = source;
    *pps = source;
    
    return ret;
    
}

BRSSource* BRSSource::fetch(std::string vhost, std::string app, std::string stream)
{
    BRSSource * source = NULL;
    
    string stream_url = brs_generate_stream_url(vhost,app,stream);
    if (pool.find(stream_url) == pool.end()) {
        return NULL;
    }

    source = pool[stream_url];

    return source;
    
}

BRSSource* BRSSource::fetch(BrsRequest* req)
{
    BRSSource* source = NULL;
    
    string stream_url = req->get_stream_url();
    if (pool.find(stream_url) == pool.end()) {
        return NULL;
    }

    source = pool[stream_url];

    // we always update the request of resource, 
    // for origin auth is on, the token in request maybe invalid,
    // and we only need to update the token of request, it's simple.
    source->_req->update_auth(req);

    return source;
}

int BRSSource::cycle_all()
{

}


void BRSSource::dispose_all()
{
    std::map<std::string,BRSSource*>::iterator itr;
      for(itr = pool.begin();itr!=pool.end();++itr)
      {
	 BRSSource *source = itr->second;
	 source->dispose();
      }
     
}


void BRSSource::destroy()
{
      std::map<std::string,BRSSource*>::iterator itr;
      for(itr = pool.begin();itr!=pool.end();++itr)
      {
	 BRSSource *source = itr->second;
	 SafeDelete(source);
      }
      pool.clear();
}


BRSSource::BRSSource()
{
    _req = NULL;
    mix_correct = false;
    mix_queue = new BRSQueue();
    cache_metadata = cache_sh_video = cache_sh_audio = NULL;
    last_packet_time = 0;
    is_monotonically_increase = false;
    _source_id = -1;
}

BRSSource::~BRSSource()
{
    SafeDelete(mix_queue);
    SafeDelete(_req);
}

int BRSSource::initialize(BrsRequest* r)
{
      _req = r->copy();
      
}

void BRSSource::dispose()
{
    SafeDelete(cache_metadata);
    SafeDelete(cache_sh_video);
    SafeDelete(cache_sh_audio);
}


int BRSSource::onMetaData(BrsCommonMessage* msg, BrsOnMetaDataPacket* metadata)
{
  
    int ret = ERROR_SUCCESS;
  
    BrsAmf0Any* prop = NULL;
    
    // when exists the duration, remove it to make ExoPlayer happy.
    if (metadata->metadata->get_property("duration") != NULL) {
        metadata->metadata->remove("duration");
    }
    
    // generate metadata info to print
    std::stringstream ss;
    if ((prop = metadata->metadata->ensure_property_number("width")) != NULL) {
        ss << ", width=" << (int)prop->to_number();
    }
    if ((prop = metadata->metadata->ensure_property_number("height")) != NULL) {
        ss << ", height=" << (int)prop->to_number();
    }
    if ((prop = metadata->metadata->ensure_property_number("videocodecid")) != NULL) {
        ss << ", vcodec=" << (int)prop->to_number();
    }
    if ((prop = metadata->metadata->ensure_property_number("audiocodecid")) != NULL) {
        ss << ", acodec=" << (int)prop->to_number();
    }
    brs_trace("got metadata%s", ss.str().c_str());
    
    // add server info to metadata
    metadata->metadata->set("server", BrsAmf0Any::str(RTMP_SIG_BRS_SERVER));
    metadata->metadata->set("srs_primary", BrsAmf0Any::str(RTMP_SIG_BRS_PRIMARY));
    metadata->metadata->set("srs_authors", BrsAmf0Any::str(RTMP_SIG_BRS_AUTHROS));
    
    // version, for example, 1.0.0
    // add version to metadata, please donot remove it, for debug.
    metadata->metadata->set("server_version", BrsAmf0Any::str(RTMP_SIG_BRS_VERSION));
    
    
    // encode the metadata to payload
    int size = 0;
    char* payload = NULL;
    if ((ret = metadata->encode(size, payload)) != ERROR_SUCCESS) {
        brs_error("encode metadata error. ret=%d", ret);
        SafeDelete(payload);
        return ret;
    }
    brs_verbose("encode metadata success.");
    
    if (size <= 0) {
        brs_warn("ignore the invalid metadata. size=%d", size);
        return ret;
    }
    
     bool drop_for_reduce = false;
    if (cache_metadata) {
        drop_for_reduce = true;
        brs_warn("drop for reduce sh metadata, size=%d", msg->size);
    }
    
    // create a shared ptr message.
    SafeDelete(cache_metadata);
    cache_metadata = new BrsSharedPtrMessage();
    
    if ((ret = cache_metadata->create(&msg->header, payload, size)) != ERROR_SUCCESS) {
        brs_error("initialize the cache metadata failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("initialize shared ptr metadata success.");
    
    
    // copy to all consumer
    if (!drop_for_reduce) {
        std::vector<BRSConsumer*>::iterator it;
        for (it = consumers.begin(); it != consumers.end(); ++it) {
            BRSConsumer* consumer = *it;
            if ((ret = consumer->enqueue(cache_metadata)) != ERROR_SUCCESS) {
                brs_error("dispatch the metadata failed. ret=%d", ret);
                return ret;
            }
        }
    }
    
    return ret;
}

int BRSSource::onVideo(BrsCommonMessage* shared_video)
{
      int ret = ERROR_SUCCESS;
      
      if (!mix_correct && is_monotonically_increase)
      {
	 if (last_packet_time > 0 && shared_video->header.timestamp < last_packet_time)
	 {
	   is_monotonically_increase = false;
	   brs_warn("VIDEO: stream not monotonically increase, please open mix_correct.");
	}
      }
      
      last_packet_time = shared_video->header.timestamp;
      
      if (!BrsFlvCodec::video_is_acceptable(shared_video->payload, shared_video->size)) {
        char b0 = 0x00;
        if (shared_video->size > 0) {
            b0 = shared_video->payload[0];
        }
        
        brs_warn("drop unknown header video, size=%d, bytes[0]=%#x", shared_video->size, b0);
        return ret;
      }
      
      BrsSharedPtrMessage msg;
      if ((ret = msg.create(shared_video))!=ERROR_SUCCESS)
      {
	brs_error("initialize the video failed. ret=%d", ret);
	return ret;
      }
      
      brs_info("Video dts=%"PRId64", size=%d", msg.timestamp, msg.size);
    
    // directly process the audio message.
    if (!mix_correct) {
        return onVideoImp(&msg);
    }
    
    // insert msg to the queue.
    mix_queue->push(msg.copy());
    
    // fetch someone from mix queue.
    BrsSharedPtrMessage* m = mix_queue->pop();
    if (!m) {
        return ret;
    }
    BrsAutoFreeE(BrsSharedPtrMessage, m);
    
    // consume the monotonically increase message.
    if (m->is_audio()) {
        ret = onAudioImp(m);
    } else {
        ret = onVideoImp(m);
    }
    SafeDelete(m);
    
    return ret;
    
}

int BRSSource::onVideoImp(BrsSharedPtrMessage* msg)
{
      int ret = ERROR_SUCCESS;
      
      brs_info("Video dts=%"PRId64", size=%d", msg->timestamp, msg->size);
      
      bool is_sequence_header = BrsFlvCodec::video_is_sequence_header(msg->payload, msg->size);
    
    // whether consumer should drop for the duplicated sequence header.
      bool drop_for_reduce = false;
      
      if (is_sequence_header) {
        SafeDelete(cache_sh_video);
        cache_sh_video = msg->copy();
        
        // parse detail audio codec
        BrsAvcAacCodec codec;
        
        // user can disable the sps parse to workaround when parse sps failed.
        // @see https://github.com/ossrs/srs/issues/474
        codec.avc_parse_sps = false;
        
        BrsCodecSample sample;
        if ((ret = codec.video_avc_demux(msg->payload, msg->size, &sample)) != ERROR_SUCCESS) {
            brs_error("source codec demux video failed. ret=%d", ret);
            return ret;
        }
        
        brs_trace("%dB video sh,  codec(%d, profile=%s, level=%s, %dx%d, %dkbps, %dfps, %ds)",
            msg->size, codec.video_codec_id,
            brs_codec_avc_profile2str(codec.avc_profile).c_str(),
            brs_codec_avc_level2str(codec.avc_level).c_str(), codec.width, codec.height,
            codec.video_data_rate / 1000, codec.frame_rate, codec.duration);
    }
    
     // copy to all consumer
    if (!drop_for_reduce) {
        for (int i = 0; i < (int)consumers.size(); i++) {
            BRSConsumer* consumer = consumers.at(i);
            if ((ret = consumer->enqueue(msg)) != ERROR_SUCCESS) {
                brs_error("dispatch the video failed. ret=%d", ret);
                return ret;
            }
        }
        brs_info("dispatch video success.");
    }
    
    return ret;
      
}



int BRSSource::onAudio(BrsCommonMessage* shared_audio)
{
    int ret = ERROR_SUCCESS;
    
    // monotically increase detect.
    if (!mix_correct && is_monotonically_increase) {
        if (last_packet_time > 0 && shared_audio->header.timestamp < last_packet_time) {
            is_monotonically_increase = false;
            brs_warn("AUDIO: stream not monotonically increase, please open mix_correct.");
        }
    }
    last_packet_time = shared_audio->header.timestamp;
    
    // convert shared_audio to msg, user should not use shared_audio again.
    // the payload is transfer to msg, and set to NULL in shared_audio.
    BrsSharedPtrMessage msg;
    if ((ret = msg.create(shared_audio)) != ERROR_SUCCESS) {
        brs_error("initialize the audio failed. ret=%d", ret);
        return ret;
    }
    brs_info("Audio dts=%"PRId64", size=%d", msg.timestamp, msg.size);
    
    // directly process the audio message.
    if (!mix_correct) {
        return onAudioImp(&msg);
    }
    
    // insert msg to the queue.
    mix_queue->push(msg.copy());
    
    // fetch someone from mix queue.
    BrsSharedPtrMessage* m = mix_queue->pop();
    if (!m) {
        return ret;
    }
    // consume the monotonically increase message.
    if (m->is_audio()) {
        ret = onAudioImp(m);
    } else {
        ret = onVideoImp(m);
    }
    SafeDelete(m);
    
    return ret;
}

int BRSSource::onAudioImp(BrsSharedPtrMessage* msg)
{
      int ret = ERROR_SUCCESS;
    
    brs_info("Audio dts=%"PRId64", size=%d", msg->timestamp, msg->size);
    bool is_aac_sequence_header = BrsFlvCodec::audio_is_sequence_header(msg->payload, msg->size);
    bool is_sequence_header = is_aac_sequence_header;
    
    // whether consumer should drop for the duplicated sequence header.
    bool drop_for_reduce = false;
    if (is_sequence_header && cache_sh_audio && false) {
        if (cache_sh_audio->size == msg->size) {
            drop_for_reduce = brs_bytes_equals(cache_sh_audio->payload, msg->payload, msg->size);
            brs_warn("drop for reduce sh audio, size=%d", msg->size);
        }
    }
    
    // cache the sequence header if aac
    // donot cache the sequence header to gop_cache, return here.
    if (is_aac_sequence_header) {
        // parse detail audio codec
        BrsAvcAacCodec codec;
        BrsCodecSample sample;
        if ((ret = codec.audio_aac_demux(msg->payload, msg->size, &sample)) != ERROR_SUCCESS) {
            brs_error("source codec demux audio failed. ret=%d", ret);
            return ret;
        }
        
        static int flv_sample_sizes[] = {8, 16, 0};
        static int flv_sound_types[] = {1, 2, 0};
        
      
        brs_trace("%dB audio sh, codec(%d, profile=%s, %dchannels, %dkbps, %dHZ), "
            "flv(%dbits, %dchannels, %dHZ)",
            msg->size, codec.audio_codec_id,
            brs_codec_aac_object2str(codec.aac_object).c_str(), codec.aac_channels,
            codec.audio_data_rate / 1000, aac_sample_rates[codec.aac_sample_rate],
            flv_sample_sizes[sample.sound_size], flv_sound_types[sample.sound_type],
            flv_sample_rates[sample.sound_rate]);
    }
    

    
    // copy to all consumer
    if (!drop_for_reduce) {
        for (int i = 0; i < (int)consumers.size(); i++) {
            BRSConsumer* consumer = consumers.at(i);
            if ((ret = consumer->enqueue(msg)) != ERROR_SUCCESS) {
                brs_error("dispatch the audio failed. ret=%d", ret);
                return ret;
            }
        }
        brs_info("dispatch audio success.");
    }
    
    

    // cache the sequence header of aac, or first packet of mp3.
    // for example, the mp3 is used for hls to write the "right" audio codec.
    // TODO: FIXME: to refine the stream info system.
    if (is_aac_sequence_header || !cache_sh_audio) {
        SafeDelete(cache_sh_audio);
        cache_sh_audio = msg->copy();
    }
    
    // when sequence header, donot push to gop cache and adjust the timestamp.
//     if (is_sequence_header) {
//         return ret;
//     }
    
   
    
    return ret;
}


}