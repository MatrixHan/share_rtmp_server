#include <BRSKernelCodec.h>

#include <string.h>
#include <stdlib.h>
using namespace std;

#include <BRSKernelError.h>
#include <BRSLog.h>
#include <BRSStream.h>
#include <BRSKernelUtility.h>
#include <BRSAutoFree.h>

namespace BRS{

string brs_codec_video2str(BrsCodecVideo codec)
{
    switch (codec) {
        case BrsCodecVideoAVC: 
            return "H264";
        case BrsCodecVideoOn2VP6:
        case BrsCodecVideoOn2VP6WithAlphaChannel:
            return "VP6";
        case BrsCodecVideoReserved:
        case BrsCodecVideoReserved1:
        case BrsCodecVideoReserved2:
        case BrsCodecVideoDisabled:
        case BrsCodecVideoSorensonH263:
        case BrsCodecVideoScreenVideo:
        case BrsCodecVideoScreenVideoVersion2:
        default:
            return "Other";
    }
}

string brs_codec_audio2str(BrsCodecAudio codec)
{
    switch (codec) {
        case BrsCodecAudioAAC:
            return "AAC";
        case BrsCodecAudioMP3:
            return "MP3";
        case BrsCodecAudioReserved1:
        case BrsCodecAudioLinearPCMPlatformEndian:
        case BrsCodecAudioADPCM:
        case BrsCodecAudioLinearPCMLittleEndian:
        case BrsCodecAudioNellymoser16kHzMono:
        case BrsCodecAudioNellymoser8kHzMono:
        case BrsCodecAudioNellymoser:
        case BrsCodecAudioReservedG711AlawLogarithmicPCM:
        case BrsCodecAudioReservedG711MuLawLogarithmicPCM:
        case BrsCodecAudioReserved:
        case BrsCodecAudioSpeex:
        case BrsCodecAudioReservedMP3_8kHz:
        case BrsCodecAudioReservedDeviceSpecificSound:
        default:
            return "Other";
    }
}

string brs_codec_aac_profile2str(BrsAacProfile aac_profile)
{
    switch (aac_profile) {
        case BrsAacProfileMain: return "Main";
        case BrsAacProfileLC: return "LC";
        case BrsAacProfileSSR: return "SSR";
        default: return "Other";
    }
}

string brs_codec_aac_object2str(BrsAacObjectType aac_object)
{
    switch (aac_object) {
        case BrsAacObjectTypeAacMain: return "Main";
        case BrsAacObjectTypeAacHE: return "HE";
        case BrsAacObjectTypeAacHEV2: return "HEv2";
        case BrsAacObjectTypeAacLC: return "LC";
        case BrsAacObjectTypeAacSSR: return "SSR";
        default: return "Other";
    }
}

BrsAacObjectType brs_codec_aac_ts2rtmp(BrsAacProfile profile)
{
    switch (profile) {
        case BrsAacProfileMain: return BrsAacObjectTypeAacMain;
        case BrsAacProfileLC: return BrsAacObjectTypeAacLC;
        case BrsAacProfileSSR: return BrsAacObjectTypeAacSSR;
        default: return BrsAacObjectTypeReserved;
    }
}

BrsAacProfile brs_codec_aac_rtmp2ts(BrsAacObjectType object_type)
{
    switch (object_type) {
        case BrsAacObjectTypeAacMain: return BrsAacProfileMain;
        case BrsAacObjectTypeAacHE:
        case BrsAacObjectTypeAacHEV2:
        case BrsAacObjectTypeAacLC: return BrsAacProfileLC;
        case BrsAacObjectTypeAacSSR: return BrsAacProfileSSR;
        default: return BrsAacProfileReserved;
    }
}

string brs_codec_avc_profile2str(BrsAvcProfile profile)
{
    switch (profile) {
        case BrsAvcProfileBaseline: return "Baseline";
        case BrsAvcProfileConstrainedBaseline: return "Baseline(Constrained)";
        case BrsAvcProfileMain: return "Main";
        case BrsAvcProfileExtended: return "Extended";
        case BrsAvcProfileHigh: return "High";
        case BrsAvcProfileHigh10: return "High(10)";
        case BrsAvcProfileHigh10Intra: return "High(10+Intra)";
        case BrsAvcProfileHigh422: return "High(422)";
        case BrsAvcProfileHigh422Intra: return "High(422+Intra)";
        case BrsAvcProfileHigh444: return "High(444)";
        case BrsAvcProfileHigh444Predictive: return "High(444+Predictive)";
        case BrsAvcProfileHigh444Intra: return "High(444+Intra)";
        default: return "Other";
    }
}

string brs_codec_avc_level2str(BrsAvcLevel level)
{
    switch (level) {
        case BrsAvcLevel_1: return "1";
        case BrsAvcLevel_11: return "1.1";
        case BrsAvcLevel_12: return "1.2";
        case BrsAvcLevel_13: return "1.3";
        case BrsAvcLevel_2: return "2";
        case BrsAvcLevel_21: return "2.1";
        case BrsAvcLevel_22: return "2.2";
        case BrsAvcLevel_3: return "3";
        case BrsAvcLevel_31: return "3.1";
        case BrsAvcLevel_32: return "3.2";
        case BrsAvcLevel_4: return "4";
        case BrsAvcLevel_41: return "4.1";
        case BrsAvcLevel_5: return "5";
        case BrsAvcLevel_51: return "5.1";
        default: return "Other";
    }
}

/**
* the public data, event HLS disable, others can use it.
*/
// 0 = 5.5 kHz = 5512 Hz
// 1 = 11 kHz = 11025 Hz
// 2 = 22 kHz = 22050 Hz
// 3 = 44 kHz = 44100 Hz
int flv_sample_rates[] = {5512, 11025, 22050, 44100};

// the sample rates in the codec,
// in the sequence header.
int aac_sample_rates[] = 
{
    96000, 88200, 64000, 48000,
    44100, 32000, 24000, 22050,
    16000, 12000, 11025,  8000,
    7350,     0,     0,    0
};

BrsFlvCodec::BrsFlvCodec()
{
}

BrsFlvCodec::~BrsFlvCodec()
{
}

bool BrsFlvCodec::video_is_keyframe(char* data, int size)
{
    // 2bytes required.
    if (size < 1) {
        return false;
    }

    char frame_type = data[0];
    frame_type = (frame_type >> 4) & 0x0F;
    
    return frame_type == BrsCodecVideoAVCFrameKeyFrame;
}

bool BrsFlvCodec::video_is_sequence_header(char* data, int size)
{
    // sequence header only for h264
    if (!video_is_h264(data, size)) {
        return false;
    }
    
    // 2bytes required.
    if (size < 2) {
        return false;
    }

    char frame_type = data[0];
    frame_type = (frame_type >> 4) & 0x0F;

    char avc_packet_type = data[1];
    
    return frame_type == BrsCodecVideoAVCFrameKeyFrame 
        && avc_packet_type == BrsCodecVideoAVCTypeSequenceHeader;
}

bool BrsFlvCodec::audio_is_sequence_header(char* data, int size)
{
    // sequence header only for aac
    if (!audio_is_aac(data, size)) {
        return false;
    }
    
    // 2bytes required.
    if (size < 2) {
        return false;
    }
    
    char aac_packet_type = data[1];
    
    return aac_packet_type == BrsCodecAudioTypeSequenceHeader;
}

bool BrsFlvCodec::video_is_h264(char* data, int size)
{
    // 1bytes required.
    if (size < 1) {
        return false;
    }

    char codec_id = data[0];
    codec_id = codec_id & 0x0F;
    
    return codec_id == BrsCodecVideoAVC;
}

bool BrsFlvCodec::audio_is_aac(char* data, int size)
{
    // 1bytes required.
    if (size < 1) {
        return false;
    }
    
    char sound_format = data[0];
    sound_format = (sound_format >> 4) & 0x0F;
    
    return sound_format == BrsCodecAudioAAC;
}

bool BrsFlvCodec::video_is_acceptable(char* data, int size)
{
    // 1bytes required.
    if (size < 1) {
        return false;
    }
    
    char frame_type = data[0];
    char codec_id = frame_type & 0x0f;
    frame_type = (frame_type >> 4) & 0x0f;
    
    if (frame_type < 1 || frame_type > 5) {
        return false;
    }
    
    if (codec_id < 2 || codec_id > 7) {
        return false;
    }
    
    return true;
}

string brs_codec_avc_nalu2str(BrsAvcNaluType nalu_type)
{
    switch (nalu_type) {
        case BrsAvcNaluTypeNonIDR: return "NonIDR";
        case BrsAvcNaluTypeDataPartitionA: return "DataPartitionA";
        case BrsAvcNaluTypeDataPartitionB: return "DataPartitionB";
        case BrsAvcNaluTypeDataPartitionC: return "DataPartitionC";
        case BrsAvcNaluTypeIDR: return "IDR";
        case BrsAvcNaluTypeSEI: return "SEI";
        case BrsAvcNaluTypeSPS: return "SPS";
        case BrsAvcNaluTypePPS: return "PPS";
        case BrsAvcNaluTypeAccessUnitDelimiter: return "AccessUnitDelimiter";
        case BrsAvcNaluTypeEOSequence: return "EOSequence";
        case BrsAvcNaluTypeEOStream: return "EOStream";
        case BrsAvcNaluTypeFilterData: return "FilterData";
        case BrsAvcNaluTypeSPSExt: return "SPSExt";
        case BrsAvcNaluTypePrefixNALU: return "PrefixNALU";
        case BrsAvcNaluTypeSubsetSPS: return "SubsetSPS";
        case BrsAvcNaluTypeLayerWithoutPartition: return "LayerWithoutPartition";
        case BrsAvcNaluTypeCodedSliceExt: return "CodedSliceExt";
        case BrsAvcNaluTypeReserved: default: return "Other";
    }
}

BrsCodecSampleUnit::BrsCodecSampleUnit()
{
    size = 0;
    bytes = NULL;
}

BrsCodecSampleUnit::~BrsCodecSampleUnit()
{
}

BrsCodecSample::BrsCodecSample()
{
    clear();
}

BrsCodecSample::~BrsCodecSample()
{
}

void BrsCodecSample::clear()
{
    is_video = false;
    nb_sample_units = 0;

    cts = 0;
    frame_type = BrsCodecVideoAVCFrameReserved;
    avc_packet_type = BrsCodecVideoAVCTypeReserved;
    has_idr = false;
    first_nalu_type = BrsAvcNaluTypeReserved;
    
    acodec = BrsCodecAudioReserved1;
    sound_rate = BrsCodecAudioSampleRateReserved;
    sound_size = BrsCodecAudioSampleSizeReserved;
    sound_type = BrsCodecAudioSoundTypeReserved;
    aac_packet_type = BrsCodecAudioTypeReserved;
}

int BrsCodecSample::add_sample_unit(char* bytes, int size)
{
    int ret = ERROR_SUCCESS;
    
    if (nb_sample_units >= SRS_SRS_MAX_CODEC_SAMPLE) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("hls decode samples error, "
            "exceed the max count: %d, ret=%d", SRS_SRS_MAX_CODEC_SAMPLE, ret);
        return ret;
    }
    
