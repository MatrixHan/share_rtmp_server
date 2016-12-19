#include <BRSRtmpAmf0.h>

#include <utility>
#include <vector>
#include <sstream>
using namespace std;

#include <BRSLog.h>
#include <BRSKernelError.h>
#include <BRSStream.h>


namespace BRS {
  
using namespace _brs_internal;
// AMF0 marker
#define RTMP_AMF0_Number                     0x00
#define RTMP_AMF0_Boolean                    0x01
#define RTMP_AMF0_String                     0x02
#define RTMP_AMF0_Object                     0x03
#define RTMP_AMF0_MovieClip                  0x04 // reserved, not supported
#define RTMP_AMF0_Null                       0x05
#define RTMP_AMF0_Undefined                  0x06
#define RTMP_AMF0_Reference                  0x07
#define RTMP_AMF0_EcmaArray                  0x08
#define RTMP_AMF0_ObjectEnd                  0x09
#define RTMP_AMF0_StrictArray                0x0A
#define RTMP_AMF0_Date                       0x0B
#define RTMP_AMF0_LongString                 0x0C
#define RTMP_AMF0_UnSupported                0x0D
#define RTMP_AMF0_RecordSet                  0x0E // reserved, not supported
#define RTMP_AMF0_XmlDocument                0x0F
#define RTMP_AMF0_TypedObject                0x10
// AVM+ object is the AMF3 object.
#define RTMP_AMF0_AVMplusObject              0x11
// origin array whos data takes the same form as LengthValueBytes
#define RTMP_AMF0_OriginStrictArray          0x20

// User defined
#define RTMP_AMF0_Invalid                    0x3F

BrsAmf0Any::BrsAmf0Any()
{
    marker = RTMP_AMF0_Invalid;
}

BrsAmf0Any::~BrsAmf0Any()
{
}

bool BrsAmf0Any::is_string()
{
    return marker == RTMP_AMF0_String;
}

bool BrsAmf0Any::is_boolean()
{
    return marker == RTMP_AMF0_Boolean;
}

bool BrsAmf0Any::is_number()
{
    return marker == RTMP_AMF0_Number;
}

bool BrsAmf0Any::is_null()
{
    return marker == RTMP_AMF0_Null;
}

bool BrsAmf0Any::is_undefined()
{
    return marker == RTMP_AMF0_Undefined;
}

bool BrsAmf0Any::is_object()
{
    return marker == RTMP_AMF0_Object;
}

bool BrsAmf0Any::is_ecma_array()
{
    return marker == RTMP_AMF0_EcmaArray;
}

bool BrsAmf0Any::is_strict_array()
{
    return marker == RTMP_AMF0_StrictArray;
}

bool BrsAmf0Any::is_date()
{
    return marker == RTMP_AMF0_Date;
}

bool BrsAmf0Any::is_complex_object()
{
    return is_object() || is_object_eof() || is_ecma_array() || is_strict_array();
}

string BrsAmf0Any::to_str()
{
    BrsAmf0String* p = dynamic_cast<BrsAmf0String*>(this);
    assert(p != NULL);
    return p->value;
}

const char* BrsAmf0Any::to_str_raw()
{
    BrsAmf0String* p = dynamic_cast<BrsAmf0String*>(this);
    assert(p != NULL);
    return p->value.data();
}

bool BrsAmf0Any::to_boolean()
{
    BrsAmf0Boolean* p = dynamic_cast<BrsAmf0Boolean*>(this);
    assert(p != NULL);
    return p->value;
}

double BrsAmf0Any::to_number()
{
    BrsAmf0Number* p = dynamic_cast<BrsAmf0Number*>(this);
    assert(p != NULL);
    return p->value;
}

int64_t BrsAmf0Any::to_date()
{
    BrsAmf0Date* p = dynamic_cast<BrsAmf0Date*>(this);
    assert(p != NULL);
    return p->date();
}

int16_t BrsAmf0Any::to_date_time_zone()
{
    BrsAmf0Date* p = dynamic_cast<BrsAmf0Date*>(this);
    assert(p != NULL);
    return p->time_zone();
}

BrsAmf0Object* BrsAmf0Any::to_object()
{
    BrsAmf0Object* p = dynamic_cast<BrsAmf0Object*>(this);
    assert(p != NULL);
    return p;
}

BrsAmf0EcmaArray* BrsAmf0Any::to_ecma_array()
{
    BrsAmf0EcmaArray* p = dynamic_cast<BrsAmf0EcmaArray*>(this);
    assert(p != NULL);
    return p;
}

BrsAmf0StrictArray* BrsAmf0Any::to_strict_array()
{
    BrsAmf0StrictArray* p = dynamic_cast<BrsAmf0StrictArray*>(this);
    assert(p != NULL);
    return p;
}

void BrsAmf0Any::set_number(double value)
{
    BrsAmf0Number* p = dynamic_cast<BrsAmf0Number*>(this);
    assert(p != NULL);
    p->value = value;
}

bool BrsAmf0Any::is_object_eof()
{
    return marker == RTMP_AMF0_ObjectEnd;
}

void brs_fill_level_spaces(stringstream& ss, int level)
{
    for (int i = 0; i < level; i++) {
        ss << "    ";
    }
}
void brs_amf0_do_print(BrsAmf0Any* any, stringstream& ss, int level)
{
    if (any->is_boolean()) {
        ss << "Boolean " << (any->to_boolean()? "true":"false") << endl;
    } else if (any->is_number()) {
        ss << "Number " << std::fixed << any->to_number() << endl;
    } else if (any->is_string()) {
        ss << "String " << any->to_str() << endl;
    } else if (any->is_date()) {
        ss << "Date " << std::hex << any->to_date() 
            << "/" << std::hex << any->to_date_time_zone() << endl;
    } else if (any->is_null()) {
        ss << "Null" << endl;
    } else if (any->is_ecma_array()) {
        BrsAmf0EcmaArray* obj = any->to_ecma_array();
        ss << "EcmaArray " << "(" << obj->count() << " items)" << endl;
        for (int i = 0; i < obj->count(); i++) {
            brs_fill_level_spaces(ss, level + 1);
            ss << "Elem '" << obj->key_at(i) << "' ";
            if (obj->value_at(i)->is_complex_object()) {
                brs_amf0_do_print(obj->value_at(i), ss, level + 1);
            } else {
                brs_amf0_do_print(obj->value_at(i), ss, 0);
            }
        }
    } else if (any->is_strict_array()) {
        BrsAmf0StrictArray* obj = any->to_strict_array();
        ss << "StrictArray " << "(" << obj->count() << " items)" << endl;
        for (int i = 0; i < obj->count(); i++) {
            brs_fill_level_spaces(ss, level + 1);
            ss << "Elem ";
            if (obj->at(i)->is_complex_object()) {
                brs_amf0_do_print(obj->at(i), ss, level + 1);
            } else {
                brs_amf0_do_print(obj->at(i), ss, 0);
            }
        }
    } else if (any->is_object()) {
        BrsAmf0Object* obj = any->to_object();
        ss << "Object " << "(" << obj->count() << " items)" << endl;
        for (int i = 0; i < obj->count(); i++) {
            brs_fill_level_spaces(ss, level + 1);
            ss << "Property '" << obj->key_at(i) << "' ";
            if (obj->value_at(i)->is_complex_object()) {
                brs_amf0_do_print(obj->value_at(i), ss, level + 1);
            } else {
                brs_amf0_do_print(obj->value_at(i), ss, 0);
            }
        }
    } else {
        ss << "Unknown" << endl;
    }
}

char* BrsAmf0Any::human_print(char** pdata, int* psize)
{
    stringstream ss;
    
    ss.precision(1);
    
    brs_amf0_do_print(this, ss, 0);
    
    string str = ss.str();
    if (str.empty()) {
        return NULL;
    }
    
    char* data = new char[str.length() + 1];
    memcpy(data, str.data(), str.length());
    data[str.length()] = 0;
    
    if (pdata) {
        *pdata = data;
    }
    if (psize) {
        *psize = str.length();
    }
    
    return data;
}

BrsAmf0Any* BrsAmf0Any::str(const char* value)
{
    return new BrsAmf0String(value);
}

BrsAmf0Any* BrsAmf0Any::boolean(bool value)
{
    return new BrsAmf0Boolean(value);
}

BrsAmf0Any* BrsAmf0Any::number(double value)
{
    return new BrsAmf0Number(value);
}

BrsAmf0Any* BrsAmf0Any::null()
{
    return new BrsAmf0Null();
}

BrsAmf0Any* BrsAmf0Any::undefined()
{
    return new BrsAmf0Undefined();
}

BrsAmf0Object* BrsAmf0Any::object()
{
    return new BrsAmf0Object();
}

BrsAmf0Any* BrsAmf0Any::object_eof()
{
    return new BrsAmf0ObjectEOF();
}

BrsAmf0EcmaArray* BrsAmf0Any::ecma_array()
{
    return new BrsAmf0EcmaArray();
}

BrsAmf0StrictArray* BrsAmf0Any::strict_array()
{
    return new BrsAmf0StrictArray();
}

BrsAmf0Any* BrsAmf0Any::date(int64_t value)
{
    return new BrsAmf0Date(value);
}

int BrsAmf0Any::discovery(BRSStream* stream, BrsAmf0Any** ppvalue)
{
    int ret = ERROR_SUCCESS;
    
    // detect the object-eof specially
    if (brs_amf0_is_object_eof(stream)) {
        *ppvalue = new BrsAmf0ObjectEOF();
        return ret;
    }
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read any marker failed. ret=%d", ret);
        return ret;
    }
    
