"""
ESL AP configuration.
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
# Common constants used both by the BLE and ESL threads
from ap_constants import TAG_SYNC_TIMEOUT

# Alternates between GATT Write With- or Without Response requests, and allows skipping some value range validation
# and other sanity checks when set to True, so that IOP edge case tests can run properly.
IOP_TEST = False

# Reusing encryption keys may be bad security practice, but resynchronisation by scanning cannot work without it.
KEY_MATERIAL_REUSE_ENABLED = False

# AP will send NOP commands in every 30 minutes by default
TAG_SYNC_KEEPING_INTERVAL = TAG_SYNC_TIMEOUT / 2

# PAwR train initial extended advertising enabled when AP is started in auto mode (any PAwR train restart still requires explicit 'sync start --advertise' command).
INITIAL_AUTO_ADVERTISE_PAWR_TRAIN = False

# Max number of images to upload automatically
IMAGE_MAX_AUTO_UPLOAD_COUNT = 2

# Retry count for ESL command opcodes re-sending
ESL_CMD_MAX_RETRY_COUNT = 3

# Limit of unsuccessful onboarding attempts before blocking the tag to avoid endless retries
# Set it higher than the internal retry limits of the ESL C library (ESL_LIB_CONNECTION_RETRY_COUNT_MAX value)
UNSUCCESSFUL_ONBOARDING_LIMIT = 6

# Pending count for connection requests: 1 is the minmum ad also the safest value, but auto provisioning will be the slowest
ESL_CMD_MAX_PENDING_CONNECTION_REQUEST_COUNT = 4096 # Best if aligned with elw.ESL_LIB_SKIPLIST_MAX_LEVEL_LIB macro value

# Tags in a group in automated mode addressing
ESL_MAX_TAGS_IN_AUTO_GROUP = 23 # enough for 1311 nodes with current PAwR defaults

# Default RSSI threshold in dBm
RSSI_THRESHOLD = -80

# Scanning parameters
SCAN_INTERVAL_DEFAULT_MS = 15.0
SCAN_WINDOW_DEFAULT_MS = 15.0

# Advertising timeout [s]
ADVERTISING_TIMEOUT = 60

# (Re)connecting timeout [s]
# Allow enough time for the ESL C library for the re-connection attempts, which may be delayed by other queued connections
CONNECTING_TIMEOUT = 3600 # Filter Accept List based auto connect requires huge grace period

# The timeout is needed to get the KeyboardInterrupt.
# The timeout value is a tradeoff between CPU load and KeyboardInterrupt response time.
# timeout=None: minimal CPU usage, KeyboardInterrupt not recognized until blocking wait is released.
# timeout=0: maximal CPU usage, KeyboardInterrupt recognized immediately.
# See the documentation of Queue.get method for details.
BLOCKING_WAIT_TIMEOUT = 0.1
