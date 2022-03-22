/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
 */
/*========================END LICENSE NOTICE========================*/

#include <sstream>
// Logging
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include "ncap_app_constants.h"
#include "sf-sys/utils/time_utils.h"
#include "ncap_device_auth_token.h"
#include "ncap_mqtt_backend.h"
#include "cpp-base64/include/base64.h"


ncap_mqtt_backend_t::ncap_mqtt_backend_t (
                                            // NCAP MQTT Modbus config object
                                            std::shared_ptr<ncap_mqtt_modbus_config_t> ncap_mmb_obj
                                         )
                    : ncap_mmb_obj_(ncap_mmb_obj)
                    , mqtt_wss_client_context_sptr_(std::make_shared<mqtt_ws_tls_client_app_t>())
                    , mqtt_ws_client_context_sptr_(std::make_shared<mqtt_ws_client_app_t>())
                    , mutex_(std::make_shared<std::mutex> ())
                    , condition_(std::make_shared<std::condition_variable>())
{
  init ();
}

/*
 * Initialize the MQTT backend service.
 */
bool
ncap_mqtt_backend_t::init () {
  ncap_id_ = ncap_mmb_obj_->base_config_.id_;
  auth_config_obj_ = ncap_mmb_obj_->auth_config_;
  api_server_obj_ = ncap_mmb_obj_->api_server_config_;
  mqtt_obj_ = ncap_mmb_obj_->mqtt_config_;
  ds_sub_obj_ = ncap_mmb_obj_->iot_mb_config_.data_source_.subscription_;
  sdl_controller_id_ = ncap_mmb_obj_->iot_mb_config_.mb_payload_mapping_.contoller_id_;
  sdl_agent_id_ = ncap_mmb_obj_->iot_mb_config_.mb_payload_mapping_.agent_id_;
  sdl_node_id_ = ncap_mmb_obj_->iot_mb_config_.mb_payload_mapping_.node_id_;

  sdl_device_id_ = prepare_device_id(sdl_controller_id_, sdl_agent_id_, sdl_node_id_);

  lock_ = std::make_shared<std::unique_lock<std::mutex>>(*mutex_, std::defer_lock);
  return true;
}

/*
 * Run / Execute the MQTT backend service.
 * The Supervisor (a.k.a app) creates a wrapper thread (function: mmb_wrapper_thread_fn)
 */
void
ncap_mqtt_backend_t::mmb_wrapper_thread_fn () {
  // mmb_wrapper_thread_fn -> creates all threads for backend process.
  mmb_work ();
  return;
}

/*
 * Run / Execute the MQTT backend service.
 * mmb_wrapper_thread_fn -> creates all threads for backend process.
 */
