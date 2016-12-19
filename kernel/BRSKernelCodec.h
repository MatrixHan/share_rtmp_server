#pragma once
#include <BRSCommon.h>
#include <string>

namespace BRS{

class BRSStream;

// AACPacketType IF SoundFormat == 10 UI8
// The following values are defined:
//     0 = AAC sequence header
//     1 = AAC raw
enum BrsCodecAudioType
{
    // set to the max value to reserved, for array map.
    BrsCodecAudioTypeReserved                        = 2,
    
    BrsCodecAudioTypeSequenceHeader                 = 0,
    BrsCodecAudioTypeRawData                         = 1,
};

// E.4.3.1 VIDEODATA
// Frame Type UB [4]
// Type of video frame. The following values are defined:
//     1 = key frame (for AVC, a seekable frame)
//     2 = inter frame (for AVC, a non-seekable frame)
//     3 = disposable inter frame (H.263 only)
//     4 = generated key frame (reserved for server use only)
//     5 = video info/command frame
enum BrsCodecVideoAVCFrame
{
    // set to the zero to reserved, for array map.
    BrsCodecVideoAVCFrameReserved                    = 0,
    BrsCodecVideoAVCFrameReserved1                    = 6,
    
    BrsCodecVideoAVCFrameKeyFrame                     = 1,
    BrsCodecVideoAVCFrameInterFrame                 = 2,
    BrsCodecVideoAVCFrameDisposableInterFrame         = 3,
    BrsCodecVideoAVCFrameGeneratedKeyFrame            = 4,
    BrsCodecVideoAVCFrameVideoInfoFrame                = 5,
};

// AVCPacketType IF CodecID == 7 UI8
// The following values are defined:
//     0 = AVC sequence header
//     1 = AVC NALU
//     2 = AVC end of sequence (lower level NALU sequence ender is
//         not required or supported)
enum BrsCodecVideoAVCType
{
    // set to the max value to reserved, for array map.
    BrsCodecVideoAVCTypeReserved                    = 3,
    
    BrsCodecVideoAVCTypeSequenceHeader                 = 0,
    BrsCodecVideoAVCTypeNALU                         = 1,
    BrsCodecVideoAVCTypeSequenceHeaderEOF             = 2,
};

// E.4.3.1 VIDEODATA
// CodecID UB [4]
// Codec Identifier. The following values are defined:
//     2 = Sorenson H.263
//     3 = Screen video
//     4 = On2 VP6
//     5 = On2 VP6 with alpha channel
//     6 = Screen video version 2
//     7 = AVC
enum BrsCodecVideo
{
    // set to the zero to reserved, for array map.
    BrsCodecVideoReserved                = 0,
    BrsCodecVideoReserved1                = 1,
    BrsCodecVideoReserved2                = 9,
    
    // for user to disable video, for example, use pure audio hls.
    BrsCodecVideoDisabled                = 8,
    
    BrsCodecVideoSorensonH263             = 2,
    BrsCodecVideoScreenVideo             = 3,
    BrsCodecVideoOn2VP6                 = 4,
    BrsCodecVideoOn2VP6WithAlphaChannel = 5,
    BrsCodecVideoScreenVideoVersion2     = 6,
    BrsCodecVideoAVC                     = 7,
};
std::string brs_codec_video2str(BrsCodecVideo codec);

// SoundFormat UB [4] 
// Format of SoundData. The following values are defined:
//     0 = Linear PCM, platform endian
//     1 = ADPCM
//     2 = MP3
//     3 = Linear PCM, little endian
//     4 = Nellymoser 16 kHz mono
//     5 = Nellymoser 8 kHz mono
//     6 = Nellymoser
//     7 = G.711 A-law logarithmic PCM
//     8 = G.711 mu-law logarithmic PCM
//     9 = reserved
//     10 = AAC
//     11 = Speex
//     14 = MP3 8 kHz
//     15 = Device-specific sound
// Formats 7, 8, 14, and 15 are reserved.
// AAC is supported in Flash Player 9,0,115,0 and higher.
// Speex is supported in Flash Player 10 and higher.
enum BrsCodecAudio
{
    // set to the max value to reserved, for array map.
    BrsCodecAudioReserved1                = 16,
    
