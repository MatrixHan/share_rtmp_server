#include <BRSAppUtility.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <BRSLog.h>

using namespace std;

namespace BRS 
{
string brs_get_local_ip(int fd)
{
    std::string ip;

    // discovery client information
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    if (getsockname(fd, (sockaddr*)&addr, &addrlen) == -1) {
        return ip;
    }
    brs_verbose("get local ip success.");

    // ip v4 or v6
    char buf[INET6_ADDRSTRLEN];
    memset(buf, 0, sizeof(buf));

    if ((inet_ntop(addr.sin_family, &addr.sin_addr, buf, sizeof(buf))) == NULL) {
        return ip;
    }

    ip = buf;

    brs_verbose("get local ip of client ip=%s, fd=%d", buf, fd);

    return ip;
  
}


string brs_get_peer_ip(int fd)
{
    std::string ip;
    
    // discovery client information
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    if (getpeername(fd, (sockaddr*)&addr, &addrlen) == -1) {
        return ip;
    }
    brs_verbose("get peer name success.");

    // ip v4 or v6
    char buf[INET6_ADDRSTRLEN];
    memset(buf, 0, sizeof(buf));
    
    if ((inet_ntop(addr.sin_family, &addr.sin_addr, buf, sizeof(buf))) == NULL) {
        return ip;
    }
    brs_verbose("get peer ip of client ip=%s, fd=%d", buf, fd);
    
    ip = buf;
    
    brs_verbose("get peer ip success. ip=%s, fd=%d", ip.c_str(), fd);
    
    return ip;
}
}