#pragma once 

#include <BRSCommon.h>
#include <BRSRtmpStack.h>
#include <BRSRtmpAmf0.h>

namespace BRS 
{
// FMLE
#define RTMP_AMF0_COMMAND_ON_FC_PUBLISH         "onFCPublish"
#define RTMP_AMF0_COMMAND_ON_FC_UNPUBLISH       "onFCUnpublish"

// default stream id for response the createStream request.
#define BRS_DEFAULT_SID                         1

// when got a messae header, there must be some data,
// increase recv timeout to got an entire message.
#define BRS_MIN_RECV_TIMEOUT_US (int64_t)(60*1000*1000LL)

/****************************************************************************
*****************************************************************************
****************************************************************************/
/**
* 6.1.2. Chunk Message Header
* There are four different formats for the chunk message header,
* selected by the "fmt" field in the chunk basic header.
*/
// 6.1.2.1. Type 0
// Chunks of Type 0 are 11 bytes long. This type MUST be used at the
// start of a chunk stream, and whenever the stream timestamp goes
// backward (e.g., because of a backward seek).
#define RTMP_FMT_TYPE0                          0
// 6.1.2.2. Type 1
// Chunks of Type 1 are 7 bytes long. The message stream ID is not
// included; this chunk takes the same stream ID as the preceding chunk.
// Streams with variable-sized messages (for example, many video
// formats) SHOULD use this format for the first chunk of each new
// message after the first.
#define RTMP_FMT_TYPE1                          1
// 6.1.2.3. Type 2
// Chunks of Type 2 are 3 bytes long. Neither the stream ID nor the
// message length is included; this chunk has the same stream ID and
// message length as the preceding chunk. Streams with constant-sized
// messages (for example, some audio and data formats) SHOULD use this
// format for the first chunk of each message after the first.
#define RTMP_FMT_TYPE2                          2
// 6.1.2.4. Type 3
// Chunks of Type 3 have no header. Stream ID, message length and
// timestamp delta are not present; chunks of this type take values from
// the preceding chunk. When a single message is split into chunks, all
// chunks of a message except the first one, SHOULD use this type. Refer
// to example 2 in section 6.2.2. Stream consisting of messages of
// exactly the same size, stream ID and spacing in time SHOULD use this
// type for all chunks after chunk of Type 2. Refer to example 1 in
// section 6.2.1. If the delta between the first message and the second
// message is same as the time stamp of first message, then chunk of
// type 3 would immediately follow the chunk of type 0 as there is no
// need for a chunk of type 2 to register the delta. If Type 3 chunk
// follows a Type 0 chunk, then timestamp delta for this Type 3 chunk is
// the same as the timestamp of Type 0 chunk.
#define RTMP_FMT_TYPE3                          3

/****************************************************************************
*****************************************************************************
****************************************************************************/
/**
* band width check method name, which will be invoked by client.
* band width check mothods use SrsBandwidthPacket as its internal packet type,
* so ensure you set command name when you use it.
*/
// server play control
#define BRS_BW_CHECK_START_PLAY                 "onSrsBandCheckStartPlayBytes"
#define BRS_BW_CHECK_STARTING_PLAY              "onSrsBandCheckStartingPlayBytes"
#define BRS_BW_CHECK_STOP_PLAY                  "onSrsBandCheckStopPlayBytes"
#define BRS_BW_CHECK_STOPPED_PLAY               "onSrsBandCheckStoppedPlayBytes"

// server publish control
#define BRS_BW_CHECK_START_PUBLISH              "onSrsBandCheckStartPublishBytes"
#define BRS_BW_CHECK_STARTING_PUBLISH           "onSrsBandCheckStartingPublishBytes"
#define BRS_BW_CHECK_STOP_PUBLISH               "onSrsBandCheckStopPublishBytes"
// @remark, flash never send out this packet, for its queue is full.
#define BRS_BW_CHECK_STOPPED_PUBLISH            "onSrsBandCheckStoppedPublishBytes"

// EOF control.
// the report packet when check finished.
#define BRS_BW_CHECK_FINISHED                   "onSrsBandCheckFinished"
// @remark, flash never send out this packet, for its queue is full.
#define BRS_BW_CHECK_FINAL                      "finalClientPacket"

// data packets
#define BRS_BW_CHECK_PLAYING                    "onSrsBandCheckPlaying"
#define BRS_BW_CHECK_PUBLISHING                 "onSrsBandCheckPublishing"

/****************************************************************************
*****************************************************************************
****************************************************************************/

/**
* 4.1.1. connect
* The client sends the connect command to the server to request
* connection to a server application instance.
*/
class BrsConnectAppPacket : public BrsPacket
{
public:
    /**
    * Name of the command. Set to "connect".
    */
    std::string command_name;
    /**
    * Always set to 1.
    */
    double transaction_id;
    /**
    * Command information object which has the name-value pairs.
    * @remark: alloc in packet constructor, user can directly use it, 
    *       user should never alloc it again which will cause memory leak.
    * @remark, never be NULL.
    */
    BrsAmf0Object* command_object;
    /**
    * Any optional information
    * @remark, optional, init to and maybe NULL.
    */
    BrsAmf0Object* args;
public:
    BrsConnectAppPacket();
    virtual ~BrsConnectAppPacket();
// decode functions for concrete packet to override.
public:
    virtual int decode(BRSStream* stream);
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};
/**
* response for BrsConnectAppPacket.
*/
class BrsConnectAppResPacket : public BrsPacket
{
public:
    /**
    * _result or _error; indicates whether the response is result or error.
    */
    std::string command_name;
    /**
    * Transaction ID is 1 for call connect responses
    */
    double transaction_id;
    /**
    * Name-value pairs that describe the properties(fmsver etc.) of the connection.
    * @remark, never be NULL.
    */
    BrsAmf0Object* props;
    /**
    * Name-value pairs that describe the response from|the server. 'code',
    * 'level', 'description' are names of few among such information.
    * @remark, never be NULL.
    */
    BrsAmf0Object* info;
public:
    BrsConnectAppResPacket();
    virtual ~BrsConnectAppResPacket();
// decode functions for concrete packet to override.
public:
    virtual int decode(BRSStream* stream);
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};

/**
* 4.1.2. Call
* The call method of the NetConnection object runs remote procedure
* calls (RPC) at the receiving end. The called RPC name is passed as a
* parameter to the call command.
*/
class BrsCallPacket : public BrsPacket
{
public:
    /**
    * Name of the remote procedure that is called.
    */
    std::string command_name;
    /**
    * If a response is expected we give a transaction Id. Else we pass a value of 0
    */
    double transaction_id;
    /**
    * If there exists any command info this
    * is set, else this is set to null type.
    * @remark, optional, init to and maybe NULL.
    */
    BrsAmf0Any* command_object;
    /**
    * Any optional arguments to be provided
    * @remark, optional, init to and maybe NULL.
    */
    BrsAmf0Any* arguments;
public:
    BrsCallPacket();
    virtual ~BrsCallPacket();
// decode functions for concrete packet to override.
public:
    virtual int decode(BRSStream* stream);
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};
/**
* response for BrsCallPacket.
*/
class BrsCallResPacket : public BrsPacket
{
public:
    /**
    * Name of the command. 
    */
    std::string command_name;
    /**
    * ID of the command, to which the response belongs to
    */
    double transaction_id;
    /**
    * If there exists any command info this is set, else this is set to null type.
    * @remark, optional, init to and maybe NULL.
    */
    BrsAmf0Any* command_object;
    /**
    * Response from the method that was called.
    * @remark, optional, init to and maybe NULL.
    */
    BrsAmf0Any* response;
public:
    BrsCallResPacket(double _transaction_id);
    virtual ~BrsCallResPacket();
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};

/**
* 4.1.3. createStream
* The client sends this command to the server to create a logical
* channel for message communication The publishing of audio, video, and
* metadata is carried out over stream channel created using the
* createStream command.
*/
class BrsCreateStreamPacket : public BrsPacket
{
public:
    /**
    * Name of the command. Set to "createStream".
    */
    std::string command_name;
    /**
    * Transaction ID of the command.
    */
    double transaction_id;
    /**
    * If there exists any command info this is set, else this is set to null type.
    * @remark, never be NULL, an AMF0 null instance.
    */
    BrsAmf0Any* command_object; // null
public:
    BrsCreateStreamPacket();
    virtual ~BrsCreateStreamPacket();
// decode functions for concrete packet to override.
public:
    virtual int decode(BRSStream* stream);
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};
/**
* response for BrsCreateStreamPacket.
*/
class BrsCreateStreamResPacket : public BrsPacket
{
public:
    /**
    * _result or _error; indicates whether the response is result or error.
    */
    std::string command_name;
    /**
    * ID of the command that response belongs to.
    */
    double transaction_id;
    /**
    * If there exists any command info this is set, else this is set to null type.
    * @remark, never be NULL, an AMF0 null instance.
    */
    BrsAmf0Any* command_object; // null
    /**
    * The return value is either a stream ID or an error information object.
    */
    double stream_id;
public:
    BrsCreateStreamResPacket(double _transaction_id, double _stream_id);
    virtual ~BrsCreateStreamResPacket();
// decode functions for concrete packet to override.
public:
    virtual int decode(BRSStream* stream);
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};

/**
* client close stream packet.
*/
class BrsCloseStreamPacket : public BrsPacket
{
public:
    /**
    * Name of the command, set to "closeStream".
    */
    std::string command_name;
    /**
    * Transaction ID set to 0.
    */
    double transaction_id;
    /**
    * Command information object does not exist. Set to null type.
    * @remark, never be NULL, an AMF0 null instance.
    */
    BrsAmf0Any* command_object; // null
public:
    BrsCloseStreamPacket();
    virtual ~BrsCloseStreamPacket();
// decode functions for concrete packet to override.
public:
    virtual int decode(BRSStream* stream);
};

/**
* FMLE start publish: ReleaseStream/PublishStream
*/
class BrsFMLEStartPacket : public BrsPacket
{
public:
    /**
    * Name of the command
    */
    std::string command_name;
    /**
    * the transaction ID to get the response.
    */
    double transaction_id;
    /**
    * If there exists any command info this is set, else this is set to null type.
    * @remark, never be NULL, an AMF0 null instance.
    */
    BrsAmf0Any* command_object; // null
    /**
    * the stream name to start publish or release.
    */
    std::string stream_name;
public:
    BrsFMLEStartPacket();
    virtual ~BrsFMLEStartPacket();
// decode functions for concrete packet to override.
public:
    virtual int decode(BRSStream* stream);
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
// factory method to create specified FMLE packet.
public:
    static BrsFMLEStartPacket* create_release_stream(std::string stream);
    static BrsFMLEStartPacket* create_FC_publish(std::string stream);
};
/**
* response for BrsFMLEStartPacket.
*/
class BrsFMLEStartResPacket : public BrsPacket
{
public:
    /**
    * Name of the command
    */
    std::string command_name;
    /**
    * the transaction ID to get the response.
    */
    double transaction_id;
    /**
    * If there exists any command info this is set, else this is set to null type.
    * @remark, never be NULL, an AMF0 null instance.
    */
    BrsAmf0Any* command_object; // null
    /**
    * the optional args, set to undefined.
    * @remark, never be NULL, an AMF0 undefined instance.
    */
    BrsAmf0Any* args; // undefined
public:
    BrsFMLEStartResPacket(double _transaction_id);
    virtual ~BrsFMLEStartResPacket();
// decode functions for concrete packet to override.
public:
    virtual int decode(BRSStream* stream);
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};

/**
* FMLE/flash publish
* 4.2.6. Publish
* The client sends the publish command to publish a named stream to the
* server. Using this name, any client can play this stream and receive
* the published audio, video, and data messages.
*/
class BrsPublishPacket : public BrsPacket
{
public:
    /**
    * Name of the command, set to "publish".
    */
    std::string command_name;
    /**
    * Transaction ID set to 0.
    */
    double transaction_id;
    /**
    * Command information object does not exist. Set to null type.
    * @remark, never be NULL, an AMF0 null instance.
    */
    BrsAmf0Any* command_object; // null
    /**
    * Name with which the stream is published.
    */
    std::string stream_name;
    /**
    * Type of publishing. Set to "live", "record", or "append".
    *   record: The stream is published and the data is recorded to a new file.The file
    *           is stored on the server in a subdirectory within the directory that
    *           contains the server application. If the file already exists, it is 
    *           overwritten.
    *   append: The stream is published and the data is appended to a file. If no file
    *           is found, it is created.
    *   live: Live data is published without recording it in a file.
    * @remark, SRS only support live.
    * @remark, optional, default to live.
    */
    std::string type;
public:
    BrsPublishPacket();
    virtual ~BrsPublishPacket();
// decode functions for concrete packet to override.
public:
    virtual int decode(BRSStream* stream);
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};

/**
* 4.2.8. pause
* The client sends the pause command to tell the server to pause or
* start playing.
*/
class BrsPausePacket : public BrsPacket
{
public:
    /**
    * Name of the command, set to "pause".
    */
    std::string command_name;
    /**
    * There is no transaction ID for this command. Set to 0.
    */
    double transaction_id;
    /**
    * Command information object does not exist. Set to null type.
    * @remark, never be NULL, an AMF0 null instance.
    */
    BrsAmf0Any* command_object; // null
    /**
    * true or false, to indicate pausing or resuming play
    */
    bool is_pause;
    /**
    * Number of milliseconds at which the the stream is paused or play resumed.
    * This is the current stream time at the Client when stream was paused. When the
    * playback is resumed, the server will only send messages with timestamps
    * greater than this value.
    */
    double time_ms;
public:
    BrsPausePacket();
    virtual ~BrsPausePacket();
// decode functions for concrete packet to override.
public:
    virtual int decode(BRSStream* stream);
};

/**
* 4.2.1. play
* The client sends this command to the server to play a stream.
*/
class BrsPlayPacket : public BrsPacket
{
public:
    /**
    * Name of the command. Set to "play".
    */
    std::string command_name;
    /**
    * Transaction ID set to 0.
    */
    double transaction_id;
    /**
    * Command information does not exist. Set to null type.
    * @remark, never be NULL, an AMF0 null instance.
    */
    BrsAmf0Any* command_object; // null
    /**
    * Name of the stream to play.
    * To play video (FLV) files, specify the name of the stream without a file
    *       extension (for example, "sample").
    * To play back MP3 or ID3 tags, you must precede the stream name with mp3:
    *       (for example, "mp3:sample".)
    * To play H.264/AAC files, you must precede the stream name with mp4: and specify the
    *       file extension. For example, to play the file sample.m4v, specify 
    *       "mp4:sample.m4v"
    */
    std::string stream_name;
    /**
    * An optional parameter that specifies the start time in seconds.
    * The default value is -2, which means the subscriber first tries to play the live 
    *       stream specified in the Stream Name field. If a live stream of that name is 
    *       not found, it plays the recorded stream specified in the Stream Name field.
    * If you pass -1 in the Start field, only the live stream specified in the Stream 
    *       Name field is played.
    * If you pass 0 or a positive number in the Start field, a recorded stream specified 
    *       in the Stream Name field is played beginning from the time specified in the 
    *       Start field.
    * If no recorded stream is found, the next item in the playlist is played.
    */
    double start;
    /**
    * An optional parameter that specifies the duration of playback in seconds.
    * The default value is -1. The -1 value means a live stream is played until it is no
    *       longer available or a recorded stream is played until it ends.
    * If u pass 0, it plays the single frame since the time specified in the Start field 
    *       from the beginning of a recorded stream. It is assumed that the value specified 
    *       in the Start field is equal to or greater than 0.
    * If you pass a positive number, it plays a live stream for the time period specified 
    *       in the Duration field. After that it becomes available or plays a recorded 
    *       stream for the time specified in the Duration field. (If a stream ends before the
    *       time specified in the Duration field, playback ends when the stream ends.)
    * If you pass a negative number other than -1 in the Duration field, it interprets the 
    *       value as if it were -1.
    */
    double duration;
    /**
    * An optional Boolean value or number that specifies whether to flush any
    * previous playlist.
    */
    bool reset;
public:
    BrsPlayPacket();
    virtual ~BrsPlayPacket();
// decode functions for concrete packet to override.
public:
    virtual int decode(BRSStream* stream);
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};

/**
* response for BrsPlayPacket.
* @remark, user must set the stream_id in header.
*/
class BrsPlayResPacket : public BrsPacket
{
public:
    /**
    * Name of the command. If the play command is successful, the command
    * name is set to onStatus.
    */
    std::string command_name;
    /**
    * Transaction ID set to 0.
    */
    double transaction_id;
    /**
    * Command information does not exist. Set to null type.
    * @remark, never be NULL, an AMF0 null instance.
    */
    BrsAmf0Any* command_object; // null
    /**
    * If the play command is successful, the client receives OnStatus message from
    * server which is NetStream.Play.Start. If the specified stream is not found,
    * NetStream.Play.StreamNotFound is received.
    * @remark, never be NULL, an AMF0 object instance.
    */
    BrsAmf0Object* desc;
public:
    BrsPlayResPacket();
    virtual ~BrsPlayResPacket();
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};

/**
* when bandwidth test done, notice client.
*/
class BrsOnBWDonePacket : public BrsPacket
{
public:
    /**
    * Name of command. Set to "onBWDone"
    */
    std::string command_name;
    /**
    * Transaction ID set to 0.
    */
    double transaction_id;
    /**
    * Command information does not exist. Set to null type.
    * @remark, never be NULL, an AMF0 null instance.
    */
    BrsAmf0Any* args; // null
public:
    BrsOnBWDonePacket();
    virtual ~BrsOnBWDonePacket();
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};

/**
* onStatus command, AMF0 Call
* @remark, user must set the stream_id by BrsCommonMessage.set_packet().
*/
class BrsOnStatusCallPacket : public BrsPacket
{
public:
    /**
    * Name of command. Set to "onStatus"
    */
    std::string command_name;
    /**
    * Transaction ID set to 0.
    */
    double transaction_id;
    /**
    * Command information does not exist. Set to null type.
    * @remark, never be NULL, an AMF0 null instance.
    */
    BrsAmf0Any* args; // null
    /**
    * Name-value pairs that describe the response from the server. 
    * 'code','level', 'description' are names of few among such information.
    * @remark, never be NULL, an AMF0 object instance.
    */
    BrsAmf0Object* data;
public:
    BrsOnStatusCallPacket();
    virtual ~BrsOnStatusCallPacket();
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};

/**
* the special packet for the bandwidth test.
* actually, it's a BrsOnStatusCallPacket, but
* 1. encode with data field, to send data to client.
* 2. decode ignore the data field, donot care.
*/
class BrsBandwidthPacket : public BrsPacket
{
private:
    disable_default_copy(BrsBandwidthPacket);
public:
    /**
    * Name of command. 
    */
    std::string command_name;
    /**
    * Transaction ID set to 0.
    */
    double transaction_id;
    /**
    * Command information does not exist. Set to null type.
    * @remark, never be NULL, an AMF0 null instance.
    */
    BrsAmf0Any* args; // null
    /**
    * Name-value pairs that describe the response from the server.
    * 'code','level', 'description' are names of few among such information.
    * @remark, never be NULL, an AMF0 object instance.
    */
    BrsAmf0Object* data;
public:
    BrsBandwidthPacket();
    virtual ~BrsBandwidthPacket();
// decode functions for concrete packet to override.
public:
    virtual int decode(BRSStream* stream);
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
// help function for bandwidth packet.
public:
    virtual bool is_start_play();
    virtual bool is_starting_play();
    virtual bool is_stop_play();
    virtual bool is_stopped_play();
    virtual bool is_start_publish();
    virtual bool is_starting_publish();
    virtual bool is_stop_publish();
    virtual bool is_stopped_publish();
    virtual bool is_finish();
    virtual bool is_final();
    static BrsBandwidthPacket* create_start_play();
    static BrsBandwidthPacket* create_starting_play();
    static BrsBandwidthPacket* create_playing();
    static BrsBandwidthPacket* create_stop_play();
    static BrsBandwidthPacket* create_stopped_play();
    static BrsBandwidthPacket* create_start_publish();
    static BrsBandwidthPacket* create_starting_publish();
    static BrsBandwidthPacket* create_publishing();
    static BrsBandwidthPacket* create_stop_publish();
    static BrsBandwidthPacket* create_stopped_publish();
    static BrsBandwidthPacket* create_finish();
    static BrsBandwidthPacket* create_final();
private:
    virtual BrsBandwidthPacket* set_command(std::string command);
};

/**
* onStatus data, AMF0 Data
* @remark, user must set the stream_id by BrsCommonMessage.set_packet().
*/
class BrsOnStatusDataPacket : public BrsPacket
{
public:
    /**
    * Name of command. Set to "onStatus"
    */
    std::string command_name;
    /**
    * Name-value pairs that describe the response from the server.
    * 'code', are names of few among such information.
    * @remark, never be NULL, an AMF0 object instance.
    */
    BrsAmf0Object* data;
public:
    BrsOnStatusDataPacket();
    virtual ~BrsOnStatusDataPacket();
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};

/**
* AMF0Data RtmpSampleAccess
* @remark, user must set the stream_id by BrsCommonMessage.set_packet().
*/
class BrsSampleAccessPacket : public BrsPacket
{
public:
    /**
    * Name of command. Set to "|RtmpSampleAccess".
    */
    std::string command_name;
    /**
    * whether allow access the sample of video.
    * @see: https://github.com/ossrs/srs/issues/49
    * @see: http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/net/NetStream.html#videoSampleAccess
    */
    bool video_sample_access;
    /**
    * whether allow access the sample of audio.
    * @see: https://github.com/ossrs/srs/issues/49
    * @see: http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/net/NetStream.html#audioSampleAccess
    */
    bool audio_sample_access;
public:
    BrsSampleAccessPacket();
    virtual ~BrsSampleAccessPacket();
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};

/**
* the stream metadata.
* FMLE: @setDataFrame
* others: onMetaData
*/
class BrsOnMetaDataPacket : public BrsPacket
{
public:
    /**
    * Name of metadata. Set to "onMetaData"
    */
    std::string name;
    /**
    * Metadata of stream.
    * @remark, never be NULL, an AMF0 object instance.
    */
    BrsAmf0Object* metadata;
public:
    BrsOnMetaDataPacket();
    virtual ~BrsOnMetaDataPacket();
// decode functions for concrete packet to override.
public:
    virtual int decode(BRSStream* stream);
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};

/**
* 5.5. Window Acknowledgement Size (5)
* The client or the server sends this message to inform the peer which
* window size to use when sending acknowledgment.
*/
class BrsSetWindowAckSizePacket : public BrsPacket
{
public:
    int32_t ackowledgement_window_size;
public:
    BrsSetWindowAckSizePacket();
    virtual ~BrsSetWindowAckSizePacket();
// decode functions for concrete packet to override.
public:
    virtual int decode(BRSStream* stream);
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};

/**
* 5.3. Acknowledgement (3)
* The client or the server sends the acknowledgment to the peer after
* receiving bytes equal to the window size.
*/
class BrsAcknowledgementPacket : public BrsPacket
{
public:
    int32_t sequence_number;
public:
    BrsAcknowledgementPacket();
    virtual ~BrsAcknowledgementPacket();
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};

/**
* 7.1. Set Chunk Size
* Protocol control message 1, Set Chunk Size, is used to notify the
* peer about the new maximum chunk size.
*/
class BrsSetChunkSizePacket : public BrsPacket
{
public:
    /**
    * The maximum chunk size can be 65536 bytes. The chunk size is
    * maintained independently for each direction.
    */
    int32_t chunk_size;
public:
    BrsSetChunkSizePacket();
    virtual ~BrsSetChunkSizePacket();
// decode functions for concrete packet to override.
public:
    virtual int decode(BRSStream* stream);
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};

// 5.6. Set Peer Bandwidth (6)
enum BrsPeerBandwidthType
{
    // The sender can mark this message hard (0), soft (1), or dynamic (2)
    // using the Limit type field.
    BrsPeerBandwidthHard = 0,
    BrsPeerBandwidthSoft = 1,
    BrsPeerBandwidthDynamic = 2,
};

/**
* 5.6. Set Peer Bandwidth (6)
* The client or the server sends this message to update the output
* bandwidth of the peer.
*/
class BrsSetPeerBandwidthPacket : public BrsPacket
{
public:
    int32_t bandwidth;
    // @see: BrsPeerBandwidthType
    int8_t type;
public:
    BrsSetPeerBandwidthPacket();
    virtual ~BrsSetPeerBandwidthPacket();
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};

// 3.7. User Control message
enum SrcPCUCEventType
{
    // generally, 4bytes event-data
    
