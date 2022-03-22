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
#include "h-data/hdf5/node/modbus/mb_node_master_hdf5_recorder.h"
#include "supervisor/supervisor.h"

void mb_node_master_dataset_ws_client_worker_t::work()
{
  std::stringstream ss;
  ss << "ws://" << mb_master_contoller_address_ << ":" << mb_master_contoller_port_;

  mb_node_master_dataset_ws_client_t ws_client(ss.str(), ds_rec_env_obj_, msg_primary_channel_, msg_secondary_channel_);

  // This call doesn't return.
  ws_client.run();
}

/////////////////////////////////////////////

mb_node_master_dataset_recorder_main_t::mb_node_master_dataset_recorder_main_t (std::shared_ptr<app_supervisor_t> that_supervisor)
{
  that_supervisor_  = that_supervisor;
  mb_node_registry_ = that_supervisor_->modbus_node_registry_;
  logger_registry_  = that_supervisor_->logger_registry_;
};

void mb_node_master_dataset_recorder_main_t::init_h5_service_loggers(const std::string& controller_logger_code, const std::string& logger_name)
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

int mb_node_master_dataset_recorder_main_t::backup_dataset_file_async() const
{
  // Get the instance of launcher logger.
  auto logger = spdlog::get(SMART_LOGGER_H5_LOGGER_NAME_);
  logger->info("Dataset Backup is initiated...");

  nlohmann::json jbkp_and_no_truncate = {};
  jbkp_and_no_truncate["message_type"]  = "backup_and_no_truncate";
  std::string str_jbkp_and_no_truncate = jbkp_and_no_truncate.dump();
  so_5::send<dataset_recorder_mailbox_message_t>(msg_primary_channel_, std::move(str_jbkp_and_no_truncate));

  return 0;
}


int mb_node_master_dataset_recorder_main_t::record_mb_coil_measurement
(const std::string& v_device_id, int block_id, int start_address
  , int coil_number, const h5_modbus_master_coil_bit_single_measurement_instance_t& measurement)
{
  // Get the instance of launcher logger.
  auto logger = spdlog::get(SMART_LOGGER_H5_LOGGER_NAME_);

  hid_t       file  = -1;
  hid_t       grp   = -1;
  hid_t       field_type[H5_MB_COIL_BIT_MEASUREMENT_INSTANCE_NUM_FIELDS];
  hid_t       eu_factor_string_type;
  hid_t       eu_quantity_string_type;
  hsize_t     nfields_out;
  hsize_t     nrecords_out;
  herr_t      status;
  hsize_t     chunk_size = 10;
  int         compress  = 0;

  // Create a local copy of the measurement.
  h5_modbus_master_coil_bit_single_measurement_instance_t fill_data[1] = { };
  fill_data[0].mb_block_id_ = measurement.mb_block_id_;
  fill_data[0].mb_mapping_bits_start_address_ = measurement.mb_mapping_bits_start_address_;
  fill_data[0].mb_mapping_bit_number_ = measurement.mb_mapping_bit_number_;
  fill_data[0].measured_value_ = measurement.measured_value_;
  strncpy (fill_data[0].eu_factor_, measurement.eu_factor_, EU_FACTOR_NAME_MAX_LEN);
  strncpy (fill_data[0].eu_quantity_, measurement.eu_quantity_, EU_FACTOR_NAME_MAX_LEN);
  fill_data[0].eu_manifest_constant_ = measurement.eu_manifest_constant_;
  fill_data[0].time_instance_ = measurement.time_instance_;

  size_t dst_sizes[H5_MB_COIL_BIT_MEASUREMENT_INSTANCE_NUM_FIELDS] = {
    sizeof(fill_data[0].mb_block_id_),
    sizeof(fill_data[0].mb_mapping_bits_start_address_),
    sizeof(fill_data[0].mb_mapping_bit_number_),
    sizeof(fill_data[0].measured_value_),
    sizeof(fill_data[0].eu_factor_),
    sizeof(fill_data[0].eu_quantity_),
    sizeof(fill_data[0].eu_manifest_constant_),
    sizeof(fill_data[0].time_instance_)
  };

  on_scope_exit close_hdf5_resources {
    [&] {
      H5Gclose(grp);
      H5Fclose(file);
    }
  };

  // Try open the file first
  file = H5Fopen(SMART_LOGGER_H5_DATASET_FILE_,H5F_ACC_RDWR, H5P_DEFAULT);

  if(file < 0) {
    // Someone deleted the dataset file and hence not found. Reinitialize the dataset file.
    init_h5_dataset_file();
    logger->error("Someone deleted the dataset file and hence not found. Reinitialized the dataset file.");

    // Try again to open the file.
    file = H5Fopen(SMART_LOGGER_H5_DATASET_FILE_,H5F_ACC_RDWR, H5P_DEFAULT);

    if(file < 0) {
      logger->error("Failed to open HDF5 file - {}. Check the file / direcory permissions.", SMART_LOGGER_H5_DATASET_FILE_);
      return -1;
    }
  }

  std::string coil_grp = prepare_mb_coil_and_input_bits_mapping_h5_group_id(v_device_id, MODBUS_MAPPING_NAME_COILS, block_id, start_address, coil_number);

  // Create a group in the file.
  grp = H5Gopen2(file, coil_grp.c_str(), H5P_DEFAULT);

  // Open failed!
  if(grp < 0) {
    logger->error("Failed to open {} group", coil_grp);
    return -1;
  }

  /* Get table info  */
  status = H5TBget_table_info(grp,HDF5_COILS_TABLE_NAME.c_str(), &nfields_out, &nrecords_out);

  /* Calculate the size and the offsets of our struct members in memory */
  size_t dst_size =  sizeof(h5_modbus_master_coil_bit_single_measurement_instance_t);
  size_t dst_offset[H5_MB_COIL_BIT_MEASUREMENT_INSTANCE_NUM_FIELDS] = {
    HOFFSET(h5_modbus_master_coil_bit_single_measurement_instance_t, mb_block_id_),
    HOFFSET(h5_modbus_master_coil_bit_single_measurement_instance_t, mb_mapping_bits_start_address_),
    HOFFSET(h5_modbus_master_coil_bit_single_measurement_instance_t, mb_mapping_bit_number_),
    HOFFSET(h5_modbus_master_coil_bit_single_measurement_instance_t, measured_value_),
    HOFFSET(h5_modbus_master_coil_bit_single_measurement_instance_t, eu_factor_),
    HOFFSET(h5_modbus_master_coil_bit_single_measurement_instance_t, eu_quantity_),
    HOFFSET(h5_modbus_master_coil_bit_single_measurement_instance_t, eu_manifest_constant_),
    HOFFSET(h5_modbus_master_coil_bit_single_measurement_instance_t, time_instance_)
  };

  if(status < 0) {
    /* Define field information */
    const char *field_names[H5_MB_COIL_BIT_MEASUREMENT_INSTANCE_NUM_FIELDS]  =
    { "Block Id","Start Address", "Coil Number", "Measurement", "SI Factor", "SI Quantity", "SI Manifest", "Time Instance" };

    /* Initialize the field field_type */
    eu_factor_string_type = H5Tcopy(H5T_C_S1);
    H5Tset_size(eu_factor_string_type, EU_FACTOR_NAME_MAX_LEN);
    
    eu_quantity_string_type = H5Tcopy(H5T_C_S1);
    H5Tset_size(eu_quantity_string_type, EU_QUANTITY_NAME_MAX_LEN);
    
    field_type[0] = H5T_NATIVE_INT;
    field_type[1] = H5T_NATIVE_INT;
    field_type[2] = H5T_NATIVE_INT;
    field_type[3] = H5T_NATIVE_INT;
    field_type[4] = eu_factor_string_type;
    field_type[5] = eu_quantity_string_type;
    field_type[6] = H5T_NATIVE_INT;
    field_type[7] = H5T_NATIVE_LLONG;

    // Create Table
    status = H5TBmake_table(HDF5_COILS_TABLE_NAME.c_str()   // Table tilte
        , grp                         // location
        , HDF5_COILS_TABLE_NAME.c_str() // Table name
        , (hsize_t) H5_MB_COIL_BIT_MEASUREMENT_INSTANCE_NUM_FIELDS
        , (hsize_t) H5_NUM_MEASUREMENTS_  // Add one record as it arrives.
        , dst_size, field_names, dst_offset
        , field_type, chunk_size, fill_data
        , compress, NULL);

    if(status < 0) {
      // Failed to create a table and fill the first measurement.
      logger->error("Failed to create dataset table for {}/{}.", coil_grp, HDF5_COILS_TABLE_NAME);
      return -1;
    }
  }

  // Now Append the table with the data.
  status = H5TBappend_records(grp
      , HDF5_COILS_TABLE_NAME.c_str()
      , (hsize_t) H5_NUM_MEASUREMENTS_
      , dst_size, dst_offset, dst_sizes, fill_data);

  if(status < 0) {
    // Log Here... Appending
    logger->error("Failed to append the measurement to the dataset table for {}/{}.", coil_grp, HDF5_COILS_TABLE_NAME);
    return -1;
  }

  return 0;
}

