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
#include "sf-sys/utils/time_utils.h"
#include "sf-sys/utils/cpp_prog_utils.h"
#include "ncap/network/mqtt/ncap_mqtt_backend.h"
#include "supervisor/supervisor.h"

void mb_ncap_mqtt_ws_client_worker_t::work()
{
  std::stringstream ss;
  ss << "ws://" << mb_master_contoller_address_ << ":" << mb_master_contoller_port_;

  mb_ncap_mqtt_ws_client_t ws_client(ss.str(), ncap_mqtt_env_obj_, msg_primary_channel_, msg_secondary_channel_);

  // This call doesn't return.
  ws_client.run();
}

/////////////////////////////////////////////

mb_ncap_mqtt_backend_t::mb_ncap_mqtt_backend_t (std::shared_ptr<app_supervisor_t> that_supervisor)
{
  that_supervisor_  = that_supervisor;
  mb_node_registry_ = that_supervisor_->modbus_node_registry_;
  logger_registry_  = that_supervisor_->logger_registry_;
};

void mb_ncap_mqtt_backend_t::init_ncap_mqtt_service_loggers(const std::string& controller_logger_code, const std::string& logger_name)
{
  // Create file rotating logger with 10mb size max and 10 rotated files
  std::stringstream ssLoggerPath;

  // Log file path: "../logs/<controller-logger-code>/<logger-name>/<logger-name>-rotating.txt"
  ssLoggerPath  << "../logs/edge-data-logger/" << controller_logger_code <<  "/" << logger_name <<  "/" << logger_name << "-rotating.txt";

  auto edge_node_srvc_logger = spdlog::rotating_logger_mt(logger_name, ssLoggerPath.str(), 1048576 * 10, 10);

#ifdef LOG_LEVEL_PRODUCTION
  edge_node_srvc_logger->set_level(spdlog::level::err);
#else
  edge_node_srvc_logger->set_level(spdlog::level::debug);
#endif

  spdlog::flush_every(std::chrono::seconds(3));
}

std::string mb_ncap_mqtt_backend_t::prepare_device_id
(const std::string& controller_id, const std::string& agent_id, const std::string& node_id)
{
  std::stringstream v_id;
  v_id << controller_id << "/" << agent_id << "/" << node_id;
  return v_id.str();
}

int mb_ncap_mqtt_backend_t::ncap_mb_coil_measurement
(const std::string& v_device_id, int block_id, int start_address
  , int coil_number, const modbus_master_coil_bit_single_measurement_instance_t& measurement) 
{
  // NCAP MQTT HOOK
  
  return 0;
}

int mb_ncap_mqtt_backend_t::ncap_mb_input_bit_measurement
(const std::string& v_device_id, int block_id, int start_address
  , int input_bit_number, const modbus_master_input_bit_single_measurement_instance_t& measurement)
{
  // NCAP MQTT HOOK
  
  return 0;
}

int mb_ncap_mqtt_backend_t::ncap_mb_holding_register_measurement
(const std::string& v_device_id, int block_id, int start_address
  , int holding_reg_number, const modbus_master_holding_register_single_measurement_instance_t& measurement)
{
  // NCAP MQTT HOOK
  
  return 0;
}

int mb_ncap_mqtt_backend_t::ncap_mb_input_register_measurement
(const std::string& v_device_id, int block_id, int start_address
  , int input_reg_number, const modbus_master_input_register_single_measurement_instance_t& measurement)
{
  // NCAP MQTT HOOK
  
  return 0;
}


