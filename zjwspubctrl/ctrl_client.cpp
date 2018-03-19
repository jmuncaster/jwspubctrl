#include "ctrl_client.hpp"
#include <jws/json_with_schema.hpp>
#include <zpubctrl/ctrl_client.hpp>
#include <functional>
#include <memory>
#include <string>

namespace zjwspubctrl {

  struct CtrlClient::Detail {
    Detail(
      const jws::json& ctrl_request_schema,
      const jws::json& ctrl_reply_schema,
      const std::string& server_address,
      int ctrl_port) :
      _client(server_address, ctrl_port) {
      _ctrl_request_validator = jws::load_validator(ctrl_request_schema);
      _ctrl_reply_validator = jws::load_validator(ctrl_reply_schema);
    }
    zpubctrl::CtrlClient _client;
    jws::json_validator _ctrl_request_validator;
    jws::json_validator _ctrl_reply_validator;
  };

  CtrlClient::CtrlClient(
      const jws::json& ctrl_request_schema,
      const jws::json& ctrl_reply_schema,
      const std::string& server_address,
      int ctrl_port) :
    _detail(new Detail(ctrl_request_schema, ctrl_reply_schema, server_address, ctrl_port)) {
  }

  CtrlClient::CtrlClient(
      const std::string& ctrl_request_schema_filename,
      const std::string& ctrl_reply_schema_filename,
      const std::string& server_address,
      int ctrl_port) :
    _detail(new Detail(jws::load_json(ctrl_request_schema_filename), jws::load_json(ctrl_reply_schema_filename), server_address, ctrl_port)) {
  }

  CtrlClient::CtrlClient(
      const char* ctrl_request_schema_filename,
      const char* ctrl_reply_schema_filename,
      const std::string& server_address,
      int ctrl_port) :
        CtrlClient(std::string(ctrl_request_schema_filename), std::string(ctrl_reply_schema_filename), server_address, ctrl_port) {
  }

  CtrlClient::~CtrlClient() {
  }

  jws::json CtrlClient::request(const jws::json& payload_json, int timeout_ms) {
    _detail->_ctrl_request_validator.validate(payload_json);
    auto reply = _detail->_client.request(payload_json.dump(), timeout_ms);
    auto reply_json = jws::json::parse(reply);
    _detail->_ctrl_reply_validator.validate(reply_json);
    return reply_json;
  }

}

