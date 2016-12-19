#pragma once


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// RTMP consts values
///////////////////////////////////////////////////////////
// default vhost of rtmp
#define BRS_CONSTS_RTMP_DEFAULT_VHOST "__defaultVhost__"
// default port of rtmp
#define BRS_CONSTS_RTMP_DEFAULT_PORT "1935"

// the default chunk size for system.
#define BRS_CONSTS_RTMP_BRS_CHUNK_SIZE 60000
// 6. Chunking, RTMP protocol default chunk size.
#define BRS_CONSTS_RTMP_PROTOCOL_CHUNK_SIZE 128

/**
* 6. Chunking
* The chunk size is configurable. It can be set using a control
* message(Set Chunk Size) as described in section 7.1. The maximum
* chunk size can be 65536 bytes and minimum 128 bytes. Larger values
* reduce CPU usage, but also commit to larger writes that can delay
* other content on lower bandwidth connections. Smaller chunks are not
* good for high-bit rate streaming. Chunk size is maintained
* independently for each direction.
*/
#define BRS_CONSTS_RTMP_MIN_CHUNK_SIZE 128
#define BRS_CONSTS_RTMP_MAX_CHUNK_SIZE 65536

 
// the following is the timeout for rtmp protocol, 
// to avoid death connection.

// the timeout to send data to client,
// if timeout, close the connection.
#define BRS_CONSTS_RTMP_SEND_TIMEOUT_US (int64_t)(30*1000*1000LL)

// the timeout to wait client data,
// if timeout, close the connection.
#define BRS_CONSTS_RTMP_RECV_TIMEOUT_US (int64_t)(30*1000*1000LL)

// the timeout to wait for client control message,
// if timeout, we generally ignore and send the data to client,
// generally, it's the pulse time for data seding.
// @remark, recomment to 500ms.
#define BRS_CONSTS_RTMP_PULSE_TIMEOUT_US (int64_t)(500*1000LL)

/**
* max rtmp header size:
*     1bytes basic header,
*     11bytes message header,
*     4bytes timestamp header,
* that is, 1+11+4=16bytes.
*/
#define BRS_CONSTS_RTMP_MAX_FMT0_HEADER_SIZE 16
/**
* max rtmp header size:
*     1bytes basic header,
*     4bytes timestamp header,
* that is, 1+4=5bytes.
*/
// always use fmt0 as cache.
#define BRS_CONSTS_RTMP_MAX_FMT3_HEADER_SIZE 5

/**
* for performance issue, 
* the iovs cache, @see https://github.com/ossrs/srs/issues/194
* iovs cache for multiple messages for each connections.
* suppose the chunk size is 64k, each message send in a chunk which needs only 2 iovec,
* so the iovs max should be (BRS_PERF_MW_MSGS * 2)
*
* @remark, BRS will realloc when the iovs not enough.
*/
#define BRS_CONSTS_IOVS_MAX (BRS_PERF_MW_MSGS * 2)
/**
* for performance issue, 
* the c0c3 cache, @see https://github.com/ossrs/srs/issues/194
* c0c3 cache for multiple messages for each connections.
* each c0 <= 16byes, suppose the chunk size is 64k,
* each message send in a chunk which needs only a c0 header,
* so the c0c3 cache should be (BRS_PERF_MW_MSGS * 16)
*
* @remark, BRS will try another loop when c0c3 cache dry, for we cannot realloc it.
*       so we use larger c0c3 cache, that is (BRS_PERF_MW_MSGS * 32)
*/
#define BRS_CONSTS_C0C3_HEADERS_MAX (BRS_PERF_MW_MSGS * 32)

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// BRS consts values
///////////////////////////////////////////////////////////
#define BRS_CONSTS_NULL_FILE "/dev/null"
#define BRS_CONSTS_LOCALHOST "127.0.0.1"

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// log consts values
///////////////////////////////////////////////////////////
// downloading speed-up, play to edge, ingest from origin
#define BRS_CONSTS_LOG_EDGE_PLAY "EIG"
// uploading speed-up, publish to edge, foward to origin
#define BRS_CONSTS_LOG_EDGE_PUBLISH "EFW"
// edge/origin forwarder.
#define BRS_CONSTS_LOG_FOWARDER "FWR"
// play stream on edge/origin.
#define BRS_CONSTS_LOG_PLAY "PLA"
// client publish to edge/origin
#define BRS_CONSTS_LOG_CLIENT_PUBLISH "CPB"
// web/flash publish to edge/origin
#define BRS_CONSTS_LOG_WEB_PUBLISH "WPB"
// ingester for edge(play)/origin
#define BRS_CONSTS_LOG_INGESTER "IGS"
// hls log id.
#define BRS_CONSTS_LOG_HLS "HLS"
// encoder log id.
#define BRS_CONSTS_LOG_ENCODER "ENC"
// http stream log id.
#define BRS_CONSTS_LOG_HTTP_STREAM "HTS"
// http stream cache log id.
#define BRS_CONSTS_LOG_HTTP_STREAM_CACHE "HTC"
// stream caster log id.
#define BRS_CONSTS_LOG_STREAM_CASTER "SCS"

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// RTMP consts values
///////////////////////////////////////////////////////////
#define BRS_CONSTS_RTMP_SET_DATAFRAME            "@setDataFrame"
#define BRS_CONSTS_RTMP_ON_METADATA              "onMetaData"

