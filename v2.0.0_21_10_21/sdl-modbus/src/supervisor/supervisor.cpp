/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#include "launcher/node/modbus/mb_controllers_launcher.h"
#include "launcher/node/modbus/mb_ncap_mqtt_launcher.h"
#include "ncap/network/mqtt/ncap_mqtt_backend.h"
#include "launcher/node/modbus/mb_node_master_hdf5_recorder_launcher.h"
#include "launcher/node/modbus/mb_node_master_imgui_launcher.h"
#include "launcher/node/modbus/mb_ncap_mqtt_launcher.h"
#include "node/modbus/modbus_master_node.h"
#include "meta-data-models/thing.h"
#include "supervisor/supervisor.h"

app_supervisor_t::app_supervisor_t ()
{
  // Initialize the containers of node registry and logger registry etc.
  modbus_node_registry_ = std::make_shared<modbus_node_registry_t> ();
  logger_registry_ = std::make_shared<logger_registry_t> ();

  modbus_master_node_controllers_vec_ = std::make_shared<std::vector<modbus_master_node_controller_t>>();
  modbus_server_node_controllers_vec_ = std::make_shared<std::vector<modbus_server_node_controller_t>>();

  // It will be initialized later...
  that_supervisor_ = nullptr;
}

int app_supervisor_t::launch_app(std::shared_ptr<app_supervisor_t> that_supervisor)
{
  // Store that  = this.
  that_supervisor_ = that_supervisor;

  std::mutex                mutex_;
  std::condition_variable   condition_;

  //
  // IMPORTANT
  // Only one object in the vector / collection. Do not create multiple elements.
  // Start modbus controllers
  //
  mb_edge_controller_launcher_t mb_edge_ctrl_l(that_supervisor_);
  mb_edge_controller_launcher_vec_.emplace_back(std::move(mb_edge_ctrl_l));


  std::transform(mb_edge_controller_launcher_vec_.begin(), mb_edge_controller_launcher_vec_.end()
  , mb_edge_controller_launcher_vec_.begin(), [&condition_](mb_edge_controller_launcher_t &c) {
    c.ctrl_wrapper_thread_ = std::make_shared<std::thread>([&c, &condition_]() {
      // Build the nodes from the config before launching the threads. VERY IMPORTANT
      c.init_launcher();
      condition_.notify_one();
      c.wrapper_fn();
    });
    return c;
  });


  // Wait till the node config is read and build the node registry.
  std::unique_lock<std::mutex> lock(mutex_);
  condition_.wait(lock);

  //
  // DESIGN NOTES: IMPORTANT
  // Only one object in the vector / collection. Do not create multiple elements.
  // Start HDF5 Recorder
  //
  mb_node_master_h5_recorder_launcher_t mb_h5_launcher(that_supervisor_);
  mb_h5_launcher_vec_.emplace_back(std::move(mb_h5_launcher));

  std::transform(mb_h5_launcher_vec_.begin(), mb_h5_launcher_vec_.end()
  , mb_h5_launcher_vec_.begin(), [](mb_node_master_h5_recorder_launcher_t &w) {
    w.h5_wrapper_thread_ = std::make_shared<std::thread>([&w]() {
      w.wrapper_fn();
    });
    return w;
  });

  //
  // DESIGN NOTES: IMPORTANT
  // Only one object in the vector / collection. Do not create multiple elements.
  // Start Modbus Gui components.
  //
  mb_node_master_imgui_launcher_t mb_imgui_launcher(that_supervisor_);
  modbus_master_gui_launcher_vec_.emplace_back(std::move(mb_imgui_launcher));

  std::transform(modbus_master_gui_launcher_vec_.begin(), modbus_master_gui_launcher_vec_.end()
  , modbus_master_gui_launcher_vec_.begin(), [&condition_](mb_node_master_imgui_launcher_t &g) {
    g.gwrapper_thread_ = std::make_shared<std::thread>([&g,  &condition_]() {
      g.init_worker();
      condition_.notify_one();
      g.wrapper_fn();
    });
    return g;
  });

  condition_.wait(lock);
  
  //
  // DESIGN NOTES: IMPORTANT
  // Only one object in the vector / collection. Do not create multiple elements.
  mb_ncap_mqtt_launcher_t ncap_mqtt_launcher(that_supervisor_);
  mb_ncap_mqtt_launcher_vec_.emplace_back(std::move(ncap_mqtt_launcher));
  
  std::transform(mb_ncap_mqtt_launcher_vec_.begin(), mb_ncap_mqtt_launcher_vec_.end()
  , mb_ncap_mqtt_launcher_vec_.begin(), [&condition_](mb_ncap_mqtt_launcher_t &n) {
    n.gwrapper_thread_ = std::make_shared<std::thread>([&n,  &condition_]() {
      n.init_worker();
      condition_.notify_one();
      n.wrapper_fn();
    });
    return n;
  });
  
  condition_.wait(lock);
  
  // Start Main Gui components.
  std::shared_ptr<mb_node_master_imgui_components_main_t> mb_gui  = get_mb_master_gui_components();
  std::shared_ptr<mb_node_master_dataset_recorder_main_t> mb_h5   = get_modbus_h5_recorder_handle();

  app_ = std::make_shared<app_main_gui_t>(mb_gui, mb_h5);
  app_->imgui_main_window();

  //
  // Wait for the app compotents / threads return
  //

  std::for_each(mb_edge_controller_launcher_vec_.begin(), mb_edge_controller_launcher_vec_.end()
  , [](mb_edge_controller_launcher_t &c) {
    c.ctrl_wrapper_thread_->join();
  });

  std::for_each(mb_h5_launcher_vec_.begin(), mb_h5_launcher_vec_.end()
  , [](mb_node_master_h5_recorder_launcher_t &c) {
    c.h5_wrapper_thread_->join();
  });

  std::for_each(modbus_master_gui_launcher_vec_.begin(), modbus_master_gui_launcher_vec_.end()
  , [](mb_node_master_imgui_launcher_t &c) {
    // Wrapper thread. Wait for the "child" threads (GUI, Communication etc) to exit.
    c.gwrapper_thread_->join();
  });

  return 0;
}

