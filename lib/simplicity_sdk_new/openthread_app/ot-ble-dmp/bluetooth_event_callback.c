/*******************************************************************************
 * @file
 * @brief Bluetooth event callback support
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#if SL_OPENTHREAD_BLE_CLI_ENABLE

#include "sl_bluetooth.h"
#include "sl_bt_api.h"
#include <openthread/cli.h>

extern void printBleAddress(bd_addr address);

void sl_bt_on_event(sl_bt_msg_t *evt)
{
    switch (SL_BT_MSG_ID(evt->header))
    {
    case sl_bt_evt_system_boot_id:
    {
        sl_status_t hello_rsp;
        sl_status_t address_rsp;

        // Call these two APIs upon boot for testing purposes.
        hello_rsp = sl_bt_system_hello();
        otCliOutputFormat("BLE hello: %s\r\n", (hello_rsp == SL_STATUS_OK) ? "success" : "error");

        bd_addr address;
        uint8_t type;
        address_rsp = sl_bt_system_get_identity_address(&address, &type);

        if (address_rsp == SL_STATUS_OK)
        {
            otCliOutputFormat("BLE address: ");
            printBleAddress(address);
        }
        else
        {
            otCliOutputFormat("error: 0x%2x", address_rsp);
        }
        otCliOutputFormat("\r\n");
    }
    break;
    case sl_bt_evt_connection_opened_id:
    {
        const sl_bt_evt_connection_opened_t *conn_evt = (const sl_bt_evt_connection_opened_t *)&(evt->data);
        otCliOutputFormat("BLE connection opened handle=%d address=", conn_evt->connection);
        printBleAddress(conn_evt->address);
        otCliOutputFormat(" address_type=%d master=%d advertising_set=%d\r\n",
                          conn_evt->connection,
                          conn_evt->role,
                          conn_evt->advertiser);
    }
    break;
    case sl_bt_evt_connection_closed_id:
    {
        const sl_bt_evt_connection_closed_t *conn_evt = (const sl_bt_evt_connection_closed_t *)&(evt->data);
        otCliOutputFormat("BLE connection closed handle=%d reason=0x%2x\r\n", conn_evt->connection, conn_evt->reason);
    }
    break;
    case sl_bt_evt_connection_parameters_id:
    {
        const sl_bt_evt_connection_parameters_t *params_evt = (const sl_bt_evt_connection_parameters_t *)&(evt->data);
        otCliOutputFormat("BLE connection parameters handle=%d interval=%d latency=%d timeout=%d security_mode=%d\r\n",
                          params_evt->connection,
                          params_evt->interval,
                          params_evt->latency,
                          params_evt->timeout,
                          params_evt->security_mode);
    }
    break;
    case sl_bt_evt_scanner_legacy_advertisement_report_id:
    {
        sl_bt_evt_scanner_legacy_advertisement_report_t *rsp_evt =
            (sl_bt_evt_scanner_legacy_advertisement_report_t *)&(evt->data);
        otCliOutputFormat("BLE scan response address=");
        printBleAddress(rsp_evt->address);
        otCliOutputFormat(" address_type=%d\r\n", rsp_evt->address_type);
    }
    break;
    case sl_bt_evt_scanner_extended_advertisement_report_id:
    {
        sl_bt_evt_scanner_extended_advertisement_report_t *rsp_evt =
            (sl_bt_evt_scanner_extended_advertisement_report_t *)&(evt->data);
        otCliOutputFormat("BLE scan response address=");
        printBleAddress(rsp_evt->address);
        otCliOutputFormat(" address_type=%d\r\n", rsp_evt->address_type);
    }
    break;
    case sl_bt_evt_gatt_procedure_completed_id:
    {
        const sl_bt_evt_gatt_procedure_completed_t *proc_comp_evt =
            (const sl_bt_evt_gatt_procedure_completed_t *)&(evt->data);
        otCliOutputFormat("BLE procedure completed handle=%d result=0x%2x\r\n",
                          proc_comp_evt->connection,
                          proc_comp_evt->result);
    }
    break;
    default:
        otCliOutputFormat("BLE event: 0x%04x\r\n", SL_BT_MSG_ID(evt->header));
    }
}

#endif // SL_OPENTHREAD_BLE_CLI_ENABLE
