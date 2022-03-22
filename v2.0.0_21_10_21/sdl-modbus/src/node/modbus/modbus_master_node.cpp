/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#include "cpr/cpr.h"
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include "sf-sys/utils/time_utils.h"
#include "sf-sys/utils/cpp_prog_utils.h"
#include "node/modbus/modbus_master_node.h"
#include "meta-data-models/thing.h"
#include "node/modbus/libmodbus/libmodbus_wrapper.h"

////////////////////////////////////////////////////////////////////////
//                 MASTER NODES -> SLAVE DEVICES                      //
////////////////////////////////////////////////////////////////////////
/* MODBUS node */
modbus_master_node_t::modbus_master_node_t
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

modbus_master_node_t::~modbus_master_node_t()
{

}

// Modbus Function Messages
int modbus_master_node_t::post_write_single_coil_async(long msg_idx // Used by the application to identify the req-resp
  , int function_code
  , int block_id
  , int mb_mapping_coil_address
  , int output_value) const
{

  so_5::send<msg_modbus_write_single_coil_request_t>(mb_device_write_single_coil_channel_
    , msg_idx
    , function_code
    , block_id
    , mb_mapping_coil_address
    , output_value);

  return 0;
}

void modbus_master_node_t::slave_device_processing_fn()
{

  auto logger = spdlog::get(agent_logger_name_);

  // Some of the Lambdas to pack the modbus measurements.

  auto pack_mb_coil_response = [this, logger](int block, int start_address, int coil,  int value
                                                , const std::string& eu_factor, const std::string& eu_quantity, int eu_manifest_constant) {
    nlohmann::json jmeasurement = {};
    CTimeUtils ct;
    long long time_instance = ct.getTimestampInMilliSeconds();

    // Identity
    jmeasurement["controller_id"]   = controller_id_;
    jmeasurement["agent_id"]        = agent_id_;
    jmeasurement["node_id"]         = node_id_;

    // Measurement Mapping Type
    jmeasurement["message_type"]   = "coil_bit_reading";

    // Modbus Mapping
    jmeasurement["block_id"] = block;
    jmeasurement["coil_bits_start_address"] = start_address;
    jmeasurement["coil_bit_number"] = coil;
    jmeasurement["measured_value"]  = value;
    jmeasurement["eu_factor"]  = eu_factor;
    jmeasurement["eu_quantity"]  = eu_quantity;
    jmeasurement["eu_manifest_constant"]  = eu_manifest_constant;
    jmeasurement["time_instance"]   = time_instance;
    
    return jmeasurement;
  };

  auto pack_mb_input_bits_response = [this, logger](int block, int start_address, int input_bit,  int value
                                                      , const std::string& eu_factor, const std::string& eu_quantity, int eu_manifest_constant) {
    nlohmann::json jmeasurement = {};
    CTimeUtils ct;
    long long time_instance = ct.getTimestampInMilliSeconds();

    // Identity
    jmeasurement["controller_id"]   = controller_id_;
    jmeasurement["agent_id"]        = agent_id_;
    jmeasurement["node_id"]         = node_id_;

    // Measurement Mapping Type
    jmeasurement["message_type"]   = "input_bit_reading";

    // Modbus Mapping
    jmeasurement["block_id"] = block;
    jmeasurement["input_bits_start_address"] = start_address;
    jmeasurement["input_bit_number"] = input_bit;
    jmeasurement["measured_value"]  = value;
    jmeasurement["eu_factor"]  = eu_factor;
    jmeasurement["eu_quantity"]  = eu_quantity;
    jmeasurement["eu_manifest_constant"]  = eu_manifest_constant;
    jmeasurement["time_instance"]   = time_instance;

    return jmeasurement;
  };

  auto pack_mb_holding_register_reponse = [this, logger](int block, int start_address, int holding_reg,  int value
                                                          , const std::string& eu_factor, const std::string& eu_quantity, int eu_manifest_constant) {

    nlohmann::json jmeasurement = {};
    CTimeUtils ct;
    long long time_instance = ct.getTimestampInMilliSeconds();

    // Identity
    jmeasurement["controller_id"]   = controller_id_;
    jmeasurement["agent_id"]        = agent_id_;
    jmeasurement["node_id"]         = node_id_;

    // Measurement Mapping Type
    jmeasurement["message_type"]   = "holding_register_reading";

    // Modbus Mapping
    jmeasurement["block_id"] = block;
    jmeasurement["registers_start_address"] = start_address;
    jmeasurement["holding_register_number"] = holding_reg;
    jmeasurement["measured_value"]  = value;
    jmeasurement["eu_factor"]  = eu_factor;
    jmeasurement["eu_quantity"]  = eu_quantity;
    jmeasurement["eu_manifest_constant"]  = eu_manifest_constant;
    jmeasurement["time_instance"]   = time_instance;

    return jmeasurement;
  };

  auto pack_mb_input_register_response = [this, logger](int block, int start_address, int input_reg,  int value
                                                          , const std::string& eu_factor, const std::string& eu_quantity, int eu_manifest_constant) {

    nlohmann::json jmeasurement = {};
    CTimeUtils ct;
    long long time_instance = ct.getTimestampInMilliSeconds();

    // Identity
    jmeasurement["controller_id"]   = controller_id_;
    jmeasurement["agent_id"]        = agent_id_;
    jmeasurement["node_id"]         = node_id_;

    // Measurement Mapping Type
    jmeasurement["message_type"]   = "input_register_reading";

    // Modbus Mapping
    jmeasurement["block_id"] = block;
    jmeasurement["input_registers_start_address"] = start_address;
    jmeasurement["input_register_number"] = input_reg;
    jmeasurement["measured_value"]  = value;
    jmeasurement["eu_factor"]  = eu_factor;
    jmeasurement["eu_quantity"]  = eu_quantity;
    jmeasurement["eu_manifest_constant"]  = eu_manifest_constant;
    jmeasurement["time_instance"]   = time_instance;

    return jmeasurement;
  };

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
    , [this
      , &logger
      , &pack_mb_coil_response
      , &pack_mb_input_bits_response
      , &pack_mb_holding_register_reponse
  , &pack_mb_input_register_response](msg_modbus_poll_slave_device_events_t dev_poll_msg) {

    libmodbus_wrapper_t mb_conn(mb_slave_device_.mb_slave_address_
      , mb_slave_device_.mb_slave_port_
      , mb_slave_device_.mb_slave_id_
      , agent_logger_name_);

    modbus_t *ctx = mb_conn.modbus_new_tcp_wf();

    if(ctx == nullptr) {
      return;
    }

    // We have a Modbus/TCP connection with the slave device. //
    // Read Coils bits
    {
      auto v_mb_coil_blocks = mb_slave_device_.mb_mapping_coil_blocks_;

      for(auto coil_block : v_mb_coil_blocks) {
        int nb_bits = coil_block.second.mb_mapping_nb_bits_;

        // Read the coils only the block refresh interval does not override the device level refresh interval.
        if(((nb_bits > 0) && (nb_bits < VC_MODBUS_MAX_READ_BITS)) && (coil_block.second.mb_block_refresh_interval_ == 0)) {

          /* Allocate and initialize the memory to store the coil status */
          uint8_t *tab_bit = (uint8_t *) malloc(MODBUS_MAX_READ_BITS * sizeof(uint8_t));

          if(tab_bit == NULL) {
            logger->critical("tab_bit Memory allocation failed.");
            return;
          }
          
          memset(tab_bit, 0, MODBUS_MAX_READ_BITS * sizeof(uint8_t));

          auto destroy_tab_bit = [&] {
            if(tab_bit)
            {
              free(tab_bit);
              tab_bit = nullptr;
            }
          };
          on_scope_exit free_resources {destroy_tab_bit};
          
          // Clear any other previous stuff...
          int rc=  mb_conn.modbus_flush_wf()
            .modbus_read_bits_wf(coil_block.second.mb_mapping_bits_start_address_, nb_bits, tab_bit);

          if(rc == -1) {
            return;
          }

          for(int coil = 0; coil < nb_bits; coil++) {
            auto pack = pack_mb_coil_response(coil_block.first, coil_block.second.mb_mapping_bits_start_address_
                                                , coil,  tab_bit[coil]
                                                , coil_block.second.eu_factor_
                                                , coil_block.second.eu_quantity_
                                                , coil_block.second.eu_manifest_constant_);
            cpr::Response r = cpr::Post(cpr::Url{mb_slave_device_.measurement_pub_api_url_}
                , cpr::Body{pack.dump()}
            , cpr::Header{{"Content-Type", "application/json"}});
          }
        }
      }
    }
    // Read inputs bits
    {
      auto v_mb_input_bits_blocks = mb_slave_device_.mb_mapping_input_bit_blocks_;

      for(auto input_bit_block : v_mb_input_bits_blocks) {
        int nb_input_bits = input_bit_block.second.mb_mapping_nb_input_bits_;

        // Read the input bits only the block refresh interval does not override the device level refresh interval.
        if(((nb_input_bits > 0) && (nb_input_bits < VC_MODBUS_MAX_READ_BITS)) && (input_bit_block.second.mb_block_refresh_interval_ == 0)) {

          /* Allocate and initialize the memory to store the input bits status */
          uint8_t *tab_input_bit = (uint8_t *) malloc(MODBUS_MAX_READ_BITS * sizeof(uint8_t));

          if(tab_input_bit == NULL) {
            logger->critical("tab_input_bit Memory allocation failed.");
            return;
          }
          
          memset(tab_input_bit, 0, MODBUS_MAX_READ_BITS * sizeof(uint8_t));

          auto destroy_tab_input_bit = [&] {
            if(tab_input_bit)
            {
              free(tab_input_bit);
              tab_input_bit = nullptr;
            }
          };
          on_scope_exit free_resources {destroy_tab_input_bit};

          // Clear any other previous stuff...
          int rc=  mb_conn.modbus_flush_wf()
            .modbus_read_bits_wf(input_bit_block.second.mb_mapping_input_bits_start_address_, nb_input_bits, tab_input_bit);

          if(rc == -1) {
            return;
          }

          for(int ib = 0; ib < nb_input_bits; ib++) {
            auto pack = pack_mb_input_bits_response(input_bit_block.first
                , input_bit_block.second.mb_mapping_input_bits_start_address_
                , ib
                , tab_input_bit[ib]
                , input_bit_block.second.eu_factor_
                , input_bit_block.second.eu_quantity_
                , input_bit_block.second.eu_manifest_constant_);

            cpr::Response r = cpr::Post(cpr::Url{mb_slave_device_.measurement_pub_api_url_}
                , cpr::Body{pack.dump()}
            , cpr::Header{{"Content-Type", "application/json"}});
          }
        }
      }
    }
    // Read/Write Holing registers
    {
      auto v_mb_holding_register_blocks = mb_slave_device_.mb_mapping_holding_register_blocks_;

      for(auto holding_reg_block : v_mb_holding_register_blocks) {
        int nb_holding_regs = holding_reg_block.second.mb_mapping_nb_registers_;

        // Read the holding registers only the block refresh interval does not override the device level refresh interval.
        if(((nb_holding_regs > 0) && (nb_holding_regs < VC_MODBUS_MAX_READ_REGISTERS)) && (holding_reg_block.second.mb_block_refresh_interval_ == 0)) {

          /* Allocate and initialize the memory to store the holding registers */
          uint16_t *tab_reg = (uint16_t *) malloc(MODBUS_MAX_WR_READ_REGISTERS * sizeof(uint16_t));

          if(tab_reg == NULL) {
            logger->critical("tab_reg Memory allocation failed.");
            return;
          }
          
          memset(tab_reg, 0, MODBUS_MAX_WR_READ_REGISTERS * sizeof(uint16_t));

          auto destroy_tab_reg = [&] {
            if(tab_reg)
            {
              free(tab_reg);
              tab_reg = nullptr;
            }
          };
          on_scope_exit free_resources {destroy_tab_reg};

          // Clear any other previous stuff...
          int rc=  mb_conn.modbus_flush_wf()
            .modbus_read_registers_wf(holding_reg_block.second.mb_mapping_registers_start_address_, nb_holding_regs, tab_reg);

          if(rc == -1) {
            return;
          }

          for(int mb_holding_register_number = 0; mb_holding_register_number < nb_holding_regs; mb_holding_register_number++) {

            auto pack = pack_mb_holding_register_reponse(holding_reg_block.first
                , holding_reg_block.second.mb_mapping_registers_start_address_
                , mb_holding_register_number
                , tab_reg[mb_holding_register_number]
                , holding_reg_block.second.eu_factor_
                , holding_reg_block.second.eu_quantity_
                , holding_reg_block.second.eu_manifest_constant_);

            cpr::Response r = cpr::Post(cpr::Url{mb_slave_device_.measurement_pub_api_url_}
                , cpr::Body{pack.dump()}
            , cpr::Header{{"Content-Type", "application/json"}});
          }
        }
      }
    }
    // Read input registers
    {
      auto v_mb_input_register_blocks = mb_slave_device_.mb_mapping_input_register_blocks_;

      for(auto input_reg_block : v_mb_input_register_blocks) {
        int nb_input_regs = input_reg_block.second.mb_mapping_nb_input_registers_;

        // Read the input registers only the block refresh interval does not override the device level refresh interval.
        if(((nb_input_regs > 0) && (nb_input_regs < VC_MODBUS_MAX_READ_REGISTERS)) && (input_reg_block.second.mb_block_refresh_interval_ == 0))  {

          /* Allocate and initialize the memory to store the input registers */
          uint16_t *tab_input_reg = (uint16_t *) malloc(MODBUS_MAX_READ_REGISTERS * sizeof(uint16_t));

          if(tab_input_reg == NULL) {
            logger->critical("tab_input_reg Memory allocation failed.");
            return;
          }

          memset(tab_input_reg, 0, MODBUS_MAX_READ_REGISTERS * sizeof(uint16_t));

          auto destroy_tab_input_reg = [&] {
            if(tab_input_reg)
            {
              free(tab_input_reg);
              tab_input_reg = nullptr;
            }
          };
          on_scope_exit free_resources {destroy_tab_input_reg};

          // Clear any other previous stuff...
          int rc = mb_conn.modbus_flush_wf()
            .modbus_read_input_registers_wf(input_reg_block.second.mb_mapping_input_registers_start_address_, nb_input_regs, tab_input_reg);

          if(rc == -1) {
            return;
          }

          for(int mb_input_register_number=0; mb_input_register_number < nb_input_regs; mb_input_register_number++) {
            auto pack = pack_mb_input_register_response(input_reg_block.first
                , input_reg_block.second.mb_mapping_input_registers_start_address_
                , mb_input_register_number
                , tab_input_reg[mb_input_register_number]
                , input_reg_block.second.eu_factor_
                , input_reg_block.second.eu_quantity_
                , input_reg_block.second.eu_manifest_constant_);

            cpr::Response r = cpr::Post(cpr::Url{mb_slave_device_.measurement_pub_api_url_}
                , cpr::Body{pack.dump()}
            , cpr::Header{{"Content-Type", "application/json"}});
          }
        }
      }
    }
    return;
  }
  ),
  so_5::receive_case(mb_block_read_coils_channel_
  , [this, &logger, &pack_mb_coil_response](msg_modbus_block_read_request_t msg) {

    libmodbus_wrapper_t mb_conn(mb_slave_device_.mb_slave_address_
      , mb_slave_device_.mb_slave_port_
      , mb_slave_device_.mb_slave_id_
      , agent_logger_name_);

    modbus_t *ctx = mb_conn.modbus_new_tcp_wf();

    if(ctx == nullptr) {
      return;
    }

    // Read Coils bits
    {
      auto v_mb_coil_blocks = mb_slave_device_.mb_mapping_coil_blocks_;

      for(auto coil_block : v_mb_coil_blocks) {
        if(coil_block.first == msg.block_id_) {
          int nb_bits = coil_block.second.mb_mapping_nb_bits_;

          // Read the coils only the block refresh interval does not override the device level refresh interval.
          if((nb_bits > 0) && (coil_block.second.mb_block_refresh_interval_ != 0)) {

            /* Allocate and initialize the memory to store the coil status */
            uint8_t *tab_bit = (uint8_t *) malloc(MODBUS_MAX_READ_BITS * sizeof(uint8_t));

            if(tab_bit == NULL) {
              logger->critical("tab_bit Memory allocation failed.");
              return;
            }
            
            memset(tab_bit, 0, MODBUS_MAX_READ_BITS * sizeof(uint8_t));

            auto destroy_tab_bit = [&] {
              if(tab_bit)
              {
                free(tab_bit);
                tab_bit = nullptr;
              }
            };
            on_scope_exit free_resources {destroy_tab_bit};
            // Clear any other previous stuff...
            int rc=  mb_conn.modbus_flush_wf()
              .modbus_read_bits_wf(coil_block.second.mb_mapping_bits_start_address_, nb_bits, tab_bit);

            if(rc == -1) {
              return;
            }

            for(int coil = 0; coil < nb_bits; coil++) {
              auto pack = pack_mb_coil_response(coil_block.first, coil_block.second.mb_mapping_bits_start_address_
                                                  , coil,  tab_bit[coil]
                                                  , coil_block.second.eu_factor_
                                                  , coil_block.second.eu_quantity_
                                                  , coil_block.second.eu_manifest_constant_);
              cpr::Response r = cpr::Post(cpr::Url{mb_slave_device_.measurement_pub_api_url_}
                  , cpr::Body{pack.dump()}
              , cpr::Header{{"Content-Type", "application/json"}});
            }

          }

          return;
        }
      }
    }
  }
  ),
  so_5::receive_case(mb_block_read_input_bits_channel_
  , [this, &pack_mb_input_bits_response, &logger](msg_modbus_block_read_request_t msg) {

    libmodbus_wrapper_t mb_conn(mb_slave_device_.mb_slave_address_
      , mb_slave_device_.mb_slave_port_
      , mb_slave_device_.mb_slave_id_
      , agent_logger_name_);

    modbus_t *ctx = mb_conn.modbus_new_tcp_wf();

    if(ctx == nullptr) {
      return;
    }

    // Read inputs bits
    {
      auto v_mb_input_bits_blocks = mb_slave_device_.mb_mapping_input_bit_blocks_;

      for(auto input_bit_block : v_mb_input_bits_blocks) {
        if(input_bit_block.first == msg.block_id_) {
          int nb_input_bits = input_bit_block.second.mb_mapping_nb_input_bits_;

          // Read the input bits only the block refresh interval does not override the device level refresh interval.
          if((nb_input_bits > 0) && (input_bit_block.second.mb_block_refresh_interval_ != 0)) {

            /* Allocate and initialize the memory to store the input bits status */
            uint8_t *tab_input_bit = (uint8_t *) malloc(MODBUS_MAX_READ_BITS * sizeof(uint8_t));

            if(tab_input_bit == NULL) {
              logger->critical("tab_input_bit Memory allocation failed.");
              return;
            }
          
            memset(tab_input_bit, 0, MODBUS_MAX_READ_BITS * sizeof(uint8_t));

            auto destroy_tab_input_bit = [&] {
              if(tab_input_bit)
              {
                free(tab_input_bit);
                tab_input_bit = nullptr;
              }
            };
            on_scope_exit free_resources {destroy_tab_input_bit};

            // Clear any other previous stuff...
            int rc=  mb_conn.modbus_flush_wf()
              .modbus_read_bits_wf(input_bit_block.second.mb_mapping_input_bits_start_address_, nb_input_bits, tab_input_bit);

            if(rc == -1) {
              return;
            }

            for(int ib = 0; ib < nb_input_bits; ib++) {
              auto pack = pack_mb_input_bits_response(input_bit_block.first
                  , input_bit_block.second.mb_mapping_input_bits_start_address_
                  , ib
                  , tab_input_bit[ib]
                  , input_bit_block.second.eu_factor_
                  , input_bit_block.second.eu_quantity_
                  , input_bit_block.second.eu_manifest_constant_);

              cpr::Response r = cpr::Post(cpr::Url{mb_slave_device_.measurement_pub_api_url_}
                  , cpr::Body{pack.dump()}
              , cpr::Header{{"Content-Type", "application/json"}});
            }
          }

          return;
        }
      }
    }
    return;
  }
  ),
  so_5::receive_case(mb_block_read_holding_registers_channel_
  , [this, &pack_mb_holding_register_reponse, &logger](msg_modbus_block_read_request_t msg) {
    libmodbus_wrapper_t mb_conn(mb_slave_device_.mb_slave_address_
      , mb_slave_device_.mb_slave_port_
      , mb_slave_device_.mb_slave_id_
      , agent_logger_name_);

    modbus_t *ctx = mb_conn.modbus_new_tcp_wf();

    if(ctx == nullptr) {
      return;
    }

    // Read/Write Holing registers
    {
      auto v_mb_holding_register_blocks = mb_slave_device_.mb_mapping_holding_register_blocks_;

      for(auto holding_reg_block : v_mb_holding_register_blocks) {
        if(holding_reg_block.first == msg.block_id_) {
          int nb_holding_regs = holding_reg_block.second.mb_mapping_nb_registers_;

          // Read the holding registers only the block refresh interval does not override the device level refresh interval.
          if((nb_holding_regs > 0) && (holding_reg_block.second.mb_block_refresh_interval_ != 0)) {

            /* Allocate and initialize the memory to store the holding registers */
            uint16_t *tab_reg = (uint16_t *) malloc(MODBUS_MAX_WR_READ_REGISTERS * sizeof(uint16_t));

            if(tab_reg == NULL) {
              logger->critical("tab_reg Memory allocation failed.");
              return;
            }

            memset(tab_reg, 0, MODBUS_MAX_WR_READ_REGISTERS * sizeof(uint16_t));

            auto destroy_tab_reg = [&] {
              if(tab_reg)
              {
                free(tab_reg);
                tab_reg = nullptr;
              }
            };
            on_scope_exit free_resources {destroy_tab_reg};

            // Clear any other previous stuff...
            int rc=  mb_conn.modbus_flush_wf()
              .modbus_read_registers_wf(holding_reg_block.second.mb_mapping_registers_start_address_, nb_holding_regs, tab_reg);

            if(rc == -1) {
              return;
            }

            for(int mb_holding_register_number = 0; mb_holding_register_number < nb_holding_regs; mb_holding_register_number++) {

              auto pack = pack_mb_holding_register_reponse(holding_reg_block.first
                  , holding_reg_block.second.mb_mapping_registers_start_address_
                  , mb_holding_register_number
                  , tab_reg[mb_holding_register_number]
                  , holding_reg_block.second.eu_factor_
                  , holding_reg_block.second.eu_quantity_
                  , holding_reg_block.second.eu_manifest_constant_);

              cpr::Response r = cpr::Post(cpr::Url{mb_slave_device_.measurement_pub_api_url_}
                  , cpr::Body{pack.dump()}
              , cpr::Header{{"Content-Type", "application/json"}});
            }
          }
          return;
        }
      }
    }
    return;
  }
  ),
  so_5::receive_case(mb_block_read_input_registers_channel_
  , [this, &pack_mb_input_register_response, &logger](msg_modbus_block_read_request_t msg) {
    libmodbus_wrapper_t mb_conn(mb_slave_device_.mb_slave_address_
      , mb_slave_device_.mb_slave_port_
      , mb_slave_device_.mb_slave_id_
      , agent_logger_name_);

    modbus_t *ctx = mb_conn.modbus_new_tcp_wf();

    if(ctx == nullptr) {
      return;
    }

    // Read input registers
    {
      auto v_mb_input_register_blocks = mb_slave_device_.mb_mapping_input_register_blocks_;

      for(auto input_reg_block : v_mb_input_register_blocks) {
        if(input_reg_block.first == msg.block_id_) {

          int nb_input_regs = input_reg_block.second.mb_mapping_nb_input_registers_;

          // Read the input registers only the block refresh interval does not override the device level refresh interval.
          if((nb_input_regs > 0) && (input_reg_block.second.mb_block_refresh_interval_ != 0))  {

            /* Allocate and initialize the memory to store the input registers */
            uint16_t *tab_input_reg = (uint16_t *) malloc(MODBUS_MAX_READ_REGISTERS * sizeof(uint16_t));

            if(tab_input_reg == NULL) {
              logger->critical("tab_input_reg Memory allocation failed.");
              return;
            }
            
            memset(tab_input_reg, 0, MODBUS_MAX_READ_REGISTERS * sizeof(uint16_t));

            auto destroy_tab_input_reg = [&] {
              if(tab_input_reg)
              {
                free(tab_input_reg);
                tab_input_reg = nullptr;
              }
            };
            on_scope_exit free_resources {destroy_tab_input_reg};

            // Clear any other previous stuff...
            int rc = mb_conn.modbus_flush_wf()
              .modbus_read_input_registers_wf(input_reg_block.second.mb_mapping_input_registers_start_address_, nb_input_regs, tab_input_reg);

            if(rc == -1) {
              return;
            }

            for(int mb_input_register_number=0; mb_input_register_number < nb_input_regs; mb_input_register_number++) {
              auto pack = pack_mb_input_register_response(input_reg_block.first
                  , input_reg_block.second.mb_mapping_input_registers_start_address_
                  , mb_input_register_number
                  , tab_input_reg[mb_input_register_number]
                  , input_reg_block.second.eu_factor_
                  , input_reg_block.second.eu_quantity_
                  , input_reg_block.second.eu_manifest_constant_);

              cpr::Response r = cpr::Post(cpr::Url{mb_slave_device_.measurement_pub_api_url_}
                  , cpr::Body{pack.dump()}
              , cpr::Header{{"Content-Type", "application/json"}});
            }
          }
          return;
        }
      }
    }
    return;
  }
  ),
  // Writ single coil function
  so_5::receive_case(mb_device_write_single_coil_channel_
  , [this, &logger](msg_modbus_write_single_coil_request_t write_single_coil_msg) {

    libmodbus_wrapper_t mb_conn(mb_slave_device_.mb_slave_address_
      , mb_slave_device_.mb_slave_port_
      , mb_slave_device_.mb_slave_id_
      , agent_logger_name_);

    modbus_t *ctx = mb_conn.modbus_new_tcp_wf();

    if(ctx == nullptr) {
      return;
    }

    // Write Single Coil bit
    // Clear any other previous stuff...
    int rc = mb_conn.modbus_flush_wf()
      .modbus_write_bit_wf(write_single_coil_msg.write_coil_req_.mb_mapping_coil_address_
        , (write_single_coil_msg.write_coil_req_.output_value_ ? TRUE : FALSE));

    if(rc == 1) {
      logger->info("Succeeded Writing Single Coil {}/{}/{}: Block Id: {} Address: {} Output: {}."
        , controller_id_, agent_id_, node_id_, write_single_coil_msg.write_coil_req_.block_id_
        , write_single_coil_msg.write_coil_req_.mb_mapping_coil_address_
        , write_single_coil_msg.write_coil_req_.output_value_);

      // Send Success message back to the GUI ...
      return;
    }
    else if(rc == -1) {
      logger->critical("Failed to Write Single Coil {}/{}/{}: Block Id: {} Address: {} Output: {}."
        , controller_id_, agent_id_, node_id_, write_single_coil_msg.write_coil_req_.block_id_
        , write_single_coil_msg.write_coil_req_.mb_mapping_coil_address_
        , write_single_coil_msg.write_coil_req_.output_value_);

      // Send failure message back to the GUI ...
      return;
    }
    // The control should not reach here.
    else {
      return;
    }
  }
  ),
  so_5::receive_case(mb_secondary_channel_
  , [](msg_modbus_poll_slave_device_events_t) {
    // Do some stuff
  }
  )
  );

  do {
    so_5::select(prepared);
  }
  while(!stop);
}

