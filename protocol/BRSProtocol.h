#pragma once 

#include <BRSCommon.h>
#include <BRSStream.h>
#include <BRSReadWriter.h>
#include <BRSConsts.h>
#include <BRSKernelError.h>
#include <BRSLog.h>
#include <BRSRtmpStack.h>


namespace BRS 
{
  
#define CLASS_NAME_STRING(className) #className

/**
* max rtmp header size:
* 	1bytes basic header,
* 	11bytes message header,
* 	4bytes timestamp header,
* that is, 1+11+4=16bytes.
*/
#define RTMP_MAX_FMT0_HEADER_SIZE 16
/**
* max rtmp header size:
* 	1bytes basic header,
* 	4bytes timestamp header,
* that is, 1+4=5bytes.
*/
#define RTMP_MAX_FMT3_HEADER_SIZE 5

/**
* the protocol provides the rtmp-message-protocol services,
* to recv RTMP message from RTMP chunk stream,
* and to send out RTMP message over RTMP chunk stream.
*/
class IMergeReadHandler;
class BrsFastBuffer;
class BRSProtocol
  {
    private:
    class AckWindowSize
    {
    public:
        int ack_window_size;
        int64_t acked_size;
        
        AckWindowSize();
    };
  private:
    BRSReadWriter  *skt;
    
    std::map<double, std::string> requests;
    
    std::map<int, BrsChunkStream*> chunk_streams;
    
    BrsChunkStream** cs_cache;
    
    BrsFastBuffer*   in_buffer;
    
    /**
    * input chunk size, default to 128, set by peer packet.
    */
    int32_t in_chunk_size;
    /**
    * input ack size, when to send the acked packet.
    */
    AckWindowSize in_ack_size;
    
