#pragma once

#include <jws/json_with_schema.hpp>
#include <wspubctrl/constants.hpp>
#include <memory>
#include <string>

namespace jwspubctrl {

  // Client subscribes to a data stream
  class SubClient {
    public:
      SubClient(
        const std::string& pub_uri,
        const jws::json& pub_schema = {});

      ~SubClient();

      void connect();
      void disconnect();

      // Polls subscription for data
      // @param timeout_ms: Wait for this long for data. -1 means wait forever.
      // @returns received data in validated json format
      // @throws on socket error or timeout
      jws::json poll_json(int timeout_ms = wspubctrl::forever);

      // Polls subscription for data
      // @param timeout_ms: Wait for this long for data. -1 means wait forever.
      // @returns received raw data as string
      // @throws on socket error or timeout
      std::string poll_string(int timeout_ms = wspubctrl::forever);

    private:
      struct Detail;
      std::unique_ptr<Detail> _detail;
  };

}

