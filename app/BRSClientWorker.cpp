#include <BRSClientWorker.h>
#include <BRSServer.h>
#include <BRSErrorDef.h>

namespace BRS 
{


BRSClientWorker::BRSClientWorker(int pfd,BRSServer *mserver):BRSWorker(mserver)
{
      protocol = new BRSProtocol(pfd);
      complexHandshake = new BRSComplexHandShake();
}


  
BRSClientWorker::~BRSClientWorker()
{

}



void BRSClientWorker::do_something()
{
      
     int ret = ERROR_SUCCESS;
     ret=this->rtmpHandshake();
     if(ret<0){
       close(this->mContext.client_socketfd);
       this->brsServer->closeClient(this->mContext.client_socketfd);
       return;
    }

    
     coroutine_yield(this->mContext.menv);
     close(this->mContext.client_socketfd);
     this->brsServer->closeClient(this->mContext.client_socketfd);
}

int BRSClientWorker::rtmpHandshake()
{
    int ret = ERROR_SUCCESS;
	
    ssize_t nsize;
    BRSReadWriter skt(this->mContext.client_socketfd);
    
    char* c0c1 = new char[1537];
    BrsAutoFree(char, c0c1, true);
    if ((ret = skt.readn(c0c1, 1537, &nsize)) != ERROR_SUCCESS) {
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
    if ((ret = skt.writen(s0s1s2, 3073, &nsize)) != ERROR_SUCCESS) {
        brs_warn("simple handshake send s0s1s2 failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("simple handshake send s0s1s2 success.");
    
    char* c2 = new char[1536];
    BrsAutoFree(char, c2, true);
    if ((ret = skt.readn(c2, 1536, &nsize)) != ERROR_SUCCESS) {
        brs_warn("simple handshake read c2 failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("simple handshake read c2 success.");
    
    brs_trace("simple handshake success.");
    
	return ret;
}

int BRSClientWorker::connect_app()
{

}

  
}