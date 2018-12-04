#include "sub_client.hpp"
#include <jws/json_with_schema.hpp>
#include <wspubctrl/sub_client.hpp>
#include <functional>
#include <memory>
#include <string>

namespace jwspubctrl {

  struct SubClient::Detail {
    Detail(
      const jws::json& pub_schema,
      const std::string& pub_uri) :
      _client(pub_uri) {
      _pub_validator = jws::load_validator(pub_schema);
    }
    wspubctrl::SubClient _client;
    jws::json_validator _pub_validator;
  };

  SubClient::SubClient(const jws::json& pub_schema, const std::string& pub_uri) :
    _detail(new Detail(pub_schema, pub_uri)) {
  }

  SubClient::SubClient(const std::string& pub_schema_filename, const std::string& pub_uri) {
    auto pub_schema = !pub_schema_filename.empty() ? jws::load_json(pub_schema_filename) : jws::json{};
    _detail.reset(new Detail(pub_schema, pub_uri));
  }

  SubClient::SubClient(const char* pub_schema_filename, const std::string& pub_uri) :
    SubClient(std::string(pub_schema_filename), pub_uri) {
  }

  SubClient::~SubClient() {
  }

  void SubClient::start() {
    _detail->_client.start();
  }

  jws::json SubClient::wait_for_data(int timeout_ms) {
    auto data = _detail->_client.wait_for_data(timeout_ms);
    auto data_json = jws::json::parse(data);
    _detail->_pub_validator.validate(data_json);
    return data_json;
  }

  std::string SubClient::wait_for_raw(int timeout_ms) {
    return _detail->_client.wait_for_data(timeout_ms);
  }

}
