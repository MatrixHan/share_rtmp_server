#pragma once

#include <BRSCommon.h>



#include <string>

class BRSStream;
class BRSBitStream;

// compare
#define brs_min(a, b) (((a) < (b))? (a) : (b))
#define brs_max(a, b) (((a) < (b))? (b) : (a))

namespace BRS {
// read nalu uev.
extern int brs_avc_nalu_read_uev(BRSBitStream* stream, int32_t& v);
extern int brs_avc_nalu_read_bit(BRSBitStream* stream, int8_t& v);

// get current system time in ms, use cache to avoid performance problem
extern int64_t brs_get_system_time_ms();
extern int64_t brs_get_system_startup_time_ms();
// the deamon st-thread will update it.
extern int64_t brs_update_system_time_ms();

// dns resolve utility, return the resolved ip address.
extern std::string brs_dns_resolve(std::string host);

// whether system is little endian
extern bool brs_is_little_endian();

// replace old_str to new_str of str
extern std::string brs_string_replace(std::string str, std::string old_str, std::string new_str);
// trim char in trim_chars of str
extern std::string brs_string_trim_end(std::string str, std::string trim_chars);
// trim char in trim_chars of str
extern std::string brs_string_trim_start(std::string str, std::string trim_chars);
// remove char in remove_chars of str
extern std::string brs_string_remove(std::string str, std::string remove_chars);
// whether string end with
extern bool brs_string_ends_with(std::string str, std::string flag);
// whether string starts with
extern bool brs_string_starts_with(std::string str, std::string flag);
extern bool brs_string_starts_with(std::string str, std::string flag0, std::string flag1);
// whether string contains with
extern bool brs_string_contains(std::string str, std::string flag);
extern bool brs_string_contains(std::string str, std::string flag0, std::string flag1);
extern bool brs_string_contains(std::string str, std::string flag0, std::string flag1, std::string flag2);

// create dir recursively
extern int brs_create_dir_recursively(std::string dir);

// whether path exists.
extern bool brs_path_exists(std::string path);
// get the dirname of path
extern std::string brs_path_dirname(std::string path);
// get the basename of path
extern std::string brs_path_basename(std::string path);

/**
* whether stream starts with the avc NALU in "AnnexB" 
* from H.264-AVC-ISO_IEC_14496-10.pdf, page 211.
* start code must be "N[00] 00 00 01" where N>=0
* @param pnb_start_code output the size of start code, must >=3. 
*       NULL to ignore.
*/
extern bool brs_avc_startswith_annexb(BRSStream* stream, int* pnb_start_code = NULL);

/**
* whether stream starts with the aac ADTS 
* from aac-mp4a-format-ISO_IEC_14496-3+2001.pdf, page 75, 1.A.2.2 ADTS.
* start code must be '1111 1111 1111'B, that is 0xFFF
*/
extern bool brs_aac_startswith_adts(BRSStream* stream);

/**
* cacl the crc32 of bytes in buf.
*/
extern u_int32_t brs_crc32(const void* buf, int size);

/**
* Decode a base64-encoded string.
*
* @param out      buffer for decoded data
* @param in       null-terminated input string
* @param out_size size in bytes of the out buffer, must be at
*                 least 3/4 of the length of in
* @return         number of bytes written, or a negative value in case of
*                 invalid input
*/
extern int brs_av_base64_decode(u_int8_t* out, const char* in, int out_size);

/**
* Encode data to base64 and null-terminate.
*
* @param out      buffer for encoded data
* @param out_size size in bytes of the out buffer (including the
*                 null terminator), must be at least AV_BASE64_SIZE(in_size)
* @param in       input buffer containing the data to encode
* @param in_size  size in bytes of the in buffer
* @return         out or NULL in case of error
*/
extern char* brs_av_base64_encode(char* out, int out_size, const u_int8_t* in, int in_size);

/**
 * Calculate the output size needed to base64-encode x bytes to a
 * null-terminated string.
 */
#define BRS_AV_BASE64_SIZE(x)  (((x)+2) / 3 * 4 + 1)

/**
* convert hex string to data.
* for example, p=config='139056E5A0'
* output hex to data={0x13, 0x90, 0x56, 0xe5, 0xa0}
*/
extern int ff_hex_to_data(u_int8_t* data, const char* p);

/**
 * generate the c0 chunk header for msg.
 * @param cache, the cache to write header.
 * @param nb_cache, the size of cache.
 * @return the size of header. 0 if cache not enough.
 */
extern int brs_chunk_header_c0(
    int perfer_cid, u_int32_t timestamp, int32_t payload_length,
    int8_t message_type, int32_t stream_id,
    char* cache, int nb_cache
    );

/**
 * generate the c3 chunk header for msg.
 * @param cache, the cache to write header.
 * @param nb_cache, the size of cache.
 * @return the size of header. 0 if cache not enough.
 */
extern int brs_chunk_header_c3(
    int perfer_cid, u_int32_t timestamp,
    char* cache, int nb_cache
    );

}