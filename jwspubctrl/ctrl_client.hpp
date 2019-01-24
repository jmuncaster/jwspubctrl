#pragma once

#include <jws/json_with_schema.hpp>
#include <wspubctrl/constants.hpp>
#include <functional>
#include <memory>
#include <string>

namespace jwspubctrl {

  // Client issues synchronous control requests.
  class CtrlClient {
    public:
      CtrlClient(
        const std::string& ctrl_uri,
        const jws::json& ctrl_request_schema,
        const jws::json& ctrl_reply_schema);

      ~CtrlClient();

      void connect();
      void disconnect();

      // Synchronously issue a request and wait for the reply.
      // @returns contents of reply
      // @throws on socket error or timeout
      jws::json request(const jws::json& payload_json, int timeout_ms = wspubctrl::default_request_timeout_ms);

    private:
      struct Detail;
      std::unique_ptr<Detail> _detail;
  };

}

