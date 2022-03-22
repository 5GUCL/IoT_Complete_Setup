/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_MODBUS_MASTER_NODE_AGENT_H_
#define _CS_EDM_MODBUS_MASTER_NODE_AGENT_H_

#include <thread>
#include <vector>
#include <memory>
#include <random>
#include <functional>
// SObjectizer is one of a few cross-platform and open source "actor frameworks" for C++.
// But SObjectizer supports not only Actor Model, but also Publish-Subscribe Model and CSP-like channels.
#include <so_5/all.hpp>

// JSON library
#include "nlohmann/json.hpp"

// Logging
#include "spdlog/spdlog.h"

// Base Node
#include "node/modbus/modbus_node.h"
#include "node/modbus/modbus_node_event.h"

////////////////////////////////////////////////////////////////////////
//                 MASTER NODES -> SLAVE DEVICES                      //
////////////////////////////////////////////////////////////////////////
struct modbus_master_node_agent_t {
  modbus_master_node_agent_t
  (const std::string& controller_id
    , const std::string& agent_id, const std::string& agent_type
    , const std::string& agent_name, const std::string& agent_address
    , const std::string& agent_api_key, const std::string& agent_logger_name, long max_nodes
    , const std::string& node_id, const std::string& node_type
    , const std::string& node_name, const std::string& node_address, const std::string& node_api_key);
  ~modbus_master_node_agent_t() {}

  modbus_master_node_agent_t& init_agent();
  modbus_master_node_agent_t& process_events_fn(std::function<void(std::shared_ptr<std::vector<modbus_node_event_t>>)> node_events_callback) ;

  // Initializer list for agent
  std::string controller_id_;

  // Initializer list for agent
  std::string agent_id_;
  std::string agent_type_;
  std::string agent_name_;
  std::string agent_address_;
  std::string agent_api_key_;
  std::string agent_logger_name_;
  long max_nodes_;

  // Initializer list for Node that an agent manages
  std::string node_id_;
  std::string node_type_;
  std::string node_name_;
  std::string node_address_;
  std::string node_api_key_;

  std::shared_ptr<modbus_master_node_t> mb_master_node_;

  std::shared_ptr<std::thread> wrapper_mthread_;
};

#endif /* _CS_EDM_MODBUS_MASTER_NODE_AGENT_H_ */
