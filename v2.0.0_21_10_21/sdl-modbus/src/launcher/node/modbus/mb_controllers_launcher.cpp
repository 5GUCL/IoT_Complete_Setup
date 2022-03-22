/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#include "node/modbus/modbus_protocol.h"
#include "launcher/node/modbus/mb_controllers_launcher.h"
#include "supervisor/supervisor.h"


mb_edge_controller_worker_t::mb_edge_controller_worker_t (std::shared_ptr<app_supervisor_t> that_supervisor)
{
  that_supervisor_      = that_supervisor;
  modbus_node_registry_ = that_supervisor_->modbus_node_registry_;
  logger_registry_      = that_supervisor_->logger_registry_;

  // Launcher Logger
  launcher_logger_code_ = "launcher";                       // This is reserved code.
  launcher_logger_name_ = "controllers-launcher";            // This is reserved logger name.

  modbus_master_node_controllers_vec_ = that_supervisor_->modbus_master_node_controllers_vec_;
  modbus_server_node_controllers_vec_ = that_supervisor_->modbus_server_node_controllers_vec_;

  // Initialize the launcher. Called by the main app.
}

void mb_edge_controller_worker_t::init_launcher()
{

  // Insert the main logger name in the qnique names map.
  unique_logger_code.insert(std::make_pair(launcher_logger_code_, true));
  unique_logger_name.insert(std::make_pair(launcher_logger_name_, true));


  init_edge_node_service_loggers(launcher_logger_code_, launcher_logger_name_);

  //
  // DESIGN NOTE:
  // Read the config files and build all necessary data structures.
  //
  build_node_launcher_context();

}

void mb_edge_controller_worker_t::init_edge_node_service_loggers(const std::string& controller_logger_code, const std::string& logger_name)
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

bool mb_edge_controller_worker_t::is_controller_group_id_unique(const std::string& controller_grp_id)
{
  std::map<std::string, bool>::iterator itr;

  for(itr = unique_controller_group_id.begin(); itr != unique_controller_group_id.end(); ++itr) {
    if(controller_grp_id.compare(itr->first) == 0) {
      return false;
    }
  }

  return true;
}

bool mb_edge_controller_worker_t::is_controller_id_unique(const std::string& controller_id)
{
  std::map<std::string, bool>::iterator itr;

  for(itr = unique_controller_id.begin(); itr != unique_controller_id.end(); ++itr) {
    if(controller_id.compare(itr->first) == 0) {
      return false;
    }
  }

  return true;
}

bool mb_edge_controller_worker_t::is_controller_logger_code_unique(const std::string& logger_code)
{
  std::map<std::string, bool>::iterator itr;

  for(itr = unique_logger_code.begin(); itr != unique_logger_code.end(); ++itr) {
    if(logger_code.compare(itr->first) == 0) {
      return false;
    }
  }

  return true;
}

bool mb_edge_controller_worker_t::is_controller_listen_address_unique(const std::string& listen_address, int listen_port)
{
  std::map<std::string, bool>::iterator itr;
  std::stringstream adrs;
  adrs << listen_address << ":"<< listen_port;

  for(itr = unique_controller_listen_address.begin(); itr != unique_controller_listen_address.end(); ++itr) {
    if(adrs.str().compare(itr->first) == 0) {
      return false;
    }
  }

  return true;
}

bool mb_edge_controller_worker_t::is_logger_name_unique(const std::string& logger_name)
{
  std::map<std::string, bool>::iterator itr;

  for(itr = unique_logger_name.begin(); itr != unique_logger_name.end(); ++itr) {
    if(logger_name.compare(itr->first) == 0) {
      return false;
    }
  }

  return true;
}

