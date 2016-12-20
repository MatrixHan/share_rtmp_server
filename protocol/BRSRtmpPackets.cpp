#include <BRSRtmpPackets.h>


#include <BRSKernelError.h>
#include <BRSRtmpUtility.h>
#include <BRSLog.h>
#include <BRSFlv.h>

using namespace std;

namespace BRS 
{
  
BrsConnectAppPacket::BrsConnectAppPacket()
{
    command_name = RTMP_AMF0_COMMAND_CONNECT;
    transaction_id = 1;
    command_object = BrsAmf0Any::object();
    // optional
    args = NULL;
}

BrsConnectAppPacket::~BrsConnectAppPacket()
{
    SafeDelete(command_object);
    SafeDelete(args);
}

int BrsConnectAppPacket::decode(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = brs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("amf0 decode connect command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_CONNECT) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 decode connect command_name failed. "
            "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("amf0 decode connect transaction_id failed. ret=%d", ret);
        return ret;
    }
    
    // some client donot send id=1.0, so we only warn user if not match.
    if (transaction_id != 1.0) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_warn("amf0 decode connect transaction_id failed. "
            "required=%.1f, actual=%.1f, ret=%d", 1.0, transaction_id, ret);
        ret = ERROR_SUCCESS;
    }
    
    if ((ret = command_object->read(stream)) != ERROR_SUCCESS) {
        brs_error("amf0 decode connect command_object failed. ret=%d", ret);
        return ret;
    }
    
    if (!stream->empty()) {
        SafeDelete(args);
        
        // see: https://github.com/osbrs/brs/issues/186
        // the args maybe any amf0, for instance, a string. we should drop if not object.
        BrsAmf0Any* any = NULL;
        if ((ret = BrsAmf0Any::discovery(stream, &any)) != ERROR_SUCCESS) {
            brs_error("amf0 find connect args failed. ret=%d", ret);
            return ret;
        }
        assert(any);
        
        // read the instance
        if ((ret = any->read(stream)) != ERROR_SUCCESS) {
            brs_error("amf0 decode connect args failed. ret=%d", ret);
            SafeDelete(any);
            return ret;
        }
        
        // drop when not an AMF0 object.
        if (!any->is_object()) {
            brs_warn("drop the args, see: '4.1.1. connect', marker=%#x", any->marker);
            SafeDelete(any);
        } else {
            args = any->to_object();
        }
    }
    
    brs_info("amf0 decode connect packet success");
    
    return ret;
}

int BrsConnectAppPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int BrsConnectAppPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int BrsConnectAppPacket::get_size()
{
    int size = 0;
    
    size += BrsAmf0Size::str(command_name);
    size += BrsAmf0Size::number();
    size += BrsAmf0Size::object(command_object);
    if (args) {
        size += BrsAmf0Size::object(args);
    }
    
    return size;
}

int BrsConnectAppPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_name success.");
    
    if ((ret = brs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode transaction_id success.");
    
    if ((ret = command_object->write(stream)) != ERROR_SUCCESS) {
        brs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_object success.");
    
    if (args && (ret = args->write(stream)) != ERROR_SUCCESS) {
        brs_error("encode args failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode args success.");
    
    brs_info("encode connect app request packet success.");
    
    return ret;
}

BrsConnectAppResPacket::BrsConnectAppResPacket()
{
    command_name = RTMP_AMF0_COMMAND_RESULT;
    transaction_id = 1;
    props = BrsAmf0Any::object();
    info = BrsAmf0Any::object();
}

BrsConnectAppResPacket::~BrsConnectAppResPacket()
{
    SafeDelete(props);
    SafeDelete(info);
}

int BrsConnectAppResPacket::decode(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = brs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("amf0 decode connect command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_RESULT) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 decode connect command_name failed. "
            "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("amf0 decode connect transaction_id failed. ret=%d", ret);
        return ret;
    }
    
    // some client donot send id=1.0, so we only warn user if not match.
    if (transaction_id != 1.0) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_warn("amf0 decode connect transaction_id failed. "
            "required=%.1f, actual=%.1f, ret=%d", 1.0, transaction_id, ret);
        ret = ERROR_SUCCESS;
    }
    
    // for RED5(1.0.6), the props is NULL, we must ignore it.
    // @see https://github.com/osbrs/brs/issues/418
    if (!stream->empty()) {
        BrsAmf0Any* p = NULL;
        if ((ret = brs_amf0_read_any(stream, &p)) != ERROR_SUCCESS) {
            brs_error("amf0 decode connect props failed. ret=%d", ret);
            return ret;
        }
        
        // ignore when props is not amf0 object.
        if (!p->is_object()) {
            brs_warn("ignore connect response props marker=%#x.", (u_int8_t)p->marker);
            SafeDelete(p);
        } else {
            SafeDelete(props);
            props = p->to_object();
            brs_info("accept amf0 object connect response props");
        }
    }
    
    if ((ret = info->read(stream)) != ERROR_SUCCESS) {
        brs_error("amf0 decode connect info failed. ret=%d", ret);
        return ret;
    }
    
    brs_info("amf0 decode connect response packet success");
    
    return ret;
}

int BrsConnectAppResPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int BrsConnectAppResPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int BrsConnectAppResPacket::get_size()
{
    return BrsAmf0Size::str(command_name) + BrsAmf0Size::number() 
        + BrsAmf0Size::object(props) + BrsAmf0Size::object(info);
}

int BrsConnectAppResPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_name success.");
    
    if ((ret = brs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode transaction_id success.");
    
    if ((ret = props->write(stream)) != ERROR_SUCCESS) {
        brs_error("encode props failed. ret=%d", ret);
        return ret;
    }

    brs_verbose("encode props success.");
    
    if ((ret = info->write(stream)) != ERROR_SUCCESS) {
        brs_error("encode info failed. ret=%d", ret);
        return ret;
    }

    brs_verbose("encode info success.");
    
    brs_info("encode connect app response packet success.");
    
    return ret;
}

BrsCallPacket::BrsCallPacket()
{
    command_name = "";
    transaction_id = 0;
    command_object = NULL;
    arguments = NULL;
}

BrsCallPacket::~BrsCallPacket()
{
    SafeDelete(command_object);
    SafeDelete(arguments);
}

int BrsCallPacket::decode(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = brs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("amf0 decode call command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty()) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 decode call command_name failed. "
            "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("amf0 decode call transaction_id failed. ret=%d", ret);
        return ret;
    }
    
    SafeDelete(command_object);
    if ((ret = BrsAmf0Any::discovery(stream, &command_object)) != ERROR_SUCCESS) {
        brs_error("amf0 discovery call command_object failed. ret=%d", ret);
        return ret;
    }
    if ((ret = command_object->read(stream)) != ERROR_SUCCESS) {
        brs_error("amf0 decode call command_object failed. ret=%d", ret);
        return ret;
    }
    
    if (!stream->empty()) {
        SafeDelete(arguments);
        if ((ret = BrsAmf0Any::discovery(stream, &arguments)) != ERROR_SUCCESS) {
            brs_error("amf0 discovery call arguments failed. ret=%d", ret);
            return ret;
        }
        if ((ret = arguments->read(stream)) != ERROR_SUCCESS) {
            brs_error("amf0 decode call arguments failed. ret=%d", ret);
            return ret;
        }
    }
    
    brs_info("amf0 decode call packet success");
    
    return ret;
}

int BrsCallPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int BrsCallPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int BrsCallPacket::get_size()
{
    int size = 0;
    
    size += BrsAmf0Size::str(command_name) + BrsAmf0Size::number();
    
    if (command_object) {
        size += command_object->total_size();
    }
    
    if (arguments) {
        size += arguments->total_size();
    }
    
    return size;
}

int BrsCallPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_name success.");
    
    if ((ret = brs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode transaction_id success.");
    
    if (command_object && (ret = command_object->write(stream)) != ERROR_SUCCESS) {
        brs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_object success.");
    
    if (arguments && (ret = arguments->write(stream)) != ERROR_SUCCESS) {
        brs_error("encode arguments failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode arguments success.");
    
    brs_info("encode create stream request packet success.");
    
    return ret;
}

BrsCallResPacket::BrsCallResPacket(double _transaction_id)
{
    command_name = RTMP_AMF0_COMMAND_RESULT;
    transaction_id = _transaction_id;
    command_object = NULL;
    response = NULL;
}

BrsCallResPacket::~BrsCallResPacket()
{
    SafeDelete(command_object);
    SafeDelete(response);
}

int BrsCallResPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int BrsCallResPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int BrsCallResPacket::get_size()
{
    int size = 0;
    
    size += BrsAmf0Size::str(command_name) + BrsAmf0Size::number();
    
    if (command_object) {
        size += command_object->total_size();
    }
    
    if (response) {
        size += response->total_size();
    }
    
    return size;
}

int BrsCallResPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_name success.");
    
    if ((ret = brs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode transaction_id success.");
    
    if (command_object && (ret = command_object->write(stream)) != ERROR_SUCCESS) {
        brs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_object success.");
    
    if (response && (ret = response->write(stream)) != ERROR_SUCCESS) {
        brs_error("encode response failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode response success.");
    
    
    brs_info("encode call response packet success.");
    
    return ret;
}

BrsCreateStreamPacket::BrsCreateStreamPacket()
{
    command_name = RTMP_AMF0_COMMAND_CREATE_STREAM;
    transaction_id = 2;
    command_object = BrsAmf0Any::null();
}

BrsCreateStreamPacket::~BrsCreateStreamPacket()
{
    SafeDelete(command_object);
}

int BrsCreateStreamPacket::decode(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = brs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("amf0 decode createStream command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_CREATE_STREAM) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 decode createStream command_name failed. "
            "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("amf0 decode createStream transaction_id failed. ret=%d", ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        brs_error("amf0 decode createStream command_object failed. ret=%d", ret);
        return ret;
    }
    
    brs_info("amf0 decode createStream packet success");
    
    return ret;
}

int BrsCreateStreamPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int BrsCreateStreamPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int BrsCreateStreamPacket::get_size()
{
    return BrsAmf0Size::str(command_name) + BrsAmf0Size::number()
        + BrsAmf0Size::null();
}

int BrsCreateStreamPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_name success.");
    
    if ((ret = brs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode transaction_id success.");
    
    if ((ret = brs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        brs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_object success.");
    
    brs_info("encode create stream request packet success.");
    
    return ret;
}

BrsCreateStreamResPacket::BrsCreateStreamResPacket(double _transaction_id, double _stream_id)
{
    command_name = RTMP_AMF0_COMMAND_RESULT;
    transaction_id = _transaction_id;
    command_object = BrsAmf0Any::null();
    stream_id = _stream_id;
}

BrsCreateStreamResPacket::~BrsCreateStreamResPacket()
{
    SafeDelete(command_object);
}

int BrsCreateStreamResPacket::decode(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = brs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("amf0 decode createStream command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_RESULT) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 decode createStream command_name failed. "
            "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("amf0 decode createStream transaction_id failed. ret=%d", ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        brs_error("amf0 decode createStream command_object failed. ret=%d", ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_number(stream, stream_id)) != ERROR_SUCCESS) {
        brs_error("amf0 decode createStream stream_id failed. ret=%d", ret);
        return ret;
    }
    
    brs_info("amf0 decode createStream response packet success");
    
    return ret;
}

int BrsCreateStreamResPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int BrsCreateStreamResPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int BrsCreateStreamResPacket::get_size()
{
    return BrsAmf0Size::str(command_name) + BrsAmf0Size::number()
        + BrsAmf0Size::null() + BrsAmf0Size::number();
}

int BrsCreateStreamResPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_name success.");
    
    if ((ret = brs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode transaction_id success.");
    
    if ((ret = brs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        brs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_object success.");
    
    if ((ret = brs_amf0_write_number(stream, stream_id)) != ERROR_SUCCESS) {
        brs_error("encode stream_id failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode stream_id success.");
    
    
    brs_info("encode createStream response packet success.");
    
    return ret;
}

BrsCloseStreamPacket::BrsCloseStreamPacket()
{
    command_name = RTMP_AMF0_COMMAND_CLOSE_STREAM;
    transaction_id = 0;
    command_object = BrsAmf0Any::null();
}

BrsCloseStreamPacket::~BrsCloseStreamPacket()
{
    SafeDelete(command_object);
}

int BrsCloseStreamPacket::decode(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = brs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("amf0 decode closeStream command_name failed. ret=%d", ret);
        return ret;
    }

    if ((ret = brs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("amf0 decode closeStream transaction_id failed. ret=%d", ret);
        return ret;
    }

    if ((ret = brs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        brs_error("amf0 decode closeStream command_object failed. ret=%d", ret);
        return ret;
    }
    brs_info("amf0 decode closeStream packet success");

    return ret;
}

BrsFMLEStartPacket::BrsFMLEStartPacket()
{
    command_name = RTMP_AMF0_COMMAND_RELEASE_STREAM;
    transaction_id = 0;
    command_object = BrsAmf0Any::null();
}

BrsFMLEStartPacket::~BrsFMLEStartPacket()
{
    SafeDelete(command_object);
}

int BrsFMLEStartPacket::decode(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = brs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("amf0 decode FMLE start command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() 
        || (command_name != RTMP_AMF0_COMMAND_RELEASE_STREAM 
        && command_name != RTMP_AMF0_COMMAND_FC_PUBLISH
        && command_name != RTMP_AMF0_COMMAND_UNPUBLISH)
    ) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 decode FMLE start command_name failed. "
            "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("amf0 decode FMLE start transaction_id failed. ret=%d", ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        brs_error("amf0 decode FMLE start command_object failed. ret=%d", ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_string(stream, stream_name)) != ERROR_SUCCESS) {
        brs_error("amf0 decode FMLE start stream_name failed. ret=%d", ret);
        return ret;
    }
    
    brs_info("amf0 decode FMLE start packet success");
    
    return ret;
}

int BrsFMLEStartPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int BrsFMLEStartPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int BrsFMLEStartPacket::get_size()
{
    return BrsAmf0Size::str(command_name) + BrsAmf0Size::number()
        + BrsAmf0Size::null() + BrsAmf0Size::str(stream_name);
}

int BrsFMLEStartPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_name success.");
    
    if ((ret = brs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode transaction_id success.");
    
    if ((ret = brs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        brs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_object success.");
    
    if ((ret = brs_amf0_write_string(stream, stream_name)) != ERROR_SUCCESS) {
        brs_error("encode stream_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode stream_name success.");
    
    
    brs_info("encode FMLE start response packet success.");
    
    return ret;
}

BrsFMLEStartPacket* BrsFMLEStartPacket::create_release_stream(string stream)
{
    BrsFMLEStartPacket* pkt = new BrsFMLEStartPacket();
    
    pkt->command_name = RTMP_AMF0_COMMAND_RELEASE_STREAM;
    pkt->transaction_id = 2;
    pkt->stream_name = stream;
    
    return pkt;
}

BrsFMLEStartPacket* BrsFMLEStartPacket::create_FC_publish(string stream)
{
    BrsFMLEStartPacket* pkt = new BrsFMLEStartPacket();
    
    pkt->command_name = RTMP_AMF0_COMMAND_FC_PUBLISH;
    pkt->transaction_id = 3;
    pkt->stream_name = stream;
    
    return pkt;
}

BrsFMLEStartResPacket::BrsFMLEStartResPacket(double _transaction_id)
{
    command_name = RTMP_AMF0_COMMAND_RESULT;
    transaction_id = _transaction_id;
    command_object = BrsAmf0Any::null();
    args = BrsAmf0Any::undefined();
}

BrsFMLEStartResPacket::~BrsFMLEStartResPacket()
{
    SafeDelete(command_object);
    SafeDelete(args);
}

int BrsFMLEStartResPacket::decode(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = brs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("amf0 decode FMLE start response command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_RESULT) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 decode FMLE start response command_name failed. "
            "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("amf0 decode FMLE start response transaction_id failed. ret=%d", ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        brs_error("amf0 decode FMLE start response command_object failed. ret=%d", ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_undefined(stream)) != ERROR_SUCCESS) {
        brs_error("amf0 decode FMLE start response stream_id failed. ret=%d", ret);
        return ret;
    }
    
    brs_info("amf0 decode FMLE start packet success");
    
    return ret;
}

int BrsFMLEStartResPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int BrsFMLEStartResPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int BrsFMLEStartResPacket::get_size()
{
    return BrsAmf0Size::str(command_name) + BrsAmf0Size::number()
        + BrsAmf0Size::null() + BrsAmf0Size::undefined();
}

int BrsFMLEStartResPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_name success.");
    
    if ((ret = brs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode transaction_id success.");
    
    if ((ret = brs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        brs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_object success.");
    
    if ((ret = brs_amf0_write_undefined(stream)) != ERROR_SUCCESS) {
        brs_error("encode args failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode args success.");
    
    
    brs_info("encode FMLE start response packet success.");
    
    return ret;
}

BrsPublishPacket::BrsPublishPacket()
{
    command_name = RTMP_AMF0_COMMAND_PUBLISH;
    transaction_id = 0;
    command_object = BrsAmf0Any::null();
    type = "live";
}

BrsPublishPacket::~BrsPublishPacket()
{
    SafeDelete(command_object);
}

int BrsPublishPacket::decode(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = brs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("amf0 decode publish command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_PUBLISH) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 decode publish command_name failed. "
            "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("amf0 decode publish transaction_id failed. ret=%d", ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        brs_error("amf0 decode publish command_object failed. ret=%d", ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_string(stream, stream_name)) != ERROR_SUCCESS) {
        brs_error("amf0 decode publish stream_name failed. ret=%d", ret);
        return ret;
    }
    
    if (!stream->empty() && (ret = brs_amf0_read_string(stream, type)) != ERROR_SUCCESS) {
        brs_error("amf0 decode publish type failed. ret=%d", ret);
        return ret;
    }
    
    brs_info("amf0 decode publish packet success");
    
    return ret;
}

int BrsPublishPacket::get_prefer_cid()
{
    return RTMP_CID_OverStream;
}

int BrsPublishPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int BrsPublishPacket::get_size()
{
    return BrsAmf0Size::str(command_name) + BrsAmf0Size::number()
        + BrsAmf0Size::null() + BrsAmf0Size::str(stream_name)
        + BrsAmf0Size::str(type);
}

int BrsPublishPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_name success.");
    
    if ((ret = brs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode transaction_id success.");
    
    if ((ret = brs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        brs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_object success.");
    
    if ((ret = brs_amf0_write_string(stream, stream_name)) != ERROR_SUCCESS) {
        brs_error("encode stream_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode stream_name success.");
    
    if ((ret = brs_amf0_write_string(stream, type)) != ERROR_SUCCESS) {
        brs_error("encode type failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode type success.");
    
    brs_info("encode play request packet success.");
    
    return ret;
}

BrsPausePacket::BrsPausePacket()
{
    command_name = RTMP_AMF0_COMMAND_PAUSE;
    transaction_id = 0;
    command_object = BrsAmf0Any::null();

    time_ms = 0;
    is_pause = true;
}

BrsPausePacket::~BrsPausePacket()
{
    SafeDelete(command_object);
}

int BrsPausePacket::decode(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = brs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("amf0 decode pause command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_PAUSE) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 decode pause command_name failed. "
            "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("amf0 decode pause transaction_id failed. ret=%d", ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        brs_error("amf0 decode pause command_object failed. ret=%d", ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_boolean(stream, is_pause)) != ERROR_SUCCESS) {
        brs_error("amf0 decode pause is_pause failed. ret=%d", ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_number(stream, time_ms)) != ERROR_SUCCESS) {
        brs_error("amf0 decode pause time_ms failed. ret=%d", ret);
        return ret;
    }
    
    brs_info("amf0 decode pause packet success");
    
    return ret;
}

BrsPlayPacket::BrsPlayPacket()
{
    command_name = RTMP_AMF0_COMMAND_PLAY;
    transaction_id = 0;
    command_object = BrsAmf0Any::null();

    start = -2;
    duration = -1;
    reset = true;
}

BrsPlayPacket::~BrsPlayPacket()
{
    SafeDelete(command_object);
}

int BrsPlayPacket::decode(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = brs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("amf0 decode play command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_PLAY) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 decode play command_name failed. "
            "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("amf0 decode play transaction_id failed. ret=%d", ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        brs_error("amf0 decode play command_object failed. ret=%d", ret);
        return ret;
    }
    
    if ((ret = brs_amf0_read_string(stream, stream_name)) != ERROR_SUCCESS) {
        brs_error("amf0 decode play stream_name failed. ret=%d", ret);
        return ret;
    }
    
    if (!stream->empty() && (ret = brs_amf0_read_number(stream, start)) != ERROR_SUCCESS) {
        brs_error("amf0 decode play start failed. ret=%d", ret);
        return ret;
    }
    if (!stream->empty() && (ret = brs_amf0_read_number(stream, duration)) != ERROR_SUCCESS) {
        brs_error("amf0 decode play duration failed. ret=%d", ret);
        return ret;
    }

    if (stream->empty()) {
        return ret;
    }
    
    BrsAmf0Any* reset_value = NULL;
    if ((ret = brs_amf0_read_any(stream, &reset_value)) != ERROR_SUCCESS) {
        ret = ERROR_RTMP_AMF0_DECODE;
        brs_error("amf0 read play reset marker failed. ret=%d", ret);
        return ret;
    }
    BrsAutoFreeE(BrsAmf0Any, reset_value);
    
    if (reset_value) {
        // check if the value is bool or number
        // An optional Boolean value or number that specifies whether
        // to flush any previous playlist
        if (reset_value->is_boolean()) {
            reset = reset_value->to_boolean();
        } else if (reset_value->is_number()) {
            reset = (reset_value->to_number() != 0);
        } else {
            ret = ERROR_RTMP_AMF0_DECODE;
            brs_error("amf0 invalid type=%#x, requires number or bool, ret=%d", reset_value->marker, ret);
            return ret;
        }
    }

    brs_info("amf0 decode play packet success");
    
    return ret;
}

int BrsPlayPacket::get_prefer_cid()
{
    return RTMP_CID_OverStream;
}

int BrsPlayPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int BrsPlayPacket::get_size()
{
    int size = BrsAmf0Size::str(command_name) + BrsAmf0Size::number()
        + BrsAmf0Size::null() + BrsAmf0Size::str(stream_name);
    
    if (start != -2 || duration != -1 || !reset) {
        size += BrsAmf0Size::number();
    }
    
    if (duration != -1 || !reset) {
        size += BrsAmf0Size::number();
    }
    
    if (!reset) {
        size += BrsAmf0Size::boolean();
    }
    
    return size;
}

int BrsPlayPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_name success.");
    
    if ((ret = brs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode transaction_id success.");
    
    if ((ret = brs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        brs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_object success.");
    
    if ((ret = brs_amf0_write_string(stream, stream_name)) != ERROR_SUCCESS) {
        brs_error("encode stream_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode stream_name success.");
    
    if ((start != -2 || duration != -1 || !reset) && (ret = brs_amf0_write_number(stream, start)) != ERROR_SUCCESS) {
        brs_error("encode start failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode start success.");
    
    if ((duration != -1 || !reset) && (ret = brs_amf0_write_number(stream, duration)) != ERROR_SUCCESS) {
        brs_error("encode duration failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode duration success.");
    
    if (!reset && (ret = brs_amf0_write_boolean(stream, reset)) != ERROR_SUCCESS) {
        brs_error("encode reset failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode reset success.");
    
    brs_info("encode play request packet success.");
    
    return ret;
}

BrsPlayResPacket::BrsPlayResPacket()
{
    command_name = RTMP_AMF0_COMMAND_RESULT;
    transaction_id = 0;
    command_object = BrsAmf0Any::null();
    desc = BrsAmf0Any::object();
}

BrsPlayResPacket::~BrsPlayResPacket()
{
    SafeDelete(command_object);
    SafeDelete(desc);
}

int BrsPlayResPacket::get_prefer_cid()
{
    return RTMP_CID_OverStream;
}

int BrsPlayResPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int BrsPlayResPacket::get_size()
{
    return BrsAmf0Size::str(command_name) + BrsAmf0Size::number()
        + BrsAmf0Size::null() + BrsAmf0Size::object(desc);
}

int BrsPlayResPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_name success.");
    
    if ((ret = brs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode transaction_id success.");
    
    if ((ret = brs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        brs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_object success.");
    
    if ((ret = desc->write(stream)) != ERROR_SUCCESS) {
        brs_error("encode desc failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode desc success.");
    
    
    brs_info("encode play response packet success.");
    
    return ret;
}

BrsOnBWDonePacket::BrsOnBWDonePacket()
{
    command_name = RTMP_AMF0_COMMAND_ON_BW_DONE;
    transaction_id = 0;
    args = BrsAmf0Any::null();
}

BrsOnBWDonePacket::~BrsOnBWDonePacket()
{
    SafeDelete(args);
}

int BrsOnBWDonePacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int BrsOnBWDonePacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int BrsOnBWDonePacket::get_size()
{
    return BrsAmf0Size::str(command_name) + BrsAmf0Size::number()
        + BrsAmf0Size::null();
}

int BrsOnBWDonePacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_name success.");
    
    if ((ret = brs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode transaction_id success.");
    
    if ((ret = brs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        brs_error("encode args failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode args success.");
    
    brs_info("encode onBWDone packet success.");
    
    return ret;
}

BrsOnStatusCallPacket::BrsOnStatusCallPacket()
{
    command_name = RTMP_AMF0_COMMAND_ON_STATUS;
    transaction_id = 0;
    args = BrsAmf0Any::null();
    data = BrsAmf0Any::object();
}

BrsOnStatusCallPacket::~BrsOnStatusCallPacket()
{
    SafeDelete(args);
    SafeDelete(data);
}

int BrsOnStatusCallPacket::get_prefer_cid()
{
    return RTMP_CID_OverStream;
}

int BrsOnStatusCallPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int BrsOnStatusCallPacket::get_size()
{
    return BrsAmf0Size::str(command_name) + BrsAmf0Size::number()
        + BrsAmf0Size::null() + BrsAmf0Size::object(data);
}

int BrsOnStatusCallPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_name success.");
    
    if ((ret = brs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode transaction_id success.");
    
    if ((ret = brs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        brs_error("encode args failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode args success.");;
    
    if ((ret = data->write(stream)) != ERROR_SUCCESS) {
        brs_error("encode data failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode data success.");
    
    brs_info("encode onStatus(Call) packet success.");
    
    return ret;
}

BrsBandwidthPacket::BrsBandwidthPacket()
{
    command_name = RTMP_AMF0_COMMAND_ON_STATUS;
    transaction_id = 0;
    args = BrsAmf0Any::null();
    data = BrsAmf0Any::object();
}

BrsBandwidthPacket::~BrsBandwidthPacket()
{
    SafeDelete(args);
    SafeDelete(data);
}

int BrsBandwidthPacket::decode(BRSStream *stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = brs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("amf0 decode bwtc command_name failed. ret=%d", ret);
        return ret;
    }

    if ((ret = brs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("amf0 decode bwtc transaction_id failed. ret=%d", ret);
        return ret;
    }

    if ((ret = brs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        brs_error("amf0 decode bwtc command_object failed. ret=%d", ret);
        return ret;
    }
    
    // @remark, for bandwidth test, ignore the data field.
    // only decode the stop-play, start-publish and finish packet.
    if (is_stop_play() || is_start_publish() || is_finish()) {
        if ((ret = data->read(stream)) != ERROR_SUCCESS) {
            brs_error("amf0 decode bwtc command_object failed. ret=%d", ret);
            return ret;
        }
    }

    brs_info("decode BrsBandwidthPacket success.");

    return ret;
}

int BrsBandwidthPacket::get_prefer_cid()
{
    return RTMP_CID_OverStream;
}

int BrsBandwidthPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int BrsBandwidthPacket::get_size()
{
    return BrsAmf0Size::str(command_name) + BrsAmf0Size::number()
        + BrsAmf0Size::null() + BrsAmf0Size::object(data);
}

int BrsBandwidthPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_name success.");
    
    if ((ret = brs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        brs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode transaction_id success.");
    
    if ((ret = brs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        brs_error("encode args failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode args success.");;
    
    if ((ret = data->write(stream)) != ERROR_SUCCESS) {
        brs_error("encode data failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode data success.");
    
    brs_info("encode onStatus(Call) packet success.");
    
    return ret;
}

bool BrsBandwidthPacket::is_start_play()
{
    return command_name == BRS_BW_CHECK_START_PLAY;
}

bool BrsBandwidthPacket::is_starting_play()
{
    return command_name == BRS_BW_CHECK_STARTING_PLAY;
}

bool BrsBandwidthPacket::is_stop_play()
{
    return command_name == BRS_BW_CHECK_STOP_PLAY;
}

bool BrsBandwidthPacket::is_stopped_play()
{
    return command_name == BRS_BW_CHECK_STOPPED_PLAY;
}

bool BrsBandwidthPacket::is_start_publish()
{
    return command_name == BRS_BW_CHECK_START_PUBLISH;
}

bool BrsBandwidthPacket::is_starting_publish()
{
    return command_name == BRS_BW_CHECK_STARTING_PUBLISH;
}

bool BrsBandwidthPacket::is_stop_publish()
{
    return command_name == BRS_BW_CHECK_STOP_PUBLISH;
}

bool BrsBandwidthPacket::is_stopped_publish()
{
    return command_name == BRS_BW_CHECK_STOPPED_PUBLISH;
}

bool BrsBandwidthPacket::is_finish()
{
    return command_name == BRS_BW_CHECK_FINISHED;
}

bool BrsBandwidthPacket::is_final()
{
    return command_name == BRS_BW_CHECK_FINAL;
}

BrsBandwidthPacket* BrsBandwidthPacket::create_start_play()
{
    BrsBandwidthPacket* pkt = new BrsBandwidthPacket();
    return pkt->set_command(BRS_BW_CHECK_START_PLAY);
}

BrsBandwidthPacket* BrsBandwidthPacket::create_starting_play()
{
    BrsBandwidthPacket* pkt = new BrsBandwidthPacket();
    return pkt->set_command(BRS_BW_CHECK_STARTING_PLAY);
}

BrsBandwidthPacket* BrsBandwidthPacket::create_playing()
{
    BrsBandwidthPacket* pkt = new BrsBandwidthPacket();
    return pkt->set_command(BRS_BW_CHECK_PLAYING);
}

BrsBandwidthPacket* BrsBandwidthPacket::create_stop_play()
{
    BrsBandwidthPacket* pkt = new BrsBandwidthPacket();
    return pkt->set_command(BRS_BW_CHECK_STOP_PLAY);
}

BrsBandwidthPacket* BrsBandwidthPacket::create_stopped_play()
{
    BrsBandwidthPacket* pkt = new BrsBandwidthPacket();
    return pkt->set_command(BRS_BW_CHECK_STOPPED_PLAY);
}

BrsBandwidthPacket* BrsBandwidthPacket::create_start_publish()
{
    BrsBandwidthPacket* pkt = new BrsBandwidthPacket();
    return pkt->set_command(BRS_BW_CHECK_START_PUBLISH);
}

BrsBandwidthPacket* BrsBandwidthPacket::create_starting_publish()
{
    BrsBandwidthPacket* pkt = new BrsBandwidthPacket();
    return pkt->set_command(BRS_BW_CHECK_STARTING_PUBLISH);
}

BrsBandwidthPacket* BrsBandwidthPacket::create_publishing()
{
    BrsBandwidthPacket* pkt = new BrsBandwidthPacket();
    return pkt->set_command(BRS_BW_CHECK_PUBLISHING);
}

BrsBandwidthPacket* BrsBandwidthPacket::create_stop_publish()
{
    BrsBandwidthPacket* pkt = new BrsBandwidthPacket();
    return pkt->set_command(BRS_BW_CHECK_STOP_PUBLISH);
}

BrsBandwidthPacket* BrsBandwidthPacket::create_stopped_publish()
{
    BrsBandwidthPacket* pkt = new BrsBandwidthPacket();
    return pkt->set_command(BRS_BW_CHECK_STOPPED_PUBLISH);
}

BrsBandwidthPacket* BrsBandwidthPacket::create_finish()
{
    BrsBandwidthPacket* pkt = new BrsBandwidthPacket();
    return pkt->set_command(BRS_BW_CHECK_FINISHED);
}

BrsBandwidthPacket* BrsBandwidthPacket::create_final()
{
    BrsBandwidthPacket* pkt = new BrsBandwidthPacket();
    return pkt->set_command(BRS_BW_CHECK_FINAL);
}

BrsBandwidthPacket* BrsBandwidthPacket::set_command(string command)
{
    command_name = command;
    
    return this;
}

BrsOnStatusDataPacket::BrsOnStatusDataPacket()
{
    command_name = RTMP_AMF0_COMMAND_ON_STATUS;
    data = BrsAmf0Any::object();
}

BrsOnStatusDataPacket::~BrsOnStatusDataPacket()
{
    SafeDelete(data);
}

int BrsOnStatusDataPacket::get_prefer_cid()
{
    return RTMP_CID_OverStream;
}

int BrsOnStatusDataPacket::get_message_type()
{
    return RTMP_MSG_AMF0DataMessage;
}

int BrsOnStatusDataPacket::get_size()
{
    return BrsAmf0Size::str(command_name) + BrsAmf0Size::object(data);
}

int BrsOnStatusDataPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_name success.");
    
    if ((ret = data->write(stream)) != ERROR_SUCCESS) {
        brs_error("encode data failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode data success.");
    
    brs_info("encode onStatus(Data) packet success.");
    
    return ret;
}

BrsSampleAccessPacket::BrsSampleAccessPacket()
{
    command_name = RTMP_AMF0_DATA_SAMPLE_ACCESS;
    video_sample_access = false;
    audio_sample_access = false;
}

BrsSampleAccessPacket::~BrsSampleAccessPacket()
{
}

int BrsSampleAccessPacket::get_prefer_cid()
{
    return RTMP_CID_OverStream;
}

int BrsSampleAccessPacket::get_message_type()
{
    return RTMP_MSG_AMF0DataMessage;
}

int BrsSampleAccessPacket::get_size()
{
    return BrsAmf0Size::str(command_name)
        + BrsAmf0Size::boolean() + BrsAmf0Size::boolean();
}

int BrsSampleAccessPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        brs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode command_name success.");
    
    if ((ret = brs_amf0_write_boolean(stream, video_sample_access)) != ERROR_SUCCESS) {
        brs_error("encode video_sample_access failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode video_sample_access success.");
    
    if ((ret = brs_amf0_write_boolean(stream, audio_sample_access)) != ERROR_SUCCESS) {
        brs_error("encode audio_sample_access failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode audio_sample_access success.");;
    
    brs_info("encode |RtmpSampleAccess packet success.");
    
    return ret;
}

BrsOnMetaDataPacket::BrsOnMetaDataPacket()
{
    name = BRS_CONSTS_RTMP_ON_METADATA;
    metadata = BrsAmf0Any::object();
}

BrsOnMetaDataPacket::~BrsOnMetaDataPacket()
{
    SafeDelete(metadata);
}

int BrsOnMetaDataPacket::decode(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_read_string(stream, name)) != ERROR_SUCCESS) {
        brs_error("decode metadata name failed. ret=%d", ret);
        return ret;
    }

    // ignore the @setDataFrame
    if (name == BRS_CONSTS_RTMP_SET_DATAFRAME) {
        if ((ret = brs_amf0_read_string(stream, name)) != ERROR_SUCCESS) {
            brs_error("decode metadata name failed. ret=%d", ret);
            return ret;
        }
    }
    
    brs_verbose("decode metadata name success. name=%s", name.c_str());
    
    // the metadata maybe object or ecma array
    BrsAmf0Any* any = NULL;
    if ((ret = brs_amf0_read_any(stream, &any)) != ERROR_SUCCESS) {
        brs_error("decode metadata metadata failed. ret=%d", ret);
        return ret;
    }
    
    assert(any);
    if (any->is_object()) {
        SafeDelete(metadata);
        metadata = any->to_object();
        brs_info("decode metadata object success");
        return ret;
    }
    
    BrsAutoFreeE(BrsAmf0Any, any);
    
    if (any->is_ecma_array()) {
        BrsAmf0EcmaArray* arr = any->to_ecma_array();
    
        // if ecma array, copy to object.
        for (int i = 0; i < arr->count(); i++) {
            metadata->set(arr->key_at(i), arr->value_at(i)->copy());
        }
        
        brs_info("decode metadata array success");
    }
    
    return ret;
}

int BrsOnMetaDataPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection2;
}

int BrsOnMetaDataPacket::get_message_type()
{
    return RTMP_MSG_AMF0DataMessage;
}

int BrsOnMetaDataPacket::get_size()
{
    return BrsAmf0Size::str(name) + BrsAmf0Size::object(metadata);
}

int BrsOnMetaDataPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if ((ret = brs_amf0_write_string(stream, name)) != ERROR_SUCCESS) {
        brs_error("encode name failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode name success.");
    
    if ((ret = metadata->write(stream)) != ERROR_SUCCESS) {
        brs_error("encode metadata failed. ret=%d", ret);
        return ret;
    }
    brs_verbose("encode metadata success.");
    
    brs_info("encode onMetaData packet success.");
    return ret;
}

BrsSetWindowAckSizePacket::BrsSetWindowAckSizePacket()
{
    ackowledgement_window_size = 0;
}

BrsSetWindowAckSizePacket::~BrsSetWindowAckSizePacket()
{
}

int BrsSetWindowAckSizePacket::decode(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if (!stream->require(4)) {
        ret = ERROR_RTMP_MESSAGE_DECODE;
        brs_error("decode ack window size failed. ret=%d", ret);
        return ret;
    }
    
    ackowledgement_window_size = stream->read_4bytes();
    brs_info("decode ack window size success");
    
    return ret;
}

int BrsSetWindowAckSizePacket::get_prefer_cid()
{
    return RTMP_CID_ProtocolControl;
}

int BrsSetWindowAckSizePacket::get_message_type()
{
    return RTMP_MSG_WindowAcknowledgementSize;
}

int BrsSetWindowAckSizePacket::get_size()
{
    return 4;
}

int BrsSetWindowAckSizePacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if (!stream->require(4)) {
        ret = ERROR_RTMP_MESSAGE_ENCODE;
        brs_error("encode ack size packet failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_4bytes(ackowledgement_window_size);
    
    brs_verbose("encode ack size packet "
        "success. ack_size=%d", ackowledgement_window_size);
    
    return ret;
}

BrsAcknowledgementPacket::BrsAcknowledgementPacket()
{
    sequence_number = 0;
}

BrsAcknowledgementPacket::~BrsAcknowledgementPacket()
{
}

int BrsAcknowledgementPacket::get_prefer_cid()
{
    return RTMP_CID_ProtocolControl;
}

int BrsAcknowledgementPacket::get_message_type()
{
    return RTMP_MSG_Acknowledgement;
}

int BrsAcknowledgementPacket::get_size()
{
    return 4;
}

int BrsAcknowledgementPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if (!stream->require(4)) {
        ret = ERROR_RTMP_MESSAGE_ENCODE;
        brs_error("encode acknowledgement packet failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_4bytes(sequence_number);
    
    brs_verbose("encode acknowledgement packet "
        "success. sequence_number=%d", sequence_number);
    
    return ret;
}

BrsSetChunkSizePacket::BrsSetChunkSizePacket()
{
    chunk_size = BRS_CONSTS_RTMP_PROTOCOL_CHUNK_SIZE;
}

BrsSetChunkSizePacket::~BrsSetChunkSizePacket()
{
}

int BrsSetChunkSizePacket::decode(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if (!stream->require(4)) {
        ret = ERROR_RTMP_MESSAGE_DECODE;
        brs_error("decode chunk size failed. ret=%d", ret);
        return ret;
    }
    
    chunk_size = stream->read_4bytes();
    brs_info("decode chunk size success. chunk_size=%d", chunk_size);
    
    return ret;
}

int BrsSetChunkSizePacket::get_prefer_cid()
{
    return RTMP_CID_ProtocolControl;
}

int BrsSetChunkSizePacket::get_message_type()
{
    return RTMP_MSG_SetChunkSize;
}

int BrsSetChunkSizePacket::get_size()
{
    return 4;
}

int BrsSetChunkSizePacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if (!stream->require(4)) {
        ret = ERROR_RTMP_MESSAGE_ENCODE;
        brs_error("encode chunk packet failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_4bytes(chunk_size);
    
    brs_verbose("encode chunk packet success. ack_size=%d", chunk_size);
    
    return ret;
}

BrsSetPeerBandwidthPacket::BrsSetPeerBandwidthPacket()
{
    bandwidth = 0;
    type = BrsPeerBandwidthDynamic;
}

BrsSetPeerBandwidthPacket::~BrsSetPeerBandwidthPacket()
{
}

int BrsSetPeerBandwidthPacket::get_prefer_cid()
{
    return RTMP_CID_ProtocolControl;
}

int BrsSetPeerBandwidthPacket::get_message_type()
{
    return RTMP_MSG_SetPeerBandwidth;
}

int BrsSetPeerBandwidthPacket::get_size()
{
    return 5;
}

int BrsSetPeerBandwidthPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if (!stream->require(5)) {
        ret = ERROR_RTMP_MESSAGE_ENCODE;
        brs_error("encode set bandwidth packet failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_4bytes(bandwidth);
    stream->write_1bytes(type);
    
    brs_verbose("encode set bandwidth packet "
        "success. bandwidth=%d, type=%d", bandwidth, type);
    
    return ret;
}

BrsUserControlPacket::BrsUserControlPacket()
{
    event_type = 0;
    event_data = 0;
    extra_data = 0;
}

BrsUserControlPacket::~BrsUserControlPacket()
{
}

int BrsUserControlPacket::decode(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if (!stream->require(2)) {
        ret = ERROR_RTMP_MESSAGE_DECODE;
        brs_error("decode user control failed. ret=%d", ret);
        return ret;
    }
    
    event_type = stream->read_2bytes();
    
    if (event_type == BrsPCUCFmsEvent0) {
        if (!stream->require(1)) {
            ret = ERROR_RTMP_MESSAGE_DECODE;
            brs_error("decode user control failed. ret=%d", ret);
            return ret;
        }
        event_data = stream->read_1bytes();
    } else {
        if (!stream->require(4)) {
            ret = ERROR_RTMP_MESSAGE_DECODE;
            brs_error("decode user control failed. ret=%d", ret);
            return ret;
        }
        event_data = stream->read_4bytes();
    }
    
    if (event_type == SrcPCUCSetBufferLength) {
        if (!stream->require(4)) {
            ret = ERROR_RTMP_MESSAGE_ENCODE;
            brs_error("decode user control packet failed. ret=%d", ret);
            return ret;
        }
        extra_data = stream->read_4bytes();
    }
    
    brs_info("decode user control success. "
        "event_type=%d, event_data=%d, extra_data=%d", 
        event_type, event_data, extra_data);
    
    return ret;
}

int BrsUserControlPacket::get_prefer_cid()
{
    return RTMP_CID_ProtocolControl;
}

int BrsUserControlPacket::get_message_type()
{
    return RTMP_MSG_UserControlMessage;
}

int BrsUserControlPacket::get_size()
{
    int size = 2;
    
    if (event_type == BrsPCUCFmsEvent0) {
        size += 1;
    } else {
        size += 4;
    }
    
    if (event_type == SrcPCUCSetBufferLength) {
        size += 4;
    }
    
    return size;
}

int BrsUserControlPacket::encode_packet(BRSStream* stream)
{
    int ret = ERROR_SUCCESS;
    
    if (!stream->require(get_size())) {
        ret = ERROR_RTMP_MESSAGE_ENCODE;
        brs_error("encode user control packet failed. ret=%d", ret);
        return ret;
    }
    
    stream->write_2bytes(event_type);
    
    if (event_type == BrsPCUCFmsEvent0) {
        stream->write_1bytes(event_data);
    } else {
        stream->write_4bytes(event_data);
    }

    // when event type is set buffer length,
    // write the extra buffer length.
    if (event_type == SrcPCUCSetBufferLength) {
        stream->write_4bytes(extra_data);
        brs_verbose("user control message, buffer_length=%d", extra_data);
    }
    
    brs_verbose("encode user control packet success. "
        "event_type=%d, event_data=%d", event_type, event_data);
    
    return ret;
}

}