"""
ESL Command class.
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
from ap_constants import *
from ap_logger import getLogger
from datetime import datetime as dt


from dataclasses import dataclass, field
from typing import Optional
from datetime import datetime as dt
from logging import getLogger
from ap_config import ESL_CMD_MAX_RETRY_COUNT

@dataclass
class ESLCommand:
    """Class that encapsulates synchronization packet commands"""

    group_id: int
    data: bytes
    slot_number: Optional[int] = None
    timestamp: float = field(default_factory=lambda: dt.now().timestamp())
    response_opcode: list = field(default_factory=list)
    lifetime: int = field(default_factory=lambda: ESL_CMD_MAX_RETRY_COUNT)

    # Mapping opcode -> list of acceptable responses (by the ESL spec.)
    _RESPONSE_MAP = {
        TLV_OPCODE_PING: [TLV_RESPONSE_BASIC_STATE],
        TLV_OPCODE_UNASSOCIATE: [TLV_RESPONSE_BASIC_STATE, TLV_RESPONSE_ERROR],
        TLV_OPCODE_SERVICE_RST: [TLV_RESPONSE_BASIC_STATE, TLV_RESPONSE_ERROR],
        TLV_OPCODE_FACTORY_RST: [TLV_RESPONSE_ERROR],
        TLV_OPCODE_UPDATE_COMPLETE: [TLV_RESPONSE_ERROR],
        TLV_OPCODE_READ_SENSOR: [TLV_RESPONSE_READ_SENSOR, TLV_RESPONSE_ERROR],
        TLV_OPCODE_REFRESH_DISPLAY: [TLV_RESPONSE_DISPLAY_STATE, TLV_RESPONSE_ERROR],
        TLV_OPCODE_DISPLAY_IMAGE: [TLV_RESPONSE_DISPLAY_STATE, TLV_RESPONSE_ERROR],
        TLV_OPCODE_DISPLAY_TIMED_IMAGE: [TLV_RESPONSE_DISPLAY_STATE, TLV_RESPONSE_ERROR],
        TLV_OPCODE_LED_CONTROL: [TLV_RESPONSE_LED_STATE, TLV_RESPONSE_ERROR],
        TLV_OPCODE_LED_TIMED_CONTROL: [TLV_RESPONSE_LED_STATE, TLV_RESPONSE_ERROR],
    }

    def __post_init__(self):
        # Keep API compatibility: calculate expected responses on init
        self.resolve_acceptable_responses()

    # ------------------------------------------------------------------
    # Properties replacing the old C-style direct assignments
    # ------------------------------------------------------------------

    @property
    def log(self):
        return getLogger("CMD")

    @property
    def opcode(self):
        """Opcode is always the first byte of raw command data"""
        return self.data[0]

    @property
    def esl_id(self):
        """ESL ID is always the second byte of raw command data"""
        return self.data[1]

    @property
    def expired(self):
        """Return True if lifetime has reached zero."""
        return self.lifetime <= 0

    def decay(self):
        """Decrease lifetime counter by one, until it reaches zero."""
        if self.lifetime > 0:
            self.lifetime -= 1

    def resolve_acceptable_responses(self):
        """Resolve and store the list of acceptable response opcodes for this command."""
        op = self.opcode

        # Default: no acceptable responses
        responses = []

        # Broadcast commands never expect any response by ESL spec
        if self.esl_id == BROADCAST_ADDRESS:
            pass # so keep the default empty list
        # Standard opcode mapping
        elif op in self._RESPONSE_MAP:
            responses = self._RESPONSE_MAP[op]
        # Silabs-specific opcode example
        elif (op & TLV_OPCODE_SILABS_SKIP_SET) == TLV_OPCODE_VENDOR_SPECIFIC:
            # as the upper nibble encodes parameter count, it matches the vendor-specific responses' mask by value but is semantically distinct.
            responses = [TLV_RESPONSE_SILABS_SKIP, TLV_RESPONSE_ERROR]
        # Vendor specific case (0x_F range)
        elif (op & TLV_OPCODE_VENDOR_SPECIFIC) == TLV_OPCODE_VENDOR_SPECIFIC:
            responses = [TLV_RESPONSE_VENDOR_SPECIFIC, TLV_RESPONSE_ERROR]

        self.response_opcode = responses

    def accepts_response(self, opcode):
        """Check if response opcode is valid for this command"""
        low = opcode & 0x0F

        # Sensor (0x_E) and vendor specific (0x_F) responses are defined
        # solely by their low nibble according to the ESL specification.
        if low in (TLV_RESPONSE_READ_SENSOR, TLV_OPCODE_VENDOR_SPECIFIC):
            return True
        elif opcode in self.response_opcode:
            self.log.debug("Valid response received: 0x%02x", opcode)
            return True

        self.log.warning(
            "Unexpected response received: 0x%02x - expected: %s!",
            opcode,
            list(map(hex, self.response_opcode)),
        )
        return False

    def __str__(self):
        """Command information"""
        ts = dt.fromtimestamp(self.timestamp).strftime("%d/%b %H:%M:%S.%f")[:-3]
        return (
            "ESL command:\n"
            f"opcode:      {self.opcode:#x}\n"
            f"esl_id:      {self.esl_id}\n"
            f"group_id:    {self.group_id}\n"
            f"slot_number: {self.slot_number}\n"
            f"timestamp:   {ts}\n"
            f"raw data:    {self.data.hex()}"
        )