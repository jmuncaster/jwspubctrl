#include "server.hpp"
#include <jws/json_with_schema.hpp>
#include <zpubctrl/server.hpp>

using namespace std;

namespace zjwspubctrl {

  struct Server::Detail {
    Detail(
      const jws::json& publish_schema,
      const jws::json& ctrl_request_schema,
      const jws::json& ctrl_reply_schema,
      int pub_port,
      int ctrl_port) :
      _server(pub_port, ctrl_port) {

      _pub_validator = jws::load_validator(publish_schema);
      _ctrl_request_validator = jws::load_validator(ctrl_request_schema);
      _ctrl_reply_validator = jws::load_validator(ctrl_reply_schema);
    }
    zpubctrl::Server _server;
    jws::json_validator _pub_validator;
    jws::json_validator _ctrl_request_validator;
    jws::json_validator _ctrl_reply_validator;
  };

  Server::Server(
    const jws::json& publish_schema,
    const jws::json& ctrl_request_schema,
    const jws::json& ctrl_reply_schema,
    int pub_port,
    int ctrl_port) :
    _detail(new Detail(publish_schema, ctrl_request_schema, ctrl_reply_schema, pub_port, ctrl_port)) {
  }

  Server::~Server() { // req'd for pimpl pattern
  }

  bool Server::wait_for_request(
      int timeout_ms,
      request_callback_t request_handler,
      error_callback_t error_handler) {

    // Forward request to server, insert middleware that decodes and validates JSON.
    return _detail->_server.wait_for_request(timeout_ms, [&](const std::string& request) {

      // Middleware to decode and validate both the request and the reply
      try {
        auto request_json = jws::json::parse(request);
        _detail->_ctrl_request_validator.validate(request_json);

        // Request valid. Any exceptions here are server errors.
        try {
          auto reply_json = request_handler(request_json);
          _detail->_ctrl_reply_validator.validate(reply_json);
          return reply_json.dump();
        }
        catch (exception& e) {
          // Failed to genereate a schema-conforming reply.
          return string("Internal server error: ") + e.what();
        }
      }
      catch (exception& e) {
        // Either JSON didn't parse, or JSON didn't conform to schema -- client error.
        try {
          auto reply_json = error_handler(e);
          _detail->_ctrl_reply_validator.validate(reply_json);
          return reply_json.dump();
        }
        catch (exception& e2) {
          // Failed to generate a valid error message. This is now a server error.
          return string("Internal server error: When reporting client error '") + e.what() + "', encountered server error: " + e2.what();
        }
      }
    });
  }

  void Server::publish_data(const jws::json& payload) {
    _detail->_pub_validator.validate(payload);
    _detail->_server.publish_data(payload.dump());
  }

}

