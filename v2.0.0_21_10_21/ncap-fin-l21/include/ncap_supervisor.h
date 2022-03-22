/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _PM_ODP_NCAP_SUPERVISOR_H_
#define _PM_ODP_NCAP_SUPERVISOR_H_

#include <vector>
#include <memory>
#include <sstream>

// JSON library
#include "nlohmann/json.hpp"

// Logging
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

// Application Modules
#include "ncap_device_auth_token.h"
#include "ncap_mqtt_backend.h"

#include "ncap_config_registry.h"

struct ncap_supervisor_t {
  ncap_supervisor_t (const std::string& ncap_config_file);

  void run ();
  void wait_for_ncap_backends_to_complete ();

  std::string ncap_config_file_;
  std::shared_ptr<ncap_config_registry_t> registry_sptr_;

  // MQTT - MODBUS Backend processes.
  std::vector<std::shared_ptr<ncap_mqtt_backend_t>> mmb_backend_vec_;
};

#endif /* _PM_ODP_NCAP_SUPERVISOR_H_ */
