/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_MB_NODE_MASTER_IMGUI_MAIN_H_
#define _CS_EDM_MB_NODE_MASTER_IMGUI_MAIN_H_

#include <vector>
#include <iostream>
#include <map>
#include <mutex>

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

#include "app-gui/app_imgui_opengl_inc.h"

// MODBUS PROTOCOL STRUCTURES
#include "node/modbus/modbus_protocol.h"

// Forward declaration.
struct app_supervisor_t;

struct mb_master_node_gui_t {
  mb_master_node_gui_t () {}
  ~mb_master_node_gui_t () {}

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

  // Controller Identifier
  std::string contoller_id_;
  std::string type_;
  std::string mb_master_contoller_address_;
  int mb_master_contoller_port_;

  // Agent Identifier
  std::string agent_id_;

  // Node Identifier
  std::string node_id_;
  std::string node_name_;

};

// Mailbox message data ...
struct modbus_master_gui_mailbox_message_t {
  std::string msg_received_;
};


// Message subscriber over Websockers / OPC-UA etc.
/* uWebSocket worker runs in a separate thread */
struct mb_node_master_ws_subscriber_worker_t {

  void work();

  //
  std::shared_ptr<so_5::wrapped_env_t> mb_master_gui_env_obj_;
  so_5::mchain_t msg_primary_channel_;
  so_5::mchain_t msg_secondary_channel_;

  std::shared_ptr<std::thread> ws_thread_;

  //
  std::string mb_master_contoller_address_;
  int mb_master_contoller_port_;
};

// This is used for register type measurments.
// Forward declaration
struct mb_node_master_imgui_components_main_t;

struct ScrollingBuffer {
  int MaxSize;
  int Offset;
  ImVector<ImVec2> Data;
  //
  ScrollingBuffer(int max_size = mb_last_nth_minute_data_consts_te::max_nth_minute_data_size)
  {
    MaxSize = max_size;
    Offset  = 0;
    Data.reserve(MaxSize);

    for(int d = 0; d < mb_last_nth_minute_data_consts_te::max_nth_minute_data_size; d++) {
      AddPoint(0.0, 0.0);
    }
  }
  void AddPoint(float x, float y)
  {
    if(Data.size() < MaxSize)
      Data.push_back(ImVec2(x,y));
    else {
      Data[Offset] = ImVec2(x,y);
      Offset = (Offset + 1) % MaxSize;
    }
  }
  void Erase()
  {
    if(Data.size() > 0) {
      Data.shrink(0);
      Offset  = 0;
    }
  }
};

struct mb_node_master_imgui_callback_input_register_data_context_t {
  mb_node_master_imgui_callback_input_register_data_context_t ()
  {
    mb_mapping_type = mb_mapping_types_te::INPUT_REGISTERS;
    mb_ts_data_ = new ScrollingBuffer();
  }

  float on_nth_minute_data_y_at_idx(size_t idx)
  {
    float m = 0.0f;

    if(idx > 0 && idx < mb_last_nth_minute_data_consts_te::max_nth_minute_data_size) {
      m = mb_ts_data_->Data[idx].y;
    }

    return m;
  }

  float on_nth_minute_data_x_at_idx(size_t idx)
  {
    float m = 0.0f;

    if(idx > 0 && idx < mb_last_nth_minute_data_consts_te::max_nth_minute_data_size) {
      m = mb_ts_data_->Data[idx].x;
    }

    return m;
  }

  int get_nth_minute_data_size()
  {
    return mb_last_nth_minute_data_consts_te::max_nth_minute_data_size;
  }

  ScrollingBuffer* mb_ts_data_;

  mb_node_master_imgui_components_main_t* mb_node_master_imgui_main_;
  mb_mapping_types_te mb_mapping_type;
  int block_id_;
  int start_address_;
  int register_number_;
  std::string device_id_;
};

struct mb_node_master_imgui_callback_holding_register_data_context_t {
  mb_node_master_imgui_callback_holding_register_data_context_t ()
  {
    mb_mapping_type = mb_mapping_types_te::HOLDING_REGISTERS;
    mb_ts_data_ = new ScrollingBuffer();
  }

  float on_nth_minute_data_y_at_idx(size_t idx)
  {
    float m = 0.0f;

    if(idx > 0 && idx < mb_last_nth_minute_data_consts_te::max_nth_minute_data_size) {
      m = mb_ts_data_->Data[idx].y;
    }

    return m;
  }

  float on_nth_minute_data_x_at_idx(size_t idx)
  {
    float m = 0.0f;

    if(idx > 0 && idx < mb_last_nth_minute_data_consts_te::max_nth_minute_data_size) {
      m = mb_ts_data_->Data[idx].x;
    }

    return m;
  }