    /**
    * The server sends this event to notify the client
    * that a stream has become functional and can be
    * used for communication. By default, this event
    * is sent on ID 0 after the application connect
    * command is successfully received from the
    * client. The event data is 4-byte and represents
    * the stream ID of the stream that became
    * functional.
    */
    SrcPCUCStreamBegin              = 0x00,

    /**
    * The server sends this event to notify the client
    * that the playback of data is over as requested
    * on this stream. No more data is sent without
    * issuing additional commands. The client discards
    * the messages received for the stream. The
    * 4 bytes of event data represent the ID of the
    * stream on which playback has ended.
    */
    SrcPCUCStreamEOF                = 0x01,

    /**
    * The server sends this event to notify the client
    * that there is no more data on the stream. If the
    * server does not detect any message for a time
    * period, it can notify the subscribed clients 
    * that the stream is dry. The 4 bytes of event 
    * data represent the stream ID of the dry stream. 
    */
    SrcPCUCStreamDry                = 0x02,

    /**
    * The client sends this event to inform the server
    * of the buffer size (in milliseconds) that is 
    * used to buffer any data coming over a stream.
    * This event is sent before the server starts  
    * processing the stream. The first 4 bytes of the
    * event data represent the stream ID and the next
    * 4 bytes represent the buffer length, in 
    * milliseconds.
    */
    SrcPCUCSetBufferLength          = 0x03, // 8bytes event-data

