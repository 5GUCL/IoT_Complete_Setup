/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#include <error.h>
#include <signal.h>
#include <mutex>

#include "node/modbus/modbus_node_registry.h"
#include "loggers/logger_registry.h"
#include "launcher/node/modbus/mb_controllers_launcher.h"
#include "launcher/node/modbus/mb_node_master_hdf5_recorder_launcher.h"
#include "launcher/node/modbus/mb_node_master_imgui_launcher.h"
#include "launcher/node/modbus/mb_ncap_mqtt_launcher.h"
#include "supervisor/supervisor.h"

int main()
{
  signal(SIGINT, [](int signum) {
    // @TBD Handle signals...
    exit(signum);
  });
  
  std::shared_ptr<app_supervisor_t> mm(new app_supervisor_t ());

  mm->launch_app(mm);

  return 0;
}
