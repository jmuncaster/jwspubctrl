#include <jwspubctrl/server.hpp>
#include <chrono>
#include <ctime>   // localtime
#include <iostream>
#include <iomanip> // put_time
#include <string>
#include <sstream>
#include <thread>

using namespace std;
using jws::json;

string current_time_and_date(const string& format)
{
  auto now = chrono::system_clock::now();
  auto in_time_t = chrono::system_clock::to_time_t(now);
  stringstream ss;
  ss << std::put_time(localtime(&in_time_t), format.c_str());
  return ss.str();
}

// Main server process runs a program loop, publishes feed, and checks for control messages
int main(int argc, char** argv) {

  if (argc != 3) {
    cout << "example of publishing a string as raw data while using json for ctrl messages" << endl;
    cout << "usage: pubstring <ctrl_request.json> <ctrl_reply.json>" << endl;
    return 1;
  }

  cout << "Load " << argv[1] << endl;
  auto ctrl_request_schema = jws::load_json(argv[1]);

  cout << "Load " << argv[2] << endl;
  auto ctrl_reply_schema = jws::load_json(argv[2]);

  cout << "Start server" << endl;
  jwspubctrl::Server server(ctrl_request_schema, ctrl_reply_schema);
  server.add_publish_endpoint("/pub");
  cout << "  * publish: " << wspubctrl::default_pub_uri << endl;;
  cout << "  * control: " << wspubctrl::default_ctrl_uri << endl;;

  int message_size = 1024;
  std::uint16_t val = 0;

  while (true) {

    // Check for ctrl request to change the text
    int timeout_ms = 0;
    server.handle_request(timeout_ms,
      [&](const json& request_json) {
        // Handle format request.
        //cout << "Change format: " << format << endl;
        auto request_type = request_json["request_type"];
        if (request_type == "set_message_size") {
          message_size = request_json["message_size"];
          return jws::json{
            {"error", false},
            {"message", "OK"},
            {"message_size", message_size},
          };
        }
        else if (request_type == "get_message_size") {
          return jws::json{
            {"error", false},
            {"message", "OK"},
            {"message_size", message_size},
          };
        }
        else {
          return jws::json{
            {"error", true},
            {"message", "Not implemented"},
            {"request_type", request_type},
          };
        }
      },
      [&](const exception& e) {
        //cout << "Bad request: " << e.what() << endl;
        return json{
            {"error", true},
            {"message", e.what()}
          };
      });

    // Do some 'work' and publish
    stringstream ss;
    int bytes_written = 0;
    while (bytes_written < message_size) {
      ss.write(reinterpret_cast<char*>(&val), sizeof(val));
      bytes_written += sizeof(val);
    }
    ++val;

    server.send_string("/pub", ss.str());
  }
}

