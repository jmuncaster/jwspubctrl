cmake_minimum_required(VERSION 3.2.2)

project(jwspubctrl)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(jws)
add_subdirectory(zpubctrl)

add_library(jwspubctrl
  jwspubctrl/server.cpp
  jwspubctrl/ctrl_client.cpp
  jwspubctrl/sub_client.cpp)
target_include_directories(jwspubctrl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(jwspubctrl jws zpubctrl)

# Examples
option(BUILD_EXAMPLES "Build examples" OFF)
if (BUILD_EXAMPLES)
  add_executable(time_server examples/time_server.cpp)
  target_link_libraries(time_server jwspubctrl)

  add_executable(time_client examples/time_client.cpp)
  target_link_libraries(time_client jwspubctrl pthread)
endif()