modbus_master_node_t& modbus_master_node_t::init_modbus_node()
{

  if(mb_node_init_status_) {
    return *this;
  }

  auto logger = spdlog::get(agent_logger_name_);

  mb_slave_device_env_obj_ = std::make_shared<so_5::wrapped_env_t>();
  mb_primary_channel_ = so_5::create_mchain(*mb_slave_device_env_obj_);
  auto mb_primary_channel_closer = so_5::auto_close_drop_content(mb_primary_channel_);

  mb_secondary_channel_ = so_5::create_mchain(mb_primary_channel_->environment());
  auto mb_secondary_channel_closer = so_5::auto_close_drop_content(mb_secondary_channel_);

  mb_device_write_single_coil_channel_ = so_5::create_mchain(mb_primary_channel_->environment());
  auto mb_device_write_single_coil_channel_closer = so_5::auto_close_drop_content(mb_device_write_single_coil_channel_);


  mb_block_read_coils_channel_ = so_5::create_mchain(mb_primary_channel_->environment());
  auto mb_block_read_coils_channel_closer = so_5::auto_close_drop_content(mb_block_read_coils_channel_);

  mb_block_read_input_bits_channel_ = so_5::create_mchain(mb_primary_channel_->environment());
  auto mb_block_read_input_bits_channel_closer = so_5::auto_close_drop_content(mb_block_read_input_bits_channel_);

  mb_block_read_holding_registers_channel_  = so_5::create_mchain(mb_primary_channel_->environment());
  auto mb_block_read_holding_registers_channel_closer = so_5::auto_close_drop_content(mb_block_read_holding_registers_channel_);

  mb_block_read_input_registers_channel_ = so_5::create_mchain(mb_primary_channel_->environment());
  auto mb_block_read_input_registers_channel_closer = so_5::auto_close_drop_content(mb_block_read_input_registers_channel_);


  mb_mthread_ = std::make_shared<std::thread>([this]() {
    slave_device_processing_fn();
  });

  // Schedule a timer message based on mb_refresh_interval_ms_
  mb_poll_timer_ = so_5::send_periodic<msg_modbus_poll_slave_device_events_t>(
      mb_primary_channel_
      , std::chrono::milliseconds(mb_slave_device_.mb_refresh_interval_ms_) // Delay before trigging periodic timer.
      , std::chrono::milliseconds(mb_slave_device_.mb_refresh_interval_ms_));

  // Set up coil block timers
  auto v_mb_coil_blocks = mb_slave_device_.mb_mapping_coil_blocks_;

  for(auto block = v_mb_coil_blocks.begin(); block != v_mb_coil_blocks.end(); ++block) {

    // Read the coils only the block refresh interval does not override the device level refresh interval.
    if((block->second.mb_mapping_nb_bits_ > 0) && (block->second.mb_block_refresh_interval_ != 0)) {
      block->second.mb_poll_timer_ = so_5::send_periodic<msg_modbus_block_read_request_t>(
          mb_block_read_coils_channel_
          , std::chrono::milliseconds(block->second.mb_block_refresh_interval_) // Delay before trigging periodic timer.
          , std::chrono::milliseconds(block->second.mb_block_refresh_interval_)
          , (int)block->first);
    }
  }

  // Set up input bits block timers
  auto v_mb_input_bits_blocks = mb_slave_device_.mb_mapping_input_bit_blocks_;

  for(auto block = v_mb_input_bits_blocks.begin(); block != v_mb_input_bits_blocks.end(); ++block) {
    // Read the coils only the block refresh interval does not override the device level refresh interval.
    if((block->second.mb_mapping_nb_input_bits_ > 0) && (block->second.mb_block_refresh_interval_ != 0)) {
      block->second.mb_poll_timer_ = so_5::send_periodic<msg_modbus_block_read_request_t>(
          mb_block_read_input_bits_channel_
          , std::chrono::milliseconds(block->second.mb_block_refresh_interval_) // Delay before trigging periodic timer.
          , std::chrono::milliseconds(block->second.mb_block_refresh_interval_)
          , block->first);
    }
  }

  // Set up holding register block timers
  auto v_mb_holding_reg_blocks = mb_slave_device_.mb_mapping_holding_register_blocks_;

  for(auto block = v_mb_holding_reg_blocks.begin(); block != v_mb_holding_reg_blocks.end(); ++block) {
    // Read the coils only the block refresh interval does not override the device level refresh interval.
    if((block->second.mb_mapping_nb_registers_ > 0) && (block->second.mb_block_refresh_interval_ != 0)) {
      block->second.mb_poll_timer_ = so_5::send_periodic<msg_modbus_block_read_request_t>(
          mb_block_read_holding_registers_channel_
          , std::chrono::milliseconds(block->second.mb_block_refresh_interval_) // Delay before trigging periodic timer.
          , std::chrono::milliseconds(block->second.mb_block_refresh_interval_)
          , block->first);
    }
  }

  // Set up input register block timers
  auto v_mb_input_reg_blocks = mb_slave_device_.mb_mapping_input_register_blocks_;

  for(auto block = v_mb_input_reg_blocks.begin(); block != v_mb_input_reg_blocks.end(); ++block) {
    // Read the coils only the block refresh interval does not override the device level refresh interval.
    if((block->second.mb_mapping_nb_input_registers_ > 0) && (block->second.mb_block_refresh_interval_ != 0)) {
      block->second.mb_poll_timer_ = so_5::send_periodic<msg_modbus_block_read_request_t>(
          mb_block_read_input_registers_channel_
          , std::chrono::milliseconds(block->second.mb_block_refresh_interval_) // Delay before trigging periodic timer.
          , std::chrono::milliseconds(block->second.mb_block_refresh_interval_)
          , block->first);
    }
  }

  auto controller_joiner = so_5::auto_join(*mb_mthread_);

  // Now set the init status.
  mb_node_init_status_ = true;

  return *this;
}

modbus_master_node_t& modbus_master_node_t::prepare_modbus_node_events()
{
  auto logger = spdlog::get(agent_logger_name_);

  return (*this);
}

modbus_master_node_t& modbus_master_node_t::print_node()
{
  auto logger = spdlog::get(agent_logger_name_);
  CTimeUtils ct;
  long ms = ct.getTimestampInMilliSeconds();
  std::cout << " ========= " << ct.formatTime2Readable(ms)<< " =========== " <<  std::endl;
  std::cout << node_id_       << "/" << node_name_          << "/" << node_address_ << "/"
    << node_api_key_  << "/" << agent_logger_name_  << "/" << std::endl;

  return *this;
}

std::shared_ptr<std::vector<modbus_node_event_t>> modbus_master_node_t::get_modbus_node_events()
{
  auto logger = spdlog::get(agent_logger_name_);
  std::vector<modbus_node_event_t> node_events;
  node_events = node_events_;
  std::shared_ptr<std::vector<modbus_node_event_t>> evnts_sp = std::make_shared<std::vector<modbus_node_event_t>>(node_events);
  return evnts_sp;
}
