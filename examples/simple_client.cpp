#include <jws/json_with_schema.hpp>
#include <zpubctrl/client.hpp>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;
using jws::json;

const int timeout_ms = 5000;

int main(int argc, char* argv[]) {

  if (argc != 3) {
    cout << "usage: time_server <time_report_schema.json> <time_server_ctrl_schema.json>" << endl;
    return 1;
  }

  cout << "Load " << argv[1] << endl;
  auto time_report_validator = jws::load_validator(argv[1]);

  cout << "Load " << argv[2] << endl;
  auto time_server_ctrl_validator = jws::load_validator(argv[2]);

  cout << "Start client. Press ENTER to cycle through texts..." << endl;

  // Sub thread continuously reports the stream on the same line
  atomic<bool> quit(false);
  thread sub_thread([&]() {
    try {
      zpubctrl::SubClient sub_client;
      while (!quit) {
        auto data = sub_client.wait_for_data(timeout_ms);
        try {
          json time_report_json = json::parse(data);
          time_report_validator.validate(time_report_json);
          cout << "\r" << time_report_json["datetime"] << "\e[K" << flush;
        }
        catch (exception& e) {
          cout << "\n==> error: " << e.what() << endl;
        }
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
    vector<string> texts = {"%Y-%m-%d", "%Y-%m-%d", "%Y-%m-%d", "%Y-%m-%d"};
    for (size_t i = 0; !quit && i < texts.size(); ++i) {
      try {
        json format_request = {
          {"format", texts[i % texts.size()]}
        };
        auto reply = ctrl_client.request(format_request.dump(), timeout_ms);
        auto status_json = json::parse(reply);
        time_server_ctrl_validator.validate(status_json);
        if (status_json["error"]) {
          cout << "\n==> error, server says: " << status_json["message"] << endl;
        }
      }
      catch (exception& e) {
        cout << "\n==> error: " << e.what() << endl;
      }

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