    BrsCodecSampleUnit* sample_unit = &sample_units[nb_sample_units++];
    sample_unit->bytes = bytes;
    sample_unit->size = size;
    
    // for video, parse the nalu type, set the IDR flag.
    if (is_video) {
        BrsAvcNaluType nal_unit_type = (BrsAvcNaluType)(bytes[0] & 0x1f);
        
        if (nal_unit_type == BrsAvcNaluTypeIDR) {
            has_idr = true;
        }
    
        if (first_nalu_type == BrsAvcNaluTypeReserved) {
            first_nalu_type = nal_unit_type;
        }
    }
    
    return ret;
}

#if !defined(SRS_EXPORT_LIBRTMP)

BrsAvcAacCodec::BrsAvcAacCodec()
{
    avc_parse_sps               = true;
    
    width                       = 0;
    height                      = 0;
    duration                    = 0;
    NAL_unit_length             = 0;
    frame_rate                  = 0;

    video_data_rate             = 0;
    video_codec_id              = 0;

    audio_data_rate             = 0;
    audio_codec_id              = 0;

    avc_profile                 = BrsAvcProfileReserved;
    avc_level                   = BrsAvcLevelReserved;
    aac_object                  = BrsAacObjectTypeReserved;
    aac_sample_rate             = SRS_AAC_SAMPLE_RATE_UNSET; // sample rate ignored
    aac_channels                = 0;
    avc_extra_size              = 0;
    avc_extra_data              = NULL;
    aac_extra_size              = 0;
    aac_extra_data              = NULL;

    sequenceParameterSetLength  = 0;
    sequenceParameterSetNALUnit = NULL;
    pictureParameterSetLength   = 0;
    pictureParameterSetNALUnit  = NULL;

    payload_format = BrsAvcPayloadFormatGuess;
    stream = new BRSStream();
}

