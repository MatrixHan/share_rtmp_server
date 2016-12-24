#include <BRSProtocol.h>
#include <BRSFastBuffer.h>
#include <BRSPerformance.h>
#include <BRSRtmpUtility.h>
#include <BRSKernelUtility.h>
#include <BRSRtmpAmf0.h>
#include <BRSRtmpPackets.h>
#include <BRSAutoFree.h>

using namespace std;

namespace BRS 
{
  
BRSProtocol::AckWindowSize::AckWindowSize()
{
    ack_window_size = 0;
    acked_size = 0;
}
BRSProtocol::BRSProtocol(BRSReadWriter * pskt)
{
    in_buffer = new BrsFastBuffer();
    skt = pskt;
    
    in_chunk_size = BRS_CONSTS_RTMP_PROTOCOL_CHUNK_SIZE;
    out_chunk_size = BRS_CONSTS_RTMP_PROTOCOL_CHUNK_SIZE;
    
    nb_out_iovs = BRS_CONSTS_IOVS_MAX;
    out_iovs = (iovec*)malloc(sizeof(iovec) * nb_out_iovs);
    // each chunk consumers atleast 2 iovs
    assert(nb_out_iovs >= 2);
    
    warned_c0c3_cache_dry = false;
    auto_response_when_recv = true;
    
    cs_cache = NULL;
    if (BRS_PERF_CHUNK_STREAM_CACHE > 0) {
        cs_cache = new BrsChunkStream*[BRS_PERF_CHUNK_STREAM_CACHE];
    }
    for (int cid = 0; cid < BRS_PERF_CHUNK_STREAM_CACHE; cid++) {
        BrsChunkStream* cs = new BrsChunkStream(cid);
        // set the perfer cid of chunk,
        // which will copy to the message received.
        cs->header.perfer_cid = cid;
        
        cs_cache[cid] = cs;
    }
}

  
  
BRSProtocol::~BRSProtocol()
{
    if (true) {
        std::map<int, BrsChunkStream*>::iterator it;
        
        for (it = chunk_streams.begin(); it != chunk_streams.end(); ++it) {
            BrsChunkStream* stream = it->second;
            SafeDelete(stream);
        }
    
        chunk_streams.clear();
    }
    
    if (true) {
        std::vector<BrsPacket*>::iterator it;
        for (it = manual_response_queue.begin(); it != manual_response_queue.end(); ++it) {
            BrsPacket* pkt = *it;
            SafeDelete(pkt);
        }
        manual_response_queue.clear();
    }
    
    SafeDelete(in_buffer);
    
    // alloc by malloc, use free directly.
    if (out_iovs) {
        free(out_iovs);
        out_iovs = NULL;
    }
    
    // free all chunk stream cache.
    for (int i = 0; i < BRS_PERF_CHUNK_STREAM_CACHE; i++) {
        BrsChunkStream* cs = cs_cache[i];
        SafeDelete(cs);
    }
    SafeDeleteArray(cs_cache);
}

void BRSProtocol::set_auto_response(bool v)
{
    auto_response_when_recv = v;
}

int BRSProtocol::manual_response_flush()
{
    int ret = ERROR_SUCCESS;
    
    if(manual_response_queue.empty())
    {
      return ret;
    }
    
    std::vector<BrsPacket * >::iterator itr;
    for(itr= manual_response_queue.begin();itr!=manual_response_queue.end();)
    {
      BrsPacket * pkt = *itr;
      itr = manual_response_queue.erase(itr);
      // use underlayer api to send, donot flush again.
        if ((ret = do_send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            return ret;
        }
    }
    
    return ret;
}

#ifdef BRS_PERF_MERGED_READ
void BRSProtocol::set_merge_read(bool v, IMergeReadHandler* handler)
{
    in_buffer->set_merge_read(v, handler);
}

void BRSProtocol::set_recv_buffer(int buffer_size)
{
    in_buffer->set_buffer(buffer_size);
}
#endif

void BRSProtocol::set_recv_timeout(int64_t timeout_us)
{
    return skt->set_recv_timeout(timeout_us);
}

int64_t BRSProtocol::get_recv_timeout()
{
    return skt->get_recv_timeout();
}

void BRSProtocol::set_send_timeout(int64_t timeout_us)
{
    return skt->set_send_timeout(timeout_us);
}

int64_t BRSProtocol::get_send_timeout()
{
    return skt->get_send_timeout();
}

int64_t BRSProtocol::get_recv_bytes()
{
    return skt->get_recv_bytes();
}

int64_t BRSProtocol::get_send_bytes()
{
    return skt->get_send_bytes();
}

int BRSProtocol::recv_message(BrsCommonMessage** pmsg)
{
    *pmsg = NULL;
    
    int ret = ERROR_SUCCESS;
    
    while (true) {
        BrsCommonMessage* msg = NULL;
        
        if ((ret = recv_interlaced_message(&msg)) != ERROR_SUCCESS) {
            if (ret != ERROR_SOCKET_TIMEOUT && !brs_is_client_gracefully_close(ret)) {
                brs_error("recv interlaced message failed. ret=%d", ret);
            }
            SafeDelete(msg);
            return ret;
        }
        brs_verbose("entire msg received");
        
        if (!msg) {
            brs_info("got empty message without error.");
            continue;
        }
        
        if (msg->size <= 0 || msg->header.payload_length <= 0) {
            brs_trace("ignore empty message(type=%d, size=%d, time=%"PRId64", sid=%d).",
                msg->header.message_type, msg->header.payload_length,
                msg->header.timestamp, msg->header.stream_id);
            SafeDelete(msg);
            continue;
        }
        
        if ((ret = on_recv_message(msg)) != ERROR_SUCCESS) {
            brs_error("hook the received msg failed. ret=%d", ret);
            SafeDelete(msg);
            return ret;
        }
        
        brs_verbose("got a msg, cid=%d, type=%d, size=%d, time=%"PRId64, 
            msg->header.perfer_cid, msg->header.message_type, msg->header.payload_length, 
            msg->header.timestamp);
        *pmsg = msg;
        break;
    }
    
    return ret;
}

int BRSProtocol::decode_message(BrsCommonMessage* msg, BrsPacket** ppacket)
{
      *ppacket = NULL;
    
    int ret = ERROR_SUCCESS;
    
    assert(msg != NULL);
    assert(msg->payload != NULL);
    assert(msg->size > 0);
    
    BRSStream stream;

    // initialize the decode stream for all message,
    // it's ok for the initialize if fast and without memory copy.
    if ((ret = stream.initialize(msg->payload, msg->size)) != ERROR_SUCCESS) {
        brs_error("initialize stream failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("decode stream initialized success");
    
    // decode the packet.
    BrsPacket* packet = NULL;
    if ((ret = do_decode_message(msg->header, &stream, &packet)) != ERROR_SUCCESS) {
        SafeDelete(packet);
        return ret;
    }
    
    // set to output ppacket only when success.
    *ppacket = packet;
    
    return ret;
}

int BRSProtocol::do_send_messages(BrsSharedPtrMessage** msgs, int nb_msgs)
{
     int ret = ERROR_SUCCESS;
    
#ifdef BRS_PERF_COMPLEX_SEND
    int iov_index = 0;
    iovec* iovs = out_iovs + iov_index;
    
    int c0c3_cache_index = 0;
    char* c0c3_cache = out_c0c3_caches + c0c3_cache_index;

    // try to send use the c0c3 header cache,
    // if cache is consumed, try another loop.
    for (int i = 0; i < nb_msgs; i++) {
        BrsSharedPtrMessage* msg = msgs[i];
        
        if (!msg) {
            continue;
        }
    
        // ignore empty message.
        if (!msg->payload || msg->size <= 0) {
            brs_info("ignore empty message.");
            continue;
        }
    
        // p set to current write position,
        // it's ok when payload is NULL and size is 0.
        char* p = msg->payload;
        char* pend = msg->payload + msg->size;
        
        // always write the header event payload is empty.
        while (p < pend) {
            // always has header
            int nb_cache = BRS_CONSTS_C0C3_HEADERS_MAX - c0c3_cache_index;
            int nbh = msg->chunk_header(c0c3_cache, nb_cache, p == msg->payload);
            assert(nbh > 0);
            
            // header iov
            iovs[0].iov_base = c0c3_cache;
            iovs[0].iov_len = nbh;
            
            // payload iov
            int payload_size = brs_min(out_chunk_size, (int)(pend - p));
            iovs[1].iov_base = p;
            iovs[1].iov_len = payload_size;
            
            // consume sendout bytes.
            p += payload_size;
            
            // realloc the iovs if exceed,
            // for we donot know how many messges maybe to send entirely,
            // we just alloc the iovs, it's ok.
            if (iov_index >= nb_out_iovs - 2) {
                brs_warn("resize iovs %d => %d, max_msgs=%d", 
                    nb_out_iovs, nb_out_iovs + BRS_CONSTS_IOVS_MAX, 
                    BRS_PERF_MW_MSGS);
                    
                nb_out_iovs += BRS_CONSTS_IOVS_MAX;
                int realloc_size = sizeof(iovec) * nb_out_iovs;
                out_iovs = (iovec*)realloc(out_iovs, realloc_size);
            }
            
            // to next pair of iovs
            iov_index += 2;
            iovs = out_iovs + iov_index;

            // to next c0c3 header cache
            c0c3_cache_index += nbh;
            c0c3_cache = out_c0c3_caches + c0c3_cache_index;
            
            // the cache header should never be realloc again,
            // for the ptr is set to iovs, so we just warn user to set larger
            // and use another loop to send again.
            int c0c3_left = BRS_CONSTS_C0C3_HEADERS_MAX - c0c3_cache_index;
            if (c0c3_left < BRS_CONSTS_RTMP_MAX_FMT0_HEADER_SIZE) {
                // only warn once for a connection.
                if (!warned_c0c3_cache_dry) {
                    brs_warn("c0c3 cache header too small, recoment to %d", 
                        BRS_CONSTS_C0C3_HEADERS_MAX + BRS_CONSTS_RTMP_MAX_FMT0_HEADER_SIZE);
                    warned_c0c3_cache_dry = true;
                }
                
                // when c0c3 cache dry,
                // sendout all messages and reset the cache, then send again.
                if ((ret = do_iovs_send(out_iovs, iov_index)) != ERROR_SUCCESS) {
                    return ret;
                }
    
                // reset caches, while these cache ensure 
                // atleast we can sendout a chunk.
                iov_index = 0;
                iovs = out_iovs + iov_index;
                
                c0c3_cache_index = 0;
                c0c3_cache = out_c0c3_caches + c0c3_cache_index;
            }
        }
    }
    
    // maybe the iovs already sendout when c0c3 cache dry,
    // so just ignore when no iovs to send.
    if (iov_index <= 0) {
        return ret;
    }
    brs_info("mw %d msgs in %d iovs, max_msgs=%d, nb_out_iovs=%d",
        nb_msgs, iov_index, BRS_PERF_MW_MSGS, nb_out_iovs);

    return do_iovs_send(out_iovs, iov_index);
#else
    // try to send use the c0c3 header cache,
    // if cache is consumed, try another loop.
    for (int i = 0; i < nb_msgs; i++) {
        BrsSharedPtrMessage* msg = msgs[i];
        
        if (!msg) {
            continue;
        }
    
        // ignore empty message.
        if (!msg->payload || msg->size <= 0) {
            brs_info("ignore empty message.");
            continue;
        }
    
        // p set to current write position,
        // it's ok when payload is NULL and size is 0.
        char* p = msg->payload;
        char* pend = msg->payload + msg->size;
        
        // always write the header event payload is empty.
        while (p < pend) {
            // for simple send, send each chunk one by one
            iovec* iovs = out_iovs;
            char* c0c3_cache = out_c0c3_caches;
            int nb_cache = BRS_CONSTS_C0C3_HEADERS_MAX;
            
            // always has header
            int nbh = msg->chunk_header(c0c3_cache, nb_cache, p == msg->payload);
            assert(nbh > 0);
            
            // header iov
            iovs[0].iov_base = c0c3_cache;
            iovs[0].iov_len = nbh;
            
            // payload iov
            int payload_size = brs_min(out_chunk_size, pend - p);
            iovs[1].iov_base = p;
            iovs[1].iov_len = payload_size;
            
            // consume sendout bytes.
            p += payload_size;

            if ((ret = skt->writev(iovs, 2, NULL)) != ERROR_SUCCESS) {
                if (!brs_is_client_gracefully_close(ret)) {
                    brs_error("send packet with writev failed. ret=%d", ret);
                }
                return ret;
            }
        }
    }
    
    return ret;
#endif   
}

int BRSProtocol::do_iovs_send(iovec* iovs, int size)
{
    return brs_write_large_iovs(skt, iovs, size);
}

int BRSProtocol::do_send_and_free_packet(BrsPacket* packet, int stream_id)
{
    int ret = ERROR_SUCCESS;
    
    assert(packet);
    BrsAutoFreeE(BrsPacket, packet);
    
    int size = 0;
    char* payload = NULL;
    if ((ret = packet->encode(size, payload)) != ERROR_SUCCESS) {
        brs_error("encode RTMP packet to bytes oriented RTMP message failed. ret=%d", ret);
        return ret;
    }
    
    // encode packet to payload and size.
    if (size <= 0 || payload == NULL) {
        brs_warn("packet is empty, ignore empty message.");
        return ret;
    }
    
    // to message
    BrsMessageHeader header;
    header.payload_length = size;
    header.message_type = packet->get_message_type();
    header.stream_id = stream_id;
    header.perfer_cid = packet->get_prefer_cid();
    
    ret = do_simple_send(&header, payload, size);
    SafeDeleteArray(payload);
    if (ret == ERROR_SUCCESS) {
        ret = on_send_packet(&header, packet);
    }
    
    return ret;
}

int BRSProtocol::do_simple_send(BrsMessageHeader* mh, char* payload, int size)
{
    int ret = ERROR_SUCCESS;
    
    // we directly send out the packet,
    // use very simple algorithm, not very fast,
    // but it's ok.
    char* p = payload;
    char* end = p + size;
    char c0c3[BRS_CONSTS_RTMP_MAX_FMT0_HEADER_SIZE];
    while (p < end) {
        int nbh = 0;
        if (p == payload) {
            nbh = brs_chunk_header_c0(
                mh->perfer_cid, mh->timestamp, mh->payload_length,
                mh->message_type, mh->stream_id,
                c0c3, sizeof(c0c3));
        } else {
            nbh = brs_chunk_header_c3(
                mh->perfer_cid, mh->timestamp,
                c0c3, sizeof(c0c3));
        }
        assert(nbh > 0);;
        
        iovec iovs[2];
        iovs[0].iov_base = c0c3;
        iovs[0].iov_len = nbh;
        
        int payload_size = brs_min(end - p, out_chunk_size);
        iovs[1].iov_base = p;
        iovs[1].iov_len = payload_size;
        p += payload_size;
        
        if ((ret = skt->writev(iovs, 2, NULL)) != ERROR_SUCCESS) {
            if (!brs_is_client_gracefully_close(ret)) {
                brs_error("send packet with writev failed. ret=%d", ret);
            }
            return ret;
        }
    }
    
    return ret;
}

int BRSProtocol::do_decode_message(BrsMessageHeader& header, BRSStream* stream, BrsPacket** ppacket)
{
    int ret = ERROR_SUCCESS;
    
    BrsPacket* packet = NULL;
    
    // decode specified packet type
    if (header.is_amf0_command() || header.is_amf3_command() || header.is_amf0_data() || header.is_amf3_data()) {
        brs_verbose("start to decode AMF0/AMF3 command message.");
        
        // skip 1bytes to decode the amf3 command.
        if (header.is_amf3_command() && stream->require(1)) {
            brs_verbose("skip 1bytes to decode AMF3 command");
            stream->skip(1);
        }
        
        // amf0 command message.
        // need to read the command name.
        std::string command;
        if ((ret = brs_amf0_read_string(stream, command)) != ERROR_SUCCESS) {
            brs_error("decode AMF0/AMF3 command name failed. ret=%d", ret);
            return ret;
        }
        brs_verbose("AMF0/AMF3 command message, command_name=%s", command.c_str());
        
        // result/error packet
        if (command == RTMP_AMF0_COMMAND_RESULT || command == RTMP_AMF0_COMMAND_ERROR) {
            double transactionId = 0.0;
            if ((ret = brs_amf0_read_number(stream, transactionId)) != ERROR_SUCCESS) {
                brs_error("decode AMF0/AMF3 transcationId failed. ret=%d", ret);
                return ret;
            }
            brs_verbose("AMF0/AMF3 command id, transcationId=%.2f", transactionId);
            
            // reset stream, for header read completed.
            stream->skip(-1 * stream->pos());
            if (header.is_amf3_command()) {
                stream->skip(1);
            }
            
            // find the call name
            if (requests.find(transactionId) == requests.end()) {
                ret = ERROR_RTMP_NO_REQUEST;
                brs_error("decode AMF0/AMF3 request failed. ret=%d", ret);
                return ret;
            }
            
            std::string request_name = requests[transactionId];
            brs_verbose("AMF0/AMF3 request parsed. request_name=%s", request_name.c_str());

            if (request_name == RTMP_AMF0_COMMAND_CONNECT) {
                brs_info("decode the AMF0/AMF3 response command(%s message).", request_name.c_str());
                *ppacket = packet = new BrsConnectAppResPacket();
                return packet->decode(stream);
            } else if (request_name == RTMP_AMF0_COMMAND_CREATE_STREAM) {
                brs_info("decode the AMF0/AMF3 response command(%s message).", request_name.c_str());
                *ppacket = packet = new BrsCreateStreamResPacket(0, 0);
                return packet->decode(stream);
            } else if (request_name == RTMP_AMF0_COMMAND_RELEASE_STREAM
                || request_name == RTMP_AMF0_COMMAND_FC_PUBLISH
                || request_name == RTMP_AMF0_COMMAND_UNPUBLISH) {
                brs_info("decode the AMF0/AMF3 response command(%s message).", request_name.c_str());
                *ppacket = packet = new BrsFMLEStartResPacket(0);
                return packet->decode(stream);
            } else {
                ret = ERROR_RTMP_NO_REQUEST;
                brs_error("decode AMF0/AMF3 request failed. "
                    "request_name=%s, transactionId=%.2f, ret=%d", 
                    request_name.c_str(), transactionId, ret);
                return ret;
            }
        }
        
        // reset to zero(amf3 to 1) to restart decode.
        stream->skip(-1 * stream->pos());
        if (header.is_amf3_command()) {
            stream->skip(1);
        }
        
        // decode command object.
        if (command == RTMP_AMF0_COMMAND_CONNECT) {
            brs_info("decode the AMF0/AMF3 command(connect vhost/app message).");
            *ppacket = packet = new BrsConnectAppPacket();
            return packet->decode(stream);
        } else if(command == RTMP_AMF0_COMMAND_CREATE_STREAM) {
            brs_info("decode the AMF0/AMF3 command(createStream message).");
            *ppacket = packet = new BrsCreateStreamPacket();
            return packet->decode(stream);
        } else if(command == RTMP_AMF0_COMMAND_PLAY) {
            brs_info("decode the AMF0/AMF3 command(paly message).");
            *ppacket = packet = new BrsPlayPacket();
            return packet->decode(stream);
        } else if(command == RTMP_AMF0_COMMAND_PAUSE) {
            brs_info("decode the AMF0/AMF3 command(pause message).");
            *ppacket = packet = new BrsPausePacket();
            return packet->decode(stream);
        } else if(command == RTMP_AMF0_COMMAND_RELEASE_STREAM) {
            brs_info("decode the AMF0/AMF3 command(FMLE releaseStream message).");
            *ppacket = packet = new BrsFMLEStartPacket();
            return packet->decode(stream);
        } else if(command == RTMP_AMF0_COMMAND_FC_PUBLISH) {
            brs_info("decode the AMF0/AMF3 command(FMLE FCPublish message).");
            *ppacket = packet = new BrsFMLEStartPacket();
            return packet->decode(stream);
        } else if(command == RTMP_AMF0_COMMAND_PUBLISH) {
            brs_info("decode the AMF0/AMF3 command(publish message).");
            *ppacket = packet = new BrsPublishPacket();
            return packet->decode(stream);
        } else if(command == RTMP_AMF0_COMMAND_UNPUBLISH) {
            brs_info("decode the AMF0/AMF3 command(unpublish message).");
            *ppacket = packet = new BrsFMLEStartPacket();
            return packet->decode(stream);
        } else if(command == BRS_CONSTS_RTMP_SET_DATAFRAME || command == BRS_CONSTS_RTMP_ON_METADATA) {
            brs_info("decode the AMF0/AMF3 data(onMetaData message).");
            *ppacket = packet = new BrsOnMetaDataPacket();
            return packet->decode(stream);
        } else if(command == BRS_BW_CHECK_FINISHED
            || command == BRS_BW_CHECK_PLAYING
            || command == BRS_BW_CHECK_PUBLISHING
            || command == BRS_BW_CHECK_STARTING_PLAY
            || command == BRS_BW_CHECK_STARTING_PUBLISH
            || command == BRS_BW_CHECK_START_PLAY
            || command == BRS_BW_CHECK_START_PUBLISH
            || command == BRS_BW_CHECK_STOPPED_PLAY
            || command == BRS_BW_CHECK_STOP_PLAY
            || command == BRS_BW_CHECK_STOP_PUBLISH
            || command == BRS_BW_CHECK_STOPPED_PUBLISH
            || command == BRS_BW_CHECK_FINAL)
        {
            brs_info("decode the AMF0/AMF3 band width check message.");
            *ppacket = packet = new BrsBandwidthPacket();
            return packet->decode(stream);
        } else if (command == RTMP_AMF0_COMMAND_CLOSE_STREAM) {
            brs_info("decode the AMF0/AMF3 closeStream message.");
            *ppacket = packet = new BrsCloseStreamPacket();
            return packet->decode(stream);
        } else if (header.is_amf0_command() || header.is_amf3_command()) {
            brs_info("decode the AMF0/AMF3 call message.");
            *ppacket = packet = new BrsCallPacket();
            return packet->decode(stream);
        }
        
        // default packet to drop message.
        brs_info("drop the AMF0/AMF3 command message, command_name=%s", command.c_str());
        *ppacket = packet = new BrsPacket();
        return ret;
    } else if(header.is_user_control_message()) {
        brs_verbose("start to decode user control message.");
        *ppacket = packet = new BrsUserControlPacket();
        return packet->decode(stream);
    } else if(header.is_window_ackledgement_size()) {
        brs_verbose("start to decode set ack window size message.");
        *ppacket = packet = new BrsSetWindowAckSizePacket();
        return packet->decode(stream);
    } else if(header.is_set_chunk_size()) {
        brs_verbose("start to decode set chunk size message.");
        *ppacket = packet = new BrsSetChunkSizePacket();
        return packet->decode(stream);
    } else {
        if (!header.is_set_peer_bandwidth() && !header.is_ackledgement()) {
            brs_trace("drop unknown message, type=%d", header.message_type);
        }
    }
    
    return ret;
}

int BRSProtocol::send_and_free_message(BrsSharedPtrMessage* msg, int stream_id)
{
    return send_and_free_messages(&msg, 1, stream_id);
}

int BRSProtocol::send_and_free_messages(BrsSharedPtrMessage** msgs, int nb_msgs, int stream_id)
{
    // always not NULL msg.
    assert(msgs);
    assert(nb_msgs > 0);
    
    // update the stream id in header.
    for (int i = 0; i < nb_msgs; i++) {
        BrsSharedPtrMessage* msg = msgs[i];
        
        if (!msg) {
            continue;
        }
        
        // check perfer cid and stream,
        // when one msg stream id is ok, ignore left.
        if (msg->check(stream_id)) {
            break;
        }
    }
    
    // donot use the auto free to free the msg,
    // for performance issue.
    int ret = do_send_messages(msgs, nb_msgs);
    
    for (int i = 0; i < nb_msgs; i++) {
        BrsSharedPtrMessage* msg = msgs[i];
        SafeDelete(msg);
    }
    
    // donot flush when send failed
    if (ret != ERROR_SUCCESS) {
        return ret;
    }
    
    // flush messages in manual queue
    if ((ret = manual_response_flush()) != ERROR_SUCCESS) {
        return ret;
    }
    
    return ret;
}

int BRSProtocol::send_and_free_packet(BrsPacket* packet, int stream_id)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = do_send_and_free_packet(packet, stream_id)) != ERROR_SUCCESS) {
        return ret;
    }
    
    // flush messages in manual queue
    if ((ret = manual_response_flush()) != ERROR_SUCCESS) {
        return ret;
    }
    
    return ret;
}

int BRSProtocol::recv_interlaced_message(BrsCommonMessage** pmsg)
{
    int ret = ERROR_SUCCESS;
    
    // chunk stream basic header.
    char fmt = 0;
    int cid = 0;
    if ((ret = read_basic_header(fmt, cid)) != ERROR_SUCCESS) {
        if (ret != ERROR_SOCKET_TIMEOUT && !brs_is_client_gracefully_close(ret)) {
            brs_error("read basic header failed. ret=%d", ret);
        }
        return ret;
    }
    brs_verbose("read basic header success. fmt=%d, cid=%d", fmt, cid);
    
    // the cid must not negative.
    assert(cid >= 0);
    
    // get the cached chunk stream.
    BrsChunkStream* chunk = NULL;
    
    // use chunk stream cache to get the chunk info.
    // @see https://github.com/osbrs/brs/issues/249
    if (cid < BRS_PERF_CHUNK_STREAM_CACHE) {
        // chunk stream cache hit.
        brs_verbose("cs-cache hit, cid=%d", cid);
        // already init, use it direclty
        chunk = cs_cache[cid];
        brs_verbose("cached chunk stream: fmt=%d, cid=%d, size=%d, message(type=%d, size=%d, time=%"PRId64", sid=%d)",
            chunk->fmt, chunk->cid, (chunk->msg? chunk->msg->size : 0), chunk->header.message_type, chunk->header.payload_length,
            chunk->header.timestamp, chunk->header.stream_id);
    } else {
        // chunk stream cache miss, use map.
        if (chunk_streams.find(cid) == chunk_streams.end()) {
            chunk = chunk_streams[cid] = new BrsChunkStream(cid);
            // set the perfer cid of chunk,
            // which will copy to the message received.
            chunk->header.perfer_cid = cid;
            brs_verbose("cache new chunk stream: fmt=%d, cid=%d", fmt, cid);
        } else {
            chunk = chunk_streams[cid];
            brs_verbose("cached chunk stream: fmt=%d, cid=%d, size=%d, message(type=%d, size=%d, time=%"PRId64", sid=%d)",
                chunk->fmt, chunk->cid, (chunk->msg? chunk->msg->size : 0), chunk->header.message_type, chunk->header.payload_length,
                chunk->header.timestamp, chunk->header.stream_id);
        }
    }

    // chunk stream message header
    if ((ret = read_message_header(chunk, fmt)) != ERROR_SUCCESS) {
        if (ret != ERROR_SOCKET_TIMEOUT && !brs_is_client_gracefully_close(ret)) {
            brs_error("read message header failed. ret=%d", ret);
        }
        return ret;
    }
    brs_verbose("read message header success. "
            "fmt=%d, ext_time=%d, size=%d, message(type=%d, size=%d, time=%"PRId64", sid=%d)", 
            fmt, chunk->extended_timestamp, (chunk->msg? chunk->msg->size : 0), chunk->header.message_type, 
            chunk->header.payload_length, chunk->header.timestamp, chunk->header.stream_id);
    
    // read msg payload from chunk stream.
    BrsCommonMessage* msg = NULL;
    if ((ret = read_message_payload(chunk, &msg)) != ERROR_SUCCESS) {
        if (ret != ERROR_SOCKET_TIMEOUT && !brs_is_client_gracefully_close(ret)) {
            brs_error("read message payload failed. ret=%d", ret);
        }
        return ret;
    }
    
    // not got an entire RTMP message, try next chunk.
    if (!msg) {
        brs_verbose("get partial message success. size=%d, message(type=%d, size=%d, time=%"PRId64", sid=%d)",
                (msg? msg->size : (chunk->msg? chunk->msg->size : 0)), chunk->header.message_type, chunk->header.payload_length,
                chunk->header.timestamp, chunk->header.stream_id);
        return ret;
    }
    
    *pmsg = msg;
    brs_info("get entire message success. size=%d, message(type=%d, size=%d, time=%"PRId64", sid=%d)",
            (msg? msg->size : (chunk->msg? chunk->msg->size : 0)), chunk->header.message_type, chunk->header.payload_length,
            chunk->header.timestamp, chunk->header.stream_id);
            
    return ret;
}

/**
* 6.1.1. Chunk Basic Header
* The Chunk Basic Header encodes the chunk stream ID and the chunk
* type(represented by fmt field in the figure below). Chunk type
* determines the format of the encoded message header. Chunk Basic
* Header field may be 1, 2, or 3 bytes, depending on the chunk stream
* ID.
* 
* The bits 0-5 (least significant) in the chunk basic header represent
* the chunk stream ID.
*
* Chunk stream IDs 2-63 can be encoded in the 1-byte version of this
* field.
*    0 1 2 3 4 5 6 7
*   +-+-+-+-+-+-+-+-+
*   |fmt|   cs id   |
*   +-+-+-+-+-+-+-+-+
*   Figure 6 Chunk basic header 1
*
* Chunk stream IDs 64-319 can be encoded in the 2-byte version of this
* field. ID is computed as (the second byte + 64).
*   0                   1
*   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |fmt|    0      | cs id - 64    |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   Figure 7 Chunk basic header 2
*
* Chunk stream IDs 64-65599 can be encoded in the 3-byte version of
* this field. ID is computed as ((the third byte)*256 + the second byte
* + 64).
*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |fmt|     1     |         cs id - 64            |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   Figure 8 Chunk basic header 3
*
* cs id: 6 bits
* fmt: 2 bits
* cs id - 64: 8 or 16 bits
* 
* Chunk stream IDs with values 64-319 could be represented by both 2-
* byte version and 3-byte version of this field.
*/
int BRSProtocol::read_basic_header(char& fmt, int& cid)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = in_buffer->grow(skt, 1)) != ERROR_SUCCESS) {
        if (ret != ERROR_SOCKET_TIMEOUT && !brs_is_client_gracefully_close(ret)) {
            brs_error("read 1bytes basic header failed. required_size=%d, ret=%d", 1, ret);
        }
        return ret;
    }
    
    fmt = in_buffer->read_1byte();
    cid = fmt & 0x3f;
    fmt = (fmt >> 6) & 0x03;
    
    // 2-63, 1B chunk header
    if (cid > 1) {
        brs_verbose("basic header parsed. fmt=%d, cid=%d", fmt, cid);
        return ret;
    }

    // 64-319, 2B chunk header
    if (cid == 0) {
        if ((ret = in_buffer->grow(skt, 1)) != ERROR_SUCCESS) {
            if (ret != ERROR_SOCKET_TIMEOUT && !brs_is_client_gracefully_close(ret)) {
                brs_error("read 2bytes basic header failed. required_size=%d, ret=%d", 1, ret);
            }
            return ret;
        }
        
        cid = 64;
        cid += (u_int8_t)in_buffer->read_1byte();
        brs_verbose("2bytes basic header parsed. fmt=%d, cid=%d", fmt, cid);
    // 64-65599, 3B chunk header
    } else if (cid == 1) {
        if ((ret = in_buffer->grow(skt, 2)) != ERROR_SUCCESS) {
            if (ret != ERROR_SOCKET_TIMEOUT && !brs_is_client_gracefully_close(ret)) {
                brs_error("read 3bytes basic header failed. required_size=%d, ret=%d", 2, ret);
            }
            return ret;
        }
        
        cid = 64;
        cid += (u_int8_t)in_buffer->read_1byte();
        cid += ((u_int8_t)in_buffer->read_1byte()) * 256;
        brs_verbose("3bytes basic header parsed. fmt=%d, cid=%d", fmt, cid);
    } else {
        brs_error("invalid path, impossible basic header.");
        assert(false);
    }
    
    return ret;
}

