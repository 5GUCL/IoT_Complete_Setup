/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/


#include "cpp-base64/include/base64.h"
#include "sf-sys/utils/time_utils.h"
#include "node/modbus/modbus_server_node_controller.h"

modbus_server_node_controller_t::modbus_server_node_controller_t
(const std::string& ctrl_id, const std::string& controller_logger_name
  , const std::string& api_server_listen_address, int api_server_listen_port)
{
  controller_id_ = ctrl_id;
  controller_logger_name_ = controller_logger_name;
  api_server_listen_address_ = api_server_listen_address;
  api_server_listen_port_ = api_server_listen_port;
  primary_ch_msg_counter_= (0);
  secondary_ch_msg_counter_ = (0);
}

void modbus_server_node_controller_t::wrapper_fn()
{
  init_contorller();
  return;
}

/* Edge device manager worker / node agent event procssor function. */
void modbus_server_node_controller_t::agent_event_processor_fn()
{
  auto logger = spdlog::get(controller_logger_name_);
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
  , [](modbus_server_controller_mailbox_message_t) {
    // Do some stuff..
  }
  ),
  so_5::receive_case(msg_secondary_channel_
  , [](modbus_server_controller_mailbox_message_t) {
    // Do some stuff..
  }
  ),
  so_5::receive_case(http_post_channel_,
  [](modbus_server_controller_http_post_mailbox_message_t msg_obj) {
    // Do some stuff..
  })
  );

  do {
    so_5::select(prepared);
  }
  while(!stop);
}

void modbus_server_node_controller_t::node_api_and_agent_processor_fn()
{
  // Start the Websocket / REST API server
  std::cout << "Started the Websocket / REST API server for MODBUS ..." << std::endl;

  // Let's start with a single thread controller.
  api_workers_.resize(api_server_threads_);

  // Scale the controller with more threads.
  // api_workers_.resize(std::thread::hardware_concurrency());

  std::transform(api_workers_.begin(), api_workers_.end(), api_workers_.begin(), [this](modbus_server_controller_api_worker_t &w) {
    w.thread_ = std::make_shared<std::thread>([this, &w]() {
      w.all_api_workers_ = std::make_shared<std::vector<modbus_server_controller_api_worker_t>>(api_workers_);
      w.api_server_listen_address_ = api_server_listen_address_;
      w.api_server_listen_port_ = api_server_listen_port_;
      w.http_post_channel_ = http_post_channel_;
      w.controller_logger_name_ = controller_logger_name_;
      /* create uWebSocket worker and capture uWS::Loop, uWS::App objects. */
      w.work();
    });
    return w;
  });

  std::transform(mb_server_node_agents_vec_.begin(), mb_server_node_agents_vec_.end(), mb_server_node_agents_vec_.begin(), [](modbus_server_node_agent_t &w) {
    w.wrapper_mthread_ = std::make_shared<std::thread>([&w]() {
      w.init_agent();
    });
    return w;
  });

  std::for_each(api_workers_.begin(), api_workers_.end(), [](modbus_server_controller_api_worker_t &w) {
    w.thread_->join();
  });

  return;
}

void modbus_server_node_controller_t::init_contorller()
{

  auto logger = spdlog::get(controller_logger_name_);

  // Do some application specific initializations here.
  try {

    // Modbus Master config parameters
    mm_is_master_ = jnode_full_config_["controller"]["attributes"]["is_master"];
    mb_is_slave_  = jnode_full_config_["controller"]["attributes"]["is_slave"];

  }
  catch(nlohmann::json::exception& e) {
    // Log exception information
    std::stringstream ssReqException;
    ssReqException  << "message: " << e.what()    <<  " "
      << "exception id: " << e.id   << std::endl;
    logger->error("#modbus_node_controller_t/1 #init_contorller #exception {}",  ssReqException.str());

    // We should not continue with config errors.
    exit(0);
  }


  // Let's create two channels - Ch #1 -> Ch #2 -> Ch #1
  // Channel #1 -> process a request and post a message to channel #2
  // Then channel #2 -> process a request and post a message to channel #1
  mb_server_node_env_obj_ = std::make_shared<so_5::wrapped_env_t>();
  msg_primary_channel_ = so_5::create_mchain(*mb_server_node_env_obj_);
  auto msg_primary_channel_closer = so_5::auto_close_drop_content(msg_primary_channel_);

  msg_secondary_channel_ = so_5::create_mchain(msg_primary_channel_->environment());
  auto msg_secondary_channel_closer = so_5::auto_close_drop_content(msg_secondary_channel_);

  http_post_channel_ = so_5::create_mchain(msg_primary_channel_->environment());
  auto http_post_channel_closer = so_5::auto_close_drop_content(http_post_channel_);

  nthread_ = std::make_shared<std::thread>([this]() {
    agent_event_processor_fn();
  });
  so_5::send<modbus_server_controller_mailbox_message_t>(msg_primary_channel_);


  // Node controller provides REST API to interact with the Agents -> Nodes (actual transducers)
  node_api_and_agent_processor_fn();

  auto controller_joiner = so_5::auto_join(*nthread_);

  return;
}

// st_api_worker_t is a utility class that creates threads for API server of the Node controller.
/* uWebSocket worker thread function. */
void modbus_server_controller_api_worker_t::work()
{
  /* Every thread has its own Loop, and uWS::Loop::get() returns the Loop for current thread.*/
  loop_ = uWS::Loop::get();

  /* uWS::App object / instance is used in uWS::Loop::defer(lambda_function) */
  app_ =  std::make_shared<uWS::App>();

  app_->ws<modbus_server_controller_per_socket_data_t>("/*", {
    /* Settings */
    .compression = uWS::SHARED_COMPRESSOR,
    .maxPayloadLength = 16 * 1024 * 1024,
    .idleTimeout = 16,
    .maxBackpressure = 1 * 1024 * 1024,
    .closeOnBackpressureLimit = false,
    .resetIdleTimeoutOnSend = false,
    .sendPingsAutomatically = true,
    /* Handlers */
    .upgrade = nullptr,
    .open = [](auto *ws)
    {
      /* Let's make every connection subscribe to the "broadcast" topic.
         Since our example server runs multiple worker threads, the client connection
         may be served by any of the worker thread.
         However, this example demonstrates publishing message(s) to all subscribers
         served by all workers (threads).
      */
      ws->subscribe("broadcast");
    },
    .message = [this](auto *ws, std::string_view message, uWS::OpCode opCode)
    {
      /* Exit gracefully if we get a closedown message (ASAN debug) */
      if(message.compare("closedown") == 0) {
        /* Bye bye */
        us_listen_socket_close(0, listen_socket_);
        ws->close();
      }
      
    },
    .drain = [](auto *ws)
    {
      /* Check getBufferedAmount here */
    },
    .ping = [](auto */*ws*/, std::string_view)
    {
      /* Not implemented yet */
    },
    .pong = [](auto */*ws*/, std::string_view)
    {
      /* Not implemented yet */
    },
    .close = [](auto *ws, int code, std::string_view message)
    {
      /* We automatically unsubscribe from any topic here */
    }
  }
  ).listen(api_server_listen_port_, [this](auto *token) {
    listen_socket_ = token;

    if(listen_socket_) {
      std::cout << "Thread " << std::this_thread::get_id() << " listening on port " << api_server_listen_port_ << std::endl;
    }
    else {
      std::cout << "Thread " << std::this_thread::get_id() << " failed to listen on port " << api_server_listen_port_ << std::endl;
    }
  }).run();

  std::cout << "Thread " << std::this_thread::get_id() << " exiting" << std::endl;
}
