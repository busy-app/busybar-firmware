/**
 * @file mongoose_dns.h
 * @brief Mongoose DNS Configuration manager.
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
 * @warning Call `mongoose_dns_deinit()` before `mg_mgr_free()`.
 *
 * @param[in,out] mgr Mongoose manager instance
 */
void mongoose_dns_init(struct mg_mgr* mgr);

/**
 * @brief Cleans up all internal data that this helper modified
 *
 * @param[in,out] mgr Mongoose manager instance
 */
void mongoose_dns_deinit(struct mg_mgr* mgr);