///////////////////////////////////////////////////////////
// HTTP/HLS consts values
///////////////////////////////////////////////////////////
// @see hls-m3u8-draft-pantos-http-live-streaming-12.pdf, page 4
// Lines are terminated by either a single LF character or a CR
// character followed by an LF character.
// CR             = <US-ASCII CR, carriage return (13)>
#define BRS_CONSTS_CR '\r' // 0x0D
// LF             = <US-ASCII LF, linefeed (10)>
#define BRS_CONSTS_LF '\n' // 0x0A

///////////////////////////////////////////////////////////
// HTTP consts values
///////////////////////////////////////////////////////////
// linux path seprator
#define BRS_CONSTS_HTTP_PATH_SEP '/'
// query string seprator
#define BRS_CONSTS_HTTP_QUERY_SEP '?'

// the default recv timeout.
#define BRS_HTTP_RECV_TIMEOUT_US 60 * 1000 * 1000

// 6.1.1 Status Code and Reason Phrase
#define BRS_CONSTS_HTTP_Continue                       100
#define BRS_CONSTS_HTTP_SwitchingProtocols             101
#define BRS_CONSTS_HTTP_OK                             200
#define BRS_CONSTS_HTTP_Created                        201
#define BRS_CONSTS_HTTP_Accepted                       202
#define BRS_CONSTS_HTTP_NonAuthoritativeInformation    203
#define BRS_CONSTS_HTTP_NoContent                      204
#define BRS_CONSTS_HTTP_ResetContent                   205
#define BRS_CONSTS_HTTP_PartialContent                 206
#define BRS_CONSTS_HTTP_MultipleChoices                300
#define BRS_CONSTS_HTTP_MovedPermanently               301
#define BRS_CONSTS_HTTP_Found                          302
#define BRS_CONSTS_HTTP_SeeOther                       303
#define BRS_CONSTS_HTTP_NotModified                    304
#define BRS_CONSTS_HTTP_UseProxy                       305
#define BRS_CONSTS_HTTP_TemporaryRedirect              307
#define BRS_CONSTS_HTTP_BadRequest                     400
#define BRS_CONSTS_HTTP_Unauthorized                   401
#define BRS_CONSTS_HTTP_PaymentRequired                402
#define BRS_CONSTS_HTTP_Forbidden                      403
#define BRS_CONSTS_HTTP_NotFound                       404
#define BRS_CONSTS_HTTP_MethodNotAllowed               405
#define BRS_CONSTS_HTTP_NotAcceptable                  406
#define BRS_CONSTS_HTTP_ProxyAuthenticationRequired    407
#define BRS_CONSTS_HTTP_RequestTimeout                 408
#define BRS_CONSTS_HTTP_Conflict                       409
#define BRS_CONSTS_HTTP_Gone                           410
#define BRS_CONSTS_HTTP_LengthRequired                 411
#define BRS_CONSTS_HTTP_PreconditionFailed             412
#define BRS_CONSTS_HTTP_RequestEntityTooLarge          413
#define BRS_CONSTS_HTTP_RequestURITooLarge             414
#define BRS_CONSTS_HTTP_UnsupportedMediaType           415
#define BRS_CONSTS_HTTP_RequestedRangeNotSatisfiable   416
#define BRS_CONSTS_HTTP_ExpectationFailed              417
#define BRS_CONSTS_HTTP_InternalServerError            500
#define BRS_CONSTS_HTTP_NotImplemented                 501
#define BRS_CONSTS_HTTP_BadGateway                     502
#define BRS_CONSTS_HTTP_ServiceUnavailable             503
#define BRS_CONSTS_HTTP_GatewayTimeout                 504
#define BRS_CONSTS_HTTP_HTTPVersionNotSupported        505

