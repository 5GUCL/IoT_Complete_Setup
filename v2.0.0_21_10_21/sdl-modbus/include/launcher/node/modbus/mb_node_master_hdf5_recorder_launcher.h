/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_MB_HDF5_RECORDER_LAUNCHER_H_
#define _CS_EDM_MB_HDF5_RECORDER_LAUNCHER_H_

#include <thread>
#include <memory>
#include "node/modbus/modbus_node_registry.h"
#include "loggers/logger_registry.h"
#include "h-data/hdf5/node/modbus/mb_node_master_hdf5_recorder.h"

// Forward declaration.
struct app_supervisor_t;

struct mb_node_master_h5_recorder_worker_t {
  mb_node_master_h5_recorder_worker_t(std::shared_ptr<app_supervisor_t> that_supervisor);
  void work();
  std::shared_ptr<mb_node_master_dataset_recorder_main_t> h5_rec_;
};


struct mb_node_master_h5_recorder_launcher_t {
  mb_node_master_h5_recorder_launcher_t(std::shared_ptr<app_supervisor_t> that_supervisor);
  void wrapper_fn();
  std::shared_ptr<mb_node_master_h5_recorder_worker_t> h5_worker_;
  std::shared_ptr<std::thread> h5_wrapper_thread_;
};

#endif /* _CS_EDM_MB_HDF5_RECORDER_LAUNCHER_H_ */