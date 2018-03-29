#pragma once

#include <wspubctrl/constants.hpp>
#include <jws/json_with_schema.hpp>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

namespace jwspubctrl {

  // Server publishes data stream and accepts synchronous control requests.
  class Server {
    public:
      Server(
        const jws::json& publish_schema = {},
        const jws::json& ctrl_request_schema = {},
        const jws::json& ctrl_reply_schema = {},
        int pub_port = wspubctrl::default_port);

      Server(
        const std::string& publish_schema_filename,
        const std::string& ctrl_request_schema_filename = "",
        const std::string& ctrl_reply_schema_filename = "",
        int pub_port = wspubctrl::default_port);

      Server(
        const char* publish_schema_filename,
        const char* ctrl_request_schema_filename = "",
        const char* ctrl_reply_schema_filename = "",
        int pub_port = wspubctrl::default_port);

      ~Server();

      // Polls ctrl socket for request. If there is a request, respond with reply.
      // @param request_handler: Callback that is called if and only if there is a request.
      // @returns true if a request was handled.
      typedef std::function<jws::json(const jws::json&)> request_callback_t;
      typedef std::function<jws::json(const std::exception&)> error_callback_t;
      bool wait_for_request(
        int timeout_ms,
        request_callback_t request_handler,
        error_callback_t error_handler);

      // Publish a message to the pub socket
      void publish_data(const jws::json& payload);

    private:
      struct Detail;
      std::unique_ptr<Detail> _detail;
  };
}

