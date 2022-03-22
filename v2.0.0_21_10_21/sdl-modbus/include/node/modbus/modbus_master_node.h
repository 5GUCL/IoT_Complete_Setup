/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_MODBUS_MASTER_NODE_H_
#define _CS_EDM_MODBUS_MASTER_NODE_H_

/* Standard C++ includes */
#include <memory>
#include <mutex>
#include <map>
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
//                 MASTER -> SLAVE DEVICE NODES                       //
////////////////////////////////////////////////////////////////////////

struct modbus_slave_device_t {

  modbus_slave_device_t()
  {
    jthing_ = {};
  }
  ~modbus_slave_device_t() { }

  // Modbus Master parameters
  int             mb_slave_id_;
  std::string     mb_slave_address_;
  int             mb_slave_port_;
  int             mb_refresh_interval_ms_;

  std::map<mb_coil_block_number_tt, mb_coil_block_t> mb_mapping_coil_blocks_;
  std::map<mb_input_bit_block_number_tt, mb_input_bit_block_t> mb_mapping_input_bit_blocks_;
  std::map<mb_input_register_block_number_tt, mb_input_register_block_t> mb_mapping_input_register_blocks_;
  std::map<mb_holding_register_block_number_tt, mb_holding_register_block_t> mb_mapping_holding_register_blocks_;

  // Measurement webhook URL
  std::string     measurement_pub_api_url_;

  // Thing descriptors
  std::string     identifier_;
  std::string     description_;
  std::string     url_;

  // Details of the Thing Descriptor
  nlohmann::json  jthing_;
};

// Agent shall handle a poll device request for events (@device)
struct msg_modbus_poll_slave_device_events_t { };

// Agent shall handle a write single coil request
struct msg_modbus_write_single_coil_request_t {

  // Message constructor.
  msg_modbus_write_single_coil_request_t (long msg_idx   // Used by the application to identify the req-resp
    , int function_code
    , int block_id
    , int mb_mapping_coil_address
    , int output_value)
  {
    msg_idx_ = msg_idx;
    write_coil_req_.function_code_ = function_code;
    write_coil_req_.block_id_ = block_id;
    write_coil_req_.mb_mapping_coil_address_ = mb_mapping_coil_address;
    write_coil_req_.output_value_ = output_value;
  }

  long msg_idx_;

  // The message is of the format mb_write_single_coil_t
  mb_write_single_coil_t write_coil_req_;
};

// Agent shall handle block reads ...
struct msg_modbus_block_read_request_t {
  msg_modbus_block_read_request_t (int block_id)
  {
    block_id_ = block_id;
  }
  int block_id_;
};


////////////////////////////////////////////////////////////////////////

/* Modbus node */
// This class MUST be called from a single thread only.
struct modbus_master_node_t {
  modbus_master_node_t
  (const std::string& controller_id
    , const std::string& agent_id, const std::string& agent_type
    , const std::string& agent_name, const std::string& agent_address
    , const std::string& agent_api_key, const std::string& agent_logger_name
    , const std::string& node_id, const std::string& node_type
    , const std::string& node_name, const std::string& node_address, const std::string& node_api_key);

  virtual ~modbus_master_node_t();
  modbus_master_node_t& init_modbus_node();
  modbus_master_node_t& prepare_modbus_node_events();
  modbus_master_node_t& print_node();
  std::shared_ptr<std::vector<modbus_node_event_t>> get_modbus_node_events();

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

  modbus_slave_device_t mb_slave_device_;

  // Thread to process slave device connection, and messages.
  void slave_device_processing_fn();
  std::shared_ptr<std::thread> mb_mthread_;

  // Modbus master agent event processor
  std::shared_ptr<so_5::wrapped_env_t> mb_slave_device_env_obj_;
  so_5::mchain_t mb_primary_channel_;
  so_5::mchain_t mb_secondary_channel_;
  so_5::mchain_t mb_device_write_single_coil_channel_;
  so_5::mchain_t mb_block_read_coils_channel_;
  so_5::mchain_t mb_block_read_input_bits_channel_;
  so_5::mchain_t mb_block_read_holding_registers_channel_;
  so_5::mchain_t mb_block_read_input_registers_channel_;

  so_5::timer_id_t mb_poll_timer_;

  // Modbus Function Messages
  int post_write_single_coil_async(long msg_idx // Used by the application to identify the req-resp
    , int function_code
    , int block_id
    , int mb_mapping_coil_address
    , int output_value) const;
};

#endif /* _CS_EDM_MODBUS_MASTER_NODE_H_ */