"""
ESL Tag Database.
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
import esl_tag
from ap_constants import BROADCAST_ADDRESS


class TagDB:
    """Efficient ESL tag registry with indexed O(1) lookup."""

    def __init__(self):
        # Original API storage
        self.tags = []

        # Direct lookup indexes (O(1))
        self.by_ble_address = {}
        self.by_esl_address = {}
        self.by_connection_handle = {}

        # Set-based indexes (O(1) average)
        self.by_group_id = {}
        self.by_esl_id = {}
        self.by_state = {}
        self.by_esl_state = {}

    # ----------------------------------------------------------------------
    # ADD / REMOVE
    # ----------------------------------------------------------------------

    def add(self, lib: esl_lib.Lib, address: esl_lib.Address, dummy=False):
        tag = self.find(address)
        if tag is not None:
            return tag

        tag = esl_tag.Tag(lib, address, dummy=dummy)
        self.tags.append(tag)

        # Populate indexes
        self._index_new_tag(tag)

        # Subscribe to Tag property changes
        tag.add_observer(self._on_tag_changed)

        return tag

    def remove(self, tag: esl_tag.Tag = None, address: esl_lib.Address = None):
        if address is not None:
            tag = self.find(address)
        if tag is None:
            return

        # Unsubscribe from Tag notifications
        tag.remove_observer(self._on_tag_changed)

        # Remove from indexes
        self._unindex_tag(tag)

        self.tags.remove(tag)

    # ----------------------------------------------------------------------
    # INDEXING HELPERS
    # ----------------------------------------------------------------------

    def _index_new_tag(self, tag):
        """Insert tag into all indexes."""
        # ble_address -> Tag
        self.by_ble_address[tag.ble_address] = tag

        # esl_address -> Tag
        if tag.esl_address is not None:
            self.by_esl_address[tag.esl_address] = tag
            self.by_group_id.setdefault(tag.group_id, set()).add(tag)
            self.by_esl_id.setdefault(tag.esl_id, set()).add(tag)

        # state -> set(Tag)
        self.by_state.setdefault(tag.state, set()).add(tag)

        # esl_state -> set(Tag)
        self.by_esl_state.setdefault(tag.esl_state, set()).add(tag)

        # connection_handle -> Tag
        if tag.connection_handle is not None:
            self.by_connection_handle[tag.connection_handle] = tag

    def _unindex_tag(self, tag):
        """Remove tag from all indexes."""
        self.by_ble_address.pop(tag.ble_address, None)

        if tag.esl_address is not None:
            self.by_esl_address.pop(tag.esl_address, None)
            self.by_group_id.get(tag.group_id, set()).discard(tag)
            self.by_esl_id.get(tag.esl_id, set()).discard(tag)

        self.by_state.get(tag.state, set()).discard(tag)
        self.by_esl_state.get(tag.esl_state, set()).discard(tag)

        if tag.connection_handle is not None:
            self.by_connection_handle.pop(tag.connection_handle, None)

    # ----------------------------------------------------------------------
    # OBSERVER CALLBACK
    # ----------------------------------------------------------------------

    def _on_tag_changed(self, tag, field, old, new):
        """Called by Tag._notify() when a property changes."""

        # --------------------------------------------------------------
        # esl_address changed (affects esl_id and group_id as well)
        # --------------------------------------------------------------
        if field == "esl_address":
            if old is not None:
                self.by_esl_address.pop(old, None)
                old_group = (old >> 8) & 0x7F
                old_id = old & 0xFF
                self.by_group_id.get(old_group, set()).discard(tag)
                self.by_esl_id.get(old_id, set()).discard(tag)

            if new is not None:
                self.by_esl_address[new] = tag
                self.by_group_id.setdefault(tag.group_id, set()).add(tag)
                self.by_esl_id.setdefault(tag.esl_id, set()).add(tag)

        # --------------------------------------------------------------
        # connection_handle changed
        # --------------------------------------------------------------
        elif field == "connection_handle":
            if old is not None:
                self.by_connection_handle.pop(old, None)
            if new is not None:
                self.by_connection_handle[new] = tag

        # --------------------------------------------------------------
        # state changed
        # --------------------------------------------------------------
        elif field == "state":
            self.by_state.get(old, set()).discard(tag)
            self.by_state.setdefault(new, set()).add(tag)

        # --------------------------------------------------------------
        # esl_state changed (derived property)
        # --------------------------------------------------------------
        elif field == "esl_state":
            self.by_esl_state.get(old, set()).discard(tag)
            self.by_esl_state.setdefault(new, set()).add(tag)

    # ----------------------------------------------------------------------
    # FIND
    # ----------------------------------------------------------------------

    def find(self, node_id):
        """Find tag by BLE address, connection handle, or (esl_id, group_id)."""

        if isinstance(node_id, (esl_lib.Address, str)):
            return self.by_ble_address.get(node_id)

        elif isinstance(node_id, esl_lib.ConnectionHandle):
            return self.by_connection_handle.get(node_id)

        elif isinstance(node_id, (tuple, list)):
            esl_id, group_id = node_id
            if esl_id == BROADCAST_ADDRESS:
                return None
            esl_address = esl_id | (group_id << 8)
            return self.by_esl_address.get(esl_address)

        return None

    # ----------------------------------------------------------------------
    # LISTING
    # ----------------------------------------------------------------------

    @property
    def device_count(self):
        return len(self.tags)

    def all(self):
        return self.tags

    def list_group(self, group_id):
        return list(self.by_group_id.get(group_id, []))

    def list_state(self, state):
        if not isinstance(state, (list, tuple)):
            state = [state]
        result = []
        for s in state:
            result.extend(self.by_state.get(s, []))
        return result

    def list_esl_state(self, esl_state):
        if not isinstance(esl_state, (list, tuple)):
            esl_state = [esl_state]
        result = []
        for s in esl_state:
            result.extend(self.by_esl_state.get(s, []))
        return result

    def list_advertising(self):
        # This remains O(N) because advertising is a dynamic property
        return [tag for tag in self.tags if tag.advertising]

    def list_blocked(self):
        # Also O(N), rarely used and dynamic
        return [tag for tag in self.tags if tag.blocked]

    def list_all_groups(self):
        groups = {}
        for tag in self.tags:
            if tag.associated:
                groups.setdefault(tag.group_id, []).append(tag.esl_id)
        return dict(sorted(groups.items()))