void
ncap_mqtt_backend_t::mmb_work () {

  auto logger = spdlog::get(NCAP_L21_LOGGER);

  //
  // DESIGN NOTES
  // Now initialize the SObjectizer to process the received modbus messages.
  //
  ncap_ds_env_obj_ = std::make_shared<so_5::wrapped_env_t>();
  ds_msg_primary_channel_ = so_5::create_mchain(*ncap_ds_env_obj_);
  auto msg_primary_channel_closer = so_5::auto_close_drop_content(ds_msg_primary_channel_);

  ds_msg_secondary_channel_ = so_5::create_mchain(ds_msg_primary_channel_->environment());
  auto msg_secondary_channel_closer = so_5::auto_close_drop_content(ds_msg_secondary_channel_);

  ncap_ds_ws_subscriber_worker_t ws_sub_obj;
  ws_sub_obj.ncap_env_obj_ = ncap_ds_env_obj_;
  ws_sub_obj.msg_primary_channel_ = ds_msg_primary_channel_;
  ws_sub_obj.msg_secondary_channel_ = ds_msg_secondary_channel_;

  ws_sub_obj.ncap_id_ = ncap_id_;
  ws_sub_obj.auth_config_obj_ = auth_config_obj_;
  ws_sub_obj.ds_sub_obj_ = ds_sub_obj_;

  ncap_mqtt_ws_client_workers_vec_.emplace_back(std::move(ws_sub_obj));

  ////////////////////////////
  // MQTT Client Execution environment
  // Connects with the configured broker and maintains
  // connections etc.
  ////////////////////////////
  mqtt_client_env_obj_ = std::make_shared<so_5::wrapped_env_t>();
  mqtt_client_env_primary_channel_ = so_5::create_mchain(*mqtt_client_env_obj_);
  auto client_env_primary_channel_closer = so_5::auto_close_drop_content(mqtt_client_env_primary_channel_);

  mqtt_client_env_secondary_channel_ = so_5::create_mchain(mqtt_client_env_primary_channel_->environment());
  auto mqtt_client_env_secondary_channel_closer = so_5::auto_close_drop_content(mqtt_client_env_secondary_channel_);

  mqtt_tls_ws_client_thread_ = std::make_shared<std::thread>([this]() {
    if (mqtt_obj_.mqtt_borker_.secure_) {
      mqtt_wss_client_context_sptr_->c
        = MQTT_NS::make_tls_sync_client_ws (mqtt_wss_client_context_sptr_->ioc, mqtt_obj_.mqtt_borker_.host_, mqtt_obj_.mqtt_borker_.port_);
      mqtt_tls_ws_client_thread_fn(std::ref(mqtt_wss_client_context_sptr_));
    } else {
      mqtt_ws_client_context_sptr_->c
        = MQTT_NS::make_sync_client_ws (mqtt_ws_client_context_sptr_->ioc, mqtt_obj_.mqtt_borker_.host_, mqtt_obj_.mqtt_borker_.port_);
      mqtt_ws_client_thread_fn(std::ref(mqtt_ws_client_context_sptr_));
    }
  });

  // Wait for MQTT session to be established with the broker.
  SPDLOG_LOGGER_INFO(logger, "Waiting for MQTT session to be established with the broker.");
  condition_->wait(*lock_);
  SPDLOG_LOGGER_INFO(logger, "MQTT session established with the broker.");

  //
  // DESIGN NOTES
  // Now launch the SObjectizer to process the MQTT client activities.
  //
  mqtt_client_env_thread_ = std::make_shared<std::thread>([this]() {
    mqtt_client_env_proc_thread_fn();
  });

  //
  // DESIGN NOTES
  // Now launch the SObjectizer to process the received modbus messages.
  //
  mmb_ds_msg_proc_thread_ = std::make_shared<std::thread>([this]() {
    ncap_ds_message_proc_fn();
  });

  //
  // DESIGN NOTES
  // Now launch the Web Socket Client to connect with the local/or configured server
  // Provided node registry MAY have more than one MODBUS-MASTER CONTROLLERS
  // Each MODBUS-MASTER CONTROLLER published the data from configured master nodes.
  // [Master Controllers] -> [Publishes messages read from the SLAVE nodes]
  // Separate threads receive messages over Web Sockets for each MODBUS-MASTER CONTROLLER.
  //
  std::transform(ncap_mqtt_ws_client_workers_vec_.begin(), ncap_mqtt_ws_client_workers_vec_.end()
  , ncap_mqtt_ws_client_workers_vec_.begin(), [](ncap_ds_ws_subscriber_worker_t &s) {
    s.ws_thread_ = std::make_shared<std::thread>([&s]() {
      s.worker_thread_fn();
    });
    return s;
  });

  //
  // Join the SO environment and thread. This should be the last call and does not return.
  //
  auto ws_so_joiner = so_5::auto_join(*mmb_ds_msg_proc_thread_);
  auto mqtt_so_joiner = so_5::auto_join(*mqtt_client_env_thread_);

  return;
}

std::string
ncap_mqtt_backend_t::prepare_device_id ( const std::string& controller_id
                                           , const std::string& agent_id
                                           , const std::string& node_id ) {
  std::stringstream v_id;
  v_id << controller_id << "/" << agent_id << "/" << node_id;
  return v_id.str();
}

