/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#include "node/modbus/modbus_master_node.h"
#include "node/modbus/modbus_master_node_agent.h"

////////////////////////////////////////////////////////////////////////
//                 MASTER NODES -> SLAVE DEVICES                      //
////////////////////////////////////////////////////////////////////////
modbus_master_node_agent_t::modbus_master_node_agent_t
(const std::string& controller_id
  , const std::string& agent_id, const std::string& agent_type
  , const std::string& agent_name, const std::string& agent_address
  , const std::string& agent_api_key, const std::string& agent_logger_name, long max_nodes
  , const std::string& node_id, const std::string& node_type
  , const std::string& node_name, const std::string& node_address, const std::string& node_api_key)
  : controller_id_(controller_id)
  , agent_id_(agent_id), agent_type_(agent_type)
  , agent_name_(agent_name), agent_address_(agent_address)
  , agent_api_key_(agent_api_key), agent_logger_name_(agent_logger_name), max_nodes_(max_nodes)
  , node_id_(node_id), node_type_(node_type)
  , node_name_(node_name), node_address_(node_address), node_api_key_(node_api_key)
{

  mb_master_node_  = nullptr;

  if(node_type_.compare("modbus-master") == 0) {
    mb_master_node_ = std::make_shared<modbus_master_node_t>
      (controller_id_
        , agent_id_, agent_type_
        , agent_name_, agent_address_
        , agent_api_key_, agent_logger_name_
        , node_id_, node_type_
        , node_name_, node_address_, node_api_key_);
  }

}

/* Edge device manager worker / node agent worker function. */
modbus_master_node_agent_t& modbus_master_node_agent_t::process_events_fn
(std::function<void(std::shared_ptr<std::vector<modbus_node_event_t>>)> node_events_callback)
{
  auto node_logger = spdlog::get(agent_logger_name_);

  if(node_type_.compare("modbus-master") == 0) {
    if(mb_master_node_) {
      modbus_master_node_t *sn = (modbus_master_node_t*)mb_master_node_.get();
      sn->prepare_modbus_node_events();

      if(node_events_callback) {
        node_events_callback(sn->get_modbus_node_events());
      }
    }
  }

  return *this;
}

modbus_master_node_agent_t& modbus_master_node_agent_t::init_agent()
{
  // Now initializa the node
  // Controller -> Agent -> Node {Modbus master node}
  mb_master_node_->init_modbus_node();
  return *this;
}