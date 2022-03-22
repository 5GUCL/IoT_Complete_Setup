/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_MB_IMGUI_LAUNCGER_H_
#define _CS_EDM_MB_IMGUI_LAUNCGER_H_

#include <thread>
#include <memory>
#include "app-gui/node-gui/modbus/mb_node_master_imgui_main.h"

// Forward declaration.
struct app_supervisor_t;

struct mb_node_master_imgui_controller_worker_t {
  mb_node_master_imgui_controller_worker_t (std::shared_ptr<app_supervisor_t> that_supervisor);
  void work();
  std::shared_ptr<mb_node_master_imgui_components_main_t> mb_master_gui_components_;
};


struct mb_node_master_imgui_launcher_t {
  mb_node_master_imgui_launcher_t (std::shared_ptr<app_supervisor_t> that_supervisor);
  void wrapper_fn();
  int init_worker();
  std::shared_ptr<mb_node_master_imgui_controller_worker_t> mb_master_gui_worker_;
  std::shared_ptr<std::thread> gwrapper_thread_;
};

#endif /* _CS_EDM_MB_IMGUI_LAUNCGER_H_ */