#define BRS_CONSTS_HTTP_Continue_str                           "Continue"
#define BRS_CONSTS_HTTP_SwitchingProtocols_str                 "Switching Protocols"
#define BRS_CONSTS_HTTP_OK_str                                 "OK"
#define BRS_CONSTS_HTTP_Created_str                            "Created"
#define BRS_CONSTS_HTTP_Accepted_str                           "Accepted"
#define BRS_CONSTS_HTTP_NonAuthoritativeInformation_str        "Non Authoritative Information"
#define BRS_CONSTS_HTTP_NoContent_str                          "No Content"
#define BRS_CONSTS_HTTP_ResetContent_str                       "Reset Content"
#define BRS_CONSTS_HTTP_PartialContent_str                     "Partial Content"
#define BRS_CONSTS_HTTP_MultipleChoices_str                    "Multiple Choices"
#define BRS_CONSTS_HTTP_MovedPermanently_str                   "Moved Permanently"
#define BRS_CONSTS_HTTP_Found_str                              "Found"
#define BRS_CONSTS_HTTP_SeeOther_str                           "See Other"
#define BRS_CONSTS_HTTP_NotModified_str                        "Not Modified"
#define BRS_CONSTS_HTTP_UseProxy_str                           "Use Proxy"
#define BRS_CONSTS_HTTP_TemporaryRedirect_str                  "Temporary Redirect"
#define BRS_CONSTS_HTTP_BadRequest_str                         "Bad Request"
#define BRS_CONSTS_HTTP_Unauthorized_str                       "Unauthorized"
#define BRS_CONSTS_HTTP_PaymentRequired_str                    "Payment Required"
#define BRS_CONSTS_HTTP_Forbidden_str                          "Forbidden"
#define BRS_CONSTS_HTTP_NotFound_str                           "Not Found"
#define BRS_CONSTS_HTTP_MethodNotAllowed_str                   "Method Not Allowed"
#define BRS_CONSTS_HTTP_NotAcceptable_str                      "Not Acceptable"
#define BRS_CONSTS_HTTP_ProxyAuthenticationRequired_str        "Proxy Authentication Required"
#define BRS_CONSTS_HTTP_RequestTimeout_str                     "Request Timeout"
#define BRS_CONSTS_HTTP_Conflict_str                           "Conflict"
#define BRS_CONSTS_HTTP_Gone_str                               "Gone"
#define BRS_CONSTS_HTTP_LengthRequired_str                     "Length Required"
#define BRS_CONSTS_HTTP_PreconditionFailed_str                 "Precondition Failed"
#define BRS_CONSTS_HTTP_RequestEntityTooLarge_str              "Request Entity Too Large"
#define BRS_CONSTS_HTTP_RequestURITooLarge_str                 "Request URI Too Large"
#define BRS_CONSTS_HTTP_UnsupportedMediaType_str               "Unsupported Media Type"
#define BRS_CONSTS_HTTP_RequestedRangeNotSatisfiable_str       "Requested Range Not Satisfiable"
#define BRS_CONSTS_HTTP_ExpectationFailed_str                  "Expectation Failed"
#define BRS_CONSTS_HTTP_InternalServerError_str                "Internal Server Error"
#define BRS_CONSTS_HTTP_NotImplemented_str                     "Not Implemented"
#define BRS_CONSTS_HTTP_BadGateway_str                         "Bad Gateway"
#define BRS_CONSTS_HTTP_ServiceUnavailable_str                 "Service Unavailable"
#define BRS_CONSTS_HTTP_GatewayTimeout_str                     "Gateway Timeout"
#define BRS_CONSTS_HTTP_HTTPVersionNotSupported_str            "HTTP Version Not Supported"

