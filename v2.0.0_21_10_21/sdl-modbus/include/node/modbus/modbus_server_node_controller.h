/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_MODBUS_SERVER_NODE_CONTROLLER_H_
#define _CS_EDM_MODBUS_SERVER_NODE_CONTROLLER_H_

#include <thread>
#include <vector>
#include <memory>
#include <random>

#include <modbus/modbus.h>
// SObjectizer is one of a few cross-platform and open source "actor frameworks" for C++.
// But SObjectizer supports not only Actor Model, but also Publish-Subscribe Model and CSP-like channels.
#include <so_5/all.hpp>

// JSON library
#include "nlohmann/json.hpp"

// REST API Server
#include "uWebSockets/App.h"

// Logging
#include "spdlog/spdlog.h"

// Agents
#include "node/modbus/modbus_node_agent.h"
#include "node/modbus/modbus_node_event.h"

// Forward declaration.
struct app_supervisor_t;

/* ws->getUserData returns this */
struct modbus_server_controller_per_socket_data_t {
  std::string_view user_secure_token;
};


/* uWebSocket worker runs in a separate thread */
struct modbus_server_controller_api_worker_t {
  void work();

  /* uWebSocket worker listens on separate port, or share the same port (works on Linux). */
  struct us_listen_socket_t *listen_socket_;

  /* Every thread has its own Loop, and uWS::Loop::get() returns the Loop for current thread.*/
  struct uWS::Loop *loop_;

  /* Need to capture the uWS::App object (instance). */
  std::shared_ptr<uWS::App> app_;

  /* Thread object for uWebSocket worker */
  std::shared_ptr<std::thread> thread_;

  // Every worker "knows' other workers
  std::shared_ptr<std::vector<modbus_server_controller_api_worker_t>> all_api_workers_;
  std::string api_server_listen_address_;
  int api_server_listen_port_;

  so_5::mchain_t http_post_channel_;

  std::string controller_logger_name_;
};

// Holds post data from field devices.
struct modbus_server_controller_http_post_mailbox_message_t {
  std::string msg_posted_;
};

// Controller mailbox message
struct modbus_server_controller_mailbox_message_t { };


/* Edge device manager worker / node agent runs in a separate thread */
struct modbus_server_node_controller_t {
  void wrapper_fn();
  void init_contorller();
  void agent_event_processor_fn();
  void node_api_and_agent_processor_fn();

  modbus_server_node_controller_t
  (const std::string& ctrl_id, const std::string& controller_logger_name, const std::string& api_server_listen_address, int api_server_listen_port);

  void print_config()
  {
    std::cout<< jnode_full_config_.dump() << std::endl;
  }

  // ID
  std::string controller_id_;
  std::string controller_logger_name_;
  // Node Agents
  std::vector<modbus_server_node_agent_t> mb_server_node_agents_vec_;

  // Node Config or descriptor. It has all the info.
  // There exists a copy of full config with the node registry.
  nlohmann::json jnode_full_config_;

  // Modbus Master config parameters
  bool          mm_is_master_;
  bool          mb_is_slave_;

  // Node agent event processor
  std::shared_ptr<so_5::wrapped_env_t> mb_server_node_env_obj_;
  so_5::mchain_t msg_primary_channel_;
  so_5::mchain_t msg_secondary_channel_;
  so_5::mchain_t http_post_channel_;

  // Message counter
  int primary_ch_msg_counter_;
  int secondary_ch_msg_counter_;

  /* uWebSocket Websocket and REST API workers. */
  std::vector<modbus_server_controller_api_worker_t> api_workers_;
  std::string api_server_listen_address_;
  int api_server_listen_port_;
  int api_server_threads_;
  /* Main thread objects of the node agent */
  std::shared_ptr<std::thread> nthread_;
  std::shared_ptr<std::thread> nwrapper_thread_;

};

#endif /* _CS_EDM_MODBUS_SERVER_NODE_CONTROLLER_H_ */
