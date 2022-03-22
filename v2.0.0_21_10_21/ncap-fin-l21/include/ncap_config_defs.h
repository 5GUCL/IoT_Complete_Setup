/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _PM_ODP_NCAP_CONFIG_DEFS_H_
#define _PM_ODP_NCAP_CONFIG_DEFS_H_

/* Standard C++ includes */
#include <string>
#include <vector>
#include <map>

// Modbus protocol and memory mapping data structures.
#include "ncap_modbus_mappings.h"

// Please refer to ncap_config.json.
// The following data structures represent the JSON objects of the ncap_config.json file.

// NCAP Base Object (e.g identification.)
struct ncap_base_t {
  ncap_base_t () {  }
  ncap_base_t ( const ncap_base_t& copy_obj ) {
    id_ = copy_obj.id_;
    type_ = copy_obj.type_;
    version_ = copy_obj.version_;
    name_ = copy_obj.name_;
    description_ = copy_obj.description_;
    api_server_address_ = copy_obj.api_server_address_;
    api_server_listen_port_ = copy_obj.api_server_listen_port_;
    api_server_threads_ = copy_obj.api_server_threads_;
  }
  ncap_base_t& operator=( const ncap_base_t& copy_obj ) {
    id_ = copy_obj.id_;
    type_ = copy_obj.type_;
    version_ = copy_obj.version_;
    name_ = copy_obj.name_;
    description_ = copy_obj.description_;
    api_server_address_ = copy_obj.api_server_address_;
    api_server_listen_port_ = copy_obj.api_server_listen_port_;
    api_server_threads_ = copy_obj.api_server_threads_;
    return *this;
  }

  std::string id_;
  std::string type_;
  std::string version_;
  std::string name_;
  std::string description_;
  std::string api_server_address_;
  long api_server_listen_port_;
  long api_server_threads_;
};

// Device Authentication related data structures
struct ncap_auth_t {
  ncap_auth_t () { }
  ncap_auth_t ( const ncap_auth_t& copy_obj ) {
    token_endpoint_ = copy_obj.token_endpoint_;
    grant_type_ = copy_obj.grant_type_;
    client_id_ = copy_obj.client_id_;
    client_secret_ = copy_obj.client_secret_;
    username_ = copy_obj.username_;
    password_ = copy_obj.password_;
    password_encryption_ = copy_obj.password_encryption_;
  }
  ncap_auth_t& operator=( const ncap_auth_t& copy_obj ) {
    token_endpoint_ = copy_obj.token_endpoint_;
    grant_type_ = copy_obj.grant_type_;
    client_id_ = copy_obj.client_id_;
    client_secret_ = copy_obj.client_secret_;
    username_ = copy_obj.username_;
    password_ = copy_obj.password_;
    password_encryption_ = copy_obj.password_encryption_;

    return *this;
  }

  std::string token_endpoint_;
  std::string grant_type_;
  std::string client_id_;
  std::string client_secret_;
  std::string username_;
  std::string password_;
  std::string password_encryption_;
};

// API Server.
struct ncap_app_api_server_t {
  ncap_app_api_server_t () { }
  ncap_app_api_server_t ( const ncap_app_api_server_t& copy_obj ) {
    base_url_ = copy_obj.base_url_;
    device_token_endpoint_ = copy_obj.device_token_endpoint_;
  }
  ncap_app_api_server_t& operator=( const ncap_app_api_server_t& copy_obj ) {
    base_url_ = copy_obj.base_url_;
    device_token_endpoint_ = copy_obj.device_token_endpoint_;

    return *this;
  }
  std::string base_url_;
  std::string device_token_endpoint_;
};

// MQTT related data structures
struct ncap_mqtt_broker_t {
  ncap_mqtt_broker_t () { }
  ncap_mqtt_broker_t ( const ncap_mqtt_broker_t& copy_obj ) {
    host_ = copy_obj.host_;
    port_ = copy_obj.port_;
    protocol_ = copy_obj.protocol_;
    ca_cert_ = copy_obj.ca_cert_;
    mqtt_client_id_ = copy_obj.mqtt_client_id_;
    secure_ = copy_obj.secure_;
    username_ = copy_obj.username_;
    password_ = copy_obj.password_;
  }
  ncap_mqtt_broker_t& operator=( const ncap_mqtt_broker_t& copy_obj ) {
    host_ = copy_obj.host_;
    port_ = copy_obj.port_;
    protocol_ = copy_obj.protocol_;
    ca_cert_ = copy_obj.ca_cert_;
    mqtt_client_id_ = copy_obj.mqtt_client_id_;
    secure_ = copy_obj.secure_;
    username_ = copy_obj.username_;
    password_ = copy_obj.password_;

    return *this;
  }
  std::string host_;
  long port_;
  std::string protocol_;
  std::string ca_cert_;
  std::string mqtt_client_id_;
  bool secure_;
  std::string username_;
  std::string password_;
};

