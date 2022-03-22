/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
 */
/*========================END LICENSE NOTICE========================*/

#include <iostream>
#include <signal.h>
#include <error.h>
#include <mutex>

// Logging
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include "ncap_app_constants.h"
#include "ncap_mqtt_backend.h"
#include "ncap_supervisor.h"

int main(int argc, char **argv)
{
  // Create file rotating logger with 10mb size max and 10 rotated files
  auto ncap_logger = spdlog::rotating_logger_mt(NCAP_L21_LOGGER, "../ncap-fin-l21-logs/l21.log", 1048576 * 10, 10);
  ncap_logger->set_level(spdlog::level::trace);
  
  // Formatting options: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
  ncap_logger->set_pattern("%C-%M-%d|%T.%f|%l:[function %!] [line %#] %v");
  
  spdlog::flush_every(std::chrono::seconds(3));
  
  // Register Signal Handler.
  signal(SIGINT, [](int signum) {
    // @TBD Handle signals...
    exit(signum);
  });

  ncap_supervisor_t app_("ncap_config.yaml");
  std::cout << "Launching NCAP .... " << std::endl;
  app_.run();
  std::cout << "Waiting .... " << std::endl;
  app_.wait_for_ncap_backends_to_complete();
  
  std::cout << "Closing the App .... " << std::endl;
  return 0;
}