/**
* parse the message header.
*   3bytes: timestamp delta,    fmt=0,1,2
*   3bytes: payload length,     fmt=0,1
*   1bytes: message type,       fmt=0,1
*   4bytes: stream id,          fmt=0
* where:
*   fmt=0, 0x0X
*   fmt=1, 0x4X
*   fmt=2, 0x8X
*   fmt=3, 0xCX
*/
int BRSProtocol::read_message_header(BrsChunkStream* chunk, char fmt)
{
    int ret = ERROR_SUCCESS;
    
    /**
    * we should not assert anything about fmt, for the first packet.
    * (when first packet, the chunk->msg is NULL).
    * the fmt maybe 0/1/2/3, the FMLE will send a 0xC4 for some audio packet.
    * the previous packet is:
    *     04                // fmt=0, cid=4
    *     00 00 1a          // timestamp=26
    *     00 00 9d          // payload_length=157
    *     08                // message_type=8(audio)
    *     01 00 00 00       // stream_id=1
    * the current packet maybe:
    *     c4             // fmt=3, cid=4
    * it's ok, for the packet is audio, and timestamp delta is 26.
    * the current packet must be parsed as:
    *     fmt=0, cid=4
    *     timestamp=26+26=52
    *     payload_length=157
    *     message_type=8(audio)
    *     stream_id=1
    * so we must update the timestamp even fmt=3 for first packet.
    */
    // fresh packet used to update the timestamp even fmt=3 for first packet.
    // fresh packet always means the chunk is the first one of message.
    bool is_first_chunk_of_msg = !chunk->msg;
    
    // but, we can ensure that when a chunk stream is fresh, 
    // the fmt must be 0, a new stream.
    if (chunk->msg_count == 0 && fmt != RTMP_FMT_TYPE0) {
        // for librtmp, if ping, it will send a fresh stream with fmt=1,
        // 0x42             where: fmt=1, cid=2, protocol contorl user-control message
        // 0x00 0x00 0x00   where: timestamp=0
        // 0x00 0x00 0x06   where: payload_length=6
        // 0x04             where: message_type=4(protocol control user-control message)
        // 0x00 0x06            where: event Ping(0x06)
        // 0x00 0x00 0x0d 0x0f  where: event data 4bytes ping timestamp.
        // @see: https://github.com/osbrs/brs/issues/98
        if (chunk->cid == RTMP_CID_ProtocolControl && fmt == RTMP_FMT_TYPE1) {
            brs_warn("accept cid=2, fmt=1 to make librtmp happy.");
        } else {
            // must be a RTMP protocol level error.
            ret = ERROR_RTMP_CHUNK_START;
            brs_error("chunk stream is fresh, fmt must be %d, actual is %d. cid=%d, ret=%d", 
                RTMP_FMT_TYPE0, fmt, chunk->cid, ret);
            return ret;
        }
    }

    // when exists cache msg, means got an partial message,
    // the fmt must not be type0 which means new message.
    if (chunk->msg && fmt == RTMP_FMT_TYPE0) {
        ret = ERROR_RTMP_CHUNK_START;
        brs_error("chunk stream exists, "
            "fmt must not be %d, actual is %d. ret=%d", RTMP_FMT_TYPE0, fmt, ret);
        return ret;
    }
    
    // create msg when new chunk stream start
    if (!chunk->msg) {
        chunk->msg = new BrsCommonMessage();
        brs_verbose("create message for new chunk, fmt=%d, cid=%d", fmt, chunk->cid);
    }

    // read message header from socket to buffer.
    static char mh_sizes[] = {11, 7, 3, 0};
    int mh_size = mh_sizes[(int)fmt];
    brs_verbose("calc chunk message header size. fmt=%d, mh_size=%d", fmt, mh_size);
    
    if (mh_size > 0 && (ret = in_buffer->grow(skt, mh_size)) != ERROR_SUCCESS) {
        if (ret != ERROR_SOCKET_TIMEOUT && !brs_is_client_gracefully_close(ret)) {
            brs_error("read %dbytes message header failed. ret=%d", mh_size, ret);
        }
        return ret;
    }
    
    /**
    * parse the message header.
    *   3bytes: timestamp delta,    fmt=0,1,2
    *   3bytes: payload length,     fmt=0,1
    *   1bytes: message type,       fmt=0,1
    *   4bytes: stream id,          fmt=0
    * where:
    *   fmt=0, 0x0X
    *   fmt=1, 0x4X
    *   fmt=2, 0x8X
    *   fmt=3, 0xCX
    */
    // see also: ngx_rtmp_recv
    if (fmt <= RTMP_FMT_TYPE2) {
        char* p = in_buffer->read_slice(mh_size);
    
        char* pp = (char*)&chunk->header.timestamp_delta;
        pp[2] = *p++;
        pp[1] = *p++;
        pp[0] = *p++;
        pp[3] = 0;
        
        // fmt: 0
        // timestamp: 3 bytes
        // If the timestamp is greater than or equal to 16777215
        // (hexadecimal 0x00ffffff), this value MUST be 16777215, and the
        // 'extended timestamp header' MUST be present. Otherwise, this value
        // SHOULD be the entire timestamp.
        //
        // fmt: 1 or 2
        // timestamp delta: 3 bytes
        // If the delta is greater than or equal to 16777215 (hexadecimal
        // 0x00ffffff), this value MUST be 16777215, and the 'extended
        // timestamp header' MUST be present. Otherwise, this value SHOULD be
        // the entire delta.
        chunk->extended_timestamp = (chunk->header.timestamp_delta >= RTMP_EXTENDED_TIMESTAMP);
        if (!chunk->extended_timestamp) {
            // Extended timestamp: 0 or 4 bytes
            // This field MUST be sent when the normal timsestamp is set to
            // 0xffffff, it MUST NOT be sent if the normal timestamp is set to
            // anything else. So for values less than 0xffffff the normal
            // timestamp field SHOULD be used in which case the extended timestamp
            // MUST NOT be present. For values greater than or equal to 0xffffff
            // the normal timestamp field MUST NOT be used and MUST be set to
            // 0xffffff and the extended timestamp MUST be sent.
            if (fmt == RTMP_FMT_TYPE0) {
                // 6.1.2.1. Type 0
                // For a type-0 chunk, the absolute timestamp of the message is sent
                // here.
                chunk->header.timestamp = chunk->header.timestamp_delta;
            } else {
                // 6.1.2.2. Type 1
                // 6.1.2.3. Type 2
                // For a type-1 or type-2 chunk, the difference between the previous
                // chunk's timestamp and the current chunk's timestamp is sent here.
                chunk->header.timestamp += chunk->header.timestamp_delta;
            }
        }
        
        if (fmt <= RTMP_FMT_TYPE1) {
            int32_t payload_length = 0;
            pp = (char*)&payload_length;
            pp[2] = *p++;
            pp[1] = *p++;
            pp[0] = *p++;
            pp[3] = 0;
            
            // for a message, if msg exists in cache, the size must not changed.
            // always use the actual msg size to compare, for the cache payload length can changed,
            // for the fmt type1(stream_id not changed), user can change the payload 
            // length(it's not allowed in the continue chunks).
            if (!is_first_chunk_of_msg && chunk->header.payload_length != payload_length) {
                ret = ERROR_RTMP_PACKET_SIZE;
                brs_error("msg exists in chunk cache, "
                    "size=%d cannot change to %d, ret=%d", 
                    chunk->header.payload_length, payload_length, ret);
                return ret;
            }
            
            chunk->header.payload_length = payload_length;
            chunk->header.message_type = *p++;
            
            if (fmt == RTMP_FMT_TYPE0) {
                pp = (char*)&chunk->header.stream_id;
                pp[0] = *p++;
                pp[1] = *p++;
                pp[2] = *p++;
                pp[3] = *p++;
                brs_verbose("header read completed. fmt=%d, mh_size=%d, ext_time=%d, time=%"PRId64", payload=%d, type=%d, sid=%d", 
                    fmt, mh_size, chunk->extended_timestamp, chunk->header.timestamp, chunk->header.payload_length, 
                    chunk->header.message_type, chunk->header.stream_id);
            } else {
                brs_verbose("header read completed. fmt=%d, mh_size=%d, ext_time=%d, time=%"PRId64", payload=%d, type=%d", 
                    fmt, mh_size, chunk->extended_timestamp, chunk->header.timestamp, chunk->header.payload_length, 
                    chunk->header.message_type);
            }
        } else {
            brs_verbose("header read completed. fmt=%d, mh_size=%d, ext_time=%d, time=%"PRId64"", 
                fmt, mh_size, chunk->extended_timestamp, chunk->header.timestamp);
        }
    } else {
        // update the timestamp even fmt=3 for first chunk packet
        if (is_first_chunk_of_msg && !chunk->extended_timestamp) {
            chunk->header.timestamp += chunk->header.timestamp_delta;
        }
        brs_verbose("header read completed. fmt=%d, size=%d, ext_time=%d", 
            fmt, mh_size, chunk->extended_timestamp);
    }
    
    // read extended-timestamp
    if (chunk->extended_timestamp) {
        mh_size += 4;
        brs_verbose("read header ext time. fmt=%d, ext_time=%d, mh_size=%d", fmt, chunk->extended_timestamp, mh_size);
        if ((ret = in_buffer->grow(skt, 4)) != ERROR_SUCCESS) {
            if (ret != ERROR_SOCKET_TIMEOUT && !brs_is_client_gracefully_close(ret)) {
                brs_error("read %dbytes message header failed. required_size=%d, ret=%d", mh_size, 4, ret);
            }
            return ret;
        }
        // the ptr to the slice maybe invalid when grow()
        // reset the p to get 4bytes slice.
        char* p = in_buffer->read_slice(4);

        u_int32_t timestamp = 0x00;
        char* pp = (char*)&timestamp;
        pp[3] = *p++;
        pp[2] = *p++;
        pp[1] = *p++;
        pp[0] = *p++;

        // always use 31bits timestamp, for some server may use 32bits extended timestamp.
        // @see https://github.com/osbrs/brs/issues/111
        timestamp &= 0x7fffffff;
        
        /**
        * RTMP specification and ffmpeg/librtmp is false,
        * but, adobe changed the specification, so flash/FMLE/FMS always true.
        * default to true to support flash/FMLE/FMS.
        * 
        * ffmpeg/librtmp may donot send this filed, need to detect the value.
        * @see also: http://blog.csdn.net/win_lin/article/details/13363699
        * compare to the chunk timestamp, which is set by chunk message header
        * type 0,1 or 2.
        *
        * @remark, nginx send the extended-timestamp in sequence-header,
        * and timestamp delta in continue C1 chunks, and so compatible with ffmpeg,
        * that is, there is no continue chunks and extended-timestamp in nginx-rtmp.
        *
        * @remark, brs always send the extended-timestamp, to keep simple,
        * and compatible with adobe products.
        */
        u_int32_t chunk_timestamp = (u_int32_t)chunk->header.timestamp;
        
        /**
        * if chunk_timestamp<=0, the chunk previous packet has no extended-timestamp,
        * always use the extended timestamp.
        */
        /**
        * about the is_first_chunk_of_msg.
        * @remark, for the first chunk of message, always use the extended timestamp.
        */
        if (!is_first_chunk_of_msg && chunk_timestamp > 0 && chunk_timestamp != timestamp) {
            mh_size -= 4;
            in_buffer->skip(-4);
            brs_info("no 4bytes extended timestamp in the continued chunk");
        } else {
            chunk->header.timestamp = timestamp;
        }
        brs_verbose("header read ext_time completed. time=%"PRId64"", chunk->header.timestamp);
    }
    
    // the extended-timestamp must be unsigned-int,
    //         24bits timestamp: 0xffffff = 16777215ms = 16777.215s = 4.66h
    //         32bits timestamp: 0xffffffff = 4294967295ms = 4294967.295s = 1193.046h = 49.71d
    // because the rtmp protocol says the 32bits timestamp is about "50 days":
    //         3. Byte Order, Alignment, and Time Format
    //                Because timestamps are generally only 32 bits long, they will roll
    //                over after fewer than 50 days.
    // 
    // but, its sample says the timestamp is 31bits:
    //         An application could assume, for example, that all 
    //        adjacent timestamps are within 2^31 milliseconds of each other, so
    //        10000 comes after 4000000000, while 3000000000 comes before
    //        4000000000.
    // and flv specification says timestamp is 31bits:
    //        Extension of the Timestamp field to form a SI32 value. This
    //        field represents the upper 8 bits, while the previous
    //        Timestamp field represents the lower 24 bits of the time in
    //        milliseconds.
    // in a word, 31bits timestamp is ok.
    // convert extended timestamp to 31bits.
    chunk->header.timestamp &= 0x7fffffff;
    
    // valid message, the payload_length is 24bits,
    // so it should never be negative.
    assert(chunk->header.payload_length >= 0);
    
    // copy header to msg
    chunk->msg->header = chunk->header;
    
    // increase the msg count, the chunk stream can accept fmt=1/2/3 message now.
    chunk->msg_count++;
    
    return ret;
}

