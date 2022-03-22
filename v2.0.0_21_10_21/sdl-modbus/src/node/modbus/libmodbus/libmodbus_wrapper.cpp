/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#include "node/modbus/libmodbus/libmodbus_wrapper.h"

libmodbus_wrapper_t::libmodbus_wrapper_t (const std::string slave_address, int slave_port, int slave_id, const std::string& logger_name)
{
  slave_address_  = slave_address;
  slave_port_     = slave_port;
  slave_id_       = slave_id;
  logger_name_    = logger_name;
  mb_ctx_         = nullptr;
}


libmodbus_wrapper_t::~libmodbus_wrapper_t()
{
  if(mb_ctx_) {
    modbus_flush(mb_ctx_);
    modbus_close(mb_ctx_);
    modbus_free(mb_ctx_);
  }
}

modbus_t* libmodbus_wrapper_t::modbus_new_tcp_wf()
{
  auto logger = spdlog::get(logger_name_);

  auto destroy_modbus_conn = [&] {
    if(mb_ctx_)
    {
      modbus_flush(mb_ctx_);
      modbus_close(mb_ctx_);
      modbus_free(mb_ctx_);
      mb_ctx_ = nullptr;
    }
  };

  mb_ctx_ = modbus_new_tcp(slave_address_.c_str(), slave_port_);

  if(mb_ctx_ == NULL) {
    on_scope_exit close_modbus_resources {destroy_modbus_conn};
    logger->critical("#modbus_new_tcp connection to the slave {}:{} failed", slave_address_, slave_port_);
    return nullptr;
  }

  int rc = modbus_set_slave(mb_ctx_, slave_id_);

  if(rc == -1) {
    on_scope_exit close_modbus_resources {destroy_modbus_conn};
    logger->critical("#modbus_set_slave set slave id {}:{}/{} failed", slave_address_, slave_port_, slave_id_);
    return nullptr;
  }

  if(modbus_connect(mb_ctx_) == -1) {
    on_scope_exit close_modbus_resources {destroy_modbus_conn};
    logger->critical("Connection failed slave {}:{} with modbus error {}.", slave_address_, slave_port_, modbus_strerror(errno));
    return nullptr;
  }

  return mb_ctx_;
}

libmodbus_wrapper_t& libmodbus_wrapper_t::modbus_flush_wf()
{
  if(mb_ctx_) {
    modbus_flush(mb_ctx_);
  }

  return *this;
}

int libmodbus_wrapper_t::modbus_read_bits_wf(int bits_start_address, int nb_bits, uint8_t *tab_bit)
{
  auto logger = spdlog::get(logger_name_);

  if(mb_ctx_) {
    int rc = modbus_read_bits(mb_ctx_, bits_start_address, nb_bits, tab_bit);

    if(rc == -1) {
      logger->critical("#modbus_read_bits (coils) failed - slave {}:{} with modbus error {}."
        , slave_address_, slave_port_, modbus_strerror(errno));
      return rc;
    }

    return rc;
  }

  return -1;
}

int libmodbus_wrapper_t::modbus_read_input_bits_wf(int input_bits_start_address, int nb_input_bits, uint8_t *tab_input_bit)
{
  auto logger = spdlog::get(logger_name_);

  if(mb_ctx_) {
    int rc = modbus_read_input_bits(mb_ctx_, input_bits_start_address, nb_input_bits, tab_input_bit);

    if(rc == -1) {
      logger->critical("#modbus_read_input_bits failed - slave {}:{} with modbus error {}."
        , slave_address_, slave_port_, modbus_strerror(errno));
      return rc;
    }

    return rc;
  }

  return -1;
}

int libmodbus_wrapper_t::modbus_read_registers_wf(int registers_start_address_, int nb_holding_regs, uint16_t *tab_reg)
{
  auto logger = spdlog::get(logger_name_);

  if(mb_ctx_) {
    int rc = modbus_read_registers(mb_ctx_, registers_start_address_, nb_holding_regs, tab_reg);

    if(rc == -1) {
      logger->critical("#modbus_read_registers failed - slave {}:{} with modbus error {}."
        , slave_address_, slave_port_, modbus_strerror(errno));
      return rc;
    }

    return rc;
  }

  return -1;
}

int libmodbus_wrapper_t::modbus_read_input_registers_wf(int input_registers_start_address, int nb_input_regs, uint16_t *tab_input_reg)
{
  auto logger = spdlog::get(logger_name_);

  if(mb_ctx_) {
    int rc = modbus_read_input_registers(mb_ctx_, input_registers_start_address, nb_input_regs, tab_input_reg);

    if(rc == -1) {
      logger->critical("#modbus_read_input_registers failed - slave {}:{} with modbus error {}."
        , slave_address_, slave_port_, modbus_strerror(errno));
      return rc;
    }

    return rc;
  }

  return -1;
}

int libmodbus_wrapper_t::modbus_write_bit_wf(int bits_start_address, int value)
{
  auto logger = spdlog::get(logger_name_);

  if(mb_ctx_) {
    int rc = modbus_write_bit(mb_ctx_, bits_start_address, (value ? TRUE : FALSE));

    if(rc == -1) {
      logger->critical("#modbus_write_bit failed - slave {}:{} Coil Address {} Value {} with modbus error {}."
        , slave_address_, slave_port_, bits_start_address, (value ? TRUE : FALSE), modbus_strerror(errno));
      return rc;
    }

    return rc;
  }

  return -1;
}