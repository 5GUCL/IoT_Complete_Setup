/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _PM_ODP_NCAP_MQTT_BACKEND_H_
#define _PM_ODP_NCAP_MQTT_BACKEND_H_

#include <iostream>     // std::ios, std::istream, std::cout
#include <fstream>      // std::filebuf
#include <sstream>
#include <future>
#include <thread>
#include <chrono>

// SObjectizer is one of a few cross-platform and open source "actor frameworks" for C++.
// But SObjectizer supports not only Actor Model, but also Publish-Subscribe Model and CSP-like channels.
#include "so_5/all.hpp"

// JSON library
#include "nlohmann/json.hpp"

// Logging
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

// MQTT client
#include <boost/asio.hpp>
#include "mqtt_client_cpp.hpp"

// App specific
#include "ncap_config_registry.h"
#include "ncap_device_auth_token.h"
#include "ncap_ws_data_source_subscriber.h"

// Mailbox message data ...
struct ncap_mqtt_client_env_mailbox_message_t {
  std::string msg_;
};

#define MAX_CHAR_MESSAGE_LEN_TO_MQTT_BROKER (2048)

enum mqtt_context_ready_state_future_e {
  READY = 0,
  RECONNECTING = 1,
  DISCONNECTED = 2,
  ERROR = 3,
  UNKNOWN = 4
};

struct mqtt_ws_tls_client_app_t {
    boost::asio::io_context ioc {};
    using client_t = decltype(MQTT_NS::make_tls_sync_client_ws(std::declval<boost::asio::io_context &>(), "", 0));
    client_t c;
  };

struct mqtt_ws_client_app_t {
    boost::asio::io_context ioc {};
    using client_t = decltype(MQTT_NS::make_sync_client_ws(std::declval<boost::asio::io_context &>(), "", 0));
    client_t c;
  };

struct ncap_mqtt_backend_t {
  ncap_mqtt_backend_t (
                          // NCAP MQTT Modbus config object
                          std::shared_ptr<ncap_mqtt_modbus_config_t> ncap_mmb_obj
                      );
  // Used while constructing the object.
  bool init ();

  void mmb_wrapper_thread_fn ();
  void mmb_work ();

  // NCAP registry
  std::shared_ptr<ncap_mqtt_modbus_config_t> ncap_mmb_obj_;

  // NCAP ID.
  std::string ncap_id_;

  // OIDC/Identity for Device Auth, Access Tokens.
  ncap_auth_t auth_config_obj_;

  // APP API Server Configuration
  ncap_app_api_server_t api_server_obj_;

  // MQTT Broker and Client ID
  ncap_mqtt_t mqtt_obj_;

  // IoT Datasource Listener
  ncap_data_source_subscription_t ds_sub_obj_;

  // Protocol Mapping -> SDL Device ID
  // controller_id/agent_id/node_id
  std::string sdl_controller_id_;
  std::string sdl_agent_id_;
  std::string sdl_node_id_;
  std::string sdl_device_id_;

  // Supervisor / App creates the thread.
  std::shared_ptr<std::thread> mmb_wrapper_thread_;

  ////////////////////////////
  // Data source subscription clients
  // Websocket clients.
  ////////////////////////////
  std::vector<ncap_ds_ws_subscriber_worker_t> ncap_mqtt_ws_client_workers_vec_;

  ////////////////////////////
  // Data processor
  // Process the modbus messages (payload: modbus_mapping)
  // received over masters/controllers.
  ////////////////////////////
  std::shared_ptr<so_5::wrapped_env_t> ncap_ds_env_obj_;
  so_5::mchain_t ds_msg_primary_channel_;
  so_5::mchain_t ds_msg_secondary_channel_;
  std::shared_ptr<std::thread> mmb_ds_msg_proc_thread_;
  void ncap_ds_message_proc_fn();

  ////////////////////////////
  // MQTT Client Execution environment
  // Connects with the configured broker and maintains
  // connections etc.
  ////////////////////////////
  std::shared_ptr<so_5::wrapped_env_t> mqtt_client_env_obj_;
  so_5::mchain_t mqtt_client_env_primary_channel_;
  so_5::mchain_t mqtt_client_env_secondary_channel_;
  std::shared_ptr<std::thread> mqtt_client_env_thread_;
  void mqtt_client_env_proc_thread_fn();

  // Rnns MQTT context.
  std::shared_ptr<std::thread> mqtt_tls_ws_client_thread_;
  void mqtt_tls_ws_client_thread_fn ( std::shared_ptr<mqtt_ws_tls_client_app_t> app );
  void mqtt_ws_client_thread_fn ( std::shared_ptr<mqtt_ws_client_app_t> app );
  std::shared_ptr<mqtt_ws_tls_client_app_t> mqtt_wss_client_context_sptr_;
  std::shared_ptr<mqtt_ws_client_app_t> mqtt_ws_client_context_sptr_;

  std::shared_ptr<std::mutex>   mutex_;
  std::shared_ptr<std::unique_lock<std::mutex>>  lock_;
  std::shared_ptr<std::condition_variable> condition_;

  ////////////////////////////
  // Prepare Device Id controller_id#agent_id#node_id
  ////////////////////////////
  std::string prepare_device_id(const std::string& controller_id, const std::string& agent_id, const std::string& node_id);

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

#endif /* _PM_ODP_NCAP_MQTT_BACKEND_H_ */
