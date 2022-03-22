/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _PM_ODP_NCAP_CONFIG_REGISTRY_H_
#define _PM_ODP_NCAP_CONFIG_REGISTRY_H_

#include <vector>
#include <memory>
#include <sstream>
#include <iostream>     // std::ios, std::istream, std::cout
#include <fstream>      // std::filebuf
#include <map>

#include "sf-sys/utils/cpp_prog_utils.h"

// JSON library
#include "nlohmann/json.hpp"

#include "ncap_config_defs.h"

struct ncap_config_registry_t {
  
  // The file shall be in ../pm-fin-l21-config/
  ncap_config_registry_t( const std::string& config_yaml_file );
  bool load_ncap_config_registry ();
  bool is_ncap_id_unique( const std::string& ncap_id );
  bool is_ncap_api_server_unique( const std::string& ncap_api_server );

  std::string config_yaml_file_;
  std::string config_json_file_;
  nlohmann::json jncap_config_reg_;
  
  // NCAP REGISTRY for MQTT - MODBUS backend.
  std::map<std::string /* NCAP ID is the Key */
             , std::shared_ptr<ncap_mqtt_modbus_config_t>> mmb_registry_;
  
  // The following objects are used for building the NCAP config objects.
  // NCAP ID is the Key. 
  std::map<std::string, bool> unique_ncap_id_;
  
  // NCAP API Adress is the Key.
  std::map<std::string, bool> unique_ncap_api_server_;
};

#endif /* _PM_ODP_NCAP_CONFIG_REGISTRY_H_ */
