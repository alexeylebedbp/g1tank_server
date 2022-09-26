cmake_minimum_required(VERSION 3.21)
project(G1Tank)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost)
if(Boost_FOUND)
    message("Boost_FOUND")
    include_directories(${Boost_INCLUDE_DIRS})
endif()

include_directories(utils)
include_directories (SYSTEM ${Boost_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIR}/libs/certify/include)

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

add_executable(G1Tank main.cpp Server.cpp Server.h PilotConnection.cpp PilotConnection.h CarConnection.cpp CarConnection.h ConnectionManager.h utils/include_boost_asio.h)
target_link_libraries(G1Tank ${Boost_LIBRARIES} crypto ssl)
