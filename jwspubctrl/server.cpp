#include "server.hpp"
#include <jws/json_with_schema.hpp>
#include <wspubctrl/server.hpp>
#include <string>
#include <stdexcept>

using namespace std;

namespace jwspubctrl {

  struct Server::Detail {
    Detail(
      const jws::json& ctrl_request_schema,
      const jws::json& ctrl_reply_schema,
      int port) :
      _server(port) {

      _ctrl_request_validator = jws::load_validator(ctrl_request_schema);
      _ctrl_reply_validator = jws::load_validator(ctrl_reply_schema);
    }
    wspubctrl::Server _server;
    map<std::string, jws::json_validator> _pub_validators;
    jws::json_validator _ctrl_request_validator;
    jws::json_validator _ctrl_reply_validator;
  };

  Server::Server(
    const jws::json& ctrl_request_schema,
    const jws::json& ctrl_reply_schema,
    int port) :
    _detail(new Detail(ctrl_request_schema, ctrl_reply_schema, port)) {
  }

  Server::Server(
    const std::string& ctrl_request_schema_filename,
    const std::string& ctrl_reply_schema_filename,
    int port) {
    // Load schema or leave empty if filename is empty
    auto ctrl_request_schema = !ctrl_request_schema_filename.empty() ? jws::load_json(ctrl_request_schema_filename) : jws::json{};
    auto ctrl_reply_schema =     !ctrl_reply_schema_filename.empty() ? jws::load_json(ctrl_reply_schema_filename) : jws::json{};
    _detail.reset(new Detail(ctrl_request_schema, ctrl_reply_schema, port));
  }

  Server::Server(
    const char* ctrl_request_schema_filename,
    const char* ctrl_reply_schema_filename,
    int port) :
      Server(
        std::string(ctrl_request_schema_filename),
        std::string(ctrl_reply_schema_filename),
        port) {
  }

  Server::~Server() { // req'd for pimpl pattern
  }

  void Server::start() {
    _detail->_server.start();
  }

  bool Server::handle_request(
      int timeout_ms,
      request_callback_t request_handler,
      error_callback_t error_handler) {

    // Forward request to server, insert middleware that decodes and validates JSON.
    return _detail->_server.handle_request(timeout_ms, [&](const std::string& request) {

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

  // Add a publish socket (e.g., "/pub")
  void Server::add_publish_endpoint(const std::string& endpoint, const jws::json& schema) {
    auto it = _detail->_pub_validators.find(endpoint);
    if (it != _detail->_pub_validators.end()) {
      //throw std::runtime_error("add_publish_endpoint: endpoint '"s + endpoint + "' already added");
      throw std::runtime_error(string("add_publish_endpoint: endpoint '") + endpoint + "' already added");
    }
    _detail->_pub_validators[endpoint] = jws::load_validator(schema);
    _detail->_server.add_publish_endpoint(endpoint);
  }

  void Server::send_json(const std::string& endpoint, const jws::json& payload) {
    auto it = _detail->_pub_validators.find(endpoint);
    if (it == _detail->_pub_validators.end()) {
      //throw std::runtime_error("send_json: no publish endpoint '"s + endpoint + "'");
      throw std::runtime_error(string("send_json: no publish endpoint '") + endpoint + "'");
    }
    it->second.validate(payload);
    _detail->_server.send(endpoint, payload.dump());
  }

  void Server::send_string(const std::string& endpoint, const std::string& payload) {
    _detail->_server.send(endpoint, payload);
  }

}