///////////////////////////////////////////////////////////
// Data source subscriber(s) post messages to this execution environment.
// Pre-processing of the data is done here. For example, calling expert systems
// rule engines, harmonisation, data sampling are a few examples.
void
ncap_mqtt_backend_t::ncap_ds_message_proc_fn()
{
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
    so_5::receive_case(ds_msg_primary_channel_
      , [this](ncap_ds_env_mailbox_message_t mb_msg) {
        //
        // DESIGN NOTES
        // Store the data received.
        // Device[identifier] -> Register [number] -> [measurement instamce]
        //

        try {
          nlohmann::json jmsg = nlohmann::json::parse(mb_msg.msg_);
          std::string _msg_type = jmsg["message_type"];

          ///////////////////////////////////////
          if(_msg_type.compare("coil_bit_reading") == 0) {

            std::string _contoller_id = jmsg["controller_id"];
            std::string _agent_id     = jmsg["agent_id"];
            std::string _node_id      = jmsg["node_id"];

            std::string v_id;
            v_id = prepare_device_id(_contoller_id, _agent_id, _node_id);

            // Is the incoming device ID is of our interest?
            // The NCAP protocol mapping has device id that is of our interest.
            if (sdl_device_id_.compare(v_id) != 0) {
              // The incloing device is of no interest to us.
              return;
            }

            // We found a device that the incoming message belongs to.
            modbus_master_coil_bit_single_measurement_instance_t cm;
            cm.mb_block_id_ = jmsg["block_id"];
            cm.mb_mapping_bits_start_address_ = jmsg["coil_bits_start_address"];
            cm.mb_mapping_bit_number_ = jmsg["coil_bit_number"];

            // Refer to Modbus Standard of bits, registers indexing.
            cm.mb_mapping_bit_number_ = cm.mb_mapping_bit_number_ + 1;

            cm.measured_value_  = jmsg["measured_value"];
            cm.time_instance_   = jmsg["time_instance"];
            cm.eu_factor_       = jmsg["eu_factor"];
            cm.eu_quantity_     = jmsg["eu_quantity"];
            cm.eu_manifest_constant_ = jmsg["eu_manifest_constant"];

            // Is this varibale (coil) of our interest?
            auto ctx = std::find_if(ncap_mmb_obj_->iot_mb_config_.mb_payload_mapping_.mb_mapping_coil_blocks_.begin()
                , ncap_mmb_obj_->iot_mb_config_.mb_payload_mapping_.mb_mapping_coil_blocks_.end()
                , [&](std::pair<mb_coil_block_number_tt, mb_coil_block_t> c) {
                  return (c.first == cm.mb_block_id_) ? true : false;
            });

            // The coil is found in the NCAP. It is of our interest. Send it to NCAP/Appa.
            if (ctx != ncap_mmb_obj_->iot_mb_config_.mb_payload_mapping_.mb_mapping_coil_blocks_.end()) {

              // Check if the address space of the block is of our interest?
              if (ctx->second.mb_mapping_bits_start_address_ == cm.mb_mapping_bits_start_address_
                  && ctx->second.mb_mapping_nb_bits_ >=  cm.mb_mapping_bit_number_) {
                  // Send the data to network (NCAP)
                  ncap_mb_coil_measurement
                  (v_id, cm.mb_block_id_, cm.mb_mapping_bits_start_address_, cm.mb_mapping_bit_number_, cm);
              }
            }
          }
          ///////////////////////////////////////
          else if(_msg_type.compare("input_bit_reading") == 0) {

            std::string _contoller_id = jmsg["controller_id"];
            std::string _agent_id     = jmsg["agent_id"];
            std::string _node_id      = jmsg["node_id"];

            std::string v_id;
            v_id = prepare_device_id(_contoller_id, _agent_id, _node_id);

            // Is the incoming device ID is of our interest?
            // The NCAP protocol mapping has device id that is of our interest.
            if (sdl_device_id_.compare(v_id) != 0) {
              // The incloing device is of no interest to us.
              return;
            }

            // We found a device that the incoming message belongs to.
            modbus_master_input_bit_single_measurement_instance_t ibm;
            ibm.mb_block_id_ = jmsg["block_id"];
            ibm.mb_mapping_input_bits_start_address_ = jmsg["input_bits_start_address"];
            ibm.mb_mapping_input_bit_number_ = jmsg["input_bit_number"];

            // Refer to Modbus Standard of bits, registers indexing.
            ibm.mb_mapping_input_bit_number_ = ibm.mb_mapping_input_bit_number_ + 1;

            ibm.measured_value_ = jmsg["measured_value"];
            ibm.time_instance_  = jmsg["time_instance"];

            ibm.eu_factor_    = jmsg["eu_factor"];
            ibm.eu_quantity_  = jmsg["eu_quantity"];
            ibm.eu_manifest_constant_ = jmsg["eu_manifest_constant"];

            // Is this varibale (input bit) of our interest?
            auto ctx = std::find_if(ncap_mmb_obj_->iot_mb_config_.mb_payload_mapping_.mb_mapping_input_bit_blocks_.begin()
                , ncap_mmb_obj_->iot_mb_config_.mb_payload_mapping_.mb_mapping_input_bit_blocks_.end()
                , [&](std::pair<mb_input_bit_block_number_tt, mb_input_bit_block_t> c) {
                  return (c.first == ibm.mb_block_id_) ? true : false;
            });

            // The input bit is found in the NCAP. It is of our interest. Send it to NCAP/Appa.
            if (ctx != ncap_mmb_obj_->iot_mb_config_.mb_payload_mapping_.mb_mapping_input_bit_blocks_.end()) {

              // Check if the address space of the block is of our interest?
              if (ctx->second.mb_mapping_input_bits_start_address_ == ibm.mb_mapping_input_bits_start_address_
                  && ctx->second.mb_mapping_nb_input_bits_ >=  ibm.mb_mapping_input_bit_number_) {
                  // Send the data to network (NCAP)
                  ncap_mb_input_bit_measurement
                  (v_id, ibm.mb_block_id_, ibm.mb_mapping_input_bits_start_address_, ibm.mb_mapping_input_bit_number_, ibm);
              }

            }

          }
          ////////////////////////////////////////
          else if(_msg_type.compare("holding_register_reading") == 0) {

            std::string _contoller_id = jmsg["controller_id"];
            std::string _agent_id     = jmsg["agent_id"];
            std::string _node_id      = jmsg["node_id"];

            std::string v_id;
            v_id = prepare_device_id(_contoller_id, _agent_id, _node_id);

            // Is the incoming device ID is of our interest?
            // The NCAP protocol mapping has device id that is of our interest.
            if (sdl_device_id_.compare(v_id) != 0) {
              // The incloing device is of no interest to us.
              return;
            }

            // We found a device that the incoming message belongs to.
            modbus_master_holding_register_single_measurement_instance_t hm;
            hm.mb_block_id_ = jmsg["block_id"];
            hm.mb_mapping_registers_start_address_ = jmsg["registers_start_address"];
            hm.mb_mapping_holding_register_number_ = jmsg["holding_register_number"];
            // Refer to Modbus Standard of bits, registers indexing.
            hm.mb_mapping_holding_register_number_ = hm.mb_mapping_holding_register_number_ + 1;

            hm.measured_value_ = jmsg["measured_value"];
            hm.time_instance_  = jmsg["time_instance"];

            hm.eu_factor_   = jmsg["eu_factor"];
            hm.eu_quantity_ = jmsg["eu_quantity"];
            hm.eu_manifest_constant_ = jmsg["eu_manifest_constant"];

            // Is this varibale (Holding Register) of our interest?
            auto ctx = std::find_if(ncap_mmb_obj_->iot_mb_config_.mb_payload_mapping_.mb_mapping_holding_register_blocks_.begin()
                , ncap_mmb_obj_->iot_mb_config_.mb_payload_mapping_.mb_mapping_holding_register_blocks_.end()
                , [&](std::pair<mb_holding_register_block_number_tt, mb_holding_register_block_t> c) {
                  return (c.first == hm.mb_block_id_) ? true : false;
            });

            // The holding register is found in the NCAP. It is of our interest. Send it to NCAP/Appa.
            if (ctx != ncap_mmb_obj_->iot_mb_config_.mb_payload_mapping_.mb_mapping_holding_register_blocks_.end()) {

              // Check if the address space of the block is of our interest?
              if (ctx->second.mb_mapping_registers_start_address_ == hm.mb_mapping_registers_start_address_
                  && ctx->second.mb_mapping_nb_registers_ >=  hm.mb_mapping_holding_register_number_) {
                  // Send the data to network (NCAP)
                  ncap_mb_holding_register_measurement
                  (v_id, hm.mb_block_id_, hm.mb_mapping_registers_start_address_, hm.mb_mapping_holding_register_number_, hm);
              }
            }
          }
          ////////////////////////////////////////
          else if(_msg_type.compare("input_register_reading") == 0) {

            std::string _contoller_id = jmsg["controller_id"];
            std::string _agent_id     = jmsg["agent_id"];
            std::string _node_id      = jmsg["node_id"];

            std::string v_id;
            v_id = prepare_device_id(_contoller_id, _agent_id, _node_id);

            // Is the incoming device ID is of our interest?
            // The NCAP protocol mapping has device id that is of our interest.
            if (sdl_device_id_.compare(v_id) != 0) {
              // The incloing device is of no interest to us.
              return;
            }

            // We found a device that the incoming message belongs to.
            modbus_master_input_register_single_measurement_instance_t im;
            im.mb_block_id_ = jmsg["block_id"];
            im.mb_mapping_input_registers_start_address_ = jmsg["input_registers_start_address"];
            im.mb_mapping_input_register_number_ = jmsg["input_register_number"];
            // Refer to Modbus Standard of bits, registers indexing.
            im.mb_mapping_input_register_number_ = im.mb_mapping_input_register_number_ + 1;
            im.measured_value_ = jmsg["measured_value"];
            im.time_instance_  = jmsg["time_instance"];

            im.eu_factor_   = jmsg["eu_factor"];
            im.eu_quantity_ = jmsg["eu_quantity"];
            im.eu_manifest_constant_ = jmsg["eu_manifest_constant"];

            // Is this varibale (Input Register) of our interest?
            auto ctx = std::find_if(ncap_mmb_obj_->iot_mb_config_.mb_payload_mapping_.mb_mapping_input_register_blocks_.begin()
                , ncap_mmb_obj_->iot_mb_config_.mb_payload_mapping_.mb_mapping_input_register_blocks_.end()
                , [&](std::pair<mb_input_register_block_number_tt, mb_input_register_block_t> c) {
                  return (c.first == im.mb_block_id_) ? true : false;
            });

            // The input register is found in the NCAP. It is of our interest. Send it to NCAP/Appa.
            if (ctx != ncap_mmb_obj_->iot_mb_config_.mb_payload_mapping_.mb_mapping_input_register_blocks_.end()) {

              // Check if the address space of the block is of our interest?
              if (ctx->second.mb_mapping_input_registers_start_address_ == im.mb_mapping_input_registers_start_address_
                  && ctx->second.mb_mapping_nb_input_registers_ >=  im.mb_mapping_input_register_number_) {
                  // Send the data to network (NCAP)
                  ncap_mb_input_register_measurement(v_id, im.mb_block_id_, im.mb_mapping_input_registers_start_address_, im.mb_mapping_input_register_number_, im);
              }
            }
          }
          else {
            std::cerr << "#receive_case #mb_msg.msg_received_ unsupported message type " << _msg_type << std::endl;
          }
        }
        catch(nlohmann::json::exception& e) {
          // Log exception information
          std::stringstream ssReqException;
          ssReqException  << "message: "      << e.what()   <<  " "
                          << "exception id: " << e.id       << std::endl;
          std::cerr << "#receive_case #mb_msg.msg_received_ ill-formed message" << mb_msg.msg_
                    << " received." << "Excepton: " << ssReqException.str() << std::endl;
        }
      }
    ),
    so_5::receive_case(ds_msg_secondary_channel_
      , [](ncap_ds_env_mailbox_message_t mb_msg) {
        // Placeholder to handle error messages, metric messages.
        std::cerr << "Received other message " << mb_msg.msg_ << " on secondary channel." << std::endl;
      }
    )
  );
  do {
    so_5::select(prepared);
  }
  while(!stop);
}