    // for user to disable audio, for example, use pure video hls.
    BrsCodecAudioDisabled                   = 17,
    
    BrsCodecAudioLinearPCMPlatformEndian             = 0,
    BrsCodecAudioADPCM                                 = 1,
    BrsCodecAudioMP3                                 = 2,
    BrsCodecAudioLinearPCMLittleEndian                 = 3,
    BrsCodecAudioNellymoser16kHzMono                 = 4,
    BrsCodecAudioNellymoser8kHzMono                 = 5,
    BrsCodecAudioNellymoser                         = 6,
    BrsCodecAudioReservedG711AlawLogarithmicPCM        = 7,
    BrsCodecAudioReservedG711MuLawLogarithmicPCM    = 8,
    BrsCodecAudioReserved                             = 9,
    BrsCodecAudioAAC                                 = 10,
    BrsCodecAudioSpeex                                 = 11,
    BrsCodecAudioReservedMP3_8kHz                     = 14,
    BrsCodecAudioReservedDeviceSpecificSound         = 15,
};
std::string brs_codec_audio2str(BrsCodecAudio codec);

/**
* the FLV/RTMP supported audio sample rate.
* Sampling rate. The following values are defined:
* 0 = 5.5 kHz = 5512 Hz
* 1 = 11 kHz = 11025 Hz
* 2 = 22 kHz = 22050 Hz
* 3 = 44 kHz = 44100 Hz
*/
enum BrsCodecAudioSampleRate
{
    // set to the max value to reserved, for array map.
    BrsCodecAudioSampleRateReserved                 = 4,
    
    BrsCodecAudioSampleRate5512                     = 0,
    BrsCodecAudioSampleRate11025                    = 1,
    BrsCodecAudioSampleRate22050                    = 2,
    BrsCodecAudioSampleRate44100                    = 3,
};

/**
* E.4.1 FLV Tag, page 75
*/
enum BrsCodecFlvTag
{
    // set to the zero to reserved, for array map.
    BrsCodecFlvTagReserved = 0,

    // 8 = audio
    BrsCodecFlvTagAudio = 8,
    // 9 = video
    BrsCodecFlvTagVideo = 9,
    // 18 = script data
    BrsCodecFlvTagScript = 18,
};

/**
* Annex E. The FLV File Format
* @see BrsAvcAacCodec for the media stream codec.
*/
class BrsFlvCodec
{
public:
    BrsFlvCodec();
    virtual ~BrsFlvCodec();
// the following function used to finger out the flv/rtmp packet detail.
public:
    /**
    * only check the frame_type, not check the codec type.
    */
    static bool video_is_keyframe(char* data, int size);
    /**
    * check codec h264, keyframe, sequence header
    */
    static bool video_is_sequence_header(char* data, int size);
    /**
    * check codec aac, sequence header
    */
    static bool audio_is_sequence_header(char* data, int size);
    /**
    * check codec h264.
    */
    static bool video_is_h264(char* data, int size);
    /**
    * check codec aac.
    */
    static bool audio_is_aac(char* data, int size);
    /**
     * check the video RTMP/flv header info,
     * @return true if video RTMP/flv header is ok.
     * @remark all type of audio is possible, no need to check audio.
     */
    static bool video_is_acceptable(char* data, int size);
};

/**
* the public data, event HLS disable, others can use it.
*/
/**
* the flv sample rate map
*/
extern int flv_sample_rates[];

/**
* the aac sample rate map
*/
extern int aac_sample_rates[];

#define SRS_SRS_MAX_CODEC_SAMPLE 128
#define SRS_AAC_SAMPLE_RATE_UNSET 15

/**
* the FLV/RTMP supported audio sample size.
* Size of each audio sample. This parameter only pertains to
* uncompressed formats. Compressed formats always decode
* to 16 bits internally.
* 0 = 8-bit samples
* 1 = 16-bit samples
*/
enum BrsCodecAudioSampleSize
{
    // set to the max value to reserved, for array map.
    BrsCodecAudioSampleSizeReserved                 = 2,
    
