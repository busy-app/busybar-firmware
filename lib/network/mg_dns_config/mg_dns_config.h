/**
 * @file mgdnsconfig.h
 * Mongoose DNS Configuration manager
 * 
 * Takes DNS server configuration from lwIP and forwards it to mongoose
 */

#pragma once

#include <mongoose.h>
#include <wifi/wifi.h>

/**
 * @brief Applies DNS configuration from Wi-Fi (static or acquired via DHCP) to
 * a Mongoose manager object.
 * 
 * All DNS queries made with this manager will use the new DNS server after this
 * function returns.
 * 
 * @param[inout] mgr Mongoose manager instance
 */
void mg_dns_config_apply_auto(struct mg_mgr* mgr);

/**
 * @brief Applies DNS configuration from Wi-Fi (static or acquired via DHCP) to
 * a Mongoose manager object.
 * 
 * All DNS queries made with this manager will use the new DNS server after this
 * function returns.
 * 
 * @param[inout] mgr Mongoose manager instance
 */
void mg_dns_config_apply_from_info(struct mg_mgr* mgr, const WifiInfo* info);

/**
 * @brief Cleans up all internal data that this helper modified
 * 
 * @param[inout] mgr Mongoose manager instance
 */
void mg_dns_config_cleanup(struct mg_mgr* mgr);
