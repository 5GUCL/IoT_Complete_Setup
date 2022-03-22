/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_MB_NODE_MASTER_GUI_IMGUI_WS_CLIENT_H_
#define _CS_EDM_MB_NODE_MASTER_GUI_IMGUI_WS_CLIENT_H_

#include <mutex>
#include <atomic>
#include <condition_variable>
#include <string>
#include <sstream>

#include "ixwebsocket/IXSetThreadName.h"
#include "ixwebsocket/IXSocket.h"
#include "ixwebsocket/IXSocketOpenSSL.h"
#include "ixwebsocket/IXUserAgent.h"
#include "ixwebsocket/IXUuid.h"
#include "ixwebsocket/IXWebSocket.h"
#include "ixwebsocket/IXWebSocketHttpHeaders.h"

// SObjectizer is one of a few cross-platform and open source "actor frameworks" for C++.
// But SObjectizer supports not only Actor Model, but also Publish-Subscribe Model and CSP-like channels.
#include <so_5/all.hpp>

// JSON library
#include "nlohmann/json.hpp"

// Logging
#include "spdlog/spdlog.h"

struct mb_node_master_gui_ws_client_t {
  mb_node_master_gui_ws_client_t(const std::string& _url
    , std::shared_ptr<so_5::wrapped_env_t> mb_master_gui_env_obj
    , so_5::mchain_t& msg_primary_channel
    , so_5::mchain_t msg_secondary_channel);

  std::string truncate(const std::string& str, size_t n);
  bool run();

  std::string   controller_url_;
  ix::WebSocket app_gui_ws_;

  std::mutex    mutex_;
  std::condition_variable condition_;

  std::shared_ptr<so_5::wrapped_env_t> mb_master_gui_env_obj_;
  so_5::mchain_t msg_primary_channel_;
  so_5::mchain_t msg_secondary_channel_;

};


#endif /* _CS_EDM_MB_NODE_MASTER_GUI_IMGUI_WS_CLIENT_H_ */