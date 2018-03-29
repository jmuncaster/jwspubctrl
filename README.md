# WebSockets Publisher and Control Socket with JSON Validation

This library wraps the [wspubctrl](https://github.com/jmuncaster/wspubctrl) library with JSON validation
capabilities. It is meant to provide an easy way to stream JSON data and accept JSON control messages
with full validation of all messages.

JSON validation is done using [jws](https://github.com/jmuncaster/jws), which is just a thin wrapper
around [nlohmann/json](https://github.com/nlohmann/json) and [pboettch/json-schema-validator](https://github.com/pboettch/json-schema-validator).

The networking backend is based on [Simple-Websocket-Server](https://github.com/eidheim/Simple-WebSocket-Server).


## Getting Started

### Prerequisites

Add a snapshot of this repository to your source code or add it as a git submodule.

### Build

These instructions assume you are using [cmake](cmake.org) and you have installed ZeroMQ somewhere in your PATH.

In your CMakeLists.txt, add:
```CMake
add_subdirectory(path/to/jwspubctrl jwspubctrl)
add_executable(myserver myserver.cpp)
target_link_libraries(myserver jwspubctrl)
```

### Examples

To build examples, configure with
```bash
cmake -D BUILD_jwsPUBCTRL_EXAMPLES=ON
```

Server: See examples/time_server.cpp
Client: See examples/time_client.cpp
HTML/JS Client: See examples/time_client.html