int mb_edge_controller_worker_t::build_node_launcher_context()
{

  // Get the instance of launcher logger.
  auto logger = spdlog::get(launcher_logger_name_);

  // Read the top-level controller registry.
  // Scan each controller and build controller collection.

  // Read the controller group
  // A controller group may have similar controllers and each controller can hold 1..n agents and
  // each agent can hold one node.

  // NOTE: While reading the config JSON files in development, use "Absolute Path"
  // In production, copy the binary in run/bin and all config files to /run/config

  // std::string controller_config_root_path ("/home/maruthi/Vidcentum21/universal-gateway-21/projects/mb-edge-master-logger/config/nodes/");
  // std::ifstream fcontrollers_grp( "/home/maruthi/Vidcentum21/universal-gateway-21/projects/mb-edge-master-logger/config/nodes/node_controller_registry.json" );

  std::string controller_config_root_path("../config/nodes/");

  ////////////
  // Load the root level controller_registry YAML to a JSON.
  // Dependencies. Install yaml2json and json2yaml `npm -i -g yaml2json` `npm -i -g json2yaml`

  std::stringstream root_config_yaml_to_json_file_cmd;
  // Example yaml2json ../config/nodes/node_controller_registry.yaml > ../config/nodes/node_controller_registry.json
  root_config_yaml_to_json_file_cmd << "yaml2json  "
    << controller_config_root_path << "/"
    << "node_controller_registry.yaml"
    << "  >  "
    << controller_config_root_path << "/"
    << "node_controller_registry.json";

  auto rsr = std::system(root_config_yaml_to_json_file_cmd.str().c_str());
  std::cout << "Root level yaml to json file cmd returned -- " << rsr << std::endl;

  // Now start loading the rool level config file.
  std::ifstream fcontrollers_grp("../config/nodes/node_controller_registry.json");
  nlohmann::json jcontrollers_grp;
  fcontrollers_grp >> jcontrollers_grp;

  for(nlohmann::json::iterator cgit = jcontrollers_grp.begin(); cgit != jcontrollers_grp.end(); ++cgit) {
    try {
      nlohmann::json jcontoller_grp_item(*cgit);
      std::string contoller_grp_item_type = jcontoller_grp_item["controller_group"]["type"];
      std::string contoller_grp_item_id = jcontoller_grp_item["controller_group"]["id"];
      std::string contoller_grp_item_config_directory = jcontoller_grp_item["controller_group"]["attributes"]["directory"];

      //
      // DESIGN NOTES
      // Users should always work with YAML file. It is simple to work work with YAML files rather than JSON files.
      // While Application initialization, YAML files are converted to JSON files.
      //
      std::string contoller_grp_item_config_file_name = jcontoller_grp_item["controller_group"]["attributes"]["config_file_name"];
      std::string contoller_grp_item_config_file_name_yaml = jcontoller_grp_item["controller_group"]["attributes"]["config_file_name_yaml"];

      // Load the node level controller_registry YAML to a JSON.
      std::stringstream node_level_yaml_to_json_file_cmd;
      // Example ../config/nodes/modbus/modbus_master_nodes.yaml > ../config/nodes/modbus/modbus_master_nodes.json
      node_level_yaml_to_json_file_cmd  << "yaml2json  "
        << controller_config_root_path << "/"
        << contoller_grp_item_config_directory << "/"
        << contoller_grp_item_config_file_name_yaml
        << "  >  "
        << controller_config_root_path << "/"
        << contoller_grp_item_config_directory << "/"
        << contoller_grp_item_config_file_name;

      auto nsr = std::system(node_level_yaml_to_json_file_cmd.str().c_str());
      std::cout << "Node level yaml " << "(" << contoller_grp_item_config_file_name << ")" << " to json file cmd returned -- " << nsr << std::endl;

      if(is_controller_group_id_unique(contoller_grp_item_id)) {
        unique_controller_group_id.insert(std::make_pair(contoller_grp_item_id, true));
      }
      else {
        logger->critical("#main/1 #controller-group id {} is not unique... exiting...."
          , contoller_grp_item_id);
        exit(0);
      }

      // Controller - MODBUS-MASTER NODES CONTROLLER
      if(contoller_grp_item_type.compare("modbus-master") == 0) {

        build_modbus_master_node_context(controller_config_root_path
          , contoller_grp_item_config_directory
          , contoller_grp_item_config_file_name
          , contoller_grp_item_type
          , contoller_grp_item_id);
      }

      // Controller - MODBUS-MASTER NODES CONTROLLER
      if(contoller_grp_item_type.compare("modbus-server") == 0) {

        build_modbus_server_node_context(controller_config_root_path
          , contoller_grp_item_config_directory
          , contoller_grp_item_config_file_name
          , contoller_grp_item_type
          , contoller_grp_item_id);
      }

    }
    catch(nlohmann::json::exception& e) {
      // Log exception information
      std::stringstream ssReqException;
      ssReqException  << "message: " << e.what()    <<  " "
        << "exception id: " << e.id   << std::endl;
      logger->error("#main/4 #controller #exception {}",  ssReqException.str());
    }
  }

  return 0;
}


int mb_edge_controller_worker_t::run_launcher()
{

  // RUN Controller Type: MODBUS MASTER NODE CONTROLLER
  run_node_controller_context <modbus_master_node_controller_t> (modbus_master_node_controllers_vec_);

  // RUN Controller Type: MODBUS SERVER NODE CONTROLLER
  run_node_controller_context <modbus_server_node_controller_t> (modbus_server_node_controllers_vec_);

  // Controller Type: MODBUS MASTER NODE CONTROLLER
  wait_node_controller_threads <modbus_master_node_controller_t> (modbus_master_node_controllers_vec_);

  // Controller Type: MODBUS SERVER NODE CONTROLLER
  wait_node_controller_threads <modbus_server_node_controller_t> (modbus_server_node_controllers_vec_);

  return 0;
}