    BrsCodecAudioSampleSize8bit                     = 0,
    BrsCodecAudioSampleSize16bit                    = 1,
};

/**
* the FLV/RTMP supported audio sound type/channel.
* Mono or stereo sound
* 0 = Mono sound
* 1 = Stereo sound
*/
enum BrsCodecAudioSoundType
{
    // set to the max value to reserved, for array map.
    BrsCodecAudioSoundTypeReserved                  = 2, 
    
    BrsCodecAudioSoundTypeMono                      = 0,
    BrsCodecAudioSoundTypeStereo                    = 1,
};

/**
 * Table 7-1 - NAL unit type codes, syntax element categories, and NAL unit type classes
 * H.264-AVC-ISO_IEC_14496-10-2012.pdf, page 83.
 */
enum BrsAvcNaluType
{
    // Unspecified
    BrsAvcNaluTypeReserved = 0,
    
    // Coded slice of a non-IDR picture slice_layer_without_partitioning_rbsp( )
    BrsAvcNaluTypeNonIDR = 1,
    // Coded slice data partition A slice_data_partition_a_layer_rbsp( )
    BrsAvcNaluTypeDataPartitionA = 2,
    // Coded slice data partition B slice_data_partition_b_layer_rbsp( )
    BrsAvcNaluTypeDataPartitionB = 3,
    // Coded slice data partition C slice_data_partition_c_layer_rbsp( )
    BrsAvcNaluTypeDataPartitionC = 4,
    // Coded slice of an IDR picture slice_layer_without_partitioning_rbsp( )
    BrsAvcNaluTypeIDR = 5,
    // Supplemental enhancement information (SEI) sei_rbsp( )
    BrsAvcNaluTypeSEI = 6,
    // Sequence parameter set seq_parameter_set_rbsp( )
    BrsAvcNaluTypeSPS = 7,
    // Picture parameter set pic_parameter_set_rbsp( )
    BrsAvcNaluTypePPS = 8,
    // Access unit delimiter access_unit_delimiter_rbsp( )
    BrsAvcNaluTypeAccessUnitDelimiter = 9,
    // End of sequence end_of_seq_rbsp( )
    BrsAvcNaluTypeEOSequence = 10,
    // End of stream end_of_stream_rbsp( )
    BrsAvcNaluTypeEOStream = 11,
    // Filler data filler_data_rbsp( )
    BrsAvcNaluTypeFilterData = 12,
    // Sequence parameter set extension seq_parameter_set_extension_rbsp( )
    BrsAvcNaluTypeSPSExt = 13,
    // Prefix NAL unit prefix_nal_unit_rbsp( )
    BrsAvcNaluTypePrefixNALU = 14,
    // Subset sequence parameter set subset_seq_parameter_set_rbsp( )
    BrsAvcNaluTypeSubsetSPS = 15,
    // Coded slice of an auxiliary coded picture without partitioning slice_layer_without_partitioning_rbsp( )
    BrsAvcNaluTypeLayerWithoutPartition = 19,
    // Coded slice extension slice_layer_extension_rbsp( )
    BrsAvcNaluTypeCodedSliceExt = 20,
};

std::string brs_codec_avc_nalu2str(BrsAvcNaluType nalu_type);

/**
* the codec sample unit.
* for h.264 video packet, a NALU is a sample unit.
* for aac raw audio packet, a NALU is the entire aac raw data.
* for sequence header, it's not a sample unit.
*/
class BrsCodecSampleUnit
{
public:
    /**
    * the sample bytes is directly ptr to packet bytes,
    * user should never use it when packet destroyed.
    */
    int size;
    char* bytes;
public:
    BrsCodecSampleUnit();
    virtual ~BrsCodecSampleUnit();
};

/**
* the samples in the flv audio/video packet.
* the sample used to analysis a video/audio packet,
* split the h.264 NALUs to buffers, or aac raw data to a buffer,
* and decode the video/audio specified infos.
* 
* the sample unit:
*       a video packet codec in h.264 contains many NALUs, each is a sample unit.
*       a audio packet codec in aac is a sample unit.
* @remark, the video/audio sequence header is not sample unit,
*       all sequence header stores as extra data, 
*       @see BrsAvcAacCodec.avc_extra_data and BrsAvcAacCodec.aac_extra_data
* @remark, user must clear all samples before decode a new video/audio packet.
*/
class BrsCodecSample
{
public:
    /**
    * each audio/video raw data packet will dumps to one or multiple buffers,
    * the buffers will write to hls and clear to reset.
    * generally, aac audio packet corresponding to one buffer,
    * where avc/h264 video packet may contains multiple buffer.
    */
    int nb_sample_units;
    BrsCodecSampleUnit sample_units[SRS_SRS_MAX_CODEC_SAMPLE];
public:
    /**
    * whether the sample is video sample which demux from video packet.
    */
    bool is_video;
    /**
    * CompositionTime, video_file_format_spec_v10_1.pdf, page 78.
    * cts = pts - dts, where dts = flvheader->timestamp.
    */
    int32_t cts;
public:
    // video specified
    BrsCodecVideoAVCFrame frame_type;
    BrsCodecVideoAVCType avc_packet_type;
    // whether sample_units contains IDR frame.
    bool has_idr;
    BrsAvcNaluType first_nalu_type;
public:
    // audio specified
    BrsCodecAudio acodec;
    // audio aac specified.
    BrsCodecAudioSampleRate sound_rate;
    BrsCodecAudioSampleSize sound_size;
    BrsCodecAudioSoundType sound_type;
    BrsCodecAudioType aac_packet_type;
public:
    BrsCodecSample();
    virtual ~BrsCodecSample();
public:
    /**
    * clear all samples.
    * the sample units never copy the bytes, it directly use the ptr,
    * so when video/audio packet is destroyed, the sample must be clear.
    * in a word, user must clear sample before demux it.
    * @remark demux sample use BrsAvcAacCodec.audio_aac_demux or video_avc_demux.
    */
    void clear();
    /**
    * add the a sample unit, it's a h.264 NALU or aac raw data.
    * the sample unit directly use the ptr of packet bytes,
    * so user must never use sample unit when packet is destroyed.
    * in a word, user must clear sample before demux it.
    */
    int add_sample_unit(char* bytes, int size);
};

/**
* the avc payload format, must be ibmf or annexb format.
* we guess by annexb first, then ibmf for the first time,
* and we always use the guessed format for the next time.
*/
enum BrsAvcPayloadFormat
{
    BrsAvcPayloadFormatGuess = 0,
    BrsAvcPayloadFormatAnnexb,
    BrsAvcPayloadFormatIbmf,
};

/**
* the aac profile, for ADTS(HLS/TS)
* @see https://github.com/osbrs/brs/issues/310
*/
enum BrsAacProfile
{
    BrsAacProfileReserved = 3,
    
