#include <BRSRtmpStack.h>

#include <BRSErrorDef.h>
#include <BRSKernelUtility.h>
#include <BRSRtmpUtility.h>
#include <BRSRtmpAmf0.h>
#include <BRSFlv.h>

using namespace std;


namespace BRS 
{
  
  
  
BrsPacket::BrsPacket()
{
}

BrsPacket::~BrsPacket()
{
}

int BrsPacket::encode(int& psize, char*& ppayload)
{
    int ret = ERROR_SUCCESS;
    
    int size = get_size();
    char* payload = NULL;
    
    BRSStream stream;
    
    if (size > 0) {
        payload = new char[size];
        
        if ((ret = stream.initialize(payload, size)) != ERROR_SUCCESS) {
            brs_error("initialize the stream failed. ret=%d", ret);
            SafeDeleteArray(payload);
            return ret;
        }
    }
    
    if ((ret = encode_packet(&stream)) != ERROR_SUCCESS) {
        brs_error("encode the packet failed. ret=%d", ret);
        SafeDeleteArray(payload);
        return ret;
    }
    
    psize = size;
    ppayload = payload;
    brs_verbose("encode the packet success. size=%d", size);
    
    return ret;
}

int BrsPacket::decode(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    assert(stream != NULL);
    
    ret = ERROR_SYSTEM_PACKET_INVALID;
    brs_error("current packet is not support to decode. ret=%d", ret);
    
    return ret;
}

int BrsPacket::get_prefer_cid()
{
    return 0;
}

int BrsPacket::get_message_type()
{
    return 0;
}

int BrsPacket::get_size()
{
    return 0;
}

int BrsPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    assert(stream != NULL);
    
    ret = ERROR_SYSTEM_PACKET_INVALID;
    brs_error("current packet is not support to encode. ret=%d", ret);
    
    return ret;
}
  
  
  
  
BrsChunkStream::BrsChunkStream(int _cid)
{
    fmt = 0;
    cid = _cid;
    extended_timestamp = false;
    msg = NULL;
    msg_count = 0;
}

BrsChunkStream::~BrsChunkStream()
{
    SafeDelete(msg);
}
  
  
  
  
  BrsRequest::BrsRequest()
{
    objectEncoding = RTMP_SIG_AMF0_VER;
    duration = -1;
    args = NULL;
}

BrsRequest::~BrsRequest()
{
    SafeDeleteArray(args);
}

BrsRequest* BrsRequest::copy()
{
    BrsRequest* cp = new BrsRequest();
    
    cp->ip = ip;
    cp->app = app;
    cp->objectEncoding = objectEncoding;
    cp->pageUrl = pageUrl;
    cp->host = host;
    cp->port = port;
    cp->param = param;
    cp->schema = schema;
    cp->stream = stream;
    cp->swfUrl = swfUrl;
    cp->tcUrl = tcUrl;
    cp->vhost = vhost;
    cp->duration = duration;
    if (args) {
        cp->args = args->copy()->to_object();
    }
    
    return cp;
}

void BrsRequest::update_auth(BrsRequest* req)
{
    pageUrl = req->pageUrl;
    swfUrl = req->swfUrl;
    tcUrl = req->tcUrl;
    
    if (args) {
        SafeDelete(args);
    }
    if (req->args) {
        args = req->args->copy()->to_object();
    }
    
    brs_info("update req of soruce for auth ok");
}

string BrsRequest::get_stream_url()
{
    return brs_generate_stream_url(vhost, app, stream);
}

void BrsRequest::strip()
{
    // remove the unsupported chars in names.
    host = brs_string_remove(host, "/ \n\r\t");
    vhost = brs_string_remove(vhost, "/ \n\r\t");
    app = brs_string_remove(app, " \n\r\t");
    stream = brs_string_remove(stream, " \n\r\t");
    
    // remove end slash of app/stream
    app = brs_string_trim_end(app, "/");
    stream = brs_string_trim_end(stream, "/");
    
    // remove start slash of app/stream
    app = brs_string_trim_start(app, "/");
    stream = brs_string_trim_start(stream, "/");
}
 

BrsResponse::BrsResponse()
{
    stream_id = 1;
}

BrsResponse::~BrsResponse()
{
}
 
}