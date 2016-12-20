#include <BRSRtmpUtility.h>

// for brs-librtmp, @see https://github.com/osbrs/brs/issues/213
#ifndef _WIN32
#include <unistd.h>
#endif

#include <stdlib.h>
using namespace std;

#include <BRSLog.h>
#include <BRSKernelUtility.h>
#include <BRSStream.h>
#include <BRSRtmpStack.h>
#include <BRSKernelCodec.h>
#include <BRSConsts.h>
#include <BRSKernelError.h>
#include <BRSReadWriter.h>

namespace BRS {


void brs_discovery_tc_url(
    string tcUrl, 
    string& schema, string& host, string& vhost, 
    string& app, string& port, std::string& param
) {
    size_t pos = std::string::npos;
    std::string url = tcUrl;
    
    if ((pos = url.find("://")) != std::string::npos) {
        schema = url.substr(0, pos);
        url = url.substr(schema.length() + 3);
        brs_info("discovery schema=%s", schema.c_str());
    }
    
    if ((pos = url.find("/")) != std::string::npos) {
        host = url.substr(0, pos);
        url = url.substr(host.length() + 1);
        brs_info("discovery host=%s", host.c_str());
    }

    port = BRS_CONSTS_RTMP_DEFAULT_PORT;
    if ((pos = host.find(":")) != std::string::npos) {
        port = host.substr(pos + 1);
        host = host.substr(0, pos);
        brs_info("discovery host=%s, port=%s", host.c_str(), port.c_str());
    }
    
    app = url;
    vhost = host;
    brs_vhost_resolve(vhost, app, param);
}

void brs_vhost_resolve(string& vhost, string& app, string& param)
{
    // get original param
    size_t pos = 0;
    if ((pos = app.find("?")) != std::string::npos) {
        param = app.substr(pos);
    }
    
    // filter tcUrl
    app = brs_string_replace(app, ",", "?");
    app = brs_string_replace(app, "...", "?");
    app = brs_string_replace(app, "&&", "?");
    app = brs_string_replace(app, "=", "?");
    
    if ((pos = app.find("?")) != std::string::npos) {
        std::string query = app.substr(pos + 1);
        app = app.substr(0, pos);
        
        if ((pos = query.find("vhost?")) != std::string::npos) {
            query = query.substr(pos + 6);
            if (!query.empty()) {
                vhost = query;
            }
            if ((pos = vhost.find("?")) != std::string::npos) {
                vhost = vhost.substr(0, pos);
            }
        }
    }
    
    /* others */
}

void brs_random_generate(char* bytes, int size)
{
    static bool _random_initialized = false;
    if (!_random_initialized) {
        srand(0);
        _random_initialized = true;
        brs_trace("srand initialized the random.");
    }
    
    for (int i = 0; i < size; i++) {
        // the common value in [0x0f, 0xf0]
        bytes[i] = 0x0f + (rand() % (256 - 0x0f - 0x0f));
    }
}

string brs_generate_tc_url(string ip, string vhost, string app, string port, string param)
{
    string tcUrl = "rtmp://";
    
    if (vhost == BRS_CONSTS_RTMP_DEFAULT_VHOST) {
        tcUrl += ip;
    } else {
        tcUrl += vhost;
    }
    
    if (port != BRS_CONSTS_RTMP_DEFAULT_PORT) {
        tcUrl += ":";
        tcUrl += port;
    }
    
    tcUrl += "/";
    tcUrl += app;
    tcUrl += param;
    
    return tcUrl;
}

/**
* compare the memory in bytes.
*/
// bool brs_bytes_equals(void* pa, void* pb, int size)
// {
//     u_int8_t* a = (u_int8_t*)pa;
//     u_int8_t* b = (u_int8_t*)pb;
//     
//     if (!a && !b) {
//         return true;
//     }
//     
//     if (!a || !b) {
//         return false;
//     }
//     
//     for(int i = 0; i < size; i++){
//         if(a[i] != b[i]){
//             return false;
//         }
//     }
// 
//     return true;
// }

int brs_do_rtmp_create_msg(char type, u_int32_t timestamp, char* data, int size, int stream_id, BrsSharedPtrMessage** ppmsg)
{
    int ret = ERROR_SUCCESS;
    
    *ppmsg = NULL;
    BrsSharedPtrMessage* msg = NULL;
    
    if (type == BrsCodecFlvTagAudio) {
        BrsMessageHeader header;
        header.initialize_audio(size, timestamp, stream_id);
        
        msg = new BrsSharedPtrMessage();
        if ((ret = msg->create(&header, data, size)) != ERROR_SUCCESS) {
            SafeDelete(msg);
            return ret;
        }
    } else if (type == BrsCodecFlvTagVideo) {
        BrsMessageHeader header;
        header.initialize_video(size, timestamp, stream_id);
        
        msg = new BrsSharedPtrMessage();
        if ((ret = msg->create(&header, data, size)) != ERROR_SUCCESS) {
            SafeDelete(msg);
            return ret;
        }
    } else if (type == BrsCodecFlvTagScript) {
        BrsMessageHeader header;
        header.initialize_amf0_script(size, stream_id);
        
        msg = new BrsSharedPtrMessage();
        if ((ret = msg->create(&header, data, size)) != ERROR_SUCCESS) {
            SafeDelete(msg);
            return ret;
        }
    } else {
        ret = ERROR_STREAM_CASTER_FLV_TAG;
        brs_error("rtmp unknown tag type=%#x. ret=%d", type, ret);
        return ret;
    }

    *ppmsg = msg;

    return ret;
}

int brs_rtmp_create_msg(char type, u_int32_t timestamp, char* data, int size, int stream_id, BrsSharedPtrMessage** ppmsg)
{
    int ret = ERROR_SUCCESS;

    // only when failed, we must free the data.
    if ((ret = brs_do_rtmp_create_msg(type, timestamp, data, size, stream_id, ppmsg)) != ERROR_SUCCESS) {
        SafeDeleteArray(data);
        return ret;
    }

    return ret;
}

std::string brs_generate_stream_url(std::string vhost, std::string app, std::string stream) 
{
    std::string url = "";
    
    if (BRS_CONSTS_RTMP_DEFAULT_VHOST != vhost){
    	url += vhost;
    }
    url += "/";
    url += app;
    url += "/";
    url += stream;

    return url;
}

int brs_write_large_iovs(BRSReadWriter* skt, iovec* iovs, int size, ssize_t* pnwrite)
{
    int ret = ERROR_SUCCESS;
    
    // the limits of writev iovs.
    // for brs-librtmp, @see https://github.com/osbrs/brs/issues/213
#ifndef _WIN32
    // for linux, generally it's 1024.
    static int limits = (int)sysconf(_SC_IOV_MAX);
#else
    static int limits = 1024;
#endif
    
    // send in a time.
    if (size < limits) {
        if ((ret = skt->writen(iovs, size, pnwrite)) != ERROR_SUCCESS) {
            if (!brs_is_client_gracefully_close(ret)) {
                brs_error("send with writev failed. ret=%d", ret);
            }
            return ret;
        }
        return ret;
    }
    
    // send in multiple times.
    int cur_iov = 0;
    while (cur_iov < size) {
        int cur_count = brs_min(limits, size - cur_iov);
        if ((ret = skt->writen(iovs + cur_iov, cur_count, pnwrite)) != ERROR_SUCCESS) {
            if (!brs_is_client_gracefully_close(ret)) {
                brs_error("send with writev failed. ret=%d", ret);
            }
            return ret;
        }
        cur_iov += cur_count;
    }
    
    return ret;
}

}