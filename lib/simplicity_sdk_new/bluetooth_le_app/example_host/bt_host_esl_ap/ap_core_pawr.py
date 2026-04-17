"""
PAwR handling mixin for AccessPoint.

Contains periodic advertisement (with responses) related helpers.
This mixin does not create or hold any instance state itself and operates
on attributes of the host AccessPoint instance.
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

from datetime import datetime as dt
from ap_constants import (
    AUX_SYNC_IND_PDU_MAX_SKIP_COUNT,
    BROADCAST_ADDRESS,
    EAD_RANDOMIZER_SIZE,
    MAX_ESL_PAYLOAD_SIZE,
    TAG_UNASSOCIATE_TIMEOUT,
    TLV_OPCODE_PING,
)
from ap_config import TAG_SYNC_KEEPING_INTERVAL, TAG_SYNC_TIMEOUT, ESL_CMD_MAX_RETRY_COUNT
from ap_ead import KeyMaterial
from esl_command import ESLCommand
from esl_tag import Tag, TagState, EslState

class PAWRMixin:
    """Periodic Advertisement with Responses handling."""

    def queue_pawr_command(self, cmd : ESLCommand):
        """Prepare periodic advertisement with responses payload by appending to esl command queue"""
        self.esl_command_queue_lock.acquire()
        if cmd.group_id not in self.esl_queued_commands:
            self.esl_queued_commands[cmd.group_id] = []
        self.esl_queued_commands[cmd.group_id].append(cmd)
        self.esl_command_queue_lock.release()

    def requeue_pawr_command(self, cmd : ESLCommand):
        """Prepare periodic advertisement with responses payload by inserting to the front of esl command queue"""
        self.esl_command_queue_lock.acquire()
        if cmd.group_id not in self.esl_queued_commands:
            self.esl_queued_commands[cmd.group_id] = []
        self.esl_queued_commands[cmd.group_id].insert(0, cmd)
        self.esl_command_queue_lock.release()

    def send_pawr_commands(self, subevents):
        """Pack and send commands over PAwR sync"""
        if not self.esl_queued_commands:
            return

        # Try acquire, skip this turn if already locked
        if not self.esl_command_queue_lock.acquire(False):
            return

        empty_gid_list = []
        to_send = []  # list of (gid, commands) tuples

        # Phase 1: work only on the queue under lock
        for gid in subevents:
            if gid not in self.esl_queued_commands:
                continue

            commands = []          # ESLCommand objects
            pop_list = []

            # Iterate over queued commands for this group
            for i in range(len(self.esl_queued_commands[gid])):
                cmd = self.esl_queued_commands[gid][i]   # ESLCommand object
                tlv = cmd.data                           # bytes payload

                payload_size = self.datasize([c.data for c in commands]) + len(tlv)
                if payload_size < MAX_ESL_PAYLOAD_SIZE:
                    commands.append(cmd)
                    pop_list.append(i)
                    self.log.debug(
                        "Opcode 0x%02X to ESL ID %d in group %d written to slot %d",
                        tlv[0], tlv[1], gid, i
                    )
                elif payload_size >= MAX_ESL_PAYLOAD_SIZE - 1:
                    break

            # Remove queued commands that were packed
            for x in reversed(pop_list):
                self.esl_queued_commands[gid].pop(x)
            if len(self.esl_queued_commands[gid]) == 0:
                empty_gid_list.append(gid)

            # Store what we want to send later (outside the lock)
            if commands:
                to_send.append((gid, commands))

        # Remove empty groups
        for gid in empty_gid_list:
            del self.esl_queued_commands[gid]

        # Release queue lock before any further processing
        self.esl_command_queue_lock.release()

        # Phase 2: send data and update pending list without holding the queue lock
        for gid, commands in to_send:
            data = self.create_sync_packet(gid, [cmd.data for cmd in commands])
            if data is not None:
                self.lib.pawr_set_data(self.pawr_handle, gid, len(commands), data)

            # This may indirectly requeue commands, but the queue lock is free now
            self.update_pending_commands_list(gid, commands)

    def remove_pending_commands(self, group_id, indices):
        """
        Remove commands from the pending list for the given group ID based on indices.
        Indices will be sorted in descending order to avoid shifting issues.
        """
        # Ensure descending order
        indices = sorted(indices, reverse=True)

        with self.esl_pending_commands_lock:
            for idx in indices:
                try:
                    self.esl_pending_commands[group_id].pop(idx)
                except IndexError:
                    self.log.error(
                        "Pending command index invalid during removal (group %d, index %d).",
                        group_id,
                        idx,
                    )

    def retry_pending_commands(self, cmd_list):
        """
        Retry all pending commands when no PAwR response was received or set data failed.
        Commands that are retried must be removed from the pending list.
        """
        pop_indices = []

        for cmd, idx in cmd_list:
            if cmd.expired:
                self.log.warning(
                    "Command (0x%s) for ESL ID %d in group %d reached max retry count. Not resending.",
                    cmd.data.hex(),
                    cmd.esl_id,
                    cmd.group_id,
                )
                pop_indices.append(idx)
                continue

            cmd.decay()
            self.log.info(
                "Retry command (0x%s) due to missing PAwR response from ESL ID %d in group %d, remaining lifetime: %d",
                cmd.data.hex(),
                cmd.esl_id,
                cmd.group_id,
                cmd.lifetime,
            )
            self.requeue_pawr_command(cmd)
            pop_indices.append(idx)

        # Remove retried commands from pending list
        if pop_indices:
            group_id = cmd_list[0][0].group_id
            self.remove_pending_commands(group_id, pop_indices)

    def find_all_pending_commands(self, response_slot, subevent):
        """
        Return (tag, [(cmd, index), (cmd, index), ...])
        for all pending commands belonging to the same Tag in this subevent+slot,
        or just all pending commands for a subevent if the response_slot is unknown,
        e.g. in the ESL_LIB_STATUS_PAWR_SET_DATA_FAILED case.
        """
        results = []
        tag = None

        if subevent not in self.esl_pending_commands:
            return tag, results

        for idx, cmd in enumerate(self.esl_pending_commands[subevent]):
            slot_match = (response_slot is None) or (cmd.slot_number == response_slot)
            if slot_match and cmd.group_id == subevent:
                tag = self.tag_db.find((cmd.esl_id, cmd.group_id))
                if tag is not None or response_slot is None:
                    results.append((cmd, idx))

        # All commands belong to the same tag if slot is known, so return that tag or None
        return tag, results

    def datasize(self, cmd_list):
        """Return with the size of the commands in byte in cmd_list"""
        d_size = 0
        for x in cmd_list:
            d_size += len(x)
        return d_size

    def synchronization_handler(self):
        """Handle Tag synchronization"""
        if not self.pawr_active:
            return

        syncronized_tags = self.tag_db.list_esl_state(EslState.SYNCHRONIZED)
        connected_tags = self.tag_db.list_state((TagState.CONNECTED, TagState.CONNECTING))
        tnow = dt.now().timestamp()

        for tag in set(syncronized_tags) - set(connected_tags):
            req_timestamp_diff, resp_timestamp_diff = tag.timestamps_diff(tnow)
            # Send NOP to keep tag synchronized
            if req_timestamp_diff >= TAG_SYNC_KEEPING_INTERVAL - self.pa_timer_interval:
                data = bytearray(self.get_opcode_len(TLV_OPCODE_PING))
                data[0:2] = TLV_OPCODE_PING, tag.esl_id
                self.queue_pawr_command(ESLCommand(tag.group_id, data))
                tag.update_request_timestamp()
            # Check tag timeout
            elif resp_timestamp_diff > TAG_SYNC_TIMEOUT + self.pa_timer_interval:
                self.log.info(
                    "Set tag %s to Unsynchronized because of synchronization timeout",
                    tag.ble_address,
                )
                tag.unsynchronize()
                continue

        unsyncronized_tags = self.tag_db.list_esl_state(EslState.UNSYNCHRONIZED)
        for tag in unsyncronized_tags:
            tnow = dt.now().timestamp()
            _, resp_timestamp_diff = tag.timestamps_diff(tnow)
            # Check tag timeout
            if resp_timestamp_diff > TAG_UNASSOCIATE_TIMEOUT:
                self.log.info(
                    "Unassociate tag %s because of timeout in Unsynchronized state",
                    tag.ble_address,
                )
                tag.unassociate()

    def create_sync_packet(self, gid, tlvs):
        """
        Create ESL synchronization packet.

        parameters:
            tlvs: TLV list
        packet format:
            [ Randomizer | gid | TLV_0 | ... | TLV_n | MIC ]
        returns:
            pack: synchronization packet
        """
        pack = b""
        if (gid < 0) or (gid > 127):
            self.log.warning("Group id must be between 0 and 127")
            return None

        if tlvs:
            # print(gid)
            # gid += 1 # uncomment for ESLP/ESL/SYNC/BI-02-I [Invalid Synchronization Packet] Case 1
            payload = (gid).to_bytes(1, byteorder="little")
            for tlv in tlvs:
                payload += tlv
            self.log.debug("Unencrypted PAwR payload to send: %s", payload.hex())
            pack = self.ead.encrypt(payload, KeyMaterial(self.ap_key), self.randomizer)
            self.log.debug("Encrypted PAwR payload: %s", pack.hex())
            randomizer = int.from_bytes(self.randomizer, 'little')
            randomizer += 1
            self.randomizer = (0xFFFFFFFFFF & randomizer).to_bytes(EAD_RANDOMIZER_SIZE, 'little')
            # pack = pack[:2] + b"*" + pack[3:] # uncomment for ESLP/ESL/SYNC/BI-02-I [Invalid Synchronization Packet] Case 2
            # print(pack.hex())
        else:
            self.log.warning("No TLV values to send")
            pack = None
        return pack

    def get_pawr_params(self):
        """Get periodic advertisement current parameter set"""
        return [
            self.adv_interval_min,
            self.adv_interval_max,
            self.subevent_count,
            self.subevent_interval,
            self.response_slot_delay,
            self.response_slot_spacing,
            self.response_slot_count,
        ]

    def set_pawr_interval(self, adv_interval=None, persistive=False):
        """
        Set periodic advertisement interval

        parameter:
            adv_interval: Periodic advertisement interval in units of 1.25 ms
        """
        adv_interval_max = self.adv_interval_max
        if adv_interval:
            adv_interval_max = adv_interval[-1]
            if persistive:
                self.adv_interval_min = adv_interval[0]
                self.adv_interval_max = adv_interval_max
        self.pa_timer_interval = (
            adv_interval_max / 800.0
        )  # don't forget: the natural units of adv_interval values are 1.25[ms]!

    def pawr_configure(
        self,
        adv_interval_min,
        adv_interval_max,
        subevent_count,
        subevent_interval,
        response_slot_delay,
        response_slot_spacing,
        response_slot_count,
    ):
        """
        Set PAwR configuration, validate if possible
        """
        # Store the unchecked configuration - may turn to be invalid, in which case it will reverted to the last known good state of the ESL C lib config in the following event
        self.adv_interval_min = adv_interval_min
        self.adv_interval_max = adv_interval_max
        self.subevent_count = subevent_count
        self.subevent_interval = subevent_interval
        self.response_slot_delay = response_slot_delay
        self.response_slot_spacing = response_slot_spacing
        self.response_slot_count = response_slot_count
        if self.pawr_handle is not None:
            self.lib.pawr_configure(
                self.pawr_handle,
                adv_interval_min,
                adv_interval_max,
                subevent_count,
                subevent_interval,
                response_slot_delay,
                response_slot_spacing,
                response_slot_count,
            )
        else:
            self.log.warning(
                "PAwR hasn't been created yet, skipping parameter validation. Please note that this configuration may be inplausible."
            )

    def start_pawr_train(self, adv_interval=None, advertise=False):
        """
        Start periodic advertisement command

        parameter:
            adv_interval: Periodic advertisement interval in natural units
        """
        if self.pawr_active is None:
            self.set_pawr_interval(adv_interval)
            if self.pawr_handle is None:
                self.pawr_handle = self.lib.pawr_create()
            self.lib.pawr_configure(
                self.pawr_handle,
                adv_interval_min=self.adv_interval_min
                if adv_interval is None
                else adv_interval[0],
                adv_interval_max=self.adv_interval_max
                if adv_interval is None
                else adv_interval[-1],
                subevent_count=self.subevent_count,
                subevent_interval=self.subevent_interval,
                response_slot_delay=self.response_slot_delay,
                response_slot_spacing=self.response_slot_spacing,
                response_slot_count=self.response_slot_count,
            )
            self.lib.pawr_enable(self.pawr_handle, advertise=advertise)
            self.pawr_active = False
        else:
            self.pawr_restart = (
                adv_interval
                if adv_interval is not None
                else [self.adv_interval_min, self.adv_interval_max]
            )
            self.stop_pawr_train()
        # Actual advertise status may be overridden by EventPawrStatus event - until it arrives, assume it will advertise!
        self.pawr_advertising = advertise

    def stop_pawr_train(self):
        """Stop periodic advertisement command"""
        if self.pawr_active is not None:
            try:
                self.lib.pawr_enable(self.pawr_handle, False)
            except Exception as e:
                self.log.error(e)
        else:
            self.log.info("PAwR train is not running.")

    def update_pending_commands_list(self, gid, cmds):
        """
        Update unresponded command list using existing ESLCommand objects.
        Any previously pending commands for this group are retried first,
        because they were not answered in the previous PAwR interval (if any).
        """

        # Step 1: extract old pending commands safely
        with self.esl_pending_commands_lock:
            old_pending = [
                (cmd, idx)
                for idx, cmd in enumerate(self.esl_pending_commands.get(gid, []))
            ]
            #self.esl_pending_commands[gid] = []  # clear immediately

        # Step 2: retry old pending commands
        if old_pending:
            self.retry_pending_commands(old_pending)

        # Step 3: determine last slot number for each ESL ID
        last_slotnum = {
            cmd.esl_id: i
            for i, cmd in enumerate(cmds)
            if cmd.esl_id != BROADCAST_ADDRESS
        }

        # Step 4: build new pending list
        new_pending = []
        for cmd in cmds:
            if cmd.esl_id != BROADCAST_ADDRESS:
                cmd.slot_number = last_slotnum[cmd.esl_id]
                if cmd.response_opcode is not None:
                    new_pending.append(cmd)

        # Step 5: store new pending commands
        with self.esl_pending_commands_lock:
            self.esl_pending_commands[gid] = new_pending

    def remove_esl_pawr_requests(self, tag):
        """Remove all pending and queued PAwR commands belonging to the given tag."""
        group_id = tag.group_id
        esl_id = tag.esl_id
        removed = []
        # Remove from pending
        with self.esl_pending_commands_lock:
            # Filter out commands belonging to this ESL
            cmds = self.esl_pending_commands.get(group_id, [])
            remaining = []

            for cmd in cmds:
                if cmd.esl_id == esl_id:
                    removed.append(cmd)
                else:
                    remaining.append(cmd)

            if remaining:
                self.esl_pending_commands[group_id] = remaining
            else:
                self.esl_pending_commands.pop(group_id, None)
        # Remove from queued
        with self.esl_command_queue_lock:
            cmds = self.esl_queued_commands.get(group_id, [])
            remaining = []

            for cmd in cmds:
                if cmd.esl_id == esl_id:
                    removed.append(cmd)
                else:
                    remaining.append(cmd)

            if remaining:
                self.esl_queued_commands[group_id] = remaining
            else:
                self.esl_queued_commands.pop(group_id, None)

        return removed
