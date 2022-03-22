/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_NCAP_BACKENDS_DEFS_H_
#define _CS_EDM_NCAP_BACKENDS_DEFS_H_

#include <string>

struct ncap_mqtt_backend_config_t {
  std::string ncap_backend_;
  std::string mqtt_broker_address_;
  int mqtt_broker_port_;
  int encryption_;
  std::string fc_rules_;
};


#endif /* _CS_EDM_NCAP_BACKENDS_DEFS_H_ */