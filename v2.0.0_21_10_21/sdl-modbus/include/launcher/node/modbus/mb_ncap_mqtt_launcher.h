/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_NCAP_MQTT_LAUNCHER_H_
#define _CS_EDM_NCAP_MQTT_LAUNCHER_H_

#include <thread>
#include <memory>
#include "ncap/network/mqtt/ncap_mqtt_backend.h"

// Forward declaration.
struct app_supervisor_t;

struct mb_ncap_mqtt_worker_t {
  mb_ncap_mqtt_worker_t (std::shared_ptr<app_supervisor_t> that_supervisor);
  void work();
  std::shared_ptr<mb_ncap_mqtt_backend_t> mb_ncap_mqtt_;
};

struct mb_ncap_mqtt_launcher_t {
  mb_ncap_mqtt_launcher_t (std::shared_ptr<app_supervisor_t> that_supervisor);
  void wrapper_fn();
  int init_worker();
  std::shared_ptr<mb_ncap_mqtt_worker_t> mb_ncap_mqtt_worker_;
  std::shared_ptr<std::thread> gwrapper_thread_;
};

#endif /* _CS_EDM_NCAP_MQTT_LAUNCHER_H_ */
