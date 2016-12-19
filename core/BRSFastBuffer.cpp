#include <BRSFastBuffer.h>
#include <BRSReadWriter.h>
#include <BRSLog.h>
#include <BRSMath.h>
#include <BRSErrorDef.h>
namespace BRS 
{
  // the default recv buffer size, 128KB.
#define BRS_DEFAULT_RECV_BUFFER_SIZE 131072

// limit user-space buffer to 256KB, for 3Mbps stream delivery.
//      800*2000/8=200000B(about 195KB).
// @remark it's ok for higher stream, the buffer is ok for one chunk is 256KB.
#define BRS_MAX_SOCKET_BUFFER 262144

// the max header size,
// @see SrsProtocol::read_message_header().
#define BRS_RTMP_MAX_MESSAGE_HEADER 11
  
#define ERROR_READER_BUFFER_OVERFLOW        1022
  
  
IMergeReadHandler::IMergeReadHandler()
{
}

IMergeReadHandler::~IMergeReadHandler()
{
}


BrsFastBuffer::BrsFastBuffer()
{
    merged_read = false;
    _handler  = NULL;
    
    nb_buffer = BRS_DEFAULT_RECV_BUFFER_SIZE;
    buffer = (char *)malloc(nb_buffer);
    p = end = buffer;
}

BrsFastBuffer::~BrsFastBuffer()
{
    free(buffer);
    buffer = NULL;
}

char* BrsFastBuffer::bytes()
{
    return p;
}

int BrsFastBuffer::size()
{	
    return (int)(end - p);
}


void BrsFastBuffer::set_buffer(int buffer_size)
{
    if(buffer_size > BRS_MAX_SOCKET_BUFFER)
    {
      brs_warn("limit the use-space buffer from %d to %d",buffer_size ,BRS_MAX_SOCKET_BUFFER);
    }
    
    int nb_resize_buf = Min(buffer_size,BRS_MAX_SOCKET_BUFFER);
    if(nb_resize_buf < nb_buffer)
      return ;
    int start = (int)(p - buffer);
    int nb_bytes = (int)(end -p);
    buffer = (char *)realloc(buffer,nb_resize_buf);
    nb_buffer = nb_resize_buf;
    p = buffer + start;
    end = p + nb_bytes;
}

char* BrsFastBuffer::read_slice(int size)
{
    assert(size>=0);
    assert(end-p>=size);
    assert(p+size>=buffer);
    char *ptr = p;
    p+= size;
    
    return ptr;
}


char BrsFastBuffer::read_1byte()
{
    assert(end - p >= 1);
    return *p++;
}


void BrsFastBuffer::skip(int size)
{
    assert(end-p >= size);
    assert(p+size>=buffer);
    p+=size;
}


int BrsFastBuffer::grow(BRSReadWriter* skt, int required_size)
{
    int ret = ERROR_SUCCESS;

    // already got required size of bytes.
    if (end - p >= required_size) {
        return ret;
    }

    // must be positive.
    assert(required_size > 0);

    // the free space of buffer, 
    //      buffer = consumed_bytes + exists_bytes + free_space.
    int nb_free_space = (int)(buffer + nb_buffer - end);
    // resize the space when no left space.
    if (nb_free_space < required_size) {
        // the bytes already in buffer
        int nb_exists_bytes = (int)(end - p);
        assert(nb_exists_bytes >= 0);
        brs_verbose("move fast buffer %d bytes", nb_exists_bytes);

        // reset or move to get more space.
        if (!nb_exists_bytes) {
            // reset when buffer is empty.
            p = end = buffer;
            brs_verbose("all consumed, reset fast buffer");
        } else {
            // move the left bytes to start of buffer.
            assert(nb_exists_bytes < nb_buffer);
            buffer = (char*)memmove(buffer, p, nb_exists_bytes);
            p = buffer;
            end = p + nb_exists_bytes;
        }
        
        // check whether enough free space in buffer.
        nb_free_space = (int)(buffer + nb_buffer - end);
        if (nb_free_space < required_size) {
            ret = ERROR_READER_BUFFER_OVERFLOW;
            brs_error("buffer overflow, required=%d, max=%d, left=%d, ret=%d", 
                required_size, nb_buffer, nb_free_space, ret);
            return ret;
        }
    }

    // buffer is ok, read required size of bytes.
    while (end - p < required_size) {
        ssize_t nread;
        if ((ret = skt->readt(end, nb_free_space, &nread)) != ERROR_SUCCESS) {
            return ret;
        }
        

        if (merged_read && _handler) {
            _handler->on_read(nread);
        }

        
        // we just move the ptr to next.
        assert((int)nread > 0);
        end += nread;
        nb_free_space -= nread;
    }
    
    return ret;
    
    
}

void BrsFastBuffer::set_merge_read(bool v, IMergeReadHandler* handler)
{
    merged_read = v;
    _handler = handler;
}


}