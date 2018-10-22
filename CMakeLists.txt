cmake_minimum_required(VERSION 3.2.2)

project(jwspubctrl)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(jws)
add_subdirectory(wspubctrl)

add_library(jwspubctrl
  jwspubctrl/server.cpp
  jwspubctrl/ctrl_client.cpp
  jwspubctrl/sub_client.cpp)
target_include_directories(jwspubctrl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(jwspubctrl jws wspubctrl)

# Examples
option(BUILD_EXAMPLES "Build examples" OFF)
if (BUILD_EXAMPLES)
  find_package(Threads)

  add_executable(time_server examples/time_server/time_server.cpp)
  target_link_libraries(time_server jwspubctrl)

  add_executable(time_client examples/time_server/time_client.cpp)
  target_link_libraries(time_client jwspubctrl ${CMAKE_THREAD_LIBS_INIT})

  add_executable(pubstring_server examples/pubstring/pubstring_server.cpp)
  target_link_libraries(pubstring_server jwspubctrl)

  add_executable(pubstring_client examples/pubstring/pubstring_client.cpp)
  target_link_libraries(pubstring_client jwspubctrl ${CMAKE_THREAD_LIBS_INIT})
endif()

