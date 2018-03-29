#include <jws/json_with_schema.hpp>
#include <jwspubctrl/client.hpp>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;
using jws::json;

const int timeout_ms = 5000;

int main(int argc, char* argv[]) {

  if (argc != 4) {
    cout << "usage: time_client <publish_schema.json> <ctrl_request.json> <ctrl_reply.json>" << endl;
    return 1;
  }

  cout << "Load " << argv[1] << endl;
  auto time_report_schema = jws::load_json(argv[1]);

  cout << "Load " << argv[2] << endl;
  auto time_server_ctrl_schema = jws::load_json(argv[2]);

  cout << "Load " << argv[3] << endl;
  auto time_server_ctrl_reply_schema = jws::load_json(argv[3]);

  cout << "Start client. Press ENTER to cycle through texts..." << endl;

  // Sub thread continuously reports the stream on the same line
  atomic<bool> quit(false);
  thread sub_thread([&]() {
    try {
      jwspubctrl::SubClient sub_client(time_report_schema);
      while (!quit) {
        try {
          auto data_json = sub_client.wait_for_data(timeout_ms);
          cout << "\r" << data_json["datetime"] << "\e[K" << flush;
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
    jwspubctrl::CtrlClient ctrl_client(time_server_ctrl_schema, time_server_ctrl_reply_schema);
    vector<string> texts = {"%Y-%m-%d", "%X", "%Y-%m-%d %X", "%Y-%m-%d", ""};
    for (size_t i = 0; !quit /*&& i < texts.size()*/; ++i) {
      try {
        json format_request = {
          {"format", texts[i % texts.size()]}
        };
        if (i % texts.size() == texts.size() -1) {
          cout << "sending bad request" << endl;
          format_request = "bad request";
        }

        auto status_json = ctrl_client.request(format_request, timeout_ms);
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

