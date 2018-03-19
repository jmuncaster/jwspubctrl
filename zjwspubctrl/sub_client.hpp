#pragma once

#include <jws/json_with_schema.hpp>
#include <zpubctrl/constants.hpp>
#include <memory>
#include <string>

namespace zjwspubctrl {

  // Client subscribes to a data stream
  class SubClient {
    public:
      SubClient(
        const jws::json& pub_schema,
        const std::string& server_address = zpubctrl::default_host,
        int pub_port = zpubctrl::default_data_port);

      SubClient(
        const std::string& pub_schema_filename,
        const std::string& server_address = zpubctrl::default_host,
        int pub_port = zpubctrl::default_data_port);

      ~SubClient();

      // Polls subscription for data
      // @param timeout_ms: Wait for this long for data. -1 means wait forever.
      // @returns received data
      // @throws on socket error or timeout
      jws::json wait_for_data(int timeout_ms = zpubctrl::forever);

    private:
      struct Detail;
      std::unique_ptr<Detail> _detail;
  };

}