int mb_ncap_mqtt_backend_t::init_mb_ncap_mqtt()
{
  // Initialize the logger
  init_ncap_mqtt_service_loggers(SMART_LOGGER_NCAP_MQTT_LOGGER_CODE_, SMART_LOGGER_NCAP_MQTT_LOGGER_NAME_);

  // Get the instance of launcher logger.
  auto logger = spdlog::get(SMART_LOGGER_NCAP_MQTT_LOGGER_NAME_);

  //
  // DESIGN NOTES
  // Now initialize the SObjectizer to process the received modbus messages.
  //
  ncap_mqtt_env_obj_ = std::make_shared<so_5::wrapped_env_t>();
  msg_primary_channel_ = so_5::create_mchain(*ncap_mqtt_env_obj_);
  auto msg_primary_channel_closer = so_5::auto_close_drop_content(msg_primary_channel_);

  msg_secondary_channel_ = so_5::create_mchain(msg_primary_channel_->environment());
  auto msg_secondary_channel_closer = so_5::auto_close_drop_content(msg_secondary_channel_);

  // Read the global config / node registry.
  for(auto strnode : mb_node_registry_->node_registry_) {
    try {
      nlohmann::json jnode = nlohmann::json::parse(strnode);
      std::string controller = jnode["controller"]["type"];

      // Controller - MODBUS-MASTER
      if(controller.compare("modbus-master") == 0) {
        mb_ncap_mqtt_ws_client_worker_t s1;
        s1.mb_master_contoller_address_ = jnode["controller"]["attributes"]["api_server_listen_address"];
        s1.mb_master_contoller_port_    = jnode["controller"]["attributes"]["api_server_listen_port"];
        s1.mb_controller_id_            = jnode["controller"]["id"];
        s1.ncap_mqtt_env_obj_           = ncap_mqtt_env_obj_;
        s1.msg_primary_channel_         = msg_primary_channel_;
        s1.msg_secondary_channel_       = msg_secondary_channel_;
        ncap_mqtt_ws_client_workers_vec_.emplace_back(std::move(s1));
        
        // Get NCAP blocks.
        nlohmann::json jncap_backends = jnode["controller"]["ncap"]["backends"];
        for(nlohmann::json::iterator ncapit = jncap_backends.begin(); ncapit != jncap_backends.end(); ++ncapit) {
          nlohmann::json jnb(*ncapit);
          
          std::string ncap = jnb["ncap_backend"];
          if(ncap.compare("mqtt") == 0) {
            ncap_mqtt_backend_config_t ncap_mqtt;
            ncap_mqtt.ncap_backend_ = jnb["ncap_backend"];
            ncap_mqtt.mqtt_broker_address_ = jnb["mqtt_broker_address"];
            ncap_mqtt.mqtt_broker_port_ = jnb["mqtt_broker_port"];
            ncap_mqtt.encryption_ = jnb["encryption"];
            ncap_mqtt.fc_rules_ = jnb["fc_rules"];
            
            ncap_backend_mqtt_vec_.emplace_back(std::move(ncap_mqtt));
          }
        }
      }
    } catch(nlohmann::json::exception& e) {
      // Log exception information
      std::stringstream ssReqException;
      ssReqException  << "message: " << e.what()    <<  " "
        << "exception id: " << e.id   << std::endl;
      logger->error("#init_mb_dataset_recorder ill-formed controller configuration - {}.", ssReqException.str());
    }
  }

  //
  // DESIGN NOTES
  // Now launch the SObjectizer to process the received modbus messages.
  //
  dsthread_ = std::make_shared<std::thread>([this]() {
    message_recording_fn();
  });

  //
  // DESIGN NOTES
  // Now launch the Web Socket Client to connect with the local/or configured server
  // Provided node registry MAY have more than one MODBUS-MASTER CONTROLLERS
  // Each MODBUS-MASTER CONTROLLER published the data from configured master nodes.
  // [Master Controllers] -> [Publishes messages read from the SLAVE nodes]
  // Separate threads receive messages over Web Sockets for each MODBUS-MASTER CONTROLLER.
  //
  std::transform(ncap_mqtt_ws_client_workers_vec_.begin(), ncap_mqtt_ws_client_workers_vec_.end()
  , ncap_mqtt_ws_client_workers_vec_.begin(), [](mb_ncap_mqtt_ws_client_worker_t &s) {
    s.ws_thread_ = std::make_shared<std::thread>([&s]() {
      s.work();
    });
    return s;
  });

  //
  // Join the SO environment and thread. This should be the last call and does not return.
  //
  auto ws_so_joiner = so_5::auto_join(*dsthread_);

  return 0;
}

