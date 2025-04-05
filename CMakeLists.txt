cmake_minimum_required(VERSION 3.26)
project(HandShaker_rebuild)

set(CMAKE_CXX_STANDARD 17)

# 指定包含头文件目录（Logger.h 等都放在 include/ 目录下）
include_directories(include)

# 公共源文件列表
set(SOURCES
        src/TCPPacket.cpp
        src/TCPConnection.cpp
        src/TCPBuffer.cpp
        src/SocketWrapper.cpp
        src/AppCommunicator.cpp
)

# 构建客户端可执行文件
add_executable(client
        client/main.cpp
        ${SOURCES}
)

# 构建服务端可执行文件
add_executable(server
        server/main.cpp
        ${SOURCES}
)

# Windows 平台下需要链接 ws2_32 库
if(WIN32)
    target_link_libraries(client ws2_32)
    target_link_libraries(server ws2_32)
endif()