    char marker = stream->read_1bytes();
    brs_verbose("amf0 any marker success");
    
    // backward the 1byte marker.
    stream->skip(-1);
    
    switch (marker) {
        case RTMP_AMF0_String: {
            *ppvalue = BrsAmf0Any::str();
            return ret;
        }
        case RTMP_AMF0_Boolean: {
            *ppvalue = BrsAmf0Any::boolean();
            return ret;
        }
        case RTMP_AMF0_Number: {
            *ppvalue = BrsAmf0Any::number();
            return ret;
        }
        case RTMP_AMF0_Null: {
            *ppvalue = BrsAmf0Any::null();
            return ret;
        }
        case RTMP_AMF0_Undefined: {
            *ppvalue = BrsAmf0Any::undefined();
            return ret;
        }
        case RTMP_AMF0_Object: {
            *ppvalue = BrsAmf0Any::object();
            return ret;
        }
        case RTMP_AMF0_EcmaArray: {
            *ppvalue = BrsAmf0Any::ecma_array();
            return ret;
        }
        case RTMP_AMF0_StrictArray: {
            *ppvalue = BrsAmf0Any::strict_array();
            return ret;
        }
        case RTMP_AMF0_Date: {
            *ppvalue = BrsAmf0Any::date();
            return ret;
        }
        case RTMP_AMF0_Invalid:
        default: {
            ret = ERROR_RTMP_AMF0_INVALID;
            brs_error("invalid amf0 message type. marker=%#x, ret=%d", marker, ret);
            return ret;
        }
    }
}