///////////////////////////////////////////////////////////
// RTSP consts values
///////////////////////////////////////////////////////////
// 7.1.1 Status Code and Reason Phrase
#define BRS_CONSTS_RTSP_Continue                       100
#define BRS_CONSTS_RTSP_OK                             200
#define BRS_CONSTS_RTSP_Created                        201
#define BRS_CONSTS_RTSP_LowOnStorageSpace              250
#define BRS_CONSTS_RTSP_MultipleChoices                300
#define BRS_CONSTS_RTSP_MovedPermanently               301
#define BRS_CONSTS_RTSP_MovedTemporarily               302
#define BRS_CONSTS_RTSP_SeeOther                       303
#define BRS_CONSTS_RTSP_NotModified                    304
#define BRS_CONSTS_RTSP_UseProxy                       305
#define BRS_CONSTS_RTSP_BadRequest                     400
#define BRS_CONSTS_RTSP_Unauthorized                   401
#define BRS_CONSTS_RTSP_PaymentRequired                402
#define BRS_CONSTS_RTSP_Forbidden                      403
#define BRS_CONSTS_RTSP_NotFound                       404
#define BRS_CONSTS_RTSP_MethodNotAllowed               405
#define BRS_CONSTS_RTSP_NotAcceptable                  406
#define BRS_CONSTS_RTSP_ProxyAuthenticationRequired    407
#define BRS_CONSTS_RTSP_RequestTimeout                 408
#define BRS_CONSTS_RTSP_Gone                           410
#define BRS_CONSTS_RTSP_LengthRequired                 411
#define BRS_CONSTS_RTSP_PreconditionFailed             412
#define BRS_CONSTS_RTSP_RequestEntityTooLarge          413
#define BRS_CONSTS_RTSP_RequestURITooLarge             414
#define BRS_CONSTS_RTSP_UnsupportedMediaType           415
#define BRS_CONSTS_RTSP_ParameterNotUnderstood         451
#define BRS_CONSTS_RTSP_ConferenceNotFound             452
#define BRS_CONSTS_RTSP_NotEnoughBandwidth             453
#define BRS_CONSTS_RTSP_SessionNotFound                454
#define BRS_CONSTS_RTSP_MethodNotValidInThisState      455
#define BRS_CONSTS_RTSP_HeaderFieldNotValidForResource 456
#define BRS_CONSTS_RTSP_InvalidRange                   457
#define BRS_CONSTS_RTSP_ParameterIsReadOnly            458
#define BRS_CONSTS_RTSP_AggregateOperationNotAllowed   459
#define BRS_CONSTS_RTSP_OnlyAggregateOperationAllowed  460
#define BRS_CONSTS_RTSP_UnsupportedTransport           461
#define BRS_CONSTS_RTSP_DestinationUnreachable         462
#define BRS_CONSTS_RTSP_InternalServerError            500
#define BRS_CONSTS_RTSP_NotImplemented                 501
#define BRS_CONSTS_RTSP_BadGateway                     502
#define BRS_CONSTS_RTSP_ServiceUnavailable             503
#define BRS_CONSTS_RTSP_GatewayTimeout                 504
#define BRS_CONSTS_RTSP_RTSPVersionNotSupported        505
#define BRS_CONSTS_RTSP_OptionNotSupported             551

