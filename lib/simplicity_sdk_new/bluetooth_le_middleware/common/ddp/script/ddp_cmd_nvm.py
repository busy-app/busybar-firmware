#!/usr/bin/env python3

# Copyright 2026 Silicon Laboratories Inc. www.silabs.com
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

# Metadata
__author__ = 'Silicon Laboratories, Inc'
__copyright__ = 'Copyright 2026, Silicon Laboratories, Inc.'

import struct
from ddp_cmd import Command, Response

class CommandList:
    """List of DDP commands."""
    SL_DDP_CMD_NVM_SET = 0
    SL_DDP_CMD_NVM_GET = 1

class CommandNvmSet(Command):
    def __new__(cls, id: int, data: bytes) -> "CommandNvmSet":
        """Input structure of DDP command for NVM set.
        """
        return super().__new__(
            cls,
            CommandList.SL_DDP_CMD_NVM_SET,
            struct.pack('<LH', id, len(data)) + data
        )

class ResponseNvmSet(Response):
    """Output structure of DDP command for NVM set."""
    pass

class CommandNvmGet(Command):
    def __new__(cls, id: int) -> "CommandNvmGet":
        """Input structure of DDP command for NVM get.
        """
        return super().__new__(
            cls,
            CommandList.SL_DDP_CMD_NVM_GET,
            struct.pack('<L', id)
        )

class ResponseNvmGet(Response):
    """Output structure of DDP command for NVM get."""
    pass
