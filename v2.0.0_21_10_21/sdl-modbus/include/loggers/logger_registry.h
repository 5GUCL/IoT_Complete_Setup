/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_LOGGER_REGISTRY_H_
#define _CS_EDM_LOGGER_REGISTRY_H_

#include <string>
#include <vector>

struct logger_registry_t {
  logger_registry_t() {};

  // Returns true if new node config can run in the current process.
  bool register_new_logger(const std::string& logger_code, const std::string& logger_name)
  {
    std::string code = logger_code;
    std::string name = logger_name;
    logger_code_registry_.emplace_back(std::move(code));
    logger_name_registry_.emplace_back(std::move(name));
    return true;
  };

  // Logger code is the directory name...
  std::vector<std::string> logger_code_registry_;

  // Logger name...
  std::vector<std::string> logger_name_registry_;

};

#endif /* _CS_EDM_LOGGER_REGISTRY_H_ */
