#include <BRSClientWorker.h>
#include <BRSServer.h>
#include <BRSRtmpAmf0.h>


using namespace std;

namespace BRS 
{


BRSClientWorker::BRSClientWorker(int pfd,BRSServer *mserver):BRSWorker(mserver)
{
      
      complexHandshake = new BRSComplexHandShake();
      skt = new BRSReadWriter(pfd);
      req = new BrsRequest();
      res = new BrsResponse();
      skt = new BRSReadWriter(pfd);
      rtmp = new BrsRtmpServer(skt);
      disposed = false;
}


  
BRSClientWorker::~BRSClientWorker()
{
    SafeDelete(complexHandshake);
    SafeDelete(req);
    SafeDelete(res);
    SafeDelete(rtmp);
    SafeDelete(skt);
}



void BRSClientWorker::do_something()
{
     
      
     ip = brs_get_peer_ip(this->mContext.client_socketfd);
     int ret = ERROR_SUCCESS;
     ret=this->rtmpHandshake();
     if(ret!=ERROR_SUCCESS){
       brs_error("rtmp handshake failed. ret=%d", ret);
       close(this->mContext.client_socketfd);
       this->brsServer->closeClient(this->mContext.client_socketfd);
       return;
    }
    if((ret = rtmp->connect_app(req))!=ERROR_SUCCESS)
    {
	brs_error("rtmp connect vhost/app failed. ret=%d", ret);
        return ;
    }
    
    req->ip = ip;
    brs_info("discovery app success. schema=%s, vhost=%s, port=%s, app=%s",
        req->schema.c_str(), req->vhost.c_str(), req->port.c_str(), req->app.c_str());
    
    if (req->schema.empty() || req->vhost.empty() || req->port.empty() || req->app.empty()) {
        ret = ERROR_RTMP_REQ_TCURL;
        brs_error("discovery tcUrl failed. "
            "tcUrl=%s, schema=%s, vhost=%s, port=%s, app=%s, ret=%d",
            req->tcUrl.c_str(), req->schema.c_str(), req->vhost.c_str(), req->port.c_str(), req->app.c_str(), ret);
        return ;
    }
    brs_trace("connect app, "
        "tcUrl=%s, pageUrl=%s, swfUrl=%s, schema=%s, vhost=%s, port=%s, app=%s, args=%s", 
        req->tcUrl.c_str(), req->pageUrl.c_str(), req->swfUrl.c_str(), 
        req->schema.c_str(), req->vhost.c_str(), req->port.c_str(),
        req->app.c_str(), (req->args? "(obj)":"null"));
    // show client identity
    if(req->args) {
        std::string brs_version;
        std::string brs_server_ip;
        int brs_pid = 0;
        int brs_id = 0;
        
        BrsAmf0Any* prop = NULL;
        if ((prop = req->args->ensure_property_string("brs_version")) != NULL) {
            brs_version = prop->to_str();
        }
        if ((prop = req->args->ensure_property_string("brs_server_ip")) != NULL) {
            brs_server_ip = prop->to_str();
        }
        if ((prop = req->args->ensure_property_number("brs_pid")) != NULL) {
            brs_pid = (int)prop->to_number();
        }
        if ((prop = req->args->ensure_property_number("brs_id")) != NULL) {
            brs_id = (int)prop->to_number();
        }
        
        brs_info("edge-brs ip=%s, version=%s, pid=%d, id=%d", 
            brs_server_ip.c_str(), brs_version.c_str(), brs_pid, brs_id);
        if (brs_pid > 0) {
            brs_trace("edge-brs ip=%s, version=%s, pid=%d, id=%d", 
                brs_server_ip.c_str(), brs_version.c_str(), brs_pid, brs_id);
        }
    }
     ret = service_cycle();

     close(this->mContext.client_socketfd);
     this->brsServer->closeClient(this->mContext.client_socketfd);
}

int BRSClientWorker::rtmpHandshake()
{
    int ret = ERROR_SUCCESS;
	
    ssize_t nsize;
    
    
    char* c0c1 = new char[1537];
    BrsAutoFree(char, c0c1, true);
    if ((ret = skt->readn(c0c1, 1537, &nsize)) != ERROR_SUCCESS) {
        brs_warn("read c0c1 failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("read c0c1 success.");

	// plain text required.
	if (c0c1[0] != 0x03) {
		ret = ERROR_RTMP_PLAIN_REQUIRED;
		brs_warn("only support rtmp plain text. ret=%d", ret);
		return ret;
	}
    brs_verbose("check c0 success, required plain text.");
    
    // try complex handshake
    ret = complexHandshake->handshake(skt, c0c1 + 1);
    if (ret == ERROR_SUCCESS) {
	    brs_trace("complex handshake success.");
	    return ret;
    }
    if (ret != ERROR_RTMP_TRY_SIMPLE_HS) {
	    brs_error("complex handshake failed. ret=%d", ret);
    	return ret;
    }
    brs_info("complex handhskae failed, try simple. ret=%d", ret);
	
	char* s0s1s2 = new char[3073];
    BrsAutoFree(char, s0s1s2, true);
	// plain text required.
    s0s1s2[0] = 0x03;
    if ((ret = skt->writen(s0s1s2, 3073, &nsize)) != ERROR_SUCCESS) {
        brs_warn("simple handshake send s0s1s2 failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("simple handshake send s0s1s2 success.");
    
    char* c2 = new char[1536];
    BrsAutoFree(char, c2, true);
    if ((ret = skt->readn(c2, 1536, &nsize)) != ERROR_SUCCESS) {
        brs_warn("simple handshake read c2 failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("simple handshake read c2 success.");
    
    brs_trace("simple handshake success.");
    
	return ret;
}

int BRSClientWorker::service_cycle()
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = rtmp->set_window_ack_size((int)(2.5 * 1000 * 1000))) != ERROR_SUCCESS) {
        brs_error("set window acknowledgement size failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("set window acknowledgement size success");
        
    if ((ret = rtmp->set_peer_bandwidth((int)(2.5 * 1000 * 1000), 2)) != ERROR_SUCCESS) {
        brs_error("set peer bandwidth failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("set peer bandwidth success");
  
    // get the ip which client connected.
    string local_ip = brs_get_local_ip(this->mContext.client_socketfd);
    
    int chunk_size = 60000;
    if ((ret = rtmp->set_chunk_size(chunk_size)) != ERROR_SUCCESS) {
        brs_error("set chunk_size=%d failed. ret=%d", chunk_size, ret);
        return ret;
    }
    brs_info("set chunk_size=%d success", chunk_size);
    
    if ((ret = rtmp->response_connect_app(req, local_ip.c_str())) != ERROR_SUCCESS) {
        brs_error("response connect app failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("response connect app success");
        
    if ((ret = rtmp->on_bw_done()) != ERROR_SUCCESS) {
        brs_error("on_bw_done failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("on_bw_done success");
    
    while (!disposed) {
        ret = stream_service_cycle();
	//coroutine_yield(this->mContext.menv);
        // stream service must terminated with error, never success.
        // when terminated with success, it's user required to stop.
        if (ret == ERROR_SUCCESS) {
            continue;
        }
        
        // when not system control error, fatal error, return.
        if (!brs_is_system_control_error(ret)) {
            if (ret != ERROR_SOCKET_TIMEOUT && !brs_is_client_gracefully_close(ret)) {
                brs_error("stream service cycle failed. ret=%d", ret);
            }
            return ret;
        }
        
        // for republish, continue service
        if (ret == ERROR_CONTROL_REPUBLISH) {
            // set timeout to a larger value, wait for encoder to republish.
            rtmp->set_send_timeout(BRS_REPUBLISH_RECV_TIMEOUT_US);
            rtmp->set_recv_timeout(BRS_REPUBLISH_SEND_TIMEOUT_US);
            
            brs_trace("control message(unpublish) accept, retry stream service.");
            continue;
        }
        
        // for "some" system control error, 
        // logical accept and retry stream service.
        if (ret == ERROR_CONTROL_RTMP_CLOSE) {
            // TODO: FIXME: use ping message to anti-death of socket.
            // @see: https://github.com/osbrs/brs/issues/39
            // set timeout to a larger value, for user paused.
            rtmp->set_recv_timeout(BRS_PAUSED_RECV_TIMEOUT_US);
            rtmp->set_send_timeout(BRS_PAUSED_SEND_TIMEOUT_US);
            
            brs_trace("control message(close) accept, retry stream service.");
            continue;
        }
        
        // for other system control message, fatal error.
        brs_error("control message(%d) reject as error. ret=%d", ret, ret);
        return ret;
    }
    
    return ret;
}

int BRSClientWorker::stream_service_cycle()
{
    int ret = ERROR_SUCCESS;
        
    BrsRtmpConnType type;
    if ((ret = rtmp->identify_client(res->stream_id, type, req->stream, req->duration)) != ERROR_SUCCESS) {
        if (!brs_is_client_gracefully_close(ret)) {
            brs_error("identify client failed. ret=%d", ret);
        }
        return ret;
    }
    req->strip();
    brs_trace("client identified, type=%s, stream_name=%s, duration=%.2f", 
        brs_client_type_string(type).c_str(), req->stream.c_str(), req->duration);
    
    BRSSource * source = BRSSource::fetch(req);
    
    if(!source )
    {
      if((ret=BRSSource::create(req,&source))!=ERROR_SUCCESS)
      {
	  return ret;
      }
    }
    
    switch(type)
    {
      case BrsRtmpConnPlay:{
	brs_verbose("start to play stream %s.", req->stream.c_str());
            
            // response connection start play
            if ((ret = rtmp->start_play(res->stream_id)) != ERROR_SUCCESS) {
                brs_error("start to play stream failed. ret=%d", ret);
                return ret;
            }
         
	 ret = playing(source);
	 return ret;
	
      }
      case BrsRtmpConnFMLEPublish:{
	brs_verbose("FMLE start to publish stream %s.", req->stream.c_str());
            
            if ((ret = rtmp->start_fmle_publish(res->stream_id)) != ERROR_SUCCESS) {
                brs_error("start to publish stream failed. ret=%d", ret);
                return ret;
            }
            return publish(source);
	
      }
      case BrsRtmpConnFlashPublish:{
	   brs_verbose("flash start to publish stream %s.", req->stream.c_str());
            
            if ((ret = rtmp->start_flash_publish(res->stream_id)) != ERROR_SUCCESS) {
                brs_error("flash start to publish stream failed. ret=%d", ret);
                return ret;
            }
            return publish(source);
      }
      default: {
            ret = ERROR_SYSTEM_CLIENT_INVALID;
            brs_info("invalid client type=%d. ret=%d", type, ret);
            return ret;
        }
    }
    
    return ret;
}

int BRSClientWorker::publish(BRSSource* source)
{
      int ret = ERROR_SUCCESS;
      BrsCommonMessage * msg = NULL;
      //brsServer->addEpoll(this->mContext.client_socketfd,EPOLLIN);
      while(true){
      if((ret=rtmp->recv_message(&msg))!=ERROR_SUCCESS)
      {
	SafeDelete(msg);
	return ret;
      }
      if(msg->header.is_audio())
      {
	if((ret=source->onAudio(msg)) != ERROR_SUCCESS)
	{
	  brs_error("source process audio message failed. ret=%d", ret);
	  return ret;
	}
      }
      
      if(msg->header.is_video())
      {
	if((ret = source->onVideo(msg))!=ERROR_SUCCESS)
	{
	  brs_error("source process video message failed. ret=%d", ret);
	  return ret;
	}
      }
      // process onMetaData
    if (msg->header.is_amf0_data() || msg->header.is_amf3_data()) {
        BrsPacket* pkt = NULL;
        if ((ret = rtmp->decode_message(msg, &pkt)) != ERROR_SUCCESS) {
            brs_error("decode onMetaData message failed. ret=%d", ret);
            return ret;
        }
        BrsAutoFreeE(BrsPacket, pkt);
        if (dynamic_cast<BrsOnMetaDataPacket*>(pkt)) {
            BrsOnMetaDataPacket* metadata = dynamic_cast<BrsOnMetaDataPacket*>(pkt);
            if ((ret = source->onMetaData(msg, metadata)) != ERROR_SUCCESS) {
                brs_error("source process onMetaData message failed. ret=%d", ret);
                return ret;
            }
            brs_info("process onMetaData message success.");
        }
    }
      source->forwardAll();
      
      coroutine_yield(this->mContext.menv);
      }
      return ret;
}

int BRSClientWorker::playing(BRSSource* source)
{
    int ret = ERROR_SUCCESS;
    BRSConsumer *consumer = new BRSConsumer(source,this->mContext.client_socketfd,rtmp,res);
    
    source->pushConsumer(consumer);
    coroutine_yield(this->mContext.menv);
   BrsCommonMessage * msg = NULL;
    while(true)
    {
      if((ret=rtmp->recv_message(&msg))!=ERROR_SUCCESS)
      {
	SafeDelete(msg);
	return ret;
      }
      ret=do_playing(consumer,msg);
      source->delConsumer(consumer);
      return ret;
    }
    
    return 0;
}

int BRSClientWorker::do_playing(BRSConsumer * consumer,BrsCommonMessage *msg)
{
      int ret = ERROR_SUCCESS;
    
    if (!msg) {
        brs_verbose("ignore all empty message.");
        return ret;
    }
    BrsAutoFreeE(BrsCommonMessage, msg);
    
    if (!msg->header.is_amf0_command() && !msg->header.is_amf3_command()) {
        brs_info("ignore all message except amf0/amf3 command.");
        return ret;
    }
    
    BrsPacket* pkt = NULL;
    if ((ret = rtmp->decode_message(msg, &pkt)) != ERROR_SUCCESS) {
        brs_error("decode the amf0/amf3 command packet failed. ret=%d", ret);
        return ret;
    }
    brs_info("decode the amf0/amf3 command packet success.");
    
    BrsAutoFreeE(BrsPacket, pkt);
    
    // for jwplayer/flowplayer, which send close as pause message.
    // @see https://github.com/ossrs/srs/issues/6
    BrsCloseStreamPacket* close = dynamic_cast<BrsCloseStreamPacket*>(pkt);
    if (close) {
        ret = ERROR_CONTROL_RTMP_CLOSE;
        brs_trace("system control message: rtmp close stream. ret=%d", ret);
        return ret;
    }
    
    // call msg,
    // support response null first,
    // @see https://github.com/ossrs/srs/issues/106
    // TODO: FIXME: response in right way, or forward in edge mode.
    BrsCallPacket* call = dynamic_cast<BrsCallPacket*>(pkt);
    if (call) {
        // only response it when transaction id not zero,
        // for the zero means donot need response.
        if (call->transaction_id > 0) {
            BrsCallResPacket* res = new BrsCallResPacket(call->transaction_id);
            res->command_object = BrsAmf0Any::null();
            res->response = BrsAmf0Any::null();
            if ((ret = rtmp->send_and_free_packet(res, 0)) != ERROR_SUCCESS) {
                if (!brs_is_system_control_error(ret) && !brs_is_client_gracefully_close(ret)) {
                    brs_warn("response call failed. ret=%d", ret);
                }
                return ret;
            }
        }
        return ret;
    }
    
    // pause
    BrsPausePacket* pause = dynamic_cast<BrsPausePacket*>(pkt);
    if (pause) {
        if ((ret = rtmp->on_play_client_pause(res->stream_id, pause->is_pause)) != ERROR_SUCCESS) {
            brs_error("rtmp process play client pause failed. ret=%d", ret);
            return ret;
        }

        if ((ret = consumer->on_play_client_pause(pause->is_pause)) != ERROR_SUCCESS) {
            brs_error("consumer process play client pause failed. ret=%d", ret);
            return ret;
        }
        brs_info("process pause success, is_pause=%d, time=%d.", pause->is_pause, pause->time_ms);
        return ret;
    }
    
    // other msg.
    brs_info("ignore all amf0/amf3 command except pause and video control.");
    return ret;
}


  
}