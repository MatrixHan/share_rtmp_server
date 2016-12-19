#include <BRSFile.h>

// for brs-librtmp, @see https://github.com/osbrs/brs/issues/213
#ifndef _WIN32
#include <unistd.h>
#include <sys/uio.h>
#endif

#include <fcntl.h>
#include <sstream>
using namespace std;

#include <BRSLog.h>
#include <BRSKernelError.h>

namespace BRS {

BrsFileWriter::BrsFileWriter()
{
    fd = -1;
}

BrsFileWriter::~BrsFileWriter()
{
    close();
}

int BrsFileWriter::open(string p)
{
    int ret = ERROR_SUCCESS;
    
    if (fd > 0) {
        ret = ERROR_SYSTEM_FILE_ALREADY_OPENED;
        brs_error("file %s already opened. ret=%d", path.c_str(), ret);
        return ret;
    }
    
    int flags = O_CREAT|O_WRONLY|O_TRUNC;
    mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;

    if ((fd = ::open(p.c_str(), flags, mode)) < 0) {
        ret = ERROR_SYSTEM_FILE_OPENE;
        brs_error("open file %s failed. ret=%d", p.c_str(), ret);
        return ret;
    }
    
    path = p;
    
    return ret;
}

int BrsFileWriter::open_append(string p)
{
    int ret = ERROR_SUCCESS;
    
    if (fd > 0) {
        ret = ERROR_SYSTEM_FILE_ALREADY_OPENED;
        brs_error("file %s already opened. ret=%d", path.c_str(), ret);
        return ret;
    }
    
    int flags = O_APPEND|O_WRONLY;
    mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;

    if ((fd = ::open(p.c_str(), flags, mode)) < 0) {
        ret = ERROR_SYSTEM_FILE_OPENE;
        brs_error("open file %s failed. ret=%d", p.c_str(), ret);
        return ret;
    }
    
    path = p;
    
    return ret;
}

void BrsFileWriter::close()
{
    int ret = ERROR_SUCCESS;
    
    if (fd < 0) {
        return;
    }
    
    if (::close(fd) < 0) {
        ret = ERROR_SYSTEM_FILE_CLOSE;
        brs_error("close file %s failed. ret=%d", path.c_str(), ret);
        return;
    }
    fd = -1;
    
    return;
}

bool BrsFileWriter::is_open()
{
    return fd > 0;
}

void BrsFileWriter::lseek(int64_t offset)
{
    ::lseek(fd, (off_t)offset, SEEK_SET);
}

int64_t BrsFileWriter::tellg()
{
    return (int64_t)::lseek(fd, 0, SEEK_CUR);
}

int BrsFileWriter::write(void* buf, size_t count, ssize_t* pnwrite)
{
    int ret = ERROR_SUCCESS;
    
    ssize_t nwrite;
    // TODO: FIXME: use st_write.
    if ((nwrite = ::write(fd, buf, count)) < 0) {
        ret = ERROR_SYSTEM_FILE_WRITE;
        brs_error("write to file %s failed. ret=%d", path.c_str(), ret);
        return ret;
    }
    
    if (pnwrite != NULL) {
        *pnwrite = nwrite;
    }
    
    return ret;
}

int BrsFileWriter::writev(iovec* iov, int iovcnt, ssize_t* pnwrite)
{
    int ret = ERROR_SUCCESS;
    
    ssize_t nwrite = 0;
    for (int i = 0; i < iovcnt; i++) {
        iovec* piov = iov + i;
        ssize_t this_nwrite = 0;
        if ((ret = write(piov->iov_base, piov->iov_len, &this_nwrite)) != ERROR_SUCCESS) {
            return ret;
        }
        nwrite += this_nwrite;
    }
    
    if (pnwrite) {
        *pnwrite = nwrite;
    }
    
    return ret;
}

BrsFileReader::BrsFileReader()
{
    fd = -1;
}

BrsFileReader::~BrsFileReader()
{
    close();
}

int BrsFileReader::open(string p)
{
    int ret = ERROR_SUCCESS;
    
    if (fd > 0) {
        ret = ERROR_SYSTEM_FILE_ALREADY_OPENED;
        brs_error("file %s already opened. ret=%d", path.c_str(), ret);
        return ret;
    }

    if ((fd = ::open(p.c_str(), O_RDONLY)) < 0) {
        ret = ERROR_SYSTEM_FILE_OPENE;
        brs_error("open file %s failed. ret=%d", p.c_str(), ret);
        return ret;
    }
    
    path = p;
    
    return ret;
}

void BrsFileReader::close()
{
    int ret = ERROR_SUCCESS;
    
    if (fd < 0) {
        return;
    }
    
    if (::close(fd) < 0) {
        ret = ERROR_SYSTEM_FILE_CLOSE;
        brs_error("close file %s failed. ret=%d", path.c_str(), ret);
        return;
    }
    fd = -1;
    
    return;
}

bool BrsFileReader::is_open()
{
    return fd > 0;
}

int64_t BrsFileReader::tellg()
{
    return (int64_t)::lseek(fd, 0, SEEK_CUR);
}

void BrsFileReader::skip(int64_t size)
{
    ::lseek(fd, (off_t)size, SEEK_CUR);
}

int64_t BrsFileReader::lseek(int64_t offset)
{
    return (int64_t)::lseek(fd, (off_t)offset, SEEK_SET);
}

int64_t BrsFileReader::filesize()
{
    int64_t cur = tellg();
    int64_t size = (int64_t)::lseek(fd, 0, SEEK_END);
    ::lseek(fd, (off_t)cur, SEEK_SET);
    return size;
}

int BrsFileReader::read(void* buf, size_t count, ssize_t* pnread)
{
    int ret = ERROR_SUCCESS;
    
    ssize_t nread;
    // TODO: FIXME: use st_read.
    if ((nread = ::read(fd, buf, count)) < 0) {
        ret = ERROR_SYSTEM_FILE_READ;
        brs_error("read from file %s failed. ret=%d", path.c_str(), ret);
        return ret;
    }
    
    if (nread == 0) {
        ret = ERROR_SYSTEM_FILE_EOF;
        return ret;
    }
    
    if (pnread != NULL) {
        *pnread = nread;
    }
    
    return ret;
}

}