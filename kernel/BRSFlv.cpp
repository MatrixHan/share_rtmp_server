#include <BRSFlv.h>

// for brs-librtmp, @see https://github.com/osbrs/brs/issues/213
#ifndef _WIN32
#include <unistd.h>
#endif

#include <fcntl.h>
#include <sstream>
using namespace std;

#include <BRSLog.h>
#include <BRSKernelError.h>
#include <BRSStream.h>
#include <BRSFile.h>
#include <BRSKernelCodec.h>
#include <BRSKernelUtility.h>


namespace BRS {

BrsMessageHeader::BrsMessageHeader()
{
    message_type = 0;
    payload_length = 0;
    timestamp_delta = 0;
    stream_id = 0;
    
    timestamp = 0;
    // we always use the connection chunk-id
    perfer_cid = RTMP_CID_OverConnection;
}

BrsMessageHeader::~BrsMessageHeader()
{
}

bool BrsMessageHeader::is_audio()
{
    return message_type == RTMP_MSG_AudioMessage;
}

bool BrsMessageHeader::is_video()
{
    return message_type == RTMP_MSG_VideoMessage;
}

bool BrsMessageHeader::is_amf0_command()
{
    return message_type == RTMP_MSG_AMF0CommandMessage;
}

bool BrsMessageHeader::is_amf0_data()
{
    return message_type == RTMP_MSG_AMF0DataMessage;
}

bool BrsMessageHeader::is_amf3_command()
{
    return message_type == RTMP_MSG_AMF3CommandMessage;
}

bool BrsMessageHeader::is_amf3_data()
{
    return message_type == RTMP_MSG_AMF3DataMessage;
}

bool BrsMessageHeader::is_window_ackledgement_size()
{
    return message_type == RTMP_MSG_WindowAcknowledgementSize;
}

bool BrsMessageHeader::is_ackledgement()
{
    return message_type == RTMP_MSG_Acknowledgement;
}

bool BrsMessageHeader::is_set_chunk_size()
{
    return message_type == RTMP_MSG_SetChunkSize;
}

bool BrsMessageHeader::is_user_control_message()
{
    return message_type == RTMP_MSG_UserControlMessage;
}

bool BrsMessageHeader::is_set_peer_bandwidth()
{
    return message_type == RTMP_MSG_SetPeerBandwidth;
}

bool BrsMessageHeader::is_aggregate()
{
    return message_type == RTMP_MSG_AggregateMessage;
}

void BrsMessageHeader::initialize_amf0_script(int size, int stream)
{
    message_type = RTMP_MSG_AMF0DataMessage;
    payload_length = (int32_t)size;
    timestamp_delta = (int32_t)0;
    timestamp = (int64_t)0;
    stream_id = (int32_t)stream;
    
    // amf0 script use connection2 chunk-id
    perfer_cid = RTMP_CID_OverConnection2;
}

void BrsMessageHeader::initialize_audio(int size, u_int32_t time, int stream)
{
    message_type = RTMP_MSG_AudioMessage;
    payload_length = (int32_t)size;
    timestamp_delta = (int32_t)time;
    timestamp = (int64_t)time;
    stream_id = (int32_t)stream;
    
    // audio chunk-id
    perfer_cid = RTMP_CID_Audio;
}

void BrsMessageHeader::initialize_video(int size, u_int32_t time, int stream)
{
    message_type = RTMP_MSG_VideoMessage;
    payload_length = (int32_t)size;
    timestamp_delta = (int32_t)time;
    timestamp = (int64_t)time;
    stream_id = (int32_t)stream;
    
    // video chunk-id
    perfer_cid = RTMP_CID_Video;
}

BrsCommonMessage::BrsCommonMessage()
{
    payload = NULL;
    size = 0;
}

BrsCommonMessage::~BrsCommonMessage()
{
#ifdef BRS_AUTO_MEM_WATCH
    brs_memory_unwatch(payload);
#endif
    SafeDeleteArray(payload);
}

void BrsCommonMessage::create_payload(int size)
{
    SafeDeleteArray(payload);
    
    payload = new char[size];
    brs_verbose("create payload for RTMP message. size=%d", size);
    
#ifdef BRS_AUTO_MEM_WATCH
    brs_memory_watch(payload, "RTMP.msg.payload", size);
#endif
}

BrsSharedPtrMessage::BrsSharedPtrPayload::BrsSharedPtrPayload()
{
    payload = NULL;
    size = 0;
    shared_count = 0;
}

BrsSharedPtrMessage::BrsSharedPtrPayload::~BrsSharedPtrPayload()
{
#ifdef BRS_AUTO_MEM_WATCH
    brs_memory_unwatch(payload);
#endif
    SafeDeleteArray(payload);
}

BrsSharedPtrMessage::BrsSharedPtrMessage()
{
    ptr = NULL;
}

BrsSharedPtrMessage::~BrsSharedPtrMessage()
{
    if (ptr) {
        if (ptr->shared_count == 0) {
            SafeDelete(ptr);
        } else {
            ptr->shared_count--;
        }
    }
}

int BrsSharedPtrMessage::create(BrsCommonMessage* msg)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = create(&msg->header, msg->payload, msg->size)) != ERROR_SUCCESS) {
        return ret;
    }
    
    // to prevent double free of payload:
    // initialize already attach the payload of msg,
    // detach the payload to transfer the owner to shared ptr.
    msg->payload = NULL;
    msg->size = 0;
    
    return ret;
}

