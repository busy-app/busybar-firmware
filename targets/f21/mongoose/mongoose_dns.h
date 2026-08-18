/**
 * @file mongoose_dns.h
 * Mongoose DNS Configuration manager
 * 
 * Takes DNS server configuration from lwIP and forwards it to mongoose
 */

#pragma once

#include <mongoose.h>

/**
 * @brief Applies DNS configuration from Wi-Fi (static or acquired via DHCP) to
 * a Mongoose manager object.
 * 
 * If it's not configured or not available yet, the default DNS server is used
 * (Google's `8.8.8.8`).
 * 
 * All DNS queries made with this manager will use the new DNS server after this
 * function returns.
 * 
 * @warning Call `deinit` before `mg_mgr_free` or before using another
 *          `init_*` function.
 * 
 * @param[inout] mgr Mongoose manager instance
 */
void mongoose_dns_init_auto(struct mg_mgr* mgr);

/**
 * @brief Uses a custom DNS server.
 * 
 * All DNS queries made with this manager will use the new DNS server after this
 * function returns.
 * 
 * @warning Call `deinit` before `mg_mgr_free` or before using another
 *          `init_*` function.
 * 
 * @param[inout] mgr Mongoose manager instance
 * @param[in] address Address in machine byte order (e.g. `127.0.0.1` is
 *                    `0x0100007F`). If `0` is provided, the default DNS server
 *                    is used (Google's `8.8.8.8`)
 */
void mongoose_dns_init_manual(struct mg_mgr* mgr, uint32_t address);

/**
 * @brief Cleans up all internal data that this helper modified
 * 
 * @param[inout] mgr Mongoose manager instance
 */
void mongoose_dns_deinit(struct mg_mgr* mgr);
