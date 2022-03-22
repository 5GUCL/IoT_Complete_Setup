/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
 */
/*========================END LICENSE NOTICE========================*/

// Logging
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include "ncap_app_constants.h"
#include "ncap_config_registry.h"

ncap_config_registry_t::ncap_config_registry_t ( const std::string& config_yaml_file )
                       :config_yaml_file_(config_yaml_file)
{
  // No special init!
}

bool
ncap_config_registry_t::is_ncap_id_unique( const std::string& ncap_id )
{
  std::map<std::string, bool>::iterator itr;

  for(itr = unique_ncap_id_.begin(); itr != unique_ncap_id_.end(); ++itr) {
    if(ncap_id.compare(itr->first) == 0) {
      return false;
    }
  }
  return true;
}

bool
ncap_config_registry_t::is_ncap_api_server_unique(const std::string& ncap_api_server)
{
  std::map<std::string, bool>::iterator itr;

  for(itr = unique_ncap_api_server_.begin(); itr != unique_ncap_api_server_.end(); ++itr) {
    if(ncap_api_server.compare(itr->first) == 0) {
      return false;
    }
  }
  return true;
}


bool
ncap_config_registry_t::load_ncap_config_registry ()
{
  auto logger = spdlog::get(NCAP_L21_LOGGER);

  std::stringstream ncap_config_yaml_to_json_file_cmd;
  ncap_config_yaml_to_json_file_cmd << "yaml2json  "
    << "../ncap-fin-l21-config/"
    << config_yaml_file_
    << "  >  "
    << "../ncap-fin-l21-config/ncap_prod_config.json";

  // The file shall be in ../ncap-fin-l21-config/
  config_json_file_ = "ncap_prod_config.json";

  auto rsr = std::system(ncap_config_yaml_to_json_file_cmd.str().c_str());
  std::cout << "NCAP Config YAML file to JSON file cmd returned -- " << rsr << std::endl;
  SPDLOG_LOGGER_INFO(logger, "NCAP Config YAML file to JSON file cmd returned -- {}", rsr);

  // Now start loading the rool level config file.
  std::ifstream fncap_config_reg("../ncap-fin-l21-config/ncap_prod_config.json");
  fncap_config_reg >> jncap_config_reg_;

  for(nlohmann::json::iterator cgit = jncap_config_reg_.begin(); cgit != jncap_config_reg_.end(); ++cgit) {
    try {
      nlohmann::json jncap_obj(*cgit);

      // Load NCAP Base Objects.
      std::string ncap_type = jncap_obj["ncap"]["type"];

      // Supported types of NCAP objects.
      if (ncap_type.compare("modbus_mqtt_expert_system") == 0) {

        // Create an enmpty NCAP object and start filling it.
        auto ncap_obj = std::make_shared<ncap_mqtt_modbus_config_t> () ;

        std::string ncap_id = jncap_obj["ncap"]["id"];
        std::string ncap_api_address = jncap_obj["ncap"]["attributes"]["api_server_listen_address"];
        long ncap_api_port = jncap_obj["ncap"]["attributes"]["api_server_listen_port"];

        if(is_ncap_id_unique(ncap_id)) {
          unique_ncap_id_.insert(std::make_pair(ncap_id, true));
        }
        else {
          // Let's not run the system if NCAP objects are not well configured.
          SPDLOG_LOGGER_CRITICAL(logger, "NCAP Object ID {} is not unique! Exiting.", ncap_id);
          exit(0);
        }

        // Load the NCAP Base object into registry.
        std::stringstream ncap_api_server;
        ncap_api_server << ncap_api_address << ":"<< ncap_api_port;
        if(is_ncap_api_server_unique(ncap_api_server.str())) {
          unique_ncap_api_server_.insert(std::make_pair(ncap_api_server.str(), true));
        }
        else {
          // Let's not run the system if NCAP base objects are not well configured.
          SPDLOG_LOGGER_CRITICAL(logger, "NCAP API Address {} is not unique! Exiting.", ncap_api_server.str());
          exit(0);
        }

        // Fill up the Base NCAP Object.
        ncap_obj->base_config_.id_ = jncap_obj["ncap"]["id"];
        ncap_obj->base_config_.type_ = jncap_obj["ncap"]["type"];
        ncap_obj->base_config_.name_ = jncap_obj["ncap"]["attributes"]["name"];
        ncap_obj->base_config_.version_ = jncap_obj["ncap"]["version"];
        ncap_obj->base_config_.description_ = jncap_obj["ncap"]["attributes"]["description"];
        ncap_obj->base_config_.api_server_threads_ = jncap_obj["ncap"]["attributes"]["api_server_threads"];
        ncap_obj->base_config_.api_server_address_ = jncap_obj["ncap"]["attributes"]["api_server_listen_address"];
        ncap_obj->base_config_.api_server_listen_port_ = jncap_obj["ncap"]["attributes"]["api_server_listen_port"];

        // Fill up the Auth (device auth) of NCAP Object.
        ncap_obj->auth_config_.client_id_ = jncap_obj["ncap"]["auth"]["client_id"];
        ncap_obj->auth_config_.client_secret_ = jncap_obj["ncap"]["auth"]["client_secret"];
        ncap_obj->auth_config_.grant_type_ = jncap_obj["ncap"]["auth"]["grant_type"];
        ncap_obj->auth_config_.password_ = jncap_obj["ncap"]["auth"]["password"];
        ncap_obj->auth_config_.password_encryption_ = jncap_obj["ncap"]["auth"]["password_encryption"];
        ncap_obj->auth_config_.username_ = jncap_obj["ncap"]["auth"]["username"];
        ncap_obj->auth_config_.token_endpoint_ = jncap_obj["ncap"]["auth"]["token_endpoint"];

        // Fill up the APP API Server Endpoints and other details.
        ncap_obj->api_server_config_.base_url_ = jncap_obj["ncap"]["api_server"]["base_url"];
        ncap_obj->api_server_config_.device_token_endpoint_ = jncap_obj["ncap"]["api_server"]["device_token_endpoint"];;

        // Fill up MQTT broker of NCAP Object.
        ncap_obj->mqtt_config_.mqtt_borker_.host_ = jncap_obj["ncap"]["mqtt"]["broker"]["host"];
        ncap_obj->mqtt_config_.mqtt_borker_.port_ = jncap_obj["ncap"]["mqtt"]["broker"]["port"];
        ncap_obj->mqtt_config_.mqtt_borker_.ca_cert_ = jncap_obj["ncap"]["mqtt"]["broker"]["ca_cert"];
        ncap_obj->mqtt_config_.mqtt_borker_.mqtt_client_id_ = jncap_obj["ncap"]["mqtt"]["broker"]["mqtt_client_id"];
        ncap_obj->mqtt_config_.mqtt_borker_.protocol_ = jncap_obj["ncap"]["mqtt"]["broker"]["protocol"];
        ncap_obj->mqtt_config_.mqtt_borker_.secure_ = jncap_obj["ncap"]["mqtt"]["broker"]["secure"];

        // Usename is taken from the AUTH Object. "username": "$auth.username"
        ncap_obj->mqtt_config_.mqtt_borker_.username_ = jncap_obj["ncap"]["auth"]["username"];

        // Password is taken from the ACCESS TOKEN Object. "password": "$ACCESS_TOKEN"
        // Access token is obtained from the token_endpoint of the OIDC server.
        ncap_obj->mqtt_config_.mqtt_borker_.password_ = jncap_obj["ncap"]["mqtt"]["broker"]["password"];

        // Fill up IoT fields of NCAP Object.
        // IoT --> Asset fields of the NCAP Object.
        ncap_obj->iot_mb_config_.asset_.id_ = jncap_obj["ncap"]["iot"]["asset"]["id"];
        ncap_obj->iot_mb_config_.asset_.name_ = jncap_obj["ncap"]["iot"]["asset"]["name"];
        ncap_obj->iot_mb_config_.asset_.description_ = jncap_obj["ncap"]["iot"]["asset"]["description"];
        ncap_obj->iot_mb_config_.asset_.meta_data_url_ = jncap_obj["ncap"]["iot"]["asset"]["meta_data_url"];

        // IoT --> data_source --> subscription fields of the NCAP Object.
        ncap_obj->iot_mb_config_.data_source_.subscription_.host_ = jncap_obj["ncap"]["iot"]["data_source"]["subscription"]["host"];
        ncap_obj->iot_mb_config_.data_source_.subscription_.port_ = jncap_obj["ncap"]["iot"]["data_source"]["subscription"]["port"];
        ncap_obj->iot_mb_config_.data_source_.subscription_.ca_cert_ = jncap_obj["ncap"]["iot"]["data_source"]["subscription"]["ca_cert"];
        ncap_obj->iot_mb_config_.data_source_.subscription_.protocol_ = jncap_obj["ncap"]["iot"]["data_source"]["subscription"]["protocol"];
        ncap_obj->iot_mb_config_.data_source_.subscription_.service_enabled_ = jncap_obj["ncap"]["iot"]["data_source"]["subscription"]["service_enabled"];
        ncap_obj->iot_mb_config_.data_source_.subscription_.payload_mapping_ = jncap_obj["ncap"]["iot"]["data_source"]["subscription"]["payload_mapping"];

        // Usename is taken from the AUTH Object. "username": "$auth.username"
        ncap_obj->iot_mb_config_.data_source_.subscription_.username_ = jncap_obj["ncap"]["auth"]["username"];

        // Password is taken from the ACCESS TOKEN Object. "password": "$ACCESS_TOKEN"
        // Access token is obtained from the token_endpoint of the OIDC server.
        ncap_obj->iot_mb_config_.data_source_.subscription_.password_ = jncap_obj["ncap"]["iot"]["data_source"]["subscription"]["password"];

        // IoT --> data_source --> controller_api fields of the NCAP Object.
        ncap_obj->iot_mb_config_.data_source_.controller_api_.host_ = jncap_obj["ncap"]["iot"]["data_source"]["controller_api"]["host"];
        ncap_obj->iot_mb_config_.data_source_.controller_api_.port_ = jncap_obj["ncap"]["iot"]["data_source"]["controller_api"]["port"];
        ncap_obj->iot_mb_config_.data_source_.controller_api_.api_protocol_ = jncap_obj["ncap"]["iot"]["data_source"]["controller_api"]["api_protocol"];
        ncap_obj->iot_mb_config_.data_source_.controller_api_.service_enabled_ = jncap_obj["ncap"]["iot"]["data_source"]["controller_api"]["service_enabled"];
        ncap_obj->iot_mb_config_.data_source_.controller_api_.ca_cert_ = jncap_obj["ncap"]["iot"]["data_source"]["controller_api"]["ca_cert"];
        ncap_obj->iot_mb_config_.data_source_.controller_api_.payload_mapping_ = jncap_obj["ncap"]["iot"]["data_source"]["controller_api"]["payload_mapping"];

        // Usename is taken from the AUTH Object. "username": "$auth.username"
        ncap_obj->iot_mb_config_.data_source_.controller_api_.username_ = jncap_obj["ncap"]["auth"]["username"];

        // Password is taken from the ACCESS TOKEN Object. "password": "$ACCESS_TOKEN"
        // Access token is obtained from the token_endpoint of the OIDC server.
        ncap_obj->iot_mb_config_.data_source_.controller_api_.password_ = jncap_obj["ncap"]["iot"]["data_source"]["controller_api"]["password"];

        // IoT --> expert_system - an array of rule engines.
        {
          nlohmann::json jncap_expert_sys_array = jncap_obj["ncap"]["iot"]["expert_system"];
          for(nlohmann::json::iterator esit = jncap_expert_sys_array.begin(); esit != jncap_expert_sys_array.end(); ++esit) {
            nlohmann::json jncap_es_obj(*esit);
            ncap_expert_system_t es;
            es.id_ = jncap_es_obj["id"];
            es.facts_db_ = jncap_es_obj["facts_db"];
            es.inference_engine_ = jncap_es_obj["inference_engine"];
            es.priority_ = jncap_es_obj["priority"];
            es.rules_ = jncap_es_obj["rules"];

            ncap_obj->iot_mb_config_.expert_system_map_.try_emplace(es.id_, std::move(es));
          }
        }

        // IoT --> payload_mapping -> modbus_mapping
        ncap_obj->iot_mb_config_.mb_payload_mapping_.contoller_id_
          = jncap_obj["ncap"]["iot"]["payload_mapping"]["modbus_mapping"]["controller"]["id"];
        ncap_obj->iot_mb_config_.mb_payload_mapping_.agent_id_
          = jncap_obj["ncap"]["iot"]["payload_mapping"]["modbus_mapping"]["agent"]["id"];
        ncap_obj->iot_mb_config_.mb_payload_mapping_.node_id_
          = jncap_obj["ncap"]["iot"]["payload_mapping"]["modbus_mapping"]["node"]["id"];

        // IoT --> payload_mapping --> modbus_mapping --> Coil blocks.
        {
          nlohmann::json jcoil_blocks = jncap_obj["ncap"]["iot"]["payload_mapping"]["modbus_mapping"]["coil_blocks"];

          for(nlohmann::json::iterator coilit = jcoil_blocks.begin(); coilit != jcoil_blocks.end(); ++coilit) {
            nlohmann::json jcb(*coilit);
            mb_coil_block_t cb;
            cb.block_id_ = jcb["block_id"];
            cb.mb_mapping_bits_start_address_ = jcb["bits_start_address"];
            cb.mb_mapping_nb_bits_ = jcb["nb_bits"];
            if(cb.mb_mapping_nb_bits_ >= VC_MODBUS_MAX_WRITE_BITS) {
              SPDLOG_LOGGER_CRITICAL(logger, "Config Error. Exiting ... Number of coils are invalid - {}", cb.mb_mapping_nb_bits_);
              exit(0);
            }
            cb.mb_mapping_bits_end_address_ = jcb["bits_end_address"];
            cb.eu_factor_ = jcb["eu_factor"];
            cb.eu_quantity_ = jcb["eu_quantity"];
            cb.eu_manifest_constant_ = jcb["eu_manifest_constant"];

            ncap_obj->iot_mb_config_.mb_payload_mapping_.mb_mapping_coil_blocks_.try_emplace(cb.block_id_, std::move(cb));
          }
        }

        // IoT --> payload_mapping --> modbus_mapping --> Input bit blocks.
        {
          nlohmann::json jibit_blocks = jncap_obj["ncap"]["iot"]["payload_mapping"]["modbus_mapping"]["input_bits_blocks"];

          for(nlohmann::json::iterator ibit = jibit_blocks.begin(); ibit != jibit_blocks.end(); ++ibit) {
            nlohmann::json jib(*ibit);
            mb_input_bit_block_t ib;
            ib.block_id_ = jib["block_id"];
            ib.mb_mapping_input_bits_start_address_ = jib["input_bits_start_address"];
            ib.mb_mapping_nb_input_bits_ = jib["nb_input_bits"];

            if(ib.mb_mapping_nb_input_bits_ >= VC_MODBUS_MAX_READ_BITS) {
              SPDLOG_LOGGER_CRITICAL(logger, "Config Error. Exiting ... Number of input bits are invalid - {}", ib.mb_mapping_nb_input_bits_);
              exit(0);
            }
            ib.mb_mapping_input_bits_end_address_ = jib["input_bits_end_address"];
            ib.eu_factor_ = jib["eu_factor"];
            ib.eu_quantity_ = jib["eu_quantity"];
            ib.eu_manifest_constant_ = jib["eu_manifest_constant"];

            ncap_obj->iot_mb_config_.mb_payload_mapping_.mb_mapping_input_bit_blocks_.try_emplace(ib.block_id_, std::move(ib));
          }
        }

        // IoT --> payload_mapping --> modbus_mapping --> Holding Register blocks.
        {
          nlohmann::json jhreg_blocks = jncap_obj["ncap"]["iot"]["payload_mapping"]["modbus_mapping"]["register_blocks"];

          for(nlohmann::json::iterator hrit = jhreg_blocks.begin(); hrit != jhreg_blocks.end(); ++hrit) {
            nlohmann::json jhr(*hrit);
            mb_holding_register_block_t hr;
            hr.block_id_ = jhr["block_id"];
            hr.mb_mapping_registers_start_address_ = jhr["registers_start_address"];
            hr.mb_mapping_nb_registers_ = jhr["nb_registers"];

            if(hr.mb_mapping_nb_registers_ >= VC_MODBUS_MAX_WRITE_REGISTERS) {
              SPDLOG_LOGGER_CRITICAL(logger, "Config Error. Exiting ... Number of holding registers are invalid - {}", hr.mb_mapping_nb_registers_);
              exit(0);
            }
            hr.mb_mapping_registers_end_address_ = jhr["registers_end_address"];
            hr.eu_factor_ = jhr["eu_factor"];
            hr.eu_quantity_ = jhr["eu_quantity"];
            hr.eu_manifest_constant_ = jhr["eu_manifest_constant"];

            ncap_obj->iot_mb_config_.mb_payload_mapping_.mb_mapping_holding_register_blocks_.try_emplace(hr.block_id_, std::move(hr));
          }
        }

        // IoT --> payload_mapping --> modbus_mapping --> Input Register blocks.
        {
          nlohmann::json jinreg_blocks = jncap_obj["ncap"]["iot"]["payload_mapping"]["modbus_mapping"]["input_register_blocks"];

          for(nlohmann::json::iterator irit = jinreg_blocks.begin(); irit != jinreg_blocks.end(); ++irit) {
            nlohmann::json jir(*irit);
            mb_input_register_block_t ir;
            ir.block_id_ = jir["block_id"];
            ir.mb_mapping_input_registers_start_address_ = jir["input_registers_start_address"];
            ir.mb_mapping_nb_input_registers_ = jir["nb_input_registers"];

            if(ir.mb_mapping_nb_input_registers_ >= VC_MODBUS_MAX_READ_REGISTERS) {
              SPDLOG_LOGGER_CRITICAL(logger, "Config Error. Exiting ... Number of input registers are invalid - {}", ir.mb_mapping_nb_input_registers_);
              exit(0);
            }

            ir.mb_mapping_input_registers_end_address_ = jir["input_registers_end_address"];
            ir.eu_factor_ = jir["eu_factor"];
            ir.eu_quantity_ = jir["eu_quantity"];
            ir.eu_manifest_constant_ = jir["eu_manifest_constant"];

            ncap_obj->iot_mb_config_.mb_payload_mapping_.mb_mapping_input_register_blocks_.try_emplace(ir.block_id_, std::move(ir));
          }
        }

        // Now it is time to add the NCAP Object to  the Registry.
        mmb_registry_.try_emplace(ncap_obj->base_config_.id_, std::move(ncap_obj));

      } // END OF 'if (ncap_type.compare("modbus_mqtt_expert_system") == 0)'
    } catch(nlohmann::json::exception& e) {
      std::stringstream ssReqException;
      ssReqException  << "message: " << e.what()    <<  " "
                      << "exception id: " << e.id   << std::endl;
      SPDLOG_LOGGER_CRITICAL(logger, "Config Error. Exiting ... Bad file. Check YAML entries...{}.", ssReqException.str());
    }
  }
  return true;
}