int BrsSharedPtrMessage::create(BrsMessageHeader* pheader, char* payload, int size)
{
    int ret = ERROR_SUCCESS;
    
    if (ptr) {
        ret = ERROR_SYSTEM_ASSERT_FAILED;
        brs_error("should not set the payload twice. ret=%d", ret);
        assert(false);
        
        return ret;
    }
    
    ptr = new BrsSharedPtrPayload();
    
    // direct attach the data.
    if (pheader) {
        ptr->header.message_type = pheader->message_type;
        ptr->header.payload_length = size;
        ptr->header.perfer_cid = pheader->perfer_cid;
        this->timestamp = pheader->timestamp;
        this->stream_id = pheader->stream_id;
    }
    ptr->payload = payload;
    ptr->size = size;
    
    // message can access it.
    this->payload = ptr->payload;
    this->size = ptr->size;
    
    return ret;
}

int BrsSharedPtrMessage::count()
{
    assert(ptr);
    return ptr->shared_count;
}

bool BrsSharedPtrMessage::check(int stream_id)
{
    // we donot use the complex basic header,
    // ensure the basic header is 1bytes.
    if (ptr->header.perfer_cid < 2) {
        brs_info("change the chunk_id=%d to default=%d",
            ptr->header.perfer_cid, RTMP_CID_ProtocolControl);
        ptr->header.perfer_cid = RTMP_CID_ProtocolControl;
    }
    
    // we assume that the stream_id in a group must be the same.
    if (this->stream_id == stream_id) {
        return true;
    }
    this->stream_id = stream_id;
    
    return false;
}

bool BrsSharedPtrMessage::is_av()
{
    return ptr->header.message_type == RTMP_MSG_AudioMessage
        || ptr->header.message_type == RTMP_MSG_VideoMessage;
}

bool BrsSharedPtrMessage::is_audio()
{
    return ptr->header.message_type == RTMP_MSG_AudioMessage;
}

bool BrsSharedPtrMessage::is_video()
{
    return ptr->header.message_type == RTMP_MSG_VideoMessage;
}

int BrsSharedPtrMessage::chunk_header(char* cache, int nb_cache, bool c0)
{
    if (c0) {
        return brs_chunk_header_c0(
            ptr->header.perfer_cid, timestamp, ptr->header.payload_length,
            ptr->header.message_type, stream_id,
            cache, nb_cache);
    } else {
        return brs_chunk_header_c3(
            ptr->header.perfer_cid, timestamp,
            cache, nb_cache);
    }
}

BrsSharedPtrMessage* BrsSharedPtrMessage::copy()
{
    assert(ptr);
    
    BrsSharedPtrMessage* copy = new BrsSharedPtrMessage();
    
    copy->ptr = ptr;
    ptr->shared_count++;
    
    copy->timestamp = timestamp;
    copy->stream_id = stream_id;
    copy->payload = ptr->payload;
    copy->size = ptr->size;
    
    return copy;
}

