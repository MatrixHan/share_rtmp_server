#pragma once 


#include <BRSCommon.h>

namespace BRS 
{
/**
* how many msgs can be send entirely.
* for play clients to get msgs then totally send out.
* for the mw sleep set to 1800, the msgs is about 133.
* @remark, recomment to 128.
*/
#define BRS_PERF_MW_MSGS 128
  
//#undef BRS_PERF_COMPLEX_SEND
#define BRS_PERF_COMPLEX_SEND 
/**
* how many chunk stream to cache, [0, N].
* to imporove about 10% performance when chunk size small, and 5% for large chunk.
* @see https://github.com/ossrs/srs/issues/249
* @remark 0 to disable the chunk stream cache.
*/
#define BRS_PERF_CHUNK_STREAM_CACHE 16

/**
* the gop cache and play cache queue.
*/
// whether gop cache is on.
#define BRS_PERF_GOP_CACHE true
// in seconds, the live queue length.
#define BRS_PERF_PLAY_QUEUE 30
}