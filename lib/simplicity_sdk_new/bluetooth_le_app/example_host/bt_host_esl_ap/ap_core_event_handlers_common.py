"""
ESL AP Core - common event handlers mixin.
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
    TLV_OPCODE_UPDATE_COMPLETE,
    TLV_RESPONSE_BASIC_STATE,
)
from ap_logger import log, logLevel, LEVELS
from ap_response_parser import ResponseParser
from esl_tag import TagState, EslState
import esl_lib
import esl_lib_wrapper as elw
import esl_key_lib
from ap_ead import KeyMaterial
from ap_core_utils import skip_if_stopped


class CommonEventHandlersMixin:
    """Common (no-prefix) ESL event handlers shared by all modes"""

    @skip_if_stopped
    def esl_event_system_boot(self, evt: esl_lib.EventSystemBoot):
        """Generic handler for the boot event of the ESL library"""
        self.scan_runs = False
        self.pawr_active = None
        self.pawr_advertising = False
        self.pawr_handle = None
        self.pawr_restart = None
        self.shutdown_timer.cancel()
        self.max_conn_count_reached = False
        self.bonding_finished = True
        self.demo_controller_connected = False
        self.ncp_address = evt.address

        for tag in self.tag_db.all():
            if (
                tag.state == TagState.CONNECTING
                or tag.esl_state == EslState.CONFIGURING
            ):
                self.key_db.delete_ltk(tag.ble_address)
            tag.reset(keep_esl_address=self.exclusive_network.contains_address(tag.ble_address))

    def esl_event_tag_found(self, evt: esl_lib.EventTagFound):
        """Generic handler for the tag found (a.k.a ESL scan report) event of the ESL library"""
        if evt.rssi > self.rssi_threshold:
            tag = self.tag_db.find(evt.address)
            if tag is None:
                tag = self.tag_db.add(self.lib, evt.address)
                if self.exclusive_network.enabled:
                    tag.block(elw.ESL_LIB_STATUS_DATABASE_PERMISSION)
                    self.log.warning(
                        "Ignore ESL at %s due to exclusive network mode.",
                        tag.ble_address,
                    )
                # only need this once on creating the tag, later on tag will handle on its own!
                tag.handle_event(evt)

    def esl_event_connection_mode(self, evt: esl_lib.EventConnectionMode):
        """Generic handler for the connection mode event of the ESL library"""
        self.lib_connection_mode = evt.mode

    def esl_event_connection_opened(self, evt: esl_lib.EventConnectionOpened):
        """Generic handler for the connection opened event of the ESL library"""
        tag = self.tag_db.find(evt.address)
        if tag and tag.provisioned:
            removed = self.remove_esl_pawr_requests(tag)
            if removed:
                self.log.warning(
                    "Removed %d pending PAwR command(s) for ESL ID %d in group %d.",
                    len(removed),
                    tag.esl_id,
                    tag.group_id,
                )

    def esl_event_bonding_finished(self, evt: esl_lib.EventBondingData):
        """Generic handler for the bonding data event of the ESL library"""
        self.bonding_finished = True

    def esl_event_tag_info(self, evt: esl_lib.EventTagInfo):
        """Generic handler for the tag info arrival event of the ESL library"""
        pass

    def esl_event_pawr_config(self, evt: esl_lib.EventPawrConfig):
        """Generic handler for the PAwR configuration report event of the ESL library"""
        if not evt.configured:
            self.log.error(
                "Periodic Advertisement with Responses configuration failed!"
            )
            # Synchronize to the last known good sync parameters of the ESL C library with those the AP's
            self.adv_interval_min = evt.config.adv_interval.min
            self.adv_interval_max = evt.config.adv_interval.max
            self.subevent_count = evt.config.subevent.count
            self.subevent_interval = evt.config.subevent.interval
            self.response_slot_delay = evt.config.response_slot.delay
            self.response_slot_spacing = evt.config.response_slot.spacing
            self.response_slot_count = evt.config.response_slot.count
        else:
            self.log.info(
                "Periodic Advertisement with Responses configured successfully."
            )
            if self.pawr_active and self.pawr_restart is None:
                self.log.warning(
                    "Note that the new PAwR configuration will not take effect until the sync has been restarted!"
                )

    def esl_event_pawr_status(self, evt: esl_lib.EventPawrStatus):
        """Generic handler for the PAwR status report event of the ESL library"""
        if evt.status == elw.ESL_LIB_PAWR_STATE_RUNNING:
            self.log.info("PAwR sync train started.")
            self.pawr_advertising = False
            self.pawr_active = True
        elif evt.status == elw.ESL_LIB_PAWR_STATE_RUNNING_ADVERTISING:
            self.log.info("PAwR parameters extended advertisement running.")
            self.pawr_advertising = True
            self.pawr_active = True
        else:
            self.pawr_active = None
            if self.pawr_restart is not None:
                self.log.info("PAwR train re-start requested.")
                self.start_pawr_train(self.pawr_restart, self.pawr_advertising)
                self.pawr_restart = None
            else:
                self.log.info("Periodic Advertisement with Responses stopped.")
            self.pawr_advertising = False

    def esl_event_scan_status(self, evt: esl_lib.EventScanStatus):
        """Generic handler for the scan status report event of the ESL library"""
        if evt.enabled:
            self.log.info("Scanning started.")
        else:
            self.log.info("Scanning stopped.")
            self.scan_runs = False
            for tag in self.tag_db.list_advertising():
                tag.reset_advertising()
                if self.cmd_mode and not tag.associated:
                    self.tag_db.remove(tag)

    def esl_event_configure_tag_response(self, evt: esl_lib.EventConfigureTagResponse):
        """Generic handler for the configure tag response event of the ESL library"""
        if evt.status == elw.SL_STATUS_OK:
            tag = self.tag_db.find(evt.connection_handle)
            if tag.provisioned:
                esl_db_rec = esl_key_lib.ESLRecord(
                    tag.ble_address,
                    rk=tag.response_key,
                    esl_id=tag.esl_id,
                    group=tag.group_id,
                )
                raised = True
                try:
                    self.key_db.add_esl(esl_db_rec)
                    raised = False
                except ValueError as ve:
                    self.log.error("Database error! %s", ve)
                    tag.close_connection()
                    tag.block(elw.ESL_LIB_STATUS_DATABASE_ERROR)
                    tag.unassociate()
                except esl_key_lib.DBRecordError as dbe:
                    self.log.error("Database conflict! %s", dbe)
                    # The following sequence of instructions ensures that the label is "forgotten" -
                    # so that it can be reconfigured later by assigning a new auto ESL ID and group.
                    tag.close_connection()
                    tag.unassociate()
                    self.tag_db.remove(tag) # there's nothing else we could do with conflicting tags
                finally:
                    if raised:
                        # Adjust the config counter if needed (only in auto mode)
                        if (
                            self.auto_configured_tags_in_single_run != 0
                            and not self.cmd_mode
                        ):
                            self.auto_configured_tags_in_single_run -= 1
                        return

    def esl_event_control_point_response(self, evt: esl_lib.EventControlPointResponse):
        """Generic handler for the ESL control point response event of the ESL library"""
        tag = self.tag_db.find(evt.connection_handle)
        if (
            evt.data_sent[0] == TLV_OPCODE_UPDATE_COMPLETE
            and tag is not None
            and evt.data_sent[1] == tag.esl_id
        ):
            self.past(tag)

    def esl_event_connection_closed(self, evt: esl_lib.EventConnectionClosed):
        """Generic handler for the connection closed event of the ESL library"""
        self.max_conn_count_reached = False
        tag = self.tag_db.find(
            evt.address
        )  # Can return None after unassociate command written successfully to ESL Control Point if connected tag
        if tag is not None:
            self.ltk_conditional_removal(tag, evt.reason)
            if (
                tag.provisioned
                and not tag.advertising
                and evt.reason == elw.SL_STATUS_BT_CTRL_CONNECTION_TIMEOUT
            ):
                # Handle a special edge case in which the synced flag is not set after "successfully failed" disconnection
                # i.e. the very last LL handshake can be lost due to radio noise -> sl_status is reported as 0x1008 yet the connection is in fact closed
                self.log.debug(
                    "Check if ESL ID %d in group %d at address %s got synchronized despite the reported hypervisor timeout.",
                    tag.esl_id,
                    tag.group_id,
                    tag.ble_address,
                )
                self.ap_ping(tag.esl_id, tag.group_id)
            if self.cmd_mode or (logLevel() <= LEVELS["DEBUG"] and tag.associated):
                log("Tag info about disconnected device:", _half_indent_log=True)
                log(tag.get_info())

    def esl_event_control_point_notification(
        self, evt: esl_lib.EventControlPointNotification
    ):
        """Generic handler for the ESL control point notification event of the ESL library"""
        # after an unassociate all command sent the evt.address will not be part of the list of tags!
        tag = self.tag_db.find(evt.connection_handle)
        if tag is None:
            return
        ResponseParser(evt.data, tag.sensor_info)
        if evt.data[0] == TLV_RESPONSE_BASIC_STATE and tag.pending_unassociate:
            tag.block(
                elw.ESL_LIB_STATUS_UNASSOCIATED
            )  # blocking before calling remove_tag() will preserve the tag object in memory, but still remove its bonding database record!
            self.remove_tag(tag=tag)

    def esl_event_pawr_response(self, evt: esl_lib.EventPawrResponse):
        """Generic handler for the PAwR response event of the ESL library"""
        # get all pending in this particular subevent + slot
        _, cmd_list = self.find_all_pending_commands(evt.response_slot, evt.subevent)

        # Identify the exact responder (pending list may include commands for future responses of different tags)
        tag = self.identify_responder_tag(evt.response_slot, evt.subevent)

        # Case 1: No pending command -> no response expected (implausible -> error level!)
        if not cmd_list:
            if evt.data:
                self.log.error(
                    "PAwR response received but no matching command pending: 0x%s",
                    evt.data.hex(),
                )
        # Case 2: there is a non-empty list of pending commands, but the responder can't be identified (unlikely ->error)
        elif tag is None:
            if evt.data:
                self.log.error(
                    "PAwR response received but the responder cannot be identified for command: 0x%s. Not resending.",
                    evt.data.hex(),
                )
                # Remove all pending commands for this group because the responder could not be identified
                group_id = cmd_list[0][0].group_id
                indices = [idx for (_, idx) in cmd_list]
                self.remove_pending_commands(group_id, indices)
        # Case 3: Pending command exists, responder is identified but no data received (possible) -> retry
        elif not evt.data:
            # Filter the command list to only those belonging to this ESL and retry
            self.retry_pending_commands([(cmd, idx) for (cmd, idx) in cmd_list if cmd.esl_id == tag.esl_id])
        # Case 4: Data received -> decrypt and process
        else:
            response = self.ead.decrypt(evt.data, KeyMaterial(tag.response_key))
            if not response:
                self.log.warning(
                    "PAwR response decryption failed for ESL ID %d in group %d",
                    tag.esl_id,
                    tag.group_id,
                )
            else:
                self.handle_pawr_response(
                    response,
                    evt.response_slot,
                    evt.subevent,
                    tag,
                    cmd_list,
                )

    def esl_event_pawr_data_request(self, evt: esl_lib.EventPawrDataRequest):
        """Generic handler for the PAwR data request event of the ESL library"""
        subevents = list(
            range(evt.subevent_start, evt.subevent_start + evt.subevent_data_count)
        )
        self.send_pawr_commands([s % self.subevent_count for s in subevents])
        self.synchronization_handler()

    def esl_event_error(self, evt: esl_lib.EventError):
        """Generic handler for the various error events of the ESL library"""
        if evt.lib_status == elw.ESL_LIB_STATUS_BONDING_FAILED:
            if evt.sl_status in [
                elw.SL_STATUS_BT_CTRL_PIN_OR_KEY_MISSING,
                elw.SL_STATUS_BT_SMP_PAIRING_NOT_SUPPORTED,
            ]:
                tag = self.tag_db.find(evt.node_id)
                self.bonding_finished = True
                if tag is not None:
                    self.key_db.delete_ltk(tag.ble_address, self.ncp_address)
        elif evt.lib_status == elw.ESL_LIB_STATUS_CONN_TIMEOUT or (
            evt.lib_status == elw.ESL_LIB_STATUS_CONN_FAILED
            and evt.sl_status
            == elw.SL_STATUS_BT_CTRL_CONNECTION_ACCEPT_TIMEOUT_EXCEEDED
        ):
            self.bonding_finished = True
            tag = self.tag_db.find(evt.node_id)
            if (
                tag is not None
                and not tag.associated
                and not tag.blocked
                and evt.data == elw.ESL_LIB_CONNECTION_STATE_CONNECTING
            ):
                self.remove_tag(
                    tag=tag
                )  # keeps the tag database safe from orphaned objects which have shown no sign of existing
            if (
                evt.sl_status == elw.SL_STATUS_TIMEOUT
                and evt.data == elw.ESL_LIB_CONNECTION_STATE_PAST_CLOSE_CONNECTION
            ):
                self.check_sync(tag)

        elif evt.lib_status == elw.ESL_LIB_STATUS_CONN_FAILED:
            self.bonding_finished = True
            tag = self.tag_db.find(evt.node_id)
            if tag is not None and not tag.provisioned:
                if (
                    evt.sl_status == elw.SL_STATUS_ABORT
                    or evt.sl_status == elw.SL_STATUS_BT_CTRL_AUTHENTICATION_FAILURE
                ):  # handle advertisers that refuse connection retry attempts - e.g. because bonded to other AP
                    if not tag.blocked:
                        self.log.error(
                            "Tag at address %s has been blocked due to unsuccessful connection attempt(s).",
                            evt.node_id,
                        )
                        tag.block(elw.ESL_LIB_STATUS_BONDING_FAILED)
                elif (
                    evt.sl_status
                    == elw.SL_STATUS_BT_CTRL_CONNECTION_TERMINATED_BY_LOCAL_HOST
                    and evt.data == elw.ESL_LIB_CONNECTION_STATE_SERVICE_DISCOVERY
                ):
                    if tag.blocked:
                        self.key_db.delete_node(
                            tag.ble_address
                        )  # remove ESLs which are violating the spec (that is, which are lack of any mandatory GATT entries)
                        self.log.debug(
                            "Database entry for tag at address %s deleted due to ESL Profile/Service violation.",
                            tag.ble_address,
                        )
                elif evt.sl_status in [
                    elw.SL_STATUS_BT_CTRL_CONNECTION_TERMINATED_BY_LOCAL_HOST,
                    elw.SL_STATUS_BT_CTRL_CONNECTION_TIMEOUT,
                ] and evt.data in [
                    elw.ESL_LIB_CONNECTION_STATE_OTS_INIT,
                    elw.ESL_LIB_CONNECTION_STATE_DIS_DISCOVERY,
                    elw.ESL_LIB_CONNECTION_STATE_ESL_DISCOVERY,
                    elw.ESL_LIB_CONNECTION_STATE_ESL_SUBSCRIBE,
                    elw.ESL_LIB_CONNECTION_STATE_GET_TAG_INFO,
                ]:
                    self.ltk_conditional_removal(tag, evt.sl_status)
            if (
                evt.sl_status
                in [
                    elw.SL_STATUS_NO_MORE_RESOURCE,
                    elw.SL_STATUS_BT_CTRL_CONNECTION_LIMIT_EXCEEDED,
                ]
                and not self.max_conn_count_reached
            ):
                self.max_conn_count_reached = True
                self.log.warning(
                    "Access point connection limit reached - suspend connect requests until a connection is closed."
                )
        elif evt.lib_status == elw.ESL_LIB_STATUS_GATT_TIMEOUT:
            tag = self.tag_db.find(evt.node_id)
            if tag is not None:
                if not tag.provisioned:
                    self.key_db.delete_ltk(tag.ble_address)
                self.log.error("GATT Timeout for address %s", tag.ble_address)
        elif evt.lib_status == elw.ESL_LIB_STATUS_OTS_GOTO_FAILED:
            tag = self.tag_db.find(evt.node_id)
            if tag is not None:
                if evt.sl_status == elw.SL_STATUS_NOT_FOUND:
                    self.log.error(
                        "No object found with the requested Object ID for address %s",
                        tag.ble_address,
                    )
        elif evt.lib_status == elw.ESL_LIB_STATUS_PAWR_START_FAILED or (evt.lib_status == elw.ESL_LIB_STATUS_PAWR_CONFIG_FAILED and evt.data == elw.ESL_LIB_PAWR_STATE_IDLE):
            self.pawr_active = None
        elif evt.lib_status == elw.ESL_LIB_STATUS_PAST_INIT_FAILED:
            if evt.sl_status == elw.SL_STATUS_BT_CTRL_COMMAND_DISALLOWED:
                self.log.info("PAST skipped by ESL already in Synchronized state.")
        elif evt.lib_status == elw.ESL_LIB_STATUS_CONN_CLOSE_FAILED:
            self.bonding_finished = True
            if (
                evt.sl_status == elw.SL_STATUS_TIMEOUT
                and evt.data == elw.ESL_LIB_CONNECTION_STATE_PAST_CLOSE_CONNECTION
            ):
                self.check_sync(self.tag_db.find(evt.node_id))
        elif evt.lib_status == elw.ESL_LIB_STATUS_CONN_ESL_SERVICE_VIOLATION:
            tag = self.tag_db.find(evt.node_id)
            if tag is not None and not tag.blocked:
                self.log.error(
                    "Device at address %s has been blocked due to missing mandatory service!",
                    evt.node_id,
                )
                tag.block(evt.lib_status)
        elif evt.lib_status == elw.ESL_LIB_STATUS_PAWR_SET_DATA_FAILED:
            _, cmd_list = self.find_all_pending_commands(
                None, # slot is unknown in this error case - just match any pending data for this subevent will be OK
                evt.node_id.subevent
            )
            if not cmd_list:
                self.log.warning(
                    "PAWR_SET_DATA_FAILED event received but no pending commands for subevent %d",
                    evt.node_id.subevent,
                )
            else:
                count = len(cmd_list)
                self.log.warning(
                    "PAwR set data failed for subevent %d with status %s, retrying %d pending command%s",
                    evt.node_id.subevent,
                    esl_lib.get_sl_status_str(evt.sl_status),
                    count,
                    "" if count == 1 else "s",
                )
                self.retry_pending_commands(cmd_list)

    def esl_event_image_type(self, evt: esl_lib.EventImageType):
        """Generic handler for the image type received event of the ESL library"""
        # Cache image type
        tag = self.tag_db.find(evt.connection_handle)
        self.ap_imageupdate(
            evt.img_index,
            tag.image_file,
            address=tag.ble_address,
            label=tag.label,
            rotation=tag.rotation,
        )

    def esl_event_bonding_data(self, evt: esl_lib.EventBondingData):
        """Generic handler for the (new) bonding data ready event of the ESL library"""
        # export new LTK
        esl_record = self.key_db.find_esl(evt.address)
        if esl_record is None:  # is it a brand new ESL not in the database yet?
            esl_record = esl_key_lib.ESLRecord(evt.address, ltk=evt.ltk)
        else:
            esl_record.ltk = evt.ltk
        esl_record.apk = self.ap_key
        esl_record.ap_address = self.ncp_address
        try:
            self.key_db.add_esl(esl_record)
        except Exception as e:
            self.log.error(e)
            tag = self.tag_db.find(evt.connection_handle) or self.tag_db.find(evt.address)
            if tag is not None:
                tag.close_connection(force_close=True)
