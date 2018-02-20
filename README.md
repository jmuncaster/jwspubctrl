# ZeroMQ Publisher and Control Socket with JSON Validation

This library wraps the [zpubctrl](https://github.com/jmuncaster/zpubctrl) library with JSON validation
capabilities. It is meant to provide an easy way to stream JSON data and accept JSON control messages
with full validation of all messages.

JSON validation is done using [jws](https://github.com/jmuncaster/jws), which is just a thin wrapper
around [nlohmann/json](https://github.com/nlohmann/json) and [pboettch/json-schema-validator](https://github.com/pboettch/json-schema-validator).

The networking backend is implemented in [zeromq](http://zeromq.org) with a [C++ wrapper](https://github.com/zeromq/cppzmq).


## Getting Started

### Prerequisites

Add a snapshot of this repository to your source code or add it as a git submodule.

### Build

These instructions assume you are using [cmake](cmake.org) and you have installed ZeroMQ somewhere in your PATH.

In your CMakeLists.txt, add:
```CMake
add_subdirectory(path/to/zjwspubctrl zjwspubctrl)
add_executable(myserver myserver.cpp)
target_link_libraries(myserver zjwspubctrl)
```

### Example

Server:
```C++
#include <jws/json_with_schema.hpp>
#include <zjwspubctrl/server.hpp>
#include <cstdlib>
#include <iostream>
#include <thread>

using namespace std;

// Main server process runs a program loop, publishes feed, and checks for control messages
int main(int argc, char** argv) {

  auto pub_schema = jws::load_json("pub_schema.json");
  auto ctrl_request_schema = jws::load_json("ctrl_request_schema.json");
  auto ctrl_reply_schema = jws::load_json("ctrl_reply_schema.json");

  zjwspubctrl::Server server(pub_schema, ctrl_request_schema, ctrl_reply_schema);
  cout << "Start server" << endl;
  cout << "  * publish port: " << zpubctrl::default_data_port << endl;;
  cout << "  * control port: " << zpubctrl::default_ctrl_port << endl;;

  string text = "Hello World!";

  int iter = 0;
  while (true) {
    // Check for ctrl request to change the text
    server.wait_for_request(0, [&](const jws::json& json) {
        // JSON guarenteed to conform to ctrl_request.json
        text = json["text"];
        return json{{"error": false, "message": "OK"}};
      },
      [&](const exception& e) {
        // Error parsing request
        return json{{"error": true, "message": "Bad request"}};
    });

    // Do some 'work' mangling the text and publish
    string mangled_text = text;
    int i = ++iter % text.size();
    auto fn1 = (iter / text.size() % 2 == 0) ? ::toupper : ::tolower;
    auto fn2 = (iter / text.size() % 2 == 1) ? ::toupper : ::tolower;
    transform(text.begin(), text.begin() + i, mangled_text.begin(), fn1);
    transform(text.begin() + i, text.end(), mangled_text.begin() + i, fn2);
    this_thread::sleep_for(chrono::milliseconds(10));

    // Publish
    // JSON will be validated before it goes on the wire.
    server.publish_data(json{{"mangled_test": mangled_text}});
  }
}
```

Client:
```C++
#include <zpubctrl/client.hpp>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

const int timeout_ms = 5000;

int main(int argc, char* argv[]) {

  auto pub_schema = jws::load_json("pub_schema.json");
  auto ctrl_request_schema = jws::load_json("ctrl_request_schema.json");
  auto ctrl_reply_schema = jws::load_json("ctrl_reply_schema.json");

  cout << "Start client. Press ENTER to cycle through texts..." << endl;

  // Sub thread continuously reports the stream on the same line
  atomic<bool> quit(false);
  thread sub_thread([&]() {
    try {
      zjwspubctrl::SubClient sub_client(pub_schema);
      while (!quit) {
        // data is validated against pub_schema
        auto data = sub_client.wait_for_data(timeout_ms);
        cout << "\r" << data << "\e[K" << flush;
      }
    }
    catch (exception& e) {
      // probably a timeout
      cerr << e.what() << endl;
      quit = true; // kill app on next iteration
    }
  });

  cin.get();

  // Main loop cycles through texts in response to user input
  try {
    zpubctrl::CtrlClient ctrl_client;
    vector<string> texts = {"Bonjour!", "Next we will try the empty string", "", "This is the last text"};
    for (size_t i = 0; !quit && i < texts.size(); ++i) {
      // Both the request and the reply are validated.
      auto reply = ctrl_client.request(json{{"text", texts[i % texts.size()]}}, timeout_ms);

      cin.get(); // press enter to cycle texts
      cout << "\e[1A" << flush; // go up a line
    }
  }
  catch (exception& e) {
    // probably a timeout
    cerr << e.what() << endl;
  }

  quit = true;
  sub_thread.join();

  return 0;
}
```

### Build Examples

To build examples, configure with
```bash
cmake -D BUILD_ZJWSPUBCTRL_EXAMPLES=ON
```

Server: See examples/time_server.cpp
Client: See examples/time_client.cpp

