#pragma once 

#include "../common/BRSCommon.h"
#include "../core/BRSUtils.h"
#include <errno.h>

#define SOCKET_TIMEOUT 30

namespace BRS 
{
  
class BRSReadWriter
{
private:
	int64_t 	recv_timeout;
	int64_t 	send_timeout;
	int64_t 	recv_bytes;
	int64_t 	send_bytes;
	int64_t 	start_time_ms;
	int	 	sfd;
public:

  BRSReadWriter(int client_fd);
  ~BRSReadWriter();
  
  virtual void 		set_recv_timeout(int64_t timeout_us);
  virtual int64_t 	get_recv_timeout();
  virtual void 		set_send_timeout(int64_t timeout_us);
  virtual int64_t 	get_recv_bytes();
  virtual int64_t 	get_send_bytes();
  virtual int 		get_recv_kbps();
  virtual int 		get_send_kbps();

  virtual   int	 	readt(const void* buf, size_t size, ssize_t* nread);
  virtual   int 	readn(const void *buf ,size_t count,ssize_t* nread);
  virtual   int 	writet(const void* buf, size_t size, ssize_t* nwrite);
  virtual   int	 	writen(const void *buf ,size_t count, ssize_t* nwrite);
  
};
  
}