int mb_edge_controller_worker_t::build_modbus_master_node_context(const std::string& controller_config_root_path
  , const std::string& contoller_grp_item_config_directory
  , const std::string& contoller_grp_item_config_file_name
  , const std::string& contoller_grp_item_type
  , const std::string& contoller_grp_item_id)
{
  // Get the instance of launcher logger.
  auto logger = spdlog::get(launcher_logger_name_);

  std::stringstream controller_config_root;
  controller_config_root << controller_config_root_path;
  controller_config_root << "/" << contoller_grp_item_config_directory << "/";
  controller_config_root << contoller_grp_item_config_file_name;
  std::ifstream fnodes(controller_config_root.str());
  nlohmann::json jcontrollers;
  fnodes >> jcontrollers;

  // controller group -> controller(s) -> agent(s) -> node(s)(=device(s)) ...
  //                                   -> other configurations. e.g. for modbus, "master", "slaves" etc.
  for(nlohmann::json::iterator cit = jcontrollers.begin(); cit != jcontrollers.end(); ++cit) {
    try {
      nlohmann::json jcontoller(*cit);
      std::string contoller_id = jcontoller["controller"]["id"];
      std::string contoller_logger_code = jcontoller["controller"]["attributes"]["logger_code"];
      std::string contoller_logger_name = jcontoller["controller"]["attributes"]["controller_logger_name"];
      std::string api_server_listen_address = jcontoller["controller"]["attributes"]["api_server_listen_address"];
      int api_server_listen_port = jcontoller["controller"]["attributes"]["api_server_listen_port"];
      int api_server_threads = jcontoller["controller"]["attributes"]["api_server_threads"];

      if(is_controller_id_unique(contoller_id)) {
        unique_controller_id.insert(std::make_pair(contoller_id, true));
      }
      else {
        logger->critical("#{}/{}/2 #controller id {} is not unique... exiting...."
          , contoller_grp_item_type, contoller_grp_item_id, contoller_id);
        exit(0);
      }

      if(is_controller_logger_code_unique(contoller_logger_code)) {
        unique_logger_code.insert(std::make_pair(contoller_logger_code, true));
      }
      else {
        logger->critical("#{}/{}/3 #controller logger code {} is not unique... exiting...."
          , contoller_grp_item_type, contoller_grp_item_id, contoller_logger_code);
        exit(0);
      }

      if(is_logger_name_unique(contoller_logger_name)) {
        unique_logger_name.insert(std::make_pair(contoller_logger_name, true));

        // Register the logger
        logger_registry_->register_new_logger(contoller_logger_code, contoller_logger_name);

        // Now initialize the controller logger.
        init_edge_node_service_loggers(contoller_logger_code, contoller_logger_name);
      }
      else {
        logger->critical("#{}/{}/4 #controller logger name {} is not unique... exiting...."
          , contoller_grp_item_type, contoller_grp_item_id, contoller_logger_name);
        exit(0);
      }

      if(is_controller_listen_address_unique(api_server_listen_address, api_server_listen_port)) {
        std::stringstream adrs;
        adrs << api_server_listen_address << ":"<< api_server_listen_port;
        unique_controller_listen_address.insert(std::make_pair(adrs.str(), true));
      }
      else {
        logger->critical("#{}/{}/5 #controller api server listen {} port {} is not unique... exiting...."
          , contoller_grp_item_type, contoller_grp_item_id, api_server_listen_address, api_server_listen_port);
        exit(0);
      }

      // Create a controller instance.
      modbus_master_node_controller_t controller(contoller_id, contoller_logger_name, api_server_listen_address, api_server_listen_port);
      controller.api_server_threads_ = api_server_threads;

      // Get NCAP blocks.
      nlohmann::json jncap_backends = jcontoller["controller"]["ncap"]["backends"];
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
          
          controller.mb_ncap_mqtt_backend_vec_.emplace_back(std::move(ncap_mqtt));
        }
        
      }

      nlohmann::json jagents = jcontoller["controller"]["agents"];

      for(nlohmann::json::iterator ait = jagents.begin(); ait != jagents.end(); ++ait) {
        nlohmann::json jnode(*ait);

        std::string agent_id       = jnode["agent"]["id"];
        std::string agent_type     = jnode["agent"]["type"];

        std::string agent_logger_name = jnode["agent"]["attributes"]["agent_logger_name"]; // Logger name should be unique per node.
        std::string agent_address  = jnode["agent"]["attributes"]["agent_address"];
        std::string agent_name     = jnode["agent"]["attributes"]["agent_name"];
        std::string agent_api_key  = jnode["agent"]["attributes"]["agent_api_key"];
        long max_nodes             = jnode["agent"]["attributes"]["max_nodes"];

        std::string node_id        = jnode["agent"]["node"]["id"];
        std::string node_type      = jnode["agent"]["node"]["type"];

        std::string node_name      = jnode["agent"]["node"]["attributes"]["node_name"];
        std::string node_address   = jnode["agent"]["node"]["attributes"]["node_address"];
        std::string node_api_key   = jnode["agent"]["node"]["attributes"]["node_api_key"];

        // Check if agent logger name is unique or not.
        if(is_logger_name_unique(agent_logger_name)) {
          unique_logger_name.insert(std::make_pair(agent_logger_name, true));

          // Now initialize the logger.
          // Agent can get logger by agent_logger_name
          init_edge_node_service_loggers(contoller_logger_code, agent_logger_name);

          modbus_master_node_agent_t agent { contoller_id
            , agent_id, agent_type, agent_name, agent_address
            , agent_api_key, agent_logger_name, max_nodes
            , node_id, node_type, node_name, node_address, node_api_key
          };

          if(agent.mb_master_node_ == nullptr) {
            logger->critical("#{}/{} #controller agent {} creation failed... exiting...."
              , contoller_grp_item_type, contoller_grp_item_id, agent_id);
            exit(0);
          }

          // Store the Modbus slave device details from the config data.
          // Modbus Master parameters
          agent.mb_master_node_->mb_slave_device_.mb_slave_id_       = jnode["agent"]["node"]["modbus_master"]["slave_device"]["slave_id"];
          agent.mb_master_node_->mb_slave_device_.mb_slave_address_  = jnode["agent"]["node"]["modbus_master"]["slave_device"]["slave_address"];
          agent.mb_master_node_->mb_slave_device_.mb_slave_port_     = jnode["agent"]["node"]["modbus_master"]["slave_device"]["slave_port"];
          agent.mb_master_node_->mb_slave_device_.mb_refresh_interval_ms_ = jnode["agent"]["node"]["modbus_master"]["slave_device"]["refresh_interval_ms"];

          // Master should query the slave in every 1 sec or more.
          if(agent.mb_master_node_->mb_slave_device_.mb_refresh_interval_ms_ < 1000) {
            agent.mb_master_node_->mb_slave_device_.mb_refresh_interval_ms_ = 1000;
          }

          // Read the coil blocks.
          {
            nlohmann::json jcoil_blocks = jnode["agent"]["node"]["modbus_master"]["slave_device"]["modbus_mapping"]["coil_blocks"];

            for(nlohmann::json::iterator coilit = jcoil_blocks.begin(); coilit != jcoil_blocks.end(); ++coilit) {
              nlohmann::json jcb(*coilit);
              mb_coil_block_t cb;
              cb.block_id_ = jcb["block_id"];
              cb.mb_mapping_bits_start_address_ = jcb["bits_start_address"];
              cb.mb_mapping_nb_bits_ = jcb["nb_bits"];

              if(cb.mb_mapping_nb_bits_ >= VC_MODBUS_MAX_WRITE_BITS) {
                logger->critical("Config Error. Exiting ... Number of coils are invalid - {} ", cb.mb_mapping_nb_bits_);
                exit(0);
              }

              cb.mb_mapping_bits_end_address_ = jcb["bits_end_address"];
              cb.mb_block_refresh_interval_ = jcb["block_refresh_interval_ms"];

              if((cb.mb_block_refresh_interval_ != 0) && (cb.mb_block_refresh_interval_ < 1000)) {
                cb.mb_block_refresh_interval_ = 1000;
              }

              cb.eu_factor_ = jcb["eu_factor"];
              cb.eu_quantity_ = jcb["eu_quantity"];
              cb.eu_manifest_constant_ = jcb["eu_manifest_constant"];

              agent.mb_master_node_->mb_slave_device_.mb_mapping_coil_blocks_.try_emplace(cb.block_id_, std::move(cb));

              if(agent.mb_master_node_->mb_slave_device_.mb_mapping_coil_blocks_.size() >= MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED) {
                logger->critical("Config Error. Exiting ... Number of coil blocks are invalid - {}+ detected ", MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED);
              }
            }
          }
          // Read the input bit blocks.
          {
            nlohmann::json jibit_blocks = jnode["agent"]["node"]["modbus_master"]["slave_device"]["modbus_mapping"]["input_bits_blocks"];

            for(nlohmann::json::iterator ibit = jibit_blocks.begin(); ibit != jibit_blocks.end(); ++ibit) {
              nlohmann::json jib(*ibit);
              mb_input_bit_block_t ib;
              ib.block_id_ = jib["block_id"];
              ib.mb_mapping_input_bits_start_address_ = jib["input_bits_start_address"];
              ib.mb_mapping_nb_input_bits_ = jib["nb_input_bits"];

              if(ib.mb_mapping_nb_input_bits_ >= VC_MODBUS_MAX_READ_BITS) {
                logger->critical("Config Error. Exiting ... Number of input bits are invalid - {} ", ib.mb_mapping_nb_input_bits_);
                exit(0);
              }

              ib.mb_mapping_input_bits_end_address_ = jib["input_bits_end_address"];
              ib.mb_block_refresh_interval_ = jib["block_refresh_interval_ms"];

              if((ib.mb_block_refresh_interval_ != 0) && (ib.mb_block_refresh_interval_ < 1000)) {
                ib.mb_block_refresh_interval_ = 1000;
              }

              ib.eu_factor_ = jib["eu_factor"];
              ib.eu_quantity_ = jib["eu_quantity"];
              ib.eu_manifest_constant_ = jib["eu_manifest_constant"];
              
              agent.mb_master_node_->mb_slave_device_.mb_mapping_input_bit_blocks_.try_emplace(ib.block_id_, std::move(ib));

              if(agent.mb_master_node_->mb_slave_device_.mb_mapping_input_bit_blocks_.size() >= MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED) {
                logger->critical("Config Error. Exiting ... Number of input bits blocks are invalid - {}+ detected ", MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED);
              }
            }
          }

          // Read the Holding Register blocks.
          {
            nlohmann::json jhreg_blocks = jnode["agent"]["node"]["modbus_master"]["slave_device"]["modbus_mapping"]["register_blocks"];

            for(nlohmann::json::iterator hrit = jhreg_blocks.begin(); hrit != jhreg_blocks.end(); ++hrit) {
              nlohmann::json jhr(*hrit);
              mb_holding_register_block_t hr;
              hr.block_id_ = jhr["block_id"];
              hr.mb_mapping_registers_start_address_ = jhr["registers_start_address"];
              hr.mb_mapping_nb_registers_ = jhr["nb_registers"];

              if(hr.mb_mapping_nb_registers_ >= VC_MODBUS_MAX_WRITE_REGISTERS) {
                logger->critical("Config Error. Exiting ... Number of holding registers are invalid - {} ", hr.mb_mapping_nb_registers_);
                exit(0);
              }

              hr.mb_mapping_registers_end_address_ = jhr["registers_end_address"];
              hr.mb_block_refresh_interval_ = jhr["block_refresh_interval_ms"];

              if((hr.mb_block_refresh_interval_ != 0) && (hr.mb_block_refresh_interval_ < 1000)) {
                hr.mb_block_refresh_interval_ = 1000;
              }

              hr.eu_factor_ = jhr["eu_factor"];
              hr.eu_quantity_ = jhr["eu_quantity"];
              hr.eu_manifest_constant_ = jhr["eu_manifest_constant"];
              
              agent.mb_master_node_->mb_slave_device_.mb_mapping_holding_register_blocks_.try_emplace(hr.block_id_, std::move(hr));

              if(agent.mb_master_node_->mb_slave_device_.mb_mapping_holding_register_blocks_.size() >= MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED) {
                logger->critical("Config Error. Exiting ... Number of Holding registers blocks are invalid - {}+ detected ", MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED);
              }
            }
          }
          // Read the Input Register blocks.
          {
            nlohmann::json jinreg_blocks = jnode["agent"]["node"]["modbus_master"]["slave_device"]["modbus_mapping"]["input_register_blocks"];

            for(nlohmann::json::iterator irit = jinreg_blocks.begin(); irit != jinreg_blocks.end(); ++irit) {
              nlohmann::json jir(*irit);
              mb_input_register_block_t ir;
              ir.block_id_ = jir["block_id"];
              ir.mb_mapping_input_registers_start_address_ = jir["input_registers_start_address"];
              ir.mb_mapping_nb_input_registers_ = jir["nb_input_registers"];

              if(ir.mb_mapping_nb_input_registers_ >= VC_MODBUS_MAX_READ_REGISTERS) {
                logger->critical("Config Error. Exiting ... Number of input registers are invalid - {} ", ir.mb_mapping_nb_input_registers_);
                exit(0);
              }

              ir.mb_mapping_input_registers_end_address_ = jir["input_registers_end_address"];
              ir.mb_block_refresh_interval_ = jir["block_refresh_interval_ms"];

              if((ir.mb_block_refresh_interval_ != 0) && (ir.mb_block_refresh_interval_ < 1000)) {
                ir.mb_block_refresh_interval_ = 1000;
              }

              ir.eu_factor_ = jir["eu_factor"];
              ir.eu_quantity_ = jir["eu_quantity"];
              ir.eu_manifest_constant_ = jir["eu_manifest_constant"];

              agent.mb_master_node_->mb_slave_device_.mb_mapping_input_register_blocks_.try_emplace(ir.block_id_, std::move(ir));

              if(agent.mb_master_node_->mb_slave_device_.mb_mapping_input_register_blocks_.size() >= MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED) {
                logger->critical("Config Error. Exiting ... Number of Input registers blocks are invalid - {}+ detected ", MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED);
              }
            }
          }

          // Other data
          agent.mb_master_node_->mb_slave_device_.identifier_
            = jnode["agent"]["node"]["descriptors"]["thing"]["identifier"];

          agent.mb_master_node_->mb_slave_device_.description_
            = jnode["agent"]["node"]["descriptors"]["thing"]["description"];

          agent.mb_master_node_->mb_slave_device_.url_
            = jnode["agent"]["node"]["descriptors"]["thing"]["url"];

          // Measurement webhook URL
          agent.mb_master_node_->mb_slave_device_.measurement_pub_api_url_
            = jnode["agent"]["node"]["modbus_master"]["measurement_pub_api_url"];

          agent.mb_master_node_->mb_slave_device_.jthing_ = {};
          /*
          cpr::Response r = cpr::Get(cpr::Url{agent.mb_master_node_->mb_slave_device_.url_});
          if (r.status_code == 200) {
            agent.mb_master_node_->mb_slave_device_.jthing_ =  nlohmann::json::parse(r.text);
            logger->info("#{}/{}/{} #meta-data  {}"
                        , contoller_grp_item_type, contoller_grp_item_id, agent_id, r.text);
          } else {
            agent.mb_master_node_->mb_slave_device_.jthing_ = {};
            logger->error("#{}/{}/{} #meta-data failed to fetch thing meta data."
                        , contoller_grp_item_type, contoller_grp_item_id, agent_id);
          }
          */
          controller.mb_master_node_agents_vec_.emplace_back(std::move(agent));

          logger->info("#{}/{}/6 #added {}"
            , contoller_grp_item_type, contoller_grp_item_id, agent_id);
        }
        else {
          logger->error("#{}/{}/7 #duplicate {} detected and not starting the agent for the node."
            , contoller_grp_item_type, contoller_grp_item_id, agent_id);
        }
      }

      if(modbus_node_registry_->register_new_node(jcontoller.dump())) {
        controller.jnode_full_config_ = jcontoller;
        modbus_master_node_controllers_vec_->emplace_back(std::move(controller));
      }
      else {
        logger->error("#{}/{}/{}8 #node-registry failed and not starting the controller id {}."
          , contoller_grp_item_type, contoller_grp_item_id, contoller_id, contoller_id);
      }

    }
    catch(nlohmann::json::exception& e) {
      // Log exception information
      std::stringstream ssReqException;
      ssReqException  << "message: " << e.what()    <<  " "
        << "exception id: " << e.id   << std::endl;
      logger->error("#{}/{}/8 #exception {}",  contoller_grp_item_type, contoller_grp_item_id, ssReqException.str());
    }
  }

  return 0;
}

