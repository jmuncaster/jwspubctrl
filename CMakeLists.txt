cmake_minimum_required(VERSION 3.2.2)

project(zjwspubctrl)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(jws)
add_subdirectory(zpubctrl)

add_library(zjwspubctrl
  zjwspubctrl/server.cpp
  zjwspubctrl/ctrl_client.cpp
  zjwspubctrl/sub_client.cpp)
target_include_directories(zjwspubctrl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(zjwspubctrl jws zpubctrl)

# Examples
option(BUILD_ZJWSPUBCTRL_EXAMPLES "Build examples" OFF)
if (BUILD_ZJWSPUBCTRL_EXAMPLES)
  add_executable(time_server examples/time_server.cpp)
  target_link_libraries(time_server zjwspubctrl)

  add_executable(time_client examples/time_client.cpp)
  target_link_libraries(time_client zjwspubctrl pthread)
endif()

