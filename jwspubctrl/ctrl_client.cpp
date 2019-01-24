#include "ctrl_client.hpp"
#include <jws/json_with_schema.hpp>
#include <wspubctrl/ctrl_client.hpp>
#include <functional>
#include <memory>
#include <string>

namespace jwspubctrl {

  struct CtrlClient::Detail {
    Detail(
      const std::string& ctrl_uri,
      const jws::json& ctrl_request_schema,
      const jws::json& ctrl_reply_schema)
      : _client(ctrl_uri) {
      _ctrl_request_validator = jws::load_validator(ctrl_request_schema);
      _ctrl_reply_validator = jws::load_validator(ctrl_reply_schema);
    }
    wspubctrl::CtrlClient _client;
    jws::json_validator _ctrl_request_validator;
    jws::json_validator _ctrl_reply_validator;
  };

  CtrlClient::CtrlClient(
      const std::string& ctrl_uri,
      const jws::json& ctrl_request_schema,
      const jws::json& ctrl_reply_schema)
    : _detail(new Detail(ctrl_uri, ctrl_request_schema, ctrl_reply_schema)) {
  }

  CtrlClient::~CtrlClient() {
  }

  void CtrlClient::connect() {
    _detail->_client.connect();
  }

  void CtrlClient::disconnect() {
    _detail->_client.disconnect();
  }

  jws::json CtrlClient::request(const jws::json& payload_json, int timeout_ms) {
    _detail->_ctrl_request_validator.validate(payload_json);
    auto reply = _detail->_client.request(payload_json.dump(), timeout_ms);
    auto reply_json = jws::json::parse(reply);
    _detail->_ctrl_reply_validator.validate(reply_json);
    return reply_json;
  }

}

