/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
 */
/*========================END LICENSE NOTICE========================*/

// Logging
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include "ncap_app_constants.h"
#include "ncap_device_auth_token.h"

/*
 * Returns access token (JWT) from the response to "password grant type".
 * Empty string if access token is not granted by the OIDC provider.
 * Or a valid token. However, caller shall validate it before using the acces JWT.
*/
std::string
ncap_device_auth_token_t::get_device_access_token( const std::string& token_endpoint
                                                     , const std::string& grant_type
                                                     , const std::string& client_id
                                                     , const std::string& client_secret
                                                     , const std::string& username
                                                     , const std::string& password )
{
  auto logger = spdlog::get(NCAP_L21_LOGGER);

  std::string device_acces_token;

  cpr::Response r = cpr::Post( cpr::Url{token_endpoint}
                                 ,cpr::Payload{
                                                {"grant_type", grant_type}
                                                , {"client_id", client_id}
                                                , {"client_secret", client_secret}
                                                , {"username", username}
                                                , {"password", password}
                                              }
                                 ,cpr::Header{{"Content-Type", "application/x-www-form-urlencoded"}});
  try {
    // Let's try to obtain the access token only if the OIDC provider returned a success code.
    if (r.status_code == 200) {
      auto oidc_auth_token = nlohmann::json::parse(r.text);
      device_acces_token = oidc_auth_token["access_token"];
    } else {
      // OIDC provider did not return auth token.
      device_acces_token.clear();
      SPDLOG_LOGGER_ERROR(logger, "OIDC provider did not return auth token for the user {}", username);
    }
  } catch(nlohmann::json::exception& e) {

    // Could not parse the token response body. Let's make in empty and return to the caller.
    // Caller shall check the token length before using.
    device_acces_token.clear();

    // Log exception information
    std::stringstream ssReqException;
    ssReqException  << "message: "        << e.what()  <<  " "
                    << "exception id: "   << e.id      << std::endl;

    std::cerr << " " << ssReqException.str() << std::endl;
    SPDLOG_LOGGER_ERROR(logger, "ncap_device_auth_token_t::get_device_access_token bad response {}", username);
  }

  return device_acces_token;
}

std::string
ncap_device_auth_token_t::get_device_access_token ( const std::string& token_endpoint
                                                      , const std::string& username
                                                      , const std::string& password )
{
  auto logger = spdlog::get(NCAP_L21_LOGGER);

  std::string device_acces_token;

  cpr::Response r = cpr::Post( cpr::Url{token_endpoint}
                                 ,cpr::Payload{
                                                {"username", username}
                                                , {"password", password}
                                              }
                                 ,cpr::Header{{"Content-Type", "application/x-www-form-urlencoded"}});
  try {
    // Let's try to obtain the access token only if the OIDC provider returned a success code.
    if (r.status_code == 200) {
      auto oidc_auth_token = nlohmann::json::parse(r.text);
      device_acces_token = oidc_auth_token["access_token"];
    } else {
      // OIDC provider did not return auth token.
      device_acces_token.clear();
      SPDLOG_LOGGER_ERROR(logger, "OIDC provider did not return auth token for the user {}", username);
    }
  } catch(nlohmann::json::exception& e) {

    // Could not parse the token response body. Let's make in empty and return to the caller.
    // Caller shall check the token length before using.
    device_acces_token.clear();

    // Log exception information
    std::stringstream ssReqException;
    ssReqException  << "message: "        << e.what()  <<  " "
                    << "exception id: "   << e.id      << std::endl;

    std::cerr << " " << ssReqException.str() << std::endl;
    SPDLOG_LOGGER_ERROR(logger, "ncap_device_auth_token_t::get_device_access_token/2 bad response {}", username);
  }

  return device_acces_token;
}


