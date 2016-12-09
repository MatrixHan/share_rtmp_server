cmake_minimum_required(VERSION 2.6)
project(share_rtmp_server CXX)

INCLUDE_DIRECTORIES(objs 
    objs/hp objs/openssl/include objs/libco/include
    common core network  protocol app)

set(SOURCE_FILES brs/share_rtmp_server.cpp)
AUX_SOURCE_DIRECTORY(common SOURCE_FILES)
AUX_SOURCE_DIRECTORY(core SOURCE_FILES)
AUX_SOURCE_DIRECTORY(network SOURCE_FILES)
AUX_SOURCE_DIRECTORY(protocol SOURCE_FILES)
AUX_SOURCE_DIRECTORY(app SOURCE_FILES)

ADD_DEFINITIONS("-g -O0")

ADD_EXECUTABLE(share_rtmp_server ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(share_rtmp_server dl pthread)
TARGET_LINK_LIBRARIES(share_rtmp_server ${PROJECT_SOURCE_DIR}/objs/libco/lib/libcolib.a)
TARGET_LINK_LIBRARIES(share_rtmp_server ${PROJECT_SOURCE_DIR}/objs/openssl/lib/libssl.a)
TARGET_LINK_LIBRARIES(share_rtmp_server ${PROJECT_SOURCE_DIR}/objs/openssl/lib/libcrypto.a)
TARGET_LINK_LIBRARIES(share_rtmp_server ${PROJECT_SOURCE_DIR}/objs/hp/libhttp_parser.a)
TARGET_LINK_LIBRARIES(share_rtmp_server -ldl -lpthread)

IF(NOT EXISTS ${PROJECT_SOURCE_DIR}/objs/libco/lib/libcolib.a)
    MESSAGE("srs_libs not found")
    EXEC_PROGRAM("cd .. && ./configure")
ELSE(NOT EXISTS ${PROJECT_SOURCE_DIR}/objs/libco/lib/libcolib.a)
    MESSAGE("srs_libs is ok")
ENDIF(NOT EXISTS ${PROJECT_SOURCE_DIR}/objs/libco/lib/libcolib.a)
