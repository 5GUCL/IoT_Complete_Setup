/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_MODBUS_PROTOCOL_H_
#define _CS_EDM_MODBUS_PROTOCOL_H_

/* Standard C++ includes */

#include <map>
#include <vector>
#include <so_5/all.hpp>
#include <modbus/modbus.h>
#include "sf-sys/si-units/si_unit_constants.h"

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 1 page 12)
 * Quantity of Coils to read (2 bytes): 1 to 2000 (0x7D0)
 * (chapter 6 section 11 page 29)
 * Quantity of Coils to write (2 bytes): 1 to 1968 (0x7B0)
 */
static const int VC_MODBUS_MAX_READ_BITS   = 256; // 2000;
static const int VC_MODBUS_MAX_WRITE_BITS  = 256; // 1968;

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 3 page 15)
 * Quantity of Registers to read (2 bytes): 1 to 125 (0x7D)
 * (chapter 6 section 12 page 31)
 * Quantity of Registers to write (2 bytes) 1 to 123 (0x7B)
 * (chapter 6 section 17 page 38)
 * Quantity of Registers to write in R/W registers (2 bytes) 1 to 121 (0x79)
 */
static const int VC_MODBUS_MAX_READ_REGISTERS     = 125;
static const int VC_MODBUS_MAX_WRITE_REGISTERS    = 123;
static const int VC_MODBUS_MAX_WR_WRITE_REGISTERS = 121;
static const int VC_MODBUS_MAX_WR_READ_REGISTERS  = 125;


static const int MAX_MODBUS_MASTER_DEVICES_GUI_SUPPORTED            = 32;
static const int MAX_MODBUS_ADDRESS_BLOCKS_GUI_SUPPORTED            = 32;
static const int MAX_MODBUS_MASTER_INPUT_REGISTERS_GUI_SUPPORTED    = 127;
static const int MAX_MODBUS_MASTER_HOLDING_REGISTERS_GUI_SUPPORTED  = 127;
static const int MAX_MODBUS_MASTER_COIL_BITS_GUI_SUPPORTED          = 256;
static const int MAX_MODBUS_MASTER_INPUT_BITS_GUI_SUPPORTED         = 256;

static const std::string MODBUS_MAPPING_NAME_COILS                  = "coils";                // FC 1
static const std::string MODBUS_MAPPING_NAME_INPUT_BITS             = "input_bits";           // FC 2
static const std::string MODBUS_MAPPING_NAME_HOLDING_REGISTERS      = "holding_registers";    // FC 3
static const std::string MODBUS_MAPPING_NAME_INPUT_REGISTERS        = "input_registers";      // FC 4

// Modbus Request / Response Message types.
static const std::string MODBUS_READ_BLOCK_COILS_REQUEST_M          = "read_block_coils_request";
static const std::string MODBUS_READ_BLOCK_COILS_RESPONSE_M         = "read_block_coils_response";

static const std::string MODBUS_READ_BLOCK_INPUT_BITS_REQUEST_M     = "read_block_input_bits_request";
static const std::string MODBUS_READ_BLOCK_INPUT_BITS_REPONSET_M    = "read_block_input_bits_response";

static const std::string MODBUS_READ_BLOCK_HOLDING_REG_REQUEST_M    = "read_block_holding_reg_request";
static const std::string MODBUS_READ_BLOCK_HOLDING_REG_RESPONSE_M   = "read_block_holding_reg_response";

static const std::string MODBUS_READ_BLOCK_INPUT_REG_REQUEST_M      = "read_block_input_reg_request";
static const std::string MODBUS_READ_BLOCK_INPUT_REG_RESPONSE_M     = "read_block_input_reg_response";


static const std::string MODBUS_WRITE_SINGLE_COIL_REQUEST_M         = "write_single_coil_request";
static const std::string MODBUS_WRITE_SINGLE_COIL_RESPONSE_M        = "write_single_coil_response";


enum mb_mapping_types_te {
  COILS = 1,
  INPUT_BITS = 2,
  HOLDING_REGISTERS = 3,
  INPUT_REGISTERS = 4
};

enum mb_last_nth_minute_data_consts_te {
  max_nth_minute_data_size = 300,
  max_nth_minute_min_chart_data_size = 30
};

struct mb_coil_block_t {
  int block_id_;
  int mb_mapping_bits_start_address_;
  int mb_mapping_nb_bits_;
  int mb_mapping_bits_end_address_;
  int mb_block_refresh_interval_;
  std::string eu_quantity_;
  std::string eu_factor_;
  int eu_manifest_constant_;
  so_5::timer_id_t mb_poll_timer_;
};

struct mb_input_bit_block_t {
  int block_id_;
  int mb_mapping_input_bits_start_address_;
  int mb_mapping_nb_input_bits_;
  int mb_mapping_input_bits_end_address_;
  int mb_block_refresh_interval_;
  std::string eu_quantity_;
  std::string eu_factor_;
  int eu_manifest_constant_;
  so_5::timer_id_t mb_poll_timer_;
};

struct mb_input_register_block_t {
  int block_id_;
  int mb_mapping_input_registers_start_address_;
  int mb_mapping_nb_input_registers_;
  int mb_mapping_input_registers_end_address_;
  int mb_block_refresh_interval_;
  std::string eu_quantity_;
  std::string eu_factor_;
  int eu_manifest_constant_;
  so_5::timer_id_t mb_poll_timer_;
};

