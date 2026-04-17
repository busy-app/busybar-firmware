"""
ESL AP Core - auto mode event handler extensions.
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

from ap_config import (
    ESL_CMD_MAX_PENDING_CONNECTION_REQUEST_COUNT,
    IMAGE_MAX_AUTO_UPLOAD_COUNT,
    INITIAL_AUTO_ADVERTISE_PAWR_TRAIN,
)
from ap_constants import TLV_OPCODE_UPDATE_COMPLETE
import esl_lib
import esl_lib_wrapper as elw
from esl_tag import TagState

class AutoEventHandlersMixin:
    """Auto mode specific event handler extensions"""

    def auto_esl_event_system_boot(self, evt: esl_lib.EventSystemBoot):
        """ESL library boot event handler extension for auto (a.k.a ESL Profile) mode"""
        # Set auto connection mode
        self.log.info(
            "Booted in AUTO mode, request library Initiator Filter Policy enable."
        )
        self.lib.set_connection_mode(elw.ESL_LIB_CONNECTION_MODE_LIST)
        self.skip_mode_evt_after_boot = True
        self.start_scan()
        self.start_pawr_train(advertise=INITIAL_AUTO_ADVERTISE_PAWR_TRAIN)

    def auto_esl_event_connection_mode(self, evt: esl_lib.EventConnectionMode):
        """ESL library connection mode event handler extension for auto mode"""
        if (
            evt.mode == elw.ESL_LIB_CONNECTION_MODE_SINGLE
            and not self.skip_mode_evt_after_boot
        ):
            self.log.warning(
                "Library Initiator Filter Policy disabled in auto mode! This may affect overall system performance. Try reissuing the 'mode auto' command while the NCP is idle."
            )
        self.skip_mode_evt_after_boot = False

    def auto_esl_event_tag_found(self, evt: esl_lib.EventTagFound):
        """ESL library tag found event handler extension for auto mode"""
        tag = self.tag_db.find(evt.address)
        if tag is not None and tag.state == TagState.IDLE:
            if tag.advertising:
                if self.pawr_active and not self.max_conn_count_reached and not tag.blocked:
                    self.check_address_list(tag)
            elif not self.pawr_active: # PAwR is not running (yet) if None or False
                self.log.error(
                    "ESL tag cannot be synchronized because PAwR is not started!"
                )
                self.log.info(
                    "Please re-start auto mode with command: 'mode auto' to recover."
                )

    def auto_esl_event_error(self, evt: esl_lib.EventError):
        """ESL library error event handler extension for auto mode"""
        tag = self.tag_db.find(evt.node_id)
        if evt.lib_status in [
            elw.ESL_LIB_STATUS_OTS_ERROR,
            elw.ESL_LIB_STATUS_OTS_TRANSFER_FAILED,
            elw.ESL_LIB_STATUS_OTS_GOTO_FAILED,
            elw.ESL_LIB_STATUS_OTS_UNEXPECTED_OFFSET,
            elw.ESL_LIB_STATUS_OTS_WRITE_RESP_FAILED,
        ]:
            if evt.sl_status not in [
                elw.SL_STATUS_TIMEOUT,
                elw.SL_STATUS_BT_CTRL_CONNECTION_REJECTED_DUE_TO_NO_SUITABLE_CHANNEL_FOUND,
            ]:
                try:
                    self.upload_next_image(tag)
                    return
                except:
                    pass
        elif evt.lib_status == elw.ESL_LIB_STATUS_OTS_GOTO_FAILED:
            if evt.sl_status == elw.SL_STATUS_NOT_FOUND:
                self.upload_next_image(tag)
        elif evt.lib_status in [
            elw.ESL_LIB_STATUS_CONN_FAILED,
            elw.ESL_LIB_STATUS_CONN_CLOSE_FAILED,
            elw.ESL_LIB_STATUS_CONN_TIMEOUT,
        ]:
            self.auto_override = False
            self.set_mode_handlers()
            if evt.sl_status not in [
                elw.SL_STATUS_NO_MORE_RESOURCE,
                elw.SL_STATUS_BT_CTRL_CONNECTION_LIMIT_EXCEEDED,
                elw.SL_STATUS_BT_CTRL_UNKNOWN_CONNECTION_IDENTIFIER,
            ]:
                self.check_address_list()

    def auto_esl_event_connection_opened(self, evt: esl_lib.EventConnectionOpened):
        """ESL library connection opened event handler extension for auto mode"""
        tag = self.tag_db.find(evt.address)

        if self.auto_override:
            self.log.warning("AUTO MODE TEMPORARILY CHANGED TO MANUAL!")
            self.cmd_mode = self.auto_override
            self.set_mode_handlers()
        elif tag is not None:
            if evt.status != elw.SL_STATUS_OK:
                self.disconnect(
                    tag
                )  # auto mode can't do anything with a Tag that is connected with failure
            elif (
                tag.provisioned
            ):  # we remain in auto mode, so aviod stuck connected in special case below
                if (
                    tag.max_image_index is not None
                    and tag.has_image_transfer
                    and IMAGE_MAX_AUTO_UPLOAD_COUNT
                    and tag.auto_image_count
                    < min((tag.max_image_index + 1), IMAGE_MAX_AUTO_UPLOAD_COUNT)
                ):
                    self.upload_auto_image(
                        (tag.auto_image_count % len(self.image_files)), tag
                    )
                else:
                    self.ap_update_complete(tag.esl_id, tag.group_id)
            elif not tag.associated:
                self.auto_configured_tags_in_single_run += 1

    def auto_esl_event_tag_info(self, evt: esl_lib.EventTagInfo):
        """ESL library tag info received event handler extension for auto mode"""
        if not self.auto_override:
            tag = self.tag_db.find(evt.connection_handle)
            if (
                not tag.was_provisioned
            ):  # auto-refreshing of an existing tag config may happened already within the tag event handling
                values = self.configure(tag)
                self.write_values(tag, values)

    def auto_esl_event_control_point_response(
        self, evt: esl_lib.EventControlPointResponse
    ):
        """ESL library ESL control point response event handler extension for auto mode"""
        if evt.status == elw.SL_STATUS_OK:
            tag = self.tag_db.find(evt.connection_handle)
            if (
                evt.data_sent[1] == tag.esl_id
                and not evt.data_sent[0] == TLV_OPCODE_UPDATE_COMPLETE
            ):
                self.ap_update_complete(tag.esl_id, tag.group_id)

    def auto_esl_event_configure_tag_response(
        self, evt: esl_lib.EventConfigureTagResponse
    ):
        """ESL library configure tag response event handler extension for auto mode"""
        if evt.status == elw.SL_STATUS_OK:
            tag = self.tag_db.find(evt.connection_handle)
            if tag is not None and tag.provisioned:
                if (
                    tag.max_image_index is not None
                    and tag.has_image_transfer
                    and IMAGE_MAX_AUTO_UPLOAD_COUNT
                    and tag.auto_image_count
                    < min((tag.max_image_index + 1), IMAGE_MAX_AUTO_UPLOAD_COUNT)
                ):
                    self.upload_auto_image(
                        (tag.auto_image_count % len(self.image_files)), tag
                    )
                else:
                    self.ap_update_complete(tag.esl_id, tag.group_id)

    def auto_esl_event_image_transfer_finished(
        self, evt: esl_lib.EventImageTransferFinished
    ):
        """ESL library OTS image transfer finished event handler extension for auto mode"""
        self.upload_next_image(self.tag_db.find(evt.connection_handle))

    def auto_esl_event_connection_closed(self, evt: esl_lib.EventConnectionClosed):
        """ESL library connection closed event handler extension for auto mode"""
        if self.auto_override:
            self.cmd_mode = False
            self.auto_override = False
        if self.bonding_finished:
            self.check_address_list()

    def auto_esl_event_bonding_finished(self, evt: esl_lib.EventBondingData):
        """ESL library bonding data ready event handler extension for auto mode"""
        initiator_limit = (
            ESL_CMD_MAX_PENDING_CONNECTION_REQUEST_COUNT
            if self.lib_connection_mode == elw.ESL_LIB_CONNECTION_MODE_LIST
            else 1
        ) # Normally, there can only be one, according to the Core spec.
        if (
            not self.max_conn_count_reached
            and len(self.tag_db.list_state(TagState.CONNECTING)) < initiator_limit
        ):
            self.check_address_list()
        else:
            self.multi_connection_stats()