BrsAvcAacCodec::~BrsAvcAacCodec()
{
    SafeDeleteArray(avc_extra_data);
    SafeDeleteArray(aac_extra_data);

    SafeDelete(stream);
    SafeDeleteArray(sequenceParameterSetNALUnit);
    SafeDeleteArray(pictureParameterSetNALUnit);
}

bool BrsAvcAacCodec::is_avc_codec_ok()
{
    return avc_extra_size > 0 && avc_extra_data;
}

bool BrsAvcAacCodec::is_aac_codec_ok()
{
    return aac_extra_size > 0 && aac_extra_data;
}

int BrsAvcAacCodec::audio_aac_demux(char* data, int size, BrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;
    
    sample->is_video = false;
    
    if (!data || size <= 0) {
        brs_trace("no audio present, ignore it.");
        return ret;
    }
    
    if ((ret = stream->initialize(data, size)) != ERROR_SUCCESS) {
        return ret;
    }

    // audio decode
    if (!stream->require(1)) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("aac decode sound_format failed. ret=%d", ret);
        return ret;
    }
    
    // @see: E.4.2 Audio Tags, video_file_format_spec_v10_1.pdf, page 76
    int8_t sound_format = stream->read_1bytes();
    
    int8_t sound_type = sound_format & 0x01;
    int8_t sound_size = (sound_format >> 1) & 0x01;
    int8_t sound_rate = (sound_format >> 2) & 0x03;
    sound_format = (sound_format >> 4) & 0x0f;
    
    audio_codec_id = sound_format;
    sample->acodec = (BrsCodecAudio)audio_codec_id;

    sample->sound_type = (BrsCodecAudioSoundType)sound_type;
    sample->sound_rate = (BrsCodecAudioSampleRate)sound_rate;
    sample->sound_size = (BrsCodecAudioSampleSize)sound_size;

    // we support h.264+mp3 for hls.
    if (audio_codec_id == BrsCodecAudioMP3) {
        return ERROR_HLS_TRY_MP3;
    }
    
    // only support aac
    if (audio_codec_id != BrsCodecAudioAAC) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("aac only support mp3/aac codec. actual=%d, ret=%d", audio_codec_id, ret);
        return ret;
    }

    if (!stream->require(1)) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("aac decode aac_packet_type failed. ret=%d", ret);
        return ret;
    }
    
    int8_t aac_packet_type = stream->read_1bytes();
    sample->aac_packet_type = (BrsCodecAudioType)aac_packet_type;
    
    if (aac_packet_type == BrsCodecAudioTypeSequenceHeader) {
        // AudioSpecificConfig
        // 1.6.2.1 AudioSpecificConfig, in aac-mp4a-format-ISO_IEC_14496-3+2001.pdf, page 33.
        aac_extra_size = stream->size() - stream->pos();
        if (aac_extra_size > 0) {
            SafeDeleteArray(aac_extra_data);
            aac_extra_data = new char[aac_extra_size];
            memcpy(aac_extra_data, stream->data() + stream->pos(), aac_extra_size);

            // demux the sequence header.
            if ((ret = audio_aac_sequence_header_demux(aac_extra_data, aac_extra_size)) != ERROR_SUCCESS) {
                return ret;
            }
        }
    } else if (aac_packet_type == BrsCodecAudioTypeRawData) {
        // ensure the sequence header demuxed
        if (!is_aac_codec_ok()) {
            brs_warn("aac ignore type=%d for no sequence header. ret=%d", aac_packet_type, ret);
            return ret;
        }
        
        // Raw AAC frame data in UI8 []
        // 6.3 Raw Data, aac-iso-13818-7.pdf, page 28
        if ((ret = sample->add_sample_unit(stream->data() + stream->pos(), stream->size() - stream->pos())) != ERROR_SUCCESS) {
            brs_error("aac add sample failed. ret=%d", ret);
            return ret;
        }
    } else {
        // ignored.
    }
    
    // reset the sample rate by sequence header
    if (aac_sample_rate != SRS_AAC_SAMPLE_RATE_UNSET) {
        static int aac_sample_rates[] = {
            96000, 88200, 64000, 48000,
            44100, 32000, 24000, 22050,
            16000, 12000, 11025,  8000,
            7350,     0,     0,    0
        };
        switch (aac_sample_rates[aac_sample_rate]) {
            case 11025:
                sample->sound_rate = BrsCodecAudioSampleRate11025;
                break;
            case 22050:
                sample->sound_rate = BrsCodecAudioSampleRate22050;
                break;
            case 44100:
                sample->sound_rate = BrsCodecAudioSampleRate44100;
                break;
            default:
                break;
        };
    }
    
    brs_info("aac decoded, type=%d, codec=%d, asize=%d, rate=%d, format=%d, size=%d",
        sound_type, audio_codec_id, sound_size, sound_rate, sound_format, size);
    
    return ret;
}