BrsFlvEncoder::BrsFlvEncoder()
{
    reader = NULL;
    tag_stream = new BRSStream();
    
#ifdef BRS_PERF_FAST_FLV_ENCODER
    nb_tag_headers = 0;
    tag_headers = NULL;
    nb_iovss_cache = 0;
    iovss_cache = NULL;
    nb_ppts = 0;
    ppts = NULL;
#endif
}

BrsFlvEncoder::~BrsFlvEncoder()
{
    SafeDelete(tag_stream);
    
#ifdef BRS_PERF_FAST_FLV_ENCODER
    SafeDeleteArray(tag_headers);
    SafeDeleteArray(iovss_cache);
    SafeDeleteArray(ppts);
#endif
}

int BrsFlvEncoder::initialize(BrsFileWriter* fr)
{
    int ret = ERROR_SUCCESS;
    
    assert(fr);
    
    if (!fr->is_open()) {
        ret = ERROR_KERNEL_FLV_STREAM_CLOSED;
        brs_warn("stream is not open for encoder. ret=%d", ret);
        return ret;
    }
    
    reader = fr;
    
    return ret;
}

int BrsFlvEncoder::write_header()
{
    int ret = ERROR_SUCCESS;
    
    // 9bytes header and 4bytes first previous-tag-size
    char flv_header[] = {
        'F', 'L', 'V', // Signatures "FLV"
        (char)0x01, // File version (for example, 0x01 for FLV version 1)
        (char)0x05, // 4, audio; 1, video; 5 audio+video.
        (char)0x00, (char)0x00, (char)0x00, (char)0x09 // DataOffset UI32 The length of this header in bytes
    };
    
    // flv specification should set the audio and video flag,
    // actually in practise, application generally ignore this flag,
    // so we generally set the audio/video to 0.
    
    // write 9bytes header.
    if ((ret = write_header(flv_header)) != ERROR_SUCCESS) {
        return ret;
    }
    
    return ret;
}

int BrsFlvEncoder::write_header(char flv_header[9])
{
    int ret = ERROR_SUCCESS;
    
    // write data.
    if ((ret = reader->write(flv_header, 9, NULL)) != ERROR_SUCCESS) {
        brs_error("write flv header failed. ret=%d", ret);
        return ret;
    }
    
    // previous tag size.
    char pts[] = { (char)0x00, (char)0x00, (char)0x00, (char)0x00 };
    if ((ret = reader->write(pts, 4, NULL)) != ERROR_SUCCESS) {
        return ret;
    }
    
    return ret;
}

int BrsFlvEncoder::write_metadata(char type, char* data, int size)
{
    int ret = ERROR_SUCCESS;
    
    assert(data);
    
    if ((ret = write_metadata_to_cache(type, data, size, tag_header)) != ERROR_SUCCESS) {
        return ret;
    }
    
    if ((ret = write_tag(tag_header, sizeof(tag_header), data, size)) != ERROR_SUCCESS) {
        if (!brs_is_client_gracefully_close(ret)) {
            brs_error("write flv data tag failed. ret=%d", ret);
        }
        return ret;
    }
    
    return ret;
}

int BrsFlvEncoder::write_audio(int64_t timestamp, char* data, int size)
{
    int ret = ERROR_SUCCESS;
    
    assert(data);
    
    if ((ret = write_audio_to_cache(timestamp, data, size, tag_header)) != ERROR_SUCCESS) {
        return ret;
    }
    
    if ((ret = write_tag(tag_header, sizeof(tag_header), data, size)) != ERROR_SUCCESS) {
        if (!brs_is_client_gracefully_close(ret)) {
            brs_error("write flv audio tag failed. ret=%d", ret);
        }
        return ret;
    }
    
    return ret;
}

int BrsFlvEncoder::write_video(int64_t timestamp, char* data, int size)
{
    int ret = ERROR_SUCCESS;
    
    assert(data);
    
    if ((ret = write_video_to_cache(timestamp, data, size, tag_header)) != ERROR_SUCCESS) {
        return ret;
    }
    
    if ((ret = write_tag(tag_header, sizeof(tag_header), data, size)) != ERROR_SUCCESS) {
        brs_error("write flv video tag failed. ret=%d", ret);
        return ret;
    }
    
    return ret;
}

