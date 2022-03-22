/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_MODBUS_NODE_REGISTRY_H_
#define _CS_EDM_MODBUS_NODE_REGISTRY_H_

#include <vector>
#include <memory>
#include <sstream>
#include <iostream>     // std::ios, std::istream, std::cout
#include <fstream>      // std::filebuf
#include <map>

// JSON library
#include "nlohmann/json.hpp"

struct modbus_node_registry_t {
  modbus_node_registry_t() {};

  // Returns true if new node config can run in the current process.
  bool register_new_node(const std::string& jnode_full_config)
  {
    std::string jnode_full_config_ = jnode_full_config;
    node_registry_.emplace_back(std::move(jnode_full_config_));
    return true;
  };

  // Node Registry. Holds json objects
  std::vector<std::string> node_registry_;

};

#endif /* _CS_EDM_MODBUS_NODE_REGISTRY_H_ */
