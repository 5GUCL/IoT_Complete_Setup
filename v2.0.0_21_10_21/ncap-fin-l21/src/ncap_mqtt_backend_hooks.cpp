/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
 */
/*========================END LICENSE NOTICE========================*/

#include "ncap_app_constants.h"
#include "sf-sys/utils/time_utils.h"
#include "ncap_device_auth_token.h"
#include "ncap_mqtt_backend.h"

///////////////////////////////////////////////////////////
// THIS FILE / IMPLEMENTAION IS AN EXTENSION OF
// NCAP_MQTT_BACKEND MODULE.
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Data source subscriber(s) post messages to this execution environment.
// Pre-processing of the data is done here. For example, calling expert systems
// rule engines, harmonisation, data sampling are a few examples.
///////////////////////////////////////////////////////////

int ncap_mqtt_backend_t::ncap_mb_coil_measurement
(const std::string& v_device_id, int block_id, int start_address
  , int coil_number, const modbus_master_coil_bit_single_measurement_instance_t& measurement)
{
  // NCAP MQTT HOOK

  return 0;
}

int ncap_mqtt_backend_t::ncap_mb_input_bit_measurement
(const std::string& v_device_id, int block_id, int start_address
  , int input_bit_number, const modbus_master_input_bit_single_measurement_instance_t& measurement)
{
  // NCAP MQTT HOOK

  return 0;
}

int ncap_mqtt_backend_t::ncap_mb_holding_register_measurement
(const std::string& v_device_id, int block_id, int start_address
  , int holding_reg_number, const modbus_master_holding_register_single_measurement_instance_t& measurement)
{
  // NCAP MQTT HOOK
  nlohmann::json jorder_msg = {
      { "ncap_id", ncap_id_ },
      { "net_quantity", measurement.measured_value_ },
      { "time_instance", CTimeUtils::formatTime2Readable( measurement.time_instance_ ) },
      { "topic", "site/IoT/Order" },
      { "qos", 0 }
    };
  std::string str_order_msg = jorder_msg.dump();
  so_5::send<ncap_mqtt_client_env_mailbox_message_t>
    (mqtt_client_env_primary_channel_, std::move(str_order_msg));

  return 0;
}

int ncap_mqtt_backend_t::ncap_mb_input_register_measurement
(const std::string& v_device_id, int block_id, int start_address
  , int input_reg_number, const modbus_master_input_register_single_measurement_instance_t& measurement)
{
  // NCAP MQTT HOOK

  return 0;
}