int BRSProtocol::read_message_payload(BrsChunkStream* chunk, BrsCommonMessage** pmsg)
{
    int ret = ERROR_SUCCESS;
    
    // empty message
    if (chunk->header.payload_length <= 0) {
        brs_trace("get an empty RTMP "
                "message(type=%d, size=%d, time=%"PRId64", sid=%d)", chunk->header.message_type, 
                chunk->header.payload_length, chunk->header.timestamp, chunk->header.stream_id);
        
        *pmsg = chunk->msg;
        chunk->msg = NULL;
                
        return ret;
    }
    assert(chunk->header.payload_length > 0);
    
    // the chunk payload size.
    int payload_size = chunk->header.payload_length - chunk->msg->size;
    payload_size = brs_min(payload_size, in_chunk_size);
    brs_verbose("chunk payload size is %d, message_size=%d, received_size=%d, in_chunk_size=%d", 
        payload_size, chunk->header.payload_length, chunk->msg->size, in_chunk_size);

    // create msg payload if not initialized
    if (!chunk->msg->payload) {
        chunk->msg->create_payload(chunk->header.payload_length);
    }
    
    // read payload to buffer
    if ((ret = in_buffer->grow(skt, payload_size)) != ERROR_SUCCESS) {
        if (ret != ERROR_SOCKET_TIMEOUT && !brs_is_client_gracefully_close(ret)) {
            brs_error("read payload failed. required_size=%d, ret=%d", payload_size, ret);
        }
        return ret;
    }
    memcpy(chunk->msg->payload + chunk->msg->size, in_buffer->read_slice(payload_size), payload_size);
    chunk->msg->size += payload_size;
    
    brs_verbose("chunk payload read completed. payload_size=%d", payload_size);
    
    // got entire RTMP message?
    if (chunk->header.payload_length == chunk->msg->size) {
        *pmsg = chunk->msg;
        chunk->msg = NULL;
        brs_verbose("get entire RTMP message(type=%d, size=%d, time=%"PRId64", sid=%d)", 
                chunk->header.message_type, chunk->header.payload_length, 
                chunk->header.timestamp, chunk->header.stream_id);
        return ret;
    }
    
    brs_verbose("get partial RTMP message(type=%d, size=%d, time=%"PRId64", sid=%d), partial size=%d", 
            chunk->header.message_type, chunk->header.payload_length, 
            chunk->header.timestamp, chunk->header.stream_id,
            chunk->msg->size);
            
    return ret;
}