    /**
    * whether auto response when recv messages.
    * default to true for it's very easy to use the protocol stack.
    * @see: https://github.com/osbrs/brs/issues/217
    */
    bool auto_response_when_recv;
    /**
    * when not auto response message, manual flush the messages in queue.
    */
    std::vector<BrsPacket*> manual_response_queue;
// peer out
private:
    /**
    * cache for multiple messages send,
    * initialize to iovec[SRS_CONSTS_IOVS_MAX] and realloc when consumed,
    * it's ok to realloc the iovs cache, for all ptr is ok.
    */
    iovec* out_iovs;
    int nb_out_iovs;
    /**
    * output header cache.
    * used for type0, 11bytes(or 15bytes with extended timestamp) header.
    * or for type3, 1bytes(or 5bytes with extended timestamp) header.
    * the c0c3 caches must use unit SRS_CONSTS_RTMP_MAX_FMT0_HEADER_SIZE bytes.
    * 
    * @remark, the c0c3 cache cannot be realloc.
    */
    char out_c0c3_caches[BRS_CONSTS_C0C3_HEADERS_MAX];
    // whether warned user to increase the c0c3 header cache.
    bool warned_c0c3_cache_dry;
    /**
    * output chunk size, default to 128, set by config.
    */
    int32_t out_chunk_size;
  public:
      BRSProtocol(BRSReadWriter* pskt);
      virtual ~BRSProtocol();
public:
    /**
    * set the auto response message when recv for protocol stack.
    * @param v, whether auto response message when recv message.
    * @see: https://github.com/osbrs/brs/issues/217
    */
    virtual void set_auto_response(bool v);
    /**
    * flush for manual response when the auto response is disabled
    * by set_auto_response(false), we default use auto response, so donot
    * need to call this api(the protocol sdk will auto send message).
    * @see the auto_response_when_recv and manual_response_queue.
    */
    virtual int manual_response_flush();
public:
#ifdef SRS_PERF_MERGED_READ
    /**
    * to improve read performance, merge some packets then read,
    * when it on and read small bytes, we sleep to wait more data.,
    * that is, we merge some data to read together.
    * @param v true to ename merged read.
    * @param handler the handler when merge read is enabled.
    * @see https://github.com/osbrs/brs/issues/241
    */
    virtual void set_merge_read(bool v, IMergeReadHandler* handler);
    /**
    * create buffer with specifeid size.
    * @param buffer the size of buffer.
    * @remark when MR(SRS_PERF_MERGED_READ) disabled, always set to 8K.
    * @remark when buffer changed, the previous ptr maybe invalid.
    * @see https://github.com/osbrs/brs/issues/241
    */
    virtual void set_recv_buffer(int buffer_size);
#endif
public:
    /**
    * set/get the recv timeout in us.
    * if timeout, recv/send message return ERROR_SOCKET_TIMEOUT.
    */
    virtual void set_recv_timeout(int64_t timeout_us);
    virtual int64_t get_recv_timeout();
    /**
    * set/get the send timeout in us.
    * if timeout, recv/send message return ERROR_SOCKET_TIMEOUT.
    */
    virtual void set_send_timeout(int64_t timeout_us);
    virtual int64_t get_send_timeout();
    /**
    * get recv/send bytes.
    */
    virtual int64_t get_recv_bytes();
    virtual int64_t get_send_bytes();
public:
    /**
    * recv a RTMP message, which is bytes oriented.
    * user can use decode_message to get the decoded RTMP packet.
    * @param pmsg, set the received message, 
    *       always NULL if error, 
    *       NULL for unknown packet but return success.
    *       never NULL if decode success.
    * @remark, drop message when msg is empty or payload length is empty.
    */
    virtual int recv_message(BrsCommonMessage** pmsg);
    /**
    * decode bytes oriented RTMP message to RTMP packet,
    * @param ppacket, output decoded packet, 
    *       always NULL if error, never NULL if success.
    * @return error when unknown packet, error when decode failed.
    */
    virtual int decode_message(BrsCommonMessage* msg, BrsPacket** ppacket);
    /**
    * send the RTMP message and always free it.
    * user must never free or use the msg after this method,
    * for it will always free the msg.
    * @param msg, the msg to send out, never be NULL.
    * @param stream_id, the stream id of packet to send over, 0 for control message.
    */
    virtual int send_and_free_message(BrsSharedPtrMessage* msg, int stream_id);
    /**
    * send the RTMP message and always free it.
    * user must never free or use the msg after this method,
    * for it will always free the msg.
    * @param msgs, the msgs to send out, never be NULL.
    * @param nb_msgs, the size of msgs to send out.
    * @param stream_id, the stream id of packet to send over, 0 for control message.
    */
    virtual int send_and_free_messages(BrsSharedPtrMessage** msgs, int nb_msgs, int stream_id);
    /**
    * send the RTMP packet and always free it.
    * user must never free or use the packet after this method,
    * for it will always free the packet.
    * @param packet, the packet to send out, never be NULL.
    * @param stream_id, the stream id of packet to send over, 0 for control message.
    */
    virtual int send_and_free_packet(BrsPacket* packet, int stream_id);
public:
    /**
     * expect a specified message, drop others util got specified one.
     * @pmsg, user must free it. NULL if not success.
     * @ppacket, user must free it, which decode from payload of message. NULL if not success.
     * @remark, only when success, user can use and must free the pmsg and ppacket.
     * for example:
     *          BrsCommonMessage* msg = NULL;
     *          BrsConnectAppResPacket* pkt = NULL;
     *          if ((ret = protocol->expect_message<BrsConnectAppResPacket>(protocol, &msg, &pkt)) != ERROR_SUCCESS) {
     *              return ret;
     *          }
     *          // use then free msg and pkt
     *          SafeDelete(msg);
     *          SafeDelete(pkt);
     * user should never recv message and convert it, use this method instead.
     * if need to set timeout, use set timeout of BrsProtocol.
     */
    template<class T>
    int expect_message(BrsCommonMessage** pmsg, T** ppacket)
    {
        *pmsg = NULL;
        *ppacket = NULL;
        
        int ret = ERROR_SUCCESS;
        
        while (true) {
            BrsCommonMessage* msg = NULL;
            if ((ret = recv_message(&msg)) != ERROR_SUCCESS) {
                if (ret != ERROR_SOCKET_TIMEOUT && !brs_is_client_gracefully_close(ret)) {
                    brs_error("recv message failed. ret=%d", ret);
                }
                return ret;
            }
            brs_verbose("recv message success.");
            
            BrsPacket* packet = NULL;
            if ((ret = decode_message(msg, &packet)) != ERROR_SUCCESS) {
                brs_error("decode message failed. ret=%d", ret);
                SafeDelete(msg);
                SafeDelete(packet);
                return ret;
            }
            
            T* pkt = dynamic_cast<T*>(packet);
            if (!pkt) {
                brs_info("drop message(type=%d, size=%d, time=%"PRId64", sid=%d).", 
                    msg->header.message_type, msg->header.payload_length,
                    msg->header.timestamp, msg->header.stream_id);
                SafeDelete(msg);
                SafeDelete(packet);
                continue;
            }
            
            *pmsg = msg;
            *ppacket = pkt;
            break;
        }
        
        return ret;
    }
private:
    /**
    * send out the messages, donot free it, 
    * the caller must free the param msgs.
    */
    virtual int do_send_messages(BrsSharedPtrMessage** msgs, int nb_msgs);
    /**
    * send iovs. send multiple times if exceed limits.
    */
    virtual int do_iovs_send(iovec* iovs, int size);
    /**
    * underlayer api for send and free packet.
    */
    virtual int do_send_and_free_packet(BrsPacket* packet, int stream_id);
    /**
    * use simple algorithm to send the header and bytes.
    * @remark, for do_send_and_free_packet to send.
    */
    virtual int do_simple_send(BrsMessageHeader* mh, char* payload, int size);
    /**
    * imp for decode_message
    */
    virtual int do_decode_message(BrsMessageHeader& header, BRSStream* stream, BrsPacket** ppacket);
    /**
    * recv bytes oriented RTMP message from protocol stack.
    * return error if error occur and nerver set the pmsg,
    * return success and pmsg set to NULL if no entire message got,
    * return success and pmsg set to entire message if got one.
    */
    virtual int recv_interlaced_message(BrsCommonMessage** pmsg);
    /**
    * read the chunk basic header(fmt, cid) from chunk stream.
    * user can discovery a BrsChunkStream by cid.
    */
    virtual int read_basic_header(char& fmt, int& cid);
    /**
    * read the chunk message header(timestamp, payload_length, message_type, stream_id) 
    * from chunk stream and save to BrsChunkStream.
    */
    virtual int read_message_header(BrsChunkStream* chunk, char fmt);
    /**
    * read the chunk payload, remove the used bytes in buffer,
    * if got entire message, set the pmsg.
    */
    virtual int read_message_payload(BrsChunkStream* chunk, BrsCommonMessage** pmsg);
    /**
    * when recv message, update the context.
    */
    virtual int on_recv_message(BrsCommonMessage* msg);
    /**
    * when message sentout, update the context.
    */
    virtual int on_send_packet(BrsMessageHeader* mh, BrsPacket* packet);
private:
    /**
    * auto response the ack message.
    */
    virtual int response_acknowledgement_message();
    /**
    * auto response the ping message.
    */
    virtual int response_ping_message(int32_t timestamp);
 
};
  
/**
 * the rtmp provices rtmp-command-protocol services,
 * a high level protocol, media stream oriented services,
 * such as connect to vhost/app, play stream, get audio/video data.
 */
class BrsCreateStreamPacket;
class BrsFMLEStartPacket;
class BrsPublishPacket;
class BrsPlayPacket;
class BrsRtmpServer
{
private:
    BRSProtocol* protocol;
    BRSReadWriter * io;
public:
    BrsRtmpServer(BRSReadWriter* skt);
    virtual ~BrsRtmpServer();
    // protocol methods proxy
public:
    /**
     * set the auto response message when recv for protocol stack.
     * @param v, whether auto response message when recv message.
     * @see: https://github.com/ossrs/srs/issues/217
     */
    virtual void set_auto_response(bool v);
#ifdef SRS_PERF_MERGED_READ
    /**
     * to improve read performance, merge some packets then read,
     * when it on and read small bytes, we sleep to wait more data.,
     * that is, we merge some data to read together.
     * @param v true to ename merged read.
     * @param handler the handler when merge read is enabled.
     * @see https://github.com/ossrs/srs/issues/241
     */
    virtual void set_merge_read(bool v, IMergeReadHandler* handler);
    /**
     * create buffer with specifeid size.
     * @param buffer the size of buffer.
     * @remark when MR(SRS_PERF_MERGED_READ) disabled, always set to 8K.
     * @remark when buffer changed, the previous ptr maybe invalid.
     * @see https://github.com/ossrs/srs/issues/241
     */
    virtual void set_recv_buffer(int buffer_size);
#endif
    /**
     * set/get the recv timeout in us.
     * if timeout, recv/send message return ERROR_SOCKET_TIMEOUT.
     */
    virtual void set_recv_timeout(int64_t timeout_us);
    virtual int64_t get_recv_timeout();
    /**
     * set/get the send timeout in us.
     * if timeout, recv/send message return ERROR_SOCKET_TIMEOUT.
     */
    virtual void set_send_timeout(int64_t timeout_us);
    virtual int64_t get_send_timeout();
    /**
     * get recv/send bytes.
     */
    virtual int64_t get_recv_bytes();
    virtual int64_t get_send_bytes();
    /**
     * recv a RTMP message, which is bytes oriented.
     * user can use decode_message to get the decoded RTMP packet.
     * @param pmsg, set the received message,
     *       always NULL if error,
     *       NULL for unknown packet but return success.
     *       never NULL if decode success.
     * @remark, drop message when msg is empty or payload length is empty.
     */
    virtual int recv_message(BrsCommonMessage** pmsg);
    /**
     * decode bytes oriented RTMP message to RTMP packet,
     * @param ppacket, output decoded packet,
     *       always NULL if error, never NULL if success.
     * @return error when unknown packet, error when decode failed.
     */
    virtual int decode_message(BrsCommonMessage* msg, BrsPacket** ppacket);
    /**
     * send the RTMP message and always free it.
     * user must never free or use the msg after this method,
     * for it will always free the msg.
     * @param msg, the msg to send out, never be NULL.
     * @param stream_id, the stream id of packet to send over, 0 for control message.
     */
    virtual int send_and_free_message(BrsSharedPtrMessage* msg, int stream_id);
    /**
     * send the RTMP message and always free it.
     * user must never free or use the msg after this method,
     * for it will always free the msg.
     * @param msgs, the msgs to send out, never be NULL.
     * @param nb_msgs, the size of msgs to send out.
     * @param stream_id, the stream id of packet to send over, 0 for control message.
     *
     * @remark performance issue, to support 6k+ 250kbps client,
     *       @see https://github.com/ossrs/srs/issues/194
     */
    virtual int send_and_free_messages(BrsSharedPtrMessage** msgs, int nb_msgs, int stream_id);
    /**
     * send the RTMP packet and always free it.
     * user must never free or use the packet after this method,
     * for it will always free the packet.
     * @param packet, the packet to send out, never be NULL.
     * @param stream_id, the stream id of packet to send over, 0 for control message.
     */
    virtual int send_and_free_packet(BrsPacket* packet, int stream_id);
public:
    /**
     * do connect app with client, to discovery tcUrl.
     */
    virtual int connect_app(BrsRequest* req);
    /**
     * set ack size to client, client will send ack-size for each ack window
     */
    virtual int set_window_ack_size(int ack_size);
    /**
     * @type: The sender can mark this message hard (0), soft (1), or dynamic (2)
     * using the Limit type field.
     */
    virtual int set_peer_bandwidth(int bandwidth, int type);
    /**
     * @param server_ip the ip of server.
     */
    virtual int response_connect_app(BrsRequest* req, const char* server_ip = NULL);
    /**
     * reject the connect app request.
     */
    virtual void response_connect_reject(BrsRequest* req, const char* desc);
    /**
     * response client the onBWDone message.
     */
    virtual int on_bw_done();
    /**
     * recv some message to identify the client.
     * @stream_id, client will createStream to play or publish by flash,
     *         the stream_id used to response the createStream request.
     * @type, output the client type.
     * @stream_name, output the client publish/play stream name. @see: BrsRequest.stream
     * @duration, output the play client duration. @see: BrsRequest.duration
     */
    virtual int identify_client(int stream_id, BrsRtmpConnType& type, std::string& stream_name, double& duration);
    /**
     * set the chunk size when client type identified.
     */
    virtual int set_chunk_size(int chunk_size);
    /**
     * when client type is play, response with packets:
     * StreamBegin,
     * onStatus(NetStream.Play.Reset), onStatus(NetStream.Play.Start).,
     * |RtmpSampleAccess(false, false),
     * onStatus(NetStream.Data.Start).
     */
    virtual int start_play(int stream_id);
    /**
     * when client(type is play) send pause message,
     * if is_pause, response the following packets:
     *     onStatus(NetStream.Pause.Notify)
     *     StreamEOF
     * if not is_pause, response the following packets:
     *     onStatus(NetStream.Unpause.Notify)
     *     StreamBegin
     */
    virtual int on_play_client_pause(int stream_id, bool is_pause);
    /**
     * when client type is publish, response with packets:
     * releaseStream response
     * FCPublish
     * FCPublish response
     * createStream response
     * onFCPublish(NetStream.Publish.Start)
     * onStatus(NetStream.Publish.Start)
     */
    virtual int start_fmle_publish(int stream_id);
    /**
     * process the FMLE unpublish event.
     * @unpublish_tid the unpublish request transaction id.
     */
    virtual int fmle_unpublish(int stream_id, double unpublish_tid);
    /**
     * when client type is publish, response with packets:
     * onStatus(NetStream.Publish.Start)
     */
    virtual int start_flash_publish(int stream_id);
public:
    /**
     * expect a specified message, drop others util got specified one.
     * @pmsg, user must free it. NULL if not success.
     * @ppacket, user must free it, which decode from payload of message. NULL if not success.
     * @remark, only when success, user can use and must free the pmsg and ppacket.
     * for example:
     *          BrsCommonMessage* msg = NULL;
     *          BrsConnectAppResPacket* pkt = NULL;
     *          if ((ret = server->expect_message<BrsConnectAppResPacket>(protocol, &msg, &pkt)) != ERROR_SUCCESS) {
     *              return ret;
     *          }
     *          // use then free msg and pkt
     *          srs_freep(msg);
     *          srs_freep(pkt);
     * user should never recv message and convert it, use this method instead.
     * if need to set timeout, use set timeout of BrsProtocol.
     */
    template<class T>
    int expect_message(BrsCommonMessage** pmsg, T** ppacket)
    {
        return protocol->expect_message<T>(pmsg, ppacket);
    }
private:
    virtual int identify_create_stream_client(BrsCreateStreamPacket* req, int stream_id, BrsRtmpConnType& type, std::string& stream_name, double& duration);
    virtual int identify_fmle_publish_client(BrsFMLEStartPacket* req, BrsRtmpConnType& type, std::string& stream_name);
    virtual int identify_flash_publish_client(BrsPublishPacket* req, BrsRtmpConnType& type, std::string& stream_name);
private:
    virtual int identify_play_client(BrsPlayPacket* req, BrsRtmpConnType& type, std::string& stream_name, double& duration);
};  


}