BrsUnSortedHashtable::BrsUnSortedHashtable()
{
}

BrsUnSortedHashtable::~BrsUnSortedHashtable()
{
    clear();
}

int BrsUnSortedHashtable::count()
{
    return (int)properties.size();
}

void BrsUnSortedHashtable::clear()
{
    std::vector<BrsAmf0ObjectPropertyType>::iterator it;
    for (it = properties.begin(); it != properties.end(); ++it) {
        BrsAmf0ObjectPropertyType& elem = *it;
        BrsAmf0Any* any = elem.second;
        SafeDelete(any);
    }
    properties.clear();
}

string BrsUnSortedHashtable::key_at(int index)
{
    assert(index < count());
    BrsAmf0ObjectPropertyType& elem = properties[index];
    return elem.first;
}

const char* BrsUnSortedHashtable::key_raw_at(int index)
{
    assert(index < count());
    BrsAmf0ObjectPropertyType& elem = properties[index];
    return elem.first.data();
}

BrsAmf0Any* BrsUnSortedHashtable::value_at(int index)
{
    assert(index < count());
    BrsAmf0ObjectPropertyType& elem = properties[index];
    return elem.second;
}

void BrsUnSortedHashtable::set(string key, BrsAmf0Any* value)
{
    std::vector<BrsAmf0ObjectPropertyType>::iterator it;
    
    for (it = properties.begin(); it != properties.end(); ++it) {
        BrsAmf0ObjectPropertyType& elem = *it;
        std::string name = elem.first;
        BrsAmf0Any* any = elem.second;
        
        if (key == name) {
            SafeDelete(any);
            properties.erase(it);
            break;
        }
    }
    
    if (value) {
        properties.push_back(std::make_pair(key, value));
    }
}

BrsAmf0Any* BrsUnSortedHashtable::get_property(string name)
{
    std::vector<BrsAmf0ObjectPropertyType>::iterator it;
    
    for (it = properties.begin(); it != properties.end(); ++it) {
        BrsAmf0ObjectPropertyType& elem = *it;
        std::string key = elem.first;
        BrsAmf0Any* any = elem.second;
        if (key == name) {
            return any;
        }
    }
    
    return NULL;
}

BrsAmf0Any* BrsUnSortedHashtable::ensure_property_string(string name)
{
    BrsAmf0Any* prop = get_property(name);
    
    if (!prop) {
        return NULL;
    }
    
    if (!prop->is_string()) {
        return NULL;
    }
    
    return prop;
}

BrsAmf0Any* BrsUnSortedHashtable::ensure_property_number(string name)
{
    BrsAmf0Any* prop = get_property(name);
    
    if (!prop) {
        return NULL;
    }
    
    if (!prop->is_number()) {
        return NULL;
    }
    
    return prop;
}

void BrsUnSortedHashtable::remove(string name)
{
    std::vector<BrsAmf0ObjectPropertyType>::iterator it;
    
    for (it = properties.begin(); it != properties.end();) {
        std::string key = it->first;
        BrsAmf0Any* any = it->second;
        
        if (key == name) {
            SafeDelete(any);
            
            it = properties.erase(it);
        } else {
            ++it;
        }
    }
}

void BrsUnSortedHashtable::copy(BrsUnSortedHashtable* src)
{
    std::vector<BrsAmf0ObjectPropertyType>::iterator it;
    for (it = src->properties.begin(); it != src->properties.end(); ++it) {
        BrsAmf0ObjectPropertyType& elem = *it;
        std::string key = elem.first;
        BrsAmf0Any* any = elem.second;
        set(key, any->copy());
    }
}

BrsAmf0ObjectEOF::BrsAmf0ObjectEOF()
{
    marker = RTMP_AMF0_ObjectEnd;
}

BrsAmf0ObjectEOF::~BrsAmf0ObjectEOF()
{
}

int BrsAmf0ObjectEOF::total_size()
{
    return BrsAmf0Size::object_eof();
}

int BrsAmf0ObjectEOF::read(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    // value
    if (!stream->require(2)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read object eof value failed. ret=%d", ret);
        return ret;
    }
    int16_t temp = stream->read_2bytes();
    if (temp != 0x00) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read object eof value check failed. "
            "must be 0x00, actual is %#x, ret=%d", temp, ret);
        return ret;
    }
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read object eof marker failed. ret=%d", ret);
        return ret;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_ObjectEnd) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 check object eof marker failed. "
            "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_ObjectEnd, ret);
        return ret;
    }
    brs_verbose("amf0 read object eof marker success");
    
    brs_verbose("amf0 read object eof success");
    
    return ret;
}
int BrsAmf0ObjectEOF::write(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    // value
    if (!stream->require(2)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write object eof value failed. ret=%d", ret);
        return ret;
    }
    stream->write_2bytes(0x00);
    brs_verbose("amf0 write object eof value success");
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write object eof marker failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_1bytes(RTMP_AMF0_ObjectEnd);
    
    brs_verbose("amf0 read object eof success");
    
    return ret;
}

BrsAmf0Any* BrsAmf0ObjectEOF::copy()
{
    return new BrsAmf0ObjectEOF();
}