int BRSProtocol::on_recv_message(BrsCommonMessage* msg)
{
    int ret = ERROR_SUCCESS;
    
    assert(msg != NULL);
        
    // acknowledgement
    if (in_ack_size.ack_window_size > 0 
        && skt->get_recv_bytes() - in_ack_size.acked_size > in_ack_size.ack_window_size
    ) {
        if ((ret = response_acknowledgement_message()) != ERROR_SUCCESS) {
            return ret;
        }
    }
    
    BrsPacket* packet = NULL;
    switch (msg->header.message_type) {
        case RTMP_MSG_SetChunkSize:
        case RTMP_MSG_UserControlMessage:
        case RTMP_MSG_WindowAcknowledgementSize:
            if ((ret = decode_message(msg, &packet)) != ERROR_SUCCESS) {
                brs_error("decode packet from message payload failed. ret=%d", ret);
                return ret;
            }
            brs_verbose("decode packet from message payload success.");
            break;
        default:
            return ret;
    }
    
    assert(packet);
    
    // always free the packet.
    BrsAutoFreeE(BrsPacket, packet);
    
    switch (msg->header.message_type) {
        case RTMP_MSG_WindowAcknowledgementSize: {
            BrsSetWindowAckSizePacket* pkt = dynamic_cast<BrsSetWindowAckSizePacket*>(packet);
            assert(pkt != NULL);
            
            if (pkt->ackowledgement_window_size > 0) {
                in_ack_size.ack_window_size = pkt->ackowledgement_window_size;
                // @remark, we ignore this message, for user noneed to care.
                // but it's important for dev, for client/server will block if required 
                // ack msg not arrived.
                brs_info("set ack window size to %d", pkt->ackowledgement_window_size);
            } else {
                brs_warn("ignored. set ack window size is %d", pkt->ackowledgement_window_size);
            }
            break;
        }
        case RTMP_MSG_SetChunkSize: {
            BrsSetChunkSizePacket* pkt = dynamic_cast<BrsSetChunkSizePacket*>(packet);
            assert(pkt != NULL);

            // for some server, the actual chunk size can greater than the max value(65536),
            // so we just warning the invalid chunk size, and actually use it is ok,
            // @see: https://github.com/osbrs/brs/issues/160
            if (pkt->chunk_size < BRS_CONSTS_RTMP_MIN_CHUNK_SIZE 
                || pkt->chunk_size > BRS_CONSTS_RTMP_MAX_CHUNK_SIZE) 
            {
                brs_warn("accept chunk size %d, but should in [%d, %d], "
                    "@see: https://github.com/osbrs/brs/issues/160",
                    pkt->chunk_size, BRS_CONSTS_RTMP_MIN_CHUNK_SIZE,  BRS_CONSTS_RTMP_MAX_CHUNK_SIZE);
            }

            // @see: https://github.com/osbrs/brs/issues/541
            if (pkt->chunk_size < BRS_CONSTS_RTMP_MIN_CHUNK_SIZE) {
                ret = ERROR_RTMP_CHUNK_SIZE;
                brs_error("chunk size should be %d+, value=%d. ret=%d",
                    BRS_CONSTS_RTMP_MIN_CHUNK_SIZE, pkt->chunk_size, ret);
                return ret;
            }
            
            in_chunk_size = pkt->chunk_size;
            brs_trace("input chunk size to %d", pkt->chunk_size);

            break;
        }
        case RTMP_MSG_UserControlMessage: {
            BrsUserControlPacket* pkt = dynamic_cast<BrsUserControlPacket*>(packet);
            assert(pkt != NULL);
            
            if (pkt->event_type == SrcPCUCSetBufferLength) {
                brs_trace("ignored. set buffer length to %d", pkt->extra_data);
            }
            if (pkt->event_type == SrcPCUCPingRequest) {
                if ((ret = response_ping_message(pkt->event_data)) != ERROR_SUCCESS) {
                    return ret;
                }
            }
            break;
        }
        default:
            break;
    }
    
    return ret;
}