  int get_nth_minute_data_size()
  {
    return mb_last_nth_minute_data_consts_te::max_nth_minute_data_size;
  }

  ScrollingBuffer *mb_ts_data_;

  mb_node_master_imgui_components_main_t* mb_node_master_imgui_main_;
  mb_mapping_types_te mb_mapping_type;
  int block_id_;
  int start_address_;
  int register_number_;
  std::string device_id_;
};

struct mb_node_master_imgui_callback_coils_data_context_t {
  mb_node_master_imgui_callback_coils_data_context_t ()
  {
    mb_mapping_type = mb_mapping_types_te::COILS;
    mb_ts_data_ = new ScrollingBuffer();
  }

  float on_nth_minute_data_y_at_idx(size_t idx)
  {
    float m = 0.0f;

    if(idx > 0 && idx < mb_last_nth_minute_data_consts_te::max_nth_minute_data_size) {
      m = mb_ts_data_->Data[idx].y;
    }

    return m;
  }

  float on_nth_minute_data_x_at_idx(size_t idx)
  {
    float m = 0.0f;

    if(idx > 0 && idx < mb_last_nth_minute_data_consts_te::max_nth_minute_data_size) {
      m = mb_ts_data_->Data[idx].x;
    }

    return m;
  }

  int get_nth_minute_data_size()
  {
    return mb_last_nth_minute_data_consts_te::max_nth_minute_data_size;
  }

  ScrollingBuffer* mb_ts_data_;

  mb_node_master_imgui_components_main_t* mb_node_master_imgui_main_;
  mb_mapping_types_te mb_mapping_type;
  int block_id_;
  int start_address_;
  int coil_number_;
  std::string device_id_;
};

struct mb_node_master_imgui_callback_input_bits_data_context_t {
  mb_node_master_imgui_callback_input_bits_data_context_t ()
  {
    mb_mapping_type = mb_mapping_types_te::INPUT_BITS;
    mb_ts_data_ = new ScrollingBuffer();
  }

  float on_nth_minute_data_y_at_idx(size_t idx)
  {
    float m = 0.0f;

    if(idx > 0 && idx < mb_last_nth_minute_data_consts_te::max_nth_minute_data_size) {
      m = mb_ts_data_->Data[idx].y;
    }

    return m;
  }

  float on_nth_minute_data_x_at_idx(size_t idx)
  {
    float m = 0.0f;

    if(idx > 0 && idx < mb_last_nth_minute_data_consts_te::max_nth_minute_data_size) {
      m = mb_ts_data_->Data[idx].x;
    }

    return m;
  }

  int get_nth_minute_data_size()
  {
    return mb_last_nth_minute_data_consts_te::max_nth_minute_data_size;
  }

  ScrollingBuffer* mb_ts_data_;

  mb_node_master_imgui_components_main_t* mb_node_master_imgui_main_;
  mb_mapping_types_te mb_mapping_type;
  int block_id_;
  int start_address_;
  int input_bit_number_;
  std::string device_id_;
};

struct mb_node_master_imgui_components_main_t {

  mb_node_master_imgui_components_main_t (std::shared_ptr<app_supervisor_t> that_supervisor);
  int init_gui_components();
  int run_gui_components();

  void modbus_master_window();
  void modbus_master_menu();

  int backup_dataset_file_async();

  std::shared_ptr<app_supervisor_t> that_supervisor_;
  std::shared_ptr<modbus_node_registry_t> mb_node_registry_;
  std::shared_ptr<logger_registry_t> logger_registry_;

  // Key is of the format ->
  // controller_id/agent_id/node_id
  std::map<std::string, mb_master_node_gui_t> mb_master_devices_;
  typedef std::map<std::string, mb_master_node_gui_t>::iterator mb_node_master_gui_iterator_tt;

  //
  // Data processor
  //
  // Process the modbus messages received over masters/controllers.
  void mb_message_processing_fn();
  std::shared_ptr<so_5::wrapped_env_t> mb_master_gui_env_obj_;
  so_5::mchain_t msg_primary_channel_;
  so_5::mchain_t msg_secondary_channel_;
  std::shared_ptr<std::thread> gthread_;

  std::vector<mb_node_master_ws_subscriber_worker_t> modbus_master_ws_subscriber_worker_vec_;

  // Prepare Device Id
  std::string prepare_device_id(const std::string& controller_id, const std::string& agent_id, const std::string& node_id);

  // For thread safe operations GUI updations...
  std::shared_ptr<std::mutex> nmutex_;

  // To hold the GUI p_open of ImGUI.
  // Level 1
  bool show_gui_mb_master_devices_[MAX_MODBUS_MASTER_DEVICES_GUI_SUPPORTED] = {false};