BrsAmf0Object::BrsAmf0Object()
{
    properties = new BrsUnSortedHashtable();
    eof = new BrsAmf0ObjectEOF();
    marker = RTMP_AMF0_Object;
}

BrsAmf0Object::~BrsAmf0Object()
{
    SafeDelete(properties);
    SafeDelete(eof);
}

int BrsAmf0Object::total_size()
{
    int size = 1;
    
    for (int i = 0; i < properties->count(); i++){
        std::string name = key_at(i);
        BrsAmf0Any* value = value_at(i);
        
        size += BrsAmf0Size::utf8(name);
        size += BrsAmf0Size::any(value);
    }
    
    size += BrsAmf0Size::object_eof();
    
    return size;
}

int BrsAmf0Object::read(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read object marker failed. ret=%d", ret);
        return ret;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Object) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 check object marker failed. "
            "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_Object, ret);
        return ret;
    }
    brs_verbose("amf0 read object marker success");
    
    // value
    while (!stream->empty()) {
        // detect whether is eof.
        if (brs_amf0_is_object_eof(stream)) {
            BrsAmf0ObjectEOF pbj_eof;
            if ((ret = pbj_eof.read(stream)) != ERROR_SUCCESS) {
                brs_error("amf0 object read eof failed. ret=%d", ret);
                return ret;
            }
            brs_info("amf0 read object EOF.");
            break;
        }
        
        // property-name: utf8 string
        std::string property_name;
        if ((ret = brs_amf0_read_utf8(stream, property_name)) != ERROR_SUCCESS) {
            brs_error("amf0 object read property name failed. ret=%d", ret);
            return ret;
        }
        // property-value: any
        BrsAmf0Any* property_value = NULL;
        if ((ret = brs_amf0_read_any(stream, &property_value)) != ERROR_SUCCESS) {
            brs_error("amf0 object read property_value failed. "
                "name=%s, ret=%d", property_name.c_str(), ret);
            SafeDelete(property_value);
            return ret;
        }
        
        // add property
        this->set(property_name, property_value);
    }
    
    return ret;
}

int BrsAmf0Object::write(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write object marker failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_1bytes(RTMP_AMF0_Object);
    brs_verbose("amf0 write object marker success");
    
    // value
    for (int i = 0; i < properties->count(); i++) {
        std::string name = this->key_at(i);
        BrsAmf0Any* any = this->value_at(i);
        
        if ((ret = brs_amf0_write_utf8(stream, name)) != ERROR_SUCCESS) {
            brs_error("write object property name failed. ret=%d", ret);
            return ret;
        }
        
        if ((ret = brs_amf0_write_any(stream, any)) != ERROR_SUCCESS) {
            brs_error("write object property value failed. ret=%d", ret);
            return ret;
        }
        
        brs_verbose("write amf0 property success. name=%s", name.c_str());
    }
    
    if ((ret = eof->write(stream)) != ERROR_SUCCESS) {
        brs_error("write object eof failed. ret=%d", ret);
        return ret;
    }
    
    brs_verbose("write amf0 object success.");
    
    return ret;
}

BrsAmf0Any* BrsAmf0Object::copy()
{
    BrsAmf0Object* copy = new BrsAmf0Object();
    copy->properties->copy(properties);
    return copy;
}

void BrsAmf0Object::clear()
{
    properties->clear();
}

int BrsAmf0Object::count()
{
    return properties->count();
}

string BrsAmf0Object::key_at(int index)
{
    return properties->key_at(index);
}

const char* BrsAmf0Object::key_raw_at(int index)
{
    return properties->key_raw_at(index);
}

BrsAmf0Any* BrsAmf0Object::value_at(int index)
{
    return properties->value_at(index);
}

void BrsAmf0Object::set(string key, BrsAmf0Any* value)
{
    properties->set(key, value);
}

BrsAmf0Any* BrsAmf0Object::get_property(string name)
{
    return properties->get_property(name);
}

BrsAmf0Any* BrsAmf0Object::ensure_property_string(string name)
{
    return properties->ensure_property_string(name);
}

BrsAmf0Any* BrsAmf0Object::ensure_property_number(string name)
{
    return properties->ensure_property_number(name);
}

void BrsAmf0Object::remove(string name)
{
    properties->remove(name);
}

BrsAmf0EcmaArray::BrsAmf0EcmaArray()
{
    _count = 0;
    properties = new BrsUnSortedHashtable();
    eof = new BrsAmf0ObjectEOF();
    marker = RTMP_AMF0_EcmaArray;
}

BrsAmf0EcmaArray::~BrsAmf0EcmaArray()
{
    SafeDelete(properties);
    SafeDelete(eof);
}

int BrsAmf0EcmaArray::total_size()
{
    int size = 1 + 4;
    
    for (int i = 0; i < properties->count(); i++){
        std::string name = key_at(i);
        BrsAmf0Any* value = value_at(i);
        
        size += BrsAmf0Size::utf8(name);
        size += BrsAmf0Size::any(value);
    }
    
    size += BrsAmf0Size::object_eof();
    
    return size;
}