    // @see 7.1 Profiles, aac-iso-13818-7.pdf, page 40
    BrsAacProfileMain = 0,
    BrsAacProfileLC = 1,
    BrsAacProfileSSR = 2,
};
std::string brs_codec_aac_profile2str(BrsAacProfile aac_profile);

/**
* the aac object type, for RTMP sequence header
* for AudioSpecificConfig, @see aac-mp4a-format-ISO_IEC_14496-3+2001.pdf, page 33
* for audioObjectType, @see aac-mp4a-format-ISO_IEC_14496-3+2001.pdf, page 23
*/
enum BrsAacObjectType
{
    BrsAacObjectTypeReserved = 0,
    
    // Table 1.1 - Audio Object Type definition
    // @see @see aac-mp4a-format-ISO_IEC_14496-3+2001.pdf, page 23
    BrsAacObjectTypeAacMain = 1,
    BrsAacObjectTypeAacLC = 2,
    BrsAacObjectTypeAacSSR = 3,
    
    // AAC HE = LC+SBR
    BrsAacObjectTypeAacHE = 5,
    // AAC HEv2 = LC+SBR+PS
    BrsAacObjectTypeAacHEV2 = 29,
};
std::string brs_codec_aac_object2str(BrsAacObjectType aac_object);
// ts/hls/adts audio header profile to RTMP sequence header object type.
BrsAacObjectType brs_codec_aac_ts2rtmp(BrsAacProfile profile);
// RTMP sequence header object type to ts/hls/adts audio header profile.
BrsAacProfile brs_codec_aac_rtmp2ts(BrsAacObjectType object_type);

/**
* the profile for avc/h.264.
* @see Annex A Profiles and levels, H.264-AVC-ISO_IEC_14496-10.pdf, page 205.
*/
enum BrsAvcProfile
{
    BrsAvcProfileReserved = 0,
    
