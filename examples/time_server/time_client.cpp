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

  if (argc != 7) {
    cout << "usage: time_client <host:port> <ctrl_endpoint> <pub_endpoint> <publish_schema.json> <ctrl_request.json> <ctrl_reply.json>" << endl;
    cout << "example: ./time_client localhost:5554 /ctrl /pub ../examples/time_server/schemas/publish.json ../examples/time_server/schemas/ctrl_request.json ../examples/time_server/schemas/ctrl_reply.json" << endl;
    return 1;
  }

  auto host = (argv[1]);
  auto ctrl_uri = host + string(argv[2]);
  auto pub_uri = host + string(argv[3]);

  cout << "Loading schemas..." << endl;

  cout << "  Load " << argv[4] << endl;
  auto time_report_schema = jws::load_json(argv[4]);
  cout << "  Load " << argv[5] << " (and modify)" << endl;
  auto time_server_ctrl_schema = jws::load_json(argv[5]);
  time_server_ctrl_schema["oneOf"].push_back(R"({ "type": "integer" })"_json); // simualate a schema with a new allowed msg type
  cout << "  Load " << argv[6] << endl;
  auto time_server_ctrl_reply_schema = jws::load_json(argv[6]);

  cout << "Connecting to " << ctrl_uri << "... " << flush;
  jwspubctrl::CtrlClient ctrl_client(ctrl_uri, time_server_ctrl_schema, time_server_ctrl_reply_schema);
  ctrl_client.connect();
  cout << " connected." << endl;

  cout << "Connecting to " << pub_uri << "... " << flush;
  jwspubctrl::SubClient sub_client(pub_uri, time_report_schema);
  sub_client.connect();
  cout << " connected." << endl;

  cout << "Press ENTER to cycle through time formats..." << endl;

  // Sub thread continuously reports the stream on the same line
  atomic<bool> quit(false);
  thread sub_thread([&]() {
    try {
      while (!quit) {
        try {
          auto msg = sub_client.poll_json(timeout_ms);
          string datetime = msg["datetime"];
          cout << "\r" << datetime << "\e[K" << flush;
        }
        catch (exception& e) {
          cout << "\n==> (sub) error: " << e.what() << endl;
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

  // Main loop cycles through formats in response to user input
  try {
    vector<string> formats = {"%Y-%m-%d", "%X", "%Y-%m-%d %X", "%X %Y-%m-%d", "SIM_BAD_REQUEST", "SIM_NEW_CLIENT_VESION"};
    for (size_t i = 1; !quit /*&& i < formats.size()*/; ++i) {
      try {

        auto format = formats[i % formats.size()];

        // Construct request to change datetime format
        json format_request;
        if (format == "SIM_BAD_REQUEST") {
          cout << "(ctrl) Sending a BAD request, i.e. a request that doesn't satisfy client-side schema" << endl;
          format_request = "bad request";
        }
        else if (format == "SIM_NEW_CLIENT_VESION") {
          cout << "(ctrl) Sending a request that is legal on the client, but doesn't satisfy server-side schema" << endl;
          format_request = 42;
        }
        else {
          format_request["format"] = format;
        }


        auto status_json = ctrl_client.request(format_request, timeout_ms);
        if (status_json["error"]) {
          cout << "\n(ctrl) request failed, server says: " << status_json["message"] << endl;
        }
      }
      catch (exception& e) {
        cout << "\n(ctrl) exception: " << e.what() << endl;
      }

      cin.get(); // press enter to cycle formats
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