int BRSProtocol::on_send_packet(BrsMessageHeader* mh, BrsPacket* packet)
{
    int ret = ERROR_SUCCESS;
    
    // ignore raw bytes oriented RTMP message.
    if (packet == NULL) {
        return ret;
    }
    
    switch (mh->message_type) {
        case RTMP_MSG_SetChunkSize: {
            BrsSetChunkSizePacket* pkt = dynamic_cast<BrsSetChunkSizePacket*>(packet);
            assert(pkt != NULL);
            
            out_chunk_size = pkt->chunk_size;
            
            brs_trace("out chunk size to %d", pkt->chunk_size);
            break;
        }
        case RTMP_MSG_AMF0CommandMessage:
        case RTMP_MSG_AMF3CommandMessage: {
            if (true) {
                BrsConnectAppPacket* pkt = dynamic_cast<BrsConnectAppPacket*>(packet);
                if (pkt) {
                    requests[pkt->transaction_id] = pkt->command_name;
                    break;
                }
            }
            if (true) {
                BrsCreateStreamPacket* pkt = dynamic_cast<BrsCreateStreamPacket*>(packet);
                if (pkt) {
                    requests[pkt->transaction_id] = pkt->command_name;
                    break;
                }
            }
            if (true) {
                BrsFMLEStartPacket* pkt = dynamic_cast<BrsFMLEStartPacket*>(packet);
                if (pkt) {
                    requests[pkt->transaction_id] = pkt->command_name;
                    break;
                }
            }
            break;
        }
        default:
            break;
    }
    
    return ret;
}

