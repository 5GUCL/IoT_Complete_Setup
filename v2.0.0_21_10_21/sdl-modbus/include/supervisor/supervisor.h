/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_SUPERVISOR_H_
#define _CS_EDM_SUPERVISOR_H_

#include <thread>
#include <vector>

// SObjectizer is one of a few cross-platform and open source "actor frameworks" for C++.
// But SObjectizer supports not only Actor Model, but also Publish-Subscribe Model and CSP-like channels.
#include <so_5/all.hpp>

// JSON library
#include "nlohmann/json.hpp"

// Logging
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include "loggers/logger_registry.h"

// Node Modules..
#include "node/modbus/modbus_node_agent.h"
#include "node/modbus/modbus_node_registry.h"

#include "app-gui/main-gui/main_gui.h"

// Forward declarations of Module Launchers
struct mb_edge_controller_launcher_t;
struct mb_node_master_h5_recorder_launcher_t;
struct mb_node_master_imgui_launcher_t;
struct mb_ncap_mqtt_launcher_t;

// Forward declarations of controllers, recorders etc.
struct modbus_master_node_controller_t;
struct modbus_server_node_controller_t;
struct mb_node_master_dataset_recorder_main_t;
struct modbus_master_node_t;
struct mb_node_master_imgui_components_main_t;
struct mb_ncap_mqtt_backend_t;

//
// DESIGN NOTES
// It is a bit workaround. Main (Client) creates object and then calls launch_app().
//
struct app_supervisor_t {

  app_supervisor_t ();

  int launch_app(std::shared_ptr<app_supervisor_t> that_supervisor);

  // Initialize the containers of node registry and logger registry etc.
  std::shared_ptr<modbus_node_registry_t> modbus_node_registry_;
  std::shared_ptr<logger_registry_t> logger_registry_;

  std::shared_ptr<app_supervisor_t> that_supervisor_;

  std::shared_ptr<app_main_gui_t> app_;
  
  // IMPORTANT
  // Only one object in the vector / collection. Do not create multiple elements.
  //
  // Modbus controller(s) modules
  std::vector<mb_edge_controller_launcher_t>                      mb_edge_controller_launcher_vec_;
  std::shared_ptr<std::vector<modbus_master_node_controller_t>>   modbus_master_node_controllers_vec_;
  std::shared_ptr<std::vector<modbus_server_node_controller_t>>   modbus_server_node_controllers_vec_;
  // Start HDF5 Recorders
  std::vector<mb_node_master_h5_recorder_launcher_t>              mb_h5_launcher_vec_;
  // Start NCAP
  std::vector<mb_ncap_mqtt_launcher_t>                            mb_ncap_mqtt_launcher_vec_;
  // GUI Module
  std::vector<mb_node_master_imgui_launcher_t>                    modbus_master_gui_launcher_vec_;

  // APIS
  // Supervisor H5 APIs
  std::shared_ptr<mb_node_master_dataset_recorder_main_t> get_modbus_h5_recorder_handle() const;

  const modbus_master_node_t* get_mb_master_node_handle(const std::string& controller_id
    , const std::string& agent_id
    , const std::string& node_id) const;

  std::shared_ptr<mb_node_master_imgui_components_main_t> get_mb_master_gui_components();
};

#endif /* _CS_EDM_SUPERVISOR_H_ */