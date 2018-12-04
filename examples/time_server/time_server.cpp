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

  if (argc != 4) {
    cout << "usage: time_server <publish_schema.json> <ctrl_request.json> <ctrl_reply.json>" << endl;
    return 1;
  }

  cout << "Load " << argv[1] << endl;
  auto time_report_schema = jws::load_json(argv[1]);

  cout << "Load " << argv[2] << endl;
  auto time_server_ctrl_schema = jws::load_json(argv[2]);

  cout << "Load " << argv[3] << endl;
  auto time_server_ctrl_reply_schema = jws::load_json(argv[3]);

  jwspubctrl::Server server(time_server_ctrl_schema, time_server_ctrl_reply_schema);
  server.add_publish_endpoint("/pub", time_report_schema);
  cout << "Start server on port " << wspubctrl::default_port << endl;
  cout << "  * publish: /pub" << endl;
  cout << "  * control: /ctrl" << endl;

  string format = "%Y-%m-%d %X";

  server.start();

  while (true) {

    // Check for ctrl request to change the text
    int timeout_ms = 1000;
    server.handle_request(timeout_ms, [&](const json& request_json) {

        // Handle format request.
        //cout << "Change format: " << format << endl;
        format = request_json["format"];

        return jws::json{
          {"error", false},
          {"message", "OK"}
        };
      },
      [&](const exception& e) {
        //cout << "Bad request: " << e.what() << endl;
        return json{
            {"error", true},
            {"message", e.what()}
          };
      });

    // Do some 'work' and publish
    server.send_json("/pub", {{"datetime", current_time_and_date(format)}});
  }
}