int BRSProtocol::response_acknowledgement_message()
{
    int ret = ERROR_SUCCESS;
    
    BrsAcknowledgementPacket* pkt = new BrsAcknowledgementPacket();
    in_ack_size.acked_size = skt->get_recv_bytes();
    pkt->sequence_number = (int32_t)in_ack_size.acked_size;
    
    // cache the message and use flush to send.
    if (!auto_response_when_recv) {
        manual_response_queue.push_back(pkt);
        return ret;
    }
    
    // use underlayer api to send, donot flush again.
    if ((ret = do_send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
        brs_error("send acknowledgement failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("send acknowledgement success.");
    
    return ret;
}

int BRSProtocol::response_ping_message(int32_t timestamp)
{
    int ret = ERROR_SUCCESS;
    
    brs_trace("get a ping request, response it. timestamp=%d", timestamp);
    
    BrsUserControlPacket* pkt = new BrsUserControlPacket();
    
    pkt->event_type = SrcPCUCPingResponse;
    pkt->event_data = timestamp;
    
    // cache the message and use flush to send.
    if (!auto_response_when_recv) {
        manual_response_queue.push_back(pkt);
        return ret;
    }
    
    // use underlayer api to send, donot flush again.
    if ((ret = do_send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
        brs_error("send ping response failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("send ping response success.");
    
    return ret;
}



BrsRtmpServer::BrsRtmpServer(BRSReadWriter* skt)
{
    io = skt;
    protocol = new BRSProtocol(skt);
}

BrsRtmpServer::~BrsRtmpServer()
{
    SafeDelete(protocol);
}

void BrsRtmpServer::set_auto_response(bool v)
{
    protocol->set_auto_response(v);
}

#ifdef BRS_PERF_MERGED_READ
void BrsRtmpServer::set_merge_read(bool v, IMergeReadHandler* handler)
{
    protocol->set_merge_read(v, handler);
}

void BrsRtmpServer::set_recv_buffer(int buffer_size)
{
    protocol->set_recv_buffer(buffer_size);
}
#endif

void BrsRtmpServer::set_recv_timeout(int64_t timeout_us)
{
    protocol->set_recv_timeout(timeout_us);
}

int64_t BrsRtmpServer::get_recv_timeout()
{
    return protocol->get_recv_timeout();
}

void BrsRtmpServer::set_send_timeout(int64_t timeout_us)
{
    protocol->set_send_timeout(timeout_us);
}

int64_t BrsRtmpServer::get_send_timeout()
{
    return protocol->get_send_timeout();
}

int64_t BrsRtmpServer::get_recv_bytes()
{
    return protocol->get_recv_bytes();
}

int64_t BrsRtmpServer::get_send_bytes()
{
    return protocol->get_send_bytes();
}

int BrsRtmpServer::recv_message(BrsCommonMessage** pmsg)
{
    return protocol->recv_message(pmsg);
}

int BrsRtmpServer::decode_message(BrsCommonMessage* msg, BrsPacket** ppacket)
{
    return protocol->decode_message(msg, ppacket);
}

int BrsRtmpServer::send_and_free_message(BrsSharedPtrMessage* msg, int stream_id)
{
    return protocol->send_and_free_message(msg, stream_id);
}

int BrsRtmpServer::send_and_free_messages(BrsSharedPtrMessage** msgs, int nb_msgs, int stream_id)
{
    return protocol->send_and_free_messages(msgs, nb_msgs, stream_id);
}

int BrsRtmpServer::send_and_free_packet(BrsPacket* packet, int stream_id)
{
    return protocol->send_and_free_packet(packet, stream_id);
}



int BrsRtmpServer::connect_app(BrsRequest* req)
{
    int ret = ERROR_SUCCESS;
    
    BrsCommonMessage* msg = NULL;
    BrsConnectAppPacket* pkt = NULL;
    if ((ret = expect_message<BrsConnectAppPacket>(&msg, &pkt)) != ERROR_SUCCESS) {
        brs_error("expect connect app message failed. ret=%d", ret);
        return ret;
    }
    BrsAutoFreeE(BrsCommonMessage, msg);
    BrsAutoFreeE(BrsConnectAppPacket, pkt);
    brs_info("get connect app message");
    
    BrsAmf0Any* prop = NULL;
    
    if ((prop = pkt->command_object->ensure_property_string("tcUrl")) == NULL) {
        ret = ERROR_RTMP_REQ_CONNECT;
        brs_error("invalid request, must specifies the tcUrl. ret=%d", ret);
        return ret;
    }
    req->tcUrl = prop->to_str();
    
    if ((prop = pkt->command_object->ensure_property_string("pageUrl")) != NULL) {
        req->pageUrl = prop->to_str();
    }
    
    if ((prop = pkt->command_object->ensure_property_string("swfUrl")) != NULL) {
        req->swfUrl = prop->to_str();
    }
    
    if ((prop = pkt->command_object->ensure_property_number("objectEncoding")) != NULL) {
        req->objectEncoding = prop->to_number();
    }
    
    if (pkt->args) {
        SafeDelete(req->args);
        req->args = pkt->args->copy()->to_object();
        brs_info("copy edge traverse to origin auth args.");
    }
    
    brs_info("get connect app message params success.");
    
    brs_discovery_tc_url(req->tcUrl, 
        req->schema, req->host, req->vhost, req->app, req->port,
        req->param);
    req->strip();
    
    return ret;
}

int BrsRtmpServer::set_window_ack_size(int ack_size)
{
    int ret = ERROR_SUCCESS;
    
    BrsSetWindowAckSizePacket* pkt = new BrsSetWindowAckSizePacket();
    pkt->ackowledgement_window_size = ack_size;
    if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
        brs_error("send ack size message failed. ret=%d", ret);
        return ret;
    }
    brs_info("send ack size message success. ack_size=%d", ack_size);
    
    return ret;
}

int BrsRtmpServer::set_peer_bandwidth(int bandwidth, int type)
{
    int ret = ERROR_SUCCESS;
    
    BrsSetPeerBandwidthPacket* pkt = new BrsSetPeerBandwidthPacket();
    pkt->bandwidth = bandwidth;
    pkt->type = type;
    if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
        brs_error("send set bandwidth message failed. ret=%d", ret);
        return ret;
    }
    brs_info("send set bandwidth message "
        "success. bandwidth=%d, type=%d", bandwidth, type);
    
    return ret;
}

int BrsRtmpServer::response_connect_app(BrsRequest *req, const char* server_ip)
{
    int ret = ERROR_SUCCESS;
    
    BrsConnectAppResPacket* pkt = new BrsConnectAppResPacket();
    
    pkt->props->set("fmsVer", BrsAmf0Any::str("FMS/"RTMP_SIG_FMS_VER));
    pkt->props->set("capabilities", BrsAmf0Any::number(127));
    pkt->props->set("mode", BrsAmf0Any::number(1));
    
    pkt->info->set(StatusLevel, BrsAmf0Any::str(StatusLevelStatus));
    pkt->info->set(StatusCode, BrsAmf0Any::str(StatusCodeConnectSuccess));
    pkt->info->set(StatusDescription, BrsAmf0Any::str("Connection succeeded"));
    pkt->info->set("objectEncoding", BrsAmf0Any::number(req->objectEncoding));
    BrsAmf0EcmaArray* data = BrsAmf0Any::ecma_array();
    pkt->info->set("data", data);
    
    data->set("version", BrsAmf0Any::str(RTMP_SIG_FMS_VER));
    data->set("brs_sig", BrsAmf0Any::str(RTMP_SIG_BRS_KEY));
    data->set("brs_server", BrsAmf0Any::str(RTMP_SIG_BRS_SERVER));
    data->set("brs_license", BrsAmf0Any::str(RTMP_SIG_BRS_LICENSE));
    data->set("brs_role", BrsAmf0Any::str(RTMP_SIG_BRS_ROLE));
    data->set("brs_url", BrsAmf0Any::str(RTMP_SIG_BRS_URL));
    data->set("brs_version", BrsAmf0Any::str(RTMP_SIG_BRS_VERSION));
    data->set("brs_site", BrsAmf0Any::str(RTMP_SIG_BRS_WEB));
    data->set("brs_email", BrsAmf0Any::str(RTMP_SIG_BRS_EMAIL));
    data->set("brs_copyright", BrsAmf0Any::str(RTMP_SIG_BRS_COPYRIGHT));
    data->set("brs_primary", BrsAmf0Any::str(RTMP_SIG_BRS_PRIMARY));
    data->set("brs_authors", BrsAmf0Any::str(RTMP_SIG_BRS_AUTHROS));
    
    if (server_ip) {
        data->set("brs_server_ip", BrsAmf0Any::str(server_ip));
    }
    // for edge to directly get the id of client.
    data->set("brs_pid", BrsAmf0Any::number(getpid()));
    //data->set("brs_id", BrsAmf0Any::number(_brs_context->get_id()));
    
    if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
        brs_error("send connect app response message failed. ret=%d", ret);
        return ret;
    }
    brs_info("send connect app response message success.");
    
    return ret;
}

void BrsRtmpServer::response_connect_reject(BrsRequest* /*req*/, const char* desc)
{
    int ret = ERROR_SUCCESS;
    
    BrsOnStatusCallPacket* pkt = new BrsOnStatusCallPacket();
    pkt->data->set(StatusLevel, BrsAmf0Any::str(StatusLevelError));
    pkt->data->set(StatusCode, BrsAmf0Any::str(StatusCodeConnectRejected));
    pkt->data->set(StatusDescription, BrsAmf0Any::str(desc));
    
    if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
        brs_error("send connect app response rejected message failed. ret=%d", ret);
        return;
    }
    brs_info("send connect app response rejected message success.");

    return;
}

int BrsRtmpServer::on_bw_done()
{
    int ret = ERROR_SUCCESS;
    
    BrsOnBWDonePacket* pkt = new BrsOnBWDonePacket();
    if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
        brs_error("send onBWDone message failed. ret=%d", ret);
        return ret;
    }
    brs_info("send onBWDone message success.");
    
    return ret;
}

int BrsRtmpServer::identify_client(int stream_id, BrsRtmpConnType& type, string& stream_name, double& duration)
{
    type = BrsRtmpConnUnknown;
    int ret = ERROR_SUCCESS;
    
    while (true) {
        BrsCommonMessage* msg = NULL;
        if ((ret = protocol->recv_message(&msg)) != ERROR_SUCCESS) {
            if (!brs_is_client_gracefully_close(ret)) {
                brs_error("recv identify client message failed. ret=%d", ret);
            }
            return ret;
        }

        BrsAutoFreeE(BrsCommonMessage, msg);
        BrsMessageHeader& h = msg->header;
        
        if (h.is_ackledgement() || h.is_set_chunk_size() || h.is_window_ackledgement_size() || h.is_user_control_message()) {
            continue;
        }
        
        if (!h.is_amf0_command() && !h.is_amf3_command()) {
            brs_trace("identify ignore messages except "
                "AMF0/AMF3 command message. type=%#x", h.message_type);
            continue;
        }
        
        BrsPacket* pkt = NULL;
        if ((ret = protocol->decode_message(msg, &pkt)) != ERROR_SUCCESS) {
            brs_error("identify decode message failed. ret=%d", ret);
            return ret;
        }
        
        BrsAutoFreeE(BrsPacket, pkt);
        
        if (dynamic_cast<BrsCreateStreamPacket*>(pkt)) {
            brs_info("identify client by create stream, play or flash publish.");
            return identify_create_stream_client(dynamic_cast<BrsCreateStreamPacket*>(pkt), stream_id, type, stream_name, duration);
        }
        if (dynamic_cast<BrsFMLEStartPacket*>(pkt)) {
            brs_info("identify client by releaseStream, fmle publish.");
            return identify_fmle_publish_client(dynamic_cast<BrsFMLEStartPacket*>(pkt), type, stream_name);
        }
        if (dynamic_cast<BrsPlayPacket*>(pkt)) {
            brs_info("level0 identify client by play.");
            return identify_play_client(dynamic_cast<BrsPlayPacket*>(pkt), type, stream_name, duration);
        }
        // call msg,
        // support response null first,
        // @see https://github.com/osbrs/brs/issues/106
        // TODO: FIXME: response in right way, or forward in edge mode.
        BrsCallPacket* call = dynamic_cast<BrsCallPacket*>(pkt);
        if (call) {
            BrsCallResPacket* res = new BrsCallResPacket(call->transaction_id);
            res->command_object = BrsAmf0Any::null();
            res->response = BrsAmf0Any::null();
            if ((ret = protocol->send_and_free_packet(res, 0)) != ERROR_SUCCESS) {
                if (!brs_is_system_control_error(ret) && !brs_is_client_gracefully_close(ret)) {
                    brs_warn("response call failed. ret=%d", ret);
                }
                return ret;
            }
            continue;
        }
        
        brs_trace("ignore AMF0/AMF3 command message.");
    }
    
    return ret;
}

int BrsRtmpServer::set_chunk_size(int chunk_size)
{
    int ret = ERROR_SUCCESS;
    
    BrsSetChunkSizePacket* pkt = new BrsSetChunkSizePacket();
    pkt->chunk_size = chunk_size;
    if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
        brs_error("send set chunk size message failed. ret=%d", ret);
        return ret;
    }
    brs_info("send set chunk size message success. chunk_size=%d", chunk_size);
    
    return ret;
}

