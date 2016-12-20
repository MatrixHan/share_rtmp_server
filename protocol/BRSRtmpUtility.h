#pragma once

#include <BRSCommon.h>

#ifndef _WIN32
#include <sys/uio.h>
#endif

#include <string>

#include <BRSConsts.h>

namespace BRS {

class BrsMessageHeader;
class BrsSharedPtrMessage;
class BRSReadWriter;
/**
* parse the tcUrl, output the schema, host, vhost, app and port.
* @param tcUrl, the input tcUrl, for example, 
*       rtmp://192.168.1.10:19350/live?vhost=vhost.osbrs.net
* @param schema, for example, rtmp
* @param host, for example, 192.168.1.10
* @param vhost, for example, vhost.osbrs.net.
*       vhost default to host, when user not set vhost in query of app.
* @param app, for example, live
* @param port, for example, 19350
*       default to 1935 if not specified.
* param param, for example, vhost=vhost.osbrs.net
*/
extern void brs_discovery_tc_url(
    std::string tcUrl, 
    std::string& schema, std::string& host, std::string& vhost, 
    std::string& app, std::string& port, std::string& param
);

/**
* resolve the vhost in query string
* @pram vhost, update the vhost if query contains the vhost.
* @param app, may contains the vhost in query string format:
*   app?vhost=request_vhost
*   app...vhost...request_vhost
* @param param, the query, for example, ?vhost=xxx
*/ 
extern void brs_vhost_resolve(
    std::string& vhost, std::string& app, std::string& param
);

/**
* generate ramdom data for handshake.
*/
extern void brs_random_generate(char* bytes, int size);

/**
* generate the tcUrl.
* @param param, the app parameters in tcUrl. for example, ?key=xxx,vhost=xxx
* @return the tcUrl generated from ip/vhost/app/port.
* @remark when vhost equals to __defaultVhost__, use ip as vhost.
* @remark ignore port if port equals to default port 1935.
*/
extern std::string brs_generate_tc_url(
    std::string ip, std::string vhost, std::string app, std::string port,
    std::string param
);

/**
* compare the memory in bytes.
* @return true if completely equal; otherwise, false.
*/
extern bool brs_bytes_equals(void* pa, void* pb, int size);

/**
* create shared ptr message from bytes.
* @param data the packet bytes. user should never free it.
* @param ppmsg output the shared ptr message. user should free it.
*/
extern int brs_rtmp_create_msg(char type, u_int32_t timestamp, char* data, int size, int stream_id, BrsSharedPtrMessage** ppmsg);

// get the stream identify, vhost/app/stream.
extern std::string brs_generate_stream_url(std::string vhost, std::string app, std::string stream);

// write large numbers of iovs.
extern int brs_write_large_iovs(BRSReadWriter* skt, iovec* iovs, int size, ssize_t* pnwrite = NULL);

}