int BrsAvcAacCodec::audio_mp3_demux(char* data, int size, BrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;

    // we always decode aac then mp3.
    assert(sample->acodec == BrsCodecAudioMP3);
    
    // @see: E.4.2 Audio Tags, video_file_format_spec_v10_1.pdf, page 76
    if (!data || size <= 1) {
        brs_trace("no mp3 audio present, ignore it.");
        return ret;
    }

    // mp3 payload.
    if ((ret = sample->add_sample_unit(data + 1, size - 1)) != ERROR_SUCCESS) {
        brs_error("audio codec add mp3 sample failed. ret=%d", ret);
        return ret;
    }
    
    brs_info("audio decoded, type=%d, codec=%d, asize=%d, rate=%d, format=%d, size=%d", 
        sample->sound_type, audio_codec_id, sample->sound_size, sample->sound_rate, sample->acodec, size);
    
    return ret;
}

int BrsAvcAacCodec::audio_aac_sequence_header_demux(char* data, int size)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = stream->initialize(data, size)) != ERROR_SUCCESS) {
        return ret;
    }
        
    // only need to decode the first 2bytes:
    //      audioObjectType, aac_profile, 5bits.
    //      samplingFrequencyIndex, aac_sample_rate, 4bits.
    //      channelConfiguration, aac_channels, 4bits
    if (!stream->require(2)) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("audio codec decode aac sequence header failed. ret=%d", ret);
        return ret;
    }
    u_int8_t profile_ObjectType = stream->read_1bytes();
    u_int8_t samplingFrequencyIndex = stream->read_1bytes();
        
    aac_channels = (samplingFrequencyIndex >> 3) & 0x0f;
    samplingFrequencyIndex = ((profile_ObjectType << 1) & 0x0e) | ((samplingFrequencyIndex >> 7) & 0x01);
    profile_ObjectType = (profile_ObjectType >> 3) & 0x1f;

    // set the aac sample rate.
    aac_sample_rate = samplingFrequencyIndex;

    // convert the object type in sequence header to aac profile of ADTS.
    aac_object = (BrsAacObjectType)profile_ObjectType;
    if (aac_object == BrsAacObjectTypeReserved) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("audio codec decode aac sequence header failed, "
            "adts object=%d invalid. ret=%d", profile_ObjectType, ret);
        return ret;
    }
        
    // TODO: FIXME: to support aac he/he-v2, see: ngx_rtmp_codec_parse_aac_header
    // @see: https://github.com/winlinvip/nginx-rtmp-module/commit/3a5f9eea78fc8d11e8be922aea9ac349b9dcbfc2
    // 
    // donot force to LC, @see: https://github.com/osbrs/brs/issues/81
    // the source will print the sequence header info.
    //if (aac_profile > 3) {
        // Mark all extended profiles as LC
        // to make Android as happy as possible.
        // @see: ngx_rtmp_hls_parse_aac_header
        //aac_profile = 1;
    //}

    return ret;
}

