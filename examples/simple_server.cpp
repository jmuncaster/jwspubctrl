#include <jws/json_with_schema.hpp>
#include <zpubctrl/server.hpp>
#include <chrono>
#include <ctime>   // localtime
#include <iostream>
#include <iomanip> // put_time
#include <string>
#include <sstream>
#include <thread>

using namespace std;
using jws::json;

string current_time_and_date()
{
  auto now = chrono::system_clock::now();
  auto in_time_t = chrono::system_clock::to_time_t(now);
  stringstream ss;
  ss << std::put_time(localtime(&in_time_t), "%Y-%m-%d %X");
  return ss.str();
}

// Main server process runs a program loop, publishes feed, and checks for control messages
int main(int argc, char** argv) {

  string time_report_schema_filename = "time_report_schema.json";
  cout << "Load " << time_report_schema_filename << endl;
  auto time_report_validator = jws::load_validator(time_report_schema_filename);

  string time_server_ctrl_schema_filename = "time_server_ctrl_schema.json";
  cout << "Load " << time_server_ctrl_schema_filename << endl;
  auto time_server_ctrl_validator = jws::load_validator(time_server_ctrl_schema_filename);

  zpubctrl::Server server;
  cout << "Start server" << endl;
  cout << "  * publish port: " << zpubctrl::default_data_port << endl;;
  cout << "  * control port: " << zpubctrl::default_ctrl_port << endl;;

  while (true) {

    // Check for ctrl request to change the text
    int timeout_ms = 1000;
    server.wait_for_request(timeout_ms, [&](const string& request) {
      try {
        auto ctrl_json = json::parse(request);
        time_server_ctrl_validator.validate(ctrl_json);

        //TODO: do something
        //cout << "" << endl;

        json reply_json = {
          {"error", false},
          {"message", "OK"}
        };
        time_server_ctrl_validator.validate(reply_json);
        return reply_json.dump();
      }
      catch (exception& e) {
        try {
          json reply_json = {
            {"error", true},
            {"message", e.what()}
          };
          time_server_ctrl_validator.validate(reply_json);
          return reply_json.dump();
        }
        catch (exception& e) {
          return string("internal server error");
        }
      }
    });

    // Do some 'work'
    json time_report_json = {
      {"datetime", current_time_and_date()}
    };
    time_report_validator.validate(time_report_json);

    // Publish
    server.publish_data(time_report_json.dump());
  }
}

