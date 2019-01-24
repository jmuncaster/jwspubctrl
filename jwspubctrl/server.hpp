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
        int port = wspubctrl::default_port,
        const std::string& ctrl_endpoint = wspubctrl::default_ctrl_endpoint,
        const jws::json& ctrl_request_schema = {},
        const jws::json& ctrl_reply_schema = {}
        );

      ~Server();

      void start();

      // Polls ctrl socket for request. If there is a request, respond with reply.
      // @param request_handler: Callback that is called if and only if there is a request.
      // @returns true if a request was handled.
      typedef std::function<jws::json(const jws::json&)> request_callback_t;
      typedef std::function<jws::json(const std::exception&)> error_callback_t;
      bool handle_request(
        int timeout_ms,
        request_callback_t request_handler,
        error_callback_t error_handler);

      // Add a publish socket (e.g., "/pub")
      void add_publish_endpoint(const std::string& endpoint, const jws::json& schema = {});

      // Publish a message to a pub endpoint, with schema validation.
      void send(const std::string& endpoint, const jws::json& payload);

      // Publish raw string to a pub endpoint.
      void send(const std::string& endpoint, const std::string& payload);

    private:
      struct Detail;
      std::unique_ptr<Detail> _detail;
  };
}

