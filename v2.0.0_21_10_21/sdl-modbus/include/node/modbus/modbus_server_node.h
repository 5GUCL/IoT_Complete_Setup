/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_MODBUS_SERVER_NODE_H_
#define _CS_EDM_MODBUS_SERVER_NODE_H_

/* Standard C++ includes */
#include <memory>
#include <mutex>
#include <map>

#include <error.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cpr/cpr.h>
#include <modbus/modbus.h>
#include <so_5/all.hpp>

// JSON library
#include "nlohmann/json.hpp"

#include "sf-sys/utils/cpp_prog_utils.h"
#include "node/modbus/modbus_node_event.h"

// MODBUS PROTOCOL STRUCTURES
#include "node/modbus/modbus_protocol.h"


////////////////////////////////////////////////////////////////////////
//                         SERVER NODES                               //
////////////////////////////////////////////////////////////////////////

struct modbus_server_node_device_t {

  modbus_server_node_device_t();
  ~modbus_server_node_device_t();

  // Modbus Server Node parameters
  int             mb_slave_id_;
  std::string     mb_device_address_;
  int             mb_device_port_;
  int             mb_connections_;
  std::string     mb_role_;

  std::map<mb_coil_block_number_tt, mb_coil_block_t> mb_mapping_coil_blocks_;
  std::map<mb_input_bit_block_number_tt, mb_input_bit_block_t> mb_mapping_input_bit_blocks_;
  std::map<mb_holding_register_block_number_tt, mb_holding_register_block_t> mb_mapping_holding_register_blocks_;
  std::map<mb_input_register_block_number_tt, mb_input_register_block_t> mb_mapping_input_register_blocks_;


  bool              run_mb_server_;
  int               mb_s_;
  modbus_t          *mb_server_ctx_;

  // Thing descriptors
  std::string     identifier_;
  std::string     description_;
  std::string     url_;

  // Details of the Thing Descriptor
  nlohmann::json  jthing_;
};

// Used by controller and agent.
struct modbus_server_mailbox_message_t { };

struct modbus_server_node_t {
  modbus_server_node_t
  (const std::string& controller_id
    , const std::string& agent_id, const std::string& agent_type
    , const std::string& agent_name, const std::string& agent_address
    , const std::string& agent_api_key, const std::string& agent_logger_name
    , const std::string& node_id, const std::string& node_type
    , const std::string& node_name, const std::string& node_address, const std::string& node_api_key);

  virtual ~modbus_server_node_t();
  modbus_server_node_t& init_modbus_node();
  modbus_server_node_t& prepare_modbus_node_events();
  modbus_server_node_t& print_node();
  std::shared_ptr<std::vector<modbus_node_event_t>> get_modbus_node_events();
  void reset_modbus_server_fn(int code);
  // Init state variable
  bool mb_node_init_status_;

  // Initializer list for agent
  std::string controller_id_;

  // Initializer list for agent
  std::string agent_id_;
  std::string agent_type_;
  std::string agent_name_;
  std::string agent_address_;
  std::string agent_api_key_;
  std::string agent_logger_name_;

  // Initializer list for Node that an agent manages
  std::string node_id_;
  std::string node_type_;
  std::string node_name_;
  std::string node_address_;
  std::string node_api_key_;

  // Node Events
  std::vector<modbus_node_event_t> node_events_;

  modbus_server_node_device_t mb_server_node_device_;

  // Thread to process slave device connection, and messages.
  void mb_server_node_processing_fn();
  std::shared_ptr<std::thread> mb_mthread_;

  // Modbus master agent event processor
  std::shared_ptr<so_5::wrapped_env_t> mb_server_node_env_obj_;
  so_5::mchain_t mb_primary_channel_;
  so_5::mchain_t mb_secondary_channel_;

};

#endif /* _CS_EDM_MODBUS_SERVER_NODE_H_ */