    // @see ffmpeg, libavcodec/avcodec.h:2713
    BrsAvcProfileBaseline = 66,
    // FF_PROFILE_H264_CONSTRAINED  (1<<9)  // 8+1; constraint_set1_flag
    // FF_PROFILE_H264_CONSTRAINED_BASELINE (66|FF_PROFILE_H264_CONSTRAINED)
    BrsAvcProfileConstrainedBaseline = 578,
    BrsAvcProfileMain = 77,
    BrsAvcProfileExtended = 88,
    BrsAvcProfileHigh = 100,
    BrsAvcProfileHigh10 = 110,
    BrsAvcProfileHigh10Intra = 2158,
    BrsAvcProfileHigh422 = 122,
    BrsAvcProfileHigh422Intra = 2170,
    BrsAvcProfileHigh444 = 144,
    BrsAvcProfileHigh444Predictive = 244,
    BrsAvcProfileHigh444Intra = 2192,
};
std::string brs_codec_avc_profile2str(BrsAvcProfile profile);

/**
* the level for avc/h.264.
* @see Annex A Profiles and levels, H.264-AVC-ISO_IEC_14496-10.pdf, page 207.
*/
enum BrsAvcLevel
{
    BrsAvcLevelReserved = 0,
    
    BrsAvcLevel_1 = 10,
    BrsAvcLevel_11 = 11,
    BrsAvcLevel_12 = 12,
    BrsAvcLevel_13 = 13,
    BrsAvcLevel_2 = 20,
    BrsAvcLevel_21 = 21,
    BrsAvcLevel_22 = 22,
    BrsAvcLevel_3 = 30,
    BrsAvcLevel_31 = 31,
    BrsAvcLevel_32 = 32,
    BrsAvcLevel_4 = 40,
    BrsAvcLevel_41 = 41,
    BrsAvcLevel_5 = 50,
    BrsAvcLevel_51 = 51,
};
std::string brs_codec_avc_level2str(BrsAvcLevel level);

#if !defined(SRS_EXPORT_LIBRTMP)

/**
* the h264/avc and aac codec, for media stream.
*
* to demux the FLV/RTMP video/audio packet to sample,
* add each NALUs of h.264 as a sample unit to sample,
* while the entire aac raw data as a sample unit.
*
* for sequence header,
* demux it and save it in the avc_extra_data and aac_extra_data,
* 
* for the codec info, such as audio sample rate,
* decode from FLV/RTMP header, then use codec info in sequence 
* header to override it.
*/
class BrsAvcAacCodec
{
private:
    BRSStream* stream;
public:
    /**
    * metadata specified
    */
    int             duration;
    int             width;
    int             height;
    int             frame_rate;
    // @see: BrsCodecVideo
    int             video_codec_id;
    int             video_data_rate; // in bps
    // @see: BrsCod ecAudioType
    int             audio_codec_id;
    int             audio_data_rate; // in bps
public:
    /**
    * video specified
    */
    // profile_idc, H.264-AVC-ISO_IEC_14496-10.pdf, page 45.
    BrsAvcProfile   avc_profile; 
    // level_idc, H.264-AVC-ISO_IEC_14496-10.pdf, page 45.
    BrsAvcLevel     avc_level; 
    // lengthSizeMinusOne, H.264-AVC-ISO_IEC_14496-15.pdf, page 16
    int8_t          NAL_unit_length;
    u_int16_t       sequenceParameterSetLength;
    char*           sequenceParameterSetNALUnit;
    u_int16_t       pictureParameterSetLength;
    char*           pictureParameterSetNALUnit;
private:
    // the avc payload format.
    BrsAvcPayloadFormat payload_format;
public:
    /**
    * audio specified
    * audioObjectType, in 1.6.2.1 AudioSpecificConfig, page 33,
    * 1.5.1.1 Audio object type definition, page 23,
    *           in aac-mp4a-format-ISO_IEC_14496-3+2001.pdf.
    */
    BrsAacObjectType    aac_object;
    /**
    * samplingFrequencyIndex
    */
    u_int8_t        aac_sample_rate;
    /**
    * channelConfiguration
    */
    u_int8_t        aac_channels;
public:
    /**
    * the avc extra data, the AVC sequence header,
    * without the flv codec header,
    * @see: ffmpeg, AVCodecContext::extradata
    */
    int             avc_extra_size;
    char*           avc_extra_data;
    /**
    * the aac extra data, the AAC sequence header,
    * without the flv codec header,
    * @see: ffmpeg, AVCodecContext::extradata
    */
    int             aac_extra_size;
    char*           aac_extra_data;
public:
    // for sequence header, whether parse the h.264 sps.
    bool            avc_parse_sps;
public:
    BrsAvcAacCodec();
    virtual ~BrsAvcAacCodec();
public:
    // whether avc or aac codec sequence header or extra data is decoded ok.
    virtual bool is_avc_codec_ok();
    virtual bool is_aac_codec_ok();
// the following function used for hls to build the sample and codec.
public:
    /**
    * demux the audio packet in aac codec.
    * the packet mux in FLV/RTMP format defined in flv specification.
    * demux the audio speicified data(sound_format, sound_size, ...) to sample.
    * demux the aac specified data(aac_profile, ...) to codec from sequence header.
    * demux the aac raw to sample units.
    */
    virtual int audio_aac_demux(char* data, int size, BrsCodecSample* sample);
    virtual int audio_mp3_demux(char* data, int size, BrsCodecSample* sample);
    /**
    * demux the video packet in h.264 codec.
    * the packet mux in FLV/RTMP format defined in flv specification.
    * demux the video specified data(frame_type, codec_id, ...) to sample.
    * demux the h.264 sepcified data(avc_profile, ...) to codec from sequence header.
    * demux the h.264 NALUs to sampe units.
    */
    virtual int video_avc_demux(char* data, int size, BrsCodecSample* sample);
public:
    /**
    * directly demux the sequence header, without RTMP packet header.
    */
    virtual int audio_aac_sequence_header_demux(char* data, int size);
private:
    /**
    * when avc packet type is BrsCodecVideoAVCTypeSequenceHeader,
    * decode the sps and pps.
    */
    virtual int avc_demux_sps_pps(BRSStream* stream);
    /**
     * decode the sps rbsp stream.
     */
    virtual int avc_demux_sps();
    virtual int avc_demux_sps_rbsp(char* rbsp, int nb_rbsp);
    /**
    * demux the avc NALU in "AnnexB" 
    * from H.264-AVC-ISO_IEC_14496-10.pdf, page 211.
    */
    virtual int avc_demux_annexb_format(BRSStream* stream, BrsCodecSample* sample);
    /**
    * demux the avc NALU in "ISO Base Media File Format" 
    * from H.264-AVC-ISO_IEC_14496-15.pdf, page 20
    */
    virtual int avc_demux_ibmf_format(BRSStream* stream, BrsCodecSample* sample);
};
#endif
}

