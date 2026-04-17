"""
ESL AP Core - PAwR responses mixin.
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
    AD_FRAME_OVERHEAD,
    CONTROLLER_COMMAND_SUCCESS,
    ERROR_RESPONSE_CAPACITY_LIMIT,
    ERROR_RESPONSE_QUEUE_FULL,
    ERROR_RESPONSE_RETRY,
    ESL_AD_TYPE,
    TLV_RESPONSE_BASIC_STATE,
    TLV_RESPONSE_ERROR,
)
from ap_config import ESL_CMD_MAX_RETRY_COUNT
from ap_response_parser import ResponseParser
import esl_lib_wrapper as elw
from datetime import datetime as dt

class PAWRResponsesMixin:
    """PAwR response identification and processing"""

    def identify_responder_tag(self, response_slot, subevent):
        """Identify responder Tag"""
        if subevent in self.esl_pending_commands:
            for cmd in self.esl_pending_commands[subevent]:
                # Response is in expected slot and subevent
                if cmd.slot_number == response_slot and cmd.group_id == subevent:
                    tag = self.tag_db.find((cmd.esl_id, cmd.group_id))
                    if tag is not None:
                        self.log.info(
                            "Reply in slot %d from ESL ID %d in group %d.",
                            response_slot,
                            tag.esl_id,
                            tag.group_id,
                        )
                    else:
                        self.log.warning(
                            "Unable to identify responder tag ID %d in group %d",
                            cmd.esl_id,
                            cmd.group_id,
                        )
                    return tag
        return None

    def handle_pawr_response(self, data, response_slot, subevent, tag, cmd_list):
        """
        Handle one or more TLV responses inside a PAwR response frame.
        Supports multiple pending commands for the same tag in the same subevent.
        """

        data_index = AD_FRAME_OVERHEAD  # Skip LL frame overhead
        sensor_info = None

        # Basic frame validation
        if (
            len(data) < AD_FRAME_OVERHEAD
            or len(data) != (int(data[0]) + 1)
            or data[1] != ESL_AD_TYPE
        ):
            self.log.warning("Received malformed PAwR response: %s, ignoring", data.hex())
            return

        # List of pending command indices that should be removed
        pop_indices = []

        while data_index < len(data):
            opcode = data[data_index]
            data_size = self.get_opcode_len(opcode)
            last_data = data_index + data_size

            if last_data > len(data):
                self.log.warning(
                    "Broken PAwR response chunk: %s in slot %d",
                    data[data_index:].hex(),
                    response_slot,
                )
                break

            response_data = data[data_index:last_data]

            # Find matching pending command for this TLV
            matched_cmd = None
            matched_idx = None

            for cmd, idx in cmd_list:
                # Skip commands already processed
                if idx in pop_indices:
                    continue

                if cmd.accepts_response(response_data[0]):
                    matched_cmd = cmd
                    matched_idx = idx
                    break

            if matched_cmd is not None:
                # Process response at tag level
                tag.handle_response(response_data)
                sensor_info = tag.sensor_info

                # Handle unassociate
                if response_data[0] == TLV_RESPONSE_BASIC_STATE and tag.pending_unassociate:
                    tag.block(elw.ESL_LIB_STATUS_UNASSOCIATED)
                    self.remove_tag(tag=tag)

                # Retry logic based on command lifetime
                if response_data[0] != TLV_RESPONSE_ERROR:
                    # Not an error response -> nothing to retry
                    pass
                elif response_data[1] not in (ERROR_RESPONSE_RETRY, ERROR_RESPONSE_CAPACITY_LIMIT):
                    # Error, but not retryable
                    self.log.warning(
                        "Non-retryable error response (0x%02X) from ESL ID %d in group %d.",
                        response_data[1],
                        tag.esl_id,
                        tag.group_id,
                    )
                    if response_data[1] == ERROR_RESPONSE_QUEUE_FULL:
                        self.log.warning(
                            "The device cannot accept the timed command 0x%s right now. "
                            "Try sending it again a bit later.",
                            matched_cmd.data.hex(),
                        )
                else:
                    # Retryable error
                    if matched_cmd.expired:
                        self.log.warning(
                            "Command (0x%s) for ESL ID %d in group %d reached max retry count. Not resending.",
                            matched_cmd.data.hex(),
                            tag.esl_id,
                            tag.group_id,
                        )
                    else:
                        matched_cmd.decay()
                        self.log.info(
                            "Retry command (0x%s) due to error response from ESL ID %d in group %d, remaining lifetime: %d",
                            matched_cmd.data.hex(),
                            tag.esl_id,
                            tag.group_id,
                            matched_cmd.lifetime,
                        )
                        self.queue_pawr_command(matched_cmd)

                # Mark this pending command for removal
                pop_indices.append(matched_idx)

            # Log and optional controller notification
            self.log.debug(
                "Parsing PAwR response: %s in slot %d",
                response_data.hex(),
                response_slot,
            )
            ResponseParser(response_data, sensor_info)

            if self.controller_command is not None:
                self.notify_controller(
                    self.controller_command,
                    CONTROLLER_COMMAND_SUCCESS,
                    tag.esl_id,
                    tag.group_id,
                    response_data,
                )

            data_index += data_size

        # Remove processed pending commands
        if pop_indices:
            self.remove_pending_commands(subevent, pop_indices)
