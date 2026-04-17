"""
ESL AP Core - demo mode event handler extensions.
"""

# Copyright 2023 Silicon Laboratories Inc. www.silabs.com
#
# SPDX-License-Identifier: Zlib
#
# The licensor of this software is Silicon Laboratories Inc.
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

from ap_constants import (
    BROADCAST_ADDRESS,
    CCMD_CONFIG,
    CCMD_CONNECT,
    CCMD_DISCONNECT,
    CCMD_DISPLAY_IMAGE,
    CCMD_IMAGE_UPDATE,
    CCMD_LED,
    CCMD_LIST,
    CCMD_PING,
    CCMD_REQUEST_DATA,
    CCMD_UNASSOCIATE,
    CCMD_UNKNOWN,
    CMD_AP_CONTROL_ADV_ENABLE,
    CONTROLLER_COMMAND_FAIL,
    CONTROLLER_COMMAND_SUCCESS,
    CONTROLLER_REQUEST_LAST_DATA,
    CONTROLLER_REQUEST_MORE_DATA,
    REQUEST_IMAGE_DATA_HEADER,
    REQUEST_IMAGE_DATA_RESERVED,
)
from ap_config import INITIAL_AUTO_ADVERTISE_PAWR_TRAIN
from ap_sensor import S_ID_PRESENT_DEVICE_OPERATING_TEMPERATURE, S_ID_PRESENT_INPUT_VOLTAGE
import esl_lib
import esl_lib_wrapper as elw
import struct
from esl_tag import TagState, EslState


