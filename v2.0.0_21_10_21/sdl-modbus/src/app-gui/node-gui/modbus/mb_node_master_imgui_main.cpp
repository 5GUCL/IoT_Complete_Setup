/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#include <iostream>
#include <sstream>

#include "sf-sys/utils/cpp_prog_utils.h"
#include "meta-data-models/thing.h"
#include "node/modbus/modbus_master_node.h"
#include "launcher/node/modbus/mb_controllers_launcher.h"
#include "h-data/hdf5/node/modbus/mb_node_master_hdf5_recorder.h"
#include "launcher/node/modbus/mb_node_master_hdf5_recorder_launcher.h"
#include "launcher/node/modbus/mb_node_master_imgui_launcher.h"
#include "app-gui/node-gui/modbus/mb_node_master_gui_ws_client.h"
#include "app-gui/node-gui/modbus/mb_node_master_imgui_main.h"
#include "supervisor/supervisor.h"
#include "implot/implot.h"

void mb_node_master_ws_subscriber_worker_t::work()
{
  std::stringstream ss;
  ss << "ws://" << mb_master_contoller_address_ << ":" << mb_master_contoller_port_;

  mb_node_master_gui_ws_client_t ws_client(ss.str(), mb_master_gui_env_obj_, msg_primary_channel_, msg_secondary_channel_);

  // This call doesn't return.
  ws_client.run();
}


///////////////////////////////////
void mb_node_master_imgui_components_main_t::mb_message_processing_fn()
{

  // This flag will be set to 'true' when some of channels will be closed.
  bool stop = false;
  auto prepared = so_5::prepare_select(
      so_5::from_all().handle_all()
      .empty_timeout(std::chrono::hours(1))
      // If some channel become closed we should set out 'stop' flag.
  .on_close([&stop](const auto &) {
    stop = true;
  })
  // A predicate for stopping select() function.
  .stop_on([&stop] { return stop; }),

  // Read and handle poll_device_events_request messages from channel_.
  so_5::receive_case(msg_primary_channel_
  , [this](modbus_master_gui_mailbox_message_t mb_msg) {
    //
    // DESIGN NOTES
    // Store the data received.
    // Device[identifier] -> Register [number] -> [measurement instamce]
    //
    nmutex_->lock();
    on_scope_exit unlock_mutex {
      [this] {
        nmutex_->unlock();
      }
    };

    try {
      nlohmann::json jmsg = nlohmann::json::parse(mb_msg.msg_received_);
      std::string _msg_type = jmsg["message_type"];

      ///////////////////////////////////////
      if(_msg_type.compare("coil_bit_reading") == 0) {

        std::string _contoller_id = jmsg["controller_id"];
        std::string _agent_id = jmsg["agent_id"];
        std::string _node_id = jmsg["node_id"];

        std::string v_id;
        v_id = prepare_device_id(_contoller_id, _agent_id, _node_id);

        // We found a device that the incoming message belongs to.
        modbus_master_coil_bit_single_measurement_instance_t cm;
        cm.mb_block_id_ = jmsg["block_id"];
        cm.mb_mapping_bits_start_address_ = jmsg["coil_bits_start_address"];
        cm.mb_mapping_bit_number_ = jmsg["coil_bit_number"];
        cm.measured_value_ = jmsg["measured_value"];
        cm.time_instance_  = jmsg["time_instance"];
        
        cm.eu_factor_ = jmsg["eu_factor"];
        cm.eu_quantity_ = jmsg["eu_quantity"];
        cm.eu_manifest_constant_ = jmsg["eu_manifest_constant"];

        mb_node_master_imgui_callback_coils_data_context_t*
        data_ctx = get_mb_node_master_imgui_coils_data_callback_ctx_fn
          (v_id, cm.mb_block_id_, cm.mb_mapping_bits_start_address_, cm.mb_mapping_bit_number_);

        if(data_ctx) {
          data_ctx->mb_ts_data_->AddPoint(cm.time_instance_, cm.measured_value_);
        }
      }
      ///////////////////////////////////////
      else if(_msg_type.compare("input_bit_reading") == 0) {

        std::string _contoller_id = jmsg["controller_id"];
        std::string _agent_id = jmsg["agent_id"];
        std::string _node_id = jmsg["node_id"];

        std::string v_id;
        v_id = prepare_device_id(_contoller_id, _agent_id, _node_id);

        // We found a device that the incoming message belongs to.
        modbus_master_input_bit_single_measurement_instance_t ibm;
        ibm.mb_block_id_ = jmsg["block_id"];
        ibm.mb_mapping_input_bits_start_address_ = jmsg["input_bits_start_address"];
        ibm.mb_mapping_input_bit_number_ = jmsg["input_bit_number"];
        ibm.measured_value_ = jmsg["measured_value"];
        ibm.time_instance_  = jmsg["time_instance"];

        ibm.eu_factor_ = jmsg["eu_factor"];
        ibm.eu_quantity_ = jmsg["eu_quantity"];
        ibm.eu_manifest_constant_ = jmsg["eu_manifest_constant"];

        mb_node_master_imgui_callback_input_bits_data_context_t*
        data_ctx = get_mb_node_master_imgui_input_bits_data_callback_ctx_fn
          (v_id, ibm.mb_block_id_, ibm.mb_mapping_input_bits_start_address_, ibm.mb_mapping_input_bit_number_);

        if(data_ctx) {
          data_ctx->mb_ts_data_->AddPoint(ibm.time_instance_, ibm.measured_value_);
        }
      }
      ////////////////////////////////////////
      else if(_msg_type.compare("input_register_reading") == 0) {

        std::string _contoller_id = jmsg["controller_id"];
        std::string _agent_id = jmsg["agent_id"];
        std::string _node_id = jmsg["node_id"];

        std::string v_id;
        v_id = prepare_device_id(_contoller_id, _agent_id, _node_id);

        // We found a device that the incoming message belongs to.
        modbus_master_input_register_single_measurement_instance_t im;
        im.mb_block_id_ = jmsg["block_id"];
        im.mb_mapping_input_registers_start_address_ = jmsg["input_registers_start_address"];
        im.mb_mapping_input_register_number_ = jmsg["input_register_number"];
        im.measured_value_ = jmsg["measured_value"];
        im.time_instance_  = jmsg["time_instance"];

        im.eu_factor_ = jmsg["eu_factor"];
        im.eu_quantity_ = jmsg["eu_quantity"];
        im.eu_manifest_constant_ = jmsg["eu_manifest_constant"];

        mb_node_master_imgui_callback_input_register_data_context_t*
        data_ctx = get_mb_node_master_imgui_input_register_callback_ctx_fn
          (v_id, im.mb_block_id_, im.mb_mapping_input_registers_start_address_, im.mb_mapping_input_register_number_);

        if(data_ctx) {
          data_ctx->mb_ts_data_->AddPoint(im.time_instance_, im.measured_value_);
        }
      }
      ////////////////////////////////////////
      else if(_msg_type.compare("holding_register_reading") == 0) {

        std::string _contoller_id = jmsg["controller_id"];
        std::string _agent_id = jmsg["agent_id"];
        std::string _node_id = jmsg["node_id"];

        std::string v_id;
        v_id = prepare_device_id(_contoller_id, _agent_id, _node_id);

        // We found a device that the incoming message belongs to.
        modbus_master_holding_register_single_measurement_instance_t hm;
        hm.mb_block_id_ = jmsg["block_id"];
        hm.mb_mapping_registers_start_address_ = jmsg["registers_start_address"];
        hm.mb_mapping_holding_register_number_ = jmsg["holding_register_number"];
        hm.measured_value_ = jmsg["measured_value"];
        hm.time_instance_  = jmsg["time_instance"];

        hm.eu_factor_ = jmsg["eu_factor"];
        hm.eu_quantity_ = jmsg["eu_quantity"];
        hm.eu_manifest_constant_ = jmsg["eu_manifest_constant"];
        
        mb_node_master_imgui_callback_holding_register_data_context_t*
        data_ctx = get_mb_node_master_imgui_holding_register_callback_ctx_fn
          (v_id, hm.mb_block_id_,  hm.mb_mapping_registers_start_address_, hm.mb_mapping_holding_register_number_);

        if(data_ctx) {
          data_ctx->mb_ts_data_->AddPoint(hm.time_instance_, hm.measured_value_);
        }
      }
      ////////////////////////////////////////
      else {
        // Logging to be added.
        // std::cout << "FIXME Log @receive_case @mb_msg.msg_received_" << "Invalid Mapping Type received..." << std::endl;
      }
    }
    catch(nlohmann::json::exception& e) {
      // Log exception information
      std::stringstream ssReqException;
      ssReqException  << "message: " << e.what()    <<  " "
        << "exception id: " << e.id   << std::endl;
      // Logging to be added.
      // std::cout << "FIXME Log @receive_case @mb_msg.msg_received_" << ssReqException.str() << std::endl;
    }
  }),
  so_5::receive_case(msg_secondary_channel_
  , [](modbus_master_gui_mailbox_message_t) {

  }
  ));

  do {
    so_5::select(prepared);
  }
  while(!stop);
}