///////////////////////////////////////////////////////////
void mb_ncap_mqtt_backend_t::message_recording_fn()
{
  // Get the instance of launcher logger.
  auto logger = spdlog::get(SMART_LOGGER_NCAP_MQTT_LOGGER_NAME_);

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
  , [this, &logger](ncap_mqtt_mailbox_message_t mb_msg) {
    //
    // DESIGN NOTES
    // Store the data received.
    // Device[identifier] -> Register [number] -> [measurement instamce]
    //

    try {
      nlohmann::json jmsg = nlohmann::json::parse(mb_msg.msg_received_);
      std::string _msg_type = jmsg["message_type"];

      ///////////////////////////////////////
      if(_msg_type.compare("coil_bit_reading") == 0) {

        std::string _contoller_id = jmsg["controller_id"];
        std::string _agent_id     = jmsg["agent_id"];
        std::string _node_id      = jmsg["node_id"];

        std::string v_id;
        v_id = prepare_device_id(_contoller_id, _agent_id, _node_id);

        // We found a device that the incoming message belongs to.
        modbus_master_coil_bit_single_measurement_instance_t cm;
        cm.mb_block_id_ = jmsg["block_id"];
        cm.mb_mapping_bits_start_address_ = jmsg["coil_bits_start_address"];
        cm.mb_mapping_bit_number_ = jmsg["coil_bit_number"];

        // Refer to Modbus Standard of bits, registers indexing.
        cm.mb_mapping_bit_number_ = cm.mb_mapping_bit_number_ + 1;

        cm.measured_value_  = jmsg["measured_value"];
        cm.time_instance_   = jmsg["time_instance"];
        cm.eu_factor_       = jmsg["eu_factor"];
        cm.eu_quantity_     = jmsg["eu_quantity"];
        cm.eu_manifest_constant_ = jmsg["eu_manifest_constant"];
      
        // Send the data to network (NCAP)
        ncap_mb_coil_measurement(v_id, cm.mb_block_id_, cm.mb_mapping_bits_start_address_, cm.mb_mapping_bit_number_, cm);
      }
      ///////////////////////////////////////
      else if(_msg_type.compare("input_bit_reading") == 0) {

        std::string _contoller_id = jmsg["controller_id"];
        std::string _agent_id     = jmsg["agent_id"];
        std::string _node_id      = jmsg["node_id"];

        std::string v_id;
        v_id = prepare_device_id(_contoller_id, _agent_id, _node_id);

        // We found a device that the incoming message belongs to.
        modbus_master_input_bit_single_measurement_instance_t ibm;
        ibm.mb_block_id_ = jmsg["block_id"];
        ibm.mb_mapping_input_bits_start_address_ = jmsg["input_bits_start_address"];
        ibm.mb_mapping_input_bit_number_ = jmsg["input_bit_number"];

        // Refer to Modbus Standard of bits, registers indexing.
        ibm.mb_mapping_input_bit_number_ = ibm.mb_mapping_input_bit_number_ + 1;

        ibm.measured_value_ = jmsg["measured_value"];
        ibm.time_instance_  = jmsg["time_instance"];

        ibm.eu_factor_    = jmsg["eu_factor"];
        ibm.eu_quantity_  = jmsg["eu_quantity"];
        ibm.eu_manifest_constant_ = jmsg["eu_manifest_constant"];
      
        // Send the data to network (NCAP)
        ncap_mb_input_bit_measurement(v_id, ibm.mb_block_id_, ibm.mb_mapping_input_bits_start_address_, ibm.mb_mapping_input_bit_number_, ibm);
        
      }
      ////////////////////////////////////////
      else if(_msg_type.compare("holding_register_reading") == 0) {

        std::string _contoller_id = jmsg["controller_id"];
        std::string _agent_id     = jmsg["agent_id"];
        std::string _node_id      = jmsg["node_id"];

        std::string v_id;
        v_id = prepare_device_id(_contoller_id, _agent_id, _node_id);

        // We found a device that the incoming message belongs to.
        modbus_master_holding_register_single_measurement_instance_t hm;
        hm.mb_block_id_ = jmsg["block_id"];
        hm.mb_mapping_registers_start_address_ = jmsg["registers_start_address"];
        hm.mb_mapping_holding_register_number_ = jmsg["holding_register_number"];
        // Refer to Modbus Standard of bits, registers indexing.
        hm.mb_mapping_holding_register_number_ = hm.mb_mapping_holding_register_number_ + 1;

        hm.measured_value_ = jmsg["measured_value"];
        hm.time_instance_  = jmsg["time_instance"];
        
        hm.eu_factor_   = jmsg["eu_factor"];
        hm.eu_quantity_ = jmsg["eu_quantity"];
        hm.eu_manifest_constant_ = jmsg["eu_manifest_constant"];
      
        // Send the data to network (NCAP)
        ncap_mb_holding_register_measurement(v_id, hm.mb_block_id_, hm.mb_mapping_registers_start_address_, hm.mb_mapping_holding_register_number_, hm);
      }
      ////////////////////////////////////////
      else if(_msg_type.compare("input_register_reading") == 0) {

        std::string _contoller_id = jmsg["controller_id"];
        std::string _agent_id     = jmsg["agent_id"];
        std::string _node_id      = jmsg["node_id"];

        std::string v_id;
        v_id = prepare_device_id(_contoller_id, _agent_id, _node_id);

        // We found a device that the incoming message belongs to.
        modbus_master_input_register_single_measurement_instance_t im;
        im.mb_block_id_ = jmsg["block_id"];
        im.mb_mapping_input_registers_start_address_ = jmsg["input_registers_start_address"];
        im.mb_mapping_input_register_number_ = jmsg["input_register_number"];
        // Refer to Modbus Standard of bits, registers indexing.
        im.mb_mapping_input_register_number_ = im.mb_mapping_input_register_number_ + 1;
        im.measured_value_ = jmsg["measured_value"];
        im.time_instance_  = jmsg["time_instance"];

        im.eu_factor_   = jmsg["eu_factor"];
        im.eu_quantity_ = jmsg["eu_quantity"];
        im.eu_manifest_constant_ = jmsg["eu_manifest_constant"];
      
        // Send the data to network (NCAP)
        ncap_mb_input_register_measurement(v_id, im.mb_block_id_, im.mb_mapping_input_registers_start_address_, im.mb_mapping_input_register_number_, im);
      }
      else {
        logger->error("#receive_case #mb_msg.msg_received_ unsupported message type {}.", _msg_type);
      }
    }
    catch(nlohmann::json::exception& e) {
      // Log exception information
      std::stringstream ssReqException;
      ssReqException  << "message: " << e.what()    <<  " "
        << "exception id: " << e.id   << std::endl;
      logger->error("#receive_case #mb_msg.msg_received_ ill-formed message {} received. Excepton: {}."
        , mb_msg.msg_received_, ssReqException.str());
    }
  }
  ),
  so_5::receive_case(msg_secondary_channel_
  , [&logger](ncap_mqtt_mailbox_message_t mb_msg) {
    // Placeholder to handle error messages, metric messages.
    logger->info("Received other message {} on secondary channel.", mb_msg.msg_received_);
  }
  )
  
  );

  do {
    so_5::select(prepared);
  }
  while(!stop);
}