int BrsAvcAacCodec::video_avc_demux(char* data, int size, BrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;
    
    sample->is_video = true;
    
    if (!data || size <= 0) {
        brs_trace("no video present, ignore it.");
        return ret;
    }
    
    if ((ret = stream->initialize(data, size)) != ERROR_SUCCESS) {
        return ret;
    }

    // video decode
    if (!stream->require(1)) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("avc decode frame_type failed. ret=%d", ret);
        return ret;
    }
    
    // @see: E.4.3 Video Tags, video_file_format_spec_v10_1.pdf, page 78
    int8_t frame_type = stream->read_1bytes();
    int8_t codec_id = frame_type & 0x0f;
    frame_type = (frame_type >> 4) & 0x0f;
    
    sample->frame_type = (BrsCodecVideoAVCFrame)frame_type;
    
    // ignore info frame without error,
    // @see https://github.com/osbrs/brs/issues/288#issuecomment-69863909
    if (sample->frame_type == BrsCodecVideoAVCFrameVideoInfoFrame) {
        brs_warn("avc igone the info frame, ret=%d", ret);
        return ret;
    }
    
    // only support h.264/avc
    if (codec_id != BrsCodecVideoAVC) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("avc only support video h.264/avc codec. actual=%d, ret=%d", codec_id, ret);
        return ret;
    }
    video_codec_id = codec_id;
    
    if (!stream->require(4)) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("avc decode avc_packet_type failed. ret=%d", ret);
        return ret;
    }
    int8_t avc_packet_type = stream->read_1bytes();
    int32_t composition_time = stream->read_3bytes();
    
    // pts = dts + cts.
    sample->cts = composition_time;
    sample->avc_packet_type = (BrsCodecVideoAVCType)avc_packet_type;
    
    if (avc_packet_type == BrsCodecVideoAVCTypeSequenceHeader) {
        if ((ret = avc_demux_sps_pps(stream)) != ERROR_SUCCESS) {
            return ret;
        }
    } else if (avc_packet_type == BrsCodecVideoAVCTypeNALU){
        // ensure the sequence header demuxed
        if (!is_avc_codec_ok()) {
            brs_warn("avc ignore type=%d for no sequence header. ret=%d", avc_packet_type, ret);
            return ret;
        }

        // guess for the first time.
        if (payload_format == BrsAvcPayloadFormatGuess) {
            // One or more NALUs (Full frames are required)
            // try  "AnnexB" from H.264-AVC-ISO_IEC_14496-10.pdf, page 211.
            if ((ret = avc_demux_annexb_format(stream, sample)) != ERROR_SUCCESS) {
                // stop try when system error.
                if (ret != ERROR_HLS_AVC_TRY_OTHERS) {
                    brs_error("avc demux for annexb failed. ret=%d", ret);
                    return ret;
                }
                
                // try "ISO Base Media File Format" from H.264-AVC-ISO_IEC_14496-15.pdf, page 20
                if ((ret = avc_demux_ibmf_format(stream, sample)) != ERROR_SUCCESS) {
                    return ret;
                } else {
                    payload_format = BrsAvcPayloadFormatIbmf;
                    brs_info("hls guess avc payload is ibmf format.");
                }
            } else {
                payload_format = BrsAvcPayloadFormatAnnexb;
                brs_info("hls guess avc payload is annexb format.");
            }
        } else if (payload_format == BrsAvcPayloadFormatIbmf) {
            // try "ISO Base Media File Format" from H.264-AVC-ISO_IEC_14496-15.pdf, page 20
            if ((ret = avc_demux_ibmf_format(stream, sample)) != ERROR_SUCCESS) {
                return ret;
            }
            brs_info("hls decode avc payload in ibmf format.");
        } else {
            // One or more NALUs (Full frames are required)
            // try  "AnnexB" from H.264-AVC-ISO_IEC_14496-10.pdf, page 211.
            if ((ret = avc_demux_annexb_format(stream, sample)) != ERROR_SUCCESS) {
                // ok, we guess out the payload is annexb, but maybe changed to ibmf.
                if (ret != ERROR_HLS_AVC_TRY_OTHERS) {
                    brs_error("avc demux for annexb failed. ret=%d", ret);
                    return ret;
                }
                
                // try "ISO Base Media File Format" from H.264-AVC-ISO_IEC_14496-15.pdf, page 20
                if ((ret = avc_demux_ibmf_format(stream, sample)) != ERROR_SUCCESS) {
                    return ret;
                } else {
                    payload_format = BrsAvcPayloadFormatIbmf;
                    brs_warn("hls avc payload change from annexb to ibmf format.");
                }
            }
            brs_info("hls decode avc payload in annexb format.");
        }
    } else {
        // ignored.
    }
    
    brs_info("avc decoded, type=%d, codec=%d, avc=%d, cts=%d, size=%d",
        frame_type, video_codec_id, avc_packet_type, composition_time, size);
    
    return ret;
}

