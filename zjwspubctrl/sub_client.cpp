#include "sub_client.hpp"
#include <zpubctrl/sub_client.hpp>
#include <functional>
#include <memory>
#include <string>

namespace zjwspubctrl {

  struct SubClient::Detail {
    Detail(
      const jws::json& pub_schema,
      const std::string& server_address,
      int pub_port) :
      _client(server_address, pub_port) {
      _pub_validator.set_root_schema(pub_schema);
    }
    zpubctrl::SubClient _client;
    jws::json_validator _pub_validator;
  };

  SubClient::SubClient(const jws::json& pub_schema, const std::string& server_address, int pub_port) :
    _detail(new Detail(pub_schema, server_address, pub_port)) {
  }

  SubClient::~SubClient() {
  }

  jws::json SubClient::wait_for_data(int timeout_ms) {
    auto data = _detail->_client.wait_for_data(timeout_ms);
    auto data_json = jws::json::parse(data);
    _detail->_pub_validator.validate(data_json);
    return data_json;
  }

}