///////////
int mb_node_master_dataset_recorder_main_t::record_mb_input_bit_measurement
(const std::string& v_device_id, int block_id, int start_address
  , int input_bit_number, const h5_modbus_master_input_bit_single_measurement_instance_t& measurement)
{
  // Get the instance of launcher logger.
  auto logger = spdlog::get(SMART_LOGGER_H5_LOGGER_NAME_);

  hid_t       file  = -1;
  hid_t       grp   = -1;
  hid_t       field_type[H5_MB_INPUT_BIT_MEASUREMENT_INSTANCE_NUM_FIELDS];
  hid_t       eu_factor_string_type;
  hid_t       eu_quantity_string_type;
  hsize_t     nfields_out;
  hsize_t     nrecords_out;
  herr_t      status;
  hsize_t     chunk_size = 10;
  int         compress  = 0;

  // Create a local copy of the measurement.
  h5_modbus_master_input_bit_single_measurement_instance_t fill_data[1] = {};
  fill_data[0].mb_block_id_ = measurement.mb_block_id_;
  fill_data[0].mb_mapping_input_bits_start_address_ = measurement.mb_mapping_input_bits_start_address_;
  fill_data[0].mb_mapping_input_bit_number_ = measurement.mb_mapping_input_bit_number_;
  fill_data[0].measured_value_ = measurement.measured_value_;
  strncpy(fill_data[0].eu_factor_, measurement.eu_factor_, EU_FACTOR_NAME_MAX_LEN);
  strncpy(fill_data[0].eu_quantity_, measurement.eu_quantity_, EU_FACTOR_NAME_MAX_LEN);
  fill_data[0].eu_manifest_constant_ = measurement.eu_manifest_constant_;
  fill_data[0].time_instance_ = measurement.time_instance_;

  size_t dst_sizes[H5_MB_INPUT_BIT_MEASUREMENT_INSTANCE_NUM_FIELDS] = {
    sizeof(fill_data[0].mb_block_id_),
    sizeof(fill_data[0].mb_mapping_input_bits_start_address_),
    sizeof(fill_data[0].mb_mapping_input_bit_number_),
    sizeof(fill_data[0].measured_value_),
    sizeof(fill_data[0].eu_factor_),
    sizeof(fill_data[0].eu_quantity_),
    sizeof(fill_data[0].eu_manifest_constant_),
    sizeof(fill_data[0].time_instance_)
  };

  on_scope_exit close_hdf5_resources {
    [&] {
      H5Gclose(grp);
      H5Fclose(file);
    }
  };

  // Try open the file first
  file = H5Fopen(SMART_LOGGER_H5_DATASET_FILE_,H5F_ACC_RDWR, H5P_DEFAULT);

  if(file < 0) {
    // Someone deleted the dataset file and hence not found. Reinitialize the dataset file.
    init_h5_dataset_file();
    logger->error("Someone deleted the dataset file and hence not found. Reinitialized the dataset file.");

    // Try again to open the file.
    file = H5Fopen(SMART_LOGGER_H5_DATASET_FILE_,H5F_ACC_RDWR, H5P_DEFAULT);

    if(file < 0) {
      logger->error("Failed to open HDF5 file - {}. Check the file / direcory permissions.", SMART_LOGGER_H5_DATASET_FILE_);
      return -1;
    }
  }

  std::string input_bit_grp = prepare_mb_coil_and_input_bits_mapping_h5_group_id
    (v_device_id, MODBUS_MAPPING_NAME_INPUT_BITS, block_id, start_address, input_bit_number);

  // Create a group in the file.
  grp = H5Gopen2(file, input_bit_grp.c_str(), H5P_DEFAULT);

  // Open failed!
  if(grp < 0) {
    logger->error("Failed to open {} group ...", input_bit_grp);
    return -1;
  }

  /* Get table info  */
  status = H5TBget_table_info(grp, HDF5_INPUT_BITS_TABLE_NAME.c_str(), &nfields_out, &nrecords_out);

  /* Calculate the size and the offsets of our struct members in memory */
  size_t dst_size =  sizeof(h5_modbus_master_input_bit_single_measurement_instance_t);
  size_t dst_offset[H5_MB_INPUT_BIT_MEASUREMENT_INSTANCE_NUM_FIELDS] = {
    HOFFSET(h5_modbus_master_input_bit_single_measurement_instance_t, mb_block_id_),
    HOFFSET(h5_modbus_master_input_bit_single_measurement_instance_t, mb_mapping_input_bits_start_address_),
    HOFFSET(h5_modbus_master_input_bit_single_measurement_instance_t, mb_mapping_input_bit_number_),
    HOFFSET(h5_modbus_master_input_bit_single_measurement_instance_t, measured_value_),
    HOFFSET(h5_modbus_master_input_bit_single_measurement_instance_t, eu_factor_),
    HOFFSET(h5_modbus_master_input_bit_single_measurement_instance_t, eu_quantity_),
    HOFFSET(h5_modbus_master_input_bit_single_measurement_instance_t, eu_manifest_constant_),
    HOFFSET(h5_modbus_master_input_bit_single_measurement_instance_t, time_instance_)
  };

  if(status < 0) {
    /* Define field information */
    const char *field_names[H5_MB_INPUT_BIT_MEASUREMENT_INSTANCE_NUM_FIELDS]  =
    { "Block Id","Start Address", "Input Bit Number", "Measurement", "SI Factor", "SI Quantity", "SI Manifest", "Time Instance" };

    /* Initialize the field field_type */
    eu_factor_string_type = H5Tcopy(H5T_C_S1);
    H5Tset_size(eu_factor_string_type, EU_FACTOR_NAME_MAX_LEN);
    
    eu_quantity_string_type = H5Tcopy(H5T_C_S1);
    H5Tset_size(eu_quantity_string_type, EU_QUANTITY_NAME_MAX_LEN);
    
    field_type[0] = H5T_NATIVE_INT;
    field_type[1] = H5T_NATIVE_INT;
    field_type[2] = H5T_NATIVE_INT;
    field_type[3] = H5T_NATIVE_INT;
    field_type[4] = eu_factor_string_type;
    field_type[5] = eu_quantity_string_type;
    field_type[6] = H5T_NATIVE_INT;
    field_type[7] = H5T_NATIVE_LLONG;

    // Create Table
    status = H5TBmake_table(HDF5_INPUT_BITS_TABLE_NAME.c_str()      // Table tilte
        , grp                                 // location
        , HDF5_INPUT_BITS_TABLE_NAME.c_str()  // Table name
        , (hsize_t) H5_MB_INPUT_BIT_MEASUREMENT_INSTANCE_NUM_FIELDS
        , (hsize_t) H5_NUM_MEASUREMENTS_      // Add one record as it arrives.
        , dst_size, field_names, dst_offset
        , field_type, chunk_size, fill_data
        , compress, NULL);

    if(status < 0) {
      // Failed to create a table and fill the first measurement.
      logger->error("Faile to create dataset table for {}/{}.", input_bit_grp, HDF5_INPUT_BITS_TABLE_NAME);
      return -1;
    }
  }

  // Now Append the table with the data.
  status = H5TBappend_records(grp
      , HDF5_INPUT_BITS_TABLE_NAME.c_str()
      , (hsize_t) H5_NUM_MEASUREMENTS_
      , dst_size, dst_offset, dst_sizes, fill_data);

  if(status < 0) {
    logger->error("Faile to append dataset table for {}/{}.", input_bit_grp, HDF5_INPUT_BITS_TABLE_NAME);
    return -1;
  }

  return 0;
}

