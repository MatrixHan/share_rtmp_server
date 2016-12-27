cmake_minimum_required(VERSION 2.6)
project(share_rtmp_server CXX)

set(CXX_FLAGS -g -Wall)
set(CMAKE_CXX_COMPILE "g++")
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/common
${PROJECT_SOURCE_DIR}/core
${PROJECT_SOURCE_DIR}/kernel
${PROJECT_SOURCE_DIR}/network
${PROJECT_SOURCE_DIR}/protocol
${PROJECT_SOURCE_DIR}/app
${PROJECT_SOURCE_DIR}/brs
)

INCLUDE_DIRECTORIES(objs 
    objs/hp  objs/openssl/include   objs/coroutine/include  
    common   core  kernel  network   protocol  app)

set(SOURCE_FILES brs/share_rtmp_server.cpp )
AUX_SOURCE_DIRECTORY(common SOURCE_FILES)
AUX_SOURCE_DIRECTORY(core SOURCE_FILES)
AUX_SOURCE_DIRECTORY(kernel SOURCE_FILES)
AUX_SOURCE_DIRECTORY(network SOURCE_FILES)
AUX_SOURCE_DIRECTORY(protocol SOURCE_FILES)
AUX_SOURCE_DIRECTORY(app SOURCE_FILES)

ADD_DEFINITIONS("-g -O0")

ADD_EXECUTABLE(brs   ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(brs dl )
TARGET_LINK_LIBRARIES(brs ${PROJECT_SOURCE_DIR}/objs/coroutine/lib/coroutinelib.a)
TARGET_LINK_LIBRARIES(brs ${PROJECT_SOURCE_DIR}/objs/openssl/lib/libssl.a)
TARGET_LINK_LIBRARIES(brs ${PROJECT_SOURCE_DIR}/objs/openssl/lib/libcrypto.a)
#TARGET_LINK_LIBRARIES(brs ${PROJECT_SOURCE_DIR}/objs/hp/libhttp_parser.a)
TARGET_LINK_LIBRARIES(brs -ldl )