int BrsAvcAacCodec::avc_demux_sps_pps(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    // AVCDecoderConfigurationRecord
    // 5.2.4.1.1 Syntax, H.264-AVC-ISO_IEC_14496-15.pdf, page 16
    avc_extra_size = stream->size() - stream->pos();
    if (avc_extra_size > 0) {
        SafeDeleteArray(avc_extra_data);
        avc_extra_data = new char[avc_extra_size];
        memcpy(avc_extra_data, stream->data() + stream->pos(), avc_extra_size);
    }
    
    if (!stream->require(6)) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("avc decode sequenc header failed. ret=%d", ret);
        return ret;
    }
    //int8_t configurationVersion = stream->read_1bytes();
    stream->read_1bytes();
    //int8_t AVCProfileIndication = stream->read_1bytes();
    avc_profile = (BrsAvcProfile)stream->read_1bytes();
    //int8_t profile_compatibility = stream->read_1bytes();
    stream->read_1bytes();
    //int8_t AVCLevelIndication = stream->read_1bytes();
    avc_level = (BrsAvcLevel)stream->read_1bytes();
    
    // parse the NALU size.
    int8_t lengthSizeMinusOne = stream->read_1bytes();
    lengthSizeMinusOne &= 0x03;
    NAL_unit_length = lengthSizeMinusOne;
    
    // 5.3.4.2.1 Syntax, H.264-AVC-ISO_IEC_14496-15.pdf, page 16
    // 5.2.4.1 AVC decoder configuration record
    // 5.2.4.1.2 Semantics
    // The value of this field shall be one of 0, 1, or 3 corresponding to a
    // length encoded with 1, 2, or 4 bytes, respectively.
    if (NAL_unit_length == 2) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("sps lengthSizeMinusOne should never be 2. ret=%d", ret);
        return ret;
    }
    
    // 1 sps, 7.3.2.1 Sequence parameter set RBSP syntax
    // H.264-AVC-ISO_IEC_14496-10.pdf, page 45.
    if (!stream->require(1)) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("avc decode sequenc header sps failed. ret=%d", ret);
        return ret;
    }
    int8_t numOfSequenceParameterSets = stream->read_1bytes();
    numOfSequenceParameterSets &= 0x1f;
    if (numOfSequenceParameterSets != 1) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("avc decode sequenc header sps failed. ret=%d", ret);
        return ret;
    }
    if (!stream->require(2)) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("avc decode sequenc header sps size failed. ret=%d", ret);
        return ret;
    }
    sequenceParameterSetLength = stream->read_2bytes();
    if (!stream->require(sequenceParameterSetLength)) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("avc decode sequenc header sps data failed. ret=%d", ret);
        return ret;
    }
    if (sequenceParameterSetLength > 0) {
        SafeDeleteArray(sequenceParameterSetNALUnit);
        sequenceParameterSetNALUnit = new char[sequenceParameterSetLength];
        stream->read_bytes(sequenceParameterSetNALUnit, sequenceParameterSetLength);
    }
    // 1 pps
    if (!stream->require(1)) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("avc decode sequenc header pps failed. ret=%d", ret);
        return ret;
    }
    int8_t numOfPictureParameterSets = stream->read_1bytes();
    numOfPictureParameterSets &= 0x1f;
    if (numOfPictureParameterSets != 1) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("avc decode sequenc header pps failed. ret=%d", ret);
        return ret;
    }
    if (!stream->require(2)) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("avc decode sequenc header pps size failed. ret=%d", ret);
        return ret;
    }
    pictureParameterSetLength = stream->read_2bytes();
    if (!stream->require(pictureParameterSetLength)) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("avc decode sequenc header pps data failed. ret=%d", ret);
        return ret;
    }
    if (pictureParameterSetLength > 0) {
        SafeDeleteArray(pictureParameterSetNALUnit);
        pictureParameterSetNALUnit = new char[pictureParameterSetLength];
        stream->read_bytes(pictureParameterSetNALUnit, pictureParameterSetLength);
    }
    
    return avc_demux_sps();
}

int BrsAvcAacCodec::avc_demux_sps()
{
    int ret = ERROR_SUCCESS;
    
    if (!sequenceParameterSetLength) {
        return ret;
    }
    
    BRSStream stream;
    if ((ret = stream.initialize(sequenceParameterSetNALUnit, sequenceParameterSetLength)) != ERROR_SUCCESS) {
        return ret;
    }
    
    // for NALU, 7.3.1 NAL unit syntax
    // H.264-AVC-ISO_IEC_14496-10-2012.pdf, page 61.
    if (!stream.require(1)) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("avc decode sps failed. ret=%d", ret);
        return ret;
    }
    int8_t nutv = stream.read_1bytes();
    
    // forbidden_zero_bit shall be equal to 0.
    int8_t forbidden_zero_bit = (nutv >> 7) & 0x01;
    if (forbidden_zero_bit) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("forbidden_zero_bit shall be equal to 0. ret=%d", ret);
        return ret;
    }
    
    // nal_ref_idc not equal to 0 specifies that the content of the NAL unit contains a sequence parameter set or a picture
    // parameter set or a slice of a reference picture or a slice data partition of a reference picture.
    int8_t nal_ref_idc = (nutv >> 5) & 0x03;
    if (!nal_ref_idc) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("for sps, nal_ref_idc shall be not be equal to 0. ret=%d", ret);
        return ret;
    }
    
    // 7.4.1 NAL unit semantics
    // H.264-AVC-ISO_IEC_14496-10-2012.pdf, page 61.
    // nal_unit_type specifies the type of RBSP data structure contained in the NAL unit as specified in Table 7-1.
    BrsAvcNaluType nal_unit_type = (BrsAvcNaluType)(nutv & 0x1f);
    if (nal_unit_type != 7) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("for sps, nal_unit_type shall be equal to 7. ret=%d", ret);
        return ret;
    }
    
    // decode the rbsp from sps.
    // rbsp[ i ] a raw byte sequence payload is specified as an ordered sequence of bytes.
    int8_t* rbsp = new int8_t[sequenceParameterSetLength];
    BrsAutoFreeA(int8_t, rbsp);
    
    int nb_rbsp = 0;
    while (!stream.empty()) {
        rbsp[nb_rbsp] = stream.read_1bytes();
        
        // XX 00 00 03 XX, the 03 byte should be drop.
        if (nb_rbsp > 2 && rbsp[nb_rbsp - 2] == 0 && rbsp[nb_rbsp - 1] == 0 && rbsp[nb_rbsp] == 3) {
            // read 1byte more.
            if (stream.empty()) {
                break;
            }
            rbsp[nb_rbsp] = stream.read_1bytes();
            nb_rbsp++;
            
            continue;
        }
        
        nb_rbsp++;
    }
    
    return avc_demux_sps_rbsp((char*)rbsp, nb_rbsp);
}


