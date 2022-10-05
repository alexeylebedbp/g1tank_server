cmake_minimum_required(VERSION 3.21)
project(G1Tank)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

find_package(Boost)
if(Boost_FOUND)
    message("Boost_FOUND")
    include_directories(${Boost_INCLUDE_DIRS})
    message("Boost_INCLUDE_DIR" ${Boost_INCLUDE_DIR})
endif()

include_directories(utils)
include_directories (SYSTEM ${Boost_INCLUDE_DIR})
include_directories(/usr/local/boost_1_79_0/libs/certify/include)

#Linking OpenSSL. Works for MacOS, should be updated for Linux/Windows
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm64")
    message("ARM64")
    link_directories(/opt/homebrew/lib)
else()
    message("X86")
    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl@3/include)
    set(OPENSSL_LIB_DIR /usr/local/opt/openssl@3/lib)
    include_directories(${OPENSSL_ROOT_DIR})
    link_directories(${OPENSSL_LIB_DIR})
endif()


if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
endif()

include_directories(./)
include_directories(utils/network)
include_directories(third_party/json/include)
add_executable(G1Tank main.cpp CarSession.h CarSession.cpp Server.cpp PilotSession.h PilotSession.cpp utils/Event.h utils/Event.cpp Server.h utils/ConnectionManager.h utils/include_boost_asio.h utils/network/Websocket.h utils/network/Websocket.cpp)
target_link_libraries(G1Tank ${Boost_LIBRARIES} crypto ssl)
