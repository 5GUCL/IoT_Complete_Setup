/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#include "ncap/network/mqtt/ncap_mqtt_backend.h"


mb_ncap_mqtt_ws_client_t::mb_ncap_mqtt_ws_client_t(const std::string& _url
  , std::shared_ptr<so_5::wrapped_env_t> ncap_mqtt_env_obj
  , so_5::mchain_t& msg_primary_channel
  , so_5::mchain_t msg_secondary_channel)
  : controller_url_(_url)
  , ncap_mqtt_env_obj_(ncap_mqtt_env_obj)
  , msg_primary_channel_(msg_primary_channel)
  , msg_secondary_channel_(msg_secondary_channel)
{

}

std::string mb_ncap_mqtt_ws_client_t::truncate(const std::string& str, size_t n)
{
  if(str.size() < n) {
    return str;
  }
  else {
    return str.substr(0, n) + "...";
  }
}

bool mb_ncap_mqtt_ws_client_t::run()
{
  do {
    ncap_mqtt_ws_.setUrl(controller_url_);

    ncap_mqtt_ws_.setOnMessageCallback(
    [this](const ix::WebSocketMessagePtr& msg) {

      if(msg->type == ix::WebSocketMessageType::Open) {
        // Log the messages.

      }
      else if(msg->type == ix::WebSocketMessageType::Close) {
        // Log the messages.

        condition_.notify_one();
      }
      else if(msg->type == ix::WebSocketMessageType::Message) {

        so_5::send<ncap_mqtt_mailbox_message_t>(msg_primary_channel_, std::move(msg->str));

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

    });

    ncap_mqtt_ws_.start();

    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock);
    //
    // DESIGN NOTES
    // Do some check if the socket needs to be be closed.
    ncap_mqtt_ws_.stop();

    // Just hold the reconnection.
    // This may lead to some loss of messages.
    auto duration = std::chrono::seconds(0);
    std::this_thread::sleep_for(duration);
  }
  while(1);

  return true;
}