int BrsFlvEncoder::size_tag(int data_size)
{
    assert(data_size >= 0);
    return BRS_FLV_TAG_HEADER_SIZE + data_size + BRS_FLV_PREVIOUS_TAG_SIZE;
}

#ifdef BRS_PERF_FAST_FLV_ENCODER
int BrsFlvEncoder::write_tags(BrsSharedPtrMessage** msgs, int count)
{
    int ret = ERROR_SUCCESS;
    
    // realloc the iovss.
    int nb_iovss = 3 * count;
    iovec* iovss = iovss_cache;
    if (nb_iovss_cache < nb_iovss) {
        SafeDeleteArray(iovss_cache);
        
        nb_iovss_cache = nb_iovss;
        iovss = iovss_cache = new iovec[nb_iovss];
    }
    
    // realloc the tag headers.
    char* cache = tag_headers;
    if (nb_tag_headers < count) {
        SafeDeleteArray(tag_headers);
        
        nb_tag_headers = count;
        cache = tag_headers = new char[BRS_FLV_TAG_HEADER_SIZE * count];
    }
    
    // realloc the pts.
    char* pts = ppts;
    if (nb_ppts < count) {
        SafeDeleteArray(ppts);
        
        nb_ppts = count;
        pts = ppts = new char[BRS_FLV_PREVIOUS_TAG_SIZE * count];
    }
    
    // the cache is ok, write each messages.
    iovec* iovs = iovss;
    for (int i = 0; i < count; i++) {
        BrsSharedPtrMessage* msg = msgs[i];
        
        // cache all flv header.
        if (msg->is_audio()) {
            if ((ret = write_audio_to_cache(msg->timestamp, msg->payload, msg->size, cache)) != ERROR_SUCCESS) {
                return ret;
            }
        } else if (msg->is_video()) {
            if ((ret = write_video_to_cache(msg->timestamp, msg->payload, msg->size, cache)) != ERROR_SUCCESS) {
                return ret;
            }
        } else {
            if ((ret = write_metadata_to_cache(SrsCodecFlvTagScript, msg->payload, msg->size, cache)) != ERROR_SUCCESS) {
                return ret;
            }
        }
        
        // cache all pts.
        if ((ret = write_pts_to_cache(BRS_FLV_TAG_HEADER_SIZE + msg->size, pts)) != ERROR_SUCCESS) {
            return ret;
        }
        
        // all ioves.
        iovs[0].iov_base = cache;
        iovs[0].iov_len = BRS_FLV_TAG_HEADER_SIZE;
        iovs[1].iov_base = msg->payload;
        iovs[1].iov_len = msg->size;
        iovs[2].iov_base = pts;
        iovs[2].iov_len = BRS_FLV_PREVIOUS_TAG_SIZE;
        
        // move next.
        cache += BRS_FLV_TAG_HEADER_SIZE;
        pts += BRS_FLV_PREVIOUS_TAG_SIZE;
        iovs += 3;
    }
    
    if ((ret = reader->writev(iovss, nb_iovss, NULL)) != ERROR_SUCCESS) {
        if (!brs_is_client_gracefully_close(ret)) {
            brs_error("write flv tags failed. ret=%d", ret);
        }
        return ret;
    }
    
    return ret;
}
#endif

int BrsFlvEncoder::write_metadata_to_cache(char type, char* data, int size, char* cache)
{
    int ret = ERROR_SUCCESS;
    
    assert(data);
    
    // 11 bytes tag header
    /*char tag_header[] = {
     (char)type, // TagType UB [5], 18 = script data
     (char)0x00, (char)0x00, (char)0x00, // DataSize UI24 Length of the message.
     (char)0x00, (char)0x00, (char)0x00, // Timestamp UI24 Time in milliseconds at which the data in this tag applies.
     (char)0x00, // TimestampExtended UI8
     (char)0x00, (char)0x00, (char)0x00, // StreamID UI24 Always 0.
     };*/
    
    // write data size.
    if ((ret = tag_stream->initialize(cache, 11)) != ERROR_SUCCESS) {
        return ret;
    }
    tag_stream->write_1bytes(type);
    tag_stream->write_3bytes(size);
    tag_stream->write_3bytes(0x00);
    tag_stream->write_1bytes(0x00);
    tag_stream->write_3bytes(0x00);
    
    return ret;
}

