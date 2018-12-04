#include <jws/json_with_schema.hpp>
#include <jwspubctrl/client.hpp>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;
using jws::json;

int main(int argc, char* argv[]) {

  if (argc != 3) {
    cout << "usage: pubstring_client <ctrl_request.json> <ctrl_reply.json>" << endl;
    return 1;
  }

  cout << "Load " << argv[1] << endl;
  auto pubstring_ctrl_schema = jws::load_json(argv[1]);

  cout << "Load " << argv[2] << endl;
  auto pubstring_ctrl_reply_schema = jws::load_json(argv[2]);

  jwspubctrl::SubClient sub_client("localhost:5554/pub", json{});
  jwspubctrl::CtrlClient ctrl_client("localhost:5554/ctrl", pubstring_ctrl_schema, pubstring_ctrl_reply_schema);

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
      json reply = ctrl_client.request(request);
      current_message_size = reply["message_size"];
    }

    auto data = sub_client.wait_for_raw();
    bytes_received += data.size();
    cout << "\rReceived " << (bytes_received >> 20) << " Mb in total (current message size: " << (current_message_size >> 10) << " Kb)" << "\e[K" << flush;

    ++i;
  }

  return 0;
}