////////////////////
int mb_node_master_dataset_recorder_main_t::record_mb_holding_register_measurement
(const std::string& v_device_id, int block_id, int start_address
  , int holding_reg_number, const h5_modbus_master_holding_register_single_measurement_instance_t& measurement)
{
  // Get the instance of launcher logger.
  auto logger = spdlog::get(SMART_LOGGER_H5_LOGGER_NAME_);

  hid_t       file  = -1;
  hid_t       grp   = -1;
  hid_t       field_type[H5_MB_HOLDING_REGISTER_MEASUREMENT_INSTANCE_NUM_FIELDS];
  hid_t       eu_factor_string_type;
  hid_t       eu_quantity_string_type;
  hsize_t     nfields_out;
  hsize_t     nrecords_out;
  herr_t      status;
  hsize_t     chunk_size = 10;
  int         compress  = 0;

  // Create a local copy of the measurement.
  h5_modbus_master_holding_register_single_measurement_instance_t fill_data[1] = {};
  fill_data[0].mb_block_id_ = measurement.mb_block_id_;
  fill_data[0].mb_mapping_registers_start_address_ =  measurement.mb_mapping_registers_start_address_;
  fill_data[0].mb_mapping_holding_register_number_ =  measurement.mb_mapping_holding_register_number_;
  fill_data[0].measured_value_ = measurement.measured_value_;
  strncpy(fill_data[0].eu_factor_, measurement.eu_factor_, EU_FACTOR_NAME_MAX_LEN);
  strncpy(fill_data[0].eu_quantity_, measurement.eu_quantity_, EU_FACTOR_NAME_MAX_LEN);
  fill_data[0].eu_manifest_constant_ = measurement.eu_manifest_constant_;
  fill_data[0].time_instance_ = measurement.time_instance_;

  size_t dst_sizes[H5_MB_HOLDING_REGISTER_MEASUREMENT_INSTANCE_NUM_FIELDS] = {
    sizeof(fill_data[0].mb_block_id_),
    sizeof(fill_data[0].mb_mapping_registers_start_address_),
    sizeof(fill_data[0].mb_mapping_holding_register_number_),
    sizeof(fill_data[0].measured_value_),
    sizeof(fill_data[0].eu_factor_),
    sizeof(fill_data[0].eu_quantity_),
    sizeof(fill_data[0].eu_manifest_constant_),
    sizeof(fill_data[0].time_instance_)
  };

  on_scope_exit close_hdf5_resources {
    [&] {
      H5Gclose(grp);
      H5Fclose(file);
    }
  };

  // Try open the file first
  file = H5Fopen(SMART_LOGGER_H5_DATASET_FILE_,H5F_ACC_RDWR, H5P_DEFAULT);

  if(file < 0) {
    // Someone deleted the dataset file and hence not found. Reinitialize the dataset file.
    init_h5_dataset_file();
    logger->error("Someone deleted the dataset file and hence not found. Reinitialized the dataset file.");

    // Try again to open the file.
    file = H5Fopen(SMART_LOGGER_H5_DATASET_FILE_,H5F_ACC_RDWR, H5P_DEFAULT);

    if(file < 0) {
      logger->error("Failed to open HDF5 file - {}. Check the file / direcory permissions.", SMART_LOGGER_H5_DATASET_FILE_);
      return -1;
    }
  }

  std::string holding_reg_grp = prepare_mb_register_mapping_h5_group_id
    (v_device_id, MODBUS_MAPPING_NAME_HOLDING_REGISTERS, block_id, start_address, holding_reg_number);

  // Create a group in the file.
  grp = H5Gopen2(file, holding_reg_grp.c_str(), H5P_DEFAULT);

  // Open failed!
  if(grp < 0) {
    logger->error("Failed to open {} group ...", holding_reg_grp);
    return -1;
  }

  /* Get table info  */
  status = H5TBget_table_info(grp, HDF5_HOLDING_REGISTERS_TABLE_NAME.c_str(), &nfields_out, &nrecords_out);

  /* Calculate the size and the offsets of our struct members in memory */
  size_t dst_size =  sizeof(h5_modbus_master_holding_register_single_measurement_instance_t);
  size_t dst_offset[H5_MB_HOLDING_REGISTER_MEASUREMENT_INSTANCE_NUM_FIELDS] = {
    HOFFSET(h5_modbus_master_holding_register_single_measurement_instance_t, mb_block_id_),
    HOFFSET(h5_modbus_master_holding_register_single_measurement_instance_t, mb_mapping_registers_start_address_),
    HOFFSET(h5_modbus_master_holding_register_single_measurement_instance_t, mb_mapping_holding_register_number_),
    HOFFSET(h5_modbus_master_holding_register_single_measurement_instance_t, measured_value_),
    HOFFSET(h5_modbus_master_holding_register_single_measurement_instance_t, eu_factor_),
    HOFFSET(h5_modbus_master_holding_register_single_measurement_instance_t, eu_quantity_),
    HOFFSET(h5_modbus_master_holding_register_single_measurement_instance_t, eu_manifest_constant_),
    HOFFSET(h5_modbus_master_holding_register_single_measurement_instance_t, time_instance_)
  };

  if(status < 0) {
    /* Define field information */
    const char *field_names[H5_MB_HOLDING_REGISTER_MEASUREMENT_INSTANCE_NUM_FIELDS]  =
    { "Block Id","Start Address", "Holding Register Number", "Measurement", "SI Factor", "SI Quantity", "SI Manifest", "Time Instance" };

    /* Initialize the field field_type */
    eu_factor_string_type = H5Tcopy(H5T_C_S1);
    H5Tset_size(eu_factor_string_type, EU_FACTOR_NAME_MAX_LEN);
    
    eu_quantity_string_type = H5Tcopy(H5T_C_S1);
    H5Tset_size(eu_quantity_string_type, EU_QUANTITY_NAME_MAX_LEN);
    
    field_type[0] = H5T_NATIVE_INT;
    field_type[1] = H5T_NATIVE_INT;
    field_type[2] = H5T_NATIVE_INT;
    field_type[3] = H5T_NATIVE_INT;
    field_type[4] = eu_factor_string_type;
    field_type[5] = eu_quantity_string_type;
    field_type[6] = H5T_NATIVE_INT;
    field_type[7] = H5T_NATIVE_LLONG;

    // Create Table
    status = H5TBmake_table(HDF5_HOLDING_REGISTERS_TABLE_NAME.c_str()   // Table tilte
        , grp                         // location
        , HDF5_HOLDING_REGISTERS_TABLE_NAME.c_str() // Table name
        , (hsize_t) H5_MB_HOLDING_REGISTER_MEASUREMENT_INSTANCE_NUM_FIELDS
        , (hsize_t) H5_NUM_MEASUREMENTS_  // Add one record as it arrives.
        , dst_size, field_names, dst_offset
        , field_type, chunk_size, fill_data
        , compress, NULL);

    if(status < 0) {
      // Failed to create a table and fill the first measurement.
      logger->error("Faile to create dataset table for {}/{}.", holding_reg_grp, HDF5_HOLDING_REGISTERS_TABLE_NAME);
      return -1;
    }
  }

  // Now Append the table with the data.
  status = H5TBappend_records(grp
      , HDF5_HOLDING_REGISTERS_TABLE_NAME.c_str()
      , (hsize_t) H5_NUM_MEASUREMENTS_
      , dst_size, dst_offset, dst_sizes, fill_data);

  if(status < 0) {
    logger->error("Faile to append measurement to the dataset table for {}/{}.", holding_reg_grp, HDF5_HOLDING_REGISTERS_TABLE_NAME);
    return -1;
  }

  return 0;
}