int BrsAvcAacCodec::avc_demux_sps_rbsp(char* rbsp, int nb_rbsp)
{
    int ret = ERROR_SUCCESS;
    
    // we donot parse the detail of sps.
    // @see https://github.com/osbrs/brs/issues/474
    if (!avc_parse_sps) {
        return ret;
    }
    
    // reparse the rbsp.
    BRSStream stream;
    if ((ret = stream.initialize(rbsp, nb_rbsp)) != ERROR_SUCCESS) {
        return ret;
    }
    
    // for SPS, 7.3.2.1.1 Sequence parameter set data syntax
    // H.264-AVC-ISO_IEC_14496-10-2012.pdf, page 62.
    if (!stream.require(3)) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("sps shall atleast 3bytes. ret=%d", ret);
        return ret;
    }
    u_int8_t profile_idc = stream.read_1bytes();
    if (!profile_idc) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("sps the profile_idc invalid. ret=%d", ret);
        return ret;
    }
    
    int8_t flags = stream.read_1bytes();
    if (flags & 0x03) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("sps the flags invalid. ret=%d", ret);
        return ret;
    }
    
    u_int8_t level_idc = stream.read_1bytes();
    if (!level_idc) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("sps the level_idc invalid. ret=%d", ret);
        return ret;
    }
    
    BRSBitStream bs;
    if ((ret = bs.initialize(&stream)) != ERROR_SUCCESS) {
        return ret;
    }
    
    int32_t seq_parameter_set_id = -1;
    if ((ret = brs_avc_nalu_read_uev(&bs, seq_parameter_set_id)) != ERROR_SUCCESS) {
        return ret;
    }
    if (seq_parameter_set_id < 0) {
        ret = ERROR_HLS_DECODE_ERROR;
        brs_error("sps the seq_parameter_set_id invalid. ret=%d", ret);
        return ret;
    }
    brs_info("sps parse profile=%d, level=%d, sps_id=%d", profile_idc, level_idc, seq_parameter_set_id);
    
    int32_t chroma_format_idc = -1;
    if (profile_idc == 100 || profile_idc == 110 || profile_idc == 122 || profile_idc == 244
        || profile_idc == 44 || profile_idc == 83 || profile_idc == 86 || profile_idc == 118
        || profile_idc == 128
    ) {
        if ((ret = brs_avc_nalu_read_uev(&bs, chroma_format_idc)) != ERROR_SUCCESS) {
            return ret;
        }
        if (chroma_format_idc == 3) {
            int8_t separate_colour_plane_flag = -1;
            if ((ret = brs_avc_nalu_read_bit(&bs, separate_colour_plane_flag)) != ERROR_SUCCESS) {
                return ret;
            }
        }
        
        int32_t bit_depth_luma_minus8 = -1;
        if ((ret = brs_avc_nalu_read_uev(&bs, bit_depth_luma_minus8)) != ERROR_SUCCESS) {
            return ret;
        }
        
        int32_t bit_depth_chroma_minus8 = -1;
        if ((ret = brs_avc_nalu_read_uev(&bs, bit_depth_chroma_minus8)) != ERROR_SUCCESS) {
            return ret;
        }
        
        int8_t qpprime_y_zero_transform_bypass_flag = -1;
        if ((ret = brs_avc_nalu_read_bit(&bs, qpprime_y_zero_transform_bypass_flag)) != ERROR_SUCCESS) {
            return ret;
        }
        
        int8_t seq_scaling_matrix_present_flag = -1;
        if ((ret = brs_avc_nalu_read_bit(&bs, seq_scaling_matrix_present_flag)) != ERROR_SUCCESS) {
            return ret;
        }
        if (seq_scaling_matrix_present_flag) {
            int nb_scmpfs = ((chroma_format_idc != 3)? 8:12);
            for (int i = 0; i < nb_scmpfs; i++) {
                int8_t seq_scaling_matrix_present_flag_i = -1;
                if ((ret = brs_avc_nalu_read_bit(&bs, seq_scaling_matrix_present_flag_i)) != ERROR_SUCCESS) {
                    return ret;
                }
                if (seq_scaling_matrix_present_flag_i) {
                    ret = ERROR_HLS_DECODE_ERROR;
                    brs_error("sps the seq_scaling_matrix_present_flag invalid, i=%d, nb_scmpfs=%d. ret=%d", i, nb_scmpfs, ret);
                    return ret;
                }
            }
        }
    }
    
    int32_t log2_max_frame_num_minus4 = -1;
    if ((ret = brs_avc_nalu_read_uev(&bs, log2_max_frame_num_minus4)) != ERROR_SUCCESS) {
        return ret;
    }
    
    int32_t pic_order_cnt_type = -1;
    if ((ret = brs_avc_nalu_read_uev(&bs, pic_order_cnt_type)) != ERROR_SUCCESS) {
        return ret;
    }
    
    if (pic_order_cnt_type == 0) {
        int32_t log2_max_pic_order_cnt_lsb_minus4 = -1;
        if ((ret = brs_avc_nalu_read_uev(&bs, log2_max_pic_order_cnt_lsb_minus4)) != ERROR_SUCCESS) {
            return ret;
        }
    } else if (pic_order_cnt_type == 1) {
        int8_t delta_pic_order_always_zero_flag = -1;
        if ((ret = brs_avc_nalu_read_bit(&bs, delta_pic_order_always_zero_flag)) != ERROR_SUCCESS) {
            return ret;
        }
        
        int32_t offset_for_non_ref_pic = -1;
        if ((ret = brs_avc_nalu_read_uev(&bs, offset_for_non_ref_pic)) != ERROR_SUCCESS) {
            return ret;
        }
        
        int32_t offset_for_top_to_bottom_field = -1;
        if ((ret = brs_avc_nalu_read_uev(&bs, offset_for_top_to_bottom_field)) != ERROR_SUCCESS) {
            return ret;
        }
        
        int32_t num_ref_frames_in_pic_order_cnt_cycle = -1;
        if ((ret = brs_avc_nalu_read_uev(&bs, num_ref_frames_in_pic_order_cnt_cycle)) != ERROR_SUCCESS) {
            return ret;
        }
        if (num_ref_frames_in_pic_order_cnt_cycle) {
            ret = ERROR_HLS_DECODE_ERROR;
            brs_error("sps the num_ref_frames_in_pic_order_cnt_cycle invalid. ret=%d", ret);
            return ret;
        }
    }
    
    int32_t max_num_ref_frames = -1;
    if ((ret = brs_avc_nalu_read_uev(&bs, max_num_ref_frames)) != ERROR_SUCCESS) {
        return ret;
    }
    
    int8_t gaps_in_frame_num_value_allowed_flag = -1;
    if ((ret = brs_avc_nalu_read_bit(&bs, gaps_in_frame_num_value_allowed_flag)) != ERROR_SUCCESS) {
        return ret;
    }
    
    int32_t pic_width_in_mbs_minus1 = -1;
    if ((ret = brs_avc_nalu_read_uev(&bs, pic_width_in_mbs_minus1)) != ERROR_SUCCESS) {
        return ret;
    }
    
    int32_t pic_height_in_map_units_minus1 = -1;
    if ((ret = brs_avc_nalu_read_uev(&bs, pic_height_in_map_units_minus1)) != ERROR_SUCCESS) {
        return ret;
    }
    
    width = (int)(pic_width_in_mbs_minus1 + 1) * 16;
    height = (int)(pic_height_in_map_units_minus1 + 1) * 16;
    
    return ret;
}