int BrsRtmpServer::start_play(int stream_id)
{
    int ret = ERROR_SUCCESS;
    
    // StreamBegin
    if (true) {
        BrsUserControlPacket* pkt = new BrsUserControlPacket();
        pkt->event_type = SrcPCUCStreamBegin;
        pkt->event_data = stream_id;
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            brs_error("send PCUC(StreamBegin) message failed. ret=%d", ret);
            return ret;
        }
        brs_info("send PCUC(StreamBegin) message success.");
    }
    
    // onStatus(NetStream.Play.Reset)
    if (true) {
        BrsOnStatusCallPacket* pkt = new BrsOnStatusCallPacket();
        
        pkt->data->set(StatusLevel, BrsAmf0Any::str(StatusLevelStatus));
        pkt->data->set(StatusCode, BrsAmf0Any::str(StatusCodeStreamReset));
        pkt->data->set(StatusDescription, BrsAmf0Any::str("Playing and resetting stream."));
        pkt->data->set(StatusDetails, BrsAmf0Any::str("stream"));
        pkt->data->set(StatusClientId, BrsAmf0Any::str(RTMP_SIG_CLIENT_ID));
        
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            brs_error("send onStatus(NetStream.Play.Reset) message failed. ret=%d", ret);
            return ret;
        }
        brs_info("send onStatus(NetStream.Play.Reset) message success.");
    }
    
    // onStatus(NetStream.Play.Start)
    if (true) {
        BrsOnStatusCallPacket* pkt = new BrsOnStatusCallPacket();
        
        pkt->data->set(StatusLevel, BrsAmf0Any::str(StatusLevelStatus));
        pkt->data->set(StatusCode, BrsAmf0Any::str(StatusCodeStreamStart));
        pkt->data->set(StatusDescription, BrsAmf0Any::str("Started playing stream."));
        pkt->data->set(StatusDetails, BrsAmf0Any::str("stream"));
        pkt->data->set(StatusClientId, BrsAmf0Any::str(RTMP_SIG_CLIENT_ID));
        
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            brs_error("send onStatus(NetStream.Play.Start) message failed. ret=%d", ret);
            return ret;
        }
        brs_info("send onStatus(NetStream.Play.Start) message success.");
    }
    
    // |RtmpSampleAccess(false, false)
    if (true) {
        BrsSampleAccessPacket* pkt = new BrsSampleAccessPacket();

        // allow audio/video sample.
        // @see: https://github.com/osbrs/brs/issues/49
        pkt->audio_sample_access = true;
        pkt->video_sample_access = true;
        
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            brs_error("send |RtmpSampleAccess(false, false) message failed. ret=%d", ret);
            return ret;
        }
        brs_info("send |RtmpSampleAccess(false, false) message success.");
    }
    
    // onStatus(NetStream.Data.Start)
    if (true) {
        BrsOnStatusDataPacket* pkt = new BrsOnStatusDataPacket();
        pkt->data->set(StatusCode, BrsAmf0Any::str(StatusCodeDataStart));
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            brs_error("send onStatus(NetStream.Data.Start) message failed. ret=%d", ret);
            return ret;
        }
        brs_info("send onStatus(NetStream.Data.Start) message success.");
    }
    
    brs_info("start play success.");
    
    return ret;
}

int BrsRtmpServer::on_play_client_pause(int stream_id, bool is_pause)
{
    int ret = ERROR_SUCCESS;
    
    if (is_pause) {
        // onStatus(NetStream.Pause.Notify)
        if (true) {
            BrsOnStatusCallPacket* pkt = new BrsOnStatusCallPacket();
            
            pkt->data->set(StatusLevel, BrsAmf0Any::str(StatusLevelStatus));
            pkt->data->set(StatusCode, BrsAmf0Any::str(StatusCodeStreamPause));
            pkt->data->set(StatusDescription, BrsAmf0Any::str("Paused stream."));
            
            if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
                brs_error("send onStatus(NetStream.Pause.Notify) message failed. ret=%d", ret);
                return ret;
            }
            brs_info("send onStatus(NetStream.Pause.Notify) message success.");
        }
        // StreamEOF
        if (true) {
            BrsUserControlPacket* pkt = new BrsUserControlPacket();
            
            pkt->event_type = SrcPCUCStreamEOF;
            pkt->event_data = stream_id;
            
            if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
                brs_error("send PCUC(StreamEOF) message failed. ret=%d", ret);
                return ret;
            }
            brs_info("send PCUC(StreamEOF) message success.");
        }
    } else {
        // onStatus(NetStream.Unpause.Notify)
        if (true) {
            BrsOnStatusCallPacket* pkt = new BrsOnStatusCallPacket();
            
            pkt->data->set(StatusLevel, BrsAmf0Any::str(StatusLevelStatus));
            pkt->data->set(StatusCode, BrsAmf0Any::str(StatusCodeStreamUnpause));
            pkt->data->set(StatusDescription, BrsAmf0Any::str("Unpaused stream."));
            
            if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
                brs_error("send onStatus(NetStream.Unpause.Notify) message failed. ret=%d", ret);
                return ret;
            }
            brs_info("send onStatus(NetStream.Unpause.Notify) message success.");
        }
        // StreanBegin
        if (true) {
            BrsUserControlPacket* pkt = new BrsUserControlPacket();
            
            pkt->event_type = SrcPCUCStreamBegin;
            pkt->event_data = stream_id;
            
            if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
                brs_error("send PCUC(StreanBegin) message failed. ret=%d", ret);
                return ret;
            }
            brs_info("send PCUC(StreanBegin) message success.");
        }
    }
    
    return ret;
}

