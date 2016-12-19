#include <BRSKernelError.h>

namespace BRS {

bool brs_is_system_control_error(int error_code)
{
    return error_code == ERROR_CONTROL_RTMP_CLOSE
        || error_code == ERROR_CONTROL_REPUBLISH;
}

bool brs_is_client_gracefully_close(int error_code)
{
    return error_code == ERROR_SOCKET_READ
        || error_code == ERROR_SOCKET_READ_FULLY
        || error_code == ERROR_SOCKET_WRITE
        || error_code == ERROR_SOCKET_TIMEOUT;
}

}