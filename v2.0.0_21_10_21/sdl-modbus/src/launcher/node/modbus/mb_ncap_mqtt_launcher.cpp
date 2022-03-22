/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#include "launcher/node/modbus/mb_ncap_mqtt_launcher.h"
#include "supervisor/supervisor.h"

mb_ncap_mqtt_worker_t::mb_ncap_mqtt_worker_t(std::shared_ptr<app_supervisor_t> that_supervisor)
{
  mb_ncap_mqtt_ = std::make_shared<mb_ncap_mqtt_backend_t>(that_supervisor);
}

void mb_ncap_mqtt_worker_t::work()
{

  mb_ncap_mqtt_->init_mb_ncap_mqtt();
}

///////////////////////////////////////////////////////////

mb_ncap_mqtt_launcher_t::mb_ncap_mqtt_launcher_t(std::shared_ptr<app_supervisor_t> that_supervisor)
{
  mb_ncap_mqtt_worker_ = std::make_shared<mb_ncap_mqtt_worker_t> (that_supervisor);
}

void mb_ncap_mqtt_launcher_t::wrapper_fn()
{

  mb_ncap_mqtt_worker_->work();
}

int mb_ncap_mqtt_launcher_t::init_worker()
{
  // Placeholder method.
  
  return 0;
}