int BrsFlvEncoder::write_audio_to_cache(int64_t timestamp, char* data, int size, char* cache)
{
    int ret = ERROR_SUCCESS;
    
    assert(data);
    
    timestamp &= 0x7fffffff;
    
    // 11bytes tag header
    /*char tag_header[] = {
     (char)SrsCodecFlvTagAudio, // TagType UB [5], 8 = audio
     (char)0x00, (char)0x00, (char)0x00, // DataSize UI24 Length of the message.
     (char)0x00, (char)0x00, (char)0x00, // Timestamp UI24 Time in milliseconds at which the data in this tag applies.
     (char)0x00, // TimestampExtended UI8
     (char)0x00, (char)0x00, (char)0x00, // StreamID UI24 Always 0.
     };*/
    
    // write data size.
    if ((ret = tag_stream->initialize(cache, 11)) != ERROR_SUCCESS) {
        return ret;
    }
    tag_stream->write_1bytes(BrsCodecFlvTagAudio);
    tag_stream->write_3bytes(size);
    tag_stream->write_3bytes((int32_t)timestamp);
    // default to little-endian
    tag_stream->write_1bytes((timestamp >> 24) & 0xFF);
    tag_stream->write_3bytes(0x00);
    
    return ret;
}

int BrsFlvEncoder::write_video_to_cache(int64_t timestamp, char* data, int size, char* cache)
{
    int ret = ERROR_SUCCESS;
    
    assert(data);
    
    timestamp &= 0x7fffffff;
    
    // 11bytes tag header
    /*char tag_header[] = {
     (char)SrsCodecFlvTagVideo, // TagType UB [5], 9 = video
     (char)0x00, (char)0x00, (char)0x00, // DataSize UI24 Length of the message.
     (char)0x00, (char)0x00, (char)0x00, // Timestamp UI24 Time in milliseconds at which the data in this tag applies.
     (char)0x00, // TimestampExtended UI8
     (char)0x00, (char)0x00, (char)0x00, // StreamID UI24 Always 0.
     };*/
    
    // write data size.
    if ((ret = tag_stream->initialize(cache, 11)) != ERROR_SUCCESS) {
        return ret;
    }
    tag_stream->write_1bytes(BrsCodecFlvTagVideo);
    tag_stream->write_3bytes(size);
    tag_stream->write_3bytes((int32_t)timestamp);
    // default to little-endian
    tag_stream->write_1bytes((timestamp >> 24) & 0xFF);
    tag_stream->write_3bytes(0x00);
    
    return ret;
}

int BrsFlvEncoder::write_pts_to_cache(int size, char* cache)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = tag_stream->initialize(cache, BRS_FLV_PREVIOUS_TAG_SIZE)) != ERROR_SUCCESS) {
        return ret;
    }
    tag_stream->write_4bytes(size);
    
    return ret;
}

int BrsFlvEncoder::write_tag(char* header, int header_size, char* tag, int tag_size)
{
    int ret = ERROR_SUCCESS;
    
    // PreviousTagSizeN UI32 Size of last tag, including its header, in bytes.
    char pre_size[BRS_FLV_PREVIOUS_TAG_SIZE];
    if ((ret = write_pts_to_cache(tag_size + header_size, pre_size)) != ERROR_SUCCESS) {
        return ret;
    }
    
    iovec iovs[3];
    iovs[0].iov_base = header;
    iovs[0].iov_len = header_size;
    iovs[1].iov_base = tag;
    iovs[1].iov_len = tag_size;
    iovs[2].iov_base = pre_size;
    iovs[2].iov_len = BRS_FLV_PREVIOUS_TAG_SIZE;
    
    if ((ret = reader->writev(iovs, 3, NULL)) != ERROR_SUCCESS) {
        if (!brs_is_client_gracefully_close(ret)) {
            brs_error("write flv tag failed. ret=%d", ret);
        }
        return ret;
    }
    
    return ret;
}

SrsFlvDecoder::SrsFlvDecoder()
{
    reader = NULL;
    tag_stream = new BRSStream();
}