int mb_edge_controller_worker_t::build_modbus_server_node_context(const std::string& controller_config_root_path
  , const std::string& contoller_grp_item_config_directory
  , const std::string& contoller_grp_item_config_file_name
  , const std::string& contoller_grp_item_type
  , const std::string& contoller_grp_item_id)
{
  // Get the instance of launcher logger.
  auto logger = spdlog::get(launcher_logger_name_);

  std::stringstream controller_config_root;
  controller_config_root << controller_config_root_path;
  controller_config_root << "/" << contoller_grp_item_config_directory << "/";
  controller_config_root << contoller_grp_item_config_file_name;
  std::ifstream fnodes(controller_config_root.str());
  nlohmann::json jcontrollers;
  fnodes >> jcontrollers;

  // controller group -> controller(s) -> agent(s) -> node(s)(=device(s)) ...
  //                                   -> other configurations. e.g. for modbus, "master", "slaves" etc.
  for(nlohmann::json::iterator cit = jcontrollers.begin(); cit != jcontrollers.end(); ++cit) {
    try {
      nlohmann::json jcontoller(*cit);
      std::string contoller_id = jcontoller["controller"]["id"];
      std::string contoller_logger_code = jcontoller["controller"]["attributes"]["logger_code"];
      std::string contoller_logger_name = jcontoller["controller"]["attributes"]["controller_logger_name"];
      std::string api_server_listen_address = jcontoller["controller"]["attributes"]["api_server_listen_address"];
      int api_server_listen_port = jcontoller["controller"]["attributes"]["api_server_listen_port"];
      int api_server_threads = jcontoller["controller"]["attributes"]["api_server_threads"];

      if(is_controller_id_unique(contoller_id)) {
        unique_controller_id.insert(std::make_pair(contoller_id, true));
      }
      else {
        logger->critical("#{}/{}/2 #controller id {} is not unique... exiting...."
          , contoller_grp_item_type, contoller_grp_item_id, contoller_id);
        exit(0);
      }

      if(is_controller_logger_code_unique(contoller_logger_code)) {
        unique_logger_code.insert(std::make_pair(contoller_logger_code, true));
      }
      else {
        logger->critical("#{}/{}/3 #controller logger code {} is not unique... exiting...."
          , contoller_grp_item_type, contoller_grp_item_id, contoller_logger_code);
        exit(0);
      }

      if(is_logger_name_unique(contoller_logger_name)) {
        unique_logger_name.insert(std::make_pair(contoller_logger_name, true));

        // Register the logger
        logger_registry_->register_new_logger(contoller_logger_code, contoller_logger_name);

        // Now initialize the controller logger.
        init_edge_node_service_loggers(contoller_logger_code, contoller_logger_name);
      }
      else {
        logger->critical("#{}/{}/4 #controller logger name {} is not unique... exiting...."
          , contoller_grp_item_type, contoller_grp_item_id, contoller_logger_name);
        exit(0);
      }

      if(is_controller_listen_address_unique(api_server_listen_address, api_server_listen_port)) {
        std::stringstream adrs;
        adrs << api_server_listen_address << ":"<< api_server_listen_port;
        unique_controller_listen_address.insert(std::make_pair(adrs.str(), true));
      }
      else {
        logger->critical("#{}/{}/5 #controller api server listen address {} is not unique... exiting...."
          , contoller_grp_item_type, contoller_grp_item_id, api_server_listen_address);
        exit(0);
      }

      // Create a controller instance.
      modbus_server_node_controller_t controller(contoller_id, contoller_logger_name, api_server_listen_address, api_server_listen_port);
      controller.api_server_threads_ = api_server_threads;

      nlohmann::json jagents = jcontoller["controller"]["agents"];

      for(nlohmann::json::iterator ait = jagents.begin(); ait != jagents.end(); ++ait) {
        nlohmann::json jnode(*ait);

        std::string agent_id       = jnode["agent"]["id"];
        std::string agent_type     = jnode["agent"]["type"];

        std::string agent_logger_name = jnode["agent"]["attributes"]["agent_logger_name"]; // Logger name should be unique per node.
        std::string agent_address  = jnode["agent"]["attributes"]["agent_address"];
        std::string agent_name     = jnode["agent"]["attributes"]["agent_name"];
        std::string agent_api_key  = jnode["agent"]["attributes"]["agent_api_key"];
        long max_nodes             = jnode["agent"]["attributes"]["max_nodes"];

        std::string node_id        = jnode["agent"]["node"]["id"];
        std::string node_type      = jnode["agent"]["node"]["type"];

        std::string node_name      = jnode["agent"]["node"]["attributes"]["node_name"];
        std::string node_address   = jnode["agent"]["node"]["attributes"]["node_address"];
        std::string node_api_key   = jnode["agent"]["node"]["attributes"]["node_api_key"];

        // Check if agent logger name is unique or not.
        if(is_logger_name_unique(agent_logger_name)) {
          unique_logger_name.insert(std::make_pair(agent_logger_name, true));

          // Now initialize the logger.
          // Agent can get logger by agent_logger_name
          init_edge_node_service_loggers(contoller_logger_code, agent_logger_name);

          modbus_server_node_agent_t agent { contoller_id
            , agent_id, agent_type, agent_name, agent_address
            , agent_api_key, agent_logger_name, max_nodes
            , node_id, node_type, node_name, node_address, node_api_key
          };

          if(agent.mb_server_node_ == nullptr) {
            logger->critical("#{}/{} #controller agent {} creation failed... exiting...."
              , contoller_grp_item_type, contoller_grp_item_id, agent_id);
            exit(0);
          }

          // Store the Modbus slave device details from the config data.
          // Modbus Server parameters
          agent.mb_server_node_->mb_server_node_device_.mb_slave_id_        = jnode["agent"]["node"]["modbus_server"]["device"]["slave_id"];
          agent.mb_server_node_->mb_server_node_device_.mb_device_address_  = jnode["agent"]["node"]["modbus_server"]["device"]["device_address"];
          agent.mb_server_node_->mb_server_node_device_.mb_device_port_     = jnode["agent"]["node"]["modbus_server"]["device"]["device_port"];
          agent.mb_server_node_->mb_server_node_device_.mb_connections_     = jnode["agent"]["node"]["modbus_server"]["device"]["connections"];
          agent.mb_server_node_->mb_server_node_device_.mb_role_            = jnode["agent"]["node"]["modbus_server"]["device"]["role"];

          // Read the coil blocks.
          {
            nlohmann::json jcoil_blocks = jnode["agent"]["node"]["modbus_server"]["device"]["modbus_mapping"]["coil_blocks"];

            for(nlohmann::json::iterator coilit = jcoil_blocks.begin(); coilit != jcoil_blocks.end(); ++coilit) {
              nlohmann::json jcb(*coilit);
              mb_coil_block_t cb;
              cb.block_id_ = jcb["block_id"];
              cb.mb_mapping_bits_start_address_ = jcb["bits_start_address"];
              cb.mb_mapping_nb_bits_ = jcb["nb_bits"];
              cb.mb_mapping_bits_end_address_ = jcb["bits_end_address"];
              agent.mb_server_node_->mb_server_node_device_.mb_mapping_coil_blocks_.try_emplace(cb.block_id_, std::move(cb));
            }
          }
          // Read the input bit blocks.
          {
            nlohmann::json jibit_blocks = jnode["agent"]["node"]["modbus_server"]["device"]["modbus_mapping"]["input_bits_blocks"];

            for(nlohmann::json::iterator ibit = jibit_blocks.begin(); ibit != jibit_blocks.end(); ++ibit) {
              nlohmann::json jib(*ibit);
              mb_input_bit_block_t ib;
              ib.block_id_ = jib["block_id"];
              ib.mb_mapping_input_bits_start_address_ = jib["input_bits_start_address"];
              ib.mb_mapping_nb_input_bits_ = jib["nb_input_bits"];
              ib.mb_mapping_input_bits_end_address_ = jib["input_bits_end_address"];
              agent.mb_server_node_->mb_server_node_device_.mb_mapping_input_bit_blocks_.try_emplace(ib.block_id_, std::move(ib));
            }
          }
          // Read the Input Register blocks.
          {
            nlohmann::json jinreg_blocks = jnode["agent"]["node"]["modbus_server"]["device"]["modbus_mapping"]["input_register_blocks"];

            for(nlohmann::json::iterator irit = jinreg_blocks.begin(); irit != jinreg_blocks.end(); ++irit) {
              nlohmann::json jir(*irit);
              mb_input_register_block_t ir;
              ir.block_id_ = jir["block_id"];
              ir.mb_mapping_input_registers_start_address_ = jir["input_registers_start_address"];
              ir.mb_mapping_nb_input_registers_ = jir["nb_input_registers"];
              ir.mb_mapping_input_registers_end_address_ = jir["input_registers_end_address"];
              agent.mb_server_node_->mb_server_node_device_.mb_mapping_input_register_blocks_.try_emplace(ir.block_id_, std::move(ir));
            }
          }
          // Read the Holding Register blocks.
          {
            nlohmann::json jhreg_blocks = jnode["agent"]["node"]["modbus_server"]["device"]["modbus_mapping"]["register_blocks"];

            for(nlohmann::json::iterator hrit = jhreg_blocks.begin(); hrit != jhreg_blocks.end(); ++hrit) {
              nlohmann::json jhr(*hrit);
              mb_holding_register_block_t hr;
              hr.block_id_ = jhr["block_id"];
              hr.mb_mapping_registers_start_address_ = jhr["registers_start_address"];
              hr.mb_mapping_nb_registers_ = jhr["nb_registers"];
              hr.mb_mapping_registers_end_address_ = jhr["registers_end_address"];
              agent.mb_server_node_->mb_server_node_device_.mb_mapping_holding_register_blocks_.try_emplace(hr.block_id_, std::move(hr));
            }
          }

          agent.mb_server_node_->mb_server_node_device_.identifier_
            = jnode["agent"]["node"]["descriptors"]["thing"]["identifier"];

          agent.mb_server_node_->mb_server_node_device_.description_
            = jnode["agent"]["node"]["descriptors"]["thing"]["description"];

          agent.mb_server_node_->mb_server_node_device_.url_
            = jnode["agent"]["node"]["descriptors"]["thing"]["url"];

          agent.mb_server_node_->mb_server_node_device_.jthing_ = {};

          /*
          cpr::Response r = cpr::Get(cpr::Url{agent.mb_server_node_->mb_server_node_device_.url_});
          if (r.status_code == 200) {
            agent.mb_server_node_->mb_server_node_device_.jthing_ =  nlohmann::json::parse(r.text);
            logger->info("#{}/{}/{} #meta-data  {}"
                        , contoller_grp_item_type, contoller_grp_item_id, agent_id, r.text);
          } else {
            agent.mb_server_node_->mb_server_node_device_.jthing_ = {};
            logger->error("#{}/{}/{} #meta-data failed to fetch thing meta data."
                        , contoller_grp_item_type, contoller_grp_item_id, agent_id);
          }
          */
          controller.mb_server_node_agents_vec_.emplace_back(std::move(agent));

          logger->info("#{}/{}/6 #added {}"
            , contoller_grp_item_type, contoller_grp_item_id, agent_id);
        }
        else {
          logger->error("#{}/{}/7 #duplicate {} detected and not starting the agent for the node."
            , contoller_grp_item_type, contoller_grp_item_id, agent_id);
        }
      }

      if(modbus_node_registry_->register_new_node(jcontoller.dump())) {
        controller.jnode_full_config_ = jcontoller;
        modbus_server_node_controllers_vec_->emplace_back(std::move(controller));
      }
      else {
        logger->error("#{}/{}/{}8 #node-registry failed and not starting the controller id {}."
          , contoller_grp_item_type, contoller_grp_item_id, contoller_id, contoller_id);
      }

    }
    catch(nlohmann::json::exception& e) {
      // Log exception information
      std::stringstream ssReqException;
      ssReqException  << "message: " << e.what()    <<  " "
        << "exception id: " << e.id   << std::endl;
      logger->error("#{}/{}/8 #exception {}",  contoller_grp_item_type, contoller_grp_item_id, ssReqException.str());
    }
  }

  return 0;
}


