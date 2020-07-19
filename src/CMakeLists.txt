cmake_minimum_required(VERSION 3.15)
project(LwipApiMock)

set(CMAKE_CXX_STANDARD 14)

add_library(lwip_api_fmock
lwip_api_mock.cpp
tcp.cpp
udp.cpp
pbuf.cpp
lwip.cpp
tcp_asio.cpp
io_service.cpp)
target_include_directories(lwip_api_fmock PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/internal_include)

target_include_directories(lwip_api_fmock PUBLIC
${CMAKE_CURRENT_LIST_DIR}/include 
${CMAKE_CURRENT_LIST_DIR}/lwip-2.1.2/src/include)

find_package(Boost 1.67.0 REQUIRED system thread regex)
find_package(Threads REQUIRED)

target_link_libraries(lwip_api_fmock PRIVATE Threads::Threads ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY})

