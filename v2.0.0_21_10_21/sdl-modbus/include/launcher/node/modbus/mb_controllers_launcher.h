/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_MB_CONTROLLER_LAUNCHER_H_
#define _CS_EDM_MB_CONTROLLER_LAUNCHER_H_

#include <thread>
#include <vector>
#include <memory>
#include <iostream>     // std::ios, std::istream, std::cout
#include <fstream>      // std::filebuf
#include <map>

// SObjectizer is one of a few cross-platform and open source "actor frameworks" for C++.
// But SObjectizer supports not only Actor Model, but also Publish-Subscribe Model and CSP-like channels.
#include <so_5/all.hpp>

// JSON library
#include "nlohmann/json.hpp"

// Logging
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include "loggers/logger_registry.h"
#include "node/modbus/modbus_node_registry.h"

// Node Modules..
#include "node/modbus/modbus_node_agent.h"
#include "node/modbus/modbus_node_controller.h"

// Forward declaration.
struct app_supervisor_t;

struct mb_edge_controller_worker_t {

  mb_edge_controller_worker_t (std::shared_ptr<app_supervisor_t> that_supervisor);

  void init_launcher();
  void init_edge_node_service_loggers(const std::string& controller_logger_code, const std::string& logger_name);
  bool is_controller_group_id_unique(const std::string& controller_grp_id);
  bool is_controller_id_unique(const std::string& controller_id);
  bool is_controller_logger_code_unique(const std::string& logger_code);
  bool is_controller_listen_address_unique(const std::string& listen_address, int listen_port);
  bool is_logger_name_unique(const std::string& logger_name);
  int build_node_launcher_context();
  int run_launcher();

  int build_modbus_master_node_context(const std::string& controller_config_root_path
    , const std::string& contoller_grp_item_config_directory
    , const std::string& contoller_grp_item_config_file_name
    , const std::string& contoller_grp_item_type
    , const std::string& contoller_grp_item_id);

  int build_modbus_server_node_context(const std::string& controller_config_root_path
    , const std::string& contoller_grp_item_config_directory
    , const std::string& contoller_grp_item_config_file_name
    , const std::string& contoller_grp_item_type
    , const std::string& contoller_grp_item_id);

  template <typename NODE_CONTROLLER_T>
  int run_node_controller_context(std::shared_ptr<std::vector<NODE_CONTROLLER_T>> controller_vec);

  template <typename NODE_CONTROLLER_T>
  int wait_node_controller_threads(std::shared_ptr<std::vector<NODE_CONTROLLER_T>> controller_vec);

  /* Edge Device (Node) Contollers . */
  // Supervisor OWNs the shared pointer.
  std::shared_ptr<std::vector<modbus_master_node_controller_t>>   modbus_master_node_controllers_vec_;
  std::shared_ptr<std::vector<modbus_server_node_controller_t>>   modbus_server_node_controllers_vec_;

  // System registry (Supervisor, Modbus Nodes, Logger etc..)
  std::shared_ptr<app_supervisor_t>           that_supervisor_;
  std::shared_ptr<logger_registry_t>          logger_registry_;
  std::shared_ptr<modbus_node_registry_t>     modbus_node_registry_;

  // Controller group id should be unique.
  std::map<std::string, bool> unique_controller_group_id;

  std::map<std::string, bool> unique_controller_id;
  std::map<std::string, bool> unique_logger_code;
  std::map<std::string, bool> unique_agent_id;
  std::map<std::string, bool> unique_logger_name;
  std::map<std::string, bool> unique_agent_address;
  std::map<std::string, bool> unique_controller_listen_address;

  // Launcher Logger
  std::string launcher_logger_code_;
  std::string launcher_logger_name_;
};

struct mb_edge_controller_launcher_t {

  mb_edge_controller_launcher_t (std::shared_ptr<app_supervisor_t> that_supervisor);
  void wrapper_fn();

  // First fuction to be called. This reads the config file (YAML/JSON) and initializes
  // the modbus based edge controllers.
  void init_launcher();

  std::shared_ptr<mb_edge_controller_worker_t> mb_ctrl_worker_;

  /* Thread object for app worker */
  std::shared_ptr<std::thread> ctrl_wrapper_thread_;
};

#endif /* _CS_EDM_MB_CONTROLLER_LAUNCHER_H_ */