int BrsAvcAacCodec::avc_demux_annexb_format(BRSStream* stream, BrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;
    
    // not annexb, try others
    if (!brs_avc_startswith_annexb(stream, NULL)) {
        return ERROR_HLS_AVC_TRY_OTHERS;
    }
    
    // AnnexB
    // B.1.1 Byte stream NAL unit syntax,
    // H.264-AVC-ISO_IEC_14496-10.pdf, page 211.
    while (!stream->empty()) {
        // find start code
        int nb_start_code = 0;
        if (!brs_avc_startswith_annexb(stream, &nb_start_code)) {
            return ret;
        }

        // skip the start code.
        if (nb_start_code > 0) {
            stream->skip(nb_start_code);
        }
        
        // the NALU start bytes.
        char* p = stream->data() + stream->pos();
        
        // get the last matched NALU
        while (!stream->empty()) {
            if (brs_avc_startswith_annexb(stream, NULL)) {
                break;
            }
            
            stream->skip(1);
        }
        
        char* pp = stream->data() + stream->pos();
        
        // skip the empty.
        if (pp - p <= 0) {
            continue;
        }
        
        // got the NALU.
        if ((ret = sample->add_sample_unit(p, pp - p)) != ERROR_SUCCESS) {
            brs_error("annexb add video sample failed. ret=%d", ret);
            return ret;
        }
    }
    
    return ret;
}

int BrsAvcAacCodec::avc_demux_ibmf_format(BRSStream* stream, BrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;
    
    int PictureLength = stream->size() - stream->pos();
    
    // 5.3.4.2.1 Syntax, H.264-AVC-ISO_IEC_14496-15.pdf, page 16
    // 5.2.4.1 AVC decoder configuration record
    // 5.2.4.1.2 Semantics
    // The value of this field shall be one of 0, 1, or 3 corresponding to a
    // length encoded with 1, 2, or 4 bytes, respectively.
    assert(NAL_unit_length != 2);
    
    // 5.3.4.2.1 Syntax, H.264-AVC-ISO_IEC_14496-15.pdf, page 20
    for (int i = 0; i < PictureLength;) {
        // unsigned int((NAL_unit_length+1)*8) NALUnitLength;
        if (!stream->require(NAL_unit_length + 1)) {
            ret = ERROR_HLS_DECODE_ERROR;
            brs_error("avc decode NALU size failed. ret=%d", ret);
            return ret;
        }
        int32_t NALUnitLength = 0;
        if (NAL_unit_length == 3) {
            NALUnitLength = stream->read_4bytes();
        } else if (NAL_unit_length == 1) {
            NALUnitLength = stream->read_2bytes();
        } else {
            NALUnitLength = stream->read_1bytes();
        }
        
        // maybe stream is invalid format.
        // see: https://github.com/osbrs/brs/issues/183
        if (NALUnitLength < 0) {
            ret = ERROR_HLS_DECODE_ERROR;
            brs_error("maybe stream is AnnexB format. ret=%d", ret);
            return ret;
        }
        
        // NALUnit
        if (!stream->require(NALUnitLength)) {
            ret = ERROR_HLS_DECODE_ERROR;
            brs_error("avc decode NALU data failed. ret=%d", ret);
            return ret;
        }
        // 7.3.1 NAL unit syntax, H.264-AVC-ISO_IEC_14496-10.pdf, page 44.
        if ((ret = sample->add_sample_unit(stream->data() + stream->pos(), NALUnitLength)) != ERROR_SUCCESS) {
            brs_error("avc add video sample failed. ret=%d", ret);
            return ret;
        }
        stream->skip(NALUnitLength);
        
        i += NAL_unit_length + 1 + NALUnitLength;
    }
    
    return ret;
}
#endif
}