struct ncap_mqtt_t {
  ncap_mqtt_t () { }
  ncap_mqtt_t ( const ncap_mqtt_t& copy_obj ) {
    mqtt_borker_ = copy_obj.mqtt_borker_;
  }
  ncap_mqtt_t& operator=( const ncap_mqtt_t& copy_obj ) {
    mqtt_borker_ = copy_obj.mqtt_borker_;

    return *this;
  }
  ncap_mqtt_broker_t mqtt_borker_;
};

// IoT related data structures
struct ncap_asset_t {
  ncap_asset_t () { }
  ncap_asset_t ( const ncap_asset_t& copy_obj ) {
    id_ = copy_obj.id_;
    name_ = copy_obj.name_;
    description_ = copy_obj.description_;
    meta_data_url_ = copy_obj.meta_data_url_;
  }
  ncap_asset_t& operator= ( const ncap_asset_t& copy_obj ) {
    id_ = copy_obj.id_;
    name_ = copy_obj.name_;
    description_ = copy_obj.description_;
    meta_data_url_ = copy_obj.meta_data_url_;

    return *this;
  }

  std::string id_;
  std::string name_;
  std::string description_;
  std::string meta_data_url_;
};

// Simple Univariate Rules.
struct simple_univariate_holding_register_rules_t {
  simple_univariate_holding_register_rules_t () { }
  simple_univariate_holding_register_rules_t ( const simple_univariate_holding_register_rules_t& copy_obj ) {
    id_ = copy_obj.id_;
    priority_ = copy_obj.priority_;
    block_id_ = copy_obj.block_id_;
    registers_start_address_ = copy_obj.registers_start_address_;
    holding_register_number_ = copy_obj.holding_register_number_;
    registers_end_address_ = copy_obj.registers_end_address_;
    upper_upper_limit_ = copy_obj.upper_upper_limit_;
    upper_lower_limit_ = copy_obj.upper_lower_limit_;
    lower_upper_limit_ = copy_obj.lower_upper_limit_;
    lower_lower_limit_ = copy_obj.lower_lower_limit_;
  }
  simple_univariate_holding_register_rules_t& operator= ( const simple_univariate_holding_register_rules_t& copy_obj ) {
    id_ = copy_obj.id_;
    priority_ = copy_obj.priority_;
    block_id_ = copy_obj.block_id_;
    registers_start_address_ = copy_obj.registers_start_address_;
    holding_register_number_ = copy_obj.holding_register_number_;
    registers_end_address_ = copy_obj.registers_end_address_;
    upper_upper_limit_ = copy_obj.upper_upper_limit_;
    upper_lower_limit_ = copy_obj.upper_lower_limit_;
    lower_upper_limit_ = copy_obj.lower_upper_limit_;
    lower_lower_limit_ = copy_obj.lower_lower_limit_;

    return *this;
  }

  std::string id_;
  long priority_;
  long block_id_;
  long registers_start_address_;
  long holding_register_number_;
  long registers_end_address_;
  int upper_upper_limit_;
  int upper_lower_limit_;
  int lower_upper_limit_;
  int lower_lower_limit_;
};

// Expert System (AI/ML) Rules
struct ncap_expert_system_t {
  ncap_expert_system_t () { }
  ncap_expert_system_t ( const ncap_expert_system_t& copy_obj ) {
    id_ = copy_obj.id_;
    priority_ = copy_obj.priority_;
    facts_db_ = copy_obj.facts_db_;
    inference_engine_ = copy_obj.inference_engine_;
    rules_ = copy_obj.rules_;
  }
  ncap_expert_system_t& operator= ( const ncap_expert_system_t& copy_obj ) {
    id_ = copy_obj.id_;
    priority_ = copy_obj.priority_;
    facts_db_ = copy_obj.facts_db_;
    inference_engine_ = copy_obj.inference_engine_;
    rules_ = copy_obj.rules_;

    return *this;
  }

  std::string id_;
  long priority_;
  std::string facts_db_;
  std::string inference_engine_;
  std::string rules_;
};

struct ncap_data_source_subscription_t {

  ncap_data_source_subscription_t () { }
  ncap_data_source_subscription_t ( const ncap_data_source_subscription_t& copy_obj ) {
    service_enabled_ = copy_obj.service_enabled_;
    host_ = copy_obj.host_;
    port_ = copy_obj.port_;
    protocol_ = copy_obj.protocol_; // WS, WSS, MQTT/WS, MQTT/WSS
    payload_mapping_ = copy_obj.payload_mapping_;
    username_ = copy_obj.username_;
    password_ = copy_obj.password_;
    ca_cert_ = copy_obj.ca_cert_;
  }

  ncap_data_source_subscription_t& operator= ( const ncap_data_source_subscription_t& copy_obj ) {
    service_enabled_ = copy_obj.service_enabled_;
    host_ = copy_obj.host_;
    port_ = copy_obj.port_;
    protocol_ = copy_obj.protocol_; // WS, WSS, MQTT/WS, MQTT/WSS
    payload_mapping_ = copy_obj.payload_mapping_;
    username_ = copy_obj.username_;
    password_ = copy_obj.password_;
    ca_cert_ = copy_obj.ca_cert_;

    return *this;
  }