int BrsAmf0EcmaArray::read(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read ecma_array marker failed. ret=%d", ret);
        return ret;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_EcmaArray) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 check ecma_array marker failed. "
            "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_EcmaArray, ret);
        return ret;
    }
    brs_verbose("amf0 read ecma_array marker success");

    // count
    if (!stream->require(4)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read ecma_array count failed. ret=%d", ret);
        return ret;
    }
    
    int32_t count = stream->read_4bytes();
    brs_verbose("amf0 read ecma_array count success. count=%d", count);
    
    // value
    this->_count = count;

    while (!stream->empty()) {
        // detect whether is eof.
        if (brs_amf0_is_object_eof(stream)) {
            BrsAmf0ObjectEOF pbj_eof;
            if ((ret = pbj_eof.read(stream)) != ERROR_SUCCESS) {
                brs_error("amf0 ecma_array read eof failed. ret=%d", ret);
                return ret;
            }
            brs_info("amf0 read ecma_array EOF.");
            break;
        }
        
        // property-name: utf8 string
        std::string property_name;
        if ((ret =brs_amf0_read_utf8(stream, property_name)) != ERROR_SUCCESS) {
            brs_error("amf0 ecma_array read property name failed. ret=%d", ret);
            return ret;
        }
        // property-value: any
        BrsAmf0Any* property_value = NULL;
        if ((ret = brs_amf0_read_any(stream, &property_value)) != ERROR_SUCCESS) {
            brs_error("amf0 ecma_array read property_value failed. "
                "name=%s, ret=%d", property_name.c_str(), ret);
            return ret;
        }
        
        // add property
        this->set(property_name, property_value);
    }
    
    return ret;
}
int BrsAmf0EcmaArray::write(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write ecma_array marker failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_1bytes(RTMP_AMF0_EcmaArray);
    brs_verbose("amf0 write ecma_array marker success");

    // count
    if (!stream->require(4)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write ecma_array count failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_4bytes(this->_count);
    brs_verbose("amf0 write ecma_array count success. count=%d", _count);
    
    // value
    for (int i = 0; i < properties->count(); i++) {
        std::string name = this->key_at(i);
        BrsAmf0Any* any = this->value_at(i);
        
        if ((ret = brs_amf0_write_utf8(stream, name)) != ERROR_SUCCESS) {
            brs_error("write ecma_array property name failed. ret=%d", ret);
            return ret;
        }
        
        if ((ret = brs_amf0_write_any(stream, any)) != ERROR_SUCCESS) {
            brs_error("write ecma_array property value failed. ret=%d", ret);
            return ret;
        }
        
        brs_verbose("write amf0 property success. name=%s", name.c_str());
    }
    
    if ((ret = eof->write(stream)) != ERROR_SUCCESS) {
        brs_error("write ecma_array eof failed. ret=%d", ret);
        return ret;
    }
    
    brs_verbose("write ecma_array object success.");
    
    return ret;
}

BrsAmf0Any* BrsAmf0EcmaArray::copy()
{
    BrsAmf0EcmaArray* copy = new BrsAmf0EcmaArray();
    copy->properties->copy(properties);
    copy->_count = _count;
    return copy;
}

void BrsAmf0EcmaArray::clear()
{
    properties->clear();
}

int BrsAmf0EcmaArray::count()
{
    return properties->count();
}

string BrsAmf0EcmaArray::key_at(int index)
{
    return properties->key_at(index);
}

const char* BrsAmf0EcmaArray::key_raw_at(int index)
{
    return properties->key_raw_at(index);
}

BrsAmf0Any* BrsAmf0EcmaArray::value_at(int index)
{
    return properties->value_at(index);
}

void BrsAmf0EcmaArray::set(string key, BrsAmf0Any* value)
{
    properties->set(key, value);
}

BrsAmf0Any* BrsAmf0EcmaArray::get_property(string name)
{
    return properties->get_property(name);
}

BrsAmf0Any* BrsAmf0EcmaArray::ensure_property_string(string name)
{
    return properties->ensure_property_string(name);
}

BrsAmf0Any* BrsAmf0EcmaArray::ensure_property_number(string name)
{
    return properties->ensure_property_number(name);
}

BrsAmf0StrictArray::BrsAmf0StrictArray()
{
    marker = RTMP_AMF0_StrictArray;
    _count = 0;
}

BrsAmf0StrictArray::~BrsAmf0StrictArray()
{
    std::vector<BrsAmf0Any*>::iterator it;
    for (it = properties.begin(); it != properties.end(); ++it) {
        BrsAmf0Any* any = *it;
        SafeDelete(any);
    }
    properties.clear();
}

int BrsAmf0StrictArray::total_size()
{
    int size = 1 + 4;
    
    for (int i = 0; i < (int)properties.size(); i++){
        BrsAmf0Any* any = properties[i];
        size += any->total_size();
    }
    
    return size;
}

int BrsAmf0StrictArray::read(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read strict_array marker failed. ret=%d", ret);
        return ret;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_StrictArray) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 check strict_array marker failed. "
            "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_StrictArray, ret);
        return ret;
    }
    brs_verbose("amf0 read strict_array marker success");

    // count
    if (!stream->require(4)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read strict_array count failed. ret=%d", ret);
        return ret;
    }
    
    int32_t count = stream->read_4bytes();
    brs_verbose("amf0 read strict_array count success. count=%d", count);
    
    // value
    this->_count = count;

    for (int i = 0; i < count && !stream->empty(); i++) {
        // property-value: any
        BrsAmf0Any* elem = NULL;
        if ((ret = brs_amf0_read_any(stream, &elem)) != ERROR_SUCCESS) {
            brs_error("amf0 strict_array read value failed. ret=%d", ret);
            return ret;
        }
        
        // add property
        properties.push_back(elem);
    }
    
    return ret;
}
int BrsAmf0StrictArray::write(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write strict_array marker failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_1bytes(RTMP_AMF0_StrictArray);
    brs_verbose("amf0 write strict_array marker success");

    // count
    if (!stream->require(4)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write strict_array count failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_4bytes(this->_count);
    brs_verbose("amf0 write strict_array count success. count=%d", _count);
    
    // value
    for (int i = 0; i < (int)properties.size(); i++) {
        BrsAmf0Any* any = properties[i];
        
        if ((ret = brs_amf0_write_any(stream, any)) != ERROR_SUCCESS) {
            brs_error("write strict_array property value failed. ret=%d", ret);
            return ret;
        }
        
        brs_verbose("write amf0 property success.");
    }
    
    brs_verbose("write strict_array object success.");
    
    return ret;
}

BrsAmf0Any* BrsAmf0StrictArray::copy()
{
    BrsAmf0StrictArray* copy = new BrsAmf0StrictArray();
    
    std::vector<BrsAmf0Any*>::iterator it;
    for (it = properties.begin(); it != properties.end(); ++it) {
        BrsAmf0Any* any = *it;
        copy->append(any->copy());
    }
    
    copy->_count = _count;
    return copy;
}

void BrsAmf0StrictArray::clear()
{
    properties.clear();
}

int BrsAmf0StrictArray::count()
{
    return properties.size();
}

BrsAmf0Any* BrsAmf0StrictArray::at(int index)
{
    assert(index < (int)properties.size());
    return properties.at(index);
}

void BrsAmf0StrictArray::append(BrsAmf0Any* any)
{
    properties.push_back(any);
    _count = (int32_t)properties.size();
}

int BrsAmf0Size::utf8(string value)
{
    return 2 + value.length();
}

int BrsAmf0Size::str(string value)
{
    return 1 + BrsAmf0Size::utf8(value);
}

int BrsAmf0Size::number()
{
    return 1 + 8;
}

int BrsAmf0Size::date()
{
    return 1 + 8 + 2;
}

int BrsAmf0Size::null()
{
    return 1;
}

int BrsAmf0Size::undefined()
{
    return 1;
}

int BrsAmf0Size::boolean()
{
    return 1 + 1;
}

int BrsAmf0Size::object(BrsAmf0Object* obj)
{
    if (!obj) {
        return 0;
    }
    
    return obj->total_size();
}

int BrsAmf0Size::object_eof()
{
    return 2 + 1;
}

int BrsAmf0Size::ecma_array(BrsAmf0EcmaArray* arr)
{
    if (!arr) {
        return 0;
    }
    
    return arr->total_size();
}

int BrsAmf0Size::strict_array(BrsAmf0StrictArray* arr)
{
    if (!arr) {
        return 0;
    }
    
    return arr->total_size();
}

int BrsAmf0Size::any(BrsAmf0Any* o)
{
    if (!o) {
        return 0;
    }
    
    return o->total_size();
}

BrsAmf0String::BrsAmf0String(const char* _value)
{
    marker = RTMP_AMF0_String;
    if (_value) {
        value = _value;
    }
}

BrsAmf0String::~BrsAmf0String()
{
}

int BrsAmf0String::total_size()
{
    return BrsAmf0Size::str(value);
}

int BrsAmf0String::read(BRSStream* stream)
{
    return brs_amf0_read_string(stream, value);
}

int BrsAmf0String::write(BRSStream* stream)
{
    return brs_amf0_write_string(stream, value);
}

BrsAmf0Any* BrsAmf0String::copy()
{
    BrsAmf0String* copy = new BrsAmf0String(value.c_str());
    return copy;
}

BrsAmf0Boolean::BrsAmf0Boolean(bool _value)
{
    marker = RTMP_AMF0_Boolean;
    value = _value;
}

BrsAmf0Boolean::~BrsAmf0Boolean()
{
}

int BrsAmf0Boolean::total_size()
{
    return BrsAmf0Size::boolean();
}

int BrsAmf0Boolean::read(BRSStream* stream)
{
    return brs_amf0_read_boolean(stream, value);
}

int BrsAmf0Boolean::write(BRSStream* stream)
{
    return brs_amf0_write_boolean(stream, value);
}

BrsAmf0Any* BrsAmf0Boolean::copy()
{
    BrsAmf0Boolean* copy = new BrsAmf0Boolean(value);
    return copy;
}

BrsAmf0Number::BrsAmf0Number(double _value)
{
    marker = RTMP_AMF0_Number;
    value = _value;
}

BrsAmf0Number::~BrsAmf0Number()
{
}

int BrsAmf0Number::total_size()
{
    return BrsAmf0Size::number();
}

int BrsAmf0Number::read(BRSStream* stream)
{
    return brs_amf0_read_number(stream, value);
}

int BrsAmf0Number::write(BRSStream* stream)
{
    return brs_amf0_write_number(stream, value);
}

BrsAmf0Any* BrsAmf0Number::copy()
{
    BrsAmf0Number* copy = new BrsAmf0Number(value);
    return copy;
}

BrsAmf0Date::BrsAmf0Date(int64_t value)
{
    marker = RTMP_AMF0_Date;
    _date_value = value;
    _time_zone = 0;
}

BrsAmf0Date::~BrsAmf0Date()
{
}

int BrsAmf0Date::total_size()
{
    return BrsAmf0Size::date();
}

int BrsAmf0Date::read(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read date marker failed. ret=%d", ret);
        return ret;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Date) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 check date marker failed. "
            "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_Date, ret);
        return ret;
    }
    brs_verbose("amf0 read date marker success");

    // date value
    // An ActionScript Date is serialized as the number of milliseconds 
    // elapsed since the epoch of midnight on 1st Jan 1970 in the UTC 
    // time zone.
    if (!stream->require(8)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read date failed. ret=%d", ret);
        return ret;
    }
    
    _date_value = stream->read_8bytes();
    brs_verbose("amf0 read date success. date=%"PRId64, _date_value);
    
    // time zone
    // While the design of this type reserves room for time zone offset 
    // information, it should not be filled in, nor used, as it is unconventional 
    // to change time zones when serializing dates on a network. It is suggested 
    // that the time zone be queried independently as needed.
    if (!stream->require(2)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read time zone failed. ret=%d", ret);
        return ret;
    }
    
    _time_zone = stream->read_2bytes();
    brs_verbose("amf0 read time zone success. zone=%d", _time_zone);
    
    return ret;
}
int BrsAmf0Date::write(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write date marker failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_1bytes(RTMP_AMF0_Date);
    brs_verbose("amf0 write date marker success");

    // date value
    if (!stream->require(8)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write date failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_8bytes(_date_value);
    brs_verbose("amf0 write date success. date=%"PRId64, _date_value);

    // time zone
    if (!stream->require(2)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write time zone failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_2bytes(_time_zone);
    brs_verbose("amf0 write time zone success. date=%d", _time_zone);
    
    brs_verbose("write date object success.");
    
    return ret;
}

BrsAmf0Any* BrsAmf0Date::copy()
{
    BrsAmf0Date* copy = new BrsAmf0Date(0);
    
    copy->_date_value = _date_value;
    copy->_time_zone = _time_zone;
    
    return copy;
}

int64_t BrsAmf0Date::date()
{
    return _date_value;
}

int16_t BrsAmf0Date::time_zone()
{
    return _time_zone;
}

BrsAmf0Null::BrsAmf0Null()
{
    marker = RTMP_AMF0_Null;
}

BrsAmf0Null::~BrsAmf0Null()
{
}

int BrsAmf0Null::total_size()
{
    return BrsAmf0Size::null();
}

int BrsAmf0Null::read(BRSStream* stream)
{
    return brs_amf0_read_null(stream);
}

int BrsAmf0Null::write(BRSStream* stream)
{
    return brs_amf0_write_null(stream);
}

BrsAmf0Any* BrsAmf0Null::copy()
{
    BrsAmf0Null* copy = new BrsAmf0Null();
    return copy;
}

BrsAmf0Undefined::BrsAmf0Undefined()
{
    marker = RTMP_AMF0_Undefined;
}

BrsAmf0Undefined::~BrsAmf0Undefined()
{
}

int BrsAmf0Undefined::total_size()
{
    return BrsAmf0Size::undefined();
}

int BrsAmf0Undefined::read(BRSStream* stream)
{
    return brs_amf0_read_undefined(stream);
}

int BrsAmf0Undefined::write(BRSStream* stream)
{
    return brs_amf0_write_undefined(stream);
}

BrsAmf0Any* BrsAmf0Undefined::copy()
{
    BrsAmf0Undefined* copy = new BrsAmf0Undefined();
    return copy;
}

int brs_amf0_read_any(BRSStream* stream, BrsAmf0Any** ppvalue)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = BrsAmf0Any::discovery(stream, ppvalue)) != ERROR_SUCCESS) {
        brs_error("amf0 discovery any elem failed. ret=%d", ret);
        return ret;
    }
    
    assert(*ppvalue);
    
    if ((ret = (*ppvalue)->read(stream)) != ERROR_SUCCESS) {
        brs_error("amf0 parse elem failed. ret=%d", ret);
        SafeDelete(*ppvalue);
        return ret;
    }
    
    return ret;
}