mb_node_master_imgui_components_main_t::mb_node_master_imgui_components_main_t (std::shared_ptr<app_supervisor_t> that_supervisor)
  : nmutex_(new std::mutex)
{
  that_supervisor_  = that_supervisor;
  mb_node_registry_ = that_supervisor_->modbus_node_registry_;
  logger_registry_  = that_supervisor_->logger_registry_;
}


int mb_node_master_imgui_components_main_t::init_gui_components()
{

  // Read the global config / node registry.
  // Step 1: Check if the controller is modbus-master.
  // Step 2: modbus-server will be handled in future versions.
  // Get each node = controller -> agent -> node -> modbus_master_node

  for(auto strnode : mb_node_registry_->node_registry_) {
    try {
      nlohmann::json jnode = nlohmann::json::parse(strnode);

      std::string controller = jnode["controller"]["type"];

      // Controller - MODBUS-MASTER
      if(controller.compare("modbus-master") == 0) {
        nlohmann::json jagents = jnode["controller"]["agents"];

        for(nlohmann::json::iterator ait = jagents.begin(); ait != jagents.end(); ++ait) {
          nlohmann::json jagent(*ait);

          // Modbus device GUI object
          mb_master_node_gui_t v1 = mb_master_node_gui_t();

          // Controller Identifier
          v1.contoller_id_ = jnode["controller"]["id"];
          v1.type_ = jnode["controller"]["type"];
          v1.mb_master_contoller_address_ = jnode["controller"]["attributes"]["api_server_listen_address"];
          v1.mb_master_contoller_port_    = jnode["controller"]["attributes"]["api_server_listen_port"];

          // Agent Identifier
          v1.agent_id_ = jagent["agent"]["id"];

          // Node Identifier
          v1.node_id_ = jagent["agent"]["node"]["id"];
          v1.node_name_ = jagent["agent"]["node"]["attributes"]["node_name"];

          // Modbus Master parameters
          v1.mb_slave_id_       = jagent["agent"]["node"]["modbus_master"]["slave_device"]["slave_id"];
          v1.mb_slave_address_  = jagent["agent"]["node"]["modbus_master"]["slave_device"]["slave_address"];
          v1.mb_slave_port_     = jagent["agent"]["node"]["modbus_master"]["slave_device"]["slave_port"];
          v1.mb_refresh_interval_ms_ = jagent["agent"]["node"]["modbus_master"]["slave_device"]["refresh_interval_ms"];
        
          // Key is of the format ->
          // controller_id/agent_id/node_id
          std::string v_id;
          v_id = prepare_device_id(v1.contoller_id_, v1.agent_id_, v1.node_id_);

          // Read the coil blocks.
          {
            nlohmann::json jcoil_blocks = jagent["agent"]["node"]["modbus_master"]["slave_device"]["modbus_mapping"]["coil_blocks"];

            for(nlohmann::json::iterator coilit = jcoil_blocks.begin(); coilit != jcoil_blocks.end(); ++coilit) {
              nlohmann::json jcb(*coilit);
              mb_coil_block_t cb;
              cb.block_id_ = jcb["block_id"];
              cb.mb_mapping_bits_start_address_ = jcb["bits_start_address"];
              cb.mb_mapping_nb_bits_ = jcb["nb_bits"];
              cb.mb_block_refresh_interval_ = jcb["block_refresh_interval_ms"];

              if((cb.mb_block_refresh_interval_ != 0) && (cb.mb_block_refresh_interval_ < 1000)) {
                cb.mb_block_refresh_interval_ = 1000;
              }

              if(cb.mb_block_refresh_interval_ == 0) {
                cb.mb_block_refresh_interval_ = v1.mb_refresh_interval_ms_;
              }
                
              cb.eu_factor_ = jcb["eu_factor"];
              cb.eu_quantity_ = jcb["eu_quantity"];
              cb.eu_manifest_constant_ = jcb["eu_manifest_constant"];
              
              v1.mb_mapping_coil_blocks_.try_emplace(cb.block_id_, std::move(cb));

              // Now prepare GUI data structures.
              for(int r = 0; r < cb.mb_mapping_nb_bits_; r++) {
                add_mb_node_master_imgui_coils_data_callback_ctx_fn(v_id, cb.block_id_, cb.mb_mapping_bits_start_address_, r);
                // Log Here
              }

            }
          }
          // Read the input bit blocks.
          {
            nlohmann::json jibit_blocks = jagent["agent"]["node"]["modbus_master"]["slave_device"]["modbus_mapping"]["input_bits_blocks"];

            for(nlohmann::json::iterator ibit = jibit_blocks.begin(); ibit != jibit_blocks.end(); ++ibit) {
              nlohmann::json jib(*ibit);
              mb_input_bit_block_t ib;
              ib.block_id_ = jib["block_id"];
              ib.mb_mapping_input_bits_start_address_ = jib["input_bits_start_address"];
              ib.mb_mapping_nb_input_bits_ = jib["nb_input_bits"];
              ib.mb_block_refresh_interval_ = jib["block_refresh_interval_ms"];

              if((ib.mb_block_refresh_interval_ != 0) && (ib.mb_block_refresh_interval_ < 1000)) {
                ib.mb_block_refresh_interval_ = 1000;
              }

              if(ib.mb_block_refresh_interval_ == 0) {
                ib.mb_block_refresh_interval_ = v1.mb_refresh_interval_ms_;
              }

              ib.eu_factor_ = jib["eu_factor"];
              ib.eu_quantity_ = jib["eu_quantity"];
              ib.eu_manifest_constant_ = jib["eu_manifest_constant"];
              
              v1.mb_mapping_input_bit_blocks_.try_emplace(ib.block_id_, std::move(ib));

              // Now prepare GUI data structures.
              for(int r = 0; r < ib.mb_mapping_nb_input_bits_; r++) {
                add_mb_node_master_imgui_input_bits_data_callback_ctx_fn(v_id, ib.block_id_, ib.mb_mapping_input_bits_start_address_, r);
                // Log Here
              }
            }
          }
          // Read the Holding Register blocks.
          {
            nlohmann::json jhreg_blocks = jagent["agent"]["node"]["modbus_master"]["slave_device"]["modbus_mapping"]["register_blocks"];

            for(nlohmann::json::iterator hrit = jhreg_blocks.begin(); hrit != jhreg_blocks.end(); ++hrit) {
              nlohmann::json jhr(*hrit);
              mb_holding_register_block_t hr;
              hr.block_id_ = jhr["block_id"];
              hr.mb_mapping_registers_start_address_ = jhr["registers_start_address"];
              hr.mb_mapping_nb_registers_ = jhr["nb_registers"];
              hr.mb_block_refresh_interval_ = jhr["block_refresh_interval_ms"];

              if((hr.mb_block_refresh_interval_ != 0) && (hr.mb_block_refresh_interval_ < 1000)) {
                hr.mb_block_refresh_interval_ = 1000;
              }

              if(hr.mb_block_refresh_interval_ == 0) {
                hr.mb_block_refresh_interval_ = v1.mb_refresh_interval_ms_;
              }
              
              hr.eu_factor_ = jhr["eu_factor"];
              hr.eu_quantity_ = jhr["eu_quantity"];
              hr.eu_manifest_constant_ = jhr["eu_manifest_constant"];

              v1.mb_mapping_holding_register_blocks_.try_emplace(hr.block_id_, std::move(hr));

              // Now prepare GUI data structures.
              for(int r = 0; r < hr.mb_mapping_nb_registers_; r++) {
                add_mb_node_master_imgui_holding_register_callback_ctx_fn(v_id, hr.block_id_, hr.mb_mapping_registers_start_address_, r);
              }
            }
          }
          // Read the Input Register blocks.
          {
            nlohmann::json jinreg_blocks = jagent["agent"]["node"]["modbus_master"]["slave_device"]["modbus_mapping"]["input_register_blocks"];

            for(nlohmann::json::iterator irit = jinreg_blocks.begin(); irit != jinreg_blocks.end(); ++irit) {
              nlohmann::json jir(*irit);
              mb_input_register_block_t ir;
              ir.block_id_ = jir["block_id"];
              ir.mb_mapping_input_registers_start_address_ = jir["input_registers_start_address"];
              ir.mb_mapping_nb_input_registers_ = jir["nb_input_registers"];
              ir.mb_block_refresh_interval_ = jir["block_refresh_interval_ms"];

              if((ir.mb_block_refresh_interval_ != 0) && (ir.mb_block_refresh_interval_ < 1000)) {
                ir.mb_block_refresh_interval_ = 1000;
              }

              if(ir.mb_block_refresh_interval_ == 0) {
                ir.mb_block_refresh_interval_ = v1.mb_refresh_interval_ms_;
              }

              ir.eu_factor_ = jir["eu_factor"];
              ir.eu_quantity_ = jir["eu_quantity"];
              ir.eu_manifest_constant_ = jir["eu_manifest_constant"];
              
              v1.mb_mapping_input_register_blocks_.try_emplace(ir.block_id_, std::move(ir));

              // Now prepare GUI data structures.
              for(int r = 0; r < ir.mb_mapping_nb_input_registers_; r++) {
                add_mb_node_master_imgui_input_register_callback_ctx_fn(v_id, ir.block_id_, ir.mb_mapping_input_registers_start_address_, r);
              }

            }
          }

          v1.identifier_
            = jagent["agent"]["node"]["descriptors"]["thing"]["identifier"];

          v1.description_
            = jagent["agent"]["node"]["descriptors"]["thing"]["description"];

          v1.url_
            = jagent["agent"]["node"]["descriptors"]["thing"]["url"];

          // Measurement webhook URL
          v1.measurement_pub_api_url_
            = jagent["agent"]["node"]["modbus_master"]["measurement_pub_api_url"];

          // Now store the master device for GUI.
          mb_master_devices_.try_emplace(v_id, std::move(v1));
        }

        // Only one GUI -> modbus subscriber object per controller.
        mb_node_master_ws_subscriber_worker_t s1;
        s1.mb_master_contoller_address_ = jnode["controller"]["attributes"]["api_server_listen_address"];
        s1.mb_master_contoller_port_    = jnode["controller"]["attributes"]["api_server_listen_port"];
        modbus_master_ws_subscriber_worker_vec_.emplace_back(std::move(s1));
      }
    }
    catch(nlohmann::json::exception& e) {
      // Log exception information
      std::stringstream ssReqException;
      ssReqException  << "message: " << e.what()    <<  " "
        << "exception id: " << e.id   << std::endl;
      // Logging to be added.
      std::cout << "FIXME" << ssReqException.str() << std::endl;
    }
  }

  return 0;
}

