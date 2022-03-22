/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_H_DATA_HDF5_MB_RECORDER_H_
#define _CS_EDM_H_DATA_HDF5_MB_RECORDER_H_

#include <vector>
#include <map>
#include <iostream>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <string>
#include <sstream>
#include <thread>

#include <hdf5.h>
#include <hdf5_hl.h>

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

// Forward declaration.
struct app_supervisor_t;

//
// DESIGN NOTES
// For each controller, there exists a Web Socket client to recieve the data from modbus devices.
//

static const std::string HDF5_COILS_TABLE_NAME                  = "coils_measurements";                // FC 1
static const std::string HDF5_INPUT_BITS_TABLE_NAME             = "input_bits_measurements";           // FC 2
static const std::string HDF5_HOLDING_REGISTERS_TABLE_NAME      = "holding_registers_measurements";    // FC 3
static const std::string HDF5_INPUT_REGISTERS_TABLE_NAME        = "input_registers_measurements";      // FC 4

struct mb_node_master_dataset_ws_client_t {
  mb_node_master_dataset_ws_client_t(const std::string& _url
    , std::shared_ptr<so_5::wrapped_env_t> ds_rec_env_obj
    , so_5::mchain_t& msg_primary_channel
    , so_5::mchain_t msg_secondary_channel);

  std::string truncate(const std::string& str, size_t n);
  bool run();

  std::string   controller_url_;
  ix::WebSocket dataset_ws_;

  std::mutex    mutex_;
  std::condition_variable condition_;

  std::shared_ptr<so_5::wrapped_env_t> ds_rec_env_obj_;
  so_5::mchain_t msg_primary_channel_;
  so_5::mchain_t msg_secondary_channel_;

};

// Mailbox message data ...
struct dataset_recorder_mailbox_message_t {
  std::string msg_received_;
};

// Message subscriber over Websockers / OPC-UA etc.
/* uWebSocket worker runs in a separate thread */
struct mb_node_master_dataset_ws_client_worker_t {

  void work();

  //
  std::shared_ptr<so_5::wrapped_env_t> ds_rec_env_obj_;
  so_5::mchain_t msg_primary_channel_;
  so_5::mchain_t msg_secondary_channel_;

  std::shared_ptr<std::thread> ws_thread_;

  //
  std::string mb_controller_id_;
  std::string mb_master_contoller_address_;
  int mb_master_contoller_port_;
};


struct mb_node_master_dataset_recorder_main_t {

  const char *SMART_LOGGER_H5_DATASET_FILE_         = "../datasets/mb_master_data_recorder_v1.0.7.hdf5";
  const char *SMART_LOGGER_H5_BACKUP_DATASET_FILE_  = "../datasets/mb_master_data_recorder_v1.0.7_";
  const char *SMART_LOGGER_H5_DATASET_FILE_DIR_     = "../datasets/";

  const int   SMART_LOGGER_H5_DS_RANK_  = 2;
  const int   H5_NUM_MEASUREMENTS_      = 1;

  const std::string SMART_LOGGER_H5_LOGGER_CODE_ = "mb-recorders";
  const std::string SMART_LOGGER_H5_LOGGER_NAME_ = "mb-hdf5";

  mb_node_master_dataset_recorder_main_t (std::shared_ptr<app_supervisor_t> that_supervisor);


  // System Registry Objects
  std::shared_ptr<app_supervisor_t> that_supervisor_;
  std::shared_ptr<modbus_node_registry_t> mb_node_registry_;
  std::shared_ptr<logger_registry_t> logger_registry_;

  void init_h5_service_loggers(const std::string& controller_logger_code, const std::string& logger_name);

  int init_h5_dataset_file();

  int init_mb_dataset_recorder();


  // HDF5 Grouping Key is of the format ->
  // controller_id/agent_id/node_id/modbus_mapping <input register, holding register, input bits, coils> / block id/ start_address/<register, bit number>

  ////////////////////////////
  // Data processor
  // Process the modbus messages received over masters/controllers.
  ////////////////////////////
  void message_recording_fn();
  std::shared_ptr<so_5::wrapped_env_t> ds_rec_env_obj_;
  so_5::mchain_t msg_primary_channel_;
  so_5::mchain_t msg_secondary_channel_;
  std::shared_ptr<std::thread> dsthread_;

  std::vector<mb_node_master_dataset_ws_client_worker_t> dataset_ws_client_workers_vec_;

  ////////////////////////////
  // HDF5 Helper functions
  ////////////////////////////
  bool prepare_root_group(const char *root_group);
  bool prepare_child_group(const char *this_group, const char *new_group);

  int backup_dataset_file_async() const;

  int backup_and_no_truncate_fn();

  ////////////////////////////
  // Prepare Device Id controller_id#agent_id#node_id
  ////////////////////////////
  std::string prepare_device_id(const std::string& controller_id, const std::string& agent_id, const std::string& node_id);

  ////////////////////////////
  // Prepare HDF5 Grouping
  ////////////////////////////
  std::string prepare_mb_register_mapping_h5_group_id
  (const std::string& v_device_id, const std::string& mb_mapping_type,  int block_id, int start_address, int reg_number);

  std::string prepare_mb_coil_and_input_bits_mapping_h5_group_id
  (const std::string& v_device_id, const std::string& mb_mapping_type,  int block_id, int start_address, int bit_number);

  int record_mb_coil_measurement
  (const std::string& v_device_id, int block_id, int start_address
    , int coil_number, const h5_modbus_master_coil_bit_single_measurement_instance_t& measurement);

  int record_mb_input_bit_measurement
  (const std::string& v_device_id, int block_id, int start_address
    , int input_bit_number, const h5_modbus_master_input_bit_single_measurement_instance_t& measurement);

  int record_mb_holding_register_measurement
  (const std::string& v_device_id, int block_id, int start_address
    , int holding_reg_number, const h5_modbus_master_holding_register_single_measurement_instance_t& measurement);

  int record_mb_input_register_measurement
  (const std::string& v_device_id, int block_id, int start_address
    , int input_reg_number, const h5_modbus_master_input_register_single_measurement_instance_t& measurement);
};


#endif /* _CS_EDM_H_DATA_HDF5_MB_RECORDER_H_ */