///////////////////////////////////////////////////////////
// MQTT CLIENT EXECUTION ENVIRONMENT.
void
ncap_mqtt_backend_t::mqtt_client_env_proc_thread_fn() {
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

    // Message handlers for MQTT Client -> Broker messages.
    so_5::receive_case(mqtt_client_env_primary_channel_
      , [this](ncap_mqtt_client_env_mailbox_message_t mb_msg) {
        try {
          // accessing mqtt client is generally not thread-safe,
          // but calling c->socket() and using the result to post() is OK
          // https://github.com/redboltz/mqtt_cpp/issues/838
          // https://gist.github.com/DavidAntliff/f6e600dc146516f64d60fd68660875bf

          std::string message_to_broker = mb_msg.msg_;

          nlohmann::json jmsg = nlohmann::json::parse(message_to_broker);
          std::string topic = jmsg["topic"];

          std::string b64_ = base64_encode(std::string(message_to_broker));

          int qos = jmsg["qos"];
          mqtt::qos qos_;
          if (qos == 0)      qos_ = mqtt::qos::at_most_once;
          else if (qos == 1) qos_ = mqtt::qos::at_least_once;
          else if (qos == 2) qos_ = mqtt::qos::exactly_once;
          else qos_= mqtt::qos::at_most_once; // Default QoS

          auto const& topic_ = topic;

          // The BASE64 encoded / JSON shall not exceed 2K chars
          char buff [MAX_CHAR_MESSAGE_LEN_TO_MQTT_BROKER + 1] = {};
          strncpy (buff, message_to_broker.c_str(), MAX_CHAR_MESSAGE_LEN_TO_MQTT_BROKER);

          auto const& buff_ = buff;

          if(mqtt_obj_.mqtt_borker_.secure_) {
            mqtt_wss_client_context_sptr_->c->socket()->post(
              [this, &buff_, &topic_, &qos_] {
                mqtt_wss_client_context_sptr_->c->publish ( MQTT_NS::buffer(MQTT_NS::string_view(topic_.data(), topic_.size()))
                  , MQTT_NS::buffer(MQTT_NS::string_view(buff_, strlen(buff_)))
                  , qos_ );
                /*
                mqtt_client_context_sptr_->c->publish ( std::move(topic_.c_str())
                  , std::move(buff_)
                  , qos_ );
                */
              }
            );
          } else {
            mqtt_ws_client_context_sptr_->c->socket()->post(
              [this, &buff_, &topic_, &qos_] {
                mqtt_ws_client_context_sptr_->c->publish ( MQTT_NS::buffer(MQTT_NS::string_view(topic_.data(), topic_.size()))
                  , MQTT_NS::buffer(MQTT_NS::string_view(buff_, strlen(buff_)))
                  , qos_ );
                /*
                mqtt_client_context_sptr_->c->publish ( std::move(topic_.c_str())
                  , std::move(buff_)
                  , qos_ );
                */
              }
            );
          }
        } catch(nlohmann::json::exception& e) {
          // Log exception information
          std::stringstream ssReqException;
          ssReqException  << "message: "      << e.what()   <<  " "
                          << "exception id: " << e.id       << std::endl;
          std::cerr << "#receive_case #mqtt_client_env_proc_thread_fn ill-formed message" << mb_msg.msg_
                    << " received." << "Excepton: " << ssReqException.str() << std::endl;
        } catch (std::exception const & e) {
          // Log Here.
          // Message is dropped!!
        }
      }
    ),
    so_5::receive_case(mqtt_client_env_secondary_channel_
      , [](ncap_mqtt_client_env_mailbox_message_t mb_msg) {
        // Placeholder to handle error messages, metric messages.
        std::cerr << "Received other message " << mb_msg.msg_ << " on secondary channel." << std::endl;
      }
    )
  );
  do {
    so_5::select(prepared);
  }
  while(!stop);
}