SrsFlvDecoder::~SrsFlvDecoder()
{
    SafeDelete(tag_stream);
}

int SrsFlvDecoder::initialize(BrsFileReader* fr)
{
    int ret = ERROR_SUCCESS;
    
    assert(fr);
    
    if (!fr->is_open()) {
        ret = ERROR_KERNEL_FLV_STREAM_CLOSED;
        brs_warn("stream is not open for decoder. ret=%d", ret);
        return ret;
    }
    
    reader = fr;
    
    return ret;
}

int SrsFlvDecoder::read_header(char header[9])
{
    int ret = ERROR_SUCCESS;

    assert(header);
    
    if ((ret = reader->read(header, 9, NULL)) != ERROR_SUCCESS) {
        return ret;
    }
    
    char* h = header;
    if (h[0] != 'F' || h[1] != 'L' || h[2] != 'V') {
        ret = ERROR_KERNEL_FLV_HEADER;
        brs_warn("flv header must start with FLV. ret=%d", ret);
        return ret;
    }
    
    return ret;
}

int SrsFlvDecoder::read_tag_header(char* ptype, int32_t* pdata_size, u_int32_t* ptime)
{
    int ret = ERROR_SUCCESS;

    assert(ptype);
    assert(pdata_size);
    assert(ptime);

    char th[11]; // tag header
    
    // read tag header
    if ((ret = reader->read(th, 11, NULL)) != ERROR_SUCCESS) {
        if (ret != ERROR_SYSTEM_FILE_EOF) {
            brs_error("read flv tag header failed. ret=%d", ret);
        }
        return ret;
    }
    
    // Reserved UB [2]
    // Filter UB [1]
    // TagType UB [5]
    *ptype = (th[0] & 0x1F);
    
    // DataSize UI24
    char* pp = (char*)pdata_size;
    pp[3] = 0;
    pp[2] = th[1];
    pp[1] = th[2];
    pp[0] = th[3];
    
    // Timestamp UI24
    pp = (char*)ptime;
    pp[2] = th[4];
    pp[1] = th[5];
    pp[0] = th[6];
    
    // TimestampExtended UI8
    pp[3] = th[7];

    return ret;
}

int SrsFlvDecoder::read_tag_data(char* data, int32_t size)
{
    int ret = ERROR_SUCCESS;

    assert(data);
    
    if ((ret = reader->read(data, size, NULL)) != ERROR_SUCCESS) {
        if (ret != ERROR_SYSTEM_FILE_EOF) {
            brs_error("read flv tag header failed. ret=%d", ret);
        }
        return ret;
    }
    
    return ret;

}

int SrsFlvDecoder::read_previous_tag_size(char previous_tag_size[4])
{
    int ret = ERROR_SUCCESS;

    assert(previous_tag_size);
    
    // ignore 4bytes tag size.
    if ((ret = reader->read(previous_tag_size, 4, NULL)) != ERROR_SUCCESS) {
        if (ret != ERROR_SYSTEM_FILE_EOF) {
            brs_error("read flv previous tag size failed. ret=%d", ret);
        }
        return ret;
    }
    
    return ret;
}

SrsFlvVodStreamDecoder::SrsFlvVodStreamDecoder()
{
    reader = NULL;
    tag_stream = new BRSStream();
}

SrsFlvVodStreamDecoder::~SrsFlvVodStreamDecoder()
{
    SafeDelete(tag_stream);
}

int SrsFlvVodStreamDecoder::initialize(BrsFileReader* fr)
{
    int ret = ERROR_SUCCESS;
    
    assert(fr);
    
    if (!fr->is_open()) {
        ret = ERROR_KERNEL_FLV_STREAM_CLOSED;
        brs_warn("stream is not open for decoder. ret=%d", ret);
        return ret;
    }
    
    reader = fr;
    
    return ret;
}

int SrsFlvVodStreamDecoder::read_header_ext(char header[13])
{
    int ret = ERROR_SUCCESS;

    assert(header);
    
    // @remark, always false, for sizeof(char[13]) equals to sizeof(char*)
    //assert(13 == sizeof(header));
    
    // 9bytes header and 4bytes first previous-tag-size
    int size = 13;
    
    if ((ret = reader->read(header, size, NULL)) != ERROR_SUCCESS) {
        return ret;
    }
    
    return ret;
}