  // Level 2 Modbus Device - Descriptive Analytics
  bool show_gui_mb_master_input_register_descriptive_analytics_[MAX_MODBUS_MASTER_DEVICES_GUI_SUPPORTED] = {false};
  bool show_gui_mb_master_holding_register_descriptive_analytics_[MAX_MODBUS_MASTER_DEVICES_GUI_SUPPORTED] = {false};
  bool show_gui_mb_master_coils_bits_descriptive_analytics_[MAX_MODBUS_MASTER_DEVICES_GUI_SUPPORTED] = {false};
  bool show_gui_mb_master_inputs_bits_descriptive_analytics_[MAX_MODBUS_MASTER_DEVICES_GUI_SUPPORTED] = {false};

  // Level 3
  bool show_gui_mb_master_input_registers_[MAX_MODBUS_MASTER_DEVICES_GUI_SUPPORTED][MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED][MAX_MODBUS_MASTER_INPUT_REGISTERS_GUI_SUPPORTED] = {{{false}}};
  bool show_gui_mb_master_holding_registers_[MAX_MODBUS_MASTER_DEVICES_GUI_SUPPORTED][MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED][MAX_MODBUS_MASTER_HOLDING_REGISTERS_GUI_SUPPORTED] = {{{false}}};
  bool show_gui_mb_master_coil_bits_[MAX_MODBUS_MASTER_DEVICES_GUI_SUPPORTED][MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED][MAX_MODBUS_MASTER_COIL_BITS_GUI_SUPPORTED] = {{{false}}};
  bool show_gui_mb_master_input_bits_[MAX_MODBUS_MASTER_DEVICES_GUI_SUPPORTED][MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED][MAX_MODBUS_MASTER_INPUT_BITS_GUI_SUPPORTED] = {{{false}}};

  //
  // Very important to initailize the MAP before the GUI updation.
  //

  /////////////////////////////
  // Input Registers Descriptive Analytics data context
  /////////////////////////////
  std::map<std::string, mb_node_master_imgui_callback_input_register_data_context_t*> mb_node_master_imgui_input_register_callback_ctx_;

  mb_node_master_imgui_callback_input_register_data_context_t*
  get_mb_node_master_imgui_input_register_callback_ctx_fn(const std::string& device_id, int block_id, int start_address, int reg);

  mb_node_master_imgui_callback_input_register_data_context_t*
  add_mb_node_master_imgui_input_register_callback_ctx_fn(const std::string& device_id, int block_id, int start_address, int reg);
  /////////////////////////////

  /////////////////////////////
  // Holding Registers Descriptive Analytics data context
  /////////////////////////////
  std::map<std::string, mb_node_master_imgui_callback_holding_register_data_context_t*> mb_node_master_imgui_holding_register_callback_ctx_;

  mb_node_master_imgui_callback_holding_register_data_context_t*
  get_mb_node_master_imgui_holding_register_callback_ctx_fn(const std::string& device_id, int block_id, int start_address, int reg);

  mb_node_master_imgui_callback_holding_register_data_context_t*
  add_mb_node_master_imgui_holding_register_callback_ctx_fn(const std::string& device_id, int block_id, int start_address, int reg);

  /////////////////////////////
  // COIL Descriptive Analytics data context
  /////////////////////////////
  std::map<std::string, mb_node_master_imgui_callback_coils_data_context_t*> mb_node_master_imgui_coils_data_callback_ctx_;

  mb_node_master_imgui_callback_coils_data_context_t*
  get_mb_node_master_imgui_coils_data_callback_ctx_fn(const std::string& device_id, int block_id, int start_address, int cbit);

  mb_node_master_imgui_callback_coils_data_context_t*
  add_mb_node_master_imgui_coils_data_callback_ctx_fn(const std::string& device_id, int block_id, int start_address, int cbit);

  /////////////////////////////
  // Input BITS Descriptive Analytics data context
  /////////////////////////////
  std::map<std::string, mb_node_master_imgui_callback_input_bits_data_context_t*> mb_node_master_imgui_input_bits_data_callback_ctx_;

  mb_node_master_imgui_callback_input_bits_data_context_t*
  get_mb_node_master_imgui_input_bits_data_callback_ctx_fn(const std::string& device_id, int block_id, int start_address, int ibit);

  mb_node_master_imgui_callback_input_bits_data_context_t*
  add_mb_node_master_imgui_input_bits_data_callback_ctx_fn(const std::string& device_id, int block_id, int start_address, int ibit);

  ///////////////////////////////////

  // ImPlot Utils
  void set_style_seaborn();
};


#endif /* _CS_EDM_MB_NODE_MASTER_IMGUI_MAIN_H_ */