#define BRS_CONSTS_RTSP_Continue_str                            "Continue"
#define BRS_CONSTS_RTSP_OK_str                                  "OK"
#define BRS_CONSTS_RTSP_Created_str                             "Created"
#define BRS_CONSTS_RTSP_LowOnStorageSpace_str                   "Low on Storage Space"
#define BRS_CONSTS_RTSP_MultipleChoices_str                     "Multiple Choices"
#define BRS_CONSTS_RTSP_MovedPermanently_str                    "Moved Permanently"
#define BRS_CONSTS_RTSP_MovedTemporarily_str                    "Moved Temporarily"
#define BRS_CONSTS_RTSP_SeeOther_str                            "See Other"
#define BRS_CONSTS_RTSP_NotModified_str                         "Not Modified"
#define BRS_CONSTS_RTSP_UseProxy_str                            "Use Proxy"
#define BRS_CONSTS_RTSP_BadRequest_str                          "Bad Request"
#define BRS_CONSTS_RTSP_Unauthorized_str                        "Unauthorized"
#define BRS_CONSTS_RTSP_PaymentRequired_str                     "Payment Required"
#define BRS_CONSTS_RTSP_Forbidden_str                           "Forbidden"
#define BRS_CONSTS_RTSP_NotFound_str                            "Not Found"
#define BRS_CONSTS_RTSP_MethodNotAllowed_str                    "Method Not Allowed"
#define BRS_CONSTS_RTSP_NotAcceptable_str                       "Not Acceptable"
#define BRS_CONSTS_RTSP_ProxyAuthenticationRequired_str         "Proxy Authentication Required"
#define BRS_CONSTS_RTSP_RequestTimeout_str                      "Request Timeout"
#define BRS_CONSTS_RTSP_Gone_str                                "Gone"
#define BRS_CONSTS_RTSP_LengthRequired_str                      "Length Required"
#define BRS_CONSTS_RTSP_PreconditionFailed_str                  "Precondition Failed"
#define BRS_CONSTS_RTSP_RequestEntityTooLarge_str               "Request Entity Too Large"
#define BRS_CONSTS_RTSP_RequestURITooLarge_str                  "Request URI Too Large"
#define BRS_CONSTS_RTSP_UnsupportedMediaType_str                "Unsupported Media Type"
#define BRS_CONSTS_RTSP_ParameterNotUnderstood_str              "Invalid parameter"
#define BRS_CONSTS_RTSP_ConferenceNotFound_str                  "Illegal Conference Identifier"
#define BRS_CONSTS_RTSP_NotEnoughBandwidth_str                  "Not Enough Bandwidth"
#define BRS_CONSTS_RTSP_SessionNotFound_str                     "Session Not Found"
#define BRS_CONSTS_RTSP_MethodNotValidInThisState_str           "Method Not Valid In This State"
#define BRS_CONSTS_RTSP_HeaderFieldNotValidForResource_str      "Header Field Not Valid"
#define BRS_CONSTS_RTSP_InvalidRange_str                        "Invalid Range"
#define BRS_CONSTS_RTSP_ParameterIsReadOnly_str                 "Parameter Is Read-Only"
#define BRS_CONSTS_RTSP_AggregateOperationNotAllowed_str        "Aggregate Operation Not Allowed"
#define BRS_CONSTS_RTSP_OnlyAggregateOperationAllowed_str       "Only Aggregate Operation Allowed"
#define BRS_CONSTS_RTSP_UnsupportedTransport_str                "Unsupported Transport"
#define BRS_CONSTS_RTSP_DestinationUnreachable_str              "Destination Unreachable"
#define BRS_CONSTS_RTSP_InternalServerError_str                 "Internal Server Error"
#define BRS_CONSTS_RTSP_NotImplemented_str                      "Not Implemented"
#define BRS_CONSTS_RTSP_BadGateway_str                          "Bad Gateway"
#define BRS_CONSTS_RTSP_ServiceUnavailable_str                  "Service Unavailable"
#define BRS_CONSTS_RTSP_GatewayTimeout_str                      "Gateway Timeout"
#define BRS_CONSTS_RTSP_RTSPVersionNotSupported_str             "RTSP Version Not Supported"
#define BRS_CONSTS_RTSP_OptionNotSupported_str                  "Option not support"