int SrsFlvVodStreamDecoder::read_sequence_header_summary(int64_t* pstart, int* psize)
{
    int ret = ERROR_SUCCESS;

    assert(pstart);
    assert(psize);
    
    // simply, the first video/audio must be the sequence header.
    // and must be a sequence video and audio.
    
    // 11bytes tag header
    char tag_header[] = {
        (char)0x00, // TagType UB [5], 9 = video, 8 = audio, 18 = script data
        (char)0x00, (char)0x00, (char)0x00, // DataSize UI24 Length of the message.
        (char)0x00, (char)0x00, (char)0x00, // Timestamp UI24 Time in milliseconds at which the data in this tag applies.
        (char)0x00, // TimestampExtended UI8
        (char)0x00, (char)0x00, (char)0x00, // StreamID UI24 Always 0.
    };
    
    // discovery the sequence header video and audio.
    // @remark, maybe no video or no audio.
    bool got_video = false;
    bool got_audio = false;
    // audio/video sequence and data offset.
    int64_t av_sequence_offset_start = -1;
    int64_t av_sequence_offset_end = -1;
    for (;;) {
        if ((ret = reader->read(tag_header, BRS_FLV_TAG_HEADER_SIZE, NULL)) != ERROR_SUCCESS) {
            return ret;
        }
        
        if ((ret = tag_stream->initialize(tag_header, BRS_FLV_TAG_HEADER_SIZE)) != ERROR_SUCCESS) {
            return ret;
        }
        
        int8_t tag_type = tag_stream->read_1bytes();
        int32_t data_size = tag_stream->read_3bytes();
        
        bool is_video = tag_type == 0x09;
        bool is_audio = tag_type == 0x08;
        bool is_not_av = !is_video && !is_audio;
        if (is_not_av) {
            // skip body and tag size.
            reader->skip(data_size + BRS_FLV_PREVIOUS_TAG_SIZE);
            continue;
        }
        
        // if video duplicated, no audio
        if (is_video && got_video) {
            break;
        }
        // if audio duplicated, no video
        if (is_audio && got_audio) {
            break;
        }
        
        // video
        if (is_video) {
            assert(!got_video);
            got_video = true;
            
            if (av_sequence_offset_start < 0) {
                av_sequence_offset_start = reader->tellg() - BRS_FLV_TAG_HEADER_SIZE;
            }
            av_sequence_offset_end = reader->tellg() + data_size + BRS_FLV_PREVIOUS_TAG_SIZE;
            reader->skip(data_size + BRS_FLV_PREVIOUS_TAG_SIZE);
        }
        
        // audio
        if (is_audio) {
            assert(!got_audio);
            got_audio = true;
            
            if (av_sequence_offset_start < 0) {
                av_sequence_offset_start = reader->tellg() - BRS_FLV_TAG_HEADER_SIZE;
            }
            av_sequence_offset_end = reader->tellg() + data_size + BRS_FLV_PREVIOUS_TAG_SIZE;
            reader->skip(data_size + BRS_FLV_PREVIOUS_TAG_SIZE);
        }
    }
    
    // seek to the sequence header start offset.
    if (av_sequence_offset_start > 0) {
        reader->lseek(av_sequence_offset_start);
        *pstart = av_sequence_offset_start;
        *psize = (int)(av_sequence_offset_end - av_sequence_offset_start);
    }
    
    return ret;
}

int SrsFlvVodStreamDecoder::lseek(int64_t offset)
{
    int ret = ERROR_SUCCESS;
    
    if (offset >= reader->filesize()) {
        ret = ERROR_SYSTEM_FILE_EOF;
        brs_warn("flv fast decoder seek overflow file, "
            "size=%"PRId64", offset=%"PRId64", ret=%d", 
            reader->filesize(), offset, ret);
        return ret;
    }
    
    if (reader->lseek(offset) < 0) {
        ret = ERROR_SYSTEM_FILE_SEEK;
        brs_warn("flv fast decoder seek error, "
            "size=%"PRId64", offset=%"PRId64", ret=%d", 
            reader->filesize(), offset, ret);
        return ret;
    }
    
    return ret;
}

}
