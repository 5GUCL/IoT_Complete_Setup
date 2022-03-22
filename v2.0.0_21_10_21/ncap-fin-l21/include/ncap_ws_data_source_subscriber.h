/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _PM_ODP_NCAP_WS_DATA_SOURCE_SUBSCRIBER_H_
#define _PM_ODP_NCAP_WS_DATA_SOURCE_SUBSCRIBER_H_

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#include "ixwebsocket/IXSetThreadName.h"
#include "ixwebsocket/IXSocket.h"
#include "ixwebsocket/IXSocketOpenSSL.h"
#include "ixwebsocket/IXUserAgent.h"
#include "ixwebsocket/IXUuid.h"
#include "ixwebsocket/IXWebSocket.h"
#include "ixwebsocket/IXWebSocketHttpHeaders.h"

#include "so_5/all.hpp"
#include "nlohmann/json.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include "ncap_config_registry.h"

// Mailbox message data ...
struct ncap_ds_env_mailbox_message_t {
  std::string msg_;
};

struct ncap_ds_ws_subscriber_worker_t {
  ncap_ds_ws_subscriber_worker_t () { }
  
  std::string truncate(const std::string& str, size_t n);
  bool subscribe ();
  void worker_thread_fn();

  // To communicate Messages back to NCAP environment.
  std::shared_ptr<so_5::wrapped_env_t> ncap_env_obj_;
  so_5::mchain_t msg_primary_channel_;
  so_5::mchain_t msg_secondary_channel_;
  
  // NCAP ID.
  std::string ncap_id_;
  
  // OIDC/Identity for Device Auth, Access Tokens.
  ncap_auth_t auth_config_obj_;
  
  // IoT Datasource
  ncap_data_source_subscription_t ds_sub_obj_;
  
  // Worker Thread 
  std::shared_ptr<std::thread> ws_thread_;
};

#endif /* _PM_ODP_NCAP_WS_DATA_SOURCE_SUBSCRIBER_H_ */