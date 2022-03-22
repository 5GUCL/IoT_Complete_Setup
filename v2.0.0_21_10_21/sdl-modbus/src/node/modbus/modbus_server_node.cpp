/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#include <random>
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include "sf-sys/utils/time_utils.h"
#include "sf-sys/utils/cpp_prog_utils.h"

#include "node/modbus/modbus_server_node.h"

////////////////////////////////////////////////////////////////////////
//                         SERVER NODES                               //
////////////////////////////////////////////////////////////////////////

modbus_server_node_device_t::modbus_server_node_device_t()
{
  run_mb_server_ = false;
  mb_s_ = -1;
  mb_server_ctx_ = nullptr;
}
modbus_server_node_device_t::~modbus_server_node_device_t()
{
  // Clean up the server ...
  run_mb_server_ = false;

  if(mb_s_ != -1) {
    close(mb_s_);
  }

  mb_s_ = -1;

  if(mb_server_ctx_) {
    modbus_close(mb_server_ctx_);
    modbus_free(mb_server_ctx_);
  }

  mb_server_ctx_ = nullptr;
}


modbus_server_node_t::modbus_server_node_t
(const std::string& controller_id
  , const std::string& agent_id, const std::string& agent_type
  , const std::string& agent_name, const std::string& agent_address
  , const std::string& agent_api_key, const std::string& agent_logger_name
  , const std::string& node_id, const std::string& node_type
  , const std::string& node_name, const std::string& node_address, const std::string& node_api_key)
  : controller_id_(controller_id)
  , agent_id_(agent_id), agent_type_(agent_type)
  , agent_name_(agent_name), agent_address_(agent_address)
  , agent_api_key_(agent_api_key), agent_logger_name_(agent_logger_name)
  , node_id_(node_id), node_type_(node_type)
  , node_name_(node_name), node_address_(node_address), node_api_key_(node_api_key)
{
  mb_node_init_status_ = false;
}

modbus_server_node_t::~modbus_server_node_t()
{

}

void modbus_server_node_t::reset_modbus_server_fn(int code)
{

  // Clean up the server ...
  mb_server_node_device_.run_mb_server_ = false;

  if(mb_server_node_device_.mb_s_ != -1) {
    close(mb_server_node_device_.mb_s_);
  }

  mb_server_node_device_.mb_s_ = -1;

  if(mb_server_node_device_.mb_server_ctx_) {
    modbus_flush(mb_server_node_device_.mb_server_ctx_);
    modbus_close(mb_server_node_device_.mb_server_ctx_);
    modbus_free(mb_server_node_device_.mb_server_ctx_);
  }

  mb_server_node_device_.mb_server_ctx_ = nullptr;

  // Retry later...
  const std::chrono::milliseconds pause{0};
  so_5::send_delayed<modbus_server_mailbox_message_t>(mb_secondary_channel_, pause);
}

