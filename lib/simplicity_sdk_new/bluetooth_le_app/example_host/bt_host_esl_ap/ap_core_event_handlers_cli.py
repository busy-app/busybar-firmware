"""
ESL AP Core - CLI mode event handler extensions.
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

import esl_lib
import esl_lib_wrapper as elw
from esl_tag import TagState

class CLIEventHandlersMixin:
    """CLI mode specific event handler extensions"""

    def cli_esl_event_error(self, evt: esl_lib.EventError):
        """ESL library error event handler extension for CLI mode"""
        if evt.lib_status in [
            elw.ESL_LIB_STATUS_CONN_FAILED,
            elw.ESL_LIB_STATUS_CONN_CLOSE_FAILED,
            elw.ESL_LIB_STATUS_CONN_TIMEOUT,
        ]:
            if evt.lib_status == elw.ESL_LIB_STATUS_CONN_FAILED:
                if (
                    evt.sl_status == elw.SL_STATUS_BT_CTRL_AUTHENTICATION_FAILURE
                    or evt.data == elw.ESL_LIB_CONNECTION_STATE_NEW_BOND_REQUIRED
                ):  # handle advertisers bonded to different AP
                    self.log.info(
                        "Tag at address %s refused connection attempts - seemingly bonded to other AP",
                        evt.node_id,
                    )
            elif evt.lib_status == elw.ESL_LIB_STATUS_CONN_TIMEOUT:
                if evt.data == elw.ESL_LIB_CONNECTION_STATE_CONNECTING:
                    self.log.error(
                        "Timeout occured on connection attempt to address %s",
                        evt.node_id,
                    )
                if evt.data == elw.ESL_LIB_CONNECTION_STATE_WRITE_CONTROL_POINT:
                    # The Tag object already cleared its connection handle until we reach to this point
                    self.log.error(
                        "Timeout occured after writing ESL control point of deleted connection handle %s",
                        evt.node_id,
                    )
            self.revert_auto_mode()

    def cli_esl_event_system_boot(self, evt: esl_lib.EventSystemBoot):
        """ESL library boot event handler extension for CLI mode"""
        self.log.info("Command line event handler started, system is idle.")
        self.log.info("Type 'help' for available commands.")

    def cli_esl_event_tag_found(self, evt: esl_lib.EventTagFound):
        """ESL library tag found event handler extension for CLI mode"""
        tag = self.tag_db.find(evt.address)
        if tag is None:
            return  # Happens until the RSSI threshold is met
        if (
            not self.pawr_active # PAwR is not running (yet)
            and not tag.advertising
            and not tag.state == TagState.CONNECTING
        ):
            self.log.warning(
                "ESL tag can't be synchronized because PAwR is not running."
            )
            self.log.info(
                "Don't forget to start PAwR with 'sync start' before completing configuration!"
            )

    def cli_esl_event_connection_closed(self, evt: esl_lib.EventConnectionClosed):
        """ESL library connection closed event handler extension for CLI mode"""
        self.revert_auto_mode()