std::shared_ptr<mb_node_master_dataset_recorder_main_t>app_supervisor_t::get_modbus_h5_recorder_handle() const
{
  try {
    auto hl = mb_h5_launcher_vec_.at(0);
    return hl.h5_worker_->h5_rec_;
  }
  catch(const std::out_of_range & hrex) {
    return nullptr;
  }

  return nullptr;
}

const modbus_master_node_t* app_supervisor_t::get_mb_master_node_handle(const std::string& controller_id
  , const std::string& agent_id
  , const std::string& node_id) const
{

  auto ctrl = std::find_if(modbus_master_node_controllers_vec_->begin()
      , modbus_master_node_controllers_vec_->end()
      , [&](modbus_master_node_controller_t c) {
        return (c.controller_id_.compare(controller_id) == 0) ? true : false;
    }
  );

  if(ctrl == modbus_master_node_controllers_vec_->end()) {
    // Something wrong... the controller Id is not found.
    return nullptr;
  }

  // Found controller..Let's find the agent and modbus node.
  auto agent_vec = ctrl->mb_master_node_agents_vec_;

  auto agent = std::find_if(agent_vec.begin()
    , agent_vec.end()
    , [&](modbus_master_node_agent_t ma) {
      if((ma.agent_id_.compare(agent_id) == 0) && (ma.node_id_.compare(node_id) == 0)) {
        return true;
      }
      return false;
    }
  );

  if(agent == agent_vec.end()) {
    return nullptr;
  }

  // We have a valid modbus node.
  return agent->mb_master_node_.get();
}

std::shared_ptr<mb_node_master_imgui_components_main_t> app_supervisor_t::get_mb_master_gui_components()
{
  try {
    auto mbg = modbus_master_gui_launcher_vec_.at(0);
    return mbg.mb_master_gui_worker_->mb_master_gui_components_;
  }
  catch(const std::out_of_range & hrex) {
    return nullptr;
  }
  return nullptr;
}
