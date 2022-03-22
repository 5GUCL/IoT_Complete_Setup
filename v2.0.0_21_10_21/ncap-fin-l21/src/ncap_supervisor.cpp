/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
 */
/*========================END LICENSE NOTICE========================*/

#include <thread>
#include <iostream>     // std::ios, std::istream, std::cout
#include <fstream>      // std::filebuf
#include <sstream>
#include <map>

#include "ncap_app_constants.h"
#include "ncap_supervisor.h"

ncap_supervisor_t::ncap_supervisor_t (const std::string& ncap_config_file)
                  : ncap_config_file_(ncap_config_file)
                  , registry_sptr_(new ncap_config_registry_t(ncap_config_file))
{
  // Other required initialisations.
  registry_sptr_->load_ncap_config_registry();

  // Upon successful loading of registry, let's construct MQTT-MODBUS backend objects.
  for (auto mm: registry_sptr_->mmb_registry_) {
    auto mmb_obj = std::make_shared<ncap_mqtt_backend_t>(mm.second);
    mmb_backend_vec_.emplace_back(std::move(mmb_obj));
  }
}

void
ncap_supervisor_t::run () {
  std::transform(mmb_backend_vec_.begin(), mmb_backend_vec_.end()
    , mmb_backend_vec_.begin(), [](std::shared_ptr<ncap_mqtt_backend_t> &n) {
      n->mmb_wrapper_thread_ = std::make_shared<std::thread>([&n]() {
        n->mmb_wrapper_thread_fn();
      }
    );
    return n;
  });
}

void
ncap_supervisor_t::wait_for_ncap_backends_to_complete () {
  std::for_each(mmb_backend_vec_.begin(), mmb_backend_vec_.end()
    , [](std::shared_ptr<ncap_mqtt_backend_t> &c) {
      c->mmb_wrapper_thread_->join();
    }
  );
}
