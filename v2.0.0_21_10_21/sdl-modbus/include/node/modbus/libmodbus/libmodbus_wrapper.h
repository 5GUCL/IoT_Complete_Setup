/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_MODBUS_LIBMODBUS_WRAPPER_H_
#define _CS_EDM_MODBUS_LIBMODBUS_WRAPPER_H_

/* Standard C++ includes */
#include <string>

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <modbus/modbus.h>

#include "nlohmann/json.hpp"
#include "sf-sys/utils/cpp_prog_utils.h"
#include "sf-sys/utils/time_utils.h"
#include "spdlog/spdlog.h"

/* Base node */
struct libmodbus_wrapper_t {
  libmodbus_wrapper_t (const std::string slave_address, int slave_port, int slave_id, const std::string& logger_name);
  ~libmodbus_wrapper_t();

  std::string slave_address_;
  int         slave_port_;
  int         slave_id_;
  std::string logger_name_;

  modbus_t    *mb_ctx_;

  // APIS
  modbus_t* modbus_new_tcp_wf();
  libmodbus_wrapper_t& modbus_flush_wf();
  int modbus_read_bits_wf(int bits_start_address, int nb_bits, uint8_t *tab_bit);
  int modbus_read_input_bits_wf(int input_bits_start_address, int nb_input_bits, uint8_t *tab_input_bit);
  int modbus_read_registers_wf(int registers_start_address_, int nb_holding_regs, uint16_t *tab_reg);
  int modbus_read_input_registers_wf(int input_registers_start_address, int nb_input_regs, uint16_t *tab_input_reg);

  int modbus_write_bit_wf(int bits_start_address, int value);
};
#endif /* _CS_EDM_MODBUS_LIBMODBUS_WRAPPER_H_ */