int brs_amf0_read_string(BRSStream* stream, string& value)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read string marker failed. ret=%d", ret);
        return ret;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_String) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 check string marker failed. "
            "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_String, ret);
        return ret;
    }
    brs_verbose("amf0 read string marker success");
    
    return brs_amf0_read_utf8(stream, value);
}

int brs_amf0_write_string(BRSStream* stream, string value)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write string marker failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_1bytes(RTMP_AMF0_String);
    brs_verbose("amf0 write string marker success");
    
    return brs_amf0_write_utf8(stream, value);
}

int brs_amf0_read_boolean(BRSStream* stream, bool& value)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read bool marker failed. ret=%d", ret);
        return ret;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Boolean) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 check bool marker failed. "
            "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_Boolean, ret);
        return ret;
    }
    brs_verbose("amf0 read bool marker success");

    // value
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read bool value failed. ret=%d", ret);
        return ret;
    }

    value = (stream->read_1bytes() != 0);
    
    brs_verbose("amf0 read bool value success. value=%d", value);
    
    return ret;
}
int brs_amf0_write_boolean(BRSStream* stream, bool value)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write bool marker failed. ret=%d", ret);
        return ret;
    }
    stream->write_1bytes(RTMP_AMF0_Boolean);
    brs_verbose("amf0 write bool marker success");

    // value
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write bool value failed. ret=%d", ret);
        return ret;
    }

    if (value) {
        stream->write_1bytes(0x01);
    } else {
        stream->write_1bytes(0x00);
    }
    
    brs_verbose("amf0 write bool value success. value=%d", value);
    
    return ret;
}