void modbus_server_node_t::mb_server_node_processing_fn()
{

  std::random_device rd;
  std::mt19937 generator{rd()};
  std::uniform_int_distribution<> rand_generator{750, 3750};
  std::uniform_int_distribution<> rand_generator_binary {0, 1};

  auto logger = spdlog::get(agent_logger_name_);
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
  so_5::receive_case(mb_primary_channel_
  , [this, &generator, &rand_generator, &rand_generator_binary, &logger](modbus_server_mailbox_message_t) {
    // Modbus registers
    // Discrete Inputs                  (READ)
    // Coils (Outputs)                  (READ+WRITE)
    // Input Registers (Input Data)     (READ)
    // Holding Registers (Output Data)  (READ+WRITE)

    // Modbus server
    mb_server_node_device_.run_mb_server_ = true;
    mb_server_node_device_.mb_s_ = -1;
    mb_server_node_device_.mb_server_ctx_ = nullptr;
    int master_socket;
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    int rc;
    fd_set refset;
    fd_set rdset;

    /* Maximum file descriptor number */
    int fdmax;

    mb_server_node_device_.mb_server_ctx_ = modbus_new_tcp(mb_server_node_device_.mb_device_address_.c_str(), mb_server_node_device_.mb_device_port_);

    if(mb_server_node_device_.mb_server_ctx_ == NULL) {
      // Failed to allocate MOdbus TCP server context
      modbus_free(mb_server_node_device_.mb_server_ctx_);
      mb_server_node_device_.mb_server_ctx_ = nullptr;
      mb_server_node_device_.run_mb_server_ = false;

      // Retry later...
      const std::chrono::milliseconds pause{rand_generator(generator)};
      so_5::send_delayed<modbus_server_mailbox_message_t>(mb_secondary_channel_, pause);
      logger->error("#modbus #modbus_server_node_t::mb_server_node_processing_fn server reset/1 initiated for the controller {}.",  controller_id_);
      return;
    }

    // Let's use some debug stuff...
    // modbus_set_debug(mb_server_ctx_, TRUE);

    mb_server_node_device_.mb_s_ = modbus_tcp_listen(mb_server_node_device_.mb_server_ctx_, mb_server_node_device_.mb_connections_);

    /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(mb_server_node_device_.mb_s_, &refset);

    /* Keep track of the max file descriptor */
    fdmax = mb_server_node_device_.mb_s_;

    for(; mb_server_node_device_.run_mb_server_;) {
      rdset = refset;

      if(select(fdmax+1, &rdset, NULL, NULL, NULL) == -1) {
        logger->error("#modbus #modbus_server_node_t::mb_server_node_processing_fn Server select() failure for the controller {}.",  controller_id_);
        return reset_modbus_server_fn(1);
      }

      /* Run through the existing connections looking for data to be read */
      for(master_socket = 0; master_socket <= fdmax; master_socket++) {

        if(!FD_ISSET(master_socket, &rdset)) {
          continue;
        }

        if(master_socket == mb_server_node_device_.mb_s_) {
          /* A client is asking a new connection */
          socklen_t addrlen;
          struct sockaddr_in clientaddr;
          int newfd;

          /* Handle new connections */
          addrlen = sizeof(clientaddr);
          memset(&clientaddr, 0, sizeof(clientaddr));
          newfd = accept(mb_server_node_device_.mb_s_, (struct sockaddr *)&clientaddr, &addrlen);

          if(newfd == -1) {
            logger->error("#modbus #modbus_server_node_t::mb_server_node_processing_fn Server accept() error for the controller {}.",  controller_id_);
          }
          else {
            FD_SET(newfd, &refset);

            if(newfd > fdmax) {
              /* Keep track of the maximum */
              fdmax = newfd;
            }

            /*
            logger->info("#modbus #modbus_server_node_t::mb_server_node_processing_fn New connection from {}:{} on socket {} for the controller {}."
              , inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, newfd
              ,  controller_id_);
            */
          }
        }
        else {
          modbus_set_socket(mb_server_node_device_.mb_server_ctx_, master_socket);
          rc = modbus_receive(mb_server_node_device_.mb_server_ctx_, query);

          if(rc > 0) {
            //
            // DESIGN NOTES
            // Parse the query for supported functions.
            // 16 bits access is supported in the current version of the data logger software.
            // Function Codes:
            // 01. -> Read Input Register,      Function Code 04  HEX 04
            // 02. -> Read Holding Registers,   Function Code 03  HEX 03
            // 03. -> Write Single Register,    Function Code 06  HEX 06
            // 04. -> Read Discrete Inputs,     Function Code 02  HEX 02
            // 05. -> Read Coils,               Function Code 01  HEX 01
            // 06. -> Write Single Coil,        Function Code 05  HEX 05
            // 07. -> Write Multiple Coils,     Function Code 15  HEX 0F

            /* It's not really the minimal length (the real one is report slave ID
             * in RTU (4 bytes)) but it's a convenient size to use in RTU or TCP
             * communications to read many values or write a single one.
             * Maximum between :
             * - HEADER_LENGTH_TCP (7) + function (1) + address (2) + number (2)
             * - HEADER_LENGTH_RTU (1) + function (1) + address (2) + number (2) + CRC (2)
             */
            const int HEADER_LENGTH_TCP(7);
            const int offset = HEADER_LENGTH_TCP;
            const int function = query[offset];

            ///////////////////////////////////////////////////
            if(function == 01) {
              // Read the starting address
              int coil_bits_start_address_block = 0;
              uint8_t start_addres_byte_1 =  query[offset + 1];
              uint8_t start_addres_byte_2 =  query[offset + 2];
              coil_bits_start_address_block = start_addres_byte_1;
              coil_bits_start_address_block = (coil_bits_start_address_block << 8);
              coil_bits_start_address_block = (coil_bits_start_address_block | start_addres_byte_2);

              // Read the number of coils.
              int nb_coil_bits = 0;
              uint8_t reg_byte_1 =  query[offset + 2 + 1];
              uint8_t reg_byte_2 =  query[offset + 2 + 2];
              nb_coil_bits = reg_byte_1;
              nb_coil_bits = (nb_coil_bits << 8);
              nb_coil_bits = (nb_coil_bits | reg_byte_2);

              //
              // DESIGN NOTES
              // Chek if the given coils_bits_start_address_block is a valid in the configuration?
              // Allow only those addresses and coils.
              //

              if((0x0001 <= nb_coil_bits) && (nb_coil_bits <= 0x07D0)) {
                bool is_query_valid = false;

                //////////////////////////////
                if(std::any_of(mb_server_node_device_.mb_mapping_coil_blocks_.begin()
                    , mb_server_node_device_.mb_mapping_coil_blocks_.end()
                , [&](std::pair<const int, mb_coil_block_t> v_id) {
                if((coil_bits_start_address_block == v_id.second.mb_mapping_bits_start_address_)
                    && (nb_coil_bits <= v_id.second.mb_mapping_nb_bits_)
                    && ((coil_bits_start_address_block + (nb_coil_bits/8) + (nb_coil_bits%8) == 0 ? 0 : 1)
                      <  v_id.second.mb_mapping_bits_end_address_)) {
                    return true;
                  }

                  return false;
                })) { // End of IF
                  is_query_valid = true;
                }
                //////////////////////////////

                if(is_query_valid) {
                  modbus_mapping_t *mb_mapping = modbus_mapping_new_start_address(coil_bits_start_address_block, nb_coil_bits, 0, 0, 0, 0, 0, 0);

                  if(mb_mapping == NULL) {
                    modbus_reply_exception(mb_server_node_device_.mb_server_ctx_
                      , query
                      , MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE);
                  }
                  else {
                    // Prepare the memory map...
                    for(int r = 0; r < nb_coil_bits; r++) {
                      mb_mapping->tab_bits[r] = rand_generator_binary(generator) ? TRUE : FALSE;
                    }

                    modbus_reply(mb_server_node_device_.mb_server_ctx_, query, rc, mb_mapping);

                    // Now free the modbus memory resources
                    modbus_mapping_free(mb_mapping);
                  }
                }
                else {
                  // Log Here.
                  modbus_reply_exception(mb_server_node_device_.mb_server_ctx_
                    , query
                    , MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE);
                }
              }
              else {
                // Log Here.
                modbus_reply_exception(mb_server_node_device_.mb_server_ctx_
                  , query
                  , MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
              }
            }
            ////////////////////////////////////////////////////
            else if(function == 02) {
              // Read the starting address
              int input_bits_start_address_block = 0;
              uint8_t start_addres_byte_1 =  query[offset + 1];
              uint8_t start_addres_byte_2 =  query[offset + 2];
              input_bits_start_address_block = start_addres_byte_1;
              input_bits_start_address_block = (input_bits_start_address_block << 8);
              input_bits_start_address_block = (input_bits_start_address_block | start_addres_byte_2);

              // Read the number of input bits.
              int nb_input_bits = 0;
              uint8_t reg_byte_1 =  query[offset + 2 + 1];
              uint8_t reg_byte_2 =  query[offset + 2 + 2];
              nb_input_bits = reg_byte_1;
              nb_input_bits = (nb_input_bits << 8);
              nb_input_bits = (nb_input_bits | reg_byte_2);

              //
              // DESIGN NOTES
              // Chek if the given input_bits_start_address_block is a valid in the configuration?
              // Allow only those addresses and input bits.
              //
              if((0x0001 <= nb_input_bits) && (nb_input_bits <= 0x07D0)) {
                bool is_query_valid = false;

                //////////////
                if(std::any_of(mb_server_node_device_.mb_mapping_input_bit_blocks_.begin()
                    , mb_server_node_device_.mb_mapping_input_bit_blocks_.end()
                , [&](std::pair<const int, mb_input_bit_block_t> v_id) {
                if((input_bits_start_address_block == v_id.second.mb_mapping_input_bits_start_address_)
                    && (nb_input_bits <= v_id.second.mb_mapping_nb_input_bits_)
                    && ((input_bits_start_address_block + (nb_input_bits/8) + (nb_input_bits%8) == 0 ? 0 : 1)
                      <  v_id.second.mb_mapping_input_bits_end_address_)) {
                    // Found at least one valid address block.
                    return true;
                  }

                  return false;
                })) {
                  is_query_valid = true;
                }
                //////////////

                if(is_query_valid) {
                  modbus_mapping_t *mb_mapping = modbus_mapping_new_start_address(0, 0, input_bits_start_address_block, nb_input_bits, 0, 0, 0, 0);

                  if(mb_mapping == NULL) {
                    modbus_reply_exception(mb_server_node_device_.mb_server_ctx_
                      , query
                      , MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE);
                  }
                  else {
                    // Prepare the memory map...
                    for(int r = 0; r < nb_input_bits; r++) {
                      mb_mapping->tab_input_bits[r] = rand_generator_binary(generator) ? TRUE : FALSE;
                    }

                    modbus_reply(mb_server_node_device_.mb_server_ctx_, query, rc, mb_mapping);

                    // Now free the modbus memory resources
                    modbus_mapping_free(mb_mapping);
                  }
                }
                else {
                  // Log Here.
                  modbus_reply_exception(mb_server_node_device_.mb_server_ctx_
                    , query
                    , MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE);
                }
              }
              else {
                // Log Here.
                modbus_reply_exception(mb_server_node_device_.mb_server_ctx_
                  , query
                  , MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
              }

            } // Read Discrete Inputs

            // Read Holding Registers, Function Code 03  HEX 03
            else if(function == 03) {
              // Read the starting address
              int holding_reg_start_address_block = 0;
              uint8_t start_addres_byte_1 =  query[offset + 1];
              uint8_t start_addres_byte_2 =  query[offset + 2];
              holding_reg_start_address_block = start_addres_byte_1;
              holding_reg_start_address_block = (holding_reg_start_address_block << 8);
              holding_reg_start_address_block = (holding_reg_start_address_block | start_addres_byte_2);

              // Read the number of registers.
              int nb_holding_registers = 0;
              uint8_t reg_byte_1 =  query[offset + 2 + 1];
              uint8_t reg_byte_2 =  query[offset + 2 + 2];
              nb_holding_registers = reg_byte_1;
              nb_holding_registers = (nb_holding_registers << 8);
              nb_holding_registers = (nb_holding_registers | reg_byte_2);

              //
              // DESIGN NOTES
              // Chek if the given holding_reg_start_address_block is a valid in the configuration?
              // Allow only those addresses and registers.
              //
              if((0x0001 <= nb_holding_registers) && (nb_holding_registers <= 0x007D)) {
                bool is_query_valid = false;

                if(std::any_of(mb_server_node_device_.mb_mapping_holding_register_blocks_.begin()
                    , mb_server_node_device_.mb_mapping_holding_register_blocks_.end()
                , [&](std::pair<const int, mb_holding_register_block_t> v_id) {
                if((holding_reg_start_address_block == v_id.second.mb_mapping_registers_start_address_)
                    && (nb_holding_registers <= v_id.second.mb_mapping_nb_registers_)
                    && ((holding_reg_start_address_block + nb_holding_registers) <  v_id.second.mb_mapping_registers_end_address_)) {
                    // Found at least one valid address block.
                    return true;
                  }

                  return false;
                })) {
                  is_query_valid = true;
                }

                if(is_query_valid) {
                  modbus_mapping_t *mb_mapping = modbus_mapping_new_start_address(0,0,0,0,holding_reg_start_address_block,nb_holding_registers,0,0);

                  if(mb_mapping == NULL) {
                    modbus_reply_exception(mb_server_node_device_.mb_server_ctx_
                      , query
                      , MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE);
                  }
                  else {
                    // Prepare the memory map...
                    for(int r = 0; r < nb_holding_registers; r++) {
                      mb_mapping->tab_registers[r] = rand_generator(generator);
                    }

                    modbus_reply(mb_server_node_device_.mb_server_ctx_, query, rc, mb_mapping);

                    // Now free the modbus memory resources
                    modbus_mapping_free(mb_mapping);
                  }
                }
                else {
                  // Log Here.
                  modbus_reply_exception(mb_server_node_device_.mb_server_ctx_
                    , query
                    , MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE);
                }
              }
              else {
                // Log Here.
                modbus_reply_exception(mb_server_node_device_.mb_server_ctx_
                  , query
                  , MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
              }
            }
            // Read Input Register, Function Code 04  HEX 04
            else if(function == 04) {
              // Read the starting address
              int input_reg_start_address_block = 0;
              uint8_t start_addres_byte_1 =  query[offset + 1];
              uint8_t start_addres_byte_2 =  query[offset + 2];
              input_reg_start_address_block = start_addres_byte_1;
              input_reg_start_address_block = (input_reg_start_address_block << 8);
              input_reg_start_address_block = (input_reg_start_address_block | start_addres_byte_2);

              // Read the number of registers.
              int nb_input_registers = 0;
              uint8_t reg_byte_1 =  query[offset + 2 + 1];
              uint8_t reg_byte_2 =  query[offset + 2 + 2];
              nb_input_registers = reg_byte_1;
              nb_input_registers = (nb_input_registers << 8);
              nb_input_registers = (nb_input_registers | reg_byte_2);

              //
              // DESIGN NOTES
              // Chek if the given input_reg_start_address_block is a valid in the configuration?
              // Allow only those addresses and registers.
              //
              if((0x0001 <= nb_input_registers) && (nb_input_registers <= 0x007D)) {
                bool is_query_valid = false;

                if(std::any_of(mb_server_node_device_.mb_mapping_input_register_blocks_.begin()
                    , mb_server_node_device_.mb_mapping_input_register_blocks_.end()
                , [&](std::pair<const int, mb_input_register_block_t> v_id) {
                if((input_reg_start_address_block == v_id.second.mb_mapping_input_registers_start_address_)
                    && (nb_input_registers <= v_id.second.mb_mapping_nb_input_registers_)
                    && ((input_reg_start_address_block + nb_input_registers) < v_id.second.mb_mapping_input_registers_end_address_)) {
                    // Found at least one valid address block.
                    return true;
                  }

                  return false;
                })) {
                  is_query_valid = true;
                }

                if(is_query_valid) {
                  modbus_mapping_t *mb_mapping = modbus_mapping_new_start_address(0,0,0,0,0,0, input_reg_start_address_block, nb_input_registers);

                  if(mb_mapping == NULL) {
                    modbus_reply_exception(mb_server_node_device_.mb_server_ctx_
                      , query
                      , MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE);
                  }
                  else {
                    // Prepare the memory map...
                    for(int r = 0; r < nb_input_registers; r++) {
                      mb_mapping->tab_input_registers[r] = rand_generator(generator);
                    }

                    modbus_reply(mb_server_node_device_.mb_server_ctx_, query, rc, mb_mapping);


                    // Now free the modbus memory resources
                    modbus_mapping_free(mb_mapping);
                  }
                }
                else {
                  // Log Here.
                  modbus_reply_exception(mb_server_node_device_.mb_server_ctx_
                    , query
                    , MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE);
                }
              }
              else {
                // Log Here.
                modbus_reply_exception(mb_server_node_device_.mb_server_ctx_
                  , query
                  , MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
              }

            }

            if(function == 05) { // Writing Single Coil Bit.
              // Read the starting address
              int coil_bits_start_address = 0;
              uint8_t start_addres_byte_1 =  query[offset + 1];
              uint8_t start_addres_byte_2 =  query[offset + 2];
              coil_bits_start_address = start_addres_byte_1;
              coil_bits_start_address = (coil_bits_start_address << 8);
              coil_bits_start_address = (coil_bits_start_address | start_addres_byte_2);

              // Read the Coil Value.
              int nb_coil_output_value = 0;
              uint8_t val_byte_1 =  query[offset + 2 + 1];
              uint8_t val_byte_2 =  query[offset + 2 + 2];
              nb_coil_output_value = val_byte_1;
              nb_coil_output_value = (nb_coil_output_value << 8);
              nb_coil_output_value = (nb_coil_output_value | val_byte_2);

              //
              // DESIGN NOTES
              // Chek if the given coils_bits_start_address_block is a valid in the configuration?
              // Allow only those addresses and coils.
              //

              if((0x0000 == nb_coil_output_value) || (nb_coil_output_value == 0xFF00)) {
                bool is_query_valid = false;

                if(std::any_of(mb_server_node_device_.mb_mapping_coil_blocks_.begin()
                    , mb_server_node_device_.mb_mapping_coil_blocks_.end()
                , [&](std::pair<const int, mb_coil_block_t> v_id) {
                if((v_id.second.mb_mapping_bits_start_address_ <= coil_bits_start_address)
                    && (coil_bits_start_address <= v_id.second.mb_mapping_bits_end_address_)) {
                    // Found at least one valid address block.
                    return true;
                  }

                  return false;
                })) {
                  is_query_valid = true;
                }

                if(is_query_valid) {
                  modbus_mapping_t *mb_mapping = modbus_mapping_new_start_address(coil_bits_start_address, 1, 0, 0, 0, 0, 0, 0);

                  if(mb_mapping == NULL) {
                    modbus_reply_exception(mb_server_node_device_.mb_server_ctx_
                      , query
                      , MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE);
                  }
                  else {

                    // Now Output ON/OFF on the coil.
                    mb_mapping->tab_bits[0] = nb_coil_output_value ? TRUE : FALSE;

                    modbus_reply(mb_server_node_device_.mb_server_ctx_, query, rc, mb_mapping);

                    // Now free the modbus memory resources
                    modbus_mapping_free(mb_mapping);
                  }
                }
                else {
                  // Log Here.
                  modbus_reply_exception(mb_server_node_device_.mb_server_ctx_
                    , query
                    , MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE);
                }
              }
              else {
                // Log Here.
                modbus_reply_exception(mb_server_node_device_.mb_server_ctx_
                  , query
                  , MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
              }
            }

            else {
              // Log Here.
              modbus_reply_exception(mb_server_node_device_.mb_server_ctx_
                , query
                , MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
            }
          }
          else if(rc == -1) {
            /* Connection closing or any errors. */
            /*logger->error ("#modbus #modbus_server_node_t::mb_server_node_processing_fn Connection closed on socket {} for the controller {}."
                    , master_socket
                    , controller_id_);
            */
            close(master_socket);

            /* Remove from reference set */
            FD_CLR(master_socket, &refset);

            if(master_socket == fdmax) {
              fdmax--;
            }
          }
        }
      }
    }

    logger->error("#modbus #modbus_server_node_t::mb_server_node_processing_fn reset/3 initiated for the controller {}.",  controller_id_);

    // Clean up the server ...
    return reset_modbus_server_fn(0);

  }
  ),
  so_5::receive_case(mb_secondary_channel_
  , [this, &generator, &rand_generator](modbus_server_mailbox_message_t) {
    // Do some stuff
    const std::chrono::milliseconds pause{rand_generator(generator)};
    so_5::send_delayed<modbus_server_mailbox_message_t>(mb_primary_channel_, pause);
  }
  )
    );

  do {
    so_5::select(prepared);
  }
  while(!stop);
}

modbus_server_node_t& modbus_server_node_t::init_modbus_node()
{

  if(mb_node_init_status_) {
    return *this;
  }

  auto logger = spdlog::get(agent_logger_name_);

  mb_server_node_env_obj_ = std::make_shared<so_5::wrapped_env_t>();
  mb_primary_channel_ = so_5::create_mchain(*mb_server_node_env_obj_);
  auto mb_primary_channel_closer = so_5::auto_close_drop_content(mb_primary_channel_);

  mb_secondary_channel_ = so_5::create_mchain(mb_primary_channel_->environment());
  auto mb_secondary_channel_closer = so_5::auto_close_drop_content(mb_secondary_channel_);

  mb_mthread_ = std::make_shared<std::thread>([this]() {
    mb_server_node_processing_fn();
  });

  so_5::send<modbus_server_mailbox_message_t>(mb_primary_channel_);

  auto controller_joiner = so_5::auto_join(*mb_mthread_);

  // Now set the init status.
  mb_node_init_status_ = true;

  return *this;
}

modbus_server_node_t& modbus_server_node_t::prepare_modbus_node_events()
{
  auto logger = spdlog::get(agent_logger_name_);

  return (*this);
}

modbus_server_node_t& modbus_server_node_t::print_node()
{
  auto logger = spdlog::get(agent_logger_name_);
  CTimeUtils ct;
  long ms = ct.getTimestampInMilliSeconds();
  std::cout << " ========= " << ct.formatTime2Readable(ms)<< " =========== " <<  std::endl;
  std::cout << node_id_       << "/" << node_name_          << "/" << node_address_ << "/"
    << node_api_key_  << "/" << agent_logger_name_  << "/" << std::endl;

  return *this;
}

std::shared_ptr<std::vector<modbus_node_event_t>> modbus_server_node_t::get_modbus_node_events()
{
  auto logger = spdlog::get(agent_logger_name_);
  std::vector<modbus_node_event_t> node_events;
  node_events = node_events_;
  std::shared_ptr<std::vector<modbus_node_event_t>> evnts_sp = std::make_shared<std::vector<modbus_node_event_t>>(node_events);
  return evnts_sp;
}
