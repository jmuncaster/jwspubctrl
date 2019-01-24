#pragma once

#include <jws/json_with_schema.hpp>
#include <wspubctrl/constants.hpp>
#include <functional>
#include <memory>
#include <string>

namespace jwspubctrl {

  /** A Client for synchronous request/reply communication to a websocket endpoint.
   * CtrlClient validates JSON requests, blocks until a JSON reply is received, validates
   * the reply, and returns the JSON reply to the user.
  */
  class CtrlClient {
    public:
      CtrlClient(const std::string& ctrl_uri, const jws::json& ctrl_request_schema, const jws::json& ctrl_reply_schema);
      ~CtrlClient();

      /** Connect to ctrl_uri
       * Starts background thread and waits for connection
       * @throws on socket error or timeout
      */
      void connect();

      /** Disconnect from ctrl_uri
       * Stops background thread. On timeout, thread is killed.
      */
      void disconnect();

      /** Synchronously issue a request and wait for the reply.
       * @returns contents of reply
       * @throws on socket error or timeout
      */
      jws::json request(const jws::json& payload_json, int timeout_ms = wspubctrl::default_request_timeout_ms);

    private:
      // delete these to avoids confusion from implicit conversion
      CtrlClient(const std::string&, const jws::json&, const std::string&) = delete;
      CtrlClient(const std::string&, const std::string&, const jws::json&) = delete;
      CtrlClient(const std::string&, const std::string&, const std::string&) = delete;
      // private impl.
      struct Detail;
      std::unique_ptr<Detail> _detail;
  };
} // namespace jwspubctrl

