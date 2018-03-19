#pragma once

#include <jws/json_with_schema.hpp>
#include <zpubctrl/constants.hpp>
#include <functional>
#include <memory>
#include <string>

namespace zjwspubctrl {

  // Client issues synchronous control requests.
  class CtrlClient {
    public:
      CtrlClient(
        const jws::json& ctrl_request_schema,
        const jws::json& ctrl_reply_schema,
        const std::string& server_address = zpubctrl::default_host,
        int ctrl_port = zpubctrl::default_ctrl_port);

      CtrlClient(
        const std::string& ctrl_request_schema_filename,
        const std::string& ctrl_reply_schema_filename,
        const std::string& server_address = zpubctrl::default_host,
        int ctrl_port = zpubctrl::default_ctrl_port);

      ~CtrlClient();

      // Synchronously issue a request and wait for the reply.
      // @returns contents of reply
      // @throws on socket error or timeout
      jws::json request(const jws::json& payload_json, int timeout_ms = zpubctrl::default_request_timeout_ms);

    private:
      struct Detail;
      std::unique_ptr<Detail> _detail;
  };

}

