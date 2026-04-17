"""
Main AP commands mixin for AccessPoint.

This mixin contains the AP 'ap_*' command methods moved out of AccessPoint to make
the core class smaller and to prepare for further refactor into multiple mixins.
Methods rely on AccessPoint instance attributes and DOES NOT define instance state.
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

import re
import struct
from io import BytesIO
from datetime import datetime as dt
from esl_command import ESLCommand
from qrcode_generator import generate_qrcode
import esl_lib
import esl_key_lib
import ap_json_helper
import esl_lib_wrapper as elw
from ap_constants import (
    ADDRESS_TYPE_PUBLIC_ADDRESS,
    ADDRESS_TYPE_STATIC_ADDRESS,
    BROADCAST_ADDRESS,
    CCMD_CONNECT,
    CCMD_DISCONNECT,
    CCMD_DISPLAY_IMAGE,
    CCMD_IMAGE_UPDATE,
    CCMD_LED,
    CCMD_LIST,
    CCMD_PING,
    CCMD_REQUEST_DATA,
    CCMD_UNASSOCIATE,
    CMD_AP_CONTROL_ADV_ENABLE,
    CONTROLLER_COMMAND_FAIL,
    CONTROLLER_COMMAND_SUCCESS,
    CONTROLLER_REQUEST_LAST_DATA,
    CONTROLLER_REQUEST_MORE_DATA,
    REQUEST_IMAGE_DATA_HEADER,
    REQUEST_IMAGE_DATA_RESERVED,
    TLV_OPCODE_DISPLAY_IMAGE,
    TLV_OPCODE_DISPLAY_TIMED_IMAGE,
    TLV_OPCODE_FACTORY_RST,
    TLV_OPCODE_PING,
    TLV_OPCODE_READ_SENSOR,
    TLV_OPCODE_REFRESH_DISPLAY,
    TLV_OPCODE_SERVICE_RST,
    TLV_OPCODE_UNASSOCIATE,
    TLV_OPCODE_UPDATE_COMPLETE,
    TLV_OPCODE_VENDOR_SPECIFIC,
    VALID_ESL_ID_NUMBER_REGEX,
)
from ap_logger import log
from esl_tag import ImageUpdateFailed, ImageTypeRequired, TagState, EslState, Tag
from ap_config import INITIAL_AUTO_ADVERTISE_PAWR_TRAIN, IOP_TEST
from ap_sensor import S_ID_PRESENT_DEVICE_OPERATING_TEMPERATURE, S_ID_PRESENT_INPUT_VOLTAGE
# The mixin intentionally imports only what's needed by the CLI methods.

class CLICommandsMixin:
    """CLI user commands (extracted from AccessPoint)."""

    def ap_adv_start(self):
        """Start advertising for the demo mode if not already advertising"""
        if not self.demo_mode:
            self.lib.general_command(CMD_AP_CONTROL_ADV_ENABLE, b"\x01")
            self.log.info("Demo mode enabled.")
            self.demo_mode = True
            self.cmd_mode = True
            if not self.scan_runs:
                self.log.warning("Scan is disabled in demo mode: this may lead to unexpected timeouts in the demo controller!")
            if self.pawr_active is None:
                self.log.error("PAwR is disabled in demo mode: ESLs won't enter the Synchronized state or react to commands from the demo controller!")
            self.set_mode_handlers()
        else:
            self.log.info("Demo mode is already enabled.")

    def ap_adv_stop(self):
        """Stop advertising, call on demo mode deactivation"""
        if self.demo_mode:
            self.lib.general_command(CMD_AP_CONTROL_ADV_ENABLE, b"\x00")
            self.demo_mode = False
            if self.demo_controller_connected:
                self.log.warning(
                    "Demo controller is currently connected, demo mode will remain active until disconnection!"
                )
            else:
                self.set_mode_handlers()
                self.log.info(
                    "Demo mode disabled, switch to manual mode. To enable auto mode, please issue 'mode auto' command."
                )
        else:
            if self.demo_controller_connected:
                self.log.warning(
                    "Demo controller is still connected, demo mode disable requested already!"
                )
            else:
                self.log.info("Not in Demo mode.")

    def ap_demo_status(self):
        """Get the current status of the demo mode"""
        log(
            "  Current demo status: {0}, controller {1}.".format(
                "enabled"
                if self.demo_mode
                else "disabled"
                if not self.demo_controller_connected
                else "will be disabled",
                "connected" if self.demo_controller_connected else "not connected",
            )
        )

    def ap_scan(self, start, active=False):
        """
        Start or stop scanning for advertising ESL devices.
        input:
            - start: 'True': start scanning, 'False': stop scanning
        """
        if start is None:
            log(
                f"Scanning is currently{' ' if self.scan_runs else ' not '}in progress."
            )
        elif start:
            self.start_scan(active, clear_lists=True)
        else:
            self.stop_scan()

    def ap_connect(self, esl_id, bt_addr: str, group_id, address_type):
        """
        Connect to an ESL device with the specified address.
        input:
            - esl_id:   ESL ID or 'all' - the latter with special meaning: try connecting to more advertising tags at once
            - bt_addr:  Bluetooth address
            - group_id: ESL group ID
        """
        connecting_to = []
        if esl_id is not None:
            if esl_id == "all":
                connecting_to = [
                    tag
                    for tag in self.tag_db.list_state(TagState.IDLE)
                    if (
                        tag.advertising
                        and (group_id is None or tag.group_id == group_id)
                    )
                ]
            else:
                if group_id is None:
                    group_id = 0
                tag = self.tag_db.find((esl_id, group_id))
                if tag is None:
                    self.log.error(
                        "Can't connect to unknown tag: ESL ID: %u, Group ID: %u",
                        esl_id,
                        group_id,
                    )
                    return
                else:
                    connecting_to.append(tag)
        elif bt_addr is not None:
            if address_type is None:
                bt_address_public = esl_lib.Address.from_str(
                    bt_addr, ADDRESS_TYPE_PUBLIC_ADDRESS
                )
                bt_address_static = esl_lib.Address.from_str(
                    bt_addr, ADDRESS_TYPE_STATIC_ADDRESS
                )
                tags = [
                    self.tag_db.find(bt_address_public),
                    self.tag_db.find(bt_address_static),
                ]
                tag_count = sum(x is not None for x in tags)
                if tag_count == 0:
                    self.log.debug(
                        "No address type given - using default public address type."
                    )  # Will result in using the default ADDRESS_TYPE_PUBLIC_ADDRESS
                elif tag_count != 1:
                    self.log.error(
                        "There are more tags in the database with same address but different address type, please specify the address type!"
                    )
                    return
                else:
                    tag = next(item for item in tags if item is not None)
                    address_type = tag.ble_address.address_type
            bt_address = esl_lib.Address.from_str(bt_addr, address_type)
            tag = self.tag_db.find(bt_address)
            if tag is None or (
                address_type is not None
                and tag.ble_address.address_type != address_type
            ):
                tag = self.tag_db.add(self.lib, bt_address)
            connecting_to.append(tag)
        else:
            connecting_to = [
                tag
                for tag in self.tag_db.list_state(TagState.IDLE)
                if (tag.advertising and (group_id is None or tag.group_id == group_id))
            ]
            if len(connecting_to) > 1:
                if group_id is None:
                    self.log.warning(
                        "There are more than one tags advertising, please specify one or issue command with argument: 'all'!"
                    )
                else:
                    self.log.warning(
                        "There are more than one advertising tag in group %d, please specify one or issue command with argument: 'all -g %d'!",
                        group_id,
                        group_id,
                    )
                self.ap_list(["advertising"], group_id=group_id)
                return

        if len(connecting_to) == 0:
            self.log.warning("There's no advertising tag to connect to!")
            return

        for tag in connecting_to:
            if tag.state in (TagState.CONNECTED, TagState.CONNECTING):
                if (
                    self.controller_command == CCMD_CONNECT
                    and not self.demo_auto_reconfigure
                    and tag.state == TagState.CONNECTED
                ):
                    # If a request is coming from the demo controller by scanning the QR code of an already connected but yet unconfigured tag...
                    self.notify_controller(
                        self.controller_command, CONTROLLER_COMMAND_SUCCESS
                    )  # ...then just send the acknowledge immediately
                self.log.warning(
                    "%s already to %s, request ignored.", tag.state, tag.ble_address
                )
                continue

            if not self.cmd_mode and not self.auto_override:
                self.auto_override = True

            if not self.max_conn_count_reached:
                self.connect(tag)
            else:
                self.log.warning(
                    "Maximum number of available connections reached, connecting to 'all' halted!"
                )
                return

    def ap_disconnect(self, esl_id, bt_addr: str, group_id):
        """
        Disconnect from an ESL device with the specified address.
        Do Periodic Advertisement Sync Transfer during the procedure.
        input:
            - esl_id:   ESL ID
            - bt_addr:  Bluetooth address
            - group_id: ESL group ID
        """
        disconnect_from = []

        if esl_id is not None:
            if esl_id == "all":
                disconnect_from = [
                    tag
                    for tag in self.tag_db.list_state(
                        (TagState.CONNECTING, TagState.CONNECTED)
                    )
                    if (group_id is None or tag.group_id == group_id)
                ]
                if not disconnect_from:
                    self.log.error("No connected tag present!")
            else:
                if group_id is None:
                    group_id = 0
                tag = self.tag_db.find((esl_id, group_id))
                if tag is None:
                    self.log.error(
                        "Can't disconnect from unknown tag: ESL ID: %u, Group ID: %u",
                        esl_id,
                        group_id,
                    )
                else:
                    disconnect_from.append(tag)
        elif bt_addr is not None:
            # Also checking CONNECTING state, because it is not possible to abort connection process.
            tag = self.tag_db.find(bt_addr)
            if tag is not None and tag.state != TagState.CONNECTED:
                tag = None
            if tag is None:
                self.log.error("Can't disconnect from address %s!", bt_addr)
            else:
                disconnect_from.append(tag)
        else:
            tag = self.get_active_tag()
            if tag is not None:
                disconnect_from.append(tag)

        if len(disconnect_from) == 0:
            if self.controller_command == CCMD_DISCONNECT:
                self.notify_controller(CCMD_DISCONNECT, CONTROLLER_COMMAND_FAIL)

        for tag in disconnect_from:
            if tag.provisioned:
                if tag.esl_id is not None:
                    self.ap_update_complete(tag.esl_id, tag.group_id)
                else:
                    self.past(tag)
            else:
                self.disconnect(tag)

    def ap_config(self, params: dict, bt_addr: str = None):
        """
        Configure the writable mandatory GATT characteristics of the ESL tag(s).
        input:
            - params:   Configuration parameter dictionary. For further details see
                        the parameters of config command.
        """
        ALL = "all"
        tags_to_configure = []
        if bt_addr == ALL:
            tags_to_configure = self.tag_db.list_esl_state(
                (EslState.UPDATING, EslState.CONFIGURING)
            )
            if not tags_to_configure:
                self.log.error("No connected tag present!")
        else:
            tag = self.get_active_tag(bt_addr)
            if tag is None:
                if bt_addr is not None:
                    self.log.error(
                        "ESL at address: %s is not connected, nothing to configure.",
                        bt_addr,
                    )
                return
            else:
                tags_to_configure.append(tag)

        for tag in tags_to_configure:
            values = {}
            esl_addr = None
            group_id = None
            # Check all first
            if "full" in params.keys():
                values = self.configure(tag)
                esl_addr = values[elw.ESL_LIB_DATA_TYPE_GATT_ESL_ADDRESS][0]
            for key, param in params.items():
                # ESL ID
                if key == "esl_addr" and bt_addr != ALL:
                    if param == BROADCAST_ADDRESS:
                        if not IOP_TEST:
                            self.log.error("Broadcast address as ESL ID is prohibited, ESL address change request will be ignored!")
                            continue
                        else:
                            self.log.warning("IOP test mode enabdled, trying to set the broadcast address as ESL address.")
                    esl_addr = param & 0xFF
                    self.log.info("Set ESL ID to %d.", esl_addr)
                    if group_id is None:
                        if tag.group_id is not None:
                            group_id = tag.group_id
                        else:
                            group_id = 0
                # Group
                elif key == "group_id":
                    if param & 0x80 != 0:
                        if not IOP_TEST:
                            self.log.error("RFU bit in group ID shall not be set, ESL group change request will be ignored!")
                            continue
                        else:
                            self.log.warning("IOP test mode enabled, trying to set an invalid ESL address with RFU bit set in group ID value.")
                    group_id = param & 0xFF
                    self.log.info("Set group ID to %d.", group_id)
                    if esl_addr is None:
                        if tag.esl_id is not None:
                            esl_addr = tag.esl_id
                        else:
                            esl_addr = self.new_auto_address(tag.id)
                            self.log.warning(
                                "ESL group entered without a valid ESL ID - the ESL ID set to %d automatically to avoid ambiguous network configuration.",
                                esl_addr & 0xFF,
                            )
                # Sync Key
                elif key == "sync_key":
                    values[elw.ESL_LIB_DATA_TYPE_GATT_AP_SYNC_KEY] = self.ap_key
                # Response Key
                elif key == "response_key":
                    values[
                        elw.ESL_LIB_DATA_TYPE_GATT_RESPONSE_KEY
                    ] = self.ead.generate_key_material()
                # Raw Absolute Time value
                elif key == "absolute_time":
                    absolute_time = param
                    values[
                        elw.ESL_LIB_DATA_TYPE_GATT_CURRENT_TIME
                    ] = absolute_time.to_bytes(4, "little")
                # Time
                elif key == "time":
                    absolute_time = self.get_absolute_time()
                    values[
                        elw.ESL_LIB_DATA_TYPE_GATT_CURRENT_TIME
                    ] = absolute_time.to_bytes(4, "little")

            if group_id is not None and esl_addr is not None:
                values[elw.ESL_LIB_DATA_TYPE_GATT_ESL_ADDRESS] = bytes(
                    [esl_addr, group_id]
                )
            if len(values):
                self.write_values(tag, values)
            else:
                self.log.error("No characteristic to configure, request ignored!")

    def ap_imageupdate(
        self,
        image_index,
        file,
        raw=False,
        display_ind=None,
        label=None,
        rotation=None,
        cropfit=False,
        address=None,
        group_id=None,
    ):
        """
        Update tag image.
        inputs:
            - address:      Either Bluetooth address or ESL ID or 'all'
            - group_id:     ESL group ID
            - image_index:  Image index
            - filename:     Filename with path
            - raw:          Open and load file without conversion
            - display_ind   Display index
            - label         Label to be printed as an overlay to the image
            - rotation      Clockwise (cw), Counter-clockwise (ccw), flip
        """
        qr_request = False
        # If image file is from console, check validity
        if isinstance(file, str) and not self.controller_command:
            self.raw_image = b""
            try:
                image_file = open(file, "rb")
                self.raw_image = image_file.read()
                image_file.close()
            except FileNotFoundError:
                raise ImageUpdateFailed(f"Cannot open image file: {file}")

        tags_to_update = []
        tag = None
        if (
            address is None
        ):  # if no address is given, then check if there's only one active connection
            tag = self.get_active_tag()
            if tag is None:
                return
        else:
            if not isinstance(
                address, esl_lib.Address
            ):  # Check for non-address datatype
                esl_id, gid = self.get_esl_address(address, group_id)
                if esl_id == BROADCAST_ADDRESS:
                    tags_to_update = [
                        tag
                        for tag in self.tag_db.list_state(TagState.CONNECTED)
                        if (group_id is None or tag.group_id == group_id)
                    ]
                    if len(tags_to_update) == 0:
                        self.log.error(
                            "No ESL from group %d seems to be connected, image upload failed. Please try another group.",
                            gid,
                        )
                elif esl_id is None:
                    tag = self.tag_db.find(address)
                    if tag is None:
                        self.log.error(
                            "Tag at address %s not found in any group, command not sent",
                            address,
                        )
                        return
                else:
                    tag = self.tag_db.find((esl_id, gid))
            else:
                tag = self.tag_db.find(address)

        if tag is not None:
            tags_to_update.append(tag)

        # If command is from AP remote controller (usually mobile running the demo application), send the notification
        if self.controller_command == CCMD_IMAGE_UPDATE:
            # This is the first request from the controller
            self.controller_image_index = image_index
            self.log.info(
                "Image update command arrived from controller with the following parameters: index %s, filename %s",
                image_index,
                file,
            )
            self.notify_controller(
                CCMD_REQUEST_DATA,
                CONTROLLER_COMMAND_SUCCESS,
                REQUEST_IMAGE_DATA_HEADER,
                self.image_data_offset,
                REQUEST_IMAGE_DATA_RESERVED,
            )
            return

        if file is None and len(tags_to_update) != 0:
            self.log.info(
                "QR code generation requested for %d number of tags to image slot %d",
                len(tags_to_update),
                image_index,
            )
            qr_request = True

        for tag in tags_to_update:
            try:
                if qr_request:
                    # Create Silabs' ESL Demo specific command encoded into a QR code
                    _, file = generate_qrcode(
                        f"connect {str(tag.ble_address).partition(',')[0]}", 128, 128
                    )
                    # Convert to byte stream as if sent from demo controller
                    img_byte_arr = BytesIO()
                    file.save(img_byte_arr, format="PNG")
                    # Each tag will have its own, unique QR now as byte stream input
                    file = img_byte_arr.getvalue()
                tag.image_update(
                    image_index, file, raw, display_ind, label, rotation, cropfit
                )
                self.log.info(
                    "Image update started for tag at %s to image slot %d",
                    tag.ble_address,
                    image_index,
                )
            except ImageUpdateFailed as ex:
                self.log.error(
                    "Image update failed for tag at %s to image slot %d",
                    tag.ble_address,
                    image_index,
                )
                self.log.error(ex)
                if not self.cmd_mode:
                    self.ap_update_complete(
                        tag.esl_id, tag.group_id
                    )  # To prevent the provisioning process from stalling in automatic mode, we need to disconnect.
                continue
            except ImageTypeRequired:
                self.log.debug(
                    "Type info required for image %d on tag at %s - request readout",
                    image_index,
                    tag.ble_address,
                )
                continue

    def ap_unassociate(self, address, group_id):
        """
        Unassociate tag from AP.
        inputs:
            - address:  Either Bluetooth address or ESL ID
            - group_id: ESL group ID
        """
        tlv = TLV_OPCODE_UNASSOCIATE
        esl_id, group_id = self.get_esl_address(address, group_id)
        data = bytearray(self.get_opcode_len(tlv))
        if esl_id is not None:
            data[0:2] = tlv, esl_id
            self.route_command(esl_id, group_id, data)
            if esl_id == BROADCAST_ADDRESS:
                synced_in_group = [
                    tag
                    for tag in self.tag_db.list_esl_state(EslState.SYNCHRONIZED)
                    if tag.group_id == group_id
                ]
                for tag in synced_in_group:
                    tag.block(
                        elw.ESL_LIB_STATUS_UNASSOCIATED
                    )  # blocking before calling remove_tag() will preserve the tag object in memory, but still remove its bonding database record!
                    if tag.state != TagState.CONNECTING:
                        self.remove_tag(tag=tag)
                if self.controller_command == CCMD_UNASSOCIATE:
                    self.notify_controller(
                        CCMD_UNASSOCIATE, CONTROLLER_COMMAND_SUCCESS, esl_id
                    )
            else:
                tag = self.tag_db.find((esl_id, group_id))
                if tag is not None and tag.esl_state not in [
                    EslState.UNASSOCIATED,
                    EslState.UNSYNCHRONIZED,
                ]:
                    tag.pending_unassociate = (
                        True  # need to set explicitly for tags in synchronized state
                    )
        elif IOP_TEST and re.fullmatch(VALID_ESL_ID_NUMBER_REGEX, address) is not None:
            data[0:2] = tlv, int(address)
            self.queue_pawr_command(ESLCommand(group_id, data))
            self.log.warning(
                "Tag at address %s not found in group %d, send over PawR due IOP_TEST mode",
                address,
                group_id,
            )
        else:
            try:
                bt_address = esl_lib.Address.from_str(address)
                if self.key_db.find_ltk(bt_address) is None:
                    raise esl_key_lib.Error(elw.SL_STATUS_NOT_FOUND)
                self.key_db.delete_node(bt_address)
                self.log.warning(
                    "Currently unconfigured tag at address %s removed from the bonding database",
                    address,
                )
            except (ValueError, esl_key_lib.Error):
                self.log.error(
                    "Tag at address %s not found in any group nor in bonding database, command ignored",
                    address,
                )
                if self.controller_command == CCMD_UNASSOCIATE:
                    self.notify_controller(CCMD_UNASSOCIATE, CONTROLLER_COMMAND_FAIL)

    def ap_list(self, param_list, verbosity_level=None, group_id=None):
        """
        List tag information.
        """
        list_of_tags: list[Tag] = []
        for param in param_list:
            verbose_param = verbosity_level
            # Advertising
            if param == "advertising":
                tags = self.tag_db.list_advertising()
                verbose_param = False
            # Blocked
            elif param == "blocked":
                tags = self.tag_db.list_blocked()
                verbose_param = False
            # Connected
            elif param == "connected":
                tags = self.tag_db.list_state(TagState.CONNECTED)
            # Connecting
            elif param == "initiating":
                tags = self.tag_db.list_state(TagState.CONNECTING)
            # Synchronized
            elif param == "synchronized":
                tags = self.tag_db.list_esl_state(EslState.SYNCHRONIZED)
                list_of_tags += tags
            # Unsynchronized
            elif param == "unsynchronized":
                tags = self.tag_db.list_esl_state(EslState.UNSYNCHRONIZED)
            # Invalid
            else:
                continue

            if group_id is not None:
                tags = [tag for tag in tags if tag.group_id == group_id]
            if tags is not None:
                tags = sorted(tags)

            for tag in tags:
                if verbose_param:
                    log(tag.get_info())
                    log("-" * 36)
                elif verbosity_level is not False:
                    if tag.blocked:
                        log(
                            str(tag)
                            + f", blocked by {esl_lib.get_enum('ESL_LIB_STATUS_', tag.blocked)} on {dt.fromtimestamp(tag.last_req_timestamp).strftime('%d/%b %H:%M:%S.%f')[:-3]}"
                        )
                    else:
                        log(tag)
            if len(tags) == 0:
                log(
                    f"There's no {param} tag"
                    + (f" in group {group_id}" if group_id is not None else " at all.")
                )
            elif len(tags) == 1:
                log(
                    f"There's one {param} tag"
                    + (f" in group {group_id}" if group_id is not None else " overall.")
                )
            else:
                log(
                    f"There are {len(tags)} {param} tags"
                    + (f" in group {group_id}" if group_id is not None else " overall.")
                )

        if self.controller_command == CCMD_LIST:
            if len(list_of_tags) != 0:
                for tag in list_of_tags:
                    self.notify_controller(
                        CCMD_LIST,
                        CONTROLLER_COMMAND_SUCCESS,
                        CONTROLLER_REQUEST_MORE_DATA
                        if tag is not list_of_tags[-1]
                        else CONTROLLER_REQUEST_LAST_DATA,
                        tag.esl_address,
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
            else:
                self.notify_controller(
                    CCMD_LIST,
                    CONTROLLER_COMMAND_SUCCESS,
                    CONTROLLER_REQUEST_LAST_DATA,
                    0,
                )

    def ap_led(
        self,
        address,
        group_id,
        repeat_field,
        index,
        absolute_value,
        gamut=0,
        pattern=None,
    ):
        """
        Turn on / off or flash an LED utilizing the LED control command.
        input:
            - address:          Either Bluetooth address or ESL ID
            - group_id:         ESL group ID
            - repeat_field:     How many times the pattern shall be repeated
            - index:            Index of the LED
            - absolute_value:   Execution time of the command in ESL Absolute Time epoch value
            - gamut:            LED gamut
            - pattern:          Flashing pattern
        """
        esl_id, group_id = self.get_esl_address(address, group_id)

        if esl_id is None:
            self.log.error(
                "Tag at address %s not found in any group, command not sent", address
            )
            return

        self.led_control_command(
            esl_id, group_id, repeat_field, index, absolute_value, gamut, pattern
        )
        if self.controller_command == CCMD_LED and esl_id == BROADCAST_ADDRESS:
            self.notify_controller(CCMD_LED, CONTROLLER_COMMAND_SUCCESS, esl_id)

    def ap_read_sensor(self, address, group_id, sensor_idx):
        """
        Read sensor information.
        input:
            - address:      Either Bluetooth address or ESL ID
            - sensor_idx:   Sensor index
            - group_id:     ESL group ID
        """
        tlv = TLV_OPCODE_READ_SENSOR
        esl_id, group_id = self.get_esl_address(address, group_id)

        if esl_id is None:
            self.log.error(
                "Tag at address %s not found in any group, command not sent", address
            )
            return

        data = bytearray(self.get_opcode_len(tlv))
        data[0:3] = tlv, esl_id, sensor_idx
        self.route_command(esl_id, group_id, data)

    def ap_factory_reset(self, address, group_id, force_pawr=False):
        """
        Execute factory reset on tag.
        inputs:
            - address:  Either Bluetooth address or ESL ID
            - group_id: ESL group ID
        """
        tlv = TLV_OPCODE_FACTORY_RST
        esl_id, group_id = self.get_esl_address(address, group_id)
        if esl_id is not None:
            data = bytearray(self.get_opcode_len(tlv))
            tag = None
            data[0:2] = tlv, esl_id
            self.route_command(esl_id, group_id, data, force_pawr)
            esl_state = EslState.SYNCHRONIZED
            if esl_id != BROADCAST_ADDRESS:
                tag = self.tag_db.find((esl_id, group_id))
                esl_state = tag.esl_state
            if (
                not force_pawr
                and tag is not None
                and esl_state in [EslState.CONFIGURING, EslState.UPDATING]
            ):
                tag.block()  # keep tag objects in memory
                self.remove_tag(tag=tag)  # delete the bonding database record
                tag.unblock()  # release blocking as it was only needed for bonding removal
            else:
                synced_in_group = [
                    tag
                    for tag in self.tag_db.list_esl_state(EslState.SYNCHRONIZED)
                    if tag.group_id == group_id
                ]
                if len(synced_in_group) > 0:
                    self.log.error(
                        "Factory reset command is invalid and thus ignored by tags in Synchronized state!"
                    )
        else:
            self.log.warning(
                "Tag at address %s not found in any group, command not sent", address
            )

    def ap_update_complete(self, address, group_id):
        """
        Issue an Update Complete command to connected tag(s)
        inputs:
            - address:  Either Bluetooth address or ESL ID
            - group_id: ESL group ID
        """
        tag = None
        tlv = TLV_OPCODE_UPDATE_COMPLETE
        esl_id, gid = self.get_esl_address(address, group_id)

        if esl_id is not None:
            tag = self.tag_db.find((esl_id, gid))
            if tag.esl_state == EslState.SYNCHRONIZED:
                self.log.warning(
                    "Update complete command is invalid in Synchronized state!"
                )
        else:
            self.log.error(
                "Tag at address %s not found in any group, command not sent", address
            )
            return

        # Check for connection
        if tag is not None and tag.state != TagState.CONNECTED:
            self.log.error("Update Complete can be used only with active connection!")
            if not IOP_TEST:
                return

        data = bytearray(self.get_opcode_len(tlv))
        data[0:2] = tlv, esl_id
        self.route_command(esl_id, group_id, data)

    def ap_refresh_display(self, address, group_id, display_idx):
        """
        Refresh tag display.
        input:
            - address:      Either Bluetooth address or ESL ID
            - display_idx:  Display index
            - group_id:     ESL group ID
        """
        tlv = TLV_OPCODE_REFRESH_DISPLAY
        esl_id, group_id = self.get_esl_address(address, group_id)

        if esl_id is None:
            self.log.error(
                "Tag at address %s not found in any group, command not sent", address
            )
            return

        data = bytearray(self.get_opcode_len(tlv))
        data[0:3] = tlv, esl_id, display_idx
        self.route_command(esl_id, group_id, data)

    def ap_display_image(
        self, address, group_id, image_idx, display_idx, absolute_value
    ):
        """
        Display tag image.
        input:
            - address:       Either Bluetooth address or ESL ID
            - image_idx:     Image index
            - display_idx:   Display index
            - group_id:      ESL group ID
            - delay_ms:      Delay in milliseconds
            - absolute_base: ESL Absolute Time epoch value
        """
        tlv = TLV_OPCODE_DISPLAY_IMAGE
        esl_id, group_id = self.get_esl_address(address, group_id)

        if esl_id is None:
            self.log.error(
                "Tag at address %s not found in any group, command not sent", address
            )
            return

        if absolute_value is not None:
            tlv = TLV_OPCODE_DISPLAY_TIMED_IMAGE
        data = bytearray(self.get_opcode_len(tlv))
        data[0:4] = tlv, esl_id, display_idx, image_idx

        if absolute_value is not None:
            data[4:8] = struct.pack("<I", (absolute_value & 0xFFFFFFFF))
            if absolute_value == 0:
                self.log.info(
                    "Delete timed display image command of display index %d",
                    display_idx,
                )
            else:
                self.log.info(
                    "Delayed display image command issued at absolute time %d",
                    absolute_value,
                )
        self.route_command(esl_id, group_id, data)
        if self.controller_command == CCMD_DISPLAY_IMAGE:
            self.log.info("Display image command arrived from controller")
            if esl_id == BROADCAST_ADDRESS:
                self.notify_controller(
                    CCMD_DISPLAY_IMAGE, CONTROLLER_COMMAND_SUCCESS, esl_id
                )

    def ap_ping(self, address, group_id, force_pawr=False):
        """
        Send ESL ping command.
        input:
            - address:      Either Bluetooth address or ESL ID
            - group_id:     ESL group ID
        """
        tlv = TLV_OPCODE_PING
        esl_id, group_id = self.get_esl_address(address, group_id)

        if esl_id is None:
            self.log.error(
                "Unknown address can be a valid ESL ID only, command ignored!"
            )
            return
        elif esl_id == BROADCAST_ADDRESS and not IOP_TEST:
            self.log.error("Using broadcast with ping makes no sense, command ignored!")
            if self.controller_command != None:
                self.notify_controller(self.controller_command, CONTROLLER_COMMAND_FAIL)
            return

        data = bytearray(self.get_opcode_len(tlv))
        data[0:2] = tlv, esl_id
        self.route_command(esl_id, group_id, data, force_pawr)
        if self.controller_command == CCMD_PING and esl_id == BROADCAST_ADDRESS:
            self.notify_controller(CCMD_PING, CONTROLLER_COMMAND_SUCCESS, esl_id)

    def ap_vendor_opcode(self, address, group_id, vendor_data=None):
        """
        Send ESL vendor specific opcode.
        input:
            - address:      Either Bluetooth address or ESL ID
            - group_id:     ESL group ID
            - data:         ESL vendor specific TLV
        """
        tlv = TLV_OPCODE_VENDOR_SPECIFIC
        esl_id, group_id = self.get_esl_address(address, group_id)

        if esl_id is None:
            self.log.error(
                "Unknown address can be a valid ESL ID only, command ignored!"
            )
            return

        data_length = self.get_opcode_len(tlv)

        if vendor_data is not None:
            extra_len = len(vendor_data)
            data_length += extra_len
            tlv = self.set_tlv_len(tlv, extra_len)

        data = bytearray(data_length)
        data[0:2] = tlv, esl_id
        if vendor_data is not None:
            data[2:] = vendor_data
        self.route_command(esl_id, group_id, data)

    def ap_service_reset(self, address, group_id):
        """
        Send Service Reset command.
        input:
            - address:      Either Bluetooth address or ESL ID
            - group_id:     ESL group ID
        """
        tlv = TLV_OPCODE_SERVICE_RST
        esl_id, group_id = self.get_esl_address(address, group_id)

        if esl_id is None:
            self.log.error(
                "Tag at address %s not found in any group, command not sent", address
            )
            return

        data = bytearray(self.get_opcode_len(tlv))
        data[0:2] = tlv, esl_id
        self.route_command(esl_id, group_id, data)

    def ap_sync(self, start, pa_interval=None, advertise=False):
        """
        Start / stop sending synchronization packets.
        input:
            - start:            Start sending periodic synchronization packets
            - pa_interval:      Periodic advertising interval list in ms.
                                See 'sync' command for more details.
        """
        syncronized_tags = self.tag_db.list_esl_state(EslState.SYNCHRONIZED)
        if len(syncronized_tags) and start is not None:
            self.log.warning(
                "There are already synchronized tags, they will lose sync!"
            )

        if start is None:
            state_text = {
                True: "running",
                False: "not running yet",
                None: "not running",
            }[self.pawr_active]

            log(
                f"PAwR sync is currently {state_text}"
                f"{' and being advertised' if self.pawr_advertising else ''}"
            )
        elif not start:
            self.stop_pawr_train()
            # Count queued commands
            with self.esl_command_queue_lock:
                queued_count = sum(len(cmds) for cmds in self.esl_queued_commands.values())
                self.esl_queued_commands.clear()
            # Count pending commands
            with self.esl_pending_commands_lock:
                pending_count = sum(len(cmds) for cmds in self.esl_pending_commands.values())
                self.esl_pending_commands.clear()
            # Log only if something was actually dropped
            if queued_count or pending_count:
                self.log.warning(
                    "PAwR stop requested: %d unsent commands and %d pending commands discarded!",
                    queued_count,
                    pending_count,
                )
        else:
            self.start_pawr_train(pa_interval, advertise)
            # Restart scanning
            if self.scan_runs and (not self.cmd_mode or self.auto_override):
                self.start_scan(clear_lists=True)

    def ap_mode(self, auto_mode, lib_connection_mode=None):
        """
        Changes ESL Access Point operation mode. Also changes the ESL library connection mode on explicit user requests
         (i.e. ESL connectivity mode change doesn't happen on implicit mode change driven by the AP code)
        input:
            - auto_mode: 'True': automatic, 'False': manual, 'None': print current mode
            - library_connection_mode: '0': single, '1' Filter Accept List based
        """
        if lib_connection_mode is not None and not lib_connection_mode in [
            elw.ESL_LIB_CONNECTION_MODE_SINGLE,
            elw.ESL_LIB_CONNECTION_MODE_LIST,
        ]:
            self.log.error(
                "Invalid connection mode request value %d, request ignored!",
                lib_connection_mode,
            )
            return
        if auto_mode == True:
            if self.demo_controller_connected:
                self.log.error(
                    "Demo controller is currently connected, auto mode can't be set!"
                )
                return
            self.cmd_mode = False
            if not self.auto_override:
                if self.pawr_active is None:
                    self.start_pawr_train(advertise=INITIAL_AUTO_ADVERTISE_PAWR_TRAIN)
                    self.log.debug("PAwR sync start requested.")
                else:
                    self.log.debug("PAwR sync train already running, no need to start.")
                # Send asynchronous mode change request
                self.lib.set_connection_mode(
                    elw.ESL_LIB_CONNECTION_MODE_LIST
                    if lib_connection_mode is None
                    else lib_connection_mode
                )
                self.start_scan(clear_lists=True)
            if self.demo_mode:
                self.demo_mode = False
                self.log.warning("Demo mode disabled by changing to auto mode.")
                self.lib.general_command(CMD_AP_CONTROL_ADV_ENABLE, b"\x00")
            self.auto_override = False
            self.log.info("Operation mode: automated")
        elif auto_mode == False:
            self.cmd_mode = True
            self.auto_override = False
            self.log.info("Operation mode: manual")
            # Send asynchronous mode change request - may be discarded by the library if the NCP target is busy, but doesn't hurt business
            self.lib.set_connection_mode(
                elw.ESL_LIB_CONNECTION_MODE_SINGLE
                if lib_connection_mode is None
                else lib_connection_mode
            )
        else:
            log(
                "  Current AP mode: {0}, ESL library connection mode: {1}".format(
                    "manual" if self.cmd_mode else "automated",
                    "single"
                    if (self.lib_connection_mode == elw.ESL_LIB_CONNECTION_MODE_SINGLE)
                    else "list based",
                )
            )
        self.set_mode_handlers()

    def ap_network(self, export_file=None, import_file=None, exclusive_mode=None):
        """
        Control ESL Access Point network auto configuration and/or save actual network config.
        """
        if all(value is None for value in [export_file, import_file, exclusive_mode]):
            self.log.info(
                "Automatic ESL Address configuration is %s.",
                "currently available for all valid ESL devices"
                if not self.exclusive_network.enabled
                else f"only allowed for an exclusive set of {self.tag_db.device_count} known ESL devices",
            )
            return
        try:
            json = ap_json_helper.JSONHelper()
            if export_file is not None:
                list = json.export_network_config(
                    self.tag_db.list_esl_state(
                        (EslState.SYNCHRONIZED, EslState.UNSYNCHRONIZED)
                    ),
                    export_file,
                )
                if (group_count := len(list)) != 0:
                    device_count = self.tag_db.device_count
                    self.log.info(
                        "Successfully saved network config for %d device%s in %d group%s to file '%s'.",
                        device_count,
                        "" if device_count == 1 else "s",
                        group_count,
                        "" if group_count == 1 else "s",
                        export_file,
                    )
            if import_file is not None:
                list = json.import_network_config(
                    self.lib, self.key_db, self.tag_db, import_file
                )

                if (group_count := len(list)) == 0:
                    self.log.error(
                        "No devices could be loaded from the network config file: '%s'.",
                        import_file,
                    )
                else:
                    device_count = self.tag_db.device_count
                    self.log.info(
                        "Successfully loaded network config for %d device%s in %d group%s from file '%s'.",
                        device_count,
                        "" if device_count == 1 else "s",
                        group_count,
                        "" if group_count == 1 else "s",
                        import_file,
                    )
            if exclusive_mode is not None:
                device_count = self.tag_db.device_count
                self.exclusive_network.enabled = exclusive_mode
                if not exclusive_mode:
                    self.log.info("Exclusive network configuration disabled.")
                elif device_count == 0:
                    self.log.warning(
                        "Exclusive network configuration is enabled with no known ESLs.\n"
                        "Please import a configuration file later or configure the ESLs manually."
                    )
                else:
                    self.log.info(
                        "Exclusive network configuration is enabled for %d known ESL device%s.",
                        device_count,
                        "" if device_count == 1 else "s",
                    )
                # If exclusive mode is enabled AND a file was imported, store the filename.
                # Note: the current network configuration remains unchanged regardless of the
                # contents of the imported file. Devices and addresses already present in the
                # active network are not overwritten. If the imported file contains devices or
                # addresses that overlap with the existing network, address conflicts may occur
                # and manual reconfiguration might be required.
                if exclusive_mode and import_file is not None:
                    self.exclusive_network.source_file = import_file
                    if device_count != 0:
                        self.log.warning(
                            "The imported configuration may potentially overlap with the current network.\n" \
                            "Conflicts are not checked during file load - manual reconfiguration might be required."
                        )

        except Exception as e:
            self.log.error(e)

    def ap_set_rssi_threshold(self, rssi_tresh):
        """
        Set RSSI filter threshold value.
        input:
            - rssi_tresh:   RSSI filter threshold value
        """
        self.rssi_threshold = rssi_tresh
        self.log.info("New RSSI threshold value: %d", self.rssi_threshold)
