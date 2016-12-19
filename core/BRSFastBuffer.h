#pragma once 

#include <BRSCommon.h>

namespace BRS 
{
  class IMergeReadHandler
{
public:
    IMergeReadHandler();
    virtual ~IMergeReadHandler();
public:
    /**
    * when read from channel, notice the merge handler to sleep for
    * some small bytes.
    * @remark, it only for server-side, client srs-librtmp just ignore.
    */
    virtual void on_read(ssize_t nread) = 0;
};
  
  
  class BRSReadWriter;
  class BrsFastBuffer
  {
  private:
     bool merged_read;
    IMergeReadHandler* _handler;
  private :
    //buffer 
    char * buffer;
    //current point 
    char * p;
    //last point
    char * end;
    //count for buffer
    int    nb_buffer;
  public:
    BrsFastBuffer();
    virtual ~BrsFastBuffer();
  public:
    virtual int size();
    
    virtual char * bytes();
    
    virtual void set_buffer(int buffer_size);
    
  public:
    virtual char read_1byte();
    
    virtual char* read_slice(int size);
    
    virtual void skip(int size);
    
  public:
    virtual int grow(BRSReadWriter *skt,int size);
    
  public:
    virtual void set_merge_read(bool v , IMergeReadHandler *handler);
  };
}