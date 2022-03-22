/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#include "launcher/node/modbus/mb_node_master_hdf5_recorder_launcher.h"
#include "supervisor/supervisor.h"


mb_node_master_h5_recorder_worker_t::mb_node_master_h5_recorder_worker_t(std::shared_ptr<app_supervisor_t> that_supervisor)
{
  h5_rec_ = std::make_shared<mb_node_master_dataset_recorder_main_t>(that_supervisor);
}

void mb_node_master_h5_recorder_worker_t::work()
{

  h5_rec_->init_mb_dataset_recorder();
}

///////////////////////////////////////////////////////////

mb_node_master_h5_recorder_launcher_t::mb_node_master_h5_recorder_launcher_t(std::shared_ptr<app_supervisor_t> that_supervisor)
{
  h5_worker_ = std::make_shared<mb_node_master_h5_recorder_worker_t> (that_supervisor);
}


void mb_node_master_h5_recorder_launcher_t::wrapper_fn()
{

  h5_worker_->work();
}
