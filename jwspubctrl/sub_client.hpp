#pragma once

#include <jws/json_with_schema.hpp>
#include <wspubctrl/constants.hpp>
#include <memory>
#include <string>

namespace jwspubctrl {

  // Client subscribes to a data stream
  class SubClient {
    public:
      SubClient(const std::string& pub_uri, const jws::json& pub_schema);
      SubClient(const std::string& pub_uri, const std::string& pub_schema_filename);
      SubClient(const std::string& pub_uri, const char* pub_schema_filename);

      ~SubClient();

      void start();

      // Polls subscription for data
      // @param timeout_ms: Wait for this long for data. -1 means wait forever.
      // @returns received data in validated json format
      // @throws on socket error or timeout
      jws::json wait_for_data(int timeout_ms = wspubctrl::forever);

      // Polls subscription for data
      // @param timeout_ms: Wait for this long for data. -1 means wait forever.
      // @returns received raw data as string
      // @throws on socket error or timeout
      std::string wait_for_raw(int timeout_ms = wspubctrl::forever);

    private:
      struct Detail;
      std::unique_ptr<Detail> _detail;
  };

}

