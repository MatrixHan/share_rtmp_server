cmake_minimum_required(VERSION 2.6)
project(share_rtmp_server)

add_executable(share_rtmp_server main.cpp)

install(TARGETS share_rtmp_server RUNTIME DESTINATION bin)

add_subdirectory(core)
add_subdirectory(network)
add_subdirectory(app)
add_subdirectory(common)