  bool service_enabled_;
  std::string host_;
  long port_;
  std::string protocol_; // WS, WSS, MQTT/WS, MQTT/WSS
  std::string payload_mapping_;
  std::string username_;
  std::string password_;
  std::string ca_cert_;
};

struct ncap_data_source_controller_api_t {

  ncap_data_source_controller_api_t () { }
  ncap_data_source_controller_api_t ( const ncap_data_source_controller_api_t& copy_obj ) {
    service_enabled_ = copy_obj.service_enabled_;
    host_ = copy_obj.host_;
    port_ = copy_obj.port_;
    api_protocol_ = copy_obj.api_protocol_; // HTTP, HTTPS, WS, WSS, IEEE_1451
    payload_mapping_ = copy_obj.payload_mapping_;
    username_ = copy_obj.username_;
    password_ = copy_obj.password_;
    ca_cert_ = copy_obj.ca_cert_;
  }

  ncap_data_source_controller_api_t& operator= ( const ncap_data_source_controller_api_t& copy_obj ) {
    service_enabled_ = copy_obj.service_enabled_;
    host_ = copy_obj.host_;
    port_ = copy_obj.port_;
    api_protocol_ = copy_obj.api_protocol_; // HTTP, HTTPS, WS, WSS, IEEE_1451
    payload_mapping_ = copy_obj.payload_mapping_;
    username_ = copy_obj.username_;
    password_ = copy_obj.password_;
    ca_cert_ = copy_obj.ca_cert_;

    return *this;
  }

  bool service_enabled_;
  std::string host_;
  long port_;
  std::string api_protocol_; // HTTP, HTTPS, WS, WSS, IEEE_1451
  std::string payload_mapping_;
  std::string username_;
  std::string password_;
  std::string ca_cert_;
};

struct ncap_data_source_t {
  ncap_data_source_t () { }
  ncap_data_source_t ( const ncap_data_source_t& copy_obj ) {
    subscription_ = copy_obj.subscription_;
    controller_api_ = copy_obj.controller_api_;
  }
  ncap_data_source_t& operator= ( const ncap_data_source_t& copy_obj ) {
    subscription_ = copy_obj.subscription_;
    controller_api_ = copy_obj.controller_api_;

    return *this;
  }

  ncap_data_source_subscription_t subscription_;
  ncap_data_source_controller_api_t controller_api_;
};

// Data Source Payload related data structures.
struct ncap_modbus_payload_mapping_t {
  ncap_modbus_payload_mapping_t () { }

  // Controller Identifier
  std::string contoller_id_;

  // Agent Identifier
  std::string agent_id_;

  // Node Identifier
  std::string node_id_;

  std::map<mb_coil_block_number_tt, mb_coil_block_t> mb_mapping_coil_blocks_;
  std::map<mb_input_bit_block_number_tt, mb_input_bit_block_t> mb_mapping_input_bit_blocks_;
  std::map<mb_input_register_block_number_tt, mb_input_register_block_t> mb_mapping_input_register_blocks_;
  std::map<mb_holding_register_block_number_tt, mb_holding_register_block_t> mb_mapping_holding_register_blocks_;

};
// IoT Envelop that has all relevant data.
struct ncap_iot_modbus_t {
  ncap_iot_modbus_t () {}
  ncap_asset_t asset_;
  std::map<std::string, ncap_expert_system_t> expert_system_map_;
  ncap_data_source_t data_source_;
  ncap_modbus_payload_mapping_t mb_payload_mapping_;
};

// If another type of data payload mapping is configured, it may be added here.
// For example OPC-UA, CAN, IEEE-1451 etc
/*
  struct ncap_opcua_payload_mapping_t { ... };
  struct ncap_can_payload_mapping_t { ... };
  struct ncap_ieee_1451_payload_mapping_t { ... };
*/
/*
  struct ncap_iot_opcua_t {
    ncap_asset_t asset_;
    std::vector<ncap_expert_system_t> expert_system_vec_;
    ncap_data_source_t data_source_;
    ncap_opcua_payload_mapping_t opcua_payload_mapping_;
  };

  struct ncap_iot_can_t {
    ncap_asset_t asset_;
    std::vector<ncap_expert_system_t> expert_system_vec_;
    ncap_data_source_t data_source_;
    ncap_can_payload_mapping_t can_payload_mapping_;
  };

  struct ncap_iot_ieee_1451_t {
    ncap_asset_t asset_;
    std::vector<ncap_expert_system_t> expert_system_vec_;
    ncap_data_source_t data_source_;
    ncap_ieee_1451_payload_mapping_t ieee_1451_payload_mapping_;
  };

*/

struct ncap_mqtt_modbus_config_t {
  ncap_mqtt_modbus_config_t () { }
  ncap_base_t base_config_;
  ncap_auth_t auth_config_;
  ncap_app_api_server_t api_server_config_;
  ncap_mqtt_t mqtt_config_;
  ncap_iot_modbus_t iot_mb_config_;
};


#endif /* _PM_ODP_NCAP_CONFIG_DEFS_H_ */
