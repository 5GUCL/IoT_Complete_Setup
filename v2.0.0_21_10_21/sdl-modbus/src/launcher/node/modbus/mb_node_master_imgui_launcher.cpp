/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#include "launcher/node/modbus/mb_node_master_imgui_launcher.h"
#include "supervisor/supervisor.h"

mb_node_master_imgui_controller_worker_t::mb_node_master_imgui_controller_worker_t(std::shared_ptr<app_supervisor_t> that_supervisor)
{
  mb_master_gui_components_ = std::make_shared<mb_node_master_imgui_components_main_t>(that_supervisor);
}

void mb_node_master_imgui_controller_worker_t::work()
{
  if(mb_master_gui_components_) {
    mb_master_gui_components_->run_gui_components();
  }
}


///////////////////////////////////////////////

mb_node_master_imgui_launcher_t::mb_node_master_imgui_launcher_t(std::shared_ptr<app_supervisor_t> that_supervisor)
{
  mb_master_gui_worker_ = std::make_shared<mb_node_master_imgui_controller_worker_t>(that_supervisor);

}

void mb_node_master_imgui_launcher_t::wrapper_fn()
{
  if(mb_master_gui_worker_) {
    mb_master_gui_worker_->work();
  }
}

int mb_node_master_imgui_launcher_t::init_worker()
{
  if(mb_master_gui_worker_) {
    mb_master_gui_worker_->mb_master_gui_components_->init_gui_components();
  }

  return 0;
}