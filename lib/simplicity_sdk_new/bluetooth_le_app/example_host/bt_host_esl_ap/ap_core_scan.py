"""
ESL AP Core - scanning mixin.
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

from ap_config import SCAN_INTERVAL_DEFAULT_MS, SCAN_WINDOW_DEFAULT_MS

class ScanMixin:
    """Scanning and discovery"""

    def start_scan(self, active=False, clear_lists=False):
        """Start scanning"""
        if clear_lists:
            if self.cmd_mode:  # we don't want to reset advertisers in auto mode
                for tag in self.tag_db.list_advertising():
                    tag.reset_advertising()
            for tag in self.tag_db.list_blocked():
                if not self.exclusive_network.enabled or \
                self.exclusive_network.contains_address(tag.ble_address):
                    tag.unblock()
        if not self.scan_runs:
            self.lib.scan_configure(
                active_mode=active,
                interval_ms=SCAN_INTERVAL_DEFAULT_MS,
                window_ms=SCAN_WINDOW_DEFAULT_MS,
            )
            self.lib.scan_enable()
            self.scan_runs = True
        else:
            self.log.info("Scanning already running.")

    def stop_scan(self):
        """Stop scanning"""
        if self.scan_runs:
            self.lib.scan_enable(False)
        else:
            self.log.info("Scanning is not running.")
