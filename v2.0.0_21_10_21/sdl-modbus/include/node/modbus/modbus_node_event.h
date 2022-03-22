/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_MODBUS_NODE_EVENT_H_
#define _CS_EDM_MODBUS_NODE_EVENT_H_

/* Standard C++ includes */
#include <stdio.h>
#include <stdlib.h>
#include <string>

/* Node event*/
struct modbus_node_event_t {
  std::string time_stamp_;
  std::string event_type_;
  std::string event_topic_;
  std::string event_description_;
};
#endif /* _CS_EDM_MODBUS_NODE_EVENT_H_ */