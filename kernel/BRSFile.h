
#include <string>

// for srs-librtmp, @see https://github.com/ossrs/srs/issues/213
#ifndef _WIN32
#include <sys/uio.h>
#endif

namespace BRS
{
/**
* file writer, to write to file.
*/
class BrsFileWriter
{
private:
    std::string path;
    int fd;
public:
    BrsFileWriter();
    virtual ~BrsFileWriter();
public:
    /**
     * open file writer, in truncate mode.
     * @param p a string indicates the path of file to open.
     */
    virtual int open(std::string p);
    /**
     * open file writer, in append mode.
     * @param p a string indicates the path of file to open.
     */
    virtual int open_append(std::string p);
    /**
     * close current writer.
     * @remark user can reopen again.
     */
    virtual void close();
public:
    virtual bool is_open();
    virtual void lseek(int64_t offset);
    virtual int64_t tellg();
public:
    /**
    * write to file. 
    * @param pnwrite the output nb_write, NULL to ignore.
    */
    virtual int write(void* buf, size_t count, ssize_t* pnwrite);
    /**
     * for the HTTP FLV, to writev to improve performance.
     * @see https://github.com/ossrs/srs/issues/405
     */
    virtual int writev(iovec* iov, int iovcnt, ssize_t* pnwrite);
};

/**
* file reader, to read from file.
*/
class BrsFileReader
{
private:
    std::string path;
    int fd;
public:
    BrsFileReader();
    virtual ~BrsFileReader();
public:
    /**
     * open file reader.
     * @param p a string indicates the path of file to open.
     */
    virtual int open(std::string p);
    /**
     * close current reader.
     * @remark user can reopen again.
     */
    virtual void close();
public:
    // TODO: FIXME: extract interface.
    virtual bool is_open();
    virtual int64_t tellg();
    virtual void skip(int64_t size);
    virtual int64_t lseek(int64_t offset);
    virtual int64_t filesize();
public:
    /**
    * read from file. 
    * @param pnread the output nb_read, NULL to ignore.
    */
    virtual int read(void* buf, size_t count, ssize_t* pnread);
};


}
