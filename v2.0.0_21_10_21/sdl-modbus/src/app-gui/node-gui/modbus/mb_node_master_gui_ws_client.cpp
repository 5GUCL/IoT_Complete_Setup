/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#include "app-gui/node-gui/modbus/mb_node_master_imgui_main.h"
#include "app-gui/node-gui/modbus/mb_node_master_gui_ws_client.h"

mb_node_master_gui_ws_client_t::mb_node_master_gui_ws_client_t
(const std::string& _url
  , std::shared_ptr<so_5::wrapped_env_t> mb_master_gui_env_obj
  , so_5::mchain_t& msg_primary_channel
  , so_5::mchain_t msg_secondary_channel)
{
  controller_url_         = _url;
  mb_master_gui_env_obj_  = mb_master_gui_env_obj;
  msg_primary_channel_    = msg_primary_channel;
  msg_secondary_channel_  = msg_secondary_channel;
}

std::string mb_node_master_gui_ws_client_t::truncate(const std::string& str, size_t n)
{
  if(str.size() < n) {
    return str;
  }
  else {
    return str.substr(0, n) + "...";
  }
}

bool mb_node_master_gui_ws_client_t::run()
{
  do {
    app_gui_ws_.setUrl(controller_url_);

    app_gui_ws_.setOnMessageCallback(
    [this](const ix::WebSocketMessagePtr& msg) {

      if(msg->type == ix::WebSocketMessageType::Open) {
        std::stringstream ss;
        ss << "Uri: " << msg->openInfo.uri << std::endl;
        ss << "Handshake Headers:" << std::endl;

        for(auto it : msg->openInfo.headers) {
          ss << it.first << ": " << it.second << std::endl;
        }
        std::string str = ss.str();
        // Post the message to "secondary channel" of recorder.
        so_5::send<modbus_master_gui_mailbox_message_t>(msg_secondary_channel_, std::move(str));

      }
      else if(msg->type == ix::WebSocketMessageType::Close) {
        std::stringstream ss;
        ss << "WS connection closed:";
        ss << " code " << msg->closeInfo.code;
        ss << " reason " << msg->closeInfo.reason << std::endl;
        std::string str = ss.str();
        // Post the message to "secondary channel" of recorder.
        so_5::send<modbus_master_gui_mailbox_message_t>(msg_secondary_channel_, std::move(str));

        condition_.notify_one();
      }
      else if(msg->type == ix::WebSocketMessageType::Message) {
        // Data. Post the data to the primary channel of the recoder.
        std::string str = msg->str;
        so_5::send<modbus_master_gui_mailbox_message_t>(msg_primary_channel_, std::move(str));

      }
      else if(msg->type == ix::WebSocketMessageType::Error) {
        std::stringstream ss;
        ss << "Connection error: " << msg->errorInfo.reason << std::endl;
        ss << "#retries: " << msg->errorInfo.retries << std::endl;
        ss << "Wait time(ms): " << msg->errorInfo.wait_time << std::endl;
        ss << "HTTP Status: " << msg->errorInfo.http_status << std::endl;
        std::string str = ss.str();
        // Post the message to "secondary channel" of recorder.
        so_5::send<modbus_master_gui_mailbox_message_t>(msg_secondary_channel_, std::move(str));

        // And error can happen, in which case the test-case is marked done
        condition_.notify_one();
      }
      else if(msg->type == ix::WebSocketMessageType::Fragment) {
        // NOP
      }
      else if(msg->type == ix::WebSocketMessageType::Ping) {
        // NOP
      }
      else if(msg->type == ix::WebSocketMessageType::Pong) {
        // NOP
      }
      else {
        std::stringstream ss;
        ss << "Invalid ix::WebSocketMessageType receive!";
        std::string str = ss.str();
        // Post the message to "secondary channel" of recorder.
        so_5::send<modbus_master_gui_mailbox_message_t>(msg_secondary_channel_, std::move(str));
      }

    });

    app_gui_ws_.start();

    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock);
    //
    // DESIGN NOTES
    // Do some check if the socket needs to be be closed.
    app_gui_ws_.stop();

    // Just hold the reconnection.
    // This may lead to some loss of messages.
    auto duration = std::chrono::seconds(0);
    std::this_thread::sleep_for(duration);

    std::stringstream ss;
    ss << "Reconnecting WS of " << controller_url_ ;
    std::string str = ss.str();
    // Post the message to "secondary channel" of recorder.
    so_5::send<modbus_master_gui_mailbox_message_t>(msg_secondary_channel_, std::move(str));

  }
  while(1);

  return true;
}