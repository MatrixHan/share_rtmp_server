#pragma once 

#define ERROR_SUCCESS 				0

#define ERROR_ST_SET_EPOLL 			100
#define ERROR_ST_INITIALIZE 			101
#define ERROR_ST_OPEN_SOCKET			102
#define ERROR_ST_CREATE_LISTEN_THREAD		103
#define ERROR_ST_CREATE_CYCLE_THREAD		104

#define ERROR_SOCKET_CREATE 			200
#define ERROR_SOCKET_SETREUSE 			201
#define ERROR_SOCKET_BIND 			202
#define ERROR_SOCKET_LISTEN 			203
#define ERROR_SOCKET_CLOSED 			204
#define ERROR_SOCKET_GET_PEER_NAME		205
#define ERROR_SOCKET_GET_PEER_IP		206
#define ERROR_SOCKET_READ			207
#define ERROR_SOCKET_READ_FULLY			208
#define ERROR_SOCKET_WRITE			209
#define ERROR_SOCKET_WAIT			210
#define ERROR_SOCKET_TIMEOUT			211

#define ERROR_RTMP_PLAIN_REQUIRED		300
#define ERROR_RTMP_CHUNK_START			301
#define ERROR_RTMP_MSG_INVLIAD_SIZE		302
#define ERROR_RTMP_AMF0_DECODE			303
#define ERROR_RTMP_AMF0_INVALID			304
#define ERROR_RTMP_REQ_CONNECT			305
#define ERROR_RTMP_REQ_TCURL			306
#define ERROR_RTMP_MESSAGE_DECODE		307
#define ERROR_RTMP_MESSAGE_ENCODE		308
#define ERROR_RTMP_AMF0_ENCODE			309
#define ERROR_RTMP_CHUNK_SIZE			310
#define ERROR_RTMP_TRY_SIMPLE_HS		311
#define ERROR_RTMP_CH_SCHEMA			312
#define ERROR_RTMP_PACKET_SIZE			313

#define ERROR_SYSTEM_STREAM_INIT		400
#define ERROR_SYSTEM_PACKET_INVALID		401
#define ERROR_SYSTEM_CLIENT_INVALID		402
#define ERROR_SYSTEM_ASSERT_FAILED		403
#define ERROR_SYSTEM_SIZE_NEGATIVE		404

// see librtmp.
// failed when open ssl create the dh
#define ERROR_OpenSslCreateDH			500
// failed when open ssl create the Private key.
#define ERROR_OpenSslCreateP			501
// when open ssl create G.
#define ERROR_OpenSslCreateG			502
// when open ssl parse P1024
#define ERROR_OpenSslParseP1024			503
// when open ssl set G
#define ERROR_OpenSslSetG			504
// when open ssl generate DHKeys
#define ERROR_OpenSslGenerateDHKeys		505
// when open ssl share key already computed.
#define ERROR_OpenSslShareKeyComputed		506
// when open ssl get shared key size.
#define ERROR_OpenSslGetSharedKeySize		507
// when open ssl get peer public key.
#define ERROR_OpenSslGetPeerPublicKey		508
// when open ssl compute shared key.
#define ERROR_OpenSslComputeSharedKey		509
// when open ssl is invalid DH state.
#define ERROR_OpenSslInvalidDHState		510
// when open ssl copy key
#define ERROR_OpenSslCopyKey			511
// when open ssl sha256 digest key invalid size.
#define ERROR_OpenSslSha256DigestSize		512