    /**
    * The server sends this event to notify the client
    * that the stream is a recorded stream. The
    * 4 bytes event data represent the stream ID of
    * the recorded stream.
    */
    SrcPCUCStreamIsRecorded         = 0x04,

    /**
    * The server sends this event to test whether the
    * client is reachable. Event data is a 4-byte
    * timestamp, representing the local server time
    * when the server dispatched the command. The
    * client responds with kMsgPingResponse on
    * receiving kMsgPingRequest.  
    */
    SrcPCUCPingRequest              = 0x06,

    /**
    * The client sends this event to the server in
    * response to the ping request. The event data is
    * a 4-byte timestamp, which was received with the
    * kMsgPingRequest request.
    */
    SrcPCUCPingResponse             = 0x07,
    
    /**
     * for PCUC size=3, the payload is "00 1A 01",
     * where we think the event is 0x001a, fms defined msg,
     * which has only 1bytes event data.
     */
    BrsPCUCFmsEvent0                = 0x1a,
};

/**
* 5.4. User Control Message (4)
* 
* for the EventData is 4bytes.
* Stream Begin(=0)              4-bytes stream ID
* Stream EOF(=1)                4-bytes stream ID
* StreamDry(=2)                 4-bytes stream ID
* SetBufferLength(=3)           8-bytes 4bytes stream ID, 4bytes buffer length.
* StreamIsRecorded(=4)          4-bytes stream ID
* PingRequest(=6)               4-bytes timestamp local server time
* PingResponse(=7)              4-bytes timestamp received ping request.
* 
* 3.7. User Control message
* +------------------------------+-------------------------
* | Event Type ( 2- bytes ) | Event Data
* +------------------------------+-------------------------
* Figure 5 Pay load for the 'User Control Message'.
*/
class BrsUserControlPacket : public BrsPacket
{
public:
    /**
    * Event type is followed by Event data.
    * @see: SrcPCUCEventType
    */
    int16_t event_type;
    /**
     * the event data generally in 4bytes.
     * @remark for event type is 0x001a, only 1bytes.
     * @see BrsPCUCFmsEvent0
     */
    int32_t event_data;
    /**
    * 4bytes if event_type is SetBufferLength; otherwise 0.
    */
    int32_t extra_data;
public:
    BrsUserControlPacket();
    virtual ~BrsUserControlPacket();
// decode functions for concrete packet to override.
public:
    virtual int decode(BRSStream* stream);
// encode functions for concrete packet to override.
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(BRSStream* stream);
};


  
}