///////////////////
int mb_node_master_dataset_recorder_main_t::record_mb_input_register_measurement
(const std::string& v_device_id, int block_id, int start_address
  , int input_reg_number, const h5_modbus_master_input_register_single_measurement_instance_t& measurement)
{
  // Get the instance of launcher logger.
  auto logger = spdlog::get(SMART_LOGGER_H5_LOGGER_NAME_);

  hid_t       file = -1;
  hid_t       grp = -1;
  hid_t       field_type[H5_MB_INPUT_REGISTER_MEASUREMENT_INSTANCE_NUM_FIELDS];
  hid_t       eu_factor_string_type;
  hid_t       eu_quantity_string_type;
  hsize_t     nfields_out;
  hsize_t     nrecords_out;
  herr_t      status;
  hsize_t     chunk_size = 10;
  int         compress  = 0;

  // Create a local copy of the measurement.
  h5_modbus_master_input_register_single_measurement_instance_t fill_data[1] = {};
  fill_data[0].mb_block_id_ = measurement.mb_block_id_;
  fill_data[0].mb_mapping_input_registers_start_address_ = measurement.mb_mapping_input_registers_start_address_;
  fill_data[0].mb_mapping_input_register_number_ = measurement.mb_mapping_input_register_number_;
  fill_data[0].measured_value_ = measurement.measured_value_;
  strncpy(fill_data[0].eu_factor_, measurement.eu_factor_, EU_FACTOR_NAME_MAX_LEN);
  strncpy(fill_data[0].eu_quantity_, measurement.eu_quantity_, EU_FACTOR_NAME_MAX_LEN);
  fill_data[0].eu_manifest_constant_ = measurement.eu_manifest_constant_;
  fill_data[0].time_instance_ = measurement.time_instance_;

  size_t dst_sizes[H5_MB_INPUT_REGISTER_MEASUREMENT_INSTANCE_NUM_FIELDS] = {
    sizeof(fill_data[0].mb_block_id_),
    sizeof(fill_data[0].mb_mapping_input_registers_start_address_),
    sizeof(fill_data[0].mb_mapping_input_register_number_),
    sizeof(fill_data[0].measured_value_),
    sizeof(fill_data[0].eu_factor_),
    sizeof(fill_data[0].eu_quantity_),
    sizeof(fill_data[0].eu_manifest_constant_),
    sizeof(fill_data[0].time_instance_)
  };

  on_scope_exit close_hdf5_resources {
    [&] {
      H5Gclose(grp);
      H5Fclose(file);
    }
  };

  // Try open the file first
  file = H5Fopen(SMART_LOGGER_H5_DATASET_FILE_,H5F_ACC_RDWR, H5P_DEFAULT);

  if(file < 0) {
    // Someone deleted the dataset file and hence not found. Reinitialize the dataset file.
    init_h5_dataset_file();
    logger->error("Someone deleted the dataset file and hence not found. Reinitialized the dataset file.");

    // Try again to open the file.
    file = H5Fopen(SMART_LOGGER_H5_DATASET_FILE_,H5F_ACC_RDWR, H5P_DEFAULT);

    if(file < 0) {
      logger->error("Failed to open HDF5 file - {}. Check the file / direcory permissions.", SMART_LOGGER_H5_DATASET_FILE_);
      return -1;
    }
  }

  std::string input_reg_grp = prepare_mb_register_mapping_h5_group_id
    (v_device_id, MODBUS_MAPPING_NAME_INPUT_REGISTERS, block_id, start_address, input_reg_number);

  // Create a group in the file.
  grp = H5Gopen2(file, input_reg_grp.c_str(), H5P_DEFAULT);

  if(grp < 0) {
    logger->error("Failed to open {} group ...", input_reg_grp);
    return -1;
  }

  /* Get table info  */
  status = H5TBget_table_info(grp, HDF5_INPUT_REGISTERS_TABLE_NAME.c_str(), &nfields_out, &nrecords_out);

  /* Calculate the size and the offsets of our struct members in memory */
  size_t dst_size =  sizeof(h5_modbus_master_input_register_single_measurement_instance_t);
  size_t dst_offset[H5_MB_INPUT_REGISTER_MEASUREMENT_INSTANCE_NUM_FIELDS] = {
    HOFFSET(h5_modbus_master_input_register_single_measurement_instance_t, mb_block_id_),
    HOFFSET(h5_modbus_master_input_register_single_measurement_instance_t, mb_mapping_input_registers_start_address_),
    HOFFSET(h5_modbus_master_input_register_single_measurement_instance_t, mb_mapping_input_register_number_),
    HOFFSET(h5_modbus_master_input_register_single_measurement_instance_t, measured_value_),
    HOFFSET(h5_modbus_master_input_register_single_measurement_instance_t, eu_factor_),
    HOFFSET(h5_modbus_master_input_register_single_measurement_instance_t, eu_quantity_),
    HOFFSET(h5_modbus_master_input_register_single_measurement_instance_t, eu_manifest_constant_),
    HOFFSET(h5_modbus_master_input_register_single_measurement_instance_t, time_instance_)
  };

  if(status < 0) {
    /* Define field information */
    const char *field_names[H5_MB_INPUT_REGISTER_MEASUREMENT_INSTANCE_NUM_FIELDS]  =
    { "Block Id","Start Address", "Input Register Number", "Measurement", "SI Factor", "SI Quantity", "SI Manifest", "Time Instance" };

    /* Initialize the field field_type */
    eu_factor_string_type = H5Tcopy(H5T_C_S1);
    H5Tset_size(eu_factor_string_type, EU_FACTOR_NAME_MAX_LEN);
    
    eu_quantity_string_type = H5Tcopy(H5T_C_S1);
    H5Tset_size(eu_quantity_string_type, EU_QUANTITY_NAME_MAX_LEN);
    
    field_type[0] = H5T_NATIVE_INT;
    field_type[1] = H5T_NATIVE_INT;
    field_type[2] = H5T_NATIVE_INT;
    field_type[3] = H5T_NATIVE_INT;
    field_type[4] = eu_factor_string_type;
    field_type[5] = eu_quantity_string_type;
    field_type[6] = H5T_NATIVE_INT;
    field_type[7] = H5T_NATIVE_LLONG;

    // Create Table
    status = H5TBmake_table(HDF5_INPUT_REGISTERS_TABLE_NAME.c_str()     // Table tilte
        , grp                                     // location
        , HDF5_INPUT_REGISTERS_TABLE_NAME.c_str() // Table name
        , (hsize_t) H5_MB_INPUT_REGISTER_MEASUREMENT_INSTANCE_NUM_FIELDS
        , (hsize_t) H5_NUM_MEASUREMENTS_          // Add one record as it arrives.
        , dst_size, field_names, dst_offset
        , field_type, chunk_size, fill_data
        , compress, NULL);

    if(status < 0) {
      // Failed to create a table and fill the first measurement.
      logger->error("Faile to create dataset table for {}/{}.", input_reg_grp, HDF5_INPUT_REGISTERS_TABLE_NAME);
      return -1;
    }
  }

  // Now Append the table with the data.
  status = H5TBappend_records(grp
      , HDF5_INPUT_REGISTERS_TABLE_NAME.c_str()
      , (hsize_t) H5_NUM_MEASUREMENTS_
      , dst_size, dst_offset, dst_sizes, fill_data);

  if(status < 0) {
    logger->error("Faile to append the measurement to the dataset table for {}/{}.", input_reg_grp, HDF5_INPUT_REGISTERS_TABLE_NAME);
    return -1;
  }

  return 0;
}
//////////////////////////