void
ncap_mqtt_backend_t::mqtt_tls_ws_client_thread_fn ( std::shared_ptr<mqtt_ws_tls_client_app_t> app ) {

  auto logger = spdlog::get(NCAP_L21_LOGGER);

  // Connect with MQTT broker
  MQTT_NS::setup_log();

  std::function<std::string()> get_access_token;
  get_access_token = [&] {
    ncap_device_auth_token_t auth_tk;

    std::stringstream token_url_ss;
    token_url_ss  << api_server_obj_.base_url_
                  << api_server_obj_.device_token_endpoint_;
    std::string access_token = auth_tk.get_device_access_token(token_url_ss.str()
                                                                , auth_config_obj_.username_
                                                                , auth_config_obj_.password_);
    return access_token;
  };

  using packet_id_t = typename std::remove_reference_t<decltype(*(app->c))>::packet_id_t;

  app->c->set_client_id(ncap_id_);
  app->c->set_clean_session(true);
  app->c->set_user_name(auth_config_obj_.username_);
  app->c->set_password(get_access_token());

  /*
  // This is a hack to by-pass the SSL cert verification.
  // Use only when you know what you are doing:-)

  c->get_ssl_context().set_verify_callback(
      [&](bool&, boost::asio::ssl::verify_context&) {
          return  true;
      }
  );
  */

  // Choose the CA Certificate for Let's Encrypt.

  // https://letsencrypt.org/certificates/
  // Our RSA intermediates are signed by ISRG Root X1. ISRG Root X1 is widely trusted at this point,
  // but our RSA intermediates are still cross-signed by IdenTrust’s “DST Root CA X3” (now called
  // “TrustID X3 Root”) for additional client compatibility. The IdenTrust root has been around longer
  // and thus has better compatibility with older devices and operating systems (e.g. Windows XP, Android 7).
  // You can download “TrustID X3 Root” from IdenTrust (or, alternatively, you can download a copy from us).
  // https://letsencrypt.org/certs/trustid-x3-root.pem.txt

  // The CA cert is obtained from https://letsencrypt.org/certs/trustid-x3-root.pem.txt
  // and configured (path, file) in the NCAP configuration.
  app->c->get_ssl_context().load_verify_file(mqtt_obj_.mqtt_borker_.ca_cert_);

#if OPENSSL_VERSION_NUMBER >= 0x10101000L

  SSL_CTX_set_keylog_callback(
      app->c->get_ssl_context().native_handle(),
      [](SSL const*, char const* line) {
          std::cout << line << std::endl;
      }
  );

#endif // OPENSSL_VERSION_NUMBER >= 0x10101000L

  app->c->set_connack_handler(
    [this, &c = app->c](bool sp, mqtt::connect_return_code connack_return_code){
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
      std::cout << "Connack Return Code: " << connack_return_code << std::endl;
#endif
      condition_->notify_one();
      return true;
    }
  );

  // If NCAP <-> MQTT is lost, try reconnecting.
  // https://github.com/redboltz/mqtt_cpp/wiki/reconnect
  boost::asio::steady_timer tim(app->ioc);
  std::function<void()> reconnect;
  reconnect =
    [&] {
      // wait 3 seconds and connect again
      tim.expires_after(std::chrono::seconds(3));
      tim.async_wait(
        [&]
        (boost::system::error_code const& ec) {
          if (!ec) {
            // timer fired
#ifdef PM_FIN_L21_ENABLE_CONSOLE_MESSAGES
            std::cout << "try connect again" << std::endl;
#endif
            mqtt::error_code ec;
            app->c->set_password(get_access_token());
            app->c->connect(ec); // connect again
            if (ec) {
#ifdef PM_FIN_L21_ENABLE_CONSOLE_MESSAGES
              std::cout << "error " << ec.message() << std::endl;
#endif
              reconnect();
            }
          }
        }
      );
    };

  app->c->set_close_handler( [this, &logger, &reconnect]() {
#ifdef PM_FIN_L21_ENABLE_CONSOLE_MESSAGES
      std::cout << "Connection closed." << std::endl;
#endif
      SPDLOG_LOGGER_ERROR(logger, "Reconnecting to the MQTT Broker Host:{}, Port:{}", mqtt_obj_.mqtt_borker_.host_, mqtt_obj_.mqtt_borker_.port_);
      reconnect();
    }
  );
  app->c->set_error_handler(
    [this, &logger, &reconnect](boost::system::error_code const & ec) {
#ifdef PM_FIN_L21_ENABLE_CONSOLE_MESSAGES
      std::cout << "error: " << ec.message() << std::endl;
#endif
      SPDLOG_LOGGER_ERROR(logger, "Reconnecting to the MQTT Broker Host:{}, Port:{}", mqtt_obj_.mqtt_borker_.host_, mqtt_obj_.mqtt_borker_.port_);
      reconnect();
    }
  );
  app->c->set_puback_handler(
    [&]
    (packet_id_t packet_id){
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
      std::cout << "puback received. packet_id: " << packet_id << std::endl;
#endif
      return true;
    });
  app->c->set_pubrec_handler(
    [&]
    (packet_id_t packet_id){
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
      std::cout << "pubrec received. packet_id: " << packet_id << std::endl;
#endif
      return true;
    });
  app->c->set_pubcomp_handler(
    [&]
    (packet_id_t packet_id){
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
      std::cout << "pubcomp received. packet_id: " << packet_id << std::endl;
#endif
      return true;
    });
  app->c->set_suback_handler(
    [&c = app->c](packet_id_t packet_id, std::vector<MQTT_NS::suback_return_code> results){
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
      std::cout << "suback received. packet_id: " << packet_id << std::endl;
#endif
      for (auto const& e : results) {
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
        std::cout << "subscribe result: " << e << std::endl;
#endif
      }
      return true;
    }
  );
  app->c->set_publish_handler(
    [&c = app->c]
    ( mqtt::optional<std::uint16_t> packet_id,
      mqtt::publish_options pubopts,
      mqtt::buffer topic_name,
      mqtt::buffer contents ) {
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
        std::cout << "publish received."
                  << " dup: "    << pubopts.get_dup()
                  << " qos: "    << pubopts.get_qos()
                  << " retain: " << pubopts.get_retain() << std::endl;
#endif
        if (packet_id) {
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
          std::cout << "packet_id: " << *packet_id << std::endl;
#endif
        }
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
        std::cout << "topic_name: " << topic_name << std::endl;
        std::cout << "contents: " << contents << std::endl;
#endif
        return true;
      }
  );
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
  std::cout << "try connect" << std::endl;
#endif

  mqtt::error_code ec;
  app->c->connect(ec);
  if (ec) {
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
    std::cout << "error " << ec.message() << std::endl;
#endif
    SPDLOG_LOGGER_ERROR(logger, "Reconnecting to the MQTT Broker Host:{}, Port:{}", mqtt_obj_.mqtt_borker_.host_, mqtt_obj_.mqtt_borker_.port_);
    reconnect();
  }

  app->ioc.run();

  return;
}

