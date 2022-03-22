/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
 */
/*========================END LICENSE NOTICE========================*/

#include <string>
#include <sstream>

// Logging
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include "ncap_app_constants.h"
#include "ncap_device_auth_token.h"
#include "ncap_ws_data_source_subscriber.h"

std::string
ncap_ds_ws_subscriber_worker_t::truncate(const std::string& str, size_t n) {
  if(str.size() < n) {
    return str;
  }
  else {
    return str.substr(0, n) + "...";
  }
}

bool
ncap_ds_ws_subscriber_worker_t::subscribe() {
  auto logger = spdlog::get(NCAP_L21_LOGGER);
  std::mutex    mutex_;
  std::condition_variable condition_;

  // Websocket client subscribing to the IoT Data source.
  ix::WebSocket ncap_ds_ws_client_;

  std::stringstream ncap_data_source_url_;
  ncap_data_source_url_ << ds_sub_obj_.protocol_ << "://" << ds_sub_obj_.host_ << ":" << ds_sub_obj_.port_;

  // The WS Subscriber has all necessary info to connect to the data source.
  // This call doesn't return.
  do {
    ncap_ds_ws_client_.setUrl(ncap_data_source_url_.str());

    ncap_ds_ws_client_.setOnMessageCallback (
      [this, &condition_](const ix::WebSocketMessagePtr& msg) {
        if(msg->type == ix::WebSocketMessageType::Open) {
          // Log the messages.
        }
        else if(msg->type == ix::WebSocketMessageType::Close) {
          // Log the messages.
          condition_.notify_one();
        }
        else if(msg->type == ix::WebSocketMessageType::Message) {
          std::string ds_msg = msg->str;
          so_5::send<ncap_ds_env_mailbox_message_t>(msg_primary_channel_, std::move(ds_msg));
        }
        else if(msg->type == ix::WebSocketMessageType::Error) {
          // Log the messages.
          condition_.notify_one();
        }
        else if(msg->type == ix::WebSocketMessageType::Fragment) {
          // Log the messages.
        }
        else if(msg->type == ix::WebSocketMessageType::Ping) {
          // Log the messages.
        }
        else if(msg->type == ix::WebSocketMessageType::Pong) {
          // Log the messages.
        }
        else {
          // Log the messages.
        }
        // Log the message.
      }
    );

    SPDLOG_LOGGER_INFO(logger, "Connecting with the IoT Data source {} ...", ncap_data_source_url_.str());
    ncap_ds_ws_client_.start();

    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock);
    //
    // DESIGN NOTES
    // Do some check if the socket needs to be be closed.
    ncap_ds_ws_client_.stop();

    // Just hold the reconnection.
    // This may lead to some loss of messages.
    auto duration = std::chrono::seconds(3);
    std::this_thread::sleep_for(duration);

    SPDLOG_LOGGER_ERROR(logger, "Reconnecting with the IoT Data source {} ...", ncap_data_source_url_.str());

  } while(1);

  return true;
}

void
ncap_ds_ws_subscriber_worker_t::worker_thread_fn() {
  subscribe();
}
