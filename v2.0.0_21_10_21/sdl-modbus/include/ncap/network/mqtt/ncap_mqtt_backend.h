/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_NCAP_MQTT_BACKEND_H_
#define _CS_EDM_NCAP_MQTT_BACKEND_H_

#include <vector>
#include <map>
#include <iostream>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <string>
#include <sstream>
#include <thread>

#include "ixwebsocket/IXSetThreadName.h"
#include "ixwebsocket/IXSocket.h"
#include "ixwebsocket/IXSocketOpenSSL.h"
#include "ixwebsocket/IXUserAgent.h"
#include "ixwebsocket/IXUuid.h"
#include "ixwebsocket/IXWebSocket.h"
#include "ixwebsocket/IXWebSocketHttpHeaders.h"

// SObjectizer is one of a few cross-platform and open source "actor frameworks" for C++.
// But SObjectizer supports not only Actor Model, but also Publish-Subscribe Model and CSP-like channels.
#include <so_5/all.hpp>

// JSON library
#include "nlohmann/json.hpp"

// Logging
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

// Node registry
#include "node/modbus/modbus_node_registry.h"

// Logger Registry
#include "loggers/logger_registry.h"

#include "node/modbus/modbus_protocol.h"

#include "ncap/network/ncap_backend_defs.h"

// Forward declaration.
struct app_supervisor_t;

struct mb_ncap_mqtt_ws_client_t {
  mb_ncap_mqtt_ws_client_t(const std::string& _url
    , std::shared_ptr<so_5::wrapped_env_t> ncap_mqtt_env_obj
    , so_5::mchain_t& msg_primary_channel
    , so_5::mchain_t msg_secondary_channel);

  std::string truncate(const std::string& str, size_t n);
  bool run();

  std::string   controller_url_;
  ix::WebSocket ncap_mqtt_ws_;

  std::mutex    mutex_;
  std::condition_variable condition_;

  std::shared_ptr<so_5::wrapped_env_t> ncap_mqtt_env_obj_;
  so_5::mchain_t msg_primary_channel_;
  so_5::mchain_t msg_secondary_channel_;

};

// Mailbox message data ...
struct ncap_mqtt_mailbox_message_t {
  std::string msg_received_;
};

struct mb_ncap_mqtt_ws_client_worker_t {

  void work();

  std::shared_ptr<so_5::wrapped_env_t> ncap_mqtt_env_obj_;
  so_5::mchain_t msg_primary_channel_;
  so_5::mchain_t msg_secondary_channel_;
  std::shared_ptr<std::thread> ws_thread_;
  std::string mb_controller_id_;
  std::string mb_master_contoller_address_;
  int mb_master_contoller_port_;
};

struct mb_ncap_mqtt_backend_t {

  const std::string SMART_LOGGER_NCAP_MQTT_LOGGER_CODE_ = "mb-ncap";
  const std::string SMART_LOGGER_NCAP_MQTT_LOGGER_NAME_ = "mb-mqtt";

  mb_ncap_mqtt_backend_t (std::shared_ptr<app_supervisor_t> that_supervisor);

  // System Registry Objects
  std::shared_ptr<app_supervisor_t> that_supervisor_;
  std::shared_ptr<modbus_node_registry_t> mb_node_registry_;
  std::shared_ptr<logger_registry_t> logger_registry_;

  void init_ncap_mqtt_service_loggers(const std::string& controller_logger_code, const std::string& logger_name);
  int init_mb_ncap_mqtt();

  ////////////////////////////
  // Prepare Device Id controller_id#agent_id#node_id
  ////////////////////////////
  std::string prepare_device_id(const std::string& controller_id, const std::string& agent_id, const std::string& node_id);

  ////////////////////////////
  // Data processor
  // Process the modbus messages received over masters/controllers.
  ////////////////////////////
  void message_recording_fn();
  std::shared_ptr<so_5::wrapped_env_t> ncap_mqtt_env_obj_;
  so_5::mchain_t msg_primary_channel_;
  so_5::mchain_t msg_secondary_channel_;
  std::shared_ptr<std::thread> dsthread_;
  std::vector<mb_ncap_mqtt_ws_client_worker_t> ncap_mqtt_ws_client_workers_vec_;
  std::vector<ncap_mqtt_backend_config_t> ncap_backend_mqtt_vec_;
 
  int ncap_mb_coil_measurement
  (const std::string& v_device_id, int block_id, int start_address
    , int coil_number, const modbus_master_coil_bit_single_measurement_instance_t& measurement);

  int ncap_mb_input_bit_measurement
  (const std::string& v_device_id, int block_id, int start_address
    , int input_bit_number, const modbus_master_input_bit_single_measurement_instance_t& measurement);

  int ncap_mb_holding_register_measurement
  (const std::string& v_device_id, int block_id, int start_address
    , int holding_reg_number, const modbus_master_holding_register_single_measurement_instance_t& measurement);

  int ncap_mb_input_register_measurement
  (const std::string& v_device_id, int block_id, int start_address
    , int input_reg_number, const modbus_master_input_register_single_measurement_instance_t& measurement);
 
};

#endif /* _CS_EDM_NCAP_MQTT_BACKEND_H_ */