void
ncap_mqtt_backend_t::mqtt_ws_client_thread_fn ( std::shared_ptr<mqtt_ws_client_app_t> app ) {
  auto logger = spdlog::get(NCAP_L21_LOGGER);
  // Connect with MQTT broker
  MQTT_NS::setup_log();

  std::function<std::string()> get_access_token;
  get_access_token = [&] {
    ncap_device_auth_token_t auth_tk;

    std::stringstream token_url_ss;
    token_url_ss  << api_server_obj_.base_url_
                  << api_server_obj_.device_token_endpoint_;
    std::string access_token = auth_tk.get_device_access_token(token_url_ss.str()
                                                                , auth_config_obj_.username_
                                                                , auth_config_obj_.password_);
    return access_token;
  };

  using packet_id_t = typename std::remove_reference_t<decltype(*(app->c))>::packet_id_t;

  app->c->set_client_id(ncap_id_);
  app->c->set_clean_session(true);
  app->c->set_user_name(auth_config_obj_.username_);
  app->c->set_password(get_access_token());

  app->c->set_connack_handler(
    [this, &c = app->c](bool sp, mqtt::connect_return_code connack_return_code){
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
      std::cout << "Connack Return Code: " << connack_return_code << std::endl;
#endif
      condition_->notify_one();
      return true;
    }
  );

  // If NCAP <-> MQTT is lost, try reconnecting.
  // https://github.com/redboltz/mqtt_cpp/wiki/reconnect
  boost::asio::steady_timer tim(app->ioc);
  std::function<void()> reconnect;
  reconnect =
    [&] {
      // wait 3 seconds and connect again
      tim.expires_after(std::chrono::seconds(3));
      tim.async_wait(
        [&]
        (boost::system::error_code const& ec) {
          if (!ec) {
            // timer fired
#ifdef PM_FIN_L21_ENABLE_CONSOLE_MESSAGES
            std::cout << "try connect again" << std::endl;
#endif
            mqtt::error_code ec;
            app->c->set_password(get_access_token());
            app->c->connect(ec); // connect again
            if (ec) {
#ifdef PM_FIN_L21_ENABLE_CONSOLE_MESSAGES
              std::cout << "error " << ec.message() << std::endl;
#endif
              reconnect();
            }
          }
        }
      );
    };

  app->c->set_close_handler( [this, &logger, &reconnect]() {
#ifdef PM_FIN_L21_ENABLE_CONSOLE_MESSAGES
      std::cout << "Connection closed." << std::endl;
#endif
      SPDLOG_LOGGER_ERROR(logger, "Reconnecting to the MQTT Broker Host:{}, Port:{}", mqtt_obj_.mqtt_borker_.host_, mqtt_obj_.mqtt_borker_.port_);
      reconnect();
    }
  );
  app->c->set_error_handler(
    [this, &logger, &reconnect](boost::system::error_code const & ec) {
#ifdef PM_FIN_L21_ENABLE_CONSOLE_MESSAGES
      std::cout << "error: " << ec.message() << std::endl;
#endif
      SPDLOG_LOGGER_ERROR(logger, "Reconnecting to the MQTT Broker Host:{}, Port:{}", mqtt_obj_.mqtt_borker_.host_, mqtt_obj_.mqtt_borker_.port_);
      reconnect();
    }
  );
  app->c->set_puback_handler(
    [&]
    (packet_id_t packet_id){
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
      std::cout << "puback received. packet_id: " << packet_id << std::endl;
#endif
      return true;
    });
  app->c->set_pubrec_handler(
    [&]
    (packet_id_t packet_id){
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
      std::cout << "pubrec received. packet_id: " << packet_id << std::endl;
#endif
      return true;
    });
  app->c->set_pubcomp_handler(
    [&]
    (packet_id_t packet_id){
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
      std::cout << "pubcomp received. packet_id: " << packet_id << std::endl;
#endif
      return true;
    });
  app->c->set_suback_handler(
    [&c = app->c](packet_id_t packet_id, std::vector<MQTT_NS::suback_return_code> results){
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
      std::cout << "suback received. packet_id: " << packet_id << std::endl;
#endif
      for (auto const& e : results) {
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
        std::cout << "subscribe result: " << e << std::endl;
#endif
      }
      return true;
    }
  );
  app->c->set_publish_handler(
    [&c = app->c]
    ( mqtt::optional<std::uint16_t> packet_id,
      mqtt::publish_options pubopts,
      mqtt::buffer topic_name,
      mqtt::buffer contents ) {
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
        std::cout << "publish received."
                << " dup: "    << pubopts.get_dup()
                << " qos: "    << pubopts.get_qos()
                << " retain: " << pubopts.get_retain() << std::endl;
#endif
        if (packet_id) {
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
          std::cout << "packet_id: " << *packet_id << std::endl;
#endif
        }
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
        std::cout << "topic_name: " << topic_name << std::endl;
        std::cout << "contents: " << contents << std::endl;
#endif
        return true;
      }
  );
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
  std::cout << "try connect" << std::endl;
#endif

  mqtt::error_code ec;
  app->c->connect(ec);
  if (ec) {
#ifdef PM_FIN_L21_ENABLE_MQTT_CONSOLE_MESSAGES
    std::cout << "error " << ec.message() << std::endl;
#endif
    SPDLOG_LOGGER_ERROR(logger, "Reconnecting to the MQTT Broker Host:{}, Port:{}", mqtt_obj_.mqtt_borker_.host_, mqtt_obj_.mqtt_borker_.port_);
    reconnect();
  }

  app->ioc.run();

  return;
}
