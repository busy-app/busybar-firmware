"""
ESL AP Core - general helper mixin.
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

import random
import re
from datetime import datetime, timedelta, datetime as dt
from ap_constants import (
    BROADCAST_ADDRESS,
    CCMD_REQUEST_DATA,
    CMD_AP_CONTROL_CP_RESPONSE,
    CMD_AP_CONTROL_IT_RESPONSE,
    CONTROLLER_COMMAND_FAIL,
    ESL_MAX_DELAY,
    VALID_BD_ADDRESS_REGEX,
)
from ap_config import (
    ESL_MAX_TAGS_IN_AUTO_GROUP,
    ESL_CMD_MAX_PENDING_CONNECTION_REQUEST_COUNT,
)
from ap_core_utils import deep_size
import esl_lib
import esl_lib_wrapper as elw
from ap_logger import log
from esl_tag import Tag, TagState, EslState


class HelpersMixin:
    """Various helper methods for AccessPoint operations"""

    def new_auto_address(self, id):
        esl_id = id % ESL_MAX_TAGS_IN_AUTO_GROUP
        group_id = int(id // ESL_MAX_TAGS_IN_AUTO_GROUP)
        esl_address = (esl_id & BROADCAST_ADDRESS) | (group_id & 0x7F) << 8
        return esl_address

    def add_fake_tags(self, count):
        for _ in range(count):
            address = esl_lib.Address(bytes(random.choices(range(256), k=6)))
            tag = self.tag_db.add(self.lib, address)
            values = self.configure(tag)
            tag.gatt_values.update(values)

    def get_absolute_time(self):
        """Get absolute time in milliseconds"""
        delta_time = datetime.now() - self.start_time
        ms = delta_time / timedelta(microseconds=1000)
        return int(ms)

    def calculate_exec_time(self, now, d_hour, d_min, d_sec, d_micsec, date=None):
        """Calculate command execution time delay in milliseconds"""
        delay_ms = None
        delay_time = now
        delay_time = delay_time.replace(
            hour=d_hour, minute=d_min, second=d_sec, microsecond=d_micsec
        )
        if date is not None:
            delay_time = delay_time.replace(
                year=date.year, month=date.month, day=date.day
            )
        delta_time = delay_time - now
        ms = (
            delta_time.days * 24 * 60 * 60 + delta_time.seconds
        ) * 1000 + delta_time.microseconds / 1000.0
        if int(ms) > ESL_MAX_DELAY:
            self.log.error("Requested date and time exceeds ESL delay limit (48 days)!")
        elif int(ms) > 0:
            delay_ms = int(ms + 0.5)
        elif int(ms) < 0:
            if date is None:
                delay_ms = (
                    int(ms + 0.5) + 86400000
                )  # adds one day extra delay if time passed already and date is not specified
                self.log.warning("The time specified was interpreted as tomorrow time!")
            else:
                self.log.error("Requested date and time has passed already!")
        return delay_ms

    def get_opcode_len(self, tlv):
        """Decode opcode length from tlv"""
        o_len = (tlv & 0xF0) >> 4
        o_len += 2  # Add TLV and ESL_ID bytes to the length
        return o_len

    def set_tlv_len(self, tlv, len):
        """Encode length to a tlv"""
        new_tlv = (len & 0x0F) << 4
        new_tlv |= tlv & 0x0F
        return new_tlv

    def get_active_tag(self, bt_addr: str = None, state=TagState.CONNECTED):
        """CLI helper to get 1 connected tag if bt_addr is omitted"""
        tag = None
        if bt_addr is not None:
            if re.fullmatch(VALID_BD_ADDRESS_REGEX, str(bt_addr)) is not None:
                tag = self.tag_db.find(bt_addr)
                if tag is None:
                    self.log.error("Unknown tag address: %s", bt_addr)
            else:
                self.log.error("Invalid tag address: %s", bt_addr)
        else:
            tags = self.tag_db.list_state(state)
            if len(tags) == 0:
                self.log.error("No connected tag present!")
            elif len(tags) == 1:
                tag = tags[0]
            else:
                address_list = ", ".join([str(t.ble_address) for t in tags])
                self.log.warning(
                    "%u connected tags present, select one: %s!",
                    len(tags),
                    address_list,
                )
        return tag

    def remove_tag(self, esl_id=None, group_id=None, tag: Tag = None):
        """Remove tag from lists"""
        tags_to_remove = []
        if tag is not None:
            tags_to_remove.append(tag)
        elif esl_id is not None:
            node_id = esl_id, group_id
            if esl_id == BROADCAST_ADDRESS:
                tags_to_remove = self.tag_db.list_group(group_id)
            else:
                tag = self.tag_db.find(node_id)
                tags_to_remove.append(tag)
        if len(tags_to_remove) == 0:
            self.log.error("Unable to remove tag: no matching tag found")
            return
        for tag in tags_to_remove:
            if tag.blocked in [
                elw.ESL_LIB_STATUS_UNSPECIFIED_ERROR,
                elw.ESL_LIB_STATUS_UNASSOCIATED,
            ]:  # keep tags with blocked status - manual connection can override blocking, later!
                self.log.info(
                    "Delete bonding for blocked tag at address %s", tag.ble_address
                )
            # Provide protection for exclusive network configured tags
            elif (
                self.exclusive_network.enabled
                and self.exclusive_network.contains_address(tag.ble_address)
            ):
                self.log.warning(
                    "Tag at %s is protected by exclusive network configuration and will not be removed.",
                    tag.ble_address,
                )
            else:
                self.log.info("Tag removed at address %s", tag.ble_address)
                self.tag_db.remove(tag)
            self.key_db.delete_node(tag.ble_address)
            tag.unassociate()

    def get_esl_address(self, addr, group_id):
        """Return ESL ID from either BT address or ESL ID string - can return with BROADCAST_ADDRESS value (0xFF)!"""
        tag = None
        esl_id = None
        if group_id is None:
            group_id = 0  # defaulting group 0 if none is given
        if addr == "all" or addr == BROADCAST_ADDRESS:
            esl_id = (
                BROADCAST_ADDRESS  # convert user friendly keyword to broadcast address
            )
            if len(self.tag_db.list_group(group_id)):
                self.log.warning("All tags in group %d will be addressed!", group_id)
            else:
                self.log.warning(
                    "Group %d is empty, the command will have no effect!", group_id
                )
        elif re.fullmatch(VALID_BD_ADDRESS_REGEX, str(addr)) is not None:
            tag = self.tag_db.find(addr)
        elif addr is not None:
            try:
                esl_id = int(addr)
            except TypeError:
                self.log.error("%d is not a valid address, request ignored!", str(addr))
            else:
                tag = self.tag_db.find((esl_id, group_id))

        if tag is None:
            if esl_id is not None:
                if esl_id != BROADCAST_ADDRESS:
                    self.log.warning(
                        "Tag address unknown: %s in group %d", addr, group_id
                    )
                    if self.controller_command != None:
                        self.notify_controller(self.controller_command, CONTROLLER_COMMAND_FAIL)
                    esl_id = None
            else:
                self.log.warning("Tag address unknown: %s", addr)
        else:
            esl_id = tag.esl_id
            group_id = tag.group_id
        if self.pawr_active is None: # emit warning only if PAwR is not running at all and not even requested
            self.log.error(
                "PAwR is not running: command request is queued and won't be emitted until PAwR is active."
            )
        return esl_id, group_id

    def multi_connection_stats(self):
        """Gather some useful stats"""
        tag_list = self.tag_db.all()
        synced_count = len(self.tag_db.list_esl_state(EslState.SYNCHRONIZED))
        connected_count = len(self.tag_db.list_state(TagState.CONNECTED))
        connecting_count = len(self.tag_db.list_state(TagState.CONNECTING))
        blocked_count = len([tag for tag in tag_list if tag.blocked])
        total_count = len(tag_list)
        # Avoid repeated printings
        stat_sum = int(
            "%i%i%i%i%i"
            % (
                total_count,
                connecting_count,
                connected_count,
                blocked_count,
                synced_count,
            )
        )
        if stat_sum != self.stat_sum:
            self.log.info(
                "Multiconnection stats - TOTAL: %d, CONNECTING: %d, CONNECTED: %d, SYNCED: %d, BLOCKED: %d",
                total_count,
                connecting_count,
                connected_count,
                synced_count,
                blocked_count,
            )
            self.stat_sum = stat_sum

    def check_address_list(self, target=None):  # no specific tag by default
        """Check address list, or try connect to particular tag"""
        num_connecting = len(self.tag_db.list_state(TagState.CONNECTING))
        if (
            self.pawr_active
            and self.bonding_finished
            and num_connecting < ESL_CMD_MAX_PENDING_CONNECTION_REQUEST_COUNT
        ):
            if target is None or num_connecting:
                self.log.info("Checking for next advertising ESL")
                # Advertising IDLE state tags those are not blocked
                tag_list = [
                    tag
                    for tag in self.tag_db.list_state(TagState.IDLE)
                    if not tag.blocked and tag.advertising
                ]
            else:
                self.log.info(
                    "Initiate connection to ESL at %s address.", target.ble_address
                )
                tag_list = [target]
            if len(tag_list) > 0:
                if not self.max_conn_count_reached:
                    tag = tag_list[
                        random.randint(0, len(tag_list) - 1)
                    ]  # chosing randomly helps to cope with netwokrs that include erroneous tags
                    self.bonding_finished = (
                        True
                        if self.lib_connection_mode
                        != elw.ESL_LIB_CONNECTION_MODE_SINGLE
                        else False
                    )
                    self.connect(tag)
                    if self.auto_config_start_time is None:
                        self.auto_config_start_time = dt.now()
                elif not num_connecting:
                    self.log.info(
                        "Access point connection limit has been reached - auto provisioning is still suspended."
                    )
            else:
                if (
                    len(
                        self.tag_db.list_state(
                            (TagState.CONNECTING, TagState.CONNECTED)
                        )
                    )
                    == 0
                ):
                    if not self.scan_runs:
                        self.log.error(
                            "Scanning is disabled, auto commissioning stopped until scanning is enabled!"
                        )
                    else:
                        self.log.warning(
                            "No advertising ESL found within RSSI threshold of %d dBm, auto commissioning suspended until further detection!",
                            self.rssi_threshold,
                        )
                    self.log.debug("TagDB deep size: %d bytes", deep_size(self.tag_db))
                    log("Auto mode summary:", _half_indent_log=True)
                    time = None
                    try:
                        time = (
                            dt.now() - self.auto_config_start_time
                        )  # self.auto_config_start_time can be None in an edge case
                        time_per_tag = (
                            time.total_seconds()
                            / self.auto_configured_tags_in_single_run
                        )
                        log(
                            f"Last auto config session for {self.auto_configured_tags_in_single_run} "
                            f"{'tags' if self.auto_configured_tags_in_single_run > 1 else 'tag'} took "
                            f"a total time of {str(time)[:-3]}, which is {time_per_tag:.3f} seconds per ESL."
                        )
                    except:
                        # since self.auto_configured_tags_in_single_run will always count brand new configurations, only: it can be zero, resulting in div by 0 exception
                        if (
                            time is not None
                            and (self.auto_configured_tags_in_single_run != 0 or len(self.tag_db.list_esl_state(EslState.SYNCHRONIZED)) != 0)
                        ):  # if self.auto_config_start_time was not None, then time must be valid, at least
                            log(
                                f"Last auto re-config session took a total time of {str(time.total_seconds())[:-3]}."
                            )  # in this case we don't know how many tags has been re-configured
                    self.auto_config_start_time = None
                    self.auto_configured_tags_in_single_run = 0
                    self.ap_list(["synchronized", "unsynchronized", "blocked"], verbosity_level=False)
                if not self.scan_runs and not self.cmd_mode:
                    self.log.warning(
                        "Scanning is disabled in auto mode - please consider enabling it by issuing 'scan start' command!"
                    )
                return
            # Print stats until there are devices around advertising
            self.multi_connection_stats()

    def notify_controller(self, command_id, result, *args):
        """Send notification to the connected controller (used by the SiConnect ESL Demo app)"""
        list_to_send = [command_id, result]
        b_arr = bytes()
        if args:
            for x in args:
                if (type(x) == bytes) or (type(x) == bytearray):
                    b_arr += x
                elif type(x) == str:
                    b_arr += bytes(x, "utf-8")
                elif x == None:
                    # Tag response can be None
                    pass
                else:
                    list_to_send.append(x)
        bytearray_to_send = bytes(list_to_send)
        bytearray_to_send += b_arr
        if command_id == CCMD_REQUEST_DATA:
            self.lib.general_command(CMD_AP_CONTROL_IT_RESPONSE, bytearray_to_send[2:])
            self.controller_command = (
                CCMD_REQUEST_DATA  # request more until the end of the image file
            )
        else:
            self.lib.general_command(CMD_AP_CONTROL_CP_RESPONSE, bytearray_to_send)
            self.controller_command = None
