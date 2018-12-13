#include "server.hpp"
#include <jws/json_with_schema.hpp>
#include <wspubctrl/server.hpp>

using namespace std;

namespace jwspubctrl {

  struct Server::Detail {
    Detail(
      int port,
      const std::string& ctrl_endpoint,
      const jws::json& ctrl_request_schema,
      const jws::json& ctrl_reply_schema)
      : _server(port, ctrl_endpoint) {
      _ctrl_request_validator = jws::load_validator(ctrl_request_schema);
      _ctrl_reply_validator = jws::load_validator(ctrl_reply_schema);
    }

    void add_publish_endpoint(const string& pub_endpoint, const jws::json& pub_schema) {
      _pub_validators[pub_endpoint] = jws::load_validator(pub_schema);
      _server.add_publish_endpoint(pub_endpoint);
    }

    void validate(const string& pub_endpoint, const jws::json& payload) {
      auto it = _pub_validators.find(pub_endpoint);
      if (it == _pub_validators.end()) {
        throw runtime_error(string("endpoint not found: ") + pub_endpoint);
      }
      it->second.validate(payload);
    }

    wspubctrl::Server _server;
    map<std::string, jws::json_validator> _pub_validators;
    jws::json_validator _ctrl_request_validator;
    jws::json_validator _ctrl_reply_validator;
  };

  Server::Server(
    int port,
    const string& ctrl_endpoint,
    const jws::json& ctrl_request_schema,
    const jws::json& ctrl_reply_schema
    )
    : _detail(new Detail(port, ctrl_endpoint, ctrl_request_schema, ctrl_reply_schema)) {
  }

  Server::Server(
    int port,
    const string& ctrl_endpoint,
    const std::string& ctrl_request_schema_filename,
    const std::string& ctrl_reply_schema_filename
    ) {
    // Load schema or leave empty if filename is empty
    auto ctrl_request_schema = !ctrl_request_schema_filename.empty() ? jws::load_json(ctrl_request_schema_filename) : jws::json{};
    auto ctrl_reply_schema =     !ctrl_reply_schema_filename.empty() ? jws::load_json(ctrl_reply_schema_filename) : jws::json{};
    _detail.reset(new Detail(port, ctrl_endpoint, ctrl_request_schema, ctrl_reply_schema));
  }

  Server::Server(
    int port,
    const string& ctrl_endpoint,
    const char* ctrl_request_schema_filename,
    const char* ctrl_reply_schema_filename
    )
    : Server(
        port,
        ctrl_endpoint,
        std::string(ctrl_request_schema_filename),
        std::string(ctrl_reply_schema_filename)) {
  }

  Server::~Server() { // req'd for pimpl pattern
  }

  void Server::start() {
    _detail->_server.start();
  }

  void Server::add_publish_endpoint(const std::string& endpoint, const jws::json& json) {
    _detail->add_publish_endpoint(endpoint, json);
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

  void Server::send(const string& endpoint, const jws::json& payload) {
    _detail->validate(endpoint, payload);
    _detail->_server.send(endpoint, payload.dump());
  }

  void Server::send(const string& endpoint, const std::string& payload) {
    _detail->_server.send(endpoint, payload);
  }

}

