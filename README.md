# WebSockets Publisher and Control Socket with JSON Validation

This library wraps the [wspubctrl](https://github.com/jmuncaster/wspubctrl) library with JSON validation
capabilities. It provides an easy way to:
- Stream JSON data over websocket.
- Accept JSON control messages and reply with status, over websockets.
- Automatically validate incoming and outgoing messages.

The websocket pub/sub and ctrl logic is provided by [wspubctrl](https://github.com/jmuncaster/wspubctrl). The wrappers in this project simply do JSON decoding and validation.

JSON validation is done using [jws](https://github.com/jmuncaster/jws), which is just a thin wrapper
around [nlohmann/json](https://github.com/nlohmann/json) and [pboettch/json-schema-validator](https://github.com/pboettch/json-schema-validator).

The networking backend is based on [Simple-Websocket-Server](https://github.com/eidheim/Simple-WebSocket-Server), a websockets client/server wrapper based on Boost.ASIO.


## Getting Started

## Build

These instructions assume you are using [cmake](cmake.org).

In your CMakeLists.txt, add:
```CMake
add_subdirectory(path/to/jwspubctrl jwspubctrl)
add_executable(myserver myserver.cpp)
target_link_libraries(myserver jwspubctrl)
```

### Windows

See [wspubctrl](https://github.com/jmuncaster/wspubctrl) for required dependencies.

```bash
$ mkdir build && cd build
$ cmake -G "Visual Studio 14 2015 Win64" -D BOOST_ROOT='C:\path\to\boost' -D BUILD_EXAMPLES=ON -D BUILD_TESTS=ON ..
$ cmake --build .
```


### Examples

To build examples, configure with
```bash
cmake -D BUILD_EXAMPLES=ON
```

Run examples:

Server:
```bash
cd build
./time_server ../examples/time_server/schemas/publish.json ../examples/time_server/schemas/ctrl_request.json ../examples/time_server/schemas/ctrl_reply.json
```

CLI Client:
```bash
cd build
./time_client localhost:5554 /ctrl /pub ../examples/time_server/schemas/publish.json ../examples/time_server/schemas/ctrl_request.json ../examples/time_server/schemas/ctrl_reply.json
```

HTML/JS client:
Open `examples/time_client.html` in a web browser.
