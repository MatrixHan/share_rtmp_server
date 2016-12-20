#include <BRSDataSource.h>
#include <BRSKernelError.h>
namespace BRS 
{
  
int64_t brs_gvid = getpid();

int64_t brs_generate_id()
{
    return brs_gvid++;
}
  
BrsStatisticVhost::BrsStatisticVhost()
{
    id = brs_generate_id();
    
    //kbps = new SrsKbps();
    //kbps->set_io(NULL, NULL);
    
    nb_clients = 0;
    nb_streams = 0;
}

BrsStatisticVhost::~BrsStatisticVhost()
{
    //srs_freep(kbps);
}

int BrsStatisticVhost::dumps(std::stringstream& ss)
{
      int ret = ERROR_SUCCESS;
    
   
    
    return ret;
}

BrsStatisticStream::BrsStatisticStream()
{
    id = brs_generate_id();
    vhost = NULL;
    active = false;
    connection_cid = -1;
    
    has_video = false;
    vcodec = BrsCodecVideoReserved;
    avc_profile = BrsAvcProfileReserved;
    avc_level = BrsAvcLevelReserved;
    
    has_audio = false;
    acodec = BrsCodecAudioReserved1;
    asample_rate = BrsCodecAudioSampleRateReserved;
    asound_type = BrsCodecAudioSoundTypeReserved;
    aac_object = BrsAacObjectTypeReserved;
    
    //kbps = new SrsKbps();
    //kbps->set_io(NULL, NULL);
    
    nb_clients = 0;
}

BrsStatisticStream::~BrsStatisticStream()
{

}

void BrsStatisticStream::publish(int cid)
{
    connection_cid = cid;
    active = true;
    
    vhost->nb_streams++;
}

int BrsStatisticStream::dumps(std::stringstream& ss)
{

}

void BrsStatisticStream::close()
{
     has_video = false;
    has_audio = false;
    active = false;
    
    vhost->nb_streams--;
}



}