cmake_minimum_required(VERSION 3.2.2)

project(cortex_comms)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(jws)
add_subdirectory(zpubctrl)

add_library(zjwspubctrl zjwspubctrl/server.cpp)
target_include_directories(zjwspubctrl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(zjwspubctrl jws zpubctrl)

add_executable(time_server examples/simple_server.cpp)
target_link_libraries(time_server jws zpubctrl)
add_executable(time_server2 examples/simple_server2.cpp)
target_link_libraries(time_server2 zjwspubctrl)

add_executable(time_client examples/simple_client.cpp)
target_link_libraries(time_client jws zpubctrl pthread)
