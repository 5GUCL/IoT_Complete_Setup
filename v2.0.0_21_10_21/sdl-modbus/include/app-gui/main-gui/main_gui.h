/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_MAIN_GUI_H_
#define _CS_EDM_MAIN_GUI_H_

#include <iostream>
#include <sstream>

#include "sf-sys/utils/cpp_prog_utils.h"
#include "meta-data-models/thing.h"
#include "app-gui/node-gui/modbus/mb_node_master_imgui_main.h"
#include "h-data/hdf5/node/modbus/mb_node_master_hdf5_recorder.h"
#include "implot/implot.h"
#include "imnodes/imnodes.h"

struct app_main_gui_t {

  // Modbus and other GUI components need to be passed. Rr then can be registered aftet
  // the construction of the object.
  app_main_gui_t (std::shared_ptr<mb_node_master_imgui_components_main_t> mb_gui
    , std::shared_ptr<mb_node_master_dataset_recorder_main_t> mb_h5)
  {
    mb_gui_ = mb_gui;
    mb_h5_  = mb_h5;
  }

  int imgui_main_window();
  bool load_texture_from_file(const char* filename, GLuint* out_texture, int* out_width, int* out_height);
  void set_style_seaborn();

  std::shared_ptr<mb_node_master_imgui_components_main_t> mb_gui_;
  std::shared_ptr<mb_node_master_dataset_recorder_main_t> mb_h5_;
};


#endif  /* _CS_EDM_MAIN_GUI_H_ */