int brs_amf0_read_number(BRSStream* stream, double& value)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read number marker failed. ret=%d", ret);
        return ret;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Number) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 check number marker failed. "
            "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_Number, ret);
        return ret;
    }
    brs_verbose("amf0 read number marker success");

    // value
    if (!stream->require(8)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read number value failed. ret=%d", ret);
        return ret;
    }

    int64_t temp = stream->read_8bytes();
    memcpy(&value, &temp, 8);
    
    brs_verbose("amf0 read number value success. value=%.2f", value);
    
    return ret;
}
int brs_amf0_write_number(BRSStream* stream, double value)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write number marker failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_1bytes(RTMP_AMF0_Number);
    brs_verbose("amf0 write number marker success");

    // value
    if (!stream->require(8)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write number value failed. ret=%d", ret);
        return ret;
    }

    int64_t temp = 0x00;
    memcpy(&temp, &value, 8);
    stream->write_8bytes(temp);
    
    brs_verbose("amf0 write number value success. value=%.2f", value);
    
    return ret;
}

int brs_amf0_read_null(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read null marker failed. ret=%d", ret);
        return ret;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Null) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 check null marker failed. "
            "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_Null, ret);
        return ret;
    }
    brs_verbose("amf0 read null success");
    
    return ret;
}
int brs_amf0_write_null(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write null marker failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_1bytes(RTMP_AMF0_Null);
    brs_verbose("amf0 write null marker success");
    
    return ret;
}