////////////////////////////
// HDF5 Helper functions
////////////////////////////
bool mb_node_master_dataset_recorder_main_t::prepare_root_group(const char *root_group)
{
  // Get the instance of launcher logger.
  auto logger = spdlog::get(SMART_LOGGER_H5_LOGGER_NAME_);

  hid_t       file = -1;
  hid_t       root_grp_handle = -1;

  on_scope_exit close_hdf5_resources {
    [&] {
      H5Gclose(root_grp_handle);
      H5Fclose(file);
    }
  };

  // Try open the file first
  file = H5Fopen(SMART_LOGGER_H5_DATASET_FILE_,H5F_ACC_RDWR, H5P_DEFAULT);

  if(file < 0) {
    // Create / Open a file.
    file = H5Fcreate(SMART_LOGGER_H5_DATASET_FILE_, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    if(file < 0) {
      // Log Here
      logger->error("#prepare_root_group Failed to HDF5 open file - {}.", SMART_LOGGER_H5_DATASET_FILE_);
      return false;
    }
  }

  // Try Opening the root group in the file.
  root_grp_handle = H5Gopen2(file, root_group, H5P_DEFAULT);

  if(root_grp_handle < 0) {
    root_grp_handle = H5Gcreate2(file, root_group, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // Open / Create failed!
    if(root_grp_handle < 0) {
      logger->error("#prepare_root_group Failed to open / create Root {} group ...", root_group);
      return false;
    }
  }

  // Log here
  logger->info("#prepare_root_group opened root {} group ...", root_group);

  return true;
}

bool mb_node_master_dataset_recorder_main_t::prepare_child_group(const char *this_group, const char *new_group)
{
  // Get the instance of launcher logger.
  auto logger = spdlog::get(SMART_LOGGER_H5_LOGGER_NAME_);

  hid_t       file = -1;
  hid_t       this_grp_handle = -1;
  hid_t       new_grp_handle  = -1;

  on_scope_exit close_hdf5_resources {
    [&] {
      H5Gclose(new_grp_handle);
      H5Gclose(this_grp_handle);
      H5Fclose(file);
    }
  };

  // Try open the file first
  file = H5Fopen(SMART_LOGGER_H5_DATASET_FILE_,H5F_ACC_RDWR, H5P_DEFAULT);

  if(file < 0) {
    // Create / Open a file.
    file = H5Fcreate(SMART_LOGGER_H5_DATASET_FILE_, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    if(file < 0) {
      logger->error("#prepare_child_group Failed to HDF5 open file - {}.", SMART_LOGGER_H5_DATASET_FILE_);
      return false;
    }
  }

  // Open the source / this group in the file.
  this_grp_handle = H5Gopen2(file, this_group, H5P_DEFAULT);

  if(this_grp_handle < 0) {
    // We could not find the source group.
    logger->error("#prepare_child_group source location/group {} not found.", this_group);
    return false;
  }

  // Try opening the target group.
  new_grp_handle = H5Gopen2(this_grp_handle, new_group, H5P_DEFAULT);

  if(new_grp_handle < 0) {
    new_grp_handle = H5Gcreate2(this_grp_handle, new_group, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // Open / Create failed!
    if(new_grp_handle < 0) {
      logger->error("#prepare_child_group failed to create location/group {}/{}.", this_group, new_group);
      return false;
    }
  }

  logger->info("#prepare_child_group created/opened location/group {}/{}.", this_group, new_group);

  return true;
}


///////////////////////////////////////////////////////////
void mb_node_master_dataset_recorder_main_t::message_recording_fn()
{

  // Get the instance of launcher logger.
  auto logger = spdlog::get(SMART_LOGGER_H5_LOGGER_NAME_);

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
  , [this, &logger](dataset_recorder_mailbox_message_t mb_msg) {
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
        h5_modbus_master_coil_bit_single_measurement_instance_t cm;
        cm.mb_block_id_ = jmsg["block_id"];
        cm.mb_mapping_bits_start_address_ = jmsg["coil_bits_start_address"];
        cm.mb_mapping_bit_number_ = jmsg["coil_bit_number"];

        // Refer to Modbus Standard of bits, registers indexing.
        cm.mb_mapping_bit_number_ = cm.mb_mapping_bit_number_ + 1;

        cm.measured_value_ = jmsg["measured_value"];
        cm.time_instance_  = jmsg["time_instance"];
        std::string eu_factor = jmsg["eu_factor"];
        std::string eu_quantity = jmsg["eu_quantity"];
        cm.eu_manifest_constant_ = jmsg["eu_manifest_constant"];
        
        memset(cm.eu_factor_, 0, EU_FACTOR_NAME_MAX_LEN);
        memset(cm.eu_quantity_, 0, EU_FACTOR_NAME_MAX_LEN);
        
        strncpy(cm.eu_factor_, eu_factor.c_str(), EU_FACTOR_NAME_MAX_LEN-sizeof(char)); // MAX - 1
        strncpy(cm.eu_quantity_, eu_quantity.c_str(), EU_FACTOR_NAME_MAX_LEN-sizeof(char)); // MAX - 1

        // Save the data into HDF5 file
        record_mb_coil_measurement(v_id, cm.mb_block_id_, cm.mb_mapping_bits_start_address_, cm.mb_mapping_bit_number_, cm);

      }
      ///////////////////////////////////////
      else if(_msg_type.compare("input_bit_reading") == 0) {

        std::string _contoller_id = jmsg["controller_id"];
        std::string _agent_id     = jmsg["agent_id"];
        std::string _node_id      = jmsg["node_id"];

        std::string v_id;
        v_id = prepare_device_id(_contoller_id, _agent_id, _node_id);

        // We found a device that the incoming message belongs to.
        h5_modbus_master_input_bit_single_measurement_instance_t ibm;
        ibm.mb_block_id_ = jmsg["block_id"];
        ibm.mb_mapping_input_bits_start_address_ = jmsg["input_bits_start_address"];
        ibm.mb_mapping_input_bit_number_ = jmsg["input_bit_number"];

        // Refer to Modbus Standard of bits, registers indexing.
        ibm.mb_mapping_input_bit_number_ = ibm.mb_mapping_input_bit_number_ + 1;

        ibm.measured_value_ = jmsg["measured_value"];
        ibm.time_instance_  = jmsg["time_instance"];

        std::string eu_factor = jmsg["eu_factor"];
        std::string eu_quantity = jmsg["eu_quantity"];
        ibm.eu_manifest_constant_ = jmsg["eu_manifest_constant"];
      
        memset(ibm.eu_factor_, 0, EU_FACTOR_NAME_MAX_LEN);
        memset(ibm.eu_quantity_, 0, EU_FACTOR_NAME_MAX_LEN);
        
        strncpy(ibm.eu_factor_, eu_factor.c_str(), EU_FACTOR_NAME_MAX_LEN-sizeof(char)); // MAX - 1
        strncpy(ibm.eu_quantity_, eu_quantity.c_str(), EU_FACTOR_NAME_MAX_LEN-sizeof(char)); // MAX - 1

        // Save the data into HDF5 file
        record_mb_input_bit_measurement(v_id, ibm.mb_block_id_, ibm.mb_mapping_input_bits_start_address_, ibm.mb_mapping_input_bit_number_, ibm);

      }
      ////////////////////////////////////////
      else if(_msg_type.compare("holding_register_reading") == 0) {

        std::string _contoller_id = jmsg["controller_id"];
        std::string _agent_id     = jmsg["agent_id"];
        std::string _node_id      = jmsg["node_id"];

        std::string v_id;
        v_id = prepare_device_id(_contoller_id, _agent_id, _node_id);

        // We found a device that the incoming message belongs to.
        h5_modbus_master_holding_register_single_measurement_instance_t hm;
        hm.mb_block_id_ = jmsg["block_id"];
        hm.mb_mapping_registers_start_address_ = jmsg["registers_start_address"];
        hm.mb_mapping_holding_register_number_ = jmsg["holding_register_number"];
        // Refer to Modbus Standard of bits, registers indexing.
        hm.mb_mapping_holding_register_number_ = hm.mb_mapping_holding_register_number_ + 1;

        hm.measured_value_ = jmsg["measured_value"];
        hm.time_instance_  = jmsg["time_instance"];
        
        std::string eu_factor = jmsg["eu_factor"];
        std::string eu_quantity = jmsg["eu_quantity"];
        hm.eu_manifest_constant_ = jmsg["eu_manifest_constant"];
      
        memset(hm.eu_factor_, 0, EU_FACTOR_NAME_MAX_LEN);
        memset(hm.eu_quantity_, 0, EU_FACTOR_NAME_MAX_LEN);
        
        strncpy(hm.eu_factor_, eu_factor.c_str(), EU_FACTOR_NAME_MAX_LEN-sizeof(char)); // MAX - 1
        strncpy(hm.eu_quantity_, eu_quantity.c_str(), EU_FACTOR_NAME_MAX_LEN-sizeof(char)); // MAX - 1

        // Save the data into HDF5 file
        record_mb_holding_register_measurement(v_id, hm.mb_block_id_, hm.mb_mapping_registers_start_address_, hm.mb_mapping_holding_register_number_, hm);
      }
      ////////////////////////////////////////
      else if(_msg_type.compare("input_register_reading") == 0) {

        std::string _contoller_id = jmsg["controller_id"];
        std::string _agent_id     = jmsg["agent_id"];
        std::string _node_id      = jmsg["node_id"];

        std::string v_id;
        v_id = prepare_device_id(_contoller_id, _agent_id, _node_id);

        // We found a device that the incoming message belongs to.
        h5_modbus_master_input_register_single_measurement_instance_t im;
        im.mb_block_id_ = jmsg["block_id"];
        im.mb_mapping_input_registers_start_address_ = jmsg["input_registers_start_address"];
        im.mb_mapping_input_register_number_ = jmsg["input_register_number"];
        // Refer to Modbus Standard of bits, registers indexing.
        im.mb_mapping_input_register_number_ = im.mb_mapping_input_register_number_ + 1;
        im.measured_value_ = jmsg["measured_value"];
        im.time_instance_  = jmsg["time_instance"];

        std::string eu_factor = jmsg["eu_factor"];
        std::string eu_quantity = jmsg["eu_quantity"];
        im.eu_manifest_constant_ = jmsg["eu_manifest_constant"];
      
        memset(im.eu_factor_, 0, EU_FACTOR_NAME_MAX_LEN);
        memset(im.eu_quantity_, 0, EU_FACTOR_NAME_MAX_LEN);
        
        strncpy(im.eu_factor_, eu_factor.c_str(), EU_FACTOR_NAME_MAX_LEN-sizeof(char)); // MAX - 1
        strncpy(im.eu_quantity_, eu_quantity.c_str(), EU_FACTOR_NAME_MAX_LEN-sizeof(char)); // MAX - 1

        // Save the data into HDF5 file
        record_mb_input_register_measurement(v_id, im.mb_block_id_, im.mb_mapping_input_registers_start_address_, im.mb_mapping_input_register_number_, im);
      }
      ////////////////////////////////////////
      else if(_msg_type.compare("backup_and_no_truncate") == 0) {

        backup_and_no_truncate_fn();

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
  , [&logger](dataset_recorder_mailbox_message_t mb_msg) {
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

int mb_node_master_dataset_recorder_main_t::backup_and_no_truncate_fn()
{
  CTimeUtils ct;
  std::string new_back_file_name = ct.readable_ts_now();

  std::stringstream bkp_cmd;
  bkp_cmd << "cp "  << SMART_LOGGER_H5_DATASET_FILE_
    << "   "  << SMART_LOGGER_H5_DATASET_FILE_DIR_ << SMART_LOGGER_H5_BACKUP_DATASET_FILE_ << new_back_file_name << ".hdf5";

  return std::system(bkp_cmd.str().c_str());

}

int mb_node_master_dataset_recorder_main_t::init_h5_dataset_file()
{
  // Get the instance of launcher logger.
  auto logger = spdlog::get(SMART_LOGGER_H5_LOGGER_NAME_);

  for(auto strnode : mb_node_registry_->node_registry_) {
    try {
      nlohmann::json jnode = nlohmann::json::parse(strnode);
      std::string controller = jnode["controller"]["type"];

      // Controller - MODBUS-MASTER
      if(controller.compare("modbus-master") == 0) {
        mb_node_master_dataset_ws_client_worker_t s1;
        s1.mb_master_contoller_address_ = jnode["controller"]["attributes"]["api_server_listen_address"];
        s1.mb_master_contoller_port_    = jnode["controller"]["attributes"]["api_server_listen_port"];
        s1.mb_controller_id_            = jnode["controller"]["id"];
        s1.ds_rec_env_obj_              = ds_rec_env_obj_;
        s1.msg_primary_channel_         = msg_primary_channel_;
        s1.msg_secondary_channel_       = msg_secondary_channel_;
        dataset_ws_client_workers_vec_.emplace_back(std::move(s1));

        // Now prepare HDF5 resources.
        // Firstly prepre root group for each controller.
        std::string ctrl_id = jnode["controller"]["id"];
        std::stringstream ctrl_ss;
        ctrl_ss << "/" << ctrl_id;

        prepare_root_group(ctrl_ss.str().c_str());

        // Iterate through Agents and create sub groups.
        nlohmann::json jagents = jnode["controller"]["agents"];

        for(nlohmann::json::iterator ait = jagents.begin(); ait != jagents.end(); ++ait) {
          nlohmann::json jnode(*ait);
          std::string agent_id = jnode["agent"]["id"];

          prepare_child_group(ctrl_ss.str().c_str(), agent_id.c_str());

          // Create Node subgroup. Each Agent will have only one Node.
          std::string node_id = jnode["agent"]["node"]["id"];
          {
            std::stringstream agent_ss;
            agent_ss << ctrl_ss.str() << "/" << agent_id;
            prepare_child_group(agent_ss.str().c_str(), node_id.c_str());
          }

          // Create groups for each modbus mapping types.
          std::stringstream mb_mapping_ss;
          mb_mapping_ss << ctrl_ss.str() << "/" << agent_id << "/" << node_id;
          prepare_child_group(mb_mapping_ss.str().c_str(), MODBUS_MAPPING_NAME_COILS.c_str());
          prepare_child_group(mb_mapping_ss.str().c_str(), MODBUS_MAPPING_NAME_INPUT_BITS.c_str());
          prepare_child_group(mb_mapping_ss.str().c_str(), MODBUS_MAPPING_NAME_HOLDING_REGISTERS.c_str());
          prepare_child_group(mb_mapping_ss.str().c_str(), MODBUS_MAPPING_NAME_INPUT_REGISTERS.c_str());

          // For each mapping, create a block group.
          // Read the coil blocks.
          {
            nlohmann::json jcoil_blocks = jnode["agent"]["node"]["modbus_master"]["slave_device"]["modbus_mapping"]["coil_blocks"];

            for(nlohmann::json::iterator coilit = jcoil_blocks.begin(); coilit != jcoil_blocks.end(); ++coilit) {
              nlohmann::json jcb(*coilit);
              mb_coil_block_t cb;
              cb.block_id_ = jcb["block_id"];
              cb.mb_mapping_bits_start_address_ = jcb["bits_start_address"];
              cb.mb_mapping_nb_bits_ = jcb["nb_bits"];
              cb.mb_mapping_bits_end_address_ = jcb["bits_end_address"];

              std::stringstream mb_coils_mapping_ss;
              mb_coils_mapping_ss << mb_mapping_ss.str() << "/" << MODBUS_MAPPING_NAME_COILS.c_str();

              std::stringstream mb_coils_mapping_block_ss;
              mb_coils_mapping_block_ss << "block#" << cb.block_id_;
              prepare_child_group(mb_coils_mapping_ss.str().c_str(), mb_coils_mapping_block_ss.str().c_str());

              // Now prepare a group for the starting address of the block
              std::stringstream block_grp_ss;
              block_grp_ss << mb_coils_mapping_ss.str() << "/" << mb_coils_mapping_block_ss.str();

              std::stringstream mb_coils_mapping_block_starting_address_ss;
              mb_coils_mapping_block_starting_address_ss << "address#"<< cb.mb_mapping_bits_start_address_;

              prepare_child_group(block_grp_ss.str().c_str(), mb_coils_mapping_block_starting_address_ss.str().c_str());

              std::stringstream block_starting_address_grp_ss;
              block_starting_address_grp_ss << block_grp_ss.str() << "/" << mb_coils_mapping_block_starting_address_ss.str() ;

              // Now create / prepare a group for each channel (coil / input bit / holding register / input register).
              // Please note / refer to the modbus mapping... we have to create a full mapping 0 ... max coils etc.
              for(int coil_n = 0;  coil_n <= cb.mb_mapping_nb_bits_ ; coil_n++) {
                std::stringstream coil_n_ss;
                coil_n_ss << coil_n;
                prepare_child_group(block_starting_address_grp_ss.str().c_str(), coil_n_ss.str().c_str());
              }
            }
          }
          //////////////////////
          // INPUT BITS ...
          {
            nlohmann::json jibit_blocks = jnode["agent"]["node"]["modbus_master"]["slave_device"]["modbus_mapping"]["input_bits_blocks"];

            for(nlohmann::json::iterator ibit = jibit_blocks.begin(); ibit != jibit_blocks.end(); ++ibit) {
              nlohmann::json jib(*ibit);
              mb_input_bit_block_t ib;
              ib.block_id_ = jib["block_id"];
              ib.mb_mapping_input_bits_start_address_ = jib["input_bits_start_address"];
              ib.mb_mapping_nb_input_bits_ = jib["nb_input_bits"];
              ib.mb_mapping_input_bits_end_address_ = jib["input_bits_end_address"];

              std::stringstream mb_ib_mapping_ss;
              mb_ib_mapping_ss << mb_mapping_ss.str() << "/" << MODBUS_MAPPING_NAME_INPUT_BITS.c_str();

              std::stringstream mb_ib_mapping_block_ss;
              mb_ib_mapping_block_ss << "block#"<< ib.block_id_;

              prepare_child_group(mb_ib_mapping_ss.str().c_str(), mb_ib_mapping_block_ss.str().c_str());

              // Now prepare a group for the starting address of the block
              std::stringstream block_grp_ss;
              block_grp_ss << mb_ib_mapping_ss.str() << "/" << mb_ib_mapping_block_ss.str();

              std::stringstream mb_ib_mapping_block_starting_address_ss;
              mb_ib_mapping_block_starting_address_ss << "address#"<< ib.mb_mapping_input_bits_start_address_;

              prepare_child_group(block_grp_ss.str().c_str(), mb_ib_mapping_block_starting_address_ss.str().c_str());

              std::stringstream block_starting_address_grp_ss;
              block_starting_address_grp_ss << block_grp_ss.str() << "/" << mb_ib_mapping_block_starting_address_ss.str() ;

              // Now create / prepare a group for each channel (coil / input bit / holding register / input register).
              // Please note / refer to the modbus mapping... we have to create a full mapping 0 ... max input bits etc.
              for(int ib_n = 0;  ib_n <= ib.mb_mapping_nb_input_bits_ ; ib_n++) {
                std::stringstream ib_n_ss;
                ib_n_ss << ib_n;
                prepare_child_group(block_starting_address_grp_ss.str().c_str(), ib_n_ss.str().c_str());
              }
            }
          }
          //////////////////////
          // HOLDING REGISTERS ...
          // Read the Holding Register blocks.
          {
            nlohmann::json jhreg_blocks = jnode["agent"]["node"]["modbus_master"]["slave_device"]["modbus_mapping"]["register_blocks"];

            for(nlohmann::json::iterator hrit = jhreg_blocks.begin(); hrit != jhreg_blocks.end(); ++hrit) {
              nlohmann::json jhr(*hrit);
              mb_holding_register_block_t hr;
              hr.block_id_ = jhr["block_id"];
              hr.mb_mapping_registers_start_address_ = jhr["registers_start_address"];
              hr.mb_mapping_nb_registers_ = jhr["nb_registers"];
              hr.mb_mapping_registers_end_address_ = jhr["registers_end_address"];

              std::stringstream mb_hr_mapping_ss;
              mb_hr_mapping_ss << mb_mapping_ss.str() << "/" << MODBUS_MAPPING_NAME_HOLDING_REGISTERS.c_str();

              std::stringstream mb_hr_mapping_block_ss;
              mb_hr_mapping_block_ss << "block#"<< hr.block_id_;

              prepare_child_group(mb_hr_mapping_ss.str().c_str(), mb_hr_mapping_block_ss.str().c_str());

              // Now prepare a group for the starting address of the block
              std::stringstream block_grp_ss;
              block_grp_ss << mb_hr_mapping_ss.str() << "/" << mb_hr_mapping_block_ss.str();

              std::stringstream mb_hr_mapping_block_starting_address_ss;
              mb_hr_mapping_block_starting_address_ss << "address#"<< hr.mb_mapping_registers_start_address_;

              prepare_child_group(block_grp_ss.str().c_str(), mb_hr_mapping_block_starting_address_ss.str().c_str());

              std::stringstream block_starting_address_grp_ss;
              block_starting_address_grp_ss << block_grp_ss.str() << "/" << mb_hr_mapping_block_starting_address_ss.str() ;

              // Now create / prepare a group for each channel (coil / input bit / holding register / input register).
              // Please note / refer to the modbus mapping... we have to create a full mapping 0 ... max holding registers etc.
              for(int hr_n = 0;  hr_n <= hr.mb_mapping_nb_registers_ ; hr_n++) {
                std::stringstream hr_n_ss;
                hr_n_ss << hr_n;
                prepare_child_group(block_starting_address_grp_ss.str().c_str(), hr_n_ss.str().c_str());
              }
            }
          }
          //////////////////////
          // INPUT REGISTERS ...
          // Read the Input Register blocks.
          {
            nlohmann::json jinreg_blocks = jnode["agent"]["node"]["modbus_master"]["slave_device"]["modbus_mapping"]["input_register_blocks"];

            for(nlohmann::json::iterator irit = jinreg_blocks.begin(); irit != jinreg_blocks.end(); ++irit) {
              nlohmann::json jir(*irit);
              mb_input_register_block_t ir;
              ir.block_id_ = jir["block_id"];
              ir.mb_mapping_input_registers_start_address_ = jir["input_registers_start_address"];
              ir.mb_mapping_nb_input_registers_ = jir["nb_input_registers"];
              ir.mb_mapping_input_registers_end_address_ = jir["input_registers_end_address"];

              std::stringstream mb_ir_mapping_ss;
              mb_ir_mapping_ss << mb_mapping_ss.str() << "/" << MODBUS_MAPPING_NAME_INPUT_REGISTERS.c_str();

              std::stringstream mb_ir_mapping_block_ss;
              mb_ir_mapping_block_ss << "block#"<< ir.block_id_;

              prepare_child_group(mb_ir_mapping_ss.str().c_str(), mb_ir_mapping_block_ss.str().c_str());

              // Now prepare a group for the starting address of the block
              std::stringstream block_grp_ss;
              block_grp_ss << mb_ir_mapping_ss.str() << "/" << mb_ir_mapping_block_ss.str();

              std::stringstream mb_ir_mapping_block_starting_address_ss;
              mb_ir_mapping_block_starting_address_ss << "address#"<< ir.mb_mapping_input_registers_start_address_;

              prepare_child_group(block_grp_ss.str().c_str(), mb_ir_mapping_block_starting_address_ss.str().c_str());

              std::stringstream block_starting_address_grp_ss;
              block_starting_address_grp_ss << block_grp_ss.str() << "/" << mb_ir_mapping_block_starting_address_ss.str() ;

              // Now create / prepare a group for each channel (coil / input bit / holding register / input register).
              // Please note / refer to the modbus mapping... we have to create a full mapping 0 ... max holding registers etc.
              for(int ir_n = 0;  ir_n <= ir.mb_mapping_nb_input_registers_ ; ir_n++) {
                std::stringstream ir_n_ss;
                ir_n_ss << ir_n;
                prepare_child_group(block_starting_address_grp_ss.str().c_str(), ir_n_ss.str().c_str());
              }
            }
          }
        }
      }
    }
    catch(nlohmann::json::exception& e) {
      // Log exception information
      std::stringstream ssReqException;
      ssReqException  << "message: " << e.what()    <<  " "
        << "exception id: " << e.id   << std::endl;
      logger->error("#init_mb_dataset_recorder ill-formed controller configuration - {}.", ssReqException.str());
    }
  }

  return 0;
}

int mb_node_master_dataset_recorder_main_t::init_mb_dataset_recorder()
{

  // Initialize the logger
  init_h5_service_loggers(SMART_LOGGER_H5_LOGGER_CODE_, SMART_LOGGER_H5_LOGGER_NAME_);

  // Get the instance of launcher logger.
  auto logger = spdlog::get(SMART_LOGGER_H5_LOGGER_NAME_);

  // Read the global config / node registry.
  // Step 1: Check if the controller is modbus-master.
  // Step 2: modbus-server will be handled in future versions.
  // Get each node = controller -> agent -> node -> modbus_master_node

  //
  // DESIGN NOTES
  // Now initialize the SObjectizer to process the received modbus messages.
  //
  ds_rec_env_obj_ = std::make_shared<so_5::wrapped_env_t>();
  msg_primary_channel_ = so_5::create_mchain(*ds_rec_env_obj_);
  auto msg_primary_channel_closer = so_5::auto_close_drop_content(msg_primary_channel_);

  msg_secondary_channel_ = so_5::create_mchain(msg_primary_channel_->environment());
  auto msg_secondary_channel_closer = so_5::auto_close_drop_content(msg_secondary_channel_);

  // No initialize the dataset (HDF5) file and its initial groups.
  init_h5_dataset_file();

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
  std::transform(dataset_ws_client_workers_vec_.begin(), dataset_ws_client_workers_vec_.end()
  , dataset_ws_client_workers_vec_.begin(), [](mb_node_master_dataset_ws_client_worker_t &s) {
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

////////////////////////////
// Prepare Device Id
////////////////////////////
std::string mb_node_master_dataset_recorder_main_t::prepare_device_id
(const std::string& controller_id, const std::string& agent_id, const std::string& node_id)
{
  std::stringstream v_id;
  v_id << controller_id << "/" << agent_id << "/" << node_id;
  return v_id.str();
}

////////////////////////////
// Prepare HDF5 Grouping
////////////////////////////
std::string mb_node_master_dataset_recorder_main_t::prepare_mb_register_mapping_h5_group_id
(const std::string& v_device_id, const std::string& mb_mapping_type,  int block_id, int start_address, int reg_number)
{
  std::stringstream h5_g_id;
  h5_g_id << "/" << v_device_id << "/" << mb_mapping_type << "/" << "block#" << block_id << "/" << "address#" << start_address << "/" << reg_number;
  return h5_g_id.str();
}

std::string mb_node_master_dataset_recorder_main_t::prepare_mb_coil_and_input_bits_mapping_h5_group_id
(const std::string& v_device_id, const std::string& mb_mapping_type,  int block_id, int start_address, int bit_number)
{
  std::stringstream h5_g_id;
  h5_g_id << "/" << v_device_id << "/" << mb_mapping_type << "/" << "block#" << block_id << "/" << "address#" << start_address << "/" << bit_number;
  return h5_g_id.str();
}
