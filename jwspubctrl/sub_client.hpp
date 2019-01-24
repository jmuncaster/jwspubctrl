#pragma once

#include <jws/json_with_schema.hpp>
#include <wspubctrl/constants.hpp>
#include <memory>
#include <string>

namespace jwspubctrl {

  /** A Client for subscribing to a websocket endpoint.
   * SubClient subscribes to a websocket publisher endpoint and optionally validates incoming
   * JSON messages. Can also return raw bytes.
  */
  class SubClient {
    public:
      SubClient(const std::string& pub_uri, const jws::json& pub_schema = {});
      ~SubClient();

      /** Connect to pub_uri
       * Starts background thread and waits for connection
       * @throws on socket error or timeout
      */
      void connect();

      /** Disconnect from pub_uri
       * Stops background thread. On timeout, thread is killed.
      */
      void disconnect();

      /** Poll publisher for data
       * The first version always throws on timeout, the other returns false if a timeout occurred.
       * @param timeout_ms: Wait for this long for data. -1 means wait forever.
       * @returns received data in validated json format. Second version returns success/timeout.
       * @throws on socket error or timeout
       * @throws on socket error. First version throws on timeout
      */
      jws::json poll_json(int timeout_ms = wspubctrl::forever);
      bool poll_json(jws::json& data, int timeout_ms = wspubctrl::forever);

      /** Polls publisher for data
       * The first version always throws on timeout, the other returns false if a timeout occurred.
       * @param timeout_ms: Wait for this long for data. -1 means wait forever.
       * @returns received raw data as string. Second version returns success/timeout.
       * @throws on socket error. First version throws on timeout
      */
      std::string poll_string(int timeout_ms = wspubctrl::forever);
      bool poll_string(std::string& data, int timeout_ms = wspubctrl::forever);

    private:
      SubClient(const std::string&, const std::string&) = delete; // avoids confusion from implicit conversion
      // private impl.
      struct Detail;
      std::unique_ptr<Detail> _detail;
  };
} // namespace jwspubctrl

