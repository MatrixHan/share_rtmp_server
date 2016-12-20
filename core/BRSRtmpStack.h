#pragma once 

#include <BRSLog.h>
#include <BRSCommon.h>
#include <BRSAutoFree.h>
#include <BRSStream.h>
#include <BRSFlv.h>

namespace BRS
{
  
class BrsMessageHeader;
class BrsCommonMessage;
  /****************************************************************************
 *****************************************************************************
 ****************************************************************************/
/**
 * amf0 command message, command name macros
 */
#define RTMP_AMF0_COMMAND_CONNECT               "connect"
#define RTMP_AMF0_COMMAND_CREATE_STREAM         "createStream"
#define RTMP_AMF0_COMMAND_CLOSE_STREAM          "closeStream"
#define RTMP_AMF0_COMMAND_PLAY                  "play"
#define RTMP_AMF0_COMMAND_PAUSE                 "pause"
#define RTMP_AMF0_COMMAND_ON_BW_DONE            "onBWDone"
#define RTMP_AMF0_COMMAND_ON_STATUS             "onStatus"
#define RTMP_AMF0_COMMAND_RESULT                "_result"
#define RTMP_AMF0_COMMAND_ERROR                 "_error"
#define RTMP_AMF0_COMMAND_RELEASE_STREAM        "releaseStream"
#define RTMP_AMF0_COMMAND_FC_PUBLISH            "FCPublish"
#define RTMP_AMF0_COMMAND_UNPUBLISH             "FCUnpublish"
#define RTMP_AMF0_COMMAND_PUBLISH               "publish"
#define RTMP_AMF0_DATA_SAMPLE_ACCESS            "|RtmpSampleAccess"

/**
 * the signature for packets to client.
 */
#define RTMP_SIG_FMS_VER                        "3,5,3,888"
#define RTMP_SIG_AMF0_VER                       0
#define RTMP_SIG_CLIENT_ID                      "ASAICiss"

/**
 * onStatus consts.
 */
#define StatusLevel                             "level"
#define StatusCode                              "code"
#define StatusDescription                       "description"
#define StatusDetails                           "details"
#define StatusClientId                          "clientid"
// status value
#define StatusLevelStatus                       "status"
// status error
#define StatusLevelError                        "error"
// code value
#define StatusCodeConnectSuccess                "NetConnection.Connect.Success"
#define StatusCodeConnectRejected               "NetConnection.Connect.Rejected"
#define StatusCodeStreamReset                   "NetStream.Play.Reset"
#define StatusCodeStreamStart                   "NetStream.Play.Start"
#define StatusCodeStreamPause                   "NetStream.Pause.Notify"
#define StatusCodeStreamUnpause                 "NetStream.Unpause.Notify"
#define StatusCodePublishStart                  "NetStream.Publish.Start"
#define StatusCodeDataStart                     "NetStream.Data.Start"
#define StatusCodeUnpublishSuccess              "NetStream.Unpublish.Success"

/****************************************************************************
*****************************************************************************
****************************************************************************/
  

class BrsPacket
{
public:
    BrsPacket();
    virtual ~BrsPacket();
public:
    /**
     * the subpacket can override this encode,
     * for example, video and audio will directly set the payload withou memory copy,
     * other packet which need to serialize/encode to bytes by override the
     * get_size and encode_packet.
     */
    virtual int encode(int& size, char*& payload);
    // decode functions for concrete packet to override.
public:
    /**
     * subpacket must override to decode packet from stream.
     * @remark never invoke the super.decode, it always failed.
     */
    virtual int decode(BRSStream* stream);
    // encode functions for concrete packet to override.
public:
    /**
     * the cid(chunk id) specifies the chunk to send data over.
     * generally, each message perfer some cid, for example,
     * all protocol control messages perfer RTMP_CID_ProtocolControl,
     * BrsSetWindowAckSizePacket is protocol control message.
     */
    virtual int get_prefer_cid();
    /**
     * subpacket must override to provide the right message type.
     * the message type set the RTMP message type in header.
     */
    virtual int get_message_type();
protected:
    /**
     * subpacket can override to calc the packet size.
     */
    virtual int get_size();
    /**
     * subpacket can override to encode the payload to stream.
     * @remark never invoke the super.encode_packet, it always failed.
     */
    virtual int encode_packet(BRSStream* stream);
};

class BrsChunkStream
{
public:
    /**
     * represents the basic header fmt,
     * which used to identify the variant message header type.
     */
    char fmt;
    /**
     * represents the basic header cid,
     * which is the chunk stream id.
     */
    int cid;
    /**
     * cached message header
     */
    BrsMessageHeader header;
    /**
     * whether the chunk message header has extended timestamp.
     */
    bool extended_timestamp;
    /**
     * partially read message.
     */
    BrsCommonMessage* msg;
    /**
     * decoded msg count, to identify whether the chunk stream is fresh.
     */
    int64_t msg_count;
public:
    BrsChunkStream(int _cid);
    virtual ~BrsChunkStream();
};

class BrsAmf0Object;
class BrsRequest
{
public:
    // client ip.
    std::string ip;
public:
    /**
     * tcUrl: rtmp://request_vhost:port/app/stream
     * support pass vhost in query string, such as:
     *    rtmp://ip:port/app?vhost=request_vhost/stream
     *    rtmp://ip:port/app...vhost...request_vhost/stream
     */
    std::string tcUrl;
    std::string pageUrl;
    std::string swfUrl;
    double objectEncoding;
    // data discovery from request.
public:
    // discovery from tcUrl and play/publish.
    std::string schema;
    // the vhost in tcUrl.
    std::string vhost;
    // the host in tcUrl.
    std::string host;
    // the port in tcUrl.
    std::string port;
    // the app in tcUrl, without param.
    std::string app;
    // the param in tcUrl(app).
    std::string param;
    // the stream in play/publish
    std::string stream;
    // for play live stream,
    // used to specified the stop when exceed the duration.
    // @see https://github.com/ossrs/srs/issues/45
    // in ms.
    double duration;
    // the token in the connect request,
    // used for edge traverse to origin authentication,
    // @see https://github.com/ossrs/srs/issues/104
    BrsAmf0Object* args;
public:
    BrsRequest();
    virtual ~BrsRequest();
public:
    /**
     * deep copy the request, for source to use it to support reload,
     * for when initialize the source, the request is valid,
     * when reload it, the request maybe invalid, so need to copy it.
     */
    virtual BrsRequest* copy();
    /**
     * update the auth info of request,
     * to keep the current request ptr is ok,
     * for many components use the ptr of request.
     */
    virtual void update_auth(BrsRequest* req);
    /**
     * get the stream identify, vhost/app/stream.
     */
    virtual std::string get_stream_url();
    /**
     * strip url, user must strip when update the url.
     */
    virtual void strip();
};

/**
 * the response to client.
 */
class BrsResponse
{
public:
    /**
     * the stream id to response client createStream.
     */
    int stream_id;
public:
    BrsResponse();
    virtual ~BrsResponse();
};

/**
 * the rtmp client type.
 */
enum BrsRtmpConnType
{
    BrsRtmpConnUnknown,
    BrsRtmpConnPlay,
    BrsRtmpConnFMLEPublish,
    BrsRtmpConnFlashPublish,
};
std::string brs_client_type_string(BrsRtmpConnType type);
bool brs_client_type_is_publish(BrsRtmpConnType type);





}