int mb_node_master_imgui_components_main_t::run_gui_components()
{
  //
  // DESIGN NOTES
  // Now initialize the SObjectizer to process the received modbus messages.
  //
  mb_master_gui_env_obj_ = std::make_shared<so_5::wrapped_env_t>();
  msg_primary_channel_ = so_5::create_mchain(*mb_master_gui_env_obj_);
  auto msg_primary_channel_closer = so_5::auto_close_drop_content(msg_primary_channel_);

  msg_secondary_channel_ = so_5::create_mchain(msg_primary_channel_->environment());
  auto msg_secondary_channel_closer = so_5::auto_close_drop_content(msg_secondary_channel_);

  // Now set the essage channels for each WS object.
  for(auto ws = modbus_master_ws_subscriber_worker_vec_.begin(); ws != modbus_master_ws_subscriber_worker_vec_.end(); ++ws) {
    ws->mb_master_gui_env_obj_       = mb_master_gui_env_obj_;
    ws->msg_primary_channel_         = msg_primary_channel_;
    ws->msg_secondary_channel_       = msg_secondary_channel_;
  }

  //
  // DESIGN NOTES
  // Now launch the Web Socket Client to connect with the local/or configured server
  // Provided node registry MAY have more than one MODBUS-MASTER CONTROLLERS
  // Each MODBUS-MASTER CONTROLLER published the data from configured master nodes.
  // [Master Controllers] -> [Publishes messages read from the SLAVE nodes]
  // Separate threads receive messages over Web Sockets for each MODBUS-MASTER CONTROLLER.
  //
  std::transform(modbus_master_ws_subscriber_worker_vec_.begin(), modbus_master_ws_subscriber_worker_vec_.end()
  , modbus_master_ws_subscriber_worker_vec_.begin(), [](mb_node_master_ws_subscriber_worker_t &s) {
    s.ws_thread_ = std::make_shared<std::thread>([&s]() {
      s.work();
    });
    return s;
  });

  //
  // DESIGN NOTES
  // Now launch the SObjectizer to process the received modbus messages.
  //
  gthread_ = std::make_shared<std::thread>([this]() {
    mb_message_processing_fn();
  });

  //
  // Join the SO environment and thread. This should be the last call and does not return.
  //
  auto gui_so_joiner = so_5::auto_join(*gthread_);

  return 0;
}


// Input Registers Descriptive Analytics data context
mb_node_master_imgui_callback_input_register_data_context_t*
mb_node_master_imgui_components_main_t::get_mb_node_master_imgui_input_register_callback_ctx_fn
(const std::string& device_id
  , int block_id, int start_address, int reg)
{
  std::stringstream cb_ctx_key;
  cb_ctx_key <<  device_id << "/" << block_id << "/" << start_address << "/" << reg;

  auto ctx = std::find_if(mb_node_master_imgui_input_register_callback_ctx_.begin()
      , mb_node_master_imgui_input_register_callback_ctx_.end()
  , [&](std::pair <std::string, mb_node_master_imgui_callback_input_register_data_context_t*> c) {
    return (c.first.compare(cb_ctx_key.str()) == 0) ? true : false;
  });

  return (ctx != mb_node_master_imgui_input_register_callback_ctx_.end()) ? ctx->second : nullptr;
}

mb_node_master_imgui_callback_input_register_data_context_t*
mb_node_master_imgui_components_main_t::add_mb_node_master_imgui_input_register_callback_ctx_fn
(const std::string& device_id
  , int block_id, int start_address, int reg)
{
  std::stringstream cb_ctx_key;
  cb_ctx_key <<  device_id << "/" << block_id << "/" << start_address << "/" << reg;

  auto ctx = std::find_if(mb_node_master_imgui_input_register_callback_ctx_.begin()
      , mb_node_master_imgui_input_register_callback_ctx_.end()
  , [&](std::pair <std::string, mb_node_master_imgui_callback_input_register_data_context_t*> c) {
    return (c.first.compare(cb_ctx_key.str()) == 0) ? true : false;
  });

  if(ctx != mb_node_master_imgui_input_register_callback_ctx_.end()) {
    return ctx->second;
  }

  // If no callback context, create one here.
  mb_node_master_imgui_callback_input_register_data_context_t* ctx_ptr = new mb_node_master_imgui_callback_input_register_data_context_t();

  ctx_ptr->mb_node_master_imgui_main_ = this;
  ctx_ptr->block_id_ = block_id;
  ctx_ptr->start_address_ = start_address;
  ctx_ptr->register_number_ = reg;
  ctx_ptr->device_id_ = device_id;

  auto ins = mb_node_master_imgui_input_register_callback_ctx_.try_emplace(cb_ctx_key.str(), ctx_ptr);

  if(ins.second) {
    return mb_node_master_imgui_input_register_callback_ctx_.at(cb_ctx_key.str());
  }

  return nullptr;
}

// Holding Registers Descriptive Analytics data context
mb_node_master_imgui_callback_holding_register_data_context_t*
mb_node_master_imgui_components_main_t::get_mb_node_master_imgui_holding_register_callback_ctx_fn
(const std::string& device_id
  , int block_id, int start_address, int reg)
{
  std::stringstream cb_ctx_key;
  cb_ctx_key <<  device_id << "/" << block_id << "/" << start_address << "/" << reg;

  auto ctx = std::find_if(mb_node_master_imgui_holding_register_callback_ctx_.begin()
      , mb_node_master_imgui_holding_register_callback_ctx_.end()
  , [&](std::pair<std::string, mb_node_master_imgui_callback_holding_register_data_context_t*> c) {
    return (c.first.compare(cb_ctx_key.str()) == 0) ? true : false;
  });

  return (ctx != mb_node_master_imgui_holding_register_callback_ctx_.end()) ? ctx->second : nullptr;

}
mb_node_master_imgui_callback_holding_register_data_context_t*
mb_node_master_imgui_components_main_t::add_mb_node_master_imgui_holding_register_callback_ctx_fn
(const std::string& device_id
  , int block_id, int start_address, int reg)
{
  std::stringstream cb_ctx_key;
  cb_ctx_key <<  device_id << "/" << block_id << "/" << start_address << "/" << reg;

  auto ctx = std::find_if(mb_node_master_imgui_holding_register_callback_ctx_.begin()
      , mb_node_master_imgui_holding_register_callback_ctx_.end()
  , [&](std::pair<std::string, mb_node_master_imgui_callback_holding_register_data_context_t*> c) {
    return (c.first.compare(cb_ctx_key.str()) == 0) ? true : false;
  });

  if(ctx != mb_node_master_imgui_holding_register_callback_ctx_.end()) {
    return ctx->second;
  }

  // If no callback context, create one here.
  mb_node_master_imgui_callback_holding_register_data_context_t* ctx_ptr = new mb_node_master_imgui_callback_holding_register_data_context_t();

  ctx_ptr->mb_node_master_imgui_main_ = this;
  ctx_ptr->block_id_ = block_id;
  ctx_ptr->start_address_ = start_address;
  ctx_ptr->register_number_ = reg;
  ctx_ptr->device_id_ = device_id;

  auto ins = mb_node_master_imgui_holding_register_callback_ctx_.try_emplace(cb_ctx_key.str(), ctx_ptr);

  if(ins.second) {
    return mb_node_master_imgui_holding_register_callback_ctx_.at(cb_ctx_key.str());
  }

  return nullptr;
}
/////////////////////////////
// COIL Descriptive Analytics data context
/////////////////////////////
mb_node_master_imgui_callback_coils_data_context_t*
mb_node_master_imgui_components_main_t::get_mb_node_master_imgui_coils_data_callback_ctx_fn(const std::string& device_id, int block_id, int start_address, int cbit)
{
  std::stringstream cb_ctx_key;
  cb_ctx_key <<  device_id << "/" << block_id << "/" << start_address << "/" << cbit;

  auto ctx = std::find_if(mb_node_master_imgui_coils_data_callback_ctx_.begin()
      , mb_node_master_imgui_coils_data_callback_ctx_.end()
  , [&](std::pair<std::string, mb_node_master_imgui_callback_coils_data_context_t*> c) {
    return (c.first.compare(cb_ctx_key.str()) == 0) ? true : false;
  });

  return (ctx != mb_node_master_imgui_coils_data_callback_ctx_.end()) ? ctx->second : nullptr;
}

mb_node_master_imgui_callback_coils_data_context_t*
mb_node_master_imgui_components_main_t::add_mb_node_master_imgui_coils_data_callback_ctx_fn(const std::string& device_id, int block_id, int start_address, int cbit)
{
  std::stringstream cb_ctx_key;
  cb_ctx_key <<  device_id << "/" << block_id << "/" << start_address << "/" << cbit;

  auto ctx = std::find_if(mb_node_master_imgui_coils_data_callback_ctx_.begin()
      , mb_node_master_imgui_coils_data_callback_ctx_.end()
  , [&](std::pair<std::string, mb_node_master_imgui_callback_coils_data_context_t*> c) {
    return (c.first.compare(cb_ctx_key.str()) == 0) ? true : false;
  });

  if(ctx != mb_node_master_imgui_coils_data_callback_ctx_.end()) {
    return ctx->second;
  }

  // If no callback context, create one here.
  mb_node_master_imgui_callback_coils_data_context_t* ctx_ptr = new mb_node_master_imgui_callback_coils_data_context_t();

  ctx_ptr->mb_node_master_imgui_main_ = this;
  ctx_ptr->block_id_ = block_id;
  ctx_ptr->start_address_ = start_address;
  ctx_ptr->coil_number_ = cbit;
  ctx_ptr->device_id_ = device_id;

  auto ins = mb_node_master_imgui_coils_data_callback_ctx_.try_emplace(cb_ctx_key.str(), ctx_ptr);

  if(ins.second) {
    return mb_node_master_imgui_coils_data_callback_ctx_.at(cb_ctx_key.str());
  }

  return nullptr;
}

/////////////////////////////
// Input BITS Descriptive Analytics data context
/////////////////////////////

mb_node_master_imgui_callback_input_bits_data_context_t*
mb_node_master_imgui_components_main_t::get_mb_node_master_imgui_input_bits_data_callback_ctx_fn(const std::string& device_id, int block_id, int start_address, int ibit)
{
  std::stringstream cb_ctx_key;
  cb_ctx_key <<  device_id << "/" << block_id << "/" << start_address << "/" << ibit;

  auto ctx = std::find_if(mb_node_master_imgui_input_bits_data_callback_ctx_.begin()
      , mb_node_master_imgui_input_bits_data_callback_ctx_.end()
  , [&](std::pair<std::string, mb_node_master_imgui_callback_input_bits_data_context_t*> c) {
    return (c.first.compare(cb_ctx_key.str()) == 0) ? true : false;
  });

  return (ctx != mb_node_master_imgui_input_bits_data_callback_ctx_.end()) ? ctx->second : nullptr;

}

mb_node_master_imgui_callback_input_bits_data_context_t*
mb_node_master_imgui_components_main_t::add_mb_node_master_imgui_input_bits_data_callback_ctx_fn(const std::string& device_id, int block_id, int start_address, int ibit)
{
  std::stringstream cb_ctx_key;
  cb_ctx_key <<  device_id << "/" << block_id << "/" << start_address << "/" << ibit;

  auto ctx = std::find_if(mb_node_master_imgui_input_bits_data_callback_ctx_.begin()
      , mb_node_master_imgui_input_bits_data_callback_ctx_.end()
  , [&](std::pair<std::string, mb_node_master_imgui_callback_input_bits_data_context_t*> c) {
    return (c.first.compare(cb_ctx_key.str()) == 0) ? true : false;
  });

  if(ctx != mb_node_master_imgui_input_bits_data_callback_ctx_.end()) {
    return ctx->second;
  }

  // If no callback context, create one here.
  mb_node_master_imgui_callback_input_bits_data_context_t* ctx_ptr = new mb_node_master_imgui_callback_input_bits_data_context_t();

  ctx_ptr->mb_node_master_imgui_main_ = this;
  ctx_ptr->block_id_ = block_id;
  ctx_ptr->start_address_ = start_address;
  ctx_ptr->input_bit_number_ = ibit;
  ctx_ptr->device_id_ = device_id;

  auto ins = mb_node_master_imgui_input_bits_data_callback_ctx_.try_emplace(cb_ctx_key.str(), ctx_ptr);

  if(ins.second) {
    return mb_node_master_imgui_input_bits_data_callback_ctx_.at(cb_ctx_key.str());
  }

  return nullptr;
}