struct mb_holding_register_block_t {
  int block_id_;
  int mb_mapping_registers_start_address_;
  int mb_mapping_nb_registers_;
  int mb_mapping_registers_end_address_;
  int mb_block_refresh_interval_;
  std::string eu_quantity_;
  std::string eu_factor_;
  int eu_manifest_constant_;
  so_5::timer_id_t mb_poll_timer_;
};

//
typedef int mb_coil_block_number_tt;
typedef int mb_input_bit_block_number_tt;
typedef int mb_input_register_block_number_tt;
typedef int mb_holding_register_block_number_tt;

//
typedef int mb_coil_number_tt;
typedef int mb_input_bit_number_tt;
typedef int mb_input_register_number_tt;
typedef int mb_holding_register_number_tt;

///////////////////////////////////////
// Modbus Functions
///////////////////////////////////////
// Function Code 0x05
// Write Single Coil
///////////////////////////////////////
struct mb_write_single_coil_t {
  int function_code_;
  int block_id_;
  int mb_mapping_coil_address_;
  int output_value_;  // Standard (0xFF00 = ON. 0x0000 = OFF) | libmodbus (TRUE or FALSE)
};


///////////////////////////////////////
struct modbus_master_coil_bit_single_measurement_instance_t {
  modbus_master_coil_bit_single_measurement_instance_t () {}
  ~modbus_master_coil_bit_single_measurement_instance_t () {}
  int             mb_block_id_;
  int             mb_mapping_bits_start_address_;
  int             mb_mapping_bit_number_;
  int             measured_value_;
  std::string     eu_quantity_;
  std::string     eu_factor_;
  int             eu_manifest_constant_;
  long long       time_instance_;
};

struct h5_modbus_master_coil_bit_single_measurement_instance_t {
  int             mb_block_id_;
  int             mb_mapping_bits_start_address_;
  int             mb_mapping_bit_number_;
  int             measured_value_;
  char            eu_factor_[EU_FACTOR_NAME_MAX_LEN];
  char            eu_quantity_[EU_QUANTITY_NAME_MAX_LEN];
  int             eu_manifest_constant_;
  long long       time_instance_;
};
static const int H5_MB_COIL_BIT_MEASUREMENT_INSTANCE_NUM_FIELDS  = 8;
/////////////////

struct modbus_master_input_bit_single_measurement_instance_t {
  modbus_master_input_bit_single_measurement_instance_t () {}
  ~modbus_master_input_bit_single_measurement_instance_t () {}
  int             mb_block_id_;
  int             mb_mapping_input_bits_start_address_;
  int             mb_mapping_input_bit_number_;
  int             measured_value_;
  std::string     eu_quantity_;
  std::string     eu_factor_;
  int             eu_manifest_constant_;
  long long       time_instance_;
};

struct h5_modbus_master_input_bit_single_measurement_instance_t {
  int             mb_block_id_;
  int             mb_mapping_input_bits_start_address_;
  int             mb_mapping_input_bit_number_;
  int             measured_value_;
  char            eu_factor_[EU_FACTOR_NAME_MAX_LEN];
  char            eu_quantity_[EU_QUANTITY_NAME_MAX_LEN];
  int             eu_manifest_constant_;
  long long       time_instance_;
};
static const int H5_MB_INPUT_BIT_MEASUREMENT_INSTANCE_NUM_FIELDS  = 8;
///////////////////

struct modbus_master_input_register_single_measurement_instance_t {
  modbus_master_input_register_single_measurement_instance_t () {}
  ~modbus_master_input_register_single_measurement_instance_t () {}
  int             mb_block_id_;
  int             mb_mapping_input_registers_start_address_;
  int             mb_mapping_input_register_number_;
  int             measured_value_;
  std::string     eu_quantity_;
  std::string     eu_factor_;
  int             eu_manifest_constant_;
  long long       time_instance_;
};

struct h5_modbus_master_input_register_single_measurement_instance_t {
  int             mb_block_id_;
  int             mb_mapping_input_registers_start_address_;
  int             mb_mapping_input_register_number_;
  int             measured_value_;
  char            eu_factor_[EU_FACTOR_NAME_MAX_LEN];
  char            eu_quantity_[EU_QUANTITY_NAME_MAX_LEN];
  int             eu_manifest_constant_;
  long long       time_instance_;
};
static const int H5_MB_INPUT_REGISTER_MEASUREMENT_INSTANCE_NUM_FIELDS  = 8;
///////////////////

struct modbus_master_holding_register_single_measurement_instance_t {
  modbus_master_holding_register_single_measurement_instance_t () {}
  ~modbus_master_holding_register_single_measurement_instance_t () {}
  int             mb_block_id_;
  int             mb_mapping_registers_start_address_;
  int             mb_mapping_holding_register_number_;
  int             measured_value_;
  std::string     eu_quantity_;
  std::string     eu_factor_;
  int             eu_manifest_constant_;
  long long       time_instance_;
};

struct h5_modbus_master_holding_register_single_measurement_instance_t {
  int             mb_block_id_;
  int             mb_mapping_registers_start_address_;
  int             mb_mapping_holding_register_number_;
  int             measured_value_;
  char            eu_factor_[EU_FACTOR_NAME_MAX_LEN];
  char            eu_quantity_[EU_QUANTITY_NAME_MAX_LEN];  
  int             eu_manifest_constant_;
  long long       time_instance_;
};
static const int H5_MB_HOLDING_REGISTER_MEASUREMENT_INSTANCE_NUM_FIELDS  = 8;
///////////////////

#endif /* _CS_EDM_MODBUS_PROTOCOL_H_ */
