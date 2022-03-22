/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_META_DATA_MODELS_THINGS_H_
#define _CS_EDM_META_DATA_MODELS_THINGS_H_

/* Standard C++ includes */
#include <stdio.h>
#include <stdlib.h>
#include <string>
/* Node event*/

// Design time data
struct thing_geo_t {
  std::string type_;
  std::string name_;
  std::string latitude_;
  std::string longitude_;
};

// Design time data
struct thing_place_t {
  std::string type_;
  std::string name_;
  std::string address_;
  thing_geo_t geo_;
};

// Design time data
struct thing_organization_t {
  std::string type_;
  std::string name_;
  std::string address_;
  thing_place_t place_;
};

// Design time data
struct thing_class_t {
  std::string type_;
  std::string name_;
  std::string identifier_;
  std::string tags_;
};


// Design time data
struct thing_transducer_context_t {
  std::string type_;
  std::string name_;
  std::string identifier_;
  std::string units_;
  double      low_low_;
  double      low_high_;
  double      high_low_;
  double      high_high_;
};


// Operational/Run time data
struct asset_thing_measurement_t {
  std::string           identifier_;
  std::string           description_;
  std::string           thing_url_;
  std::string           units_;
  double                value_;
  double                time_instance_;
};
#endif /* _CS_EDM_META_DATA_MODELS_THINGS_H_ */