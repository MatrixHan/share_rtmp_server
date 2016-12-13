#include "BRSStream.h"

namespace BRS 
{
  

using namespace std;


BRSStream::BRSStream()
{
    p = bytes = NULL;
    nb_bytes = 0;
    
}

BRSStream::~BRSStream()
{
}

int BRSStream::initialize(char* b, int nb)
{
    int ret = 0;
    
    if (!b) {
        ret = -1;
       
        return ret;
    }
    
    if (nb <= 0) {
        ret = -2;
       
        return ret;
    }

    nb_bytes = nb;
    p = bytes = b;

    return ret;
}

char* BRSStream::data()
{
    return bytes;
}

int BRSStream::size()
{
    return nb_bytes;
}

int BRSStream::pos()
{
    return (int)(p - bytes);
}

bool BRSStream::empty()
{
    return !bytes || (p >= bytes + nb_bytes);
}

bool BRSStream::require(int required_size)
{
    
    return required_size <= nb_bytes - (p - bytes);
}

void BRSStream::skip(int size)
{
    
    p += size;
}

int8_t BRSStream::read_1bytes()
{
   
    
    return (int8_t)*p++;
}

int16_t BRSStream::read_2bytes()
{
    
    int16_t value;
    char* pp = (char*)&value;
    pp[1] = *p++;
    pp[0] = *p++;
    
    return value;
}

int32_t BRSStream::read_3bytes()
{
  
    
    int32_t value = 0x00;
    char* pp = (char*)&value;
    pp[2] = *p++;
    pp[1] = *p++;
    pp[0] = *p++;
    
    return value;
}

int32_t BRSStream::read_4bytes()
{
    assert(require(4));
    
    int32_t value;
    char* pp = (char*)&value;
    pp[3] = *p++;
    pp[2] = *p++;
    pp[1] = *p++;
    pp[0] = *p++;
    
    return value;
}

int64_t BRSStream::read_8bytes()
{
    assert(require(8));
    
    int64_t value;
    char* pp = (char*)&value;
    pp[7] = *p++;
    pp[6] = *p++;
    pp[5] = *p++;
    pp[4] = *p++;
    pp[3] = *p++;
    pp[2] = *p++;
    pp[1] = *p++;
    pp[0] = *p++;
    
    return value;
}

string BRSStream::read_string(int len)
{
    assert(require(len));
    
    std::string value;
    value.append(p, len);
    
    p += len;
    
    return value;
}

void BRSStream::read_bytes(char* data, int size)
{
    assert(require(size));
    
    memcpy(data, p, size);
    
    p += size;
}

void BRSStream::write_1bytes(int8_t value)
{
    assert(require(1));
    
    *p++ = value;
}

void BRSStream::write_2bytes(int16_t value)
{
    assert(require(2));
    
    char* pp = (char*)&value;
    *p++ = pp[1];
    *p++ = pp[0];
}

void BRSStream::write_4bytes(int32_t value)
{
    assert(require(4));
    
    char* pp = (char*)&value;
    *p++ = pp[3];
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];
}

void BRSStream::write_3bytes(int32_t value)
{
    assert(require(3));
    
    char* pp = (char*)&value;
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];
}

void BRSStream::write_8bytes(int64_t value)
{
    assert(require(8));
    
    char* pp = (char*)&value;
    *p++ = pp[7];
    *p++ = pp[6];
    *p++ = pp[5];
    *p++ = pp[4];
    *p++ = pp[3];
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];
}

void BRSStream::write_string(string value)
{
    assert(require((int)value.length()));
    
    memcpy(p, value.data(), value.length());
    p += value.length();
}

void BRSStream::write_bytes(char* data, int size)
{
    assert(require(size));
    
    memcpy(p, data, size);
    p += size;
}

BRSBitStream::BRSBitStream()
{
    cb = 0;
    cb_left = 0;
    stream = NULL;
}

BRSBitStream::~BRSBitStream()
{
}

int BRSBitStream::initialize(BRSStream* s) {
    stream = s;
    return 0;
}

bool BRSBitStream::empty() {
    if (cb_left) {
        return false;
    }
    return stream->empty();
}

int8_t BRSBitStream::read_bit() {
    if (!cb_left) {
        assert(!stream->empty());
        cb = stream->read_1bytes();
        cb_left = 8;
    }
    
    int8_t v = (cb >> (cb_left - 1)) & 0x01;
    cb_left--;
    return v;
}


}