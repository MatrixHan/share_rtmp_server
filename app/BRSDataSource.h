#pragma once 

#include <BRSCommon.h>
#include <BRSKernelCodec.h>
#include <BRSFlv.h>
#include <BRSRtmpPackets.h>

namespace BRS 
{
struct BrsStatisticVhost
{
public:
    int64_t id;
    std::string vhost;
    int nb_streams;
    int nb_clients;
public:
    /**
    * vhost total kbps.
    */
    //BrsKbps* kbps;
public:
    BrsStatisticVhost();
    virtual ~BrsStatisticVhost();
public:
    virtual int dumps(std::stringstream& ss);
};
  
struct BrsStatisticStream
{
public:
    int64_t id;
    BrsStatisticVhost* vhost;
    std::string app;
    std::string stream;
    std::string url;
    bool active;
    int connection_cid;
    int nb_clients;
public:
    /**
    * stream total kbps.
    */
   // BrsKbps* kbps;
public:
    bool has_video;
    BrsCodecVideo vcodec;
    // profile_idc, H.264-AVC-ISO_IEC_14496-10.pdf, page 45.
    BrsAvcProfile avc_profile;
    // level_idc, H.264-AVC-ISO_IEC_14496-10.pdf, page 45.
    BrsAvcLevel avc_level;
public:
    bool has_audio;
    BrsCodecAudio acodec;
    BrsCodecAudioSampleRate asample_rate;
    BrsCodecAudioSoundType asound_type;
    /**
    * audio specified
    * audioObjectType, in 1.6.2.1 AudioSpecificConfig, page 33,
    * 1.5.1.1 Audio object type definition, page 23,
    *           in aac-mp4a-format-ISO_IEC_14496-3+2001.pdf.
    */
    BrsAacObjectType aac_object;
public:
    BrsStatisticStream();
    virtual ~BrsStatisticStream();
public:
    virtual int dumps(std::stringstream& ss);
public:
    /**
    * publish the stream.
    */
    virtual void publish(int cid);
    /**
    * close the stream.
    */
    virtual void close();
};

struct BrsStatisticClient
{
public:
    BrsStatisticStream* stream;
    //BrsConnection* conn;
    BrsRequest* req;
    BrsRtmpConnType type;
    int id;
    int64_t create;
public:
    BrsStatisticClient();
    virtual ~BrsStatisticClient();
public:
    virtual int dumps(std::stringstream& ss);
};

class BrsStatistic
{
private:
    static BrsStatistic *_instance;
    // the id to identify the sever.
    int64_t _server_id;
private:
    // key: vhost id, value: vhost object.
    std::map<int64_t, BrsStatisticVhost*> vhosts;
    // key: vhost url, value: vhost Object.
    // @remark a fast index for vhosts.
    std::map<std::string, BrsStatisticVhost*> rvhosts;
private:
    // key: stream id, value: stream Object.
    std::map<int64_t, BrsStatisticStream*> streams;
    // key: stream url, value: stream Object.
    // @remark a fast index for streams.
    std::map<std::string, BrsStatisticStream*> rstreams;
private:
    // key: client id, value: stream object.
    std::map<int, BrsStatisticClient*> clients;
    // server total kbps.
    //BrsKbps* kbps;
private:
    BrsStatistic();
    virtual ~BrsStatistic();
public:
    static BrsStatistic* instance();
public:
    virtual BrsStatisticVhost* find_vhost(int vid);
    virtual BrsStatisticStream* find_stream(int sid);
    virtual BrsStatisticClient* find_client(int cid);
public:
    /**
    * when got video info for stream.
    */
    virtual int on_video_info(BrsRequest* req, 
        BrsCodecVideo vcodec, BrsAvcProfile avc_profile, BrsAvcLevel avc_level
    );
    /**
    * when got audio info for stream.
    */
    virtual int on_audio_info(BrsRequest* req,
        BrsCodecAudio acodec, BrsCodecAudioSampleRate asample_rate, BrsCodecAudioSoundType asound_type,
        BrsAacObjectType aac_object
    );
    /**
     * when publish stream.
     * @param req the request object of publish connection.
     * @param cid the cid of publish connection.
     */
    virtual void on_stream_publish(BrsRequest* req, int cid);
    /**
    * when close stream.
    */
    virtual void on_stream_close(BrsRequest* req);
public:
    /**
     * when got a client to publish/play stream,
     * @param id, the client srs id.
     * @param req, the client request object.
     * @param conn, the physical absract connection object.
     * @param type, the type of connection.
     */
   // virtual int on_client(int id, BrsRequest* req, BrsConnection* conn, BrsRtmpConnType type);
    /**
     * client disconnect
     * @remark the on_disconnect always call, while the on_client is call when
     *      only got the request object, so the client specified by id maybe not
     *      exists in stat.
     */
    virtual void on_disconnect(int id);
    /**
    * sample the kbps, add delta bytes of conn.
    * use kbps_sample() to get all result of kbps stat.
    */
    // TODO: FIXME: the add delta must use IKbpsDelta interface instead.
    //virtual void kbps_add_delta(BrsConnection* conn);
    /**
    * calc the result for all kbps.
    * @return the server kbps.
    */
    //virtual BrsKbps* kbps_sample();
public:
    /**
    * get the server id, used to identify the server.
    * for example, when restart, the server id must changed.
    */
    virtual int64_t server_id();
    /**
    * dumps the vhosts to sstream in json.
    */
    virtual int dumps_vhosts(std::stringstream& ss);
    /**
    * dumps the streams to sstream in json.
    */
    virtual int dumps_streams(std::stringstream& ss);
    /**
     * dumps the clients to sstream in json.
     * @param start the start index, from 0.
     * @param count the max count of clients to dump.
     */
    virtual int dumps_clients(std::stringstream& ss, int start, int count);
private:
    virtual BrsStatisticVhost* create_vhost(BrsRequest* req);
    virtual BrsStatisticStream* create_stream(BrsStatisticVhost* vhost, BrsRequest* req);
};

}