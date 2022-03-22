/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#include "h-data/hdf5/node/modbus/mb_node_master_hdf5_recorder.h"


mb_node_master_dataset_ws_client_t::mb_node_master_dataset_ws_client_t(const std::string& _url
  , std::shared_ptr<so_5::wrapped_env_t> ds_rec_env_obj
  , so_5::mchain_t& msg_primary_channel
  , so_5::mchain_t msg_secondary_channel)
  : controller_url_(_url)
  , ds_rec_env_obj_(ds_rec_env_obj)
  , msg_primary_channel_(msg_primary_channel)
  , msg_secondary_channel_(msg_secondary_channel)
{

}

std::string mb_node_master_dataset_ws_client_t::truncate(const std::string& str, size_t n)
{
  if(str.size() < n) {
    return str;
  }
  else {
    return str.substr(0, n) + "...";
  }
}

bool mb_node_master_dataset_ws_client_t::run()
{
  do {
    dataset_ws_.setUrl(controller_url_);

    dataset_ws_.setOnMessageCallback(
    [this](const ix::WebSocketMessagePtr& msg) {

      if(msg->type == ix::WebSocketMessageType::Open) {
        // Log the messages.

      }
      else if(msg->type == ix::WebSocketMessageType::Close) {
        // Log the messages.

        condition_.notify_one();
      }
      else if(msg->type == ix::WebSocketMessageType::Message) {

        so_5::send<dataset_recorder_mailbox_message_t>(msg_primary_channel_, std::move(msg->str));

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

    dataset_ws_.start();

    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock);
    //
    // DESIGN NOTES
    // Do some check if the socket needs to be be closed.
    dataset_ws_.stop();

    // Just hold the reconnection.
    // This may lead to some loss of messages.
    auto duration = std::chrono::seconds(0);
    std::this_thread::sleep_for(duration);
  }
  while(1);

  return true;
}