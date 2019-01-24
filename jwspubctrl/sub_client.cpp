#include "sub_client.hpp"
#include <jws/json_with_schema.hpp>
#include <wspubctrl/sub_client.hpp>
#include <functional>
#include <memory>
#include <string>

namespace jwspubctrl {

  struct SubClient::Detail {
    Detail(
      const std::string& pub_uri,
      const jws::json& pub_schema)
      : _client(pub_uri) {
      _pub_validator = jws::load_validator(pub_schema);
    }
    wspubctrl::SubClient _client;
    jws::json_validator _pub_validator;
  };

  SubClient::SubClient(const std::string& pub_uri, const jws::json& pub_schema)
    : _detail(new Detail(pub_uri, pub_schema)) {
  }

  SubClient::~SubClient() {
  }

  void SubClient::connect() {
    _detail->_client.connect();
  }

  void SubClient::disconnect() {
    _detail->_client.disconnect();
  }

  jws::json SubClient::poll_json(int timeout_ms) {
    auto data = _detail->_client.poll(timeout_ms);
    auto data_json = jws::json::parse(data);
    _detail->_pub_validator.validate(data_json);
    return data_json;
  }

  bool SubClient::poll_json(jws::json& data_json, int timeout_ms) {
    // Grab data, return if timeout
    std::string data;
    auto success = _detail->_client.poll(data, timeout_ms);
    if (!success) {
      return false;
    }

    // Validate json and return
    data_json = jws::json::parse(data);
    _detail->_pub_validator.validate(data_json);
    return true;
  }

  std::string SubClient::poll_string(int timeout_ms) {
    return _detail->_client.poll(timeout_ms);
  }

  bool SubClient::poll_string(std::string& data, int timeout_ms) {
    return _detail->_client.poll(data, timeout_ms);
  }

}