//////////////////////////////////////////////
// Prepare Device Id
std::string
mb_node_master_imgui_components_main_t::prepare_device_id(const std::string& controller_id, const std::string& agent_id, const std::string& node_id)
{
  std::stringstream v_id;
  v_id << controller_id << "/" << agent_id << "/" << node_id;
  return v_id.str();
}

void mb_node_master_imgui_components_main_t::modbus_master_menu()
{
  {
    // Menu Bar
    if(ImGui::BeginMenuBar()) {
      // IoT Device Menu
      if(ImGui::BeginMenu("Modbus Master Devices")) {
        // Level 2
        int mb_gui_device = 0;

        for(auto it = mb_master_devices_.begin();
          it != mb_master_devices_.end() && (mb_gui_device < MAX_MODBUS_MASTER_DEVICES_GUI_SUPPORTED);
          ++it, mb_gui_device++) {
          auto v_mb = *it;
          std::stringstream device_id;
          device_id << prepare_device_id(v_mb.second.contoller_id_, v_mb.second.agent_id_, v_mb.second.node_id_);

          // The map has a key "controller/agent/node"
          if(ImGui::BeginMenu(v_mb.first.c_str())) {
            if(ImGui::BeginMenu("Real-time Data of Input Registers")) {
              auto v_mb_input_register_blocks = v_mb.second.mb_mapping_input_register_blocks_;
              int max_blocks_configured = v_mb.second.mb_mapping_input_register_blocks_.size();

              for(auto irb : v_mb_input_register_blocks) {
                if(irb.second.mb_mapping_nb_input_registers_ > 0) {
                  for(int mb_master_input_register_number = 0;
                    mb_master_input_register_number < irb.second.mb_mapping_nb_input_registers_
                    && mb_master_input_register_number < MAX_MODBUS_MASTER_INPUT_REGISTERS_GUI_SUPPORTED
                    && max_blocks_configured < MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED;
                    mb_master_input_register_number++) {
                    //
                    // @TODO
                    // Check if the input register is a valid register or not.
                    //
                    std::stringstream title_;
                    title_  << v_mb.first << " " << "Slave Id:"<< " " << v_mb.second.mb_slave_id_
                      << " " << "Block Id:"  << " " << irb.first
                      << " " << "Start Address:"  << " " << irb.second.mb_mapping_input_registers_start_address_
                      << " " << "Input Register:" << " " << (mb_master_input_register_number + 1); // Refer to Modbus Standard of bits, registers indexing.
                    ImGui::Checkbox(title_.str().c_str(), &show_gui_mb_master_input_registers_[mb_gui_device][irb.first][mb_master_input_register_number]);
                  }
                }
                else {
                  std::stringstream title_;
                  title_  << "No Input Registers Configured for "
                    << " " << "Block Id:"  << " " << irb.first
                    << " " << "Start Address:"  << " " << irb.second.mb_mapping_input_registers_start_address_
                    << " " << "Input Register:" << " " << 0;
                  ImGui::MenuItem(title_.str().c_str());
                }
              }

              ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Real-time Data of Holding Registers")) {
              auto v_mb_holding_register_blocks = v_mb.second.mb_mapping_holding_register_blocks_;
              int max_blocks_configured = v_mb.second.mb_mapping_holding_register_blocks_.size();

              for(auto hrb : v_mb_holding_register_blocks) {
                ///////////
                if(hrb.second.mb_mapping_nb_registers_ > 0) {
                  for(int mb_master_holding_register_number = 0;
                    mb_master_holding_register_number < hrb.second.mb_mapping_nb_registers_
                    && mb_master_holding_register_number < MAX_MODBUS_MASTER_HOLDING_REGISTERS_GUI_SUPPORTED
                    && max_blocks_configured < MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED;
                    mb_master_holding_register_number++) {
                    std::stringstream title_;
                    title_  << v_mb.first
                      << " " << "Slave Id:"           << " " << v_mb.second.mb_slave_id_
                      << " " << "Block Id:"           << " " << hrb.first
                      << " " << "Start Address:"      << " " << hrb.second.mb_mapping_registers_start_address_
                      << " " << "Holding Register: "  << (mb_master_holding_register_number + 1); // Refer to Modbus Standard of bits, registers indexing.
                    ImGui::Checkbox(title_.str().c_str(), &show_gui_mb_master_holding_registers_[mb_gui_device][hrb.first][mb_master_holding_register_number]);

                  }
                }
                else {
                  std::stringstream title_;
                  title_  << "No Holding Registers Configured for "
                    << " " << "Block Id:"           << " " << hrb.first
                    << " " << "Start Address:"      << " " << hrb.second.mb_mapping_registers_start_address_
                    << " " << "Holding Register: "  << 0;
                  ImGui::MenuItem(title_.str().c_str());
                }

              }

              ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Real-time Data of Input Bits")) {
              auto v_mb_input_bits_blocks = v_mb.second.mb_mapping_input_bit_blocks_;
              int max_blocks_configured = v_mb.second.mb_mapping_input_bit_blocks_.size();

              for(auto ibb : v_mb_input_bits_blocks) {
                if(ibb.second.mb_mapping_nb_input_bits_ > 0) {
                  for(int mb_master_input_bits_number = 0;
                    mb_master_input_bits_number < ibb.second.mb_mapping_nb_input_bits_
                    && mb_master_input_bits_number < MAX_MODBUS_MASTER_INPUT_BITS_GUI_SUPPORTED
                    && max_blocks_configured < MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED;
                    mb_master_input_bits_number++) {
                    std::stringstream title_;
                    title_  << v_mb.first
                      << " " << "Slave Id:"         << " " << v_mb.second.mb_slave_id_
                      << " " << "Block Id:"         << " " << ibb.first
                      << " " << "Start Address:"    << " " << ibb.second.mb_mapping_input_bits_start_address_
                      << " " << "Input Bit Number:" << " " << (mb_master_input_bits_number + 1); // Refer to Modbus Standard of bits, registers indexing.
                    ImGui::Checkbox(title_.str().c_str()
                      , &show_gui_mb_master_input_bits_[mb_gui_device][ibb.first][mb_master_input_bits_number]);
                  }
                }
                else {
                  std::stringstream title_;
                  title_  << "No Input Bits Configured for "
                    << " " << "Block Id:"         << " " << ibb.first
                    << " " << "Start Address:"    << " " << ibb.second.mb_mapping_input_bits_start_address_
                    << " " << "Input Bit Number:" << " " << 0;
                  ImGui::MenuItem(title_.str().c_str());
                }
              }

              ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Real-time Data of Coil Bits")) {
              auto v_mb_coil_bits_blocks = v_mb.second.mb_mapping_coil_blocks_;
              int max_blocks_configured = v_mb.second.mb_mapping_coil_blocks_.size();

              for(auto cbb : v_mb_coil_bits_blocks) {
                if(cbb.second.mb_mapping_nb_bits_ > 0) {
                  for(int mb_master_coil_bits_number = 0;
                    mb_master_coil_bits_number < cbb.second.mb_mapping_nb_bits_
                    && mb_master_coil_bits_number < MAX_MODBUS_MASTER_COIL_BITS_GUI_SUPPORTED
                    && max_blocks_configured < MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED;
                    mb_master_coil_bits_number++) {
                    //
                    // @TODO
                    // Check if the input register is a valid register or not.
                    //
                    std::stringstream title_;
                    title_  << v_mb.first
                      << " " << "Slave Id:"         << " " << v_mb.second.mb_slave_id_
                      << " " << "Block Id:"         << " " << cbb.first
                      << " " << "Start Address:"    << " " << cbb.second.mb_mapping_bits_start_address_
                      << " " << "Coil Bit Number:"  << " " << (mb_master_coil_bits_number + 1); // Refer to Modbus Standard of bits, registers indexing.
                    ImGui::Checkbox(title_.str().c_str()
                      , &show_gui_mb_master_coil_bits_[mb_gui_device][cbb.first][mb_master_coil_bits_number]);

                    //////////////
                    // Send ON to Device Coils
                    std::stringstream on_title_;
                    on_title_ << "Send ON Message##" << title_.str();

                    if(ImGui::Button(on_title_.str().c_str())) {

                      ImGui::OpenPopup(on_title_.str().c_str());
                    }

                    // Always center this window when appearing
                    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

                    if(ImGui::BeginPopupModal(on_title_.str().c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                      ImGui::Text("Send ON (0xFF00) message to the device.\n\n");
                      ImGui::Separator();

                      if(ImGui::Button("OK", ImVec2(120, 0))) {
                        auto ma= that_supervisor_->get_mb_master_node_handle(v_mb.second.contoller_id_, v_mb.second.agent_id_, v_mb.second.node_id_);

                        if(ma) {
                          ma->post_write_single_coil_async(1 // Used by the application to identify the req-resp
                            , 0x05
                            , cbb.first
                            , cbb.second.mb_mapping_bits_start_address_
                              + (mb_master_coil_bits_number/8)
                              + ((mb_master_coil_bits_number%8) == 0 ? 0 : 1)
                            , 0xFF00);
                        }

                        ImGui::CloseCurrentPopup();
                      }

                      ImGui::SetItemDefaultFocus();
                      ImGui::SameLine();

                      if(ImGui::Button("Cancel", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                      }

                      ImGui::EndPopup();
                    }

                    ImGui::SameLine();

                    /////////////
                    // Send OFF to Device Coils
                    std::stringstream off_title_;
                    off_title_ << "Send OFF Message ##" << title_.str();

                    if(ImGui::Button(off_title_.str().c_str())) {

                      ImGui::OpenPopup(off_title_.str().c_str());
                    }

                    // Always center this window when appearing
                    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

                    if(ImGui::BeginPopupModal(off_title_.str().c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                      ImGui::Text("Send OFF (0x0000) message to the device.\n\n");
                      ImGui::Separator();

                      if(ImGui::Button("OK", ImVec2(120, 0))) {
                        auto ma = that_supervisor_->get_mb_master_node_handle(v_mb.second.contoller_id_, v_mb.second.agent_id_, v_mb.second.node_id_);

                        if(ma) {
                          ma->post_write_single_coil_async(1 // Used by the application to identify the req-resp
                            , 0x05
                            , cbb.first
                            , cbb.second.mb_mapping_bits_start_address_
                              + (mb_master_coil_bits_number/8)
                              + ((mb_master_coil_bits_number%8) == 0 ? 0 : 1)
                            , 0x0000);
                        }

                        ImGui::CloseCurrentPopup();
                      }

                      ImGui::SetItemDefaultFocus();
                      ImGui::SameLine();

                      if(ImGui::Button("Cancel", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                      }

                      ImGui::EndPopup();
                    }

                    ImGui::Separator();
                    ImGui::Separator();
                    /////////////
                  }
                }
                else {
                  std::stringstream title_;
                  title_  << "No Coil Bits Configured for "
                    << " " << "Block Id:"  << " " << cbb.first
                    << " " << "Start Address:"  << " " << cbb.second.mb_mapping_bits_start_address_
                    << " " << "Coil Bit Number:" << " " << 0;
                  ImGui::MenuItem(title_.str().c_str());
                }
              }

              ImGui::EndMenu();
            }

            ImGui::Separator();
            ImGui::Separator();

            // Descriptive Analytics
            if(ImGui::BeginMenu("Descriptive Analytics")) {
              ImGui::MenuItem("To be implemented ...");
              ImGui::EndMenu();
            }

            ImGui::Separator();
            ImGui::Separator();

            // Predictive Analytics
            if(ImGui::BeginMenu("Predictive Analytics")) {
              ImGui::MenuItem("To be implemented ...");
              ImGui::EndMenu();
            }

            ImGui::Separator();
            ImGui::Separator();

            // Prescriptive Analytics
            if(ImGui::BeginMenu("Prescriptive Analytics")) {
              ImGui::MenuItem("To be implemented ...");
              ImGui::EndMenu();
            }

            // Close the Menu.
            ImGui::EndMenu();
          }

          ImGui::Separator();
          ImGui::Separator();
        }

        ImGui::EndMenu();
      }

    }

    ImGui::EndMenuBar();
  }
}


void mb_node_master_imgui_components_main_t::modbus_master_window()
{
  // Level 2
  int mb_gui_device = 0;

  for(auto it = mb_master_devices_.begin();
    it != mb_master_devices_.end() && (mb_gui_device < MAX_MODBUS_MASTER_DEVICES_GUI_SUPPORTED);
    ++it, mb_gui_device++) {
    auto v_mb = *it;
    std::stringstream device_id;
    device_id << prepare_device_id(v_mb.second.contoller_id_, v_mb.second.agent_id_, v_mb.second.node_id_);

    /////////////////// INPUT REGISTER REAL-TIME DATA /////////////////////////////
    {
      // The map has a key "controller/agent/node"
      auto v_mb_input_register_blocks = v_mb.second.mb_mapping_input_register_blocks_;
      int max_blocks_configured = v_mb.second.mb_mapping_input_register_blocks_.size();

      for(auto irb : v_mb_input_register_blocks) {
        int mb_master_input_register_number = 0;

        for(mb_master_input_register_number = 0;
          mb_master_input_register_number < irb.second.mb_mapping_nb_input_registers_
          && mb_master_input_register_number < MAX_MODBUS_MASTER_INPUT_REGISTERS_GUI_SUPPORTED
          && max_blocks_configured < MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED;
          mb_master_input_register_number++) {

          if(show_gui_mb_master_input_registers_[mb_gui_device][irb.first][mb_master_input_register_number]) {
            std::stringstream title_l;
            title_l << v_mb.first
              << " " << "Slave Id:"         << " " << v_mb.second.mb_slave_id_
              << " " << "Block Id:"         << " " << irb.first
              << " " << "Start Address:"    << " " << irb.second.mb_mapping_input_registers_start_address_
              << " " << "Input Register: "  << (mb_master_input_register_number + 1) << " " // Refer to Modbus Standard of bits, registers indexing.
              << " " << "Sampling:"         << " " << irb.second.mb_block_refresh_interval_ << "ms"
              << " " << "Analytics: Lower Conditional Data";

            ImGui::SetNextWindowSize(ImVec2(1200,650), ImGuiCond_FirstUseEver);
            ImGui::Begin(title_l.str().c_str(), &show_gui_mb_master_input_registers_[mb_gui_device][irb.first][mb_master_input_register_number]);

            mb_node_master_imgui_callback_input_register_data_context_t*
            data_ctx = get_mb_node_master_imgui_input_register_callback_ctx_fn
              (device_id.str()
                , irb.first, irb.second.mb_mapping_input_registers_start_address_, mb_master_input_register_number);

            ImPlot::PushColormap(ImPlotColormap_Deep);
            ImPlotStyle backup = ImPlot::GetStyle();
            set_style_seaborn();
  
            std::stringstream eu_string_l;
            eu_string_l << irb.second.eu_factor_ << " " << irb.second.eu_quantity_;
            
            if(ImPlot::BeginPlot(title_l.str().c_str(), "Measured Instances", eu_string_l.str().c_str(), ImVec2(-1,0), ImPlotFlags_NoMenus)) {
              ImPlot::PlotBarsG(""
              , [](void* data, int idx) {
                auto that = (mb_node_master_imgui_callback_input_register_data_context_t*)data;

                if(that == nullptr) {
                  ImPlotPoint point(idx, 0);
                  return point;
                }

                ImPlotPoint point(idx, that->on_nth_minute_data_y_at_idx(idx));
                return point;
              }
              , (void*)data_ctx, mb_last_nth_minute_data_consts_te::max_nth_minute_data_size, 0.67);

              ImPlot::EndPlot();
            }

            std::stringstream title_h;
            title_h << v_mb.first
              << " " << "Slave Id:"         << " " << v_mb.second.mb_slave_id_
              << " " << "Block Id:"         << " " << irb.first
              << " " << "Start Address:"    << " " << irb.second.mb_mapping_input_registers_start_address_
              << " " << "Input Register: "  << (mb_master_input_register_number + 1) << " " // Refer to Modbus Standard of bits, registers indexing.
              << " " << "Sampling:"         << " " << irb.second.mb_block_refresh_interval_ << "ms"
              << " " << "Analytics: Higher Conditional Data";

            std::stringstream eu_string_h;
            eu_string_h << irb.second.eu_factor_ << " " << irb.second.eu_quantity_;
  
            if(ImPlot::BeginPlot(title_h.str().c_str(), "Measured Instances", eu_string_h.str().c_str(), ImVec2(-1,0), ImPlotFlags_NoMenus)) {
              ImPlot::PlotLineG(""
              , [](void* data, int idx) {
                auto that = (mb_node_master_imgui_callback_input_register_data_context_t*)data;

                if(that == nullptr) {
                  ImPlotPoint point(idx, 0);
                  return point;
                }

                ImPlotPoint point(idx, that->on_nth_minute_data_y_at_idx(idx));
                return point;
              }
              , (void*)data_ctx, mb_last_nth_minute_data_consts_te::max_nth_minute_data_size);

              ImPlot::EndPlot();
            }

            // Restore Style.
            ImPlot::GetStyle() = backup;
            ImPlot::PopColormap();

            ImGui::End();
          }
        }
      }
    }
    /////////////////// HOLDING REGISTER REAL-TIME DATA /////////////////////////////
    {
      auto v_mb_holding_register_blocks = v_mb.second.mb_mapping_holding_register_blocks_;
      int max_blocks_configured = v_mb.second.mb_mapping_holding_register_blocks_.size();

      for(auto hrb : v_mb_holding_register_blocks) {
        ///////////
        int mb_master_holding_register_number = 0;

        for(mb_master_holding_register_number = 0;
          mb_master_holding_register_number < hrb.second.mb_mapping_nb_registers_
          && mb_master_holding_register_number < MAX_MODBUS_MASTER_HOLDING_REGISTERS_GUI_SUPPORTED
          && max_blocks_configured < MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED;
          mb_master_holding_register_number++) {

          if(show_gui_mb_master_holding_registers_[mb_gui_device][hrb.first][mb_master_holding_register_number]) {
            std::stringstream title_l;
            title_l << v_mb.first
              << " " << "Slave Id:"           << " " << v_mb.second.mb_slave_id_
              << " " << "Block Id:"           << " " << hrb.first
              << " " << "Start Address:"      << " " << hrb.second.mb_mapping_registers_start_address_
              << " " << "Holding Register: "  << (mb_master_holding_register_number + 1) << " " // Refer to Modbus Standard of bits, registers indexing.
              << " " << "Sampling:"           << " " << hrb.second.mb_block_refresh_interval_ << "ms"
              << " " << "Analytics: Lower Conditional Data";

            ImGui::SetNextWindowSize(ImVec2(1200,650), ImGuiCond_FirstUseEver);
            ImGui::Begin(title_l.str().c_str(), &show_gui_mb_master_holding_registers_[mb_gui_device][hrb.first][mb_master_holding_register_number]);

            mb_node_master_imgui_callback_holding_register_data_context_t*
            data_ctx = get_mb_node_master_imgui_holding_register_callback_ctx_fn
              (device_id.str()
                , hrb.first, hrb.second.mb_mapping_registers_start_address_, mb_master_holding_register_number);
          
            std::stringstream eu_string_l;
            eu_string_l << hrb.second.eu_factor_ << " " << hrb.second.eu_quantity_;

            if(ImPlot::BeginPlot(title_l.str().c_str(), "Measured Instances", eu_string_l.str().c_str(), ImVec2(-1,0), ImPlotFlags_NoMenus)) {
              ImPlot::PlotBarsG(""
              , [](void* data, int idx) {
                auto that = (mb_node_master_imgui_callback_holding_register_data_context_t*)data;

                if(that == nullptr) {
                  ImPlotPoint point(idx, 0);
                  return point;
                }

                ImPlotPoint point(idx, that->on_nth_minute_data_y_at_idx(idx));
                return point;
              }
              , (void*)data_ctx, mb_last_nth_minute_data_consts_te::max_nth_minute_data_size, 0.67);
              ImPlot::EndPlot();
            }

            std::stringstream title_h;
            title_h << v_mb.first
              << " " << "Slave Id:"           << " " << v_mb.second.mb_slave_id_
              << " " << "Block Id:"           << " " << hrb.first
              << " " << "Start Address:"      << " " << hrb.second.mb_mapping_registers_start_address_
              << " " << "Holding Register: "  << (mb_master_holding_register_number + 1) << " " // Refer to Modbus Standard of bits, registers indexing.
              << " " << "Sampling:"           << " " << hrb.second.mb_block_refresh_interval_ << "ms"
              << " " << "Analytics: Higher Conditional Data";
          
            std::stringstream eu_string_h;
            eu_string_h << hrb.second.eu_factor_ << " " << hrb.second.eu_quantity_;

            if(ImPlot::BeginPlot(title_h.str().c_str(), "Measured Instances", eu_string_h.str().c_str(), ImVec2(-1,0), ImPlotFlags_NoMenus)) {
              ImPlot::PlotLineG(""
              , [](void* data, int idx) {
                auto that = (mb_node_master_imgui_callback_holding_register_data_context_t*)data;

                if(that == nullptr) {
                  ImPlotPoint point(idx, 0);
                  return point;
                }

                ImPlotPoint point(idx, that->on_nth_minute_data_y_at_idx(idx));
                return point;
              }
              , (void*)data_ctx, mb_last_nth_minute_data_consts_te::max_nth_minute_data_size);
              ImPlot::EndPlot();
            }

            ImGui::End();
          }
        }
      }
    }
    /////////////////// INPUT BITS REAL-TIME DATA /////////////////////////////
    {
      auto v_mb_input_bits_blocks = v_mb.second.mb_mapping_input_bit_blocks_;
      int max_blocks_configured = v_mb.second.mb_mapping_input_bit_blocks_.size();

      for(auto ibb : v_mb_input_bits_blocks) {
        ///////////
        int mb_master_input_bits_number = 0;

        for(mb_master_input_bits_number = 0;
          mb_master_input_bits_number < ibb.second.mb_mapping_nb_input_bits_
          && mb_master_input_bits_number < MAX_MODBUS_MASTER_INPUT_BITS_GUI_SUPPORTED
          && max_blocks_configured < MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED;
          mb_master_input_bits_number++) {
          std::stringstream title_;
          title_  << v_mb.first
            << " " << "Slave Id:"           << " " << v_mb.second.mb_slave_id_
            << " " << "Block Id:"           << " " << ibb.first
            << " " << "Start Address:"      << " " << ibb.second.mb_mapping_input_bits_start_address_
            << " " << "Input Bit Number: "  << (mb_master_input_bits_number + 1) // Refer to Modbus Standard of bits, registers indexing.
            << " " << "Sampling:"           << " " << ibb.second.mb_block_refresh_interval_ << "ms";

          if(show_gui_mb_master_input_bits_[mb_gui_device][ibb.first][mb_master_input_bits_number]) {

            ImGui::SetNextWindowSize(ImVec2(1200,350));
            ImGui::Begin(title_.str().c_str(), &show_gui_mb_master_input_bits_[mb_gui_device][ibb.first][mb_master_input_bits_number]);

            mb_node_master_imgui_callback_input_bits_data_context_t*
            data_ctx = get_mb_node_master_imgui_input_bits_data_callback_ctx_fn
              (device_id.str()
                , ibb.first, ibb.second.mb_mapping_input_bits_start_address_, mb_master_input_bits_number);

            ImPlot::PushColormap(ImPlotColormap_Deep);
            ImPlotStyle backup = ImPlot::GetStyle();
            set_style_seaborn();

            ImPlot::SetNextPlotLimitsY(0,20);
            ImPlot::SetNextPlotLimitsX(0, mb_last_nth_minute_data_consts_te::max_nth_minute_data_size, ImGuiCond_Always);

            std::stringstream eu_string;
            eu_string << ibb.second.eu_factor_ << " " << ibb.second.eu_quantity_;

            if(ImPlot::BeginPlot(title_.str().c_str(), "Measured Instances", eu_string.str().c_str(), ImVec2(-1,0), ImPlotFlags_NoMenus, 0, ImPlotAxisFlags_NoTickLabels)) {
              ImPlot::PlotDigitalG("##Digital"
              , [](void* data, int idx) {
                auto that = (mb_node_master_imgui_callback_input_bits_data_context_t*)data;

                if(that == nullptr) {
                  ImPlotPoint point(idx, 0);
                  return point;
                }

                // 10 * is to show a cler GUI only...it has no impact on measurements.
                ImPlotPoint point(idx, 10 * that->on_nth_minute_data_y_at_idx(idx));
                return point;
              }
              , (void*)data_ctx, mb_last_nth_minute_data_consts_te::max_nth_minute_data_size);
              ImPlot::EndPlot();
            }

            // Restore Style.
            ImPlot::GetStyle() = backup;
            ImPlot::PopColormap();

            ImGui::End();
          }
        }
      }
    }
    /////////////////// COIL BITS REAL-TIME DATA /////////////////////////////
    {
      auto v_mb_coil_bits_blocks = v_mb.second.mb_mapping_coil_blocks_;
      int max_blocks_configured = v_mb.second.mb_mapping_coil_blocks_.size();

      for(auto cbb : v_mb_coil_bits_blocks) {
        ///////////
        int mb_master_coil_bits_number = 0;

        for(mb_master_coil_bits_number = 0;
          mb_master_coil_bits_number <  cbb.second.mb_mapping_nb_bits_
          && mb_master_coil_bits_number < MAX_MODBUS_MASTER_COIL_BITS_GUI_SUPPORTED
          && max_blocks_configured < MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED;
          mb_master_coil_bits_number++) {
          std::stringstream title_;
          title_  << v_mb.first
            << " " << "Slave Id:"           << " " << v_mb.second.mb_slave_id_
            << " " << "Block Id:"           << " " << cbb.first
            << " " << "Start Address:"      << " " << cbb.second.mb_mapping_bits_start_address_
            << " " << "Coil Bit Number: "   << (mb_master_coil_bits_number + 1) // Refer to Modbus Standard of bits, registers indexing.
            << " " << "Sampling:"           << " " << cbb.second.mb_block_refresh_interval_ << "ms";

          if(show_gui_mb_master_coil_bits_[mb_gui_device][cbb.first][mb_master_coil_bits_number]) {

            ImGui::SetNextWindowSize(ImVec2(1200, 350), ImGuiCond_FirstUseEver);
            ImGui::Begin(title_.str().c_str(), &show_gui_mb_master_coil_bits_[mb_gui_device][cbb.first][mb_master_coil_bits_number]);

            mb_node_master_imgui_callback_coils_data_context_t*
            data_ctx = get_mb_node_master_imgui_coils_data_callback_ctx_fn
              (device_id.str()
                , cbb.first, cbb.second.mb_mapping_bits_start_address_, mb_master_coil_bits_number);

            ImPlot::SetNextPlotLimitsY(0,20);
            ImPlot::SetNextPlotLimitsX(0, mb_last_nth_minute_data_consts_te::max_nth_minute_data_size, ImGuiCond_Always);

            std::stringstream eu_string;
            eu_string << cbb.second.eu_factor_ << " " << cbb.second.eu_quantity_;

            if(ImPlot::BeginPlot(title_.str().c_str(), "Measured Instances", eu_string.str().c_str(), ImVec2(-1,0), ImPlotFlags_NoMenus, 0, ImPlotAxisFlags_NoTickLabels)) {
              ImPlot::PlotDigitalG("##Digital"
              , [](void* data, int idx) {
                auto that = (mb_node_master_imgui_callback_coils_data_context_t*)data;

                if(that == nullptr) {
                  ImPlotPoint point(idx, 0);
                  return point;
                }

                // 10 * is to show a cler GUI only...it has no impact on measurements.
                ImPlotPoint point(idx, 10 * that->on_nth_minute_data_y_at_idx(idx));
                return point;
              }
              , (void*)data_ctx, mb_last_nth_minute_data_consts_te::max_nth_minute_data_size);
              ImPlot::EndPlot();
            }

            ImGui::End();
          }
        }
      }
    }
  }
}


void mb_node_master_imgui_components_main_t::set_style_seaborn()
{

  ImPlotStyle& style              = ImPlot::GetStyle();

  ImVec4* colors                  = style.Colors;
  colors[ImPlotCol_Line]          = IMPLOT_AUTO_COL;
  colors[ImPlotCol_Fill]          = IMPLOT_AUTO_COL;
  colors[ImPlotCol_MarkerOutline] = IMPLOT_AUTO_COL;
  colors[ImPlotCol_MarkerFill]    = IMPLOT_AUTO_COL;
  colors[ImPlotCol_ErrorBar]      = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImPlotCol_FrameBg]       = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImPlotCol_PlotBg]        = ImVec4(0.92f, 0.92f, 0.95f, 1.00f);
  colors[ImPlotCol_PlotBorder]    = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImPlotCol_LegendBg]      = ImVec4(0.92f, 0.92f, 0.95f, 1.00f);
  colors[ImPlotCol_LegendBorder]  = ImVec4(0.80f, 0.81f, 0.85f, 1.00f);
  colors[ImPlotCol_LegendText]    = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImPlotCol_TitleText]     = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImPlotCol_InlayText]     = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImPlotCol_XAxis]         = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImPlotCol_XAxisGrid]     = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImPlotCol_YAxis]         = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImPlotCol_YAxisGrid]     = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImPlotCol_YAxis2]        = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImPlotCol_YAxisGrid2]    = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImPlotCol_YAxis3]        = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImPlotCol_YAxisGrid3]    = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImPlotCol_Selection]     = ImVec4(1.00f, 0.65f, 0.00f, 1.00f);
  colors[ImPlotCol_Query]         = ImVec4(0.23f, 0.10f, 0.64f, 1.00f);
  colors[ImPlotCol_Crosshairs]    = ImVec4(0.23f, 0.10f, 0.64f, 0.50f);

  style.LineWeight       = 1.5;
  style.Marker           = ImPlotMarker_None;
  style.MarkerSize       = 4;
  style.MarkerWeight     = 1;
  style.FillAlpha        = 1.0f;
  style.ErrorBarSize     = 5;
  style.ErrorBarWeight   = 1.5f;
  style.DigitalBitHeight = 8;
  style.DigitalBitGap    = 4;
  style.PlotBorderSize   = 0;
  style.MinorAlpha       = 1.0f;
  style.MajorTickLen     = ImVec2(0,0);
  style.MinorTickLen     = ImVec2(0,0);
  style.MajorTickSize    = ImVec2(0,0);
  style.MinorTickSize    = ImVec2(0,0);
  style.MajorGridSize    = ImVec2(1.2f,1.2f);
  style.MinorGridSize    = ImVec2(1.2f,1.2f);
  style.PlotPadding      = ImVec2(12,12);
  style.LabelPadding     = ImVec2(5,5);
  style.LegendPadding    = ImVec2(5,5);
  style.MousePosPadding  = ImVec2(5,5);
  style.PlotMinSize      = ImVec2(300,225);
}
