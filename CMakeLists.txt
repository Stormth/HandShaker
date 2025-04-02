cmake_minimum_required(VERSION 3.26)
project(HandShaker_rebuild)

set(CMAKE_CXX_STANDARD 17)

# 包含头文件路径
include_directories(include)

# 通用源文件列表
set(SOURCES
        src/TCPPacket.cpp
        src/TCPConnection.cpp
        src/TCPBuffer.cpp
        src/SocketWrapper.cpp
        src/AppCommunicator.cpp
)

# 添加客户端
add_executable(client
        client/main.cpp
        ${SOURCES}
)

# 添加服务端
add_executable(server
        server/main.cpp
        ${SOURCES}
)

# Windows 兼容：链接 WinSock2
if(WIN32)
    target_link_libraries(client ws2_32)
    target_link_libraries(server ws2_32)
endif()