class DemoEventHandlersMixin:
    """Demo mode specific event handler extensions"""

    def demo_esl_event_system_boot(self, evt: esl_lib.EventSystemBoot):
        """ESL library boot event handler extension for demo mode"""
        self.log.warning(
            "ESL AP started in demo mode - open the 'ESL Demo' activity on the 'Demo' page in Simplicity Connect mobile application."
        )
        # Enable advertising
        self.lib.general_command(CMD_AP_CONTROL_ADV_ENABLE, b"\x01")
        self.start_scan()
        self.start_pawr_train(advertise=INITIAL_AUTO_ADVERTISE_PAWR_TRAIN)

    def demo_esl_event_general(self, evt: esl_lib.EventGeneral):
        """ESL library general event handler extension for demo mode"""
        subevent = evt.data[0]
        if subevent == 3:
            self.demo_ap_control_image_transfer(evt.data[1:])
        elif subevent == 2:
            self.demo_ap_control_request(evt.data[1:])
        elif subevent == 1:
            self.demo_ap_control_status(evt.data[1])

    def demo_ap_control_request(self, data: bytes):
        """ESL library general event control subevent processor in demo mode"""
        try:
            mcommand = data.decode("ascii")
            self.log.info("Command arrived from controller: %s", mcommand)
        except UnicodeDecodeError as e:
            self.log.error("Cannot decode command")
            self.log.error(e)
            self.notify_controller(CCMD_UNKNOWN, CONTROLLER_COMMAND_FAIL)
            return
        DEMO_COMMAND_DICT = {
            "connect": CCMD_CONNECT,
            "config": CCMD_CONFIG,
            "disconnect": CCMD_DISCONNECT,
            "image_update": CCMD_IMAGE_UPDATE,
            "display_image": CCMD_DISPLAY_IMAGE,
            "led": CCMD_LED,
            "list": CCMD_LIST,
            "ping": CCMD_PING,
            "unassociate": CCMD_UNASSOCIATE,
        }
        parts = mcommand.split()
        command = parts[0]
        try:
            esl_id = (int(parts[1]), 0)
        except (IndexError, ValueError):
            esl_id = (BROADCAST_ADDRESS, 0)
        tag = self.tag_db.find(esl_id)
        # Display warning or error message if PAwR is not (yet) active and command requires it
        if (
            not self.pawr_active
            and (
                esl_id == BROADCAST_ADDRESS
                or command == "led"
                or (tag is not None and tag.state != TagState.CONNECTED)
            )
        ):
            if command == "connect":
                self.log.warning("PAwR is not running: connection request may time out in the demo controller!")
            else:
                self.log.error("PAwR is not running: command will time out in the demo controller!")
        # Additional actions for image update
        if command == "image_update":
            self.image_from_controller = b""
            self.image_data_offset = bytearray(4)
        try:
            self.controller_command = DEMO_COMMAND_DICT[command]
            if self.cli_queue is not None:
                if mcommand.find("full") != -1:
                    mcommand = mcommand.replace("full", "--full")
                elif mcommand.find("index=0") != -1:
                    mcommand = mcommand.replace("index=", "--index ")
                self.cli_queue.put(mcommand)
        except KeyError:
            self.log.error("Not a valid command")
            self.notify_controller(CCMD_UNKNOWN, CONTROLLER_COMMAND_FAIL)

    def esl_event_control_point_notification(
        self, evt: esl_lib.EventControlPointNotification
    ):
        """ESL library control point notification event addon for demo mode"""
        # after an unassociate all command sent the evt.address will not be part of the list of tags!
        tag = self.tag_db.find(evt.connection_handle)
        if tag is None:
            return
        if self.controller_command != None:
            self.notify_controller(
                self.controller_command,
                CONTROLLER_COMMAND_SUCCESS,
                tag.esl_id,
                tag.group_id,
                evt.data,
            )

    def demo_esl_event_image_transfer_finished(
        self, evt: esl_lib.EventImageTransferFinished
    ):
        """ESL library image transfer finished event processor addon for demo mode"""
        if self.controller_command == CCMD_REQUEST_DATA:
            self.notify_controller(CCMD_IMAGE_UPDATE, CONTROLLER_COMMAND_SUCCESS)

    def demo_ap_control_image_transfer(self, data: bytes):
        """ESL library general event image transfer subevent processor in demo mode"""
        if len(data) >= 3:
            # Store image data
            self.image_from_controller += data[1:]
            self.image_data_offset = struct.pack(
                "<I", (len(self.image_from_controller) & 0xFFFFFFFF)
            )
        if data[0] == CONTROLLER_REQUEST_MORE_DATA:
            # Request more data
            self.notify_controller(
                CCMD_REQUEST_DATA,
                CONTROLLER_COMMAND_SUCCESS,
                REQUEST_IMAGE_DATA_HEADER,
                self.image_data_offset,
                REQUEST_IMAGE_DATA_RESERVED,
            )
        elif data[0] == CONTROLLER_REQUEST_LAST_DATA:
            # This was the last data, starting image update
            self.ap_imageupdate(self.controller_image_index, self.image_from_controller)
        else:
            self.log.error("Invalid data chunk arrived during image_update")
            self.notify_controller(CCMD_IMAGE_UPDATE, CONTROLLER_COMMAND_FAIL)

    def demo_ap_control_status(self, status: int):
        """ESL library general event status subevent processor in demo mode"""
        status_dict = {0x00: "disconnected", 0x01: "connected", 0x02: "subscribed"}
        try:
            status_str = status_dict[status]
        except KeyError:
            status_str = "unknown"
        self.log.info("AP control status %s", status_str)
        if status == 0:
            self.demo_controller_connected = False
            if not self.demo_mode:
                self.set_mode_handlers()
        elif not self.demo_controller_connected:
            self.demo_controller_connected = True

    def demo_esl_event_connection_opened(self, evt: esl_lib.EventConnectionOpened):
        """ESL library connection opened event handler extension for demo mode"""
        if self.demo_auto_reconfigure:
            tag = self.tag_db.find(evt.address)
            if tag is not None and tag.provisioned and not tag.blocked:
                self.past(tag)
        elif self.controller_command != None:
            if evt.status == elw.SL_STATUS_OK:
                self.notify_controller(
                    self.controller_command, CONTROLLER_COMMAND_SUCCESS
                )
            else:
                self.notify_controller(self.controller_command, CONTROLLER_COMMAND_FAIL)
                tag = self.tag_db.find(evt.address)
                if tag is not None:
                    self.disconnect(tag)

    def demo_esl_event_tag_found(self, evt: esl_lib.EventTagFound):
        """ESL library tag found event handler extension for demo mode"""
        tag = self.tag_db.find(evt.address)
        if (
            self.pawr_active
            and tag is not None
            and tag.esl_state == EslState.UNSYNCHRONIZED
            and tag.state == TagState.IDLE
        ):
            self.connect(tag)
            self.demo_auto_reconfigure = True

    def demo_esl_event_configure_tag_response(
        self, evt: esl_lib.EventConfigureTagResponse
    ):
        """ESL library configure tag response event handler extension for demo mode"""
        if evt.status == elw.SL_STATUS_OK:
            tag = self.tag_db.find(evt.connection_handle)
            if tag is not None and tag.provisioned:
                if self.controller_command == CCMD_CONFIG:
                    self.notify_controller(
                        CCMD_CONFIG,
                        CONTROLLER_COMMAND_SUCCESS,
                        tag.esl_id,
                        str(tag.ble_address),
                        tag.max_image_index + 1
                        if tag.max_image_index is not None
                        else 0,
                        len(tag.display_info),
                        struct.pack("<H", S_ID_PRESENT_INPUT_VOLTAGE)
                        if S_ID_PRESENT_INPUT_VOLTAGE in tag.sensor_info
                        else struct.pack("<H", 0x0000),
                        struct.pack("<H", S_ID_PRESENT_DEVICE_OPERATING_TEMPERATURE)
                        if S_ID_PRESENT_DEVICE_OPERATING_TEMPERATURE in tag.sensor_info
                        else struct.pack("<H", 0x0000),
                    )
                if self.demo_auto_reconfigure:
                    self.ap_update_complete(tag.esl_id, tag.group_id)

    def demo_esl_event_connection_closed(self, evt: esl_lib.EventConnectionClosed):
        """ESL library connection closed event handler extension for demo mode"""
        if (
            self.demo_auto_reconfigure
            and len(self.tag_db.list_state((TagState.CONNECTED, TagState.CONNECTING)))
            == 0
        ):
            self.demo_auto_reconfigure = False
        if self.controller_command == CCMD_DISCONNECT:
            self.notify_controller(self.controller_command, CONTROLLER_COMMAND_SUCCESS)
        elif self.controller_command != None:
            # In case of other command, the connection should't be closed -if closed it's a failure
            self.notify_controller(self.controller_command, CONTROLLER_COMMAND_FAIL)

    def demo_esl_event_error(self, evt: esl_lib.EventError):
        """ESL library error event handler extension for demo mode"""
        if evt.lib_status == elw.ESL_LIB_STATUS_CONN_FAILED:
            if self.controller_command != None:
                self.notify_controller(self.controller_command, CONTROLLER_COMMAND_FAIL)
        elif evt.lib_status == elw.ESL_LIB_STATUS_OTS_ERROR:
            if self.controller_command == CCMD_REQUEST_DATA:
                self.notify_controller(CCMD_IMAGE_UPDATE, CONTROLLER_COMMAND_FAIL)
        elif evt.lib_status in [
            elw.ESL_LIB_STATUS_OTS_INIT_FAILED,
            elw.ESL_LIB_STATUS_OTS_META_READ_FAILED,
        ]:
            tag = self.tag_db.find(evt.node_id)
            self.disconnect(tag)
