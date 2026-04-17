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

class Command(bytes):
    def __new__(cls, cmd, body: bytes) -> "Command":
        """Input structure of DDP command.

        :param cmd: Command ID
        :param body: Command body
        """
        return super().__new__(cls, struct.pack('<HH', int(cmd), len(body)) + body)

class Response:
    def __init__(self, data: bytes):
        """Output structure of DDP command.

        :param data: Response data
        """
        self.status, length = struct.unpack('<iH', data[:6])
        self.body = data[6:6 + length]