int BrsRtmpServer::start_fmle_publish(int stream_id)
{
    int ret = ERROR_SUCCESS;
    
    // FCPublish
    double fc_publish_tid = 0;
    if (true) {
        BrsCommonMessage* msg = NULL;
        BrsFMLEStartPacket* pkt = NULL;
        if ((ret = expect_message<BrsFMLEStartPacket>(&msg, &pkt)) != ERROR_SUCCESS) {
            brs_error("recv FCPublish message failed. ret=%d", ret);
            return ret;
        }
        brs_info("recv FCPublish request message success.");
        
        BrsAutoFreeE(BrsCommonMessage, msg);
        BrsAutoFreeE(BrsFMLEStartPacket, pkt);
    
        fc_publish_tid = pkt->transaction_id;
    }
    // FCPublish response
    if (true) {
        BrsFMLEStartResPacket* pkt = new BrsFMLEStartResPacket(fc_publish_tid);
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            brs_error("send FCPublish response message failed. ret=%d", ret);
            return ret;
        }
        brs_info("send FCPublish response message success.");
    }
    
    // createStream
    double create_stream_tid = 0;
    if (true) {
        BrsCommonMessage* msg = NULL;
        BrsCreateStreamPacket* pkt = NULL;
        if ((ret = expect_message<BrsCreateStreamPacket>(&msg, &pkt)) != ERROR_SUCCESS) {
            brs_error("recv createStream message failed. ret=%d", ret);
            return ret;
        }
        brs_info("recv createStream request message success.");
        
        BrsAutoFreeE(BrsCommonMessage, msg);
        BrsAutoFreeE(BrsCreateStreamPacket, pkt);
        
        create_stream_tid = pkt->transaction_id;
    }
    // createStream response
    if (true) {
        BrsCreateStreamResPacket* pkt = new BrsCreateStreamResPacket(create_stream_tid, stream_id);
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            brs_error("send createStream response message failed. ret=%d", ret);
            return ret;
        }
        brs_info("send createStream response message success.");
    }
    
    // publish
    if (true) {
        BrsCommonMessage* msg = NULL;
        BrsPublishPacket* pkt = NULL;
        if ((ret = expect_message<BrsPublishPacket>(&msg, &pkt)) != ERROR_SUCCESS) {
            brs_error("recv publish message failed. ret=%d", ret);
            return ret;
        }
        brs_info("recv publish request message success.");
        
        BrsAutoFreeE(BrsCommonMessage, msg);
        BrsAutoFreeE(BrsPublishPacket, pkt);
    }
    // publish response onFCPublish(NetStream.Publish.Start)
    if (true) {
        BrsOnStatusCallPacket* pkt = new BrsOnStatusCallPacket();
        
        pkt->command_name = RTMP_AMF0_COMMAND_ON_FC_PUBLISH;
        pkt->data->set(StatusCode, BrsAmf0Any::str(StatusCodePublishStart));
        pkt->data->set(StatusDescription, BrsAmf0Any::str("Started publishing stream."));
        
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            brs_error("send onFCPublish(NetStream.Publish.Start) message failed. ret=%d", ret);
            return ret;
        }
        brs_info("send onFCPublish(NetStream.Publish.Start) message success.");
    }
    // publish response onStatus(NetStream.Publish.Start)
    if (true) {
        BrsOnStatusCallPacket* pkt = new BrsOnStatusCallPacket();
        
        pkt->data->set(StatusLevel, BrsAmf0Any::str(StatusLevelStatus));
        pkt->data->set(StatusCode, BrsAmf0Any::str(StatusCodePublishStart));
        pkt->data->set(StatusDescription, BrsAmf0Any::str("Started publishing stream."));
        pkt->data->set(StatusClientId, BrsAmf0Any::str(RTMP_SIG_CLIENT_ID));
        
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            brs_error("send onStatus(NetStream.Publish.Start) message failed. ret=%d", ret);
            return ret;
        }
        brs_info("send onStatus(NetStream.Publish.Start) message success.");
    }
    
    brs_info("FMLE publish success.");
    
    return ret;
}

int BrsRtmpServer::fmle_unpublish(int stream_id, double unpublish_tid)
{
    int ret = ERROR_SUCCESS;
    
    // publish response onFCUnpublish(NetStream.unpublish.Success)
    if (true) {
        BrsOnStatusCallPacket* pkt = new BrsOnStatusCallPacket();
        
        pkt->command_name = RTMP_AMF0_COMMAND_ON_FC_UNPUBLISH;
        pkt->data->set(StatusCode, BrsAmf0Any::str(StatusCodeUnpublishSuccess));
        pkt->data->set(StatusDescription, BrsAmf0Any::str("Stop publishing stream."));
        
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            if (!brs_is_system_control_error(ret) && !brs_is_client_gracefully_close(ret)) {
                brs_error("send onFCUnpublish(NetStream.unpublish.Success) message failed. ret=%d", ret);
            }
            return ret;
        }
        brs_info("send onFCUnpublish(NetStream.unpublish.Success) message success.");
    }
    // FCUnpublish response
    if (true) {
        BrsFMLEStartResPacket* pkt = new BrsFMLEStartResPacket(unpublish_tid);
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            if (!brs_is_system_control_error(ret) && !brs_is_client_gracefully_close(ret)) {
                brs_error("send FCUnpublish response message failed. ret=%d", ret);
            }
            return ret;
        }
        brs_info("send FCUnpublish response message success.");
    }
    // publish response onStatus(NetStream.Unpublish.Success)
    if (true) {
        BrsOnStatusCallPacket* pkt = new BrsOnStatusCallPacket();
        
        pkt->data->set(StatusLevel, BrsAmf0Any::str(StatusLevelStatus));
        pkt->data->set(StatusCode, BrsAmf0Any::str(StatusCodeUnpublishSuccess));
        pkt->data->set(StatusDescription, BrsAmf0Any::str("Stream is now unpublished"));
        pkt->data->set(StatusClientId, BrsAmf0Any::str(RTMP_SIG_CLIENT_ID));
        
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            if (!brs_is_system_control_error(ret) && !brs_is_client_gracefully_close(ret)) {
                brs_error("send onStatus(NetStream.Unpublish.Success) message failed. ret=%d", ret);
            }
            return ret;
        }
        brs_info("send onStatus(NetStream.Unpublish.Success) message success.");
    }
    
    brs_info("FMLE unpublish success.");
    
    return ret;
}

int BrsRtmpServer::start_flash_publish(int stream_id)
{
    int ret = ERROR_SUCCESS;
    
    // publish response onStatus(NetStream.Publish.Start)
    if (true) {
        BrsOnStatusCallPacket* pkt = new BrsOnStatusCallPacket();
        
        pkt->data->set(StatusLevel, BrsAmf0Any::str(StatusLevelStatus));
        pkt->data->set(StatusCode, BrsAmf0Any::str(StatusCodePublishStart));
        pkt->data->set(StatusDescription, BrsAmf0Any::str("Started publishing stream."));
        pkt->data->set(StatusClientId, BrsAmf0Any::str(RTMP_SIG_CLIENT_ID));
        
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            brs_error("send onStatus(NetStream.Publish.Start) message failed. ret=%d", ret);
            return ret;
        }
        brs_info("send onStatus(NetStream.Publish.Start) message success.");
    }
    
    brs_info("flash publish success.");
    
    return ret;
}

int BrsRtmpServer::identify_create_stream_client(BrsCreateStreamPacket* req, int stream_id, BrsRtmpConnType& type, string& stream_name, double& duration)
{
    int ret = ERROR_SUCCESS;
    
    if (true) {
        BrsCreateStreamResPacket* pkt = new BrsCreateStreamResPacket(req->transaction_id, stream_id);
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            brs_error("send createStream response message failed. ret=%d", ret);
            return ret;
        }
        brs_info("send createStream response message success.");
    }
    
    while (true) {
        BrsCommonMessage* msg = NULL;
        if ((ret = protocol->recv_message(&msg)) != ERROR_SUCCESS) {
            if (!brs_is_client_gracefully_close(ret)) {
                brs_error("recv identify client message failed. ret=%d", ret);
            }
            return ret;
        }

        BrsAutoFreeE(BrsCommonMessage, msg);
        BrsMessageHeader& h = msg->header;
        
        if (h.is_ackledgement() || h.is_set_chunk_size() || h.is_window_ackledgement_size() || h.is_user_control_message()) {
            continue;
        }
    
        if (!h.is_amf0_command() && !h.is_amf3_command()) {
            brs_trace("identify ignore messages except "
                "AMF0/AMF3 command message. type=%#x", h.message_type);
            continue;
        }
        
        BrsPacket* pkt = NULL;
        if ((ret = protocol->decode_message(msg, &pkt)) != ERROR_SUCCESS) {
            brs_error("identify decode message failed. ret=%d", ret);
            return ret;
        }

        BrsAutoFreeE(BrsPacket, pkt);
        
        if (dynamic_cast<BrsPlayPacket*>(pkt)) {
            brs_info("level1 identify client by play.");
            return identify_play_client(dynamic_cast<BrsPlayPacket*>(pkt), type, stream_name, duration);
        }
        if (dynamic_cast<BrsPublishPacket*>(pkt)) {
            brs_info("identify client by publish, falsh publish.");
            return identify_flash_publish_client(dynamic_cast<BrsPublishPacket*>(pkt), type, stream_name);
        }
        if (dynamic_cast<BrsCreateStreamPacket*>(pkt)) {
            brs_info("identify client by create stream, play or flash publish.");
            return identify_create_stream_client(dynamic_cast<BrsCreateStreamPacket*>(pkt), stream_id, type, stream_name, duration);
        }
        
        brs_trace("ignore AMF0/AMF3 command message.");
    }
    
    return ret;
}

int BrsRtmpServer::identify_fmle_publish_client(BrsFMLEStartPacket* req, BrsRtmpConnType& type, string& stream_name)
{
    int ret = ERROR_SUCCESS;
    
    type = BrsRtmpConnFMLEPublish;
    stream_name = req->stream_name;
    
    // releaseStream response
    if (true) {
        BrsFMLEStartResPacket* pkt = new BrsFMLEStartResPacket(req->transaction_id);
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            brs_error("send releaseStream response message failed. ret=%d", ret);
            return ret;
        }
        brs_info("send releaseStream response message success.");
    }
    
    return ret;
}

int BrsRtmpServer::identify_flash_publish_client(BrsPublishPacket* req, BrsRtmpConnType& type, string& stream_name)
{
    int ret = ERROR_SUCCESS;
    
    type = BrsRtmpConnFlashPublish;
    stream_name = req->stream_name;
    
    return ret;
}

int BrsRtmpServer::identify_play_client(BrsPlayPacket* req, BrsRtmpConnType& type, string& stream_name, double& duration)
{
    int ret = ERROR_SUCCESS;
    
    type = BrsRtmpConnPlay;
    stream_name = req->stream_name;
    duration = req->duration;
    
    brs_info("identity client type=play, stream_name=%s, duration=%.2f", stream_name.c_str(), duration);

    return ret;
}

}