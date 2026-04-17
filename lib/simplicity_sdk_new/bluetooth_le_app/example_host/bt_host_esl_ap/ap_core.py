"""
ESL AP Core.
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

import threading
import os
import queue
import traceback
from datetime import datetime
from ap_config import ESL_MAX_TAGS_IN_AUTO_GROUP, RSSI_THRESHOLD, BLOCKING_WAIT_TIMEOUT
from ap_constants import ESL_MAX_TAGS_IN_GROUP, EAD_RANDOMIZER_SIZE
from ap_logger import getLogger, log
from esl_tag import TagState, PAwRSyncLostError
from esl_tag_db import TagDB
from ap_ead import EAD
from secrets import token_bytes
import esl_key_lib
import esl_lib
import esl_lib_wrapper as elw
from ap_core_utils import (
    ExclusiveMode,
    skip_if_stopped,
    StackSizeManager,
    LibProxy,
)
# import AP Core MixIn classes
from ap_core_commands import CLICommandsMixin
from ap_core_pawr import PAWRMixin
from ap_core_tag_commands import TagCommandsMixin
from ap_core_pawr_responses import PAWRResponsesMixin
from ap_core_helpers import HelpersMixin
from ap_core_scan import ScanMixin
from ap_core_event_handlers_common import CommonEventHandlersMixin
from ap_core_event_handlers_cli import CLIEventHandlersMixin
from ap_core_event_handlers_auto import AutoEventHandlersMixin
from ap_core_event_handlers_demo import DemoEventHandlersMixin

# sanity check
if ESL_MAX_TAGS_IN_GROUP < ESL_MAX_TAGS_IN_AUTO_GROUP:
    raise Exception(
        "Invalid configuration for auto addressing: ESL_MAX_TAGS_IN_AUTO_GROUP violates ESL specification"
    )

class AccessPoint(
    HelpersMixin,
    # Common event handlers for all modes
    CommonEventHandlersMixin,
    # Mode specific extensions (do not override common handlers, just extend)
    CLIEventHandlersMixin,
    AutoEventHandlersMixin,
    DemoEventHandlersMixin,
    # CLI commands
    CLICommandsMixin,
    # PAwR control
    PAWRMixin,
    # Tag commands
    TagCommandsMixin,
    # PAwR response handling and helpers
    PAWRResponsesMixin,
    ScanMixin,
):
    """Access Point"""

    def __init__(
        self, config, unsecure, cmd_mode=False, demo_mode=False, exclusive_list=None
    ):
        manager = StackSizeManager(self.log)
        manager.configure_stack_size()  # Can terminate the script if stack setup fails

        self.scan_runs = False
        self.pawr_active = None # None: disabled, True: enabled, False: requested but not yet active
        self.pawr_advertising = False
        self.auto_override = False
        self.cmd_mode = cmd_mode if not demo_mode else True
        self.lib_connection_mode = elw.ESL_LIB_CONNECTION_MODE_SINGLE
        self.skip_mode_evt_after_boot = False
        self.demo_mode = demo_mode
        self.demo_controller_connected = False
        self.evt_dispatcher = esl_lib.EventDispatcher()
        self.event_handler_prefix_list = [""]
        self.subscribe_event_handlers()
        self.evt_dispatcher.lock_current_observers()
        self.set_mode_handlers()
        self.randomizer = token_bytes(EAD_RANDOMIZER_SIZE)
        # some PAwR stuff
        self.cli_queue = None
        self.ead = EAD()
        if not unsecure:
            config += " -secure"
        else:
            self.log.warning("Starting with NCP encryption disabled!")

        self.lib = LibProxy(esl_lib.Lib(config), self)

        self.rssi_threshold = RSSI_THRESHOLD

        self.tag_db = TagDB()
        self.ap_key = self.ead.generate_key_material()
        self.ncp_address = None

        self.image_path = "image/"
        self.image_files = [
            f
            for f in os.listdir(self.image_path)
            if os.path.isfile(os.path.join(self.image_path, f))
        ]
        self.start_time = datetime.now()
        self.auto_config_start_time = None
        self.auto_configured_tags_in_single_run = 0
        # ESL command dictionary of lists in a format: {group_id : [tlv0, tlv1, ..., tlvN]}
        self.esl_queued_commands = {}
        self.esl_pending_commands = {}
        self.esl_command_queue_lock = threading.Lock()
        self.esl_pending_commands_lock = threading.Lock()
        # Shutdown timer is executed if boot event fails
        self.shutdown_timer = threading.Timer(5.0, self.shutdown_timeout)
        self.shutdown_timer.daemon = True
        self.shutdown_timer.start()
        # ESL Demo controller related attributes
        self.controller_command = None
        self.demo_auto_reconfigure = False
        self.image_from_controller = b""
        self.controller_image_index = None
        self.image_data_offset = bytearray(4)

        self.pawr_handle = None
        self.pawr_restart = None
        # PAwR default parameters
        self.adv_interval_min = elw.ESL_LIB_PAWR_MIN_INTERVAL_DEFAULT
        self.adv_interval_max = elw.ESL_LIB_PAWR_MAX_INTERVAL_DEFAULT
        self.adv_interval_last = elw.ESL_LIB_PAWR_MAX_INTERVAL_DEFAULT
        self.subevent_count = elw.ESL_LIB_PAWR_SUBEVENT_COUNT_DEFAULT
        self.subevent_interval = elw.ESL_LIB_PAWR_SUBEVENT_INTERVAL_DEFAULT
        self.response_slot_delay = elw.ESL_LIB_PAWR_RESPONSE_SLOT_DELAY_DEFAULT
        self.response_slot_spacing = elw.ESL_LIB_PAWR_RESPONSE_SLOT_SPACING_DEFAULT
        self.response_slot_count = elw.ESL_LIB_PAWR_RESPONSE_SLOT_COUNT_DEFAULT
        self.set_pawr_interval()

        # State of connection count for demo/auto modes
        self.max_conn_count_reached = False
        self.bonding_finished = True

        self.stop_event = threading.Event()
        self.consumer = threading.Thread(target=self.dequeue, daemon=True)
        self.consumer.start()

        self.key_db = esl_key_lib.Lib()
        self.stat_sum = None
        self.exclusive_network = ExclusiveMode.from_file(exclusive_list, self.lib, self.key_db, self.tag_db, self.log)

    # Logger
    @property
    def log(self):
        return getLogger()

    ################# CLI Handler methods #################

    # All ESL methods (ap_* methods) were moved to CLICommandsMixin
    # to reduce the size of this file as pat of a modular refactor.
    # These methods now live in ap_core_cli_commands.py

    #################### local helpers ####################
    def subscribe_event_handlers(self):
        for prefix in self.event_handler_prefix_list:
            for evt in esl_lib.EventType.all():
                method_name = prefix + "esl_event_" + evt.name
                if hasattr(self, method_name):
                    self.evt_dispatcher.subscribe(evt.name, getattr(self, method_name))

    def set_mode_handlers(self):
        self.evt_dispatcher.prune()
        while len(self.event_handler_prefix_list) > 1:
            elem = self.event_handler_prefix_list.pop()
        if self.auto_override or self.cmd_mode:
            self.event_handler_prefix_list.append("cli_")
        else:
            self.event_handler_prefix_list.append("auto_")
        if self.demo_mode:
            self.event_handler_prefix_list.append("demo_")
        self.subscribe_event_handlers()

    def revert_auto_mode(self):
        if (
            self.auto_override
            and len(self.tag_db.list_state((TagState.CONNECTED, TagState.CONNECTING)))
            == 0
        ):
            self.cmd_mode = False
            self.auto_override = False
            self.log.warning("REVERT TO AUTO MODE!")
            self.set_mode_handlers()

    ##################### ESL methods #####################

    # The following tag-related methods were moved to TagCommandsMixin class:
    #   configure, connect, disconnect, past, write_values, upload_auto_image,
    #   upload_next_image, route_command, send_cp_command, led_control_command,
    #   ltk_conditional_removal, check_sync

    ################## ESL event handlers #################

    # ----------------------------------------------------------------------------------------------
    # Common ESL event handler methods which executes in all modes (auto/command line/demo) moved to
    # ap_core_event_handlers_common.py to reduce the size of this file and begin modular refactor.

    # ----------------------------------------------------------------------------------------------
    # ESL event handler method extensions for the CLI mode moved to CLIEventHandlersMixin class in
    # ap_core_event_handlers_cli.py to reduce the size of this file and begin modular refactor.

    # ----------------------------------------------------------------------------------------------
    # ESL event handler method extensions for auto mode moved to AutoEventHandlersMixin class in
    # ap_core_event_handlers_auto.py to reduce the size of this file and begin modular refactor.

    # ----------------------------------------------------------------------------------------------
    # ESL event handler method extensions for demo mode moved to DemoEventHandlersMixin class in
    # ap_core_event_handlers_demo.py to reduce the size of this file and begin modular refactor.

    # ----------------------------------------------------------------------------------------------
    # Other AP Class methods
    def dequeue(self):
        """Check and process event queue"""
        while True:
            try:
                event = self.lib.event_queue.get(timeout=BLOCKING_WAIT_TIMEOUT)
            except queue.Empty:
                continue

            if isinstance(event, esl_lib.EventError):
                if event.lib_status not in [
                    elw.ESL_LIB_STATUS_PAST_INIT_FAILED,
                    elw.ESL_LIB_STATUS_PAWR_START_FAILED,
                    elw.ESL_LIB_STATUS_PAWR_SET_DATA_FAILED,
                ]:
                    self.log.warning("[Event] " + str(event))
                elif event.sl_status == elw.SL_STATUS_TRANSMIT or (
                    event.sl_status
                    == elw.SL_STATUS_BT_CTRL_UNKNOWN_ADVERTISING_IDENTIFIER
                    and self.pawr_active
                ):
                    self.log.error("[Event] " + str(event))
            else:
                filter_events = [
                    elw.ESL_LIB_EVT_PAWR_DATA_REQUEST,
                    elw.ESL_LIB_EVT_TAG_FOUND,
                ]
                if not event.evt_code in filter_events:
                    self.log.debug("[Event] " + str(event))

            if hasattr(event, "node_id"):
                tag = self.tag_db.find(event.node_id)
            elif hasattr(event, "address"):
                tag = self.tag_db.find(event.address)
            elif hasattr(event, "connection_handle"):
                tag = self.tag_db.find(event.connection_handle)
            else:
                tag = None  # prevent sending unsolicited / mismatching events to tag found in previous cylcle iteration!

            # skip tag events processing if stop is in progress (e.g. due boot timeout)
            if not self.stop_event.is_set() and tag is not None:
                try:
                    tag.handle_event(event)
                except PAwRSyncLostError as e:
                    if self.pawr_active and self.subevent_count <= e.group_id:
                        self.log.error(
                            "ESL ID %d in group %d at %s address can't be synced because the group ID can't be mapped to an existing PAwR subevent (subevent count: %d)!",
                            e.esl_id,
                            e.group_id,
                            e.ble_address,
                            self.subevent_count,
                        )
                    else:
                        tag.response_key_update(self.ead.generate_key_material())
                        self.log.warning(
                            "ESL ID %d in group %d at %s address lost sync!",
                            e.esl_id,
                            e.group_id,
                            e.ble_address,
                        )

            try:
                event_name = esl_lib.EventType(event.evt_code).name
                self.evt_dispatcher.notify(event_name, event)
            except Exception as err:
                log(err)
                log(traceback.format_exc())
                self.shutdown_cli()

    @skip_if_stopped
    def shutdown(self):
        """Initiate shutdown"""
        self.stop_event.set()
        try:
            self.lib.stop() # this method blocks the main thread until esl_lib deinit is done
        except esl_lib.CommandFailedError:
            self.log.warning("Clean shutdown failed")

    def shutdown_timeout(self):
        """Shutdown on ESL AP NCP init timeout"""
        self.log.critical(
            "ESL AP NCP initialization timeout, no response received! Shutdown."
        )
        self.shutdown_cli()

    def shutdown_cli(self):
        """Propagate shutdown towards the CLI"""
        if self.cli_queue is not None:
            self.cli_queue.put("exit")
