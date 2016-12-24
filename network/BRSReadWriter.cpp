#include <BRSReadWriter.h>
#include <BRSErrorDef.h>

namespace BRS {

  
BRSReadWriter::BRSReadWriter(int client_fd)
{
	sfd = client_fd;
	send_timeout = recv_timeout = SOCKET_TIMEOUT;
	recv_bytes = send_bytes = 0;
	start_time_ms = getCurrentTime();
}
BRSReadWriter::~BRSReadWriter()
{

}

  

void BRSReadWriter::set_recv_timeout(int64_t timeout_us)
{
	recv_timeout = timeout_us;
}

int64_t BRSReadWriter::get_recv_timeout()
{
	return recv_timeout;
}

void BRSReadWriter::set_send_timeout(int64_t timeout_us)
{
	send_timeout = timeout_us;
}

int64_t BRSReadWriter::get_recv_bytes()
{
	return recv_bytes;
}

int64_t BRSReadWriter::get_send_bytes()
{
	return send_bytes;
}

int BRSReadWriter::get_recv_kbps()
{
	int64_t diff_ms = getCurrentTime() - start_time_ms;
	
	if (diff_ms <= 0) {
		return 0;
	}
	
	return recv_bytes * 8 / diff_ms;
}

int BRSReadWriter::get_send_kbps()
{
	int64_t diff_ms = getCurrentTime() - start_time_ms;
	
	if (diff_ms <= 0) {
		return 0;
	}
	
	return send_bytes * 8 / diff_ms;
}

int64_t BRSReadWriter::get_send_timeout()
{
    return send_timeout;
}

int BRSReadWriter::readt(const void* buf, size_t size, ssize_t* nread)
{
    int ret = 0;
    
    *nread = read(sfd, (void*)buf, size);
    
    //On success a non-negative integer indicating the number of bytes actually read is returned 
    //(a value of 0 means the network connection is closed or end of file is reached).
    if (*nread <= 0) {
		if (errno == ETIME) {
			return -1;
		}
		
        if (*nread == 0) {
            errno = ECONNRESET;
        }
        
        return -2;
    }
    
    recv_bytes += *nread;
        
    return ret;
}



int BRSReadWriter::writet(const void* buf, size_t size, ssize_t* nwrite)
{
    int ret = 0;
    
    *nwrite = write(sfd, (void*)buf, size);
    
    if (*nwrite <= 0) {
		if (errno == ETIME) {
			return -1;
		}
		
        return -2;
    }
    
    send_bytes += *nwrite;
        
    return ret;
}

int BRSReadWriter::readn(const void* buf ,size_t count, ssize_t* nread)
{
    size_t nleft = count;
    void *bufp = (void *)buf;
    int  nnread = 0;
    while(nleft > 0)
    {
      if ((nnread = read(sfd ,bufp ,nleft))<0)
      {
	      if (errno == EINTR||errno == EAGAIN)
	      {
		  continue;
	      }else if (errno == ETIME) {
			return ERROR_SOCKET_TIMEOUT;
		}
	      else
	      {
		  return -1;
	      }
      }
      else if(nnread == 0)
      {
	  break;
      }
      bufp += nnread;
      nleft -=nnread;
      if(nread)
      *nread += nnread;
      recv_bytes += nnread;
    }
     return  0;
}
int BRSReadWriter::writen(const void* buf ,size_t count,ssize_t* nwrite)
{
    size_t nleft = count;
    ssize_t nwritten=0;
    void *bufp = (void *)buf;
    while(nleft > 0)
    {
      if ((nwritten = write(sfd ,bufp ,nleft))<0)
      {
	      if (errno == EINTR||errno == EAGAIN)
	      {
		  continue;
	      }else if (errno == ETIME) {
			return ERROR_SOCKET_TIMEOUT;
		}
	      else
	      {
		  return -1;
	      }
      }
      else if(nwritten == 0)
      {
	  continue;
      }
      bufp += nwritten;
      nleft -=nwritten;
      if(nwrite)
      *nwrite += nwritten;
      send_bytes += nwritten;
    }
     return  0;
  
}

int BRSReadWriter::writev(const iovec *iov, int iov_size, ssize_t* nwrite)
{
    int ret = ERROR_SUCCESS;
   
    ssize_t nb_write = ::writev(sfd, iov, iov_size);
    if (nwrite) {
        *nwrite = nb_write;
    }
    
    // On success a non-negative integer equal to nbyte is returned.
    // Otherwise, a value of -1 is returned and errno is set to indicate the error.
    if (nb_write <= 0) {
        // @see https://github.com/ossrs/srs/issues/200
        if (nb_write < 0 && errno == ETIME) {
            return ERROR_SOCKET_TIMEOUT;
        }
        
        return ERROR_SOCKET_WRITE;
    }
    
    send_bytes += nb_write;
    
    return ret;
}

}