template <typename NODE_CONTROLLER_T>
int mb_edge_controller_worker_t::run_node_controller_context(std::shared_ptr<std::vector<NODE_CONTROLLER_T>> controller_vec)
{
  std::transform(controller_vec->begin(), controller_vec->end(), controller_vec->begin(), [](NODE_CONTROLLER_T &c) {
    c.nwrapper_thread_ = std::make_shared<std::thread>([&c]() {
      c.wrapper_fn();
    });
    return c;
  });
  return 0;
}

template <typename NODE_CONTROLLER_T>
int mb_edge_controller_worker_t::wait_node_controller_threads(std::shared_ptr<std::vector<NODE_CONTROLLER_T>> controller_vec)
{
  std::for_each(controller_vec->begin(), controller_vec->end(), [](NODE_CONTROLLER_T &c) {
    c.nwrapper_thread_->join();
  });
  return 0;
}


/////////////////////////////////////////////
void mb_edge_controller_launcher_t::wrapper_fn()
{
  if(mb_ctrl_worker_) {
    mb_ctrl_worker_->run_launcher();
  }
}

mb_edge_controller_launcher_t::mb_edge_controller_launcher_t (std::shared_ptr<app_supervisor_t> that_supervisor)
{
  mb_ctrl_worker_ = std::make_shared<mb_edge_controller_worker_t>(that_supervisor);
}

void mb_edge_controller_launcher_t::init_launcher()
{
  if(mb_ctrl_worker_) {
    mb_ctrl_worker_->init_launcher();
  }
}
