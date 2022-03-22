/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _PM_ODP_NCAP_DEVICE_AUTH_TOKEN_H_
#define _PM_ODP_NCAP_DEVICE_AUTH_TOKEN_H_

#include <thread>
#include <vector>
#include <memory>
#include <iostream>     // std::ios, std::istream, std::cout
#include <fstream>      // std::filebuf
#include <sstream>
#include <map>

// SObjectizer is one of a few cross-platform and open source "actor frameworks" for C++.
// But SObjectizer supports not only Actor Model, but also Publish-Subscribe Model and CSP-like channels.
#include "so_5/all.hpp"

// JSON library
#include "nlohmann/json.hpp"

// Logging
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

// REST client
#include "cpr/cpr.h"

struct ncap_device_auth_token_t {
  ncap_device_auth_token_t () { /* Empty constructor*/ };
  std::string get_device_access_token ( const std::string& token_endpoint
                                          , const std::string& grant_type
                                          , const std::string& client_id
                                          , const std::string& client_secret
                                          , const std::string& username
                                          , const std::string& password );
  std::string get_device_access_token ( const std::string& token_endpoint
                                          , const std::string& username
                                          , const std::string& password );
};

#endif /* _PM_ODP_NCAP_DEVICE_AUTH_TOKEN_H_ */
