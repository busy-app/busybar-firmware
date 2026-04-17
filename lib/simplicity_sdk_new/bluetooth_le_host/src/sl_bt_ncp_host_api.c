/***************************************************************************//**
 * @brief SL_BT_API commands for NCP host
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "sl_bt_api.h"
#include "sli_bt_api.h"

void sl_bt_host_handle_command();
void sl_bt_host_handle_command_noresponse();
extern sl_bt_msg_t *sl_bt_cmd_msg;
extern sl_bt_msg_t *sl_bt_rsp_msg;

sl_status_t sl_bt_dfu_flash_set_address(uint32_t address)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_dfu_flash_set_address_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_dfu_class_id,
                                       sli_bt_dfu_flash_set_address_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_dfu_flash_set_address.address = address;
    sl_bt_host_handle_command();

    return rsp->data.rsp_dfu_flash_set_address.result;
}

sl_status_t sl_bt_dfu_flash_upload(size_t data_len, const uint8_t* data)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_dfu_flash_upload_t) + data_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_dfu_class_id,
                                       sli_bt_dfu_flash_upload_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_dfu_flash_upload.data.len = data_len;
    memcpy(cmd->data.cmd_dfu_flash_upload.data.data, data, data_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_dfu_flash_upload.result;
}

sl_status_t sl_bt_dfu_flash_upload_finish(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_dfu_class_id,
                                       sli_bt_dfu_flash_upload_finish_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_dfu_flash_upload_finish.result;
}

sl_status_t sl_bt_system_hello(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_hello_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_system_hello.result;
}

sl_status_t sl_bt_system_start_bluetooth(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_start_bluetooth_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_system_start_bluetooth.result;
}

sl_status_t sl_bt_system_stop_bluetooth(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_stop_bluetooth_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_system_stop_bluetooth.result;
}

sl_status_t sl_bt_system_forcefully_stop_bluetooth(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_forcefully_stop_bluetooth_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_system_forcefully_stop_bluetooth.result;
}

sl_status_t sl_bt_system_get_version(uint16_t *major,
                                     uint16_t *minor,
                                     uint16_t *patch,
                                     uint16_t *build,
                                     uint32_t *bootloader,
                                     uint32_t *hash)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_get_version_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    if (major) {
        *major = rsp->data.rsp_system_get_version.major;
    }
    if (minor) {
        *minor = rsp->data.rsp_system_get_version.minor;
    }
    if (patch) {
        *patch = rsp->data.rsp_system_get_version.patch;
    }
    if (build) {
        *build = rsp->data.rsp_system_get_version.build;
    }
    if (bootloader) {
        *bootloader = rsp->data.rsp_system_get_version.bootloader;
    }
    if (hash) {
        *hash = rsp->data.rsp_system_get_version.hash;
    }

    return rsp->data.rsp_system_get_version.result;
}

void sl_bt_system_reboot(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_reboot_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command_noresponse();
}

sl_status_t sl_bt_system_halt(uint8_t halt)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_system_halt_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_halt_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_system_halt.halt = halt;
    sl_bt_host_handle_command();

    return rsp->data.rsp_system_halt.result;
}

sl_status_t sl_bt_system_linklayer_configure(uint8_t key,
                                             size_t data_len,
                                             const uint8_t* data)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_system_linklayer_configure_t) + data_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_linklayer_configure_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_system_linklayer_configure.key = key;
    cmd->data.cmd_system_linklayer_configure.data.len = data_len;
    memcpy(cmd->data.cmd_system_linklayer_configure.data.data, data, data_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_system_linklayer_configure.result;
}

sl_status_t sl_bt_system_set_tx_power(int16_t min_power,
                                      int16_t max_power,
                                      int16_t *set_min,
                                      int16_t *set_max)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_system_set_tx_power_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_set_tx_power_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_system_set_tx_power.min_power = min_power;
    cmd->data.cmd_system_set_tx_power.max_power = max_power;
    sl_bt_host_handle_command();

    if (set_min) {
        *set_min = rsp->data.rsp_system_set_tx_power.set_min;
    }
    if (set_max) {
        *set_max = rsp->data.rsp_system_set_tx_power.set_max;
    }

    return rsp->data.rsp_system_set_tx_power.result;
}

sl_status_t sl_bt_system_get_tx_power_setting(int16_t *support_min,
                                              int16_t *support_max,
                                              int16_t *set_min,
                                              int16_t *set_max,
                                              int16_t *rf_path_gain)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_get_tx_power_setting_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    if (support_min) {
        *support_min = rsp->data.rsp_system_get_tx_power_setting.support_min;
    }
    if (support_max) {
        *support_max = rsp->data.rsp_system_get_tx_power_setting.support_max;
    }
    if (set_min) {
        *set_min = rsp->data.rsp_system_get_tx_power_setting.set_min;
    }
    if (set_max) {
        *set_max = rsp->data.rsp_system_get_tx_power_setting.set_max;
    }
    if (rf_path_gain) {
        *rf_path_gain = rsp->data.rsp_system_get_tx_power_setting.rf_path_gain;
    }

    return rsp->data.rsp_system_get_tx_power_setting.result;
}

sl_status_t sl_bt_system_set_identity_address(bd_addr address, uint8_t type)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_system_set_identity_address_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_set_identity_address_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    memcpy(&cmd->data.cmd_system_set_identity_address.address, &address, sizeof(bd_addr));
    cmd->data.cmd_system_set_identity_address.type = type;
    sl_bt_host_handle_command();

    return rsp->data.rsp_system_set_identity_address.result;
}

sl_status_t sl_bt_system_get_identity_address(bd_addr *address, uint8_t *type)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_get_identity_address_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    if (address) {
        memcpy(address, &rsp->data.rsp_system_get_identity_address.address, sizeof(bd_addr));
    }
    if (type) {
        *type = rsp->data.rsp_system_get_identity_address.type;
    }

    return rsp->data.rsp_system_get_identity_address.result;
}

sl_status_t sl_bt_system_get_random_data(uint8_t length,
                                         size_t max_data_size,
                                         size_t *data_len,
                                         uint8_t *data)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_system_get_random_data_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_get_random_data_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_system_get_random_data.length = length;
    sl_bt_host_handle_command();

    if (data_len) {
        *data_len = rsp->data.rsp_system_get_random_data.data.len;
    }
    if (data && (rsp->data.rsp_system_get_random_data.data.len <= max_data_size)) {
        memcpy(data, rsp->data.rsp_system_get_random_data.data.data, rsp->data.rsp_system_get_random_data.data.len);
    }

    return rsp->data.rsp_system_get_random_data.result;
}

sl_status_t sl_bt_system_data_buffer_write(size_t data_len,
                                           const uint8_t* data)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_system_data_buffer_write_t) + data_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_data_buffer_write_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_system_data_buffer_write.data.len = data_len;
    memcpy(cmd->data.cmd_system_data_buffer_write.data.data, data, data_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_system_data_buffer_write.result;
}

sl_status_t sl_bt_system_data_buffer_clear(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_data_buffer_clear_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_system_data_buffer_clear.result;
}

sl_status_t sl_bt_system_get_counters(uint8_t reset,
                                      uint16_t *tx_packets,
                                      uint16_t *rx_packets,
                                      uint16_t *crc_errors,
                                      uint16_t *failures)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_system_get_counters_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_get_counters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_system_get_counters.reset = reset;
    sl_bt_host_handle_command();

    if (tx_packets) {
        *tx_packets = rsp->data.rsp_system_get_counters.tx_packets;
    }
    if (rx_packets) {
        *rx_packets = rsp->data.rsp_system_get_counters.rx_packets;
    }
    if (crc_errors) {
        *crc_errors = rsp->data.rsp_system_get_counters.crc_errors;
    }
    if (failures) {
        *failures = rsp->data.rsp_system_get_counters.failures;
    }

    return rsp->data.rsp_system_get_counters.result;
}

sl_status_t sl_bt_system_set_lazy_soft_timer(uint32_t time,
                                             uint32_t slack,
                                             uint8_t handle,
                                             uint8_t single_shot)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_system_set_lazy_soft_timer_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_system_class_id,
                                       sli_bt_system_set_lazy_soft_timer_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_system_set_lazy_soft_timer.time = time;
    cmd->data.cmd_system_set_lazy_soft_timer.slack = slack;
    cmd->data.cmd_system_set_lazy_soft_timer.handle = handle;
    cmd->data.cmd_system_set_lazy_soft_timer.single_shot = single_shot;
    sl_bt_host_handle_command();

    return rsp->data.rsp_system_set_lazy_soft_timer.result;
}

sl_status_t sl_bt_linklayer_event_info_reporting_enable(uint8_t enable,
                                                        uint32_t configuration,
                                                        uint8_t procedure_type,
                                                        size_t procedure_identifier_len,
                                                        const uint8_t* procedure_identifier)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_linklayer_event_info_reporting_enable_t) + procedure_identifier_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_linklayer_class_id,
                                       sli_bt_linklayer_event_info_reporting_enable_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_linklayer_event_info_reporting_enable.enable = enable;
    cmd->data.cmd_linklayer_event_info_reporting_enable.configuration = configuration;
    cmd->data.cmd_linklayer_event_info_reporting_enable.procedure_type = procedure_type;
    cmd->data.cmd_linklayer_event_info_reporting_enable.procedure_identifier.len = procedure_identifier_len;
    memcpy(cmd->data.cmd_linklayer_event_info_reporting_enable.procedure_identifier.data, procedure_identifier, procedure_identifier_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_linklayer_event_info_reporting_enable.result;
}

sl_status_t sl_bt_resource_get_status(uint32_t *total_bytes,
                                      uint32_t *free_bytes)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_resource_class_id,
                                       sli_bt_resource_get_status_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    if (total_bytes) {
        *total_bytes = rsp->data.rsp_resource_get_status.total_bytes;
    }
    if (free_bytes) {
        *free_bytes = rsp->data.rsp_resource_get_status.free_bytes;
    }

    return rsp->data.rsp_resource_get_status.result;
}

sl_status_t sl_bt_resource_set_report_threshold(uint32_t low, uint32_t high)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_resource_set_report_threshold_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_resource_class_id,
                                       sli_bt_resource_set_report_threshold_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_resource_set_report_threshold.low = low;
    cmd->data.cmd_resource_set_report_threshold.high = high;
    sl_bt_host_handle_command();

    return rsp->data.rsp_resource_set_report_threshold.result;
}

sl_status_t sl_bt_resource_enable_connection_tx_report(uint16_t packet_count)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_resource_enable_connection_tx_report_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_resource_class_id,
                                       sli_bt_resource_enable_connection_tx_report_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_resource_enable_connection_tx_report.packet_count = packet_count;
    sl_bt_host_handle_command();

    return rsp->data.rsp_resource_enable_connection_tx_report.result;
}

sl_status_t sl_bt_resource_get_connection_tx_status(uint8_t connection,
                                                    uint16_t *flags,
                                                    uint16_t *packet_count,
                                                    uint32_t *data_len)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_resource_get_connection_tx_status_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_resource_class_id,
                                       sli_bt_resource_get_connection_tx_status_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_resource_get_connection_tx_status.connection = connection;
    sl_bt_host_handle_command();

    if (flags) {
        *flags = rsp->data.rsp_resource_get_connection_tx_status.flags;
    }
    if (packet_count) {
        *packet_count = rsp->data.rsp_resource_get_connection_tx_status.packet_count;
    }
    if (data_len) {
        *data_len = rsp->data.rsp_resource_get_connection_tx_status.data_len;
    }

    return rsp->data.rsp_resource_get_connection_tx_status.result;
}

sl_status_t sl_bt_resource_disable_connection_tx_report(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_resource_class_id,
                                       sli_bt_resource_disable_connection_tx_report_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_resource_disable_connection_tx_report.result;
}

sl_status_t sl_bt_gap_set_privacy_mode(uint8_t privacy, uint8_t interval)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gap_set_privacy_mode_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gap_class_id,
                                       sli_bt_gap_set_privacy_mode_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gap_set_privacy_mode.privacy = privacy;
    cmd->data.cmd_gap_set_privacy_mode.interval = interval;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gap_set_privacy_mode.result;
}

sl_status_t sl_bt_gap_set_data_channel_classification(size_t channel_map_len,
                                                      const uint8_t* channel_map)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gap_set_data_channel_classification_t) + channel_map_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gap_class_id,
                                       sli_bt_gap_set_data_channel_classification_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gap_set_data_channel_classification.channel_map.len = channel_map_len;
    memcpy(cmd->data.cmd_gap_set_data_channel_classification.channel_map.data, channel_map, channel_map_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_gap_set_data_channel_classification.result;
}

sl_status_t sl_bt_gap_set_identity_address(bd_addr address, uint8_t addr_type)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gap_set_identity_address_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gap_class_id,
                                       sli_bt_gap_set_identity_address_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    memcpy(&cmd->data.cmd_gap_set_identity_address.address, &address, sizeof(bd_addr));
    cmd->data.cmd_gap_set_identity_address.addr_type = addr_type;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gap_set_identity_address.result;
}

sl_status_t sl_bt_gap_get_identity_address(bd_addr *address, uint8_t *type)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gap_class_id,
                                       sli_bt_gap_get_identity_address_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    if (address) {
        memcpy(address, &rsp->data.rsp_gap_get_identity_address.address, sizeof(bd_addr));
    }
    if (type) {
        *type = rsp->data.rsp_gap_get_identity_address.type;
    }

    return rsp->data.rsp_gap_get_identity_address.result;
}

sl_status_t sl_bt_gap_get_max_connections(uint8_t *num_connections)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gap_class_id,
                                       sli_bt_gap_get_max_connections_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    if (num_connections) {
        *num_connections = rsp->data.rsp_gap_get_max_connections.num_connections;
    }

    return rsp->data.rsp_gap_get_max_connections.result;
}

sl_status_t sl_bt_advertiser_create_set(uint8_t *handle)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_advertiser_class_id,
                                       sli_bt_advertiser_create_set_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    if (handle) {
        *handle = rsp->data.rsp_advertiser_create_set.handle;
    }

    return rsp->data.rsp_advertiser_create_set.result;
}

sl_status_t sl_bt_advertiser_configure(uint8_t advertising_set, uint32_t flags)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_advertiser_configure_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_advertiser_class_id,
                                       sli_bt_advertiser_configure_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_advertiser_configure.advertising_set = advertising_set;
    cmd->data.cmd_advertiser_configure.flags = flags;
    sl_bt_host_handle_command();

    return rsp->data.rsp_advertiser_configure.result;
}

sl_status_t sl_bt_advertiser_set_timing(uint8_t advertising_set,
                                        uint32_t interval_min,
                                        uint32_t interval_max,
                                        uint16_t duration,
                                        uint8_t maxevents)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_advertiser_set_timing_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_advertiser_class_id,
                                       sli_bt_advertiser_set_timing_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_advertiser_set_timing.advertising_set = advertising_set;
    cmd->data.cmd_advertiser_set_timing.interval_min = interval_min;
    cmd->data.cmd_advertiser_set_timing.interval_max = interval_max;
    cmd->data.cmd_advertiser_set_timing.duration = duration;
    cmd->data.cmd_advertiser_set_timing.maxevents = maxevents;
    sl_bt_host_handle_command();

    return rsp->data.rsp_advertiser_set_timing.result;
}

sl_status_t sl_bt_advertiser_set_channel_map(uint8_t advertising_set,
                                             uint8_t channel_map)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_advertiser_set_channel_map_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_advertiser_class_id,
                                       sli_bt_advertiser_set_channel_map_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_advertiser_set_channel_map.advertising_set = advertising_set;
    cmd->data.cmd_advertiser_set_channel_map.channel_map = channel_map;
    sl_bt_host_handle_command();

    return rsp->data.rsp_advertiser_set_channel_map.result;
}

sl_status_t sl_bt_advertiser_set_tx_power(uint8_t advertising_set,
                                          int16_t power,
                                          int16_t *set_power)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_advertiser_set_tx_power_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_advertiser_class_id,
                                       sli_bt_advertiser_set_tx_power_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_advertiser_set_tx_power.advertising_set = advertising_set;
    cmd->data.cmd_advertiser_set_tx_power.power = power;
    sl_bt_host_handle_command();

    if (set_power) {
        *set_power = rsp->data.rsp_advertiser_set_tx_power.set_power;
    }

    return rsp->data.rsp_advertiser_set_tx_power.result;
}

sl_status_t sl_bt_advertiser_set_report_scan_request(uint8_t advertising_set,
                                                     uint8_t report_scan_req)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_advertiser_set_report_scan_request_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_advertiser_class_id,
                                       sli_bt_advertiser_set_report_scan_request_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_advertiser_set_report_scan_request.advertising_set = advertising_set;
    cmd->data.cmd_advertiser_set_report_scan_request.report_scan_req = report_scan_req;
    sl_bt_host_handle_command();

    return rsp->data.rsp_advertiser_set_report_scan_request.result;
}

sl_status_t sl_bt_advertiser_set_random_address(uint8_t advertising_set,
                                                uint8_t addr_type,
                                                bd_addr address,
                                                bd_addr *address_out)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_advertiser_set_random_address_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_advertiser_class_id,
                                       sli_bt_advertiser_set_random_address_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_advertiser_set_random_address.advertising_set = advertising_set;
    cmd->data.cmd_advertiser_set_random_address.addr_type = addr_type;
    memcpy(&cmd->data.cmd_advertiser_set_random_address.address, &address, sizeof(bd_addr));
    sl_bt_host_handle_command();

    if (address_out) {
        memcpy(address_out, &rsp->data.rsp_advertiser_set_random_address.address_out, sizeof(bd_addr));
    }

    return rsp->data.rsp_advertiser_set_random_address.result;
}

sl_status_t sl_bt_advertiser_clear_random_address(uint8_t advertising_set)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_advertiser_clear_random_address_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_advertiser_class_id,
                                       sli_bt_advertiser_clear_random_address_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_advertiser_clear_random_address.advertising_set = advertising_set;
    sl_bt_host_handle_command();

    return rsp->data.rsp_advertiser_clear_random_address.result;
}

sl_status_t sl_bt_advertiser_stop(uint8_t advertising_set)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_advertiser_stop_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_advertiser_class_id,
                                       sli_bt_advertiser_stop_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_advertiser_stop.advertising_set = advertising_set;
    sl_bt_host_handle_command();

    return rsp->data.rsp_advertiser_stop.result;
}

sl_status_t sl_bt_advertiser_delete_set(uint8_t advertising_set)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_advertiser_delete_set_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_advertiser_class_id,
                                       sli_bt_advertiser_delete_set_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_advertiser_delete_set.advertising_set = advertising_set;
    sl_bt_host_handle_command();

    return rsp->data.rsp_advertiser_delete_set.result;
}

sl_status_t sl_bt_legacy_advertiser_set_data(uint8_t advertising_set,
                                             uint8_t type,
                                             size_t data_len,
                                             const uint8_t* data)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_legacy_advertiser_set_data_t) + data_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_legacy_advertiser_class_id,
                                       sli_bt_legacy_advertiser_set_data_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_legacy_advertiser_set_data.advertising_set = advertising_set;
    cmd->data.cmd_legacy_advertiser_set_data.type = type;
    cmd->data.cmd_legacy_advertiser_set_data.data.len = data_len;
    memcpy(cmd->data.cmd_legacy_advertiser_set_data.data.data, data, data_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_legacy_advertiser_set_data.result;
}

sl_status_t sl_bt_legacy_advertiser_generate_data(uint8_t advertising_set,
                                                  uint8_t discover)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_legacy_advertiser_generate_data_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_legacy_advertiser_class_id,
                                       sli_bt_legacy_advertiser_generate_data_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_legacy_advertiser_generate_data.advertising_set = advertising_set;
    cmd->data.cmd_legacy_advertiser_generate_data.discover = discover;
    sl_bt_host_handle_command();

    return rsp->data.rsp_legacy_advertiser_generate_data.result;
}

sl_status_t sl_bt_legacy_advertiser_start(uint8_t advertising_set,
                                          uint8_t connect)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_legacy_advertiser_start_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_legacy_advertiser_class_id,
                                       sli_bt_legacy_advertiser_start_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_legacy_advertiser_start.advertising_set = advertising_set;
    cmd->data.cmd_legacy_advertiser_start.connect = connect;
    sl_bt_host_handle_command();

    return rsp->data.rsp_legacy_advertiser_start.result;
}

sl_status_t sl_bt_legacy_advertiser_start_directed(uint8_t advertising_set,
                                                   uint8_t connect,
                                                   bd_addr peer_addr,
                                                   uint8_t peer_addr_type)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_legacy_advertiser_start_directed_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_legacy_advertiser_class_id,
                                       sli_bt_legacy_advertiser_start_directed_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_legacy_advertiser_start_directed.advertising_set = advertising_set;
    cmd->data.cmd_legacy_advertiser_start_directed.connect = connect;
    memcpy(&cmd->data.cmd_legacy_advertiser_start_directed.peer_addr, &peer_addr, sizeof(bd_addr));
    cmd->data.cmd_legacy_advertiser_start_directed.peer_addr_type = peer_addr_type;
    sl_bt_host_handle_command();

    return rsp->data.rsp_legacy_advertiser_start_directed.result;
}

sl_status_t sl_bt_extended_advertiser_set_phy(uint8_t advertising_set,
                                              uint8_t primary_phy,
                                              uint8_t secondary_phy)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_extended_advertiser_set_phy_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_extended_advertiser_class_id,
                                       sli_bt_extended_advertiser_set_phy_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_extended_advertiser_set_phy.advertising_set = advertising_set;
    cmd->data.cmd_extended_advertiser_set_phy.primary_phy = primary_phy;
    cmd->data.cmd_extended_advertiser_set_phy.secondary_phy = secondary_phy;
    sl_bt_host_handle_command();

    return rsp->data.rsp_extended_advertiser_set_phy.result;
}

sl_status_t sl_bt_extended_advertiser_set_data(uint8_t advertising_set,
                                               size_t data_len,
                                               const uint8_t* data)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_extended_advertiser_set_data_t) + data_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_extended_advertiser_class_id,
                                       sli_bt_extended_advertiser_set_data_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_extended_advertiser_set_data.advertising_set = advertising_set;
    cmd->data.cmd_extended_advertiser_set_data.data.len = data_len;
    memcpy(cmd->data.cmd_extended_advertiser_set_data.data.data, data, data_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_extended_advertiser_set_data.result;
}

sl_status_t sl_bt_extended_advertiser_set_long_data(uint8_t advertising_set)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_extended_advertiser_set_long_data_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_extended_advertiser_class_id,
                                       sli_bt_extended_advertiser_set_long_data_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_extended_advertiser_set_long_data.advertising_set = advertising_set;
    sl_bt_host_handle_command();

    return rsp->data.rsp_extended_advertiser_set_long_data.result;
}

sl_status_t sl_bt_extended_advertiser_generate_data(uint8_t advertising_set,
                                                    uint8_t discover)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_extended_advertiser_generate_data_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_extended_advertiser_class_id,
                                       sli_bt_extended_advertiser_generate_data_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_extended_advertiser_generate_data.advertising_set = advertising_set;
    cmd->data.cmd_extended_advertiser_generate_data.discover = discover;
    sl_bt_host_handle_command();

    return rsp->data.rsp_extended_advertiser_generate_data.result;
}

sl_status_t sl_bt_extended_advertiser_start(uint8_t advertising_set,
                                            uint8_t connect,
                                            uint32_t flags)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_extended_advertiser_start_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_extended_advertiser_class_id,
                                       sli_bt_extended_advertiser_start_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_extended_advertiser_start.advertising_set = advertising_set;
    cmd->data.cmd_extended_advertiser_start.connect = connect;
    cmd->data.cmd_extended_advertiser_start.flags = flags;
    sl_bt_host_handle_command();

    return rsp->data.rsp_extended_advertiser_start.result;
}

sl_status_t sl_bt_extended_advertiser_start_directed(uint8_t advertising_set,
                                                     uint8_t connect,
                                                     uint32_t flags,
                                                     bd_addr peer_addr,
                                                     uint8_t peer_addr_type)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_extended_advertiser_start_directed_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_extended_advertiser_class_id,
                                       sli_bt_extended_advertiser_start_directed_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_extended_advertiser_start_directed.advertising_set = advertising_set;
    cmd->data.cmd_extended_advertiser_start_directed.connect = connect;
    cmd->data.cmd_extended_advertiser_start_directed.flags = flags;
    memcpy(&cmd->data.cmd_extended_advertiser_start_directed.peer_addr, &peer_addr, sizeof(bd_addr));
    cmd->data.cmd_extended_advertiser_start_directed.peer_addr_type = peer_addr_type;
    sl_bt_host_handle_command();

    return rsp->data.rsp_extended_advertiser_start_directed.result;
}

sl_status_t sl_bt_periodic_advertiser_set_data(uint8_t advertising_set,
                                               size_t data_len,
                                               const uint8_t* data)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_periodic_advertiser_set_data_t) + data_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_periodic_advertiser_class_id,
                                       sli_bt_periodic_advertiser_set_data_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_periodic_advertiser_set_data.advertising_set = advertising_set;
    cmd->data.cmd_periodic_advertiser_set_data.data.len = data_len;
    memcpy(cmd->data.cmd_periodic_advertiser_set_data.data.data, data, data_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_periodic_advertiser_set_data.result;
}

sl_status_t sl_bt_periodic_advertiser_set_long_data(uint8_t advertising_set)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_periodic_advertiser_set_long_data_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_periodic_advertiser_class_id,
                                       sli_bt_periodic_advertiser_set_long_data_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_periodic_advertiser_set_long_data.advertising_set = advertising_set;
    sl_bt_host_handle_command();

    return rsp->data.rsp_periodic_advertiser_set_long_data.result;
}

sl_status_t sl_bt_periodic_advertiser_start(uint8_t advertising_set,
                                            uint16_t interval_min,
                                            uint16_t interval_max,
                                            uint32_t flags)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_periodic_advertiser_start_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_periodic_advertiser_class_id,
                                       sli_bt_periodic_advertiser_start_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_periodic_advertiser_start.advertising_set = advertising_set;
    cmd->data.cmd_periodic_advertiser_start.interval_min = interval_min;
    cmd->data.cmd_periodic_advertiser_start.interval_max = interval_max;
    cmd->data.cmd_periodic_advertiser_start.flags = flags;
    sl_bt_host_handle_command();

    return rsp->data.rsp_periodic_advertiser_start.result;
}

sl_status_t sl_bt_periodic_advertiser_stop(uint8_t advertising_set)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_periodic_advertiser_stop_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_periodic_advertiser_class_id,
                                       sli_bt_periodic_advertiser_stop_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_periodic_advertiser_stop.advertising_set = advertising_set;
    sl_bt_host_handle_command();

    return rsp->data.rsp_periodic_advertiser_stop.result;
}

sl_status_t sl_bt_scanner_set_parameters(uint8_t mode,
                                         uint16_t interval,
                                         uint16_t window)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_scanner_set_parameters_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_scanner_class_id,
                                       sli_bt_scanner_set_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_scanner_set_parameters.mode = mode;
    cmd->data.cmd_scanner_set_parameters.interval = interval;
    cmd->data.cmd_scanner_set_parameters.window = window;
    sl_bt_host_handle_command();

    return rsp->data.rsp_scanner_set_parameters.result;
}

sl_status_t sl_bt_scanner_set_parameters_and_filter(uint8_t mode,
                                                    uint16_t interval,
                                                    uint16_t window,
                                                    uint32_t flags,
                                                    uint8_t filter_policy)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_scanner_set_parameters_and_filter_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_scanner_class_id,
                                       sli_bt_scanner_set_parameters_and_filter_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_scanner_set_parameters_and_filter.mode = mode;
    cmd->data.cmd_scanner_set_parameters_and_filter.interval = interval;
    cmd->data.cmd_scanner_set_parameters_and_filter.window = window;
    cmd->data.cmd_scanner_set_parameters_and_filter.flags = flags;
    cmd->data.cmd_scanner_set_parameters_and_filter.filter_policy = filter_policy;
    sl_bt_host_handle_command();

    return rsp->data.rsp_scanner_set_parameters_and_filter.result;
}

sl_status_t sl_bt_scanner_start(uint8_t scanning_phy, uint8_t discover_mode)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_scanner_start_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_scanner_class_id,
                                       sli_bt_scanner_start_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_scanner_start.scanning_phy = scanning_phy;
    cmd->data.cmd_scanner_start.discover_mode = discover_mode;
    sl_bt_host_handle_command();

    return rsp->data.rsp_scanner_start.result;
}

sl_status_t sl_bt_scanner_stop(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_scanner_class_id,
                                       sli_bt_scanner_stop_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_scanner_stop.result;
}

sl_status_t sl_bt_sync_set_reporting_mode(uint16_t sync,
                                          uint8_t reporting_mode)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sync_set_reporting_mode_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sync_class_id,
                                       sli_bt_sync_set_reporting_mode_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sync_set_reporting_mode.sync = sync;
    cmd->data.cmd_sync_set_reporting_mode.reporting_mode = reporting_mode;
    sl_bt_host_handle_command();

    return rsp->data.rsp_sync_set_reporting_mode.result;
}

sl_status_t sl_bt_sync_update_sync_parameters(uint16_t sync,
                                              uint16_t skip,
                                              uint16_t timeout)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sync_update_sync_parameters_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sync_class_id,
                                       sli_bt_sync_update_sync_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sync_update_sync_parameters.sync = sync;
    cmd->data.cmd_sync_update_sync_parameters.skip = skip;
    cmd->data.cmd_sync_update_sync_parameters.timeout = timeout;
    sl_bt_host_handle_command();

    return rsp->data.rsp_sync_update_sync_parameters.result;
}

sl_status_t sl_bt_sync_close(uint16_t sync)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sync_close_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sync_class_id,
                                       sli_bt_sync_close_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sync_close.sync = sync;
    sl_bt_host_handle_command();

    return rsp->data.rsp_sync_close.result;
}

sl_status_t sl_bt_sync_scanner_set_sync_parameters(uint16_t skip,
                                                   uint16_t timeout,
                                                   uint8_t reporting_mode)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sync_scanner_set_sync_parameters_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sync_scanner_class_id,
                                       sli_bt_sync_scanner_set_sync_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sync_scanner_set_sync_parameters.skip = skip;
    cmd->data.cmd_sync_scanner_set_sync_parameters.timeout = timeout;
    cmd->data.cmd_sync_scanner_set_sync_parameters.reporting_mode = reporting_mode;
    sl_bt_host_handle_command();

    return rsp->data.rsp_sync_scanner_set_sync_parameters.result;
}

sl_status_t sl_bt_sync_scanner_open(bd_addr address,
                                    uint8_t address_type,
                                    uint8_t adv_sid,
                                    uint16_t *sync)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sync_scanner_open_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sync_scanner_class_id,
                                       sli_bt_sync_scanner_open_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    memcpy(&cmd->data.cmd_sync_scanner_open.address, &address, sizeof(bd_addr));
    cmd->data.cmd_sync_scanner_open.address_type = address_type;
    cmd->data.cmd_sync_scanner_open.adv_sid = adv_sid;
    sl_bt_host_handle_command();

    if (sync) {
        *sync = rsp->data.rsp_sync_scanner_open.sync;
    }

    return rsp->data.rsp_sync_scanner_open.result;
}

sl_status_t sl_bt_past_receiver_set_default_sync_receive_parameters(uint8_t mode,
                                                                    uint16_t skip,
                                                                    uint16_t timeout,
                                                                    uint8_t reporting_mode)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_past_receiver_set_default_sync_receive_parameters_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_past_receiver_class_id,
                                       sli_bt_past_receiver_set_default_sync_receive_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_past_receiver_set_default_sync_receive_parameters.mode = mode;
    cmd->data.cmd_past_receiver_set_default_sync_receive_parameters.skip = skip;
    cmd->data.cmd_past_receiver_set_default_sync_receive_parameters.timeout = timeout;
    cmd->data.cmd_past_receiver_set_default_sync_receive_parameters.reporting_mode = reporting_mode;
    sl_bt_host_handle_command();

    return rsp->data.rsp_past_receiver_set_default_sync_receive_parameters.result;
}

sl_status_t sl_bt_past_receiver_set_sync_receive_parameters(uint8_t connection,
                                                            uint8_t mode,
                                                            uint16_t skip,
                                                            uint16_t timeout,
                                                            uint8_t reporting_mode)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_past_receiver_set_sync_receive_parameters_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_past_receiver_class_id,
                                       sli_bt_past_receiver_set_sync_receive_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_past_receiver_set_sync_receive_parameters.connection = connection;
    cmd->data.cmd_past_receiver_set_sync_receive_parameters.mode = mode;
    cmd->data.cmd_past_receiver_set_sync_receive_parameters.skip = skip;
    cmd->data.cmd_past_receiver_set_sync_receive_parameters.timeout = timeout;
    cmd->data.cmd_past_receiver_set_sync_receive_parameters.reporting_mode = reporting_mode;
    sl_bt_host_handle_command();

    return rsp->data.rsp_past_receiver_set_sync_receive_parameters.result;
}

sl_status_t sl_bt_past_receiver_set_default_sync_receive_over_sync_parameters(uint8_t mode,
                                                                              uint16_t skip,
                                                                              uint16_t timeout,
                                                                              uint8_t reporting_mode)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_past_receiver_set_default_sync_receive_over_sync_parameters_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_past_receiver_class_id,
                                       sli_bt_past_receiver_set_default_sync_receive_over_sync_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_past_receiver_set_default_sync_receive_over_sync_parameters.mode = mode;
    cmd->data.cmd_past_receiver_set_default_sync_receive_over_sync_parameters.skip = skip;
    cmd->data.cmd_past_receiver_set_default_sync_receive_over_sync_parameters.timeout = timeout;
    cmd->data.cmd_past_receiver_set_default_sync_receive_over_sync_parameters.reporting_mode = reporting_mode;
    sl_bt_host_handle_command();

    return rsp->data.rsp_past_receiver_set_default_sync_receive_over_sync_parameters.result;
}

sl_status_t sl_bt_past_receiver_set_sync_receive_over_sync_parameters(uint16_t sync,
                                                                      uint8_t mode,
                                                                      uint16_t skip,
                                                                      uint16_t timeout,
                                                                      uint8_t reporting_mode)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_past_receiver_set_sync_receive_over_sync_parameters_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_past_receiver_class_id,
                                       sli_bt_past_receiver_set_sync_receive_over_sync_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_past_receiver_set_sync_receive_over_sync_parameters.sync = sync;
    cmd->data.cmd_past_receiver_set_sync_receive_over_sync_parameters.mode = mode;
    cmd->data.cmd_past_receiver_set_sync_receive_over_sync_parameters.skip = skip;
    cmd->data.cmd_past_receiver_set_sync_receive_over_sync_parameters.timeout = timeout;
    cmd->data.cmd_past_receiver_set_sync_receive_over_sync_parameters.reporting_mode = reporting_mode;
    sl_bt_host_handle_command();

    return rsp->data.rsp_past_receiver_set_sync_receive_over_sync_parameters.result;
}

sl_status_t sl_bt_advertiser_past_transfer(uint8_t connection,
                                           uint16_t service_data,
                                           uint8_t advertising_set)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_advertiser_past_transfer_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_advertiser_past_class_id,
                                       sli_bt_advertiser_past_transfer_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_advertiser_past_transfer.connection = connection;
    cmd->data.cmd_advertiser_past_transfer.service_data = service_data;
    cmd->data.cmd_advertiser_past_transfer.advertising_set = advertising_set;
    sl_bt_host_handle_command();

    return rsp->data.rsp_advertiser_past_transfer.result;
}

sl_status_t sl_bt_advertiser_past_transfer_over_pawr_advertiser(uint8_t transferring_advertising_set,
                                                                uint16_t service_data,
                                                                uint8_t advertising_set,
                                                                uint8_t repeat_count,
                                                                size_t subevents_len,
                                                                const uint8_t* subevents)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_advertiser_past_transfer_over_pawr_advertiser_t) + subevents_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_advertiser_past_class_id,
                                       sli_bt_advertiser_past_transfer_over_pawr_advertiser_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_advertiser_past_transfer_over_pawr_advertiser.transferring_advertising_set = transferring_advertising_set;
    cmd->data.cmd_advertiser_past_transfer_over_pawr_advertiser.service_data = service_data;
    cmd->data.cmd_advertiser_past_transfer_over_pawr_advertiser.advertising_set = advertising_set;
    cmd->data.cmd_advertiser_past_transfer_over_pawr_advertiser.repeat_count = repeat_count;
    cmd->data.cmd_advertiser_past_transfer_over_pawr_advertiser.subevents.len = subevents_len;
    memcpy(cmd->data.cmd_advertiser_past_transfer_over_pawr_advertiser.subevents.data, subevents, subevents_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_advertiser_past_transfer_over_pawr_advertiser.result;
}

sl_status_t sl_bt_sync_past_transfer(uint8_t connection,
                                     uint16_t service_data,
                                     uint16_t sync)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sync_past_transfer_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sync_past_class_id,
                                       sli_bt_sync_past_transfer_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sync_past_transfer.connection = connection;
    cmd->data.cmd_sync_past_transfer.service_data = service_data;
    cmd->data.cmd_sync_past_transfer.sync = sync;
    sl_bt_host_handle_command();

    return rsp->data.rsp_sync_past_transfer.result;
}

sl_status_t sl_bt_pawr_sync_set_sync_subevents(uint16_t sync,
                                               size_t subevents_len,
                                               const uint8_t* subevents)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_pawr_sync_set_sync_subevents_t) + subevents_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_pawr_sync_class_id,
                                       sli_bt_pawr_sync_set_sync_subevents_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_pawr_sync_set_sync_subevents.sync = sync;
    cmd->data.cmd_pawr_sync_set_sync_subevents.subevents.len = subevents_len;
    memcpy(cmd->data.cmd_pawr_sync_set_sync_subevents.subevents.data, subevents, subevents_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_pawr_sync_set_sync_subevents.result;
}

sl_status_t sl_bt_pawr_sync_set_response_data(uint16_t sync,
                                              uint16_t request_event,
                                              uint8_t request_subevent,
                                              uint8_t response_subevent,
                                              uint8_t response_slot,
                                              size_t response_data_len,
                                              const uint8_t* response_data)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_pawr_sync_set_response_data_t) + response_data_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_pawr_sync_class_id,
                                       sli_bt_pawr_sync_set_response_data_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_pawr_sync_set_response_data.sync = sync;
    cmd->data.cmd_pawr_sync_set_response_data.request_event = request_event;
    cmd->data.cmd_pawr_sync_set_response_data.request_subevent = request_subevent;
    cmd->data.cmd_pawr_sync_set_response_data.response_subevent = response_subevent;
    cmd->data.cmd_pawr_sync_set_response_data.response_slot = response_slot;
    cmd->data.cmd_pawr_sync_set_response_data.response_data.len = response_data_len;
    memcpy(cmd->data.cmd_pawr_sync_set_response_data.response_data.data, response_data, response_data_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_pawr_sync_set_response_data.result;
}

sl_status_t sl_bt_pawr_advertiser_start(uint8_t advertising_set,
                                        uint16_t interval_min,
                                        uint16_t interval_max,
                                        uint32_t flags,
                                        uint8_t num_subevents,
                                        uint8_t subevent_interval,
                                        uint8_t response_slot_delay,
                                        uint8_t response_slot_spacing,
                                        uint8_t response_slots)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_pawr_advertiser_start_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_pawr_advertiser_class_id,
                                       sli_bt_pawr_advertiser_start_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_pawr_advertiser_start.advertising_set = advertising_set;
    cmd->data.cmd_pawr_advertiser_start.interval_min = interval_min;
    cmd->data.cmd_pawr_advertiser_start.interval_max = interval_max;
    cmd->data.cmd_pawr_advertiser_start.flags = flags;
    cmd->data.cmd_pawr_advertiser_start.num_subevents = num_subevents;
    cmd->data.cmd_pawr_advertiser_start.subevent_interval = subevent_interval;
    cmd->data.cmd_pawr_advertiser_start.response_slot_delay = response_slot_delay;
    cmd->data.cmd_pawr_advertiser_start.response_slot_spacing = response_slot_spacing;
    cmd->data.cmd_pawr_advertiser_start.response_slots = response_slots;
    sl_bt_host_handle_command();

    return rsp->data.rsp_pawr_advertiser_start.result;
}

sl_status_t sl_bt_pawr_advertiser_change_parameters(uint8_t advertising_set,
                                                    uint16_t interval_min,
                                                    uint16_t interval_max,
                                                    uint32_t flags,
                                                    uint8_t num_subevents,
                                                    uint8_t subevent_interval,
                                                    uint8_t response_slot_delay,
                                                    uint8_t response_slot_spacing,
                                                    uint8_t response_slots,
                                                    uint8_t phy,
                                                    uint8_t repeat_count)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_pawr_advertiser_change_parameters_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_pawr_advertiser_class_id,
                                       sli_bt_pawr_advertiser_change_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_pawr_advertiser_change_parameters.advertising_set = advertising_set;
    cmd->data.cmd_pawr_advertiser_change_parameters.interval_min = interval_min;
    cmd->data.cmd_pawr_advertiser_change_parameters.interval_max = interval_max;
    cmd->data.cmd_pawr_advertiser_change_parameters.flags = flags;
    cmd->data.cmd_pawr_advertiser_change_parameters.num_subevents = num_subevents;
    cmd->data.cmd_pawr_advertiser_change_parameters.subevent_interval = subevent_interval;
    cmd->data.cmd_pawr_advertiser_change_parameters.response_slot_delay = response_slot_delay;
    cmd->data.cmd_pawr_advertiser_change_parameters.response_slot_spacing = response_slot_spacing;
    cmd->data.cmd_pawr_advertiser_change_parameters.response_slots = response_slots;
    cmd->data.cmd_pawr_advertiser_change_parameters.phy = phy;
    cmd->data.cmd_pawr_advertiser_change_parameters.repeat_count = repeat_count;
    sl_bt_host_handle_command();

    return rsp->data.rsp_pawr_advertiser_change_parameters.result;
}

sl_status_t sl_bt_pawr_advertiser_set_subevent_data(uint8_t advertising_set,
                                                    uint8_t subevent,
                                                    uint8_t response_slot_start,
                                                    uint8_t response_slot_count,
                                                    size_t adv_data_len,
                                                    const uint8_t* adv_data)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_pawr_advertiser_set_subevent_data_t) + adv_data_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_pawr_advertiser_class_id,
                                       sli_bt_pawr_advertiser_set_subevent_data_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_pawr_advertiser_set_subevent_data.advertising_set = advertising_set;
    cmd->data.cmd_pawr_advertiser_set_subevent_data.subevent = subevent;
    cmd->data.cmd_pawr_advertiser_set_subevent_data.response_slot_start = response_slot_start;
    cmd->data.cmd_pawr_advertiser_set_subevent_data.response_slot_count = response_slot_count;
    cmd->data.cmd_pawr_advertiser_set_subevent_data.adv_data.len = adv_data_len;
    memcpy(cmd->data.cmd_pawr_advertiser_set_subevent_data.adv_data.data, adv_data, adv_data_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_pawr_advertiser_set_subevent_data.result;
}

sl_status_t sl_bt_pawr_advertiser_create_connection(uint8_t advertising_set,
                                                    uint8_t subevent,
                                                    bd_addr address,
                                                    uint8_t address_type,
                                                    uint8_t *connection)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_pawr_advertiser_create_connection_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_pawr_advertiser_class_id,
                                       sli_bt_pawr_advertiser_create_connection_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_pawr_advertiser_create_connection.advertising_set = advertising_set;
    cmd->data.cmd_pawr_advertiser_create_connection.subevent = subevent;
    memcpy(&cmd->data.cmd_pawr_advertiser_create_connection.address, &address, sizeof(bd_addr));
    cmd->data.cmd_pawr_advertiser_create_connection.address_type = address_type;
    sl_bt_host_handle_command();

    if (connection) {
        *connection = rsp->data.rsp_pawr_advertiser_create_connection.connection;
    }

    return rsp->data.rsp_pawr_advertiser_create_connection.result;
}

sl_status_t sl_bt_pawr_advertiser_stop(uint8_t advertising_set)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_pawr_advertiser_stop_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_pawr_advertiser_class_id,
                                       sli_bt_pawr_advertiser_stop_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_pawr_advertiser_stop.advertising_set = advertising_set;
    sl_bt_host_handle_command();

    return rsp->data.rsp_pawr_advertiser_stop.result;
}

sl_status_t sl_bt_connection_set_default_parameters(uint16_t min_interval,
                                                    uint16_t max_interval,
                                                    uint16_t latency,
                                                    uint16_t timeout,
                                                    uint16_t min_ce_length,
                                                    uint16_t max_ce_length)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_set_default_parameters_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_set_default_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_set_default_parameters.min_interval = min_interval;
    cmd->data.cmd_connection_set_default_parameters.max_interval = max_interval;
    cmd->data.cmd_connection_set_default_parameters.latency = latency;
    cmd->data.cmd_connection_set_default_parameters.timeout = timeout;
    cmd->data.cmd_connection_set_default_parameters.min_ce_length = min_ce_length;
    cmd->data.cmd_connection_set_default_parameters.max_ce_length = max_ce_length;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_set_default_parameters.result;
}

sl_status_t sl_bt_connection_set_default_preferred_phy(uint8_t preferred_phy,
                                                       uint8_t accepted_phy)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_set_default_preferred_phy_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_set_default_preferred_phy_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_set_default_preferred_phy.preferred_phy = preferred_phy;
    cmd->data.cmd_connection_set_default_preferred_phy.accepted_phy = accepted_phy;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_set_default_preferred_phy.result;
}

sl_status_t sl_bt_connection_set_default_data_length(uint16_t tx_data_len)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_set_default_data_length_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_set_default_data_length_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_set_default_data_length.tx_data_len = tx_data_len;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_set_default_data_length.result;
}

sl_status_t sl_bt_connection_set_default_acceptable_subrate(uint16_t min_subrate,
                                                            uint16_t max_subrate,
                                                            uint16_t max_latency,
                                                            uint16_t continuation_number,
                                                            uint16_t max_timeout)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_set_default_acceptable_subrate_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_set_default_acceptable_subrate_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_set_default_acceptable_subrate.min_subrate = min_subrate;
    cmd->data.cmd_connection_set_default_acceptable_subrate.max_subrate = max_subrate;
    cmd->data.cmd_connection_set_default_acceptable_subrate.max_latency = max_latency;
    cmd->data.cmd_connection_set_default_acceptable_subrate.continuation_number = continuation_number;
    cmd->data.cmd_connection_set_default_acceptable_subrate.max_timeout = max_timeout;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_set_default_acceptable_subrate.result;
}

sl_status_t sl_bt_connection_open(bd_addr address,
                                  uint8_t address_type,
                                  uint8_t initiating_phy,
                                  uint8_t *connection)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_open_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_open_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    memcpy(&cmd->data.cmd_connection_open.address, &address, sizeof(bd_addr));
    cmd->data.cmd_connection_open.address_type = address_type;
    cmd->data.cmd_connection_open.initiating_phy = initiating_phy;
    sl_bt_host_handle_command();

    if (connection) {
        *connection = rsp->data.rsp_connection_open.connection;
    }

    return rsp->data.rsp_connection_open.result;
}

sl_status_t sl_bt_connection_open_with_accept_list(uint8_t initiating_phy,
                                                   uint8_t *connection)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_open_with_accept_list_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_open_with_accept_list_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_open_with_accept_list.initiating_phy = initiating_phy;
    sl_bt_host_handle_command();

    if (connection) {
        *connection = rsp->data.rsp_connection_open_with_accept_list.connection;
    }

    return rsp->data.rsp_connection_open_with_accept_list.result;
}

sl_status_t sl_bt_connection_set_parameters(uint8_t connection,
                                            uint16_t min_interval,
                                            uint16_t max_interval,
                                            uint16_t latency,
                                            uint16_t timeout,
                                            uint16_t min_ce_length,
                                            uint16_t max_ce_length)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_set_parameters_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_set_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_set_parameters.connection = connection;
    cmd->data.cmd_connection_set_parameters.min_interval = min_interval;
    cmd->data.cmd_connection_set_parameters.max_interval = max_interval;
    cmd->data.cmd_connection_set_parameters.latency = latency;
    cmd->data.cmd_connection_set_parameters.timeout = timeout;
    cmd->data.cmd_connection_set_parameters.min_ce_length = min_ce_length;
    cmd->data.cmd_connection_set_parameters.max_ce_length = max_ce_length;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_set_parameters.result;
}

sl_status_t sl_bt_connection_set_preferred_phy(uint8_t connection,
                                               uint8_t preferred_phy,
                                               uint8_t accepted_phy)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_set_preferred_phy_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_set_preferred_phy_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_set_preferred_phy.connection = connection;
    cmd->data.cmd_connection_set_preferred_phy.preferred_phy = preferred_phy;
    cmd->data.cmd_connection_set_preferred_phy.accepted_phy = accepted_phy;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_set_preferred_phy.result;
}

sl_status_t sl_bt_connection_disable_slave_latency(uint8_t connection,
                                                   uint8_t disable)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_disable_slave_latency_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_disable_slave_latency_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_disable_slave_latency.connection = connection;
    cmd->data.cmd_connection_disable_slave_latency.disable = disable;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_disable_slave_latency.result;
}

sl_status_t sl_bt_connection_get_median_rssi(uint8_t connection, int8_t *rssi)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_get_median_rssi_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_get_median_rssi_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_get_median_rssi.connection = connection;
    sl_bt_host_handle_command();

    if (rssi) {
        *rssi = rsp->data.rsp_connection_get_median_rssi.rssi;
    }

    return rsp->data.rsp_connection_get_median_rssi.result;
}

sl_status_t sl_bt_connection_read_channel_map(uint8_t connection,
                                              size_t max_channel_map_size,
                                              size_t *channel_map_len,
                                              uint8_t *channel_map)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_read_channel_map_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_read_channel_map_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_read_channel_map.connection = connection;
    sl_bt_host_handle_command();

    if (channel_map_len) {
        *channel_map_len = rsp->data.rsp_connection_read_channel_map.channel_map.len;
    }
    if (channel_map && (rsp->data.rsp_connection_read_channel_map.channel_map.len <= max_channel_map_size)) {
        memcpy(channel_map, rsp->data.rsp_connection_read_channel_map.channel_map.data, rsp->data.rsp_connection_read_channel_map.channel_map.len);
    }

    return rsp->data.rsp_connection_read_channel_map.result;
}

sl_status_t sl_bt_connection_set_power_reporting(uint8_t connection,
                                                 uint8_t mode)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_set_power_reporting_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_set_power_reporting_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_set_power_reporting.connection = connection;
    cmd->data.cmd_connection_set_power_reporting.mode = mode;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_set_power_reporting.result;
}

sl_status_t sl_bt_connection_set_remote_power_reporting(uint8_t connection,
                                                        uint8_t mode)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_set_remote_power_reporting_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_set_remote_power_reporting_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_set_remote_power_reporting.connection = connection;
    cmd->data.cmd_connection_set_remote_power_reporting.mode = mode;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_set_remote_power_reporting.result;
}

sl_status_t sl_bt_connection_get_tx_power(uint8_t connection,
                                          uint8_t phy,
                                          int8_t *current_level,
                                          int8_t *max_level)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_get_tx_power_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_get_tx_power_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_get_tx_power.connection = connection;
    cmd->data.cmd_connection_get_tx_power.phy = phy;
    sl_bt_host_handle_command();

    if (current_level) {
        *current_level = rsp->data.rsp_connection_get_tx_power.current_level;
    }
    if (max_level) {
        *max_level = rsp->data.rsp_connection_get_tx_power.max_level;
    }

    return rsp->data.rsp_connection_get_tx_power.result;
}

sl_status_t sl_bt_connection_get_remote_tx_power(uint8_t connection,
                                                 uint8_t phy)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_get_remote_tx_power_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_get_remote_tx_power_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_get_remote_tx_power.connection = connection;
    cmd->data.cmd_connection_get_remote_tx_power.phy = phy;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_get_remote_tx_power.result;
}

sl_status_t sl_bt_connection_set_tx_power(uint8_t connection,
                                          int16_t tx_power,
                                          int16_t *tx_power_out)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_set_tx_power_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_set_tx_power_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_set_tx_power.connection = connection;
    cmd->data.cmd_connection_set_tx_power.tx_power = tx_power;
    sl_bt_host_handle_command();

    if (tx_power_out) {
        *tx_power_out = rsp->data.rsp_connection_set_tx_power.tx_power_out;
    }

    return rsp->data.rsp_connection_set_tx_power.result;
}

sl_status_t sl_bt_connection_read_remote_used_features(uint8_t connection)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_read_remote_used_features_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_read_remote_used_features_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_read_remote_used_features.connection = connection;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_read_remote_used_features.result;
}

sl_status_t sl_bt_connection_get_security_status(uint8_t connection,
                                                 uint8_t *security_mode,
                                                 uint8_t *key_size,
                                                 uint8_t *bonding_handle)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_get_security_status_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_get_security_status_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_get_security_status.connection = connection;
    sl_bt_host_handle_command();

    if (security_mode) {
        *security_mode = rsp->data.rsp_connection_get_security_status.security_mode;
    }
    if (key_size) {
        *key_size = rsp->data.rsp_connection_get_security_status.key_size;
    }
    if (bonding_handle) {
        *bonding_handle = rsp->data.rsp_connection_get_security_status.bonding_handle;
    }

    return rsp->data.rsp_connection_get_security_status.result;
}

sl_status_t sl_bt_connection_set_data_length(uint8_t connection,
                                             uint16_t tx_data_len,
                                             uint16_t tx_time_us)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_set_data_length_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_set_data_length_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_set_data_length.connection = connection;
    cmd->data.cmd_connection_set_data_length.tx_data_len = tx_data_len;
    cmd->data.cmd_connection_set_data_length.tx_time_us = tx_time_us;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_set_data_length.result;
}

sl_status_t sl_bt_connection_read_statistics(uint8_t connection, uint8_t reset)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_read_statistics_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_read_statistics_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_read_statistics.connection = connection;
    cmd->data.cmd_connection_read_statistics.reset = reset;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_read_statistics.result;
}

sl_status_t sl_bt_connection_get_scheduling_details(uint8_t connection,
                                                    uint32_t *access_address,
                                                    uint8_t *role,
                                                    uint32_t *crc_init,
                                                    uint16_t *interval,
                                                    uint16_t *supervision_timeout,
                                                    uint8_t *central_clock_accuracy,
                                                    uint8_t *central_phy,
                                                    uint8_t *peripheral_phy,
                                                    uint8_t *channel_selection_algorithm,
                                                    uint8_t *hop,
                                                    sl_bt_connection_channel_map_t *channel_map,
                                                    uint8_t *channel,
                                                    uint16_t *event_counter,
                                                    uint32_t *start_time_us)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_get_scheduling_details_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_get_scheduling_details_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_get_scheduling_details.connection = connection;
    sl_bt_host_handle_command();

    if (access_address) {
        *access_address = rsp->data.rsp_connection_get_scheduling_details.access_address;
    }
    if (role) {
        *role = rsp->data.rsp_connection_get_scheduling_details.role;
    }
    if (crc_init) {
        *crc_init = rsp->data.rsp_connection_get_scheduling_details.crc_init;
    }
    if (interval) {
        *interval = rsp->data.rsp_connection_get_scheduling_details.interval;
    }
    if (supervision_timeout) {
        *supervision_timeout = rsp->data.rsp_connection_get_scheduling_details.supervision_timeout;
    }
    if (central_clock_accuracy) {
        *central_clock_accuracy = rsp->data.rsp_connection_get_scheduling_details.central_clock_accuracy;
    }
    if (central_phy) {
        *central_phy = rsp->data.rsp_connection_get_scheduling_details.central_phy;
    }
    if (peripheral_phy) {
        *peripheral_phy = rsp->data.rsp_connection_get_scheduling_details.peripheral_phy;
    }
    if (channel_selection_algorithm) {
        *channel_selection_algorithm = rsp->data.rsp_connection_get_scheduling_details.channel_selection_algorithm;
    }
    if (hop) {
        *hop = rsp->data.rsp_connection_get_scheduling_details.hop;
    }
    if (channel_map) {
        memcpy(channel_map, &rsp->data.rsp_connection_get_scheduling_details.channel_map, sizeof(sl_bt_connection_channel_map_t));
    }
    if (channel) {
        *channel = rsp->data.rsp_connection_get_scheduling_details.channel;
    }
    if (event_counter) {
        *event_counter = rsp->data.rsp_connection_get_scheduling_details.event_counter;
    }
    if (start_time_us) {
        *start_time_us = rsp->data.rsp_connection_get_scheduling_details.start_time_us;
    }

    return rsp->data.rsp_connection_get_scheduling_details.result;
}

sl_status_t sl_bt_connection_get_remote_address(uint8_t connection,
                                                bd_addr *address,
                                                uint8_t *address_type)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_get_remote_address_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_get_remote_address_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_get_remote_address.connection = connection;
    sl_bt_host_handle_command();

    if (address) {
        memcpy(address, &rsp->data.rsp_connection_get_remote_address.address, sizeof(bd_addr));
    }
    if (address_type) {
        *address_type = rsp->data.rsp_connection_get_remote_address.address_type;
    }

    return rsp->data.rsp_connection_get_remote_address.result;
}

sl_status_t sl_bt_connection_request_subrate(uint8_t connection,
                                             uint16_t min_subrate,
                                             uint16_t max_subrate,
                                             uint16_t max_latency,
                                             uint16_t continuation_number,
                                             uint16_t timeout)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_request_subrate_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_request_subrate_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_request_subrate.connection = connection;
    cmd->data.cmd_connection_request_subrate.min_subrate = min_subrate;
    cmd->data.cmd_connection_request_subrate.max_subrate = max_subrate;
    cmd->data.cmd_connection_request_subrate.max_latency = max_latency;
    cmd->data.cmd_connection_request_subrate.continuation_number = continuation_number;
    cmd->data.cmd_connection_request_subrate.timeout = timeout;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_request_subrate.result;
}

sl_status_t sl_bt_connection_get_state(uint8_t connection, uint8_t *state)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_get_state_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_get_state_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_get_state.connection = connection;
    sl_bt_host_handle_command();

    if (state) {
        *state = rsp->data.rsp_connection_get_state.state;
    }

    return rsp->data.rsp_connection_get_state.result;
}

sl_status_t sl_bt_connection_close(uint8_t connection)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_close_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_close_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_close.connection = connection;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_close.result;
}

sl_status_t sl_bt_connection_forcefully_close(uint8_t connection)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_forcefully_close_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_class_id,
                                       sli_bt_connection_forcefully_close_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_forcefully_close.connection = connection;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_forcefully_close.result;
}

sl_status_t sl_bt_gatt_set_max_mtu(uint16_t max_mtu, uint16_t *max_mtu_out)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_set_max_mtu_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_set_max_mtu_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_set_max_mtu.max_mtu = max_mtu;
    sl_bt_host_handle_command();

    if (max_mtu_out) {
        *max_mtu_out = rsp->data.rsp_gatt_set_max_mtu.max_mtu_out;
    }

    return rsp->data.rsp_gatt_set_max_mtu.result;
}

sl_status_t sl_bt_gatt_discover_primary_services(uint8_t connection)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_discover_primary_services_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_discover_primary_services_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_discover_primary_services.connection = connection;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_discover_primary_services.result;
}

sl_status_t sl_bt_gatt_discover_primary_services_by_uuid(uint8_t connection,
                                                         size_t uuid_len,
                                                         const uint8_t* uuid)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_discover_primary_services_by_uuid_t) + uuid_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_discover_primary_services_by_uuid_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_discover_primary_services_by_uuid.connection = connection;
    cmd->data.cmd_gatt_discover_primary_services_by_uuid.uuid.len = uuid_len;
    memcpy(cmd->data.cmd_gatt_discover_primary_services_by_uuid.uuid.data, uuid, uuid_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_discover_primary_services_by_uuid.result;
}

sl_status_t sl_bt_gatt_find_included_services(uint8_t connection,
                                              uint32_t service)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_find_included_services_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_find_included_services_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_find_included_services.connection = connection;
    cmd->data.cmd_gatt_find_included_services.service = service;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_find_included_services.result;
}

sl_status_t sl_bt_gatt_discover_characteristics(uint8_t connection,
                                                uint32_t service)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_discover_characteristics_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_discover_characteristics_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_discover_characteristics.connection = connection;
    cmd->data.cmd_gatt_discover_characteristics.service = service;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_discover_characteristics.result;
}

sl_status_t sl_bt_gatt_discover_characteristics_by_uuid(uint8_t connection,
                                                        uint32_t service,
                                                        size_t uuid_len,
                                                        const uint8_t* uuid)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_discover_characteristics_by_uuid_t) + uuid_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_discover_characteristics_by_uuid_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_discover_characteristics_by_uuid.connection = connection;
    cmd->data.cmd_gatt_discover_characteristics_by_uuid.service = service;
    cmd->data.cmd_gatt_discover_characteristics_by_uuid.uuid.len = uuid_len;
    memcpy(cmd->data.cmd_gatt_discover_characteristics_by_uuid.uuid.data, uuid, uuid_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_discover_characteristics_by_uuid.result;
}

sl_status_t sl_bt_gatt_discover_descriptors(uint8_t connection,
                                            uint16_t characteristic)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_discover_descriptors_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_discover_descriptors_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_discover_descriptors.connection = connection;
    cmd->data.cmd_gatt_discover_descriptors.characteristic = characteristic;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_discover_descriptors.result;
}

sl_status_t sl_bt_gatt_discover_characteristic_descriptors(uint8_t connection,
                                                           uint16_t start,
                                                           uint16_t end)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_discover_characteristic_descriptors_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_discover_characteristic_descriptors_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_discover_characteristic_descriptors.connection = connection;
    cmd->data.cmd_gatt_discover_characteristic_descriptors.start = start;
    cmd->data.cmd_gatt_discover_characteristic_descriptors.end = end;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_discover_characteristic_descriptors.result;
}

sl_status_t sl_bt_gatt_set_characteristic_notification(uint8_t connection,
                                                       uint16_t characteristic,
                                                       uint8_t flags)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_set_characteristic_notification_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_set_characteristic_notification_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_set_characteristic_notification.connection = connection;
    cmd->data.cmd_gatt_set_characteristic_notification.characteristic = characteristic;
    cmd->data.cmd_gatt_set_characteristic_notification.flags = flags;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_set_characteristic_notification.result;
}

sl_status_t sl_bt_gatt_send_characteristic_confirmation(uint8_t connection)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_send_characteristic_confirmation_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_send_characteristic_confirmation_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_send_characteristic_confirmation.connection = connection;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_send_characteristic_confirmation.result;
}

sl_status_t sl_bt_gatt_read_characteristic_value(uint8_t connection,
                                                 uint16_t characteristic)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_read_characteristic_value_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_read_characteristic_value_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_read_characteristic_value.connection = connection;
    cmd->data.cmd_gatt_read_characteristic_value.characteristic = characteristic;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_read_characteristic_value.result;
}

sl_status_t sl_bt_gatt_read_characteristic_value_from_offset(uint8_t connection,
                                                             uint16_t characteristic,
                                                             uint16_t offset,
                                                             uint16_t maxlen)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_read_characteristic_value_from_offset_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_read_characteristic_value_from_offset_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_read_characteristic_value_from_offset.connection = connection;
    cmd->data.cmd_gatt_read_characteristic_value_from_offset.characteristic = characteristic;
    cmd->data.cmd_gatt_read_characteristic_value_from_offset.offset = offset;
    cmd->data.cmd_gatt_read_characteristic_value_from_offset.maxlen = maxlen;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_read_characteristic_value_from_offset.result;
}

sl_status_t sl_bt_gatt_read_multiple_characteristic_values(uint8_t connection,
                                                           size_t characteristic_list_len,
                                                           const uint8_t* characteristic_list)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_read_multiple_characteristic_values_t) + characteristic_list_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_read_multiple_characteristic_values_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_read_multiple_characteristic_values.connection = connection;
    cmd->data.cmd_gatt_read_multiple_characteristic_values.characteristic_list.len = characteristic_list_len;
    memcpy(cmd->data.cmd_gatt_read_multiple_characteristic_values.characteristic_list.data, characteristic_list, characteristic_list_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_read_multiple_characteristic_values.result;
}

sl_status_t sl_bt_gatt_read_variable_length_characteristic_values(uint8_t connection,
                                                                  size_t characteristic_list_len,
                                                                  const uint8_t* characteristic_list)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_read_variable_length_characteristic_values_t) + characteristic_list_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_read_variable_length_characteristic_values_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_read_variable_length_characteristic_values.connection = connection;
    cmd->data.cmd_gatt_read_variable_length_characteristic_values.characteristic_list.len = characteristic_list_len;
    memcpy(cmd->data.cmd_gatt_read_variable_length_characteristic_values.characteristic_list.data, characteristic_list, characteristic_list_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_read_variable_length_characteristic_values.result;
}

sl_status_t sl_bt_gatt_read_characteristic_value_by_uuid(uint8_t connection,
                                                         uint32_t service,
                                                         size_t uuid_len,
                                                         const uint8_t* uuid)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_read_characteristic_value_by_uuid_t) + uuid_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_read_characteristic_value_by_uuid_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_read_characteristic_value_by_uuid.connection = connection;
    cmd->data.cmd_gatt_read_characteristic_value_by_uuid.service = service;
    cmd->data.cmd_gatt_read_characteristic_value_by_uuid.uuid.len = uuid_len;
    memcpy(cmd->data.cmd_gatt_read_characteristic_value_by_uuid.uuid.data, uuid, uuid_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_read_characteristic_value_by_uuid.result;
}

sl_status_t sl_bt_gatt_write_characteristic_value(uint8_t connection,
                                                  uint16_t characteristic,
                                                  size_t value_len,
                                                  const uint8_t* value)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_write_characteristic_value_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_write_characteristic_value_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_write_characteristic_value.connection = connection;
    cmd->data.cmd_gatt_write_characteristic_value.characteristic = characteristic;
    cmd->data.cmd_gatt_write_characteristic_value.value.len = value_len;
    memcpy(cmd->data.cmd_gatt_write_characteristic_value.value.data, value, value_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_write_characteristic_value.result;
}

sl_status_t sl_bt_gatt_write_characteristic_value_without_response(uint8_t connection,
                                                                   uint16_t characteristic,
                                                                   size_t value_len,
                                                                   const uint8_t* value,
                                                                   uint16_t *sent_len)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_write_characteristic_value_without_response_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_write_characteristic_value_without_response_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_write_characteristic_value_without_response.connection = connection;
    cmd->data.cmd_gatt_write_characteristic_value_without_response.characteristic = characteristic;
    cmd->data.cmd_gatt_write_characteristic_value_without_response.value.len = value_len;
    memcpy(cmd->data.cmd_gatt_write_characteristic_value_without_response.value.data, value, value_len);
    sl_bt_host_handle_command();

    if (sent_len) {
        *sent_len = rsp->data.rsp_gatt_write_characteristic_value_without_response.sent_len;
    }

    return rsp->data.rsp_gatt_write_characteristic_value_without_response.result;
}

sl_status_t sl_bt_gatt_prepare_characteristic_value_write(uint8_t connection,
                                                          uint16_t characteristic,
                                                          uint16_t offset,
                                                          size_t value_len,
                                                          const uint8_t* value,
                                                          uint16_t *sent_len)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_prepare_characteristic_value_write_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_prepare_characteristic_value_write_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_prepare_characteristic_value_write.connection = connection;
    cmd->data.cmd_gatt_prepare_characteristic_value_write.characteristic = characteristic;
    cmd->data.cmd_gatt_prepare_characteristic_value_write.offset = offset;
    cmd->data.cmd_gatt_prepare_characteristic_value_write.value.len = value_len;
    memcpy(cmd->data.cmd_gatt_prepare_characteristic_value_write.value.data, value, value_len);
    sl_bt_host_handle_command();

    if (sent_len) {
        *sent_len = rsp->data.rsp_gatt_prepare_characteristic_value_write.sent_len;
    }

    return rsp->data.rsp_gatt_prepare_characteristic_value_write.result;
}

sl_status_t sl_bt_gatt_prepare_characteristic_value_reliable_write(uint8_t connection,
                                                                   uint16_t characteristic,
                                                                   uint16_t offset,
                                                                   size_t value_len,
                                                                   const uint8_t* value,
                                                                   uint16_t *sent_len)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_prepare_characteristic_value_reliable_write_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_prepare_characteristic_value_reliable_write_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_prepare_characteristic_value_reliable_write.connection = connection;
    cmd->data.cmd_gatt_prepare_characteristic_value_reliable_write.characteristic = characteristic;
    cmd->data.cmd_gatt_prepare_characteristic_value_reliable_write.offset = offset;
    cmd->data.cmd_gatt_prepare_characteristic_value_reliable_write.value.len = value_len;
    memcpy(cmd->data.cmd_gatt_prepare_characteristic_value_reliable_write.value.data, value, value_len);
    sl_bt_host_handle_command();

    if (sent_len) {
        *sent_len = rsp->data.rsp_gatt_prepare_characteristic_value_reliable_write.sent_len;
    }

    return rsp->data.rsp_gatt_prepare_characteristic_value_reliable_write.result;
}

sl_status_t sl_bt_gatt_execute_characteristic_value_write(uint8_t connection,
                                                          uint8_t flags)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_execute_characteristic_value_write_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_execute_characteristic_value_write_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_execute_characteristic_value_write.connection = connection;
    cmd->data.cmd_gatt_execute_characteristic_value_write.flags = flags;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_execute_characteristic_value_write.result;
}

sl_status_t sl_bt_gatt_read_descriptor_value(uint8_t connection,
                                             uint16_t descriptor)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_read_descriptor_value_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_read_descriptor_value_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_read_descriptor_value.connection = connection;
    cmd->data.cmd_gatt_read_descriptor_value.descriptor = descriptor;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_read_descriptor_value.result;
}

sl_status_t sl_bt_gatt_write_descriptor_value(uint8_t connection,
                                              uint16_t descriptor,
                                              size_t value_len,
                                              const uint8_t* value)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_write_descriptor_value_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_write_descriptor_value_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_write_descriptor_value.connection = connection;
    cmd->data.cmd_gatt_write_descriptor_value.descriptor = descriptor;
    cmd->data.cmd_gatt_write_descriptor_value.value.len = value_len;
    memcpy(cmd->data.cmd_gatt_write_descriptor_value.value.data, value, value_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_write_descriptor_value.result;
}

sl_status_t sl_bt_gatt_get_mtu(uint8_t connection, uint16_t *mtu)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_get_mtu_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_class_id,
                                       sli_bt_gatt_get_mtu_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_get_mtu.connection = connection;
    sl_bt_host_handle_command();

    if (mtu) {
        *mtu = rsp->data.rsp_gatt_get_mtu.mtu;
    }

    return rsp->data.rsp_gatt_get_mtu.result;
}

sl_status_t sl_bt_gattdb_new_session(uint16_t *session)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_new_session_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    if (session) {
        *session = rsp->data.rsp_gattdb_new_session.session;
    }

    return rsp->data.rsp_gattdb_new_session.result;
}

sl_status_t sl_bt_gattdb_add_service(uint16_t session,
                                     uint8_t type,
                                     uint8_t property,
                                     size_t uuid_len,
                                     const uint8_t* uuid,
                                     uint16_t *service)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_add_service_t) + uuid_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_add_service_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_add_service.session = session;
    cmd->data.cmd_gattdb_add_service.type = type;
    cmd->data.cmd_gattdb_add_service.property = property;
    cmd->data.cmd_gattdb_add_service.uuid.len = uuid_len;
    memcpy(cmd->data.cmd_gattdb_add_service.uuid.data, uuid, uuid_len);
    sl_bt_host_handle_command();

    if (service) {
        *service = rsp->data.rsp_gattdb_add_service.service;
    }

    return rsp->data.rsp_gattdb_add_service.result;
}

sl_status_t sl_bt_gattdb_remove_service(uint16_t session, uint16_t service)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_remove_service_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_remove_service_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_remove_service.session = session;
    cmd->data.cmd_gattdb_remove_service.service = service;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gattdb_remove_service.result;
}

sl_status_t sl_bt_gattdb_add_included_service(uint16_t session,
                                              uint16_t service,
                                              uint16_t included_service,
                                              uint16_t *attribute)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_add_included_service_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_add_included_service_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_add_included_service.session = session;
    cmd->data.cmd_gattdb_add_included_service.service = service;
    cmd->data.cmd_gattdb_add_included_service.included_service = included_service;
    sl_bt_host_handle_command();

    if (attribute) {
        *attribute = rsp->data.rsp_gattdb_add_included_service.attribute;
    }

    return rsp->data.rsp_gattdb_add_included_service.result;
}

sl_status_t sl_bt_gattdb_remove_included_service(uint16_t session,
                                                 uint16_t attribute)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_remove_included_service_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_remove_included_service_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_remove_included_service.session = session;
    cmd->data.cmd_gattdb_remove_included_service.attribute = attribute;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gattdb_remove_included_service.result;
}

sl_status_t sl_bt_gattdb_add_uuid16_characteristic(uint16_t session,
                                                   uint16_t service,
                                                   uint16_t property,
                                                   uint16_t security,
                                                   uint8_t flag,
                                                   sl_bt_uuid_16_t uuid,
                                                   uint8_t value_type,
                                                   uint16_t maxlen,
                                                   size_t value_len,
                                                   const uint8_t* value,
                                                   uint16_t *characteristic)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_add_uuid16_characteristic_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_add_uuid16_characteristic_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_add_uuid16_characteristic.session = session;
    cmd->data.cmd_gattdb_add_uuid16_characteristic.service = service;
    cmd->data.cmd_gattdb_add_uuid16_characteristic.property = property;
    cmd->data.cmd_gattdb_add_uuid16_characteristic.security = security;
    cmd->data.cmd_gattdb_add_uuid16_characteristic.flag = flag;
    cmd->data.cmd_gattdb_add_uuid16_characteristic.uuid = uuid;
    cmd->data.cmd_gattdb_add_uuid16_characteristic.value_type = value_type;
    cmd->data.cmd_gattdb_add_uuid16_characteristic.maxlen = maxlen;
    cmd->data.cmd_gattdb_add_uuid16_characteristic.value.len = value_len;
    memcpy(cmd->data.cmd_gattdb_add_uuid16_characteristic.value.data, value, value_len);
    sl_bt_host_handle_command();

    if (characteristic) {
        *characteristic = rsp->data.rsp_gattdb_add_uuid16_characteristic.characteristic;
    }

    return rsp->data.rsp_gattdb_add_uuid16_characteristic.result;
}

sl_status_t sl_bt_gattdb_add_uuid128_characteristic(uint16_t session,
                                                    uint16_t service,
                                                    uint16_t property,
                                                    uint16_t security,
                                                    uint8_t flag,
                                                    uuid_128 uuid,
                                                    uint8_t value_type,
                                                    uint16_t maxlen,
                                                    size_t value_len,
                                                    const uint8_t* value,
                                                    uint16_t *characteristic)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_add_uuid128_characteristic_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_add_uuid128_characteristic_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_add_uuid128_characteristic.session = session;
    cmd->data.cmd_gattdb_add_uuid128_characteristic.service = service;
    cmd->data.cmd_gattdb_add_uuid128_characteristic.property = property;
    cmd->data.cmd_gattdb_add_uuid128_characteristic.security = security;
    cmd->data.cmd_gattdb_add_uuid128_characteristic.flag = flag;
    cmd->data.cmd_gattdb_add_uuid128_characteristic.uuid = uuid;
    cmd->data.cmd_gattdb_add_uuid128_characteristic.value_type = value_type;
    cmd->data.cmd_gattdb_add_uuid128_characteristic.maxlen = maxlen;
    cmd->data.cmd_gattdb_add_uuid128_characteristic.value.len = value_len;
    memcpy(cmd->data.cmd_gattdb_add_uuid128_characteristic.value.data, value, value_len);
    sl_bt_host_handle_command();

    if (characteristic) {
        *characteristic = rsp->data.rsp_gattdb_add_uuid128_characteristic.characteristic;
    }

    return rsp->data.rsp_gattdb_add_uuid128_characteristic.result;
}

sl_status_t sl_bt_gattdb_remove_characteristic(uint16_t session,
                                               uint16_t characteristic)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_remove_characteristic_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_remove_characteristic_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_remove_characteristic.session = session;
    cmd->data.cmd_gattdb_remove_characteristic.characteristic = characteristic;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gattdb_remove_characteristic.result;
}

sl_status_t sl_bt_gattdb_add_uuid16_descriptor(uint16_t session,
                                               uint16_t characteristic,
                                               uint16_t property,
                                               uint16_t security,
                                               sl_bt_uuid_16_t uuid,
                                               uint8_t value_type,
                                               uint16_t maxlen,
                                               size_t value_len,
                                               const uint8_t* value,
                                               uint16_t *descriptor)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_add_uuid16_descriptor_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_add_uuid16_descriptor_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_add_uuid16_descriptor.session = session;
    cmd->data.cmd_gattdb_add_uuid16_descriptor.characteristic = characteristic;
    cmd->data.cmd_gattdb_add_uuid16_descriptor.property = property;
    cmd->data.cmd_gattdb_add_uuid16_descriptor.security = security;
    cmd->data.cmd_gattdb_add_uuid16_descriptor.uuid = uuid;
    cmd->data.cmd_gattdb_add_uuid16_descriptor.value_type = value_type;
    cmd->data.cmd_gattdb_add_uuid16_descriptor.maxlen = maxlen;
    cmd->data.cmd_gattdb_add_uuid16_descriptor.value.len = value_len;
    memcpy(cmd->data.cmd_gattdb_add_uuid16_descriptor.value.data, value, value_len);
    sl_bt_host_handle_command();

    if (descriptor) {
        *descriptor = rsp->data.rsp_gattdb_add_uuid16_descriptor.descriptor;
    }

    return rsp->data.rsp_gattdb_add_uuid16_descriptor.result;
}

sl_status_t sl_bt_gattdb_add_uuid128_descriptor(uint16_t session,
                                                uint16_t characteristic,
                                                uint16_t property,
                                                uint16_t security,
                                                uuid_128 uuid,
                                                uint8_t value_type,
                                                uint16_t maxlen,
                                                size_t value_len,
                                                const uint8_t* value,
                                                uint16_t *descriptor)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_add_uuid128_descriptor_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_add_uuid128_descriptor_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_add_uuid128_descriptor.session = session;
    cmd->data.cmd_gattdb_add_uuid128_descriptor.characteristic = characteristic;
    cmd->data.cmd_gattdb_add_uuid128_descriptor.property = property;
    cmd->data.cmd_gattdb_add_uuid128_descriptor.security = security;
    cmd->data.cmd_gattdb_add_uuid128_descriptor.uuid = uuid;
    cmd->data.cmd_gattdb_add_uuid128_descriptor.value_type = value_type;
    cmd->data.cmd_gattdb_add_uuid128_descriptor.maxlen = maxlen;
    cmd->data.cmd_gattdb_add_uuid128_descriptor.value.len = value_len;
    memcpy(cmd->data.cmd_gattdb_add_uuid128_descriptor.value.data, value, value_len);
    sl_bt_host_handle_command();

    if (descriptor) {
        *descriptor = rsp->data.rsp_gattdb_add_uuid128_descriptor.descriptor;
    }

    return rsp->data.rsp_gattdb_add_uuid128_descriptor.result;
}

sl_status_t sl_bt_gattdb_remove_descriptor(uint16_t session,
                                           uint16_t descriptor)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_remove_descriptor_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_remove_descriptor_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_remove_descriptor.session = session;
    cmd->data.cmd_gattdb_remove_descriptor.descriptor = descriptor;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gattdb_remove_descriptor.result;
}

sl_status_t sl_bt_gattdb_start_service(uint16_t session, uint16_t service)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_start_service_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_start_service_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_start_service.session = session;
    cmd->data.cmd_gattdb_start_service.service = service;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gattdb_start_service.result;
}

sl_status_t sl_bt_gattdb_stop_service(uint16_t session, uint16_t service)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_stop_service_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_stop_service_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_stop_service.session = session;
    cmd->data.cmd_gattdb_stop_service.service = service;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gattdb_stop_service.result;
}

sl_status_t sl_bt_gattdb_start_characteristic(uint16_t session,
                                              uint16_t characteristic)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_start_characteristic_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_start_characteristic_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_start_characteristic.session = session;
    cmd->data.cmd_gattdb_start_characteristic.characteristic = characteristic;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gattdb_start_characteristic.result;
}

sl_status_t sl_bt_gattdb_stop_characteristic(uint16_t session,
                                             uint16_t characteristic)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_stop_characteristic_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_stop_characteristic_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_stop_characteristic.session = session;
    cmd->data.cmd_gattdb_stop_characteristic.characteristic = characteristic;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gattdb_stop_characteristic.result;
}

sl_status_t sl_bt_gattdb_commit(uint16_t session)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_commit_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_commit_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_commit.session = session;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gattdb_commit.result;
}

sl_status_t sl_bt_gattdb_abort(uint16_t session)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_abort_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_abort_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_abort.session = session;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gattdb_abort.result;
}

sl_status_t sl_bt_gattdb_get_attribute_state(uint16_t attribute,
                                             uint8_t *state)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gattdb_get_attribute_state_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gattdb_class_id,
                                       sli_bt_gattdb_get_attribute_state_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gattdb_get_attribute_state.attribute = attribute;
    sl_bt_host_handle_command();

    if (state) {
        *state = rsp->data.rsp_gattdb_get_attribute_state.state;
    }

    return rsp->data.rsp_gattdb_get_attribute_state.result;
}

sl_status_t sl_bt_gatt_server_set_max_mtu(uint16_t max_mtu,
                                          uint16_t *max_mtu_out)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_set_max_mtu_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_set_max_mtu_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_set_max_mtu.max_mtu = max_mtu;
    sl_bt_host_handle_command();

    if (max_mtu_out) {
        *max_mtu_out = rsp->data.rsp_gatt_server_set_max_mtu.max_mtu_out;
    }

    return rsp->data.rsp_gatt_server_set_max_mtu.result;
}

sl_status_t sl_bt_gatt_server_get_mtu(uint8_t connection, uint16_t *mtu)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_get_mtu_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_get_mtu_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_get_mtu.connection = connection;
    sl_bt_host_handle_command();

    if (mtu) {
        *mtu = rsp->data.rsp_gatt_server_get_mtu.mtu;
    }

    return rsp->data.rsp_gatt_server_get_mtu.result;
}

sl_status_t sl_bt_gatt_server_find_attribute(uint16_t start,
                                             size_t type_len,
                                             const uint8_t* type,
                                             uint16_t *attribute)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_find_attribute_t) + type_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_find_attribute_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_find_attribute.start = start;
    cmd->data.cmd_gatt_server_find_attribute.type.len = type_len;
    memcpy(cmd->data.cmd_gatt_server_find_attribute.type.data, type, type_len);
    sl_bt_host_handle_command();

    if (attribute) {
        *attribute = rsp->data.rsp_gatt_server_find_attribute.attribute;
    }

    return rsp->data.rsp_gatt_server_find_attribute.result;
}

sl_status_t sl_bt_gatt_server_find_primary_service(uint16_t start,
                                                   size_t uuid_len,
                                                   const uint8_t* uuid,
                                                   uint16_t *start_out,
                                                   uint16_t *end_out)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_find_primary_service_t) + uuid_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_find_primary_service_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_find_primary_service.start = start;
    cmd->data.cmd_gatt_server_find_primary_service.uuid.len = uuid_len;
    memcpy(cmd->data.cmd_gatt_server_find_primary_service.uuid.data, uuid, uuid_len);
    sl_bt_host_handle_command();

    if (start_out) {
        *start_out = rsp->data.rsp_gatt_server_find_primary_service.start_out;
    }
    if (end_out) {
        *end_out = rsp->data.rsp_gatt_server_find_primary_service.end_out;
    }

    return rsp->data.rsp_gatt_server_find_primary_service.result;
}

sl_status_t sl_bt_gatt_server_read_attribute_value(uint16_t attribute,
                                                   uint16_t offset,
                                                   size_t max_value_size,
                                                   size_t *value_len,
                                                   uint8_t *value)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_read_attribute_value_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_read_attribute_value_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_read_attribute_value.attribute = attribute;
    cmd->data.cmd_gatt_server_read_attribute_value.offset = offset;
    sl_bt_host_handle_command();

    if (value_len) {
        *value_len = rsp->data.rsp_gatt_server_read_attribute_value.value.len;
    }
    if (value && (rsp->data.rsp_gatt_server_read_attribute_value.value.len <= max_value_size)) {
        memcpy(value, rsp->data.rsp_gatt_server_read_attribute_value.value.data, rsp->data.rsp_gatt_server_read_attribute_value.value.len);
    }

    return rsp->data.rsp_gatt_server_read_attribute_value.result;
}

sl_status_t sl_bt_gatt_server_read_attribute_type(uint16_t attribute,
                                                  size_t max_type_size,
                                                  size_t *type_len,
                                                  uint8_t *type)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_read_attribute_type_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_read_attribute_type_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_read_attribute_type.attribute = attribute;
    sl_bt_host_handle_command();

    if (type_len) {
        *type_len = rsp->data.rsp_gatt_server_read_attribute_type.type.len;
    }
    if (type && (rsp->data.rsp_gatt_server_read_attribute_type.type.len <= max_type_size)) {
        memcpy(type, rsp->data.rsp_gatt_server_read_attribute_type.type.data, rsp->data.rsp_gatt_server_read_attribute_type.type.len);
    }

    return rsp->data.rsp_gatt_server_read_attribute_type.result;
}

sl_status_t sl_bt_gatt_server_read_attribute_properties(uint16_t attribute,
                                                        uint8_t *category,
                                                        uint16_t *security,
                                                        uint16_t *properties,
                                                        uint8_t *value_type,
                                                        uint16_t *len,
                                                        uint16_t *max_writable_len)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_read_attribute_properties_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_read_attribute_properties_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_read_attribute_properties.attribute = attribute;
    sl_bt_host_handle_command();

    if (category) {
        *category = rsp->data.rsp_gatt_server_read_attribute_properties.category;
    }
    if (security) {
        *security = rsp->data.rsp_gatt_server_read_attribute_properties.security;
    }
    if (properties) {
        *properties = rsp->data.rsp_gatt_server_read_attribute_properties.properties;
    }
    if (value_type) {
        *value_type = rsp->data.rsp_gatt_server_read_attribute_properties.value_type;
    }
    if (len) {
        *len = rsp->data.rsp_gatt_server_read_attribute_properties.len;
    }
    if (max_writable_len) {
        *max_writable_len = rsp->data.rsp_gatt_server_read_attribute_properties.max_writable_len;
    }

    return rsp->data.rsp_gatt_server_read_attribute_properties.result;
}

sl_status_t sl_bt_gatt_server_write_attribute_value(uint16_t attribute,
                                                    uint16_t offset,
                                                    size_t value_len,
                                                    const uint8_t* value)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_write_attribute_value_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_write_attribute_value_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_write_attribute_value.attribute = attribute;
    cmd->data.cmd_gatt_server_write_attribute_value.offset = offset;
    cmd->data.cmd_gatt_server_write_attribute_value.value.len = value_len;
    memcpy(cmd->data.cmd_gatt_server_write_attribute_value.value.data, value, value_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_server_write_attribute_value.result;
}

sl_status_t sl_bt_gatt_server_send_user_read_response(uint8_t connection,
                                                      uint16_t characteristic,
                                                      uint8_t att_errorcode,
                                                      size_t value_len,
                                                      const uint8_t* value,
                                                      uint16_t *sent_len)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_send_user_read_response_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_send_user_read_response_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_send_user_read_response.connection = connection;
    cmd->data.cmd_gatt_server_send_user_read_response.characteristic = characteristic;
    cmd->data.cmd_gatt_server_send_user_read_response.att_errorcode = att_errorcode;
    cmd->data.cmd_gatt_server_send_user_read_response.value.len = value_len;
    memcpy(cmd->data.cmd_gatt_server_send_user_read_response.value.data, value, value_len);
    sl_bt_host_handle_command();

    if (sent_len) {
        *sent_len = rsp->data.rsp_gatt_server_send_user_read_response.sent_len;
    }

    return rsp->data.rsp_gatt_server_send_user_read_response.result;
}

sl_status_t sl_bt_gatt_server_send_user_write_response(uint8_t connection,
                                                       uint16_t characteristic,
                                                       uint8_t att_errorcode)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_send_user_write_response_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_send_user_write_response_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_send_user_write_response.connection = connection;
    cmd->data.cmd_gatt_server_send_user_write_response.characteristic = characteristic;
    cmd->data.cmd_gatt_server_send_user_write_response.att_errorcode = att_errorcode;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_server_send_user_write_response.result;
}

sl_status_t sl_bt_gatt_server_send_notification(uint8_t connection,
                                                uint16_t characteristic,
                                                size_t value_len,
                                                const uint8_t* value)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_send_notification_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_send_notification_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_send_notification.connection = connection;
    cmd->data.cmd_gatt_server_send_notification.characteristic = characteristic;
    cmd->data.cmd_gatt_server_send_notification.value.len = value_len;
    memcpy(cmd->data.cmd_gatt_server_send_notification.value.data, value, value_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_server_send_notification.result;
}

sl_status_t sl_bt_gatt_server_send_notification_with_options(uint8_t connection,
                                                             uint16_t characteristic,
                                                             uint32_t options,
                                                             size_t value_len,
                                                             const uint8_t* value)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_send_notification_with_options_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_send_notification_with_options_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_send_notification_with_options.connection = connection;
    cmd->data.cmd_gatt_server_send_notification_with_options.characteristic = characteristic;
    cmd->data.cmd_gatt_server_send_notification_with_options.options = options;
    cmd->data.cmd_gatt_server_send_notification_with_options.value.len = value_len;
    memcpy(cmd->data.cmd_gatt_server_send_notification_with_options.value.data, value, value_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_server_send_notification_with_options.result;
}

sl_status_t sl_bt_gatt_server_send_indication(uint8_t connection,
                                              uint16_t characteristic,
                                              size_t value_len,
                                              const uint8_t* value)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_send_indication_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_send_indication_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_send_indication.connection = connection;
    cmd->data.cmd_gatt_server_send_indication.characteristic = characteristic;
    cmd->data.cmd_gatt_server_send_indication.value.len = value_len;
    memcpy(cmd->data.cmd_gatt_server_send_indication.value.data, value, value_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_server_send_indication.result;
}

sl_status_t sl_bt_gatt_server_send_indication_with_options(uint8_t connection,
                                                           uint16_t characteristic,
                                                           uint32_t options,
                                                           size_t value_len,
                                                           const uint8_t* value)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_send_indication_with_options_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_send_indication_with_options_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_send_indication_with_options.connection = connection;
    cmd->data.cmd_gatt_server_send_indication_with_options.characteristic = characteristic;
    cmd->data.cmd_gatt_server_send_indication_with_options.options = options;
    cmd->data.cmd_gatt_server_send_indication_with_options.value.len = value_len;
    memcpy(cmd->data.cmd_gatt_server_send_indication_with_options.value.data, value, value_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_server_send_indication_with_options.result;
}

sl_status_t sl_bt_gatt_server_notify_all(uint16_t characteristic,
                                         size_t value_len,
                                         const uint8_t* value)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_notify_all_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_notify_all_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_notify_all.characteristic = characteristic;
    cmd->data.cmd_gatt_server_notify_all.value.len = value_len;
    memcpy(cmd->data.cmd_gatt_server_notify_all.value.data, value, value_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_server_notify_all.result;
}

sl_status_t sl_bt_gatt_server_read_client_configuration(uint8_t connection,
                                                        uint16_t characteristic,
                                                        uint16_t *client_config_flags)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_read_client_configuration_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_read_client_configuration_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_read_client_configuration.connection = connection;
    cmd->data.cmd_gatt_server_read_client_configuration.characteristic = characteristic;
    sl_bt_host_handle_command();

    if (client_config_flags) {
        *client_config_flags = rsp->data.rsp_gatt_server_read_client_configuration.client_config_flags;
    }

    return rsp->data.rsp_gatt_server_read_client_configuration.result;
}

sl_status_t sl_bt_gatt_server_send_user_prepare_write_response(uint8_t connection,
                                                               uint16_t characteristic,
                                                               uint8_t att_errorcode,
                                                               uint16_t offset,
                                                               size_t value_len,
                                                               const uint8_t* value)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_send_user_prepare_write_response_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_send_user_prepare_write_response_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_send_user_prepare_write_response.connection = connection;
    cmd->data.cmd_gatt_server_send_user_prepare_write_response.characteristic = characteristic;
    cmd->data.cmd_gatt_server_send_user_prepare_write_response.att_errorcode = att_errorcode;
    cmd->data.cmd_gatt_server_send_user_prepare_write_response.offset = offset;
    cmd->data.cmd_gatt_server_send_user_prepare_write_response.value.len = value_len;
    memcpy(cmd->data.cmd_gatt_server_send_user_prepare_write_response.value.data, value, value_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_server_send_user_prepare_write_response.result;
}

sl_status_t sl_bt_gatt_server_set_capabilities(uint32_t caps,
                                               uint32_t reserved)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_set_capabilities_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_set_capabilities_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_set_capabilities.caps = caps;
    cmd->data.cmd_gatt_server_set_capabilities.reserved = reserved;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_server_set_capabilities.result;
}

sl_status_t sl_bt_gatt_server_enable_capabilities(uint32_t caps)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_enable_capabilities_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_enable_capabilities_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_enable_capabilities.caps = caps;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_server_enable_capabilities.result;
}

sl_status_t sl_bt_gatt_server_disable_capabilities(uint32_t caps)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_disable_capabilities_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_disable_capabilities_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_disable_capabilities.caps = caps;
    sl_bt_host_handle_command();

    return rsp->data.rsp_gatt_server_disable_capabilities.result;
}

sl_status_t sl_bt_gatt_server_get_enabled_capabilities(uint32_t *caps)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_get_enabled_capabilities_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    if (caps) {
        *caps = rsp->data.rsp_gatt_server_get_enabled_capabilities.caps;
    }

    return rsp->data.rsp_gatt_server_get_enabled_capabilities.result;
}

sl_status_t sl_bt_gatt_server_read_client_supported_features(uint8_t connection,
                                                             uint8_t *client_features)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_gatt_server_read_client_supported_features_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_gatt_server_class_id,
                                       sli_bt_gatt_server_read_client_supported_features_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_gatt_server_read_client_supported_features.connection = connection;
    sl_bt_host_handle_command();

    if (client_features) {
        *client_features = rsp->data.rsp_gatt_server_read_client_supported_features.client_features;
    }

    return rsp->data.rsp_gatt_server_read_client_supported_features.result;
}

sl_status_t sl_bt_nvm_save(uint16_t key,
                           size_t value_len,
                           const uint8_t* value)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_nvm_save_t) + value_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_nvm_class_id,
                                       sli_bt_nvm_save_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_nvm_save.key = key;
    cmd->data.cmd_nvm_save.value.len = value_len;
    memcpy(cmd->data.cmd_nvm_save.value.data, value, value_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_nvm_save.result;
}

sl_status_t sl_bt_nvm_load(uint16_t key,
                           size_t max_value_size,
                           size_t *value_len,
                           uint8_t *value)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_nvm_load_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_nvm_class_id,
                                       sli_bt_nvm_load_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_nvm_load.key = key;
    sl_bt_host_handle_command();

    if (value_len) {
        *value_len = rsp->data.rsp_nvm_load.value.len;
    }
    if (value && (rsp->data.rsp_nvm_load.value.len <= max_value_size)) {
        memcpy(value, rsp->data.rsp_nvm_load.value.data, rsp->data.rsp_nvm_load.value.len);
    }

    return rsp->data.rsp_nvm_load.result;
}

sl_status_t sl_bt_nvm_erase(uint16_t key)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_nvm_erase_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_nvm_class_id,
                                       sli_bt_nvm_erase_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_nvm_erase.key = key;
    sl_bt_host_handle_command();

    return rsp->data.rsp_nvm_erase.result;
}

sl_status_t sl_bt_nvm_erase_all(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_nvm_class_id,
                                       sli_bt_nvm_erase_all_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_nvm_erase_all.result;
}

sl_status_t sl_bt_test_dtm_tx_v4(uint8_t packet_type,
                                 uint8_t length,
                                 uint8_t channel,
                                 uint8_t phy,
                                 int8_t power_level)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_test_dtm_tx_v4_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_test_class_id,
                                       sli_bt_test_dtm_tx_v4_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_test_dtm_tx_v4.packet_type = packet_type;
    cmd->data.cmd_test_dtm_tx_v4.length = length;
    cmd->data.cmd_test_dtm_tx_v4.channel = channel;
    cmd->data.cmd_test_dtm_tx_v4.phy = phy;
    cmd->data.cmd_test_dtm_tx_v4.power_level = power_level;
    sl_bt_host_handle_command();

    return rsp->data.rsp_test_dtm_tx_v4.result;
}

sl_status_t sl_bt_test_dtm_tx_cw(uint8_t packet_type,
                                 uint8_t channel,
                                 uint8_t phy,
                                 int16_t power_level)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_test_dtm_tx_cw_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_test_class_id,
                                       sli_bt_test_dtm_tx_cw_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_test_dtm_tx_cw.packet_type = packet_type;
    cmd->data.cmd_test_dtm_tx_cw.channel = channel;
    cmd->data.cmd_test_dtm_tx_cw.phy = phy;
    cmd->data.cmd_test_dtm_tx_cw.power_level = power_level;
    sl_bt_host_handle_command();

    return rsp->data.rsp_test_dtm_tx_cw.result;
}

sl_status_t sl_bt_test_dtm_rx(uint8_t channel, uint8_t phy)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_test_dtm_rx_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_test_class_id,
                                       sli_bt_test_dtm_rx_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_test_dtm_rx.channel = channel;
    cmd->data.cmd_test_dtm_rx.phy = phy;
    sl_bt_host_handle_command();

    return rsp->data.rsp_test_dtm_rx.result;
}

sl_status_t sl_bt_test_dtm_end(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_test_class_id,
                                       sli_bt_test_dtm_end_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_test_dtm_end.result;
}

sl_status_t sl_bt_sm_configure(uint8_t flags, uint8_t io_capabilities)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_configure_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_configure_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sm_configure.flags = flags;
    cmd->data.cmd_sm_configure.io_capabilities = io_capabilities;
    sl_bt_host_handle_command();

    return rsp->data.rsp_sm_configure.result;
}

sl_status_t sl_bt_sm_set_minimum_key_size(uint8_t minimum_key_size)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_set_minimum_key_size_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_set_minimum_key_size_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sm_set_minimum_key_size.minimum_key_size = minimum_key_size;
    sl_bt_host_handle_command();

    return rsp->data.rsp_sm_set_minimum_key_size.result;
}

sl_status_t sl_bt_sm_set_debug_mode(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_set_debug_mode_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_sm_set_debug_mode.result;
}

sl_status_t sl_bt_sm_store_bonding_configuration(uint8_t max_bonding_count,
                                                 uint8_t policy_flags)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_store_bonding_configuration_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_store_bonding_configuration_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sm_store_bonding_configuration.max_bonding_count = max_bonding_count;
    cmd->data.cmd_sm_store_bonding_configuration.policy_flags = policy_flags;
    sl_bt_host_handle_command();

    return rsp->data.rsp_sm_store_bonding_configuration.result;
}

sl_status_t sl_bt_sm_set_bondable_mode(uint8_t bondable)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_set_bondable_mode_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_set_bondable_mode_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sm_set_bondable_mode.bondable = bondable;
    sl_bt_host_handle_command();

    return rsp->data.rsp_sm_set_bondable_mode.result;
}

sl_status_t sl_bt_sm_set_passkey(int32_t passkey)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_set_passkey_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_set_passkey_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sm_set_passkey.passkey = passkey;
    sl_bt_host_handle_command();

    return rsp->data.rsp_sm_set_passkey.result;
}

sl_status_t sl_bt_sm_increase_security(uint8_t connection)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_increase_security_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_increase_security_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sm_increase_security.connection = connection;
    sl_bt_host_handle_command();

    return rsp->data.rsp_sm_increase_security.result;
}

sl_status_t sl_bt_sm_enter_passkey(uint8_t connection, int32_t passkey)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_enter_passkey_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_enter_passkey_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sm_enter_passkey.connection = connection;
    cmd->data.cmd_sm_enter_passkey.passkey = passkey;
    sl_bt_host_handle_command();

    return rsp->data.rsp_sm_enter_passkey.result;
}

sl_status_t sl_bt_sm_passkey_confirm(uint8_t connection, uint8_t confirm)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_passkey_confirm_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_passkey_confirm_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sm_passkey_confirm.connection = connection;
    cmd->data.cmd_sm_passkey_confirm.confirm = confirm;
    sl_bt_host_handle_command();

    return rsp->data.rsp_sm_passkey_confirm.result;
}

sl_status_t sl_bt_sm_bonding_confirm(uint8_t connection, uint8_t confirm)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_bonding_confirm_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_bonding_confirm_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sm_bonding_confirm.connection = connection;
    cmd->data.cmd_sm_bonding_confirm.confirm = confirm;
    sl_bt_host_handle_command();

    return rsp->data.rsp_sm_bonding_confirm.result;
}

sl_status_t sl_bt_sm_delete_bonding(uint8_t bonding)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_delete_bonding_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_delete_bonding_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sm_delete_bonding.bonding = bonding;
    sl_bt_host_handle_command();

    return rsp->data.rsp_sm_delete_bonding.result;
}

sl_status_t sl_bt_sm_delete_bondings(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_delete_bondings_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_sm_delete_bondings.result;
}

sl_status_t sl_bt_sm_get_bonding_handles(uint32_t reserved,
                                         uint32_t *num_bondings,
                                         size_t max_bondings_size,
                                         size_t *bondings_len,
                                         uint8_t *bondings)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_get_bonding_handles_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_get_bonding_handles_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sm_get_bonding_handles.reserved = reserved;
    sl_bt_host_handle_command();

    if (num_bondings) {
        *num_bondings = rsp->data.rsp_sm_get_bonding_handles.num_bondings;
    }
    if (bondings_len) {
        *bondings_len = rsp->data.rsp_sm_get_bonding_handles.bondings.len;
    }
    if (bondings && (rsp->data.rsp_sm_get_bonding_handles.bondings.len <= max_bondings_size)) {
        memcpy(bondings, rsp->data.rsp_sm_get_bonding_handles.bondings.data, rsp->data.rsp_sm_get_bonding_handles.bondings.len);
    }

    return rsp->data.rsp_sm_get_bonding_handles.result;
}

sl_status_t sl_bt_sm_get_bonding_details(uint32_t bonding,
                                         bd_addr *address,
                                         uint8_t *address_type,
                                         uint8_t *security_mode,
                                         uint8_t *key_size)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_get_bonding_details_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_get_bonding_details_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sm_get_bonding_details.bonding = bonding;
    sl_bt_host_handle_command();

    if (address) {
        memcpy(address, &rsp->data.rsp_sm_get_bonding_details.address, sizeof(bd_addr));
    }
    if (address_type) {
        *address_type = rsp->data.rsp_sm_get_bonding_details.address_type;
    }
    if (security_mode) {
        *security_mode = rsp->data.rsp_sm_get_bonding_details.security_mode;
    }
    if (key_size) {
        *key_size = rsp->data.rsp_sm_get_bonding_details.key_size;
    }

    return rsp->data.rsp_sm_get_bonding_details.result;
}

sl_status_t sl_bt_sm_find_bonding_by_address(bd_addr address,
                                             uint32_t *bonding,
                                             uint8_t *security_mode,
                                             uint8_t *key_size)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_find_bonding_by_address_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_find_bonding_by_address_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    memcpy(&cmd->data.cmd_sm_find_bonding_by_address.address, &address, sizeof(bd_addr));
    sl_bt_host_handle_command();

    if (bonding) {
        *bonding = rsp->data.rsp_sm_find_bonding_by_address.bonding;
    }
    if (security_mode) {
        *security_mode = rsp->data.rsp_sm_find_bonding_by_address.security_mode;
    }
    if (key_size) {
        *key_size = rsp->data.rsp_sm_find_bonding_by_address.key_size;
    }

    return rsp->data.rsp_sm_find_bonding_by_address.result;
}

sl_status_t sl_bt_sm_resolve_rpa(bd_addr rpa,
                                 bd_addr *address,
                                 uint8_t *address_type,
                                 uint32_t *bonding)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_resolve_rpa_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_resolve_rpa_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    memcpy(&cmd->data.cmd_sm_resolve_rpa.rpa, &rpa, sizeof(bd_addr));
    sl_bt_host_handle_command();

    if (address) {
        memcpy(address, &rsp->data.rsp_sm_resolve_rpa.address, sizeof(bd_addr));
    }
    if (address_type) {
        *address_type = rsp->data.rsp_sm_resolve_rpa.address_type;
    }
    if (bonding) {
        *bonding = rsp->data.rsp_sm_resolve_rpa.bonding;
    }

    return rsp->data.rsp_sm_resolve_rpa.result;
}

sl_status_t sl_bt_sm_set_legacy_oob(uint8_t enable, aes_key_128 oob_data)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_set_legacy_oob_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_set_legacy_oob_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sm_set_legacy_oob.enable = enable;
    memcpy(&cmd->data.cmd_sm_set_legacy_oob.oob_data, &oob_data, sizeof(aes_key_128));
    sl_bt_host_handle_command();

    return rsp->data.rsp_sm_set_legacy_oob.result;
}

sl_status_t sl_bt_sm_set_oob(uint8_t enable,
                             aes_key_128 *random,
                             aes_key_128 *confirm)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_set_oob_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_set_oob_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sm_set_oob.enable = enable;
    sl_bt_host_handle_command();

    if (random) {
        memcpy(random, &rsp->data.rsp_sm_set_oob.random, sizeof(aes_key_128));
    }
    if (confirm) {
        memcpy(confirm, &rsp->data.rsp_sm_set_oob.confirm, sizeof(aes_key_128));
    }

    return rsp->data.rsp_sm_set_oob.result;
}

sl_status_t sl_bt_sm_set_remote_oob(uint8_t enable,
                                    aes_key_128 random,
                                    aes_key_128 confirm)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_sm_set_remote_oob_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_sm_class_id,
                                       sli_bt_sm_set_remote_oob_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_sm_set_remote_oob.enable = enable;
    memcpy(&cmd->data.cmd_sm_set_remote_oob.random, &random, sizeof(aes_key_128));
    memcpy(&cmd->data.cmd_sm_set_remote_oob.confirm, &confirm, sizeof(aes_key_128));
    sl_bt_host_handle_command();

    return rsp->data.rsp_sm_set_remote_oob.result;
}

sl_status_t sl_bt_external_bondingdb_set_data(uint8_t connection,
                                              uint8_t type,
                                              size_t data_len,
                                              const uint8_t* data)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_external_bondingdb_set_data_t) + data_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_external_bondingdb_class_id,
                                       sli_bt_external_bondingdb_set_data_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_external_bondingdb_set_data.connection = connection;
    cmd->data.cmd_external_bondingdb_set_data.type = type;
    cmd->data.cmd_external_bondingdb_set_data.data.len = data_len;
    memcpy(cmd->data.cmd_external_bondingdb_set_data.data.data, data, data_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_external_bondingdb_set_data.result;
}

sl_status_t sl_bt_external_bondingdb_set_local_irk(size_t irk_len,
                                                   const uint8_t* irk)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_external_bondingdb_set_local_irk_t) + irk_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_external_bondingdb_class_id,
                                       sli_bt_external_bondingdb_set_local_irk_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_external_bondingdb_set_local_irk.irk.len = irk_len;
    memcpy(cmd->data.cmd_external_bondingdb_set_local_irk.irk.data, irk, irk_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_external_bondingdb_set_local_irk.result;
}

sl_status_t sl_bt_resolving_list_add_device_by_bonding(uint32_t bonding,
                                                       uint8_t privacy_mode)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_resolving_list_add_device_by_bonding_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_resolving_list_class_id,
                                       sli_bt_resolving_list_add_device_by_bonding_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_resolving_list_add_device_by_bonding.bonding = bonding;
    cmd->data.cmd_resolving_list_add_device_by_bonding.privacy_mode = privacy_mode;
    sl_bt_host_handle_command();

    return rsp->data.rsp_resolving_list_add_device_by_bonding.result;
}

sl_status_t sl_bt_resolving_list_add_device_by_address(bd_addr address,
                                                       uint8_t address_type,
                                                       aes_key_128 key,
                                                       uint8_t privacy_mode)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_resolving_list_add_device_by_address_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_resolving_list_class_id,
                                       sli_bt_resolving_list_add_device_by_address_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    memcpy(&cmd->data.cmd_resolving_list_add_device_by_address.address, &address, sizeof(bd_addr));
    cmd->data.cmd_resolving_list_add_device_by_address.address_type = address_type;
    memcpy(&cmd->data.cmd_resolving_list_add_device_by_address.key, &key, sizeof(aes_key_128));
    cmd->data.cmd_resolving_list_add_device_by_address.privacy_mode = privacy_mode;
    sl_bt_host_handle_command();

    return rsp->data.rsp_resolving_list_add_device_by_address.result;
}

sl_status_t sl_bt_resolving_list_remove_device_by_bonding(uint32_t bonding)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_resolving_list_remove_device_by_bonding_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_resolving_list_class_id,
                                       sli_bt_resolving_list_remove_device_by_bonding_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_resolving_list_remove_device_by_bonding.bonding = bonding;
    sl_bt_host_handle_command();

    return rsp->data.rsp_resolving_list_remove_device_by_bonding.result;
}

sl_status_t sl_bt_resolving_list_remove_device_by_address(bd_addr address,
                                                          uint8_t address_type)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_resolving_list_remove_device_by_address_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_resolving_list_class_id,
                                       sli_bt_resolving_list_remove_device_by_address_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    memcpy(&cmd->data.cmd_resolving_list_remove_device_by_address.address, &address, sizeof(bd_addr));
    cmd->data.cmd_resolving_list_remove_device_by_address.address_type = address_type;
    sl_bt_host_handle_command();

    return rsp->data.rsp_resolving_list_remove_device_by_address.result;
}

sl_status_t sl_bt_resolving_list_remove_all_devices(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_resolving_list_class_id,
                                       sli_bt_resolving_list_remove_all_devices_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_resolving_list_remove_all_devices.result;
}

sl_status_t sl_bt_accept_list_add_device_by_bonding(uint32_t bonding)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_accept_list_add_device_by_bonding_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_accept_list_class_id,
                                       sli_bt_accept_list_add_device_by_bonding_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_accept_list_add_device_by_bonding.bonding = bonding;
    sl_bt_host_handle_command();

    return rsp->data.rsp_accept_list_add_device_by_bonding.result;
}

sl_status_t sl_bt_accept_list_add_device_by_address(bd_addr address,
                                                    uint8_t address_type)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_accept_list_add_device_by_address_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_accept_list_class_id,
                                       sli_bt_accept_list_add_device_by_address_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    memcpy(&cmd->data.cmd_accept_list_add_device_by_address.address, &address, sizeof(bd_addr));
    cmd->data.cmd_accept_list_add_device_by_address.address_type = address_type;
    sl_bt_host_handle_command();

    return rsp->data.rsp_accept_list_add_device_by_address.result;
}

sl_status_t sl_bt_accept_list_remove_device_by_bonding(uint32_t bonding)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_accept_list_remove_device_by_bonding_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_accept_list_class_id,
                                       sli_bt_accept_list_remove_device_by_bonding_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_accept_list_remove_device_by_bonding.bonding = bonding;
    sl_bt_host_handle_command();

    return rsp->data.rsp_accept_list_remove_device_by_bonding.result;
}

sl_status_t sl_bt_accept_list_remove_device_by_address(bd_addr address,
                                                       uint8_t address_type)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_accept_list_remove_device_by_address_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_accept_list_class_id,
                                       sli_bt_accept_list_remove_device_by_address_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    memcpy(&cmd->data.cmd_accept_list_remove_device_by_address.address, &address, sizeof(bd_addr));
    cmd->data.cmd_accept_list_remove_device_by_address.address_type = address_type;
    sl_bt_host_handle_command();

    return rsp->data.rsp_accept_list_remove_device_by_address.result;
}

sl_status_t sl_bt_accept_list_remove_all_devices(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_accept_list_class_id,
                                       sli_bt_accept_list_remove_all_devices_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_accept_list_remove_all_devices.result;
}

sl_status_t sl_bt_coex_set_options(uint32_t mask, uint32_t options)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_coex_set_options_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_coex_class_id,
                                       sli_bt_coex_set_options_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_coex_set_options.mask = mask;
    cmd->data.cmd_coex_set_options.options = options;
    sl_bt_host_handle_command();

    return rsp->data.rsp_coex_set_options.result;
}

sl_status_t sl_bt_coex_set_parameters(uint8_t priority,
                                      uint8_t request,
                                      uint8_t pwm_period,
                                      uint8_t pwm_dutycycle)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_coex_set_parameters_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_coex_class_id,
                                       sli_bt_coex_set_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_coex_set_parameters.priority = priority;
    cmd->data.cmd_coex_set_parameters.request = request;
    cmd->data.cmd_coex_set_parameters.pwm_period = pwm_period;
    cmd->data.cmd_coex_set_parameters.pwm_dutycycle = pwm_dutycycle;
    sl_bt_host_handle_command();

    return rsp->data.rsp_coex_set_parameters.result;
}

sl_status_t sl_bt_coex_set_directional_priority_pulse(uint8_t pulse)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_coex_set_directional_priority_pulse_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_coex_class_id,
                                       sli_bt_coex_set_directional_priority_pulse_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_coex_set_directional_priority_pulse.pulse = pulse;
    sl_bt_host_handle_command();

    return rsp->data.rsp_coex_set_directional_priority_pulse.result;
}

sl_status_t sl_bt_coex_get_parameters(uint8_t *priority,
                                      uint8_t *request,
                                      uint8_t *pwm_period,
                                      uint8_t *pwm_dutycycle)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_coex_class_id,
                                       sli_bt_coex_get_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    if (priority) {
        *priority = rsp->data.rsp_coex_get_parameters.priority;
    }
    if (request) {
        *request = rsp->data.rsp_coex_get_parameters.request;
    }
    if (pwm_period) {
        *pwm_period = rsp->data.rsp_coex_get_parameters.pwm_period;
    }
    if (pwm_dutycycle) {
        *pwm_dutycycle = rsp->data.rsp_coex_get_parameters.pwm_dutycycle;
    }

    return rsp->data.rsp_coex_get_parameters.result;
}

sl_status_t sl_bt_coex_get_counters(uint8_t reset,
                                    size_t max_counters_size,
                                    size_t *counters_len,
                                    uint8_t *counters)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_coex_get_counters_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_coex_class_id,
                                       sli_bt_coex_get_counters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_coex_get_counters.reset = reset;
    sl_bt_host_handle_command();

    if (counters_len) {
        *counters_len = rsp->data.rsp_coex_get_counters.counters.len;
    }
    if (counters && (rsp->data.rsp_coex_get_counters.counters.len <= max_counters_size)) {
        memcpy(counters, rsp->data.rsp_coex_get_counters.counters.data, rsp->data.rsp_coex_get_counters.counters.len);
    }

    return rsp->data.rsp_coex_get_counters.result;
}

sl_status_t sl_bt_cs_security_enable(uint8_t connection)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cs_security_enable_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cs_class_id,
                                       sli_bt_cs_security_enable_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cs_security_enable.connection = connection;
    sl_bt_host_handle_command();

    return rsp->data.rsp_cs_security_enable.result;
}

sl_status_t sl_bt_cs_set_default_settings(uint8_t connection,
                                          uint8_t initiator_status,
                                          uint8_t reflector_status,
                                          uint8_t antenna_identifier,
                                          int8_t max_tx_power)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cs_set_default_settings_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cs_class_id,
                                       sli_bt_cs_set_default_settings_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cs_set_default_settings.connection = connection;
    cmd->data.cmd_cs_set_default_settings.initiator_status = initiator_status;
    cmd->data.cmd_cs_set_default_settings.reflector_status = reflector_status;
    cmd->data.cmd_cs_set_default_settings.antenna_identifier = antenna_identifier;
    cmd->data.cmd_cs_set_default_settings.max_tx_power = max_tx_power;
    sl_bt_host_handle_command();

    return rsp->data.rsp_cs_set_default_settings.result;
}

sl_status_t sl_bt_cs_create_config(uint8_t connection,
                                   uint8_t config_id,
                                   uint8_t create_context,
                                   uint8_t main_mode_type,
                                   uint8_t sub_mode_type,
                                   uint8_t min_main_mode_steps,
                                   uint8_t max_main_mode_steps,
                                   uint8_t main_mode_repetition,
                                   uint8_t mode_calibration_steps,
                                   uint8_t role,
                                   uint8_t rtt_type,
                                   uint8_t cs_sync_phy,
                                   const sl_bt_cs_channel_map_t *channel_map,
                                   uint8_t channel_map_repetition,
                                   uint8_t channel_selection_type,
                                   uint8_t ch3c_shape,
                                   uint8_t ch3c_jump,
                                   uint8_t reserved)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cs_create_config_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cs_class_id,
                                       sli_bt_cs_create_config_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cs_create_config.connection = connection;
    cmd->data.cmd_cs_create_config.config_id = config_id;
    cmd->data.cmd_cs_create_config.create_context = create_context;
    cmd->data.cmd_cs_create_config.main_mode_type = main_mode_type;
    cmd->data.cmd_cs_create_config.sub_mode_type = sub_mode_type;
    cmd->data.cmd_cs_create_config.min_main_mode_steps = min_main_mode_steps;
    cmd->data.cmd_cs_create_config.max_main_mode_steps = max_main_mode_steps;
    cmd->data.cmd_cs_create_config.main_mode_repetition = main_mode_repetition;
    cmd->data.cmd_cs_create_config.mode_calibration_steps = mode_calibration_steps;
    cmd->data.cmd_cs_create_config.role = role;
    cmd->data.cmd_cs_create_config.rtt_type = rtt_type;
    cmd->data.cmd_cs_create_config.cs_sync_phy = cs_sync_phy;
    memcpy(&cmd->data.cmd_cs_create_config.channel_map, channel_map, sizeof(sl_bt_cs_channel_map_t));
    cmd->data.cmd_cs_create_config.channel_map_repetition = channel_map_repetition;
    cmd->data.cmd_cs_create_config.channel_selection_type = channel_selection_type;
    cmd->data.cmd_cs_create_config.ch3c_shape = ch3c_shape;
    cmd->data.cmd_cs_create_config.ch3c_jump = ch3c_jump;
    cmd->data.cmd_cs_create_config.reserved = reserved;
    sl_bt_host_handle_command();

    return rsp->data.rsp_cs_create_config.result;
}

sl_status_t sl_bt_cs_remove_config(uint8_t connection, uint8_t config_id)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cs_remove_config_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cs_class_id,
                                       sli_bt_cs_remove_config_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cs_remove_config.connection = connection;
    cmd->data.cmd_cs_remove_config.config_id = config_id;
    sl_bt_host_handle_command();

    return rsp->data.rsp_cs_remove_config.result;
}

sl_status_t sl_bt_cs_set_channel_classification(const sl_bt_cs_channel_map_t *channel_map)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cs_set_channel_classification_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cs_class_id,
                                       sli_bt_cs_set_channel_classification_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    memcpy(&cmd->data.cmd_cs_set_channel_classification.channel_map, channel_map, sizeof(sl_bt_cs_channel_map_t));
    sl_bt_host_handle_command();

    return rsp->data.rsp_cs_set_channel_classification.result;
}

sl_status_t sl_bt_cs_set_procedure_parameters(uint8_t connection,
                                              uint8_t config_id,
                                              uint16_t max_procedure_len,
                                              uint16_t min_procedure_interval,
                                              uint16_t max_procedure_interval,
                                              uint16_t max_procedure_count,
                                              uint32_t min_subevent_len,
                                              uint32_t max_subevent_len,
                                              uint8_t tone_antenna_config_selection,
                                              uint8_t phy,
                                              int8_t tx_pwr_delta,
                                              uint8_t preferred_peer_antenna,
                                              uint8_t snr_control_initiator,
                                              uint8_t snr_control_reflector)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cs_set_procedure_parameters_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cs_class_id,
                                       sli_bt_cs_set_procedure_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cs_set_procedure_parameters.connection = connection;
    cmd->data.cmd_cs_set_procedure_parameters.config_id = config_id;
    cmd->data.cmd_cs_set_procedure_parameters.max_procedure_len = max_procedure_len;
    cmd->data.cmd_cs_set_procedure_parameters.min_procedure_interval = min_procedure_interval;
    cmd->data.cmd_cs_set_procedure_parameters.max_procedure_interval = max_procedure_interval;
    cmd->data.cmd_cs_set_procedure_parameters.max_procedure_count = max_procedure_count;
    cmd->data.cmd_cs_set_procedure_parameters.min_subevent_len = min_subevent_len;
    cmd->data.cmd_cs_set_procedure_parameters.max_subevent_len = max_subevent_len;
    cmd->data.cmd_cs_set_procedure_parameters.tone_antenna_config_selection = tone_antenna_config_selection;
    cmd->data.cmd_cs_set_procedure_parameters.phy = phy;
    cmd->data.cmd_cs_set_procedure_parameters.tx_pwr_delta = tx_pwr_delta;
    cmd->data.cmd_cs_set_procedure_parameters.preferred_peer_antenna = preferred_peer_antenna;
    cmd->data.cmd_cs_set_procedure_parameters.snr_control_initiator = snr_control_initiator;
    cmd->data.cmd_cs_set_procedure_parameters.snr_control_reflector = snr_control_reflector;
    sl_bt_host_handle_command();

    return rsp->data.rsp_cs_set_procedure_parameters.result;
}

sl_status_t sl_bt_cs_procedure_enable(uint8_t connection,
                                      uint8_t enable,
                                      uint8_t config_id)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cs_procedure_enable_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cs_class_id,
                                       sli_bt_cs_procedure_enable_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cs_procedure_enable.connection = connection;
    cmd->data.cmd_cs_procedure_enable.enable = enable;
    cmd->data.cmd_cs_procedure_enable.config_id = config_id;
    sl_bt_host_handle_command();

    return rsp->data.rsp_cs_procedure_enable.result;
}

sl_status_t sl_bt_cs_set_antenna_configuration(size_t antenna_element_offset_len,
                                               const uint8_t* antenna_element_offset)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cs_set_antenna_configuration_t) + antenna_element_offset_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cs_class_id,
                                       sli_bt_cs_set_antenna_configuration_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cs_set_antenna_configuration.antenna_element_offset.len = antenna_element_offset_len;
    memcpy(cmd->data.cmd_cs_set_antenna_configuration.antenna_element_offset.data, antenna_element_offset, antenna_element_offset_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_cs_set_antenna_configuration.result;
}

sl_status_t sl_bt_cs_read_local_supported_capabilities(uint8_t *num_config,
                                                       uint16_t *max_consecutive_procedures,
                                                       uint8_t *num_antennas,
                                                       uint8_t *max_antenna_paths,
                                                       uint8_t *roles,
                                                       uint8_t *modes,
                                                       uint8_t *rtt_capability,
                                                       uint8_t *rtt_aa_only,
                                                       uint8_t *rtt_sounding,
                                                       uint8_t *rtt_random_payload,
                                                       uint16_t *nadm_sounding_capability,
                                                       uint16_t *nadm_random_capability,
                                                       uint8_t *cs_sync_phys,
                                                       uint16_t *subfeatures,
                                                       uint16_t *t_ip1_times,
                                                       uint16_t *t_ip2_times,
                                                       uint16_t *t_fcs_times,
                                                       uint16_t *t_pm_times,
                                                       uint8_t *t_sw_times,
                                                       uint8_t *tx_snr_capability)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cs_class_id,
                                       sli_bt_cs_read_local_supported_capabilities_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    if (num_config) {
        *num_config = rsp->data.rsp_cs_read_local_supported_capabilities.num_config;
    }
    if (max_consecutive_procedures) {
        *max_consecutive_procedures = rsp->data.rsp_cs_read_local_supported_capabilities.max_consecutive_procedures;
    }
    if (num_antennas) {
        *num_antennas = rsp->data.rsp_cs_read_local_supported_capabilities.num_antennas;
    }
    if (max_antenna_paths) {
        *max_antenna_paths = rsp->data.rsp_cs_read_local_supported_capabilities.max_antenna_paths;
    }
    if (roles) {
        *roles = rsp->data.rsp_cs_read_local_supported_capabilities.roles;
    }
    if (modes) {
        *modes = rsp->data.rsp_cs_read_local_supported_capabilities.modes;
    }
    if (rtt_capability) {
        *rtt_capability = rsp->data.rsp_cs_read_local_supported_capabilities.rtt_capability;
    }
    if (rtt_aa_only) {
        *rtt_aa_only = rsp->data.rsp_cs_read_local_supported_capabilities.rtt_aa_only;
    }
    if (rtt_sounding) {
        *rtt_sounding = rsp->data.rsp_cs_read_local_supported_capabilities.rtt_sounding;
    }
    if (rtt_random_payload) {
        *rtt_random_payload = rsp->data.rsp_cs_read_local_supported_capabilities.rtt_random_payload;
    }
    if (nadm_sounding_capability) {
        *nadm_sounding_capability = rsp->data.rsp_cs_read_local_supported_capabilities.nadm_sounding_capability;
    }
    if (nadm_random_capability) {
        *nadm_random_capability = rsp->data.rsp_cs_read_local_supported_capabilities.nadm_random_capability;
    }
    if (cs_sync_phys) {
        *cs_sync_phys = rsp->data.rsp_cs_read_local_supported_capabilities.cs_sync_phys;
    }
    if (subfeatures) {
        *subfeatures = rsp->data.rsp_cs_read_local_supported_capabilities.subfeatures;
    }
    if (t_ip1_times) {
        *t_ip1_times = rsp->data.rsp_cs_read_local_supported_capabilities.t_ip1_times;
    }
    if (t_ip2_times) {
        *t_ip2_times = rsp->data.rsp_cs_read_local_supported_capabilities.t_ip2_times;
    }
    if (t_fcs_times) {
        *t_fcs_times = rsp->data.rsp_cs_read_local_supported_capabilities.t_fcs_times;
    }
    if (t_pm_times) {
        *t_pm_times = rsp->data.rsp_cs_read_local_supported_capabilities.t_pm_times;
    }
    if (t_sw_times) {
        *t_sw_times = rsp->data.rsp_cs_read_local_supported_capabilities.t_sw_times;
    }
    if (tx_snr_capability) {
        *tx_snr_capability = rsp->data.rsp_cs_read_local_supported_capabilities.tx_snr_capability;
    }

    return rsp->data.rsp_cs_read_local_supported_capabilities.result;
}

sl_status_t sl_bt_cs_read_remote_supported_capabilities(uint8_t connection)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cs_read_remote_supported_capabilities_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cs_class_id,
                                       sli_bt_cs_read_remote_supported_capabilities_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cs_read_remote_supported_capabilities.connection = connection;
    sl_bt_host_handle_command();

    return rsp->data.rsp_cs_read_remote_supported_capabilities.result;
}

sl_status_t sl_bt_cs_test_start(uint8_t main_mode_type,
                                uint8_t sub_mode_type,
                                uint8_t main_mode_repetition,
                                uint8_t mode_calibration_steps,
                                uint8_t role,
                                uint8_t rtt_type,
                                uint8_t cs_sync_phy,
                                uint8_t antenna_selection,
                                const sl_bt_cs_subevent_length_t *subevent_len,
                                uint16_t subevent_interval,
                                uint8_t max_num_subevents,
                                int8_t tx_power,
                                uint8_t t_ip1_time,
                                uint8_t t_ip2_time,
                                uint8_t t_fcs_time,
                                uint8_t t_pm_time,
                                uint8_t t_sw_time,
                                uint8_t tone_antenna_config,
                                uint8_t reserved,
                                uint8_t snr_control_initiator,
                                uint8_t snr_control_reflector,
                                uint16_t drbg_nonce,
                                uint8_t channel_map_repetition,
                                uint16_t override_config,
                                size_t override_parameters_len,
                                const uint8_t* override_parameters)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cs_test_start_t) + override_parameters_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cs_test_class_id,
                                       sli_bt_cs_test_start_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cs_test_start.main_mode_type = main_mode_type;
    cmd->data.cmd_cs_test_start.sub_mode_type = sub_mode_type;
    cmd->data.cmd_cs_test_start.main_mode_repetition = main_mode_repetition;
    cmd->data.cmd_cs_test_start.mode_calibration_steps = mode_calibration_steps;
    cmd->data.cmd_cs_test_start.role = role;
    cmd->data.cmd_cs_test_start.rtt_type = rtt_type;
    cmd->data.cmd_cs_test_start.cs_sync_phy = cs_sync_phy;
    cmd->data.cmd_cs_test_start.antenna_selection = antenna_selection;
    memcpy(&cmd->data.cmd_cs_test_start.subevent_len, subevent_len, sizeof(sl_bt_cs_subevent_length_t));
    cmd->data.cmd_cs_test_start.subevent_interval = subevent_interval;
    cmd->data.cmd_cs_test_start.max_num_subevents = max_num_subevents;
    cmd->data.cmd_cs_test_start.tx_power = tx_power;
    cmd->data.cmd_cs_test_start.t_ip1_time = t_ip1_time;
    cmd->data.cmd_cs_test_start.t_ip2_time = t_ip2_time;
    cmd->data.cmd_cs_test_start.t_fcs_time = t_fcs_time;
    cmd->data.cmd_cs_test_start.t_pm_time = t_pm_time;
    cmd->data.cmd_cs_test_start.t_sw_time = t_sw_time;
    cmd->data.cmd_cs_test_start.tone_antenna_config = tone_antenna_config;
    cmd->data.cmd_cs_test_start.reserved = reserved;
    cmd->data.cmd_cs_test_start.snr_control_initiator = snr_control_initiator;
    cmd->data.cmd_cs_test_start.snr_control_reflector = snr_control_reflector;
    cmd->data.cmd_cs_test_start.drbg_nonce = drbg_nonce;
    cmd->data.cmd_cs_test_start.channel_map_repetition = channel_map_repetition;
    cmd->data.cmd_cs_test_start.override_config = override_config;
    cmd->data.cmd_cs_test_start.override_parameters.len = override_parameters_len;
    memcpy(cmd->data.cmd_cs_test_start.override_parameters.data, override_parameters, override_parameters_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_cs_test_start.result;
}

sl_status_t sl_bt_cs_test_end(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cs_test_class_id,
                                       sli_bt_cs_test_end_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_cs_test_end.result;
}

sl_status_t sl_bt_l2cap_open_le_channel(uint8_t connection,
                                        uint16_t spsm,
                                        uint16_t max_sdu,
                                        uint16_t max_pdu,
                                        uint16_t credit,
                                        uint16_t *cid)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_l2cap_open_le_channel_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_l2cap_class_id,
                                       sli_bt_l2cap_open_le_channel_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_l2cap_open_le_channel.connection = connection;
    cmd->data.cmd_l2cap_open_le_channel.spsm = spsm;
    cmd->data.cmd_l2cap_open_le_channel.max_sdu = max_sdu;
    cmd->data.cmd_l2cap_open_le_channel.max_pdu = max_pdu;
    cmd->data.cmd_l2cap_open_le_channel.credit = credit;
    sl_bt_host_handle_command();

    if (cid) {
        *cid = rsp->data.rsp_l2cap_open_le_channel.cid;
    }

    return rsp->data.rsp_l2cap_open_le_channel.result;
}

sl_status_t sl_bt_l2cap_send_le_channel_open_response(uint8_t connection,
                                                      uint16_t cid,
                                                      uint16_t max_sdu,
                                                      uint16_t max_pdu,
                                                      uint16_t credit,
                                                      uint16_t errorcode)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_l2cap_send_le_channel_open_response_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_l2cap_class_id,
                                       sli_bt_l2cap_send_le_channel_open_response_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_l2cap_send_le_channel_open_response.connection = connection;
    cmd->data.cmd_l2cap_send_le_channel_open_response.cid = cid;
    cmd->data.cmd_l2cap_send_le_channel_open_response.max_sdu = max_sdu;
    cmd->data.cmd_l2cap_send_le_channel_open_response.max_pdu = max_pdu;
    cmd->data.cmd_l2cap_send_le_channel_open_response.credit = credit;
    cmd->data.cmd_l2cap_send_le_channel_open_response.errorcode = errorcode;
    sl_bt_host_handle_command();

    return rsp->data.rsp_l2cap_send_le_channel_open_response.result;
}

sl_status_t sl_bt_l2cap_channel_send_data(uint8_t connection,
                                          uint16_t cid,
                                          size_t data_len,
                                          const uint8_t* data)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_l2cap_channel_send_data_t) + data_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_l2cap_class_id,
                                       sli_bt_l2cap_channel_send_data_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_l2cap_channel_send_data.connection = connection;
    cmd->data.cmd_l2cap_channel_send_data.cid = cid;
    cmd->data.cmd_l2cap_channel_send_data.data.len = data_len;
    memcpy(cmd->data.cmd_l2cap_channel_send_data.data.data, data, data_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_l2cap_channel_send_data.result;
}

sl_status_t sl_bt_l2cap_channel_send_credit(uint8_t connection,
                                            uint16_t cid,
                                            uint16_t credit)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_l2cap_channel_send_credit_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_l2cap_class_id,
                                       sli_bt_l2cap_channel_send_credit_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_l2cap_channel_send_credit.connection = connection;
    cmd->data.cmd_l2cap_channel_send_credit.cid = cid;
    cmd->data.cmd_l2cap_channel_send_credit.credit = credit;
    sl_bt_host_handle_command();

    return rsp->data.rsp_l2cap_channel_send_credit.result;
}

sl_status_t sl_bt_l2cap_close_channel(uint8_t connection, uint16_t cid)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_l2cap_close_channel_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_l2cap_class_id,
                                       sli_bt_l2cap_close_channel_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_l2cap_close_channel.connection = connection;
    cmd->data.cmd_l2cap_close_channel.cid = cid;
    sl_bt_host_handle_command();

    return rsp->data.rsp_l2cap_close_channel.result;
}

sl_status_t sl_bt_cte_transmitter_set_dtm_parameters(uint8_t cte_length,
                                                     uint8_t cte_type,
                                                     size_t switching_pattern_len,
                                                     const uint8_t* switching_pattern)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_transmitter_set_dtm_parameters_t) + switching_pattern_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_transmitter_class_id,
                                       sli_bt_cte_transmitter_set_dtm_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_transmitter_set_dtm_parameters.cte_length = cte_length;
    cmd->data.cmd_cte_transmitter_set_dtm_parameters.cte_type = cte_type;
    cmd->data.cmd_cte_transmitter_set_dtm_parameters.switching_pattern.len = switching_pattern_len;
    memcpy(cmd->data.cmd_cte_transmitter_set_dtm_parameters.switching_pattern.data, switching_pattern, switching_pattern_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_transmitter_set_dtm_parameters.result;
}

sl_status_t sl_bt_cte_transmitter_clear_dtm_parameters(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_transmitter_class_id,
                                       sli_bt_cte_transmitter_clear_dtm_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_transmitter_clear_dtm_parameters.result;
}

sl_status_t sl_bt_cte_transmitter_enable_connection_cte(uint8_t connection,
                                                        uint8_t cte_types,
                                                        size_t switching_pattern_len,
                                                        const uint8_t* switching_pattern)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_transmitter_enable_connection_cte_t) + switching_pattern_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_transmitter_class_id,
                                       sli_bt_cte_transmitter_enable_connection_cte_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_transmitter_enable_connection_cte.connection = connection;
    cmd->data.cmd_cte_transmitter_enable_connection_cte.cte_types = cte_types;
    cmd->data.cmd_cte_transmitter_enable_connection_cte.switching_pattern.len = switching_pattern_len;
    memcpy(cmd->data.cmd_cte_transmitter_enable_connection_cte.switching_pattern.data, switching_pattern, switching_pattern_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_transmitter_enable_connection_cte.result;
}

sl_status_t sl_bt_cte_transmitter_disable_connection_cte(uint8_t connection)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_transmitter_disable_connection_cte_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_transmitter_class_id,
                                       sli_bt_cte_transmitter_disable_connection_cte_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_transmitter_disable_connection_cte.connection = connection;
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_transmitter_disable_connection_cte.result;
}

sl_status_t sl_bt_cte_transmitter_enable_connectionless_cte(uint8_t handle,
                                                            uint8_t cte_length,
                                                            uint8_t cte_type,
                                                            uint8_t cte_count,
                                                            size_t switching_pattern_len,
                                                            const uint8_t* switching_pattern)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_transmitter_enable_connectionless_cte_t) + switching_pattern_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_transmitter_class_id,
                                       sli_bt_cte_transmitter_enable_connectionless_cte_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_transmitter_enable_connectionless_cte.handle = handle;
    cmd->data.cmd_cte_transmitter_enable_connectionless_cte.cte_length = cte_length;
    cmd->data.cmd_cte_transmitter_enable_connectionless_cte.cte_type = cte_type;
    cmd->data.cmd_cte_transmitter_enable_connectionless_cte.cte_count = cte_count;
    cmd->data.cmd_cte_transmitter_enable_connectionless_cte.switching_pattern.len = switching_pattern_len;
    memcpy(cmd->data.cmd_cte_transmitter_enable_connectionless_cte.switching_pattern.data, switching_pattern, switching_pattern_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_transmitter_enable_connectionless_cte.result;
}

sl_status_t sl_bt_cte_transmitter_disable_connectionless_cte(uint8_t handle)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_transmitter_disable_connectionless_cte_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_transmitter_class_id,
                                       sli_bt_cte_transmitter_disable_connectionless_cte_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_transmitter_disable_connectionless_cte.handle = handle;
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_transmitter_disable_connectionless_cte.result;
}

sl_status_t sl_bt_cte_transmitter_enable_silabs_cte(uint8_t handle,
                                                    uint8_t cte_length,
                                                    uint8_t cte_type,
                                                    uint8_t cte_count,
                                                    size_t switching_pattern_len,
                                                    const uint8_t* switching_pattern)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_transmitter_enable_silabs_cte_t) + switching_pattern_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_transmitter_class_id,
                                       sli_bt_cte_transmitter_enable_silabs_cte_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_transmitter_enable_silabs_cte.handle = handle;
    cmd->data.cmd_cte_transmitter_enable_silabs_cte.cte_length = cte_length;
    cmd->data.cmd_cte_transmitter_enable_silabs_cte.cte_type = cte_type;
    cmd->data.cmd_cte_transmitter_enable_silabs_cte.cte_count = cte_count;
    cmd->data.cmd_cte_transmitter_enable_silabs_cte.switching_pattern.len = switching_pattern_len;
    memcpy(cmd->data.cmd_cte_transmitter_enable_silabs_cte.switching_pattern.data, switching_pattern, switching_pattern_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_transmitter_enable_silabs_cte.result;
}

sl_status_t sl_bt_cte_transmitter_disable_silabs_cte(uint8_t handle)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_transmitter_disable_silabs_cte_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_transmitter_class_id,
                                       sli_bt_cte_transmitter_disable_silabs_cte_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_transmitter_disable_silabs_cte.handle = handle;
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_transmitter_disable_silabs_cte.result;
}

sl_status_t sl_bt_cte_receiver_set_dtm_parameters(uint8_t cte_length,
                                                  uint8_t cte_type,
                                                  uint8_t slot_durations,
                                                  size_t switching_pattern_len,
                                                  const uint8_t* switching_pattern)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_receiver_set_dtm_parameters_t) + switching_pattern_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_receiver_class_id,
                                       sli_bt_cte_receiver_set_dtm_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_receiver_set_dtm_parameters.cte_length = cte_length;
    cmd->data.cmd_cte_receiver_set_dtm_parameters.cte_type = cte_type;
    cmd->data.cmd_cte_receiver_set_dtm_parameters.slot_durations = slot_durations;
    cmd->data.cmd_cte_receiver_set_dtm_parameters.switching_pattern.len = switching_pattern_len;
    memcpy(cmd->data.cmd_cte_receiver_set_dtm_parameters.switching_pattern.data, switching_pattern, switching_pattern_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_receiver_set_dtm_parameters.result;
}

sl_status_t sl_bt_cte_receiver_clear_dtm_parameters(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_receiver_class_id,
                                       sli_bt_cte_receiver_clear_dtm_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_receiver_clear_dtm_parameters.result;
}

sl_status_t sl_bt_cte_receiver_set_sync_cte_type(uint8_t sync_cte_type)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_receiver_set_sync_cte_type_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_receiver_class_id,
                                       sli_bt_cte_receiver_set_sync_cte_type_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_receiver_set_sync_cte_type.sync_cte_type = sync_cte_type;
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_receiver_set_sync_cte_type.result;
}

sl_status_t sl_bt_cte_receiver_set_default_sync_receive_parameters(uint8_t mode,
                                                                   uint16_t skip,
                                                                   uint16_t timeout,
                                                                   uint8_t sync_cte_type,
                                                                   uint8_t reporting_mode)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_receiver_set_default_sync_receive_parameters_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_receiver_class_id,
                                       sli_bt_cte_receiver_set_default_sync_receive_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_receiver_set_default_sync_receive_parameters.mode = mode;
    cmd->data.cmd_cte_receiver_set_default_sync_receive_parameters.skip = skip;
    cmd->data.cmd_cte_receiver_set_default_sync_receive_parameters.timeout = timeout;
    cmd->data.cmd_cte_receiver_set_default_sync_receive_parameters.sync_cte_type = sync_cte_type;
    cmd->data.cmd_cte_receiver_set_default_sync_receive_parameters.reporting_mode = reporting_mode;
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_receiver_set_default_sync_receive_parameters.result;
}

sl_status_t sl_bt_cte_receiver_set_sync_receive_parameters(uint8_t connection,
                                                           uint8_t mode,
                                                           uint16_t skip,
                                                           uint16_t timeout,
                                                           uint8_t sync_cte_type,
                                                           uint8_t reporting_mode)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_receiver_set_sync_receive_parameters_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_receiver_class_id,
                                       sli_bt_cte_receiver_set_sync_receive_parameters_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_receiver_set_sync_receive_parameters.connection = connection;
    cmd->data.cmd_cte_receiver_set_sync_receive_parameters.mode = mode;
    cmd->data.cmd_cte_receiver_set_sync_receive_parameters.skip = skip;
    cmd->data.cmd_cte_receiver_set_sync_receive_parameters.timeout = timeout;
    cmd->data.cmd_cte_receiver_set_sync_receive_parameters.sync_cte_type = sync_cte_type;
    cmd->data.cmd_cte_receiver_set_sync_receive_parameters.reporting_mode = reporting_mode;
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_receiver_set_sync_receive_parameters.result;
}

sl_status_t sl_bt_cte_receiver_configure(uint8_t flags)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_receiver_configure_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_receiver_class_id,
                                       sli_bt_cte_receiver_configure_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_receiver_configure.flags = flags;
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_receiver_configure.result;
}

sl_status_t sl_bt_cte_receiver_enable_connection_cte(uint8_t connection,
                                                     uint16_t interval,
                                                     uint8_t cte_length,
                                                     uint8_t cte_type,
                                                     uint8_t slot_durations,
                                                     size_t switching_pattern_len,
                                                     const uint8_t* switching_pattern)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_receiver_enable_connection_cte_t) + switching_pattern_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_receiver_class_id,
                                       sli_bt_cte_receiver_enable_connection_cte_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_receiver_enable_connection_cte.connection = connection;
    cmd->data.cmd_cte_receiver_enable_connection_cte.interval = interval;
    cmd->data.cmd_cte_receiver_enable_connection_cte.cte_length = cte_length;
    cmd->data.cmd_cte_receiver_enable_connection_cte.cte_type = cte_type;
    cmd->data.cmd_cte_receiver_enable_connection_cte.slot_durations = slot_durations;
    cmd->data.cmd_cte_receiver_enable_connection_cte.switching_pattern.len = switching_pattern_len;
    memcpy(cmd->data.cmd_cte_receiver_enable_connection_cte.switching_pattern.data, switching_pattern, switching_pattern_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_receiver_enable_connection_cte.result;
}

sl_status_t sl_bt_cte_receiver_disable_connection_cte(uint8_t connection)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_receiver_disable_connection_cte_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_receiver_class_id,
                                       sli_bt_cte_receiver_disable_connection_cte_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_receiver_disable_connection_cte.connection = connection;
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_receiver_disable_connection_cte.result;
}

sl_status_t sl_bt_cte_receiver_enable_connectionless_cte(uint16_t sync,
                                                         uint8_t slot_durations,
                                                         uint8_t cte_count,
                                                         size_t switching_pattern_len,
                                                         const uint8_t* switching_pattern)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_receiver_enable_connectionless_cte_t) + switching_pattern_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_receiver_class_id,
                                       sli_bt_cte_receiver_enable_connectionless_cte_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_receiver_enable_connectionless_cte.sync = sync;
    cmd->data.cmd_cte_receiver_enable_connectionless_cte.slot_durations = slot_durations;
    cmd->data.cmd_cte_receiver_enable_connectionless_cte.cte_count = cte_count;
    cmd->data.cmd_cte_receiver_enable_connectionless_cte.switching_pattern.len = switching_pattern_len;
    memcpy(cmd->data.cmd_cte_receiver_enable_connectionless_cte.switching_pattern.data, switching_pattern, switching_pattern_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_receiver_enable_connectionless_cte.result;
}

sl_status_t sl_bt_cte_receiver_disable_connectionless_cte(uint16_t sync)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_receiver_disable_connectionless_cte_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_receiver_class_id,
                                       sli_bt_cte_receiver_disable_connectionless_cte_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_receiver_disable_connectionless_cte.sync = sync;
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_receiver_disable_connectionless_cte.result;
}

sl_status_t sl_bt_cte_receiver_enable_silabs_cte(uint8_t slot_durations,
                                                 uint8_t cte_count,
                                                 size_t switching_pattern_len,
                                                 const uint8_t* switching_pattern)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_cte_receiver_enable_silabs_cte_t) + switching_pattern_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_receiver_class_id,
                                       sli_bt_cte_receiver_enable_silabs_cte_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_cte_receiver_enable_silabs_cte.slot_durations = slot_durations;
    cmd->data.cmd_cte_receiver_enable_silabs_cte.cte_count = cte_count;
    cmd->data.cmd_cte_receiver_enable_silabs_cte.switching_pattern.len = switching_pattern_len;
    memcpy(cmd->data.cmd_cte_receiver_enable_silabs_cte.switching_pattern.data, switching_pattern, switching_pattern_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_receiver_enable_silabs_cte.result;
}

sl_status_t sl_bt_cte_receiver_disable_silabs_cte(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_cte_receiver_class_id,
                                       sli_bt_cte_receiver_disable_silabs_cte_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_cte_receiver_disable_silabs_cte.result;
}

sl_status_t sl_bt_connection_analyzer_start(uint32_t access_address,
                                            uint32_t crc_init,
                                            uint16_t interval,
                                            uint16_t supervision_timeout,
                                            uint8_t central_clock_accuracy,
                                            uint8_t central_phy,
                                            uint8_t peripheral_phy,
                                            uint8_t channel_selection_algorithm,
                                            uint8_t hop,
                                            const sl_bt_connection_channel_map_t *channel_map,
                                            uint8_t channel,
                                            uint16_t event_counter,
                                            int32_t start_time_us,
                                            uint32_t flags,
                                            uint8_t *analyzer)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_analyzer_start_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_analyzer_class_id,
                                       sli_bt_connection_analyzer_start_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_analyzer_start.access_address = access_address;
    cmd->data.cmd_connection_analyzer_start.crc_init = crc_init;
    cmd->data.cmd_connection_analyzer_start.interval = interval;
    cmd->data.cmd_connection_analyzer_start.supervision_timeout = supervision_timeout;
    cmd->data.cmd_connection_analyzer_start.central_clock_accuracy = central_clock_accuracy;
    cmd->data.cmd_connection_analyzer_start.central_phy = central_phy;
    cmd->data.cmd_connection_analyzer_start.peripheral_phy = peripheral_phy;
    cmd->data.cmd_connection_analyzer_start.channel_selection_algorithm = channel_selection_algorithm;
    cmd->data.cmd_connection_analyzer_start.hop = hop;
    memcpy(&cmd->data.cmd_connection_analyzer_start.channel_map, channel_map, sizeof(sl_bt_connection_channel_map_t));
    cmd->data.cmd_connection_analyzer_start.channel = channel;
    cmd->data.cmd_connection_analyzer_start.event_counter = event_counter;
    cmd->data.cmd_connection_analyzer_start.start_time_us = start_time_us;
    cmd->data.cmd_connection_analyzer_start.flags = flags;
    sl_bt_host_handle_command();

    if (analyzer) {
        *analyzer = rsp->data.rsp_connection_analyzer_start.analyzer;
    }

    return rsp->data.rsp_connection_analyzer_start.result;
}

sl_status_t sl_bt_connection_analyzer_stop(uint8_t analyzer)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_connection_analyzer_stop_t);

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_connection_analyzer_class_id,
                                       sli_bt_connection_analyzer_stop_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_connection_analyzer_stop.analyzer = analyzer;
    sl_bt_host_handle_command();

    return rsp->data.rsp_connection_analyzer_stop.result;
}

sl_status_t sl_bt_user_message_to_target(size_t data_len,
                                         const uint8_t* data,
                                         size_t max_response_size,
                                         size_t *response_len,
                                         uint8_t *response)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_user_message_to_target_t) + data_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_user_class_id,
                                       sli_bt_user_message_to_target_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_user_message_to_target.data.len = data_len;
    memcpy(cmd->data.cmd_user_message_to_target.data.data, data, data_len);
    sl_bt_host_handle_command();

    if (response_len) {
        *response_len = rsp->data.rsp_user_message_to_target.response.len;
    }
    if (response && (rsp->data.rsp_user_message_to_target.response.len <= max_response_size)) {
        memcpy(response, rsp->data.rsp_user_message_to_target.response.data, rsp->data.rsp_user_message_to_target.response.len);
    }

    return rsp->data.rsp_user_message_to_target.result;
}

sl_status_t sl_bt_user_manage_event_filter(size_t data_len,
                                           const uint8_t* data)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_user_manage_event_filter_t) + data_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_user_class_id,
                                       sli_bt_user_manage_event_filter_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_user_manage_event_filter.data.len = data_len;
    memcpy(cmd->data.cmd_user_manage_event_filter.data.data, data, data_len);
    sl_bt_host_handle_command();

    return rsp->data.rsp_user_manage_event_filter.result;
}

void sl_bt_user_reset_to_dfu(void)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    size_t cmd_payload_len = 0;

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_user_class_id,
                                       sli_bt_user_reset_to_dfu_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    sl_bt_host_handle_command_noresponse();
}

sl_status_t sl_bt_user_cs_service_message_to_target(size_t data_len,
                                                    const uint8_t* data,
                                                    size_t max_response_size,
                                                    size_t *response_len,
                                                    uint8_t *response)
{
    struct sl_bt_packet *cmd = (struct sl_bt_packet *)sl_bt_cmd_msg;
    struct sl_bt_packet *rsp = (struct sl_bt_packet *)sl_bt_rsp_msg;
    size_t cmd_payload_len = sizeof(sl_bt_cmd_user_cs_service_message_to_target_t) + data_len;
    if (cmd_payload_len > SL_BGAPI_MAX_PAYLOAD_SIZE) {
        return SL_STATUS_COMMAND_TOO_LONG;
    }

    cmd->header = SLI_BGAPI_MSG_HEADER(sli_bt_user_class_id,
                                       sli_bt_user_cs_service_message_to_target_command_id,
                                       (uint8_t) sl_bgapi_msg_type_cmd | (uint8_t) sl_bgapi_dev_type_bt,
                                       cmd_payload_len);
    cmd->data.cmd_user_cs_service_message_to_target.data.len = data_len;
    memcpy(cmd->data.cmd_user_cs_service_message_to_target.data.data, data, data_len);
    sl_bt_host_handle_command();

    if (response_len) {
        *response_len = rsp->data.rsp_user_cs_service_message_to_target.response.len;
    }
    if (response && (rsp->data.rsp_user_cs_service_message_to_target.response.len <= max_response_size)) {
        memcpy(response, rsp->data.rsp_user_cs_service_message_to_target.response.data, rsp->data.rsp_user_cs_service_message_to_target.response.len);
    }

    return rsp->data.rsp_user_cs_service_message_to_target.result;
}