int brs_amf0_read_undefined(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read undefined marker failed. ret=%d", ret);
        return ret;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Undefined) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 check undefined marker failed. "
            "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_Undefined, ret);
        return ret;
    }
    brs_verbose("amf0 read undefined success");
    
    return ret;
}
int brs_amf0_write_undefined(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    // marker
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        brs_error("amf0 write undefined marker failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_1bytes(RTMP_AMF0_Undefined);
    brs_verbose("amf0 write undefined marker success");
    
    return ret;
}


namespace _brs_internal
{
    int brs_amf0_read_utf8(BRSStream* stream, string& value)
    {
        int ret = ERROR_SUCCESS;
        
        // len
        if (!stream->require(2)) {
            ret = ERROR_RTMP_AMF0_DECODE;
            brs_error("amf0 read string length failed. ret=%d", ret);
            return ret;
        }
        int16_t len = stream->read_2bytes();
        brs_verbose("amf0 read string length success. len=%d", len);
        
        // empty string
        if (len <= 0) {
            brs_verbose("amf0 read empty string. ret=%d", ret);
            return ret;
        }
        
        // data
        if (!stream->require(len)) {
            ret = ERROR_RTMP_AMF0_DECODE;
            brs_error("amf0 read string data failed. ret=%d", ret);
            return ret;
        }
        std::string str = stream->read_string(len);
        
        // support utf8-1 only
        // 1.3.1 Strings and UTF-8
        // UTF8-1 = %x00-7F
        // TODO: support other utf-8 strings
        /*for (int i = 0; i < len; i++) {
            char ch = *(str.data() + i);
            if ((ch & 0x80) != 0) {
                ret = ERROR_RTMP_AMF0_DECODE;
                brs_error("ignored. only support utf8-1, 0x00-0x7F, actual is %#x. ret=%d", (int)ch, ret);
                ret = ERROR_SUCCESS;
            }
        }*/
        
        value = str;
        brs_verbose("amf0 read string data success. str=%s", str.c_str());
        
        return ret;
    }
    int brs_amf0_write_utf8(BRSStream* stream, string value)
    {
        int ret = ERROR_SUCCESS;
        
        // len
        if (!stream->require(2)) {
            ret = ERROR_RTMP_AMF0_ENCODE;
            brs_error("amf0 write string length failed. ret=%d", ret);
            return ret;
        }
        stream->write_2bytes(value.length());
        brs_verbose("amf0 write string length success. len=%d", (int)value.length());
        
        // empty string
        if (value.length() <= 0) {
            brs_verbose("amf0 write empty string. ret=%d", ret);
            return ret;
        }
        
        // data
        if (!stream->require(value.length())) {
            ret = ERROR_RTMP_AMF0_ENCODE;
            brs_error("amf0 write string data failed. ret=%d", ret);
            return ret;
        }
        stream->write_string(value);
        brs_verbose("amf0 write string data success. str=%s", value.c_str());
        
        return ret;
    }
    
    bool brs_amf0_is_object_eof(BRSStream* stream) 
    {
        // detect the object-eof specially
        if (stream->require(3)) {
            int32_t flag = stream->read_3bytes();
            stream->skip(-3);
            
            return 0x09 == flag;
        }
        
        return false;
    }
    
    int brs_amf0_write_object_eof(BRSStream* stream, BrsAmf0ObjectEOF* value)
    {
        int ret = ERROR_SUCCESS;
        
        assert(value != NULL);
        
        // value
        if (!stream->require(2)) {
            ret = ERROR_RTMP_AMF0_ENCODE;
            brs_error("amf0 write object eof value failed. ret=%d", ret);
            return ret;
        }
        stream->write_2bytes(0x00);
        brs_verbose("amf0 write object eof value success");
        
        // marker
        if (!stream->require(1)) {
            ret = ERROR_RTMP_AMF0_ENCODE;
            brs_error("amf0 write object eof marker failed. ret=%d", ret);
            return ret;
        }
        
        stream->write_1bytes(RTMP_AMF0_ObjectEnd);
        
        brs_verbose("amf0 read object eof success");
        
        return ret;
    }

    int brs_amf0_write_any(BRSStream* stream, BrsAmf0Any* value)
    {
        assert(value != NULL);
        return value->write(stream);
    }
} 
  
}