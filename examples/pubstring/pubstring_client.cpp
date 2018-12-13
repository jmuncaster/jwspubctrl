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

  if (argc != 6) {
    cout << "usage: pubstring_client <host:port> <ctrl_endpoint> <pub_endpoint> <ctrl_request.json> <ctrl_reply.json>" << endl;
    cout << "example: ./pubstring_client localhost:5554 /ctrl /pub ../examples/pubstring/schemas/ctrl_request.json ../examples/pubstring/schemas/ctrl_reply.json" << endl;
    return 1;
  }

  auto host = (argv[1]);
  auto ctrl_uri = host + string(argv[2]);
  auto pub_uri = host + string(argv[3]);

  cout << "Loading schemas..." << endl;
  cout << "  Load " << argv[4] << endl;
  auto pubstring_ctrl_schema = jws::load_json(argv[4]);
  cout << "  Load " << argv[5] << endl;
  auto pubstring_ctrl_reply_schema = jws::load_json(argv[5]);

  cout << "Connecting to " << ctrl_uri << "... " << flush;
  jwspubctrl::CtrlClient ctrl_client(ctrl_uri, pubstring_ctrl_schema, pubstring_ctrl_reply_schema);
  ctrl_client.connect();
  cout << " connected." << endl;

  cout << "Connecting to " << pub_uri << "... " << flush;
  jwspubctrl::SubClient sub_client(pub_uri, json{});
  sub_client.connect();
  cout << " connected." << endl;

  int i = 0;
  int desired_size = 1;
  int current_message_size = 0;
  unsigned long bytes_received = 0;
  for (;;) {
    if (i % 10 == 0) {
      json request = R"({ "request_type": "get_message_size" })"_json;
      json reply = ctrl_client.request(request);
      current_message_size = reply["message_size"];
    }
    if (i % 100 == 0) {
      desired_size = desired_size << 1;
      if (desired_size > (8 << 20)) {
        desired_size = 1;
      }
      json request = jws::json{
        {"request_type", "set_message_size"},
        {"message_size", desired_size},
      };
      cout << "\nRequest change to message size: " << (desired_size >> 10) << " Kb" << endl;
      json reply = ctrl_client.request(request);
      current_message_size = reply["message_size"];
    }

    auto data = sub_client.poll_string();
    bytes_received += data.size();
    cout << "\rReceived " << (bytes_received >> 20) << " Mb in total (current message size: " << (current_message_size >> 10) << " Kb)" << "\e[K" << flush;

    ++i;
  }

  return 0;
}

