"""
Tag commands mixin for AccessPoint.

Contains tag-specific operations and routing/Control Point helpers.
This mixin does not introduce its own instance state, only operate on
attributes of the AccessPoint target instance.
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
    TLV_OPCODE_DISPLAY_IMAGE,
    TLV_OPCODE_FACTORY_RST,
    TLV_OPCODE_LED_CONTROL,
    TLV_OPCODE_LED_TIMED_CONTROL,
    TLV_OPCODE_PING,
    TLV_OPCODE_UNASSOCIATE,
)
from ap_config import (
    IMAGE_MAX_AUTO_UPLOAD_COUNT,
    IOP_TEST,
    KEY_MATERIAL_REUSE_ENABLED,
)
from esl_command import ESLCommand
from esl_tag import Tag, TagState, EslState, InvalidTagStateError, PAwRSyncLostError
import esl_lib
import esl_lib_wrapper as elw
import random
import struct

class TagCommandsMixin:
    """Tag-specific commands and routing"""

    def connect(
        self,
        tag: Tag,
        ap_identity: esl_lib.Address = esl_lib.Address.from_str("3425B4A91C8A"),
    ):
        """Establish connection with a tag normally or via PAwR"""
        # group ID shouldn't be None if the tag is in sychronized state, but better to doublecheck
        if tag.esl_state == EslState.SYNCHRONIZED and tag.group_id is not None:
            pawr = esl_lib.PAWRSubevent(self.pawr_handle, tag.group_id)
        else:
            pawr = None
        ltk = self.key_db.find_ltk(tag.ble_address)
        if ltk is not None:
            key_type = elw.ESL_LIB_KEY_TYPE_LTK
            self.log.info("Bonding LTK found for ESL at %s", tag.ble_address)
        else:
            key_type = elw.ESL_LIB_KEY_TYPE_NO_KEY
        self.log.info(
            "Request connecting to: %s %s",
            tag.ble_address,
            "over PAwR" if pawr else "via connectable advertisement",
        )
        try:
            tag.connect(pawr, key_type=key_type, key=ltk, timeout=2*1.25*self.adv_interval_max if pawr else None)
        except Exception as e:
            self.log.error(e)

    def write_values(self, tag: Tag, values: dict):
        """Write values to a tag (GATT configure)"""
        tag.configure_tag(values)

    def configure(self, tag: Tag):
        """Configure tag
        parameters:
            tag: ESL tag to configure"""
        rkey_material = None
        # Try to recover the previous ESL Address from the bonding database in the first place
        esl_record = self.key_db.find_esl(tag.ble_address)
        if esl_record is not None:
            self.log.debug(esl_record)
            if esl_record.esl_address is not None:
                tag.esl_address = esl_record.esl_address
            if esl_record.rk is not None and KEY_MATERIAL_REUSE_ENABLED:
                self.log.warning("SECURITY RISK: Response Key Material reuse enabled!")
                rkey_material = esl_record.rk
        if rkey_material is None:
            rkey_material = self.ead.generate_key_material()

        if tag.esl_address is None:
            esl_address = self.new_auto_address(tag.id)
            self.log.info("New auto ESL Address: 0x%04x", esl_address)
            tag.esl_address = esl_address # must be set explicitly because of TagDB indexing!
        else:
            esl_address = tag.esl_address
            self.log.info("Reuse ESL Address: 0x%04x", esl_address)

        absolute_time = self.get_absolute_time()

        values = {
            elw.ESL_LIB_DATA_TYPE_GATT_ESL_ADDRESS: esl_address.to_bytes(2, "little"),
            elw.ESL_LIB_DATA_TYPE_GATT_RESPONSE_KEY: rkey_material,
            elw.ESL_LIB_DATA_TYPE_GATT_AP_SYNC_KEY: self.ap_key,
            elw.ESL_LIB_DATA_TYPE_GATT_CURRENT_TIME: absolute_time.to_bytes(
                4, "little"
            ),
        }
        return values

    def disconnect(self, tag: Tag):
        """Close connection with a tag"""
        try:
            tag.close_connection()
        except esl_lib.CommandFailedError as err:
            self.log.warning("Failed to disconnect from %s: %s", tag.ble_address, err)

    def past(self, tag: Tag):
        """Do Periodic Advertisement Sync Transfer over connection"""
        if tag.past_initiated:
            return
        elif not self.pawr_active:
            if self.cmd_mode:
                self.log.warning("PAwR is not running, tag won't be synchronized!")
            else:
                self.log.error(
                    "PAwR is not running, auto provisioning will stop until PAwR train is started by 'sync start' command!"
                )
            tag.close_connection()
            return
        try:
            tag.initiate_past(self.pawr_handle, self.pa_timer_interval, self.subevent_count)
        except PAwRSyncLostError as e:
            tag.block(elw.ESL_LIB_STATUS_PAST_INIT_FAILED)
            self.log.error(e)
        except Exception as e:
            self.log.error(e)

    def upload_auto_image(self, image_index, tag: Tag):
        """Automatic image update helper"""
        image_path = self.image_path + random.choice(
            random.sample(self.image_files, len(self.image_files))
        )
        self.ap_imageupdate(image_index, image_path, address=tag.ble_address)

    def upload_next_image(self, tag: Tag):
        """Upload next image in the automatic image upload sequence (kept here for tag-related logic)"""
        if tag is None:
            return
        tag.auto_image_count += 1
        if tag.auto_image_count < min(
            (tag.max_image_index + 1), IMAGE_MAX_AUTO_UPLOAD_COUNT
        ):
            self.log.info("Sending new image to ESL at address %s", tag.ble_address)
            self.upload_auto_image((tag.auto_image_count % len(self.image_files)), tag)
        elif tag.provisioned:
            disp_image = bytearray(self.get_opcode_len(TLV_OPCODE_DISPLAY_IMAGE))
            disp_image[0:4] = TLV_OPCODE_DISPLAY_IMAGE, tag.esl_id, 0, 0
            disp_image = bytes(disp_image)
            self.send_cp_command(tag, disp_image)
            self.log.info("Display Image command sent")

    def route_command(self, esl_id, group_id, data, force_pawr=False):
        """Auto route commands between periodic advertisement (with responses) and ESL Control Point"""
        if esl_id == BROADCAST_ADDRESS:
            connected_in_group = [
                tag
                for tag in self.tag_db.list_state(
                    (TagState.CONNECTED, TagState.CONNECTING)
                )
                if (group_id is None or tag.group_id == group_id)
            ]
            control_point_data = data.copy()
            # Handle the connected ESL tag(s) within the given group explicitly, if any
            for tag in connected_in_group:
                # broadcast addresses are otherwise routed over periodic advertisement, always!
                if (
                    not IOP_TEST
                ):  # following line could break future IOP BI test cases that require writing 0xFF ESL ID to ESL CP!
                    control_point_data[
                        1
                    ] = (
                        tag.esl_id
                    )  # ESL Service Spec v1.0 3.9.2 Command behavior, paragraph 3: broadcast address would be rejected by INVALID_PARAMETER error.
                self.send_cp_command(tag, control_point_data)
                if control_point_data[0] == TLV_OPCODE_UNASSOCIATE:
                    self.log.warning(
                        "Unassociate connected ESL tag at %s results in immediate disconnection!",
                        tag.ble_address,
                    )
                elif control_point_data[0] == TLV_OPCODE_FACTORY_RST:
                    self.log.warning(
                        "Factory reset of ESL tag at %s results in immediate disconnection!",
                        tag.ble_address,
                    )

        tag = self.tag_db.find((esl_id, group_id))
        if tag is not None:
            # check for different dispatch path override cases
            if force_pawr:
                self.log.warning("Override command dispatch path: send via PAwR")
                tag = None
            elif tag.state != TagState.CONNECTED:
                # tag exists but is currently not connected then send via PAwR by default
                if data[0] == TLV_OPCODE_PING:
                    tag.update_request_timestamp()
                tag = None
        elif IOP_TEST:
            # force sending commands with unknown ESL ID or PING to ALL (IOP test-only command!) to ESL CP
            active_tag = self.get_active_tag()
            if (
                active_tag is not None
                and len(active_tag) == 1
                and active_tag.group_id == group_id
                and (esl_id != BROADCAST_ADDRESS or (data[0] == TLV_OPCODE_PING))
            ):
                self.log.warning(
                    "Sending a deliberately misaddressed command to the current ESL Control Point"
                )
                tag = active_tag

        if tag is None:
            # Check if the current PAwR has the proper amount of subevents for the group request
            if group_id >= self.subevent_count:
                self.log.error(
                    "Sending to group %d is impossible because there are only %d subevents in current PAwR train!",
                    group_id,
                    self.subevent_count,
                )
                return
            self.queue_pawr_command(ESLCommand(group_id, data))
        else:
            self.send_cp_command(tag, data)

    def send_cp_command(self, tag: Tag, data: bytes):
        """Writes a command to the control point"""
        try:
            # Use GATT Write With Response in most cases, unless IOP_TEST is set. In that case, it will be randomly overridden.
            tag.write_control_point(
                data,
                att_response=True if not IOP_TEST else random.choice([True, False]),
            )
        except InvalidTagStateError:
            self.log.error(
                "ESL ID %d in group %d is not connected, Control Point can't be written!",
                tag.esl_id,
                tag.group_id,
            )

    def led_control_command(
        self,
        esl_id,
        group_id,
        repeat_field,
        led_idx,
        absolute_value,
        gamut=0,
        pattern=None,
    ):
        """Execute LED control command"""
        tlv = TLV_OPCODE_LED_CONTROL
        if absolute_value is not None:
            tlv = TLV_OPCODE_LED_TIMED_CONTROL
        if pattern is None:
            pattern = bytearray(7)

        data = bytearray(self.get_opcode_len(tlv))

        data[0:3] = tlv, esl_id, led_idx
        data[3] = gamut  # color and brightness bits
        data[4:11] = pattern  # Flashing_Pattern field
        data[11:13] = repeat_field
        if absolute_value is not None:
            data[13:17] = struct.pack("<I", (absolute_value & 0xFFFFFFFF))
            if absolute_value == 0:
                self.log.info(
                    "Delete timed LED control command of led index %d", led_idx
                )
            else:
                self.log.info(
                    "Delayed LED control command issued at absolute time %d",
                    absolute_value,
                )

        self.route_command(esl_id, group_id, data)

    def ltk_conditional_removal(self, tag: Tag, reason: int): # reason must be an elw.sl_status_t, but it's not a valid PEP 484 type annotation syntax
        """Remove stored LTK conditionally based on the disconnection reason and tag state"""
        if not tag.provisioned or (
            reason == elw.SL_STATUS_BT_CTRL_REMOTE_USER_TERMINATED
            and tag.pending_unassociate
        ):
            # delete stored LTK for tags that didn't finish provisioning before the connection closed according to ESL Profile spec.
            # or if there's a pending unassociate executed successfully
            self.log.debug(
                "The bonding data for ESL at %s deleted due to incomplete configuration.",
                tag.ble_address,
            )
            self.key_db.delete_ltk(tag.ble_address)
            tag.reset(keep_esl_address=self.exclusive_network.contains_address(tag.ble_address))

    def check_sync(self, tag: Tag):
        """Check if tag got synchronized despite the connection closing timeout"""
        if tag is not None and tag.provisioned and not tag.advertising:
            self.log.debug(
                "Check if Tag at address %s got synchronized despite the connection closing timeout.",
                tag.ble_address,
            )
            self.ap_ping(
                tag.esl_id, tag.group_id, force_pawr=True
            )  # Special edge case in which the synced flag may not be set after disconnection -> check if tag is synced
