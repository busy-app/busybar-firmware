"""
ESL Library
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

import ctypes

# True multiprocessing is temporarily disabled.
import multiprocessing.dummy as mp
from multiprocessing.connection import Connection
import os
import sys
import threading
import ap_logger
import esl_lib_wrapper as elw
from ap_constants import ADDRESS_TYPE_PUBLIC_ADDRESS

SCAN_PHY_1M = 1
SCAN_PHY_CODED = 4
SCAN_PHY_1M_AND_CODED = 5
SCAN_DISCOVER_LIMITED = 0
SCAN_DISCOVER_GENERIC = 1
SCAN_DISCOVER_OBSERVATION = 2


def byref(x):
    """Return the reference to an object, or None if the input is None"""
    if x is None:
        return None
    return ctypes.byref(x)


def bytes_to_array(source: bytes):
    if source is None:
        return None
    return (ctypes.c_ubyte * (1 + len(source)))(len(source), *source)


def array_to_bytes(source: elw.esl_lib_array_t):
    data = ctypes.cast(
        ctypes.addressof(source.data), ctypes.POINTER(ctypes.c_ubyte * source.len)
    )
    return bytes(data.contents)


def array_p(array):
    if array is None:
        return None
    return ctypes.cast(ctypes.pointer(array), ctypes.POINTER(elw.esl_lib_array_t))


def bytes_to_long_array(source: bytes):
    if source is None:
        return None
    array = len(source).to_bytes(4, sys.byteorder) + source
    return (ctypes.c_ubyte * (4 + len(source)))(*array)


def long_array_to_bytes(source: elw.esl_lib_long_array_t):
    data = ctypes.cast(
        ctypes.addressof(source.data), ctypes.POINTER(ctypes.c_ubyte * source.len)
    )
    return bytes(data.contents)


def long_array_p(array):
    if array is None:
        return None
    return ctypes.cast(ctypes.pointer(array), ctypes.POINTER(elw.esl_lib_long_array_t))


def get_enum(prefix, index):
    """Return a dictionary from an enum"""
    try:
        return {
            getattr(elw, name): name for name in dir(elw) if name.startswith(prefix)
        }[index]
    except KeyError:
        return f"Unknown {index} ({hex(index)})"


def get_sl_status_str(index):
    """Return string for an sl_status_t index"""
    return get_enum("SL_STATUS_", index)


def event_factory(evt_code: elw.esl_lib_evt_type_t, evt_data: elw.esl_lib_evt_data_t):
    """Transform ctype object to specific class instance"""
    evt_class_list = [
        EventSystemBoot,
        EventConnectionMode,
        EventScanStatus,
        EventTagFound,
        EventTagInfo,
        EventConfigureTagResponse,
        EventConnectionRetry,
        EventConnectionClosed,
        EventConnectionOpened,
        EventBondingData,
        EventBondingFinished,
        EventPawrConfig,
        EventPawrStatus,
        EventPawrResponse,
        EventPawrDataRequest,
        EventShutdownReady,
        EventError,
        EventImageTransferFinished,
        EventImageType,
        EventControlPointResponse,
        EventControlPointNotification,
        EventGeneral,
    ]
    for cls in evt_class_list:
        if cls.evt_code == evt_code:
            return cls(evt_data)
    raise Exception(f"Unknown ESL event: {evt_code}")


def get_node_id(node_id: elw.esl_lib_node_id_t):
    """Convert node ID to a native Python object"""
    if node_id.type == elw.ESL_LIB_NODE_ID_TYPE_ADDRESS:
        return Address.from_ctype(node_id.id.address)
    if node_id.type == elw.ESL_LIB_NODE_ID_TYPE_CONNECTION:
        return ConnectionHandle(node_id.id.connection_handle)
    if node_id.type == elw.ESL_LIB_NODE_ID_TYPE_PAWR:
        return PAWRSubevent.from_ctype(node_id.id.pawr)
    return None


class Address:
    """Bluetooth address"""

    def __init__(self, address: bytes, address_type=None):
        if isinstance(address, Address):
            self.addr = address.addr
            self.address_type = address.address_type
        elif len(address) != 6:
            raise ValueError(f"Invalid address: {address}")
        else:
            self.addr = address
            if address_type is None:
                # use public device address type per default
                self.address_type = ADDRESS_TYPE_PUBLIC_ADDRESS
            else:
                self.address_type = int(address_type)

    def __len__(self):
        return len(self.addr)

    def __repr__(self) -> str:
        return (
            ":".join([f"{b:02X}" for b in iter(reversed(self.addr))])
        )

    def __str__(self) -> str:
        return (
            ":".join([f"{b:02X}" for b in iter(reversed(self.addr))])
            + f", type {self.address_type}"
        )

    def __eq__(self, other):
        if other is None:
            return False
        if isinstance(other, Address):
            return (self.addr == other.addr) and (
                self.address_type == other.address_type
            )
        if len(other) >= 12:
            val_b = bytes.fromhex(other.replace(":", ""))[::-1]
            return val_b == self.addr
        return other == self.addr

    def __hash__(self):
        return hash((self.addr, self.address_type))

    @classmethod
    def from_str(cls, address: str, address_type=None):
        """Create new address instance from string object"""
        return cls(
            bytes.fromhex(address.replace(":", ""))[::-1],
            ADDRESS_TYPE_PUBLIC_ADDRESS if address_type is None else int(address_type),
        )

    @classmethod
    def from_ctype(cls, address: elw.esl_lib_address_t):
        """Create new address instance from ctype object"""
        return cls(bytes(address.addr), int(address.address_type))

    def to_ctype(self) -> elw.esl_lib_address_t:
        """Convert to ctype object"""
        # Added 1 byte padding for Filter Accept List search comparison boost
        return elw.esl_lib_address_t.from_buffer_copy(
            self.addr + self.address_type.to_bytes(1, "little") + bytes(1)
        )


class ConnectionHandle(int):
    """Connection handle representation"""

    def __repr__(self):
        return hex(self)


class PAWRSubevent:
    """Wrapper for esl_lib_pawr_subevent_t"""

    def __init__(self, handle: int, subevent: int):
        self.handle = handle
        self.subevent = (
            subevent
            if subevent != elw.ESL_LIB_PAWR_UNSPECIFIED_SUBEVENT
            else "unspecified"
        )

    def __repr__(self) -> str:
        return f"PAWR 0x{self.handle:08x} subevent {self.subevent}"

    @classmethod
    def from_ctype(cls, pawr: elw.esl_lib_pawr_subevent_t):
        """Convert from ctype object"""
        # Store pointer as an integer
        return cls(pawr.handle, pawr.subevent)

    def to_ctype(self) -> elw.esl_lib_pawr_subevent_t:
        """Convert to ctype object"""
        return elw.esl_lib_pawr_subevent_t(ctypes.c_void_p(self.handle), self.subevent)

class EventDispatcher:
    """Thread-safe event dispatcher for ESL library event management."""

    def __init__(self):
        """Initialize dispatcher with subscriber storage, lock, and protected observers list."""
        self._subscribers = {}
        # lock is used to avoid race conditions when multiple threads might modify subscribers
        self._lock = threading.Lock()
        # a separate list of protected observers is kept even on clearing
        self._protected = {}

    def subscribe(self, event_name, callback, prefix=None, protected=False):
        """Register a callback for a given event name, optionally as protected."""
        with self._lock:
            subscribers = self._subscribers.setdefault(event_name, [])
            if (prefix, callback) not in subscribers:
                subscribers.append((prefix, callback))
            if protected:
                protected_list = self._protected.setdefault(event_name, [])
                if (prefix, callback) not in protected_list:
                    protected_list.append((prefix, callback))

    def unsubscribe(self, event_name, callback, forced=False):
        """Remove a callback for a given event name (skip if protected unless forced)."""
        with self._lock:
            if event_name in self._subscribers:
                self._subscribers[event_name] = [
                    (p, c) for (p, c) in self._subscribers[event_name]
                    if c != callback or (
                        not forced and event_name in self._protected and (p, c) in self._protected[event_name]
                    )
                ]
                if forced and event_name in self._protected:
                    self._protected[event_name] = [
                        (p, c) for (p, c) in self._protected[event_name] if c != callback
                    ]

    def prune(self):
        """Remove all callbacks except the protected observers."""
        with self._lock:
            self._subscribers = {k: list(self._protected.get(k, [])) for k in self._subscribers.keys()}

    def clear(self):
        """Remove all callbacks for all events."""
        with self._lock:
            # clearing dictionaries ensures no stale references remain
            self._subscribers.clear()
            self._protected.clear()

    def notify(self, event_name, event):
        """Notify all callbacks subscribed to a given event."""
        # copy list under lock to avoid issues if subscribers change during iteration
        with self._lock:
            observers = list(self._subscribers.get(event_name, []))
        # release lock before executing callbacks to avoid blocking other operations
        for _, callback in observers:
            callback(event)

    def lock_current_observers(self):
        """Mark all currently registered observers as protected."""
        with self._lock:
            # shallow copy is enough because tuples and callbacks are immutable references
            self._protected = {k: list(v) for k, v in self._subscribers.items()}

    def unlock_prefix(self, prefix):
        """Remove a prefix from the protected observers."""
        with self._lock:
            for event_name, callbacks in self._protected.items():
                self._protected[event_name] = [(p, c) for (p, c) in callbacks if p != prefix]

    def unlock_callback(self, callback):
        """Remove a specific callback from the protected observers."""
        with self._lock:
            for event_name, callbacks in self._protected.items():
                self._protected[event_name] = [(p, c) for (p, c) in callbacks if c != callback]

    def lock_prefix(self, prefix):
        """Mark all observers with the given prefix as protected (extend existing, avoid duplicates)."""
        with self._lock:
            for event_name, callbacks in self._subscribers.items():
                protected_list = self._protected.setdefault(event_name, [])
                for (p, c) in callbacks:
                    # None and empty ("") prefixes are the same
                    if (p or "") == (prefix or "") and (p, c) not in protected_list:
                        protected_list.append((p, c))

    def unsubscribe_prefix(self, prefix):
        """Remove all callbacks with the given prefix (skip if protected)."""
        with self._lock:
            for event_name, callbacks in list(self._subscribers.items()):
                self._subscribers[event_name] = [
                    (p, c) for (p, c) in callbacks
                    if (p or "") != (prefix or "") or
                       (event_name in self._protected and (p, c) in self._protected[event_name])
                ]

EVENT_PREFIX = "ESL_LIB_EVT_"

class EventType(int):
    """Wrapper for esl_lib_evt_type_t"""

    def __repr__(self) -> str:
        return get_enum(EVENT_PREFIX, self)

    @property
    def name(self) -> str:
        """Normalized event name without prefix, lowercase"""
        return repr(self)[len(EVENT_PREFIX):].lower()

    @classmethod
    def all(cls):
        """Return all known event types as a list of EventType objects"""
        codes = []
        for name, value in vars(elw).items():
            if name.startswith(EVENT_PREFIX):
                codes.append(cls(value))
        return codes

class EventSystemBoot:
    """Wrapper for esl_lib_evt_system_boot_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_SYSTEM_BOOT)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.address = Address.from_ctype(evt_data.evt_boot.address)
        self.radio_id = evt_data.evt_boot.radio_id
        self.status = evt_data.evt_boot.status

    def __repr__(self) -> str:
        return f"{self.evt_code}, {repr(self.address)}, {self.radio_id}, {self.status}"


class EventConnectionMode:
    """Wrapper for esl_lib_evt_connection_mode_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_CONNECTION_MODE)

    def __init__(self, evt_data: elw.esl_lib_evt_connection_mode_t):
        self.mode = evt_data.evt_connection_mode.mode
        self.core_state = evt_data.evt_connection_mode.core_state
        self.filter_size = evt_data.evt_connection_mode.filter_size
        self.connections = evt_data.evt_connection_mode.connections

    def __repr__(self) -> str:
        return f"{self.evt_code}, {get_enum('ESL_LIB_CONNECTION_MODE_', self.mode)}, {get_enum('ESL_LIB_CORE_STATE_',self.core_state)}, {self.filter_size}, {self.connections}"


class EventScanStatus:
    """Wrapper for esl_lib_evt_scan_status_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_SCAN_STATUS)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.enabled = bool(evt_data.evt_scan_status.enabled)
        self.configured = bool(evt_data.evt_scan_status.configured)
        self.parameters = evt_data.evt_scan_status.parameters

    def __repr__(self) -> str:
        return f"{self.evt_code}, {self.enabled}, {self.configured}, {self.parameters.mode}, {self.parameters.interval}, {self.parameters.window}, {self.parameters.scanning_phy}, {self.parameters.discover_mode}"


class EventTagFound:
    """Wrapper for esl_lib_evt_tag_found_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_TAG_FOUND)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.address = Address.from_ctype(evt_data.evt_tag_found.address)
        self.rssi = evt_data.evt_tag_found.rssi

    def __repr__(self) -> str:
        return f"{self.evt_code}, {repr(self.address)}, {self.rssi}"


class EventTagInfo:
    """Wrapper for esl_lib_evt_tag_info_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_TAG_INFO)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.connection_handle = ConnectionHandle(
            evt_data.evt_tag_info.connection_handle
        )
        tlv_data = long_array_to_bytes(evt_data.evt_tag_info.tlv_data)
        self.tlv_data = {}
        tlv_position = 0
        while tlv_position < len(tlv_data):
            tlv = elw.esl_lib_tlv_t.from_buffer_copy(tlv_data, tlv_position)
            tlv_position += ctypes.sizeof(tlv)
            value = tlv_data[tlv_position : tlv_position + tlv.data.len]
            self.tlv_data[tlv.type] = value
            tlv_position += len(value)

    def __repr__(self) -> str:
        tlv_data_str = ", ".join(
            [
                f'{get_enum("ESL_LIB_DATA_TYPE_", key)}: {value.hex()}'
                for key, value in self.tlv_data.items()
            ]
        )
        return f"{self.evt_code}, {self.connection_handle}, {tlv_data_str}"


class EventConfigureTagResponse:
    """Wrapper for esl_lib_evt_configure_tag_response_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_CONFIGURE_TAG_RESPONSE)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.connection_handle = ConnectionHandle(
            evt_data.evt_configure_tag_response.connection_handle
        )
        self.type = evt_data.evt_configure_tag_response.type
        self.status = evt_data.evt_configure_tag_response.status

    def __repr__(self) -> str:
        type_str = get_enum("ESL_LIB_DATA_TYPE_", self.type)
        status_str = get_sl_status_str(self.status)
        return f"{self.evt_code}, {self.connection_handle}, {type_str}, {status_str}"


class EventControlPointResponse:
    """Wrapper for esl_lib_evt_control_point_response_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_CONTROL_POINT_RESPONSE)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.connection_handle = ConnectionHandle(
            evt_data.evt_control_point_response.connection_handle
        )
        self.status = evt_data.evt_control_point_response.status
        self.data_sent = array_to_bytes(evt_data.evt_control_point_response.data_sent)

    def __repr__(self) -> str:
        status_str = get_sl_status_str(self.status)
        return f"{self.evt_code}, {self.connection_handle}, {status_str}, {self.data_sent.hex()}"


class EventControlPointNotification:
    """Wrapper for esl_lib_evt_control_point_notification_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_CONTROL_POINT_NOTIFICATION)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.connection_handle = ConnectionHandle(
            evt_data.evt_control_point_notification.connection_handle
        )
        self.data = array_to_bytes(evt_data.evt_control_point_notification.data)

    def __repr__(self) -> str:
        return f"{self.evt_code}, {self.connection_handle}, {self.data.hex()}"


class EventConnectionOpened:
    """Wrapper for esl_lib_evt_connection_opened_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_CONNECTION_OPENED)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.connection_handle = ConnectionHandle(
            evt_data.evt_connection_opened.connection_handle
        )
        self.address = Address.from_ctype(evt_data.evt_connection_opened.address)
        self.gattdb_handles = elw.esl_lib_gattdb_handles_t.from_buffer_copy(
            evt_data.evt_connection_opened.gattdb_handles
        )
        self.status = evt_data.evt_connection_opened.status

    def __repr__(self) -> str:
        gattdb_str = f"[{hex(self.gattdb_handles.services.esl)}, {hex(self.gattdb_handles.services.ots)}, {hex(self.gattdb_handles.services.dis)}]"
        status_str = get_sl_status_str(self.status)
        return f"{self.evt_code}, {self.connection_handle}, {repr(self.address)}, {status_str}, {gattdb_str}"


class EventConnectionRetry:
    """Wrapper for esl_lib_evt_connection_retry_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_CONNECTION_RETRY)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.connection_handle = ConnectionHandle(
            evt_data.evt_connection_retry.connection_handle
        )
        self.reason = evt_data.evt_connection_retry.reason
        self.connection_state = evt_data.evt_connection_retry.connection_state
        self.address = Address.from_ctype(evt_data.evt_connection_retry.address)
        self.retries_left = evt_data.evt_connection_retry.retries_left

    def __repr__(self) -> str:
        reason_str = get_sl_status_str(self.reason)
        state_str = get_enum("ESL_LIB_CONNECTION_STATE_", self.connection_state)
        return f"{self.evt_code}, {self.connection_handle}, {reason_str}, {state_str}, {repr(self.address)}, {self.retries_left}"


class EventConnectionClosed:
    """Wrapper for esl_lib_evt_connection_closed_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_CONNECTION_CLOSED)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.connection_handle = ConnectionHandle(
            evt_data.evt_connection_closed.connection_handle
        )
        self.address = Address.from_ctype(evt_data.evt_connection_closed.address)
        self.reason = evt_data.evt_connection_closed.reason

    def __repr__(self) -> str:
        reason_str = get_sl_status_str(self.reason)
        return (
            f"{self.evt_code}, {self.connection_handle}, {repr(self.address)}, {reason_str}"
        )


class EventBondingData:
    """Wrapper for esl_lib_evt_bonding_data_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_BONDING_DATA)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.connection_handle = ConnectionHandle(
            evt_data.evt_bonding_data.connection_handle
        )
        self.address = Address.from_ctype(evt_data.evt_bonding_data.address)
        self.ltk = bytes(evt_data.evt_bonding_data.ltk)

    def __repr__(self) -> str:
        return f"{self.evt_code}, {self.connection_handle}, {repr(self.address)}, {self.ltk.hex()}"


class EventBondingFinished:
    """Wrapper for esl_lib_evt_bonding_finished_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_BONDING_FINISHED)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.connection_handle = ConnectionHandle(
            evt_data.evt_bonding_finished.connection_handle
        )
        self.address = Address.from_ctype(evt_data.evt_bonding_finished.address)

    def __repr__(self) -> str:
        return f"{self.evt_code}, {self.connection_handle}, {repr(self.address)}"


class EventImageTransferFinished:
    """Wrapper for esl_lib_evt_image_transfer_finished_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_IMAGE_TRANSFER_FINISHED)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.connection_handle = ConnectionHandle(
            evt_data.evt_image_transfer_finished.connection_handle
        )
        self.img_index = evt_data.evt_image_transfer_finished.img_index
        self.status = evt_data.evt_image_transfer_finished.status

    def __repr__(self) -> str:
        status_str = get_sl_status_str(self.status)
        return (
            f"{self.evt_code}, {self.connection_handle}, {self.img_index}, {status_str}"
        )


class EventImageType:
    """Wrapper for esl_lib_evt_image_type_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_IMAGE_TYPE)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.connection_handle = ConnectionHandle(
            evt_data.evt_image_type.connection_handle
        )
        self.img_index = evt_data.evt_image_type.img_index
        self.type_data = long_array_to_bytes(evt_data.evt_image_type.type_data)

    def __repr__(self) -> str:
        return f"{self.evt_code}, {self.connection_handle}, {self.img_index}, {self.type_data.hex()}"


class EventPawrStatus:
    """Wrapper for esl_lib_evt_pawr_status_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_PAWR_STATUS)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.pawr_handle = evt_data.evt_pawr_status.pawr_handle
        self.status = evt_data.evt_pawr_status.status
        self.configured = bool(evt_data.evt_pawr_status.configured)
        self.config = evt_data.evt_pawr_status.config

    def __repr__(self) -> str:
        state_str = get_enum("ESL_LIB_PAWR_STATE_", self.status)
        return f"{self.evt_code}, {self.pawr_handle:#x}, {state_str}, {self.configured}, {self.config.adv_interval.min}, {self.config.adv_interval.max}, {self.config.subevent.count}, {self.config.subevent.interval}, {self.config.response_slot.delay}, {self.config.response_slot.spacing}, {self.config.response_slot.count}"


class EventPawrConfig(EventPawrStatus):
    """Wrapper for esl_lib_evt_pawr_config_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_PAWR_CONFIG)
    pass


class EventPawrResponse:
    """Wrapper for esl_lib_evt_pawr_response_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_PAWR_RESPONSE)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.pawr_handle = evt_data.evt_pawr_response.pawr_handle
        self.response_slot = evt_data.evt_pawr_response.response_slot
        self.subevent = evt_data.evt_pawr_response.subevent
        self.data = long_array_to_bytes(evt_data.evt_pawr_response.data)

    def __repr__(self) -> str:
        return f"{self.evt_code}, {self.pawr_handle:#08x}, {self.subevent}, {self.response_slot}, {self.data.hex() or 'EMPTY'}"


class EventPawrDataRequest:
    """Wrapper for esl_lib_evt_pawr_data_request_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_PAWR_DATA_REQUEST)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.pawr_handle = evt_data.evt_pawr_data_request.pawr_handle
        self.subevent_start = evt_data.evt_pawr_data_request.subevent_start
        self.subevent_data_count = evt_data.evt_pawr_data_request.subevent_data_count

    def __repr__(self) -> str:
        return f"{self.evt_code}, {self.pawr_handle:#x}, {self.subevent_start}, {self.subevent_data_count}"


class EventShutdownReady:
    """Wrapper for ESL_LIB_EVT_SHUTDOWN_READY"""

    evt_code = EventType(elw.ESL_LIB_EVT_SHUTDOWN_READY)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        pass

    def __repr__(self) -> str:
        return f"{self.evt_code}"


class EventError:
    """Wrapper for esl_lib_evt_error_t"""

    evt_code = EventType(elw.ESL_LIB_EVT_ERROR)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.node_id = get_node_id(evt_data.evt_error.node_id)
        self.lib_status = evt_data.evt_error.lib_status
        self.sl_status = evt_data.evt_error.sl_status
        self.data = evt_data.evt_error.data.core_state

    def __repr__(self) -> str:
        lib_status_str = get_enum("ESL_LIB_STATUS_", self.lib_status)
        sl_status_str = get_sl_status_str(self.sl_status)
        try:
            if isinstance(self.node_id, ConnectionHandle) or isinstance(
                self.node_id, (Address)
            ):
                # Connection handle node ID type
                data_str = get_enum("ESL_LIB_CONNECTION_STATE_", self.data)
            elif isinstance(self.node_id, PAWRSubevent):
                # PAWR handle node ID type
                data_str = get_enum("ESL_LIB_PAWR_STATE_", self.data)
            else:
                data_str = get_enum("ESL_LIB_CORE_STATE_", self.data)
        except KeyError:
            data_str = str(self.data)
        return f"{self.evt_code}, {self.node_id}, {lib_status_str}, {sl_status_str}, {data_str}"


class EventGeneral:
    """Wrapper for general events"""

    evt_code = EventType(elw.ESL_LIB_EVT_GENERAL)

    def __init__(self, evt_data: elw.esl_lib_evt_data_t):
        self.data = long_array_to_bytes(evt_data.evt_general)

    def __repr__(self) -> str:
        return f"{self.evt_code}, {self.data.hex()}"


class CommandFailedError(Exception):
    """ESL Library command failed"""

    def __init__(self, message: str, sl_status=None) -> None:
        self.sl_status = sl_status
        self.message = message

    def __repr__(self) -> str:
        return self.message

    __str__ = __repr__


class Lib:
    """ESL library running in its own process"""

    def __init__(self, config):
        ap_logger.addLogLevel(ap_logger.LEVELS["TRACE"], "TRACE")
        self.config = config
        self.event_queue = mp.Queue()
        self._command_lock = mp.Lock()
        self._conn, conn = mp.Pipe()
        self._process = mp.Process(
            target=self._process_run,
            args=(
                conn,
                ap_logger.stdout,
                ap_logger.level,
            ),
            name="ESL library",
        )
        self._process.daemon = True
        self._process.start()

    def _on_event(
        self,
        evt_code: elw.esl_lib_evt_type_t,
        evt_data: ctypes.POINTER(elw.esl_lib_evt_data_t),
    ):
        """ESL event callback"""
        event = event_factory(evt_code, evt_data.contents)
        self.event_queue.put(event)

    def _process_run(self, conn, stdout, level):
        """Main method of the ESL lib process"""
        ap_logger.stdout = stdout
        ap_logger.level = level
        self.log.debug("started pid: %u", os.getpid())
        stop_event = threading.Event()
        threading.Thread(
            target=self._deserialize_command,
            daemon=True,
            args=(conn, stop_event),
            name="NCP host receiver",
        ).start()
        # Instantiate callback function pointer
        on_event_func = elw.esl_lib_on_event_t(self._on_event)

        def log(level: int, module: str, log: str, file: str, line: int, function: str):
            """Logging callback for ESL lib instance"""
            filter_events = []
            LOG_LEVEL_DICT = {
                elw.ESL_LIB_LOG_LEVEL_DEBUG: self.log.trace,
                elw.ESL_LIB_LOG_LEVEL_INFO: self.log.info,
                elw.ESL_LIB_LOG_LEVEL_WARNING: self.log.warning,
                elw.ESL_LIB_LOG_LEVEL_ERROR: self.log.error,
                elw.ESL_LIB_LOG_LEVEL_CRITICAL: self.log.critical,
            }
            if (
                ap_logger.logLevel() > ap_logger.LEVELS["NOTSET"]
            ):  # Note: The lowest log level becomes extra verbose, completely flooding the CLI!
                filter_events = [
                    b"PAwR response, data status = 255",
                    b", RSSI =",
                    b"characteristic",
                    b"tag info",
                ]
            if not any(flt in log for flt in filter_events):
                with ap_logger.lock:
                    LOG_LEVEL_DICT[level](
                        "[%s] %s in %s:%s()@%d",
                        module,
                        log.rstrip(),
                        file,
                        function,
                        line,
                    )

        log_func = elw.esl_lib_log_callback_t(log)
        try:
            elw.esl_lib_start(self.config, on_event_func, log_func)
        except OSError as e:
            lastlog = ap_logger.getLogger("SYS")
            lastlog.critical(
                f"The {threading.current_thread().name} process terminated abruptly due to {e}"
            )
            os._exit(1)
        finally:
            stop_event.set()
            self.log.debug("terminated pid: %u", os.getpid())

    @property
    def log(self):
        return ap_logger.getLogger("LIB")

    def _deserialize_command(self, conn: Connection, stop_event: threading.Event):
        """Deserialize command and serialize result"""
        while not stop_event.is_set():
            try:
                command, args = conn.recv()
                result = getattr(self, command)(*args)
                conn.send(result)
            except OSError as e:
                lastlog = ap_logger.getLogger("SYS")
                lastlog.critical(
                    f"The {threading.current_thread().name} thread terminated abruptly due to {e}"
                )
                os._exit(1)

    def _serialize_command(self, command, args=None):
        """Serialize command and deserialize result"""
        if args is None:
            # Use empty tuple for functions with void argument list
            args = ()
        with self._command_lock:
            try:
                self._conn.send((command, args))
                result = self._conn.recv()
            except BrokenPipeError as err:
                raise CommandFailedError(
                    "ESL library process terminated unexpectedly"
                ) from err
            if result[0]:
                raise CommandFailedError(
                    f"{command[1:]} failed with result: {get_sl_status_str(result[0])}",
                    result[0],
                )
            return result

    def stop(self, timeout=3):
        """Public wrapper for esl_lib_stop"""
        self._serialize_command("_stop")
        # Wait for the process to terminate
        self._process.join(timeout=timeout)

    def _stop(self):
        """Internal wrapper for esl_lib_stop"""
        status = elw.esl_lib_stop()
        return (status,)

    def scan_configure(
        self,
        active_mode: bool = False,
        interval_ms: float = 10.0,
        window_ms: float = 8.75,
        scanning_phy: int = SCAN_PHY_1M,
        discover_mode: int = SCAN_DISCOVER_GENERIC,
    ):
        """Public wrapper for esl_lib_scan_configure"""
        self._serialize_command(
            "_scan_configure",
            (active_mode, interval_ms, window_ms, scanning_phy, discover_mode),
        )

    def _scan_configure(
        self,
        active_mode: bool = False,
        interval_ms: float = 10.0,
        window_ms: float = 8.75,
        scanning_phy: int = SCAN_PHY_1M,
        discover_mode: int = SCAN_DISCOVER_GENERIC,
    ):
        """Internal wrapper for esl_lib_scan_configure"""
        parameters = elw.esl_lib_scan_parameters_t(
            mode=active_mode,
            interval=round(interval_ms / 0.625),
            window=round(window_ms / 0.625),
            scanning_phy=scanning_phy,
            discover_mode=discover_mode,
        )
        status = elw.esl_lib_scan_configure(byref(parameters))
        return (status,)

    def scan_enable(self, enable: bool = True):
        """Public wrapper for esl_lib_scan_enable"""
        self._serialize_command("_scan_enable", (enable,))

    def _scan_enable(self, enable: bool):
        """Internal wrapper for esl_lib_scan_enable"""
        status = elw.esl_lib_scan_enable(enable)
        return (status,)

    def get_scan_status(self):
        """Public wrapper for esl_lib_get_scan_status"""
        self._serialize_command("_get_scan_status")

    def _get_scan_status(self):
        """Internal wrapper for esl_lib_get_scan_status"""
        status = elw.esl_lib_get_scan_status()
        return (status,)

    def connect(
        self,
        address: Address,
        pawr=None,
        identity: Address = None,
        key_type: int = elw.ESL_LIB_KEY_TYPE_NO_KEY,
        key: bytes = None,
        gattdb=None,
    ):
        """Public wrapper for esl_lib_connect"""
        self._serialize_command(
            "_connect", (address, pawr, identity, key_type, key, gattdb)
        )

    def _connect(
        self,
        address: Address,
        pawr: PAWRSubevent,
        identity: Address,
        key_type: int,
        key: bytes,
        gattdb,
    ):
        """Internal wrapper for esl_lib_connect"""
        c_address = address.to_ctype()
        c_pawr = None
        if pawr is not None:
            c_pawr = pawr.to_ctype()
        c_identity = None
        if identity is not None:
            c_identity = identity.to_ctype()
        c_key = None
        if key is not None:
            c_key = bytes_to_array(key)
        status = elw.esl_lib_connect(
            c_address,
            byref(c_pawr),
            byref(c_identity),
            key_type,
            array_p(c_key),
            byref(gattdb),
        )
        return (status,)

    def close_connection(self, connection_handle):
        """Public wrapper for esl_lib_close_connection"""
        return self._serialize_command("_close_connection", (connection_handle,))[0]

    def _close_connection(self, connection_handle):
        """Internal wrapper for esl_lib_close_connection"""
        status = elw.esl_lib_close_connection(connection_handle)
        return (status,)

    def get_tag_info(self, connection_handle):
        """Public wrapper for esl_lib_get_tag_info"""
        self._serialize_command("_get_tag_info", (connection_handle,))

    def _get_tag_info(self, connection_handle):
        """Internal wrapper for esl_lib_get_tag_info"""
        status = elw.esl_lib_get_tag_info(connection_handle)
        return (status,)

    def configure_tag(self, connection_handle, tlv_data: dict):
        """Public wrapper for esl_lib_configure_tag"""
        data = b""
        for key, value in tlv_data.items():
            data += int(key).to_bytes(
                ctypes.sizeof(elw.esl_lib_data_type_t), sys.byteorder
            )
            data += len(value).to_bytes(ctypes.sizeof(elw.uint32_t), sys.byteorder)
            data += value
        self._serialize_command(
            "_configure_tag", (connection_handle, data)
        )

    def _configure_tag(self, connection_handle, tlv_data: bytes):
        """Internal wrapper for esl_lib_configure_tag"""
        c_tlv_data = bytes_to_long_array(tlv_data)
        status = elw.esl_lib_configure_tag(connection_handle, long_array_p(c_tlv_data))
        return (status,)

    def write_control_point(
        self, connection_handle, data: bytes, att_response: bool = True
    ):
        """Public wrapper for esl_lib_write_control_point"""
        self._serialize_command(
            "_write_control_point", (connection_handle, data, att_response)
        )

    def _write_control_point(self, connection_handle, data: bytes, att_response: bool):
        """Internal wrapper for esl_lib_write_control_point"""
        c_data = bytes_to_array(data)
        status = elw.esl_lib_write_control_point(
            connection_handle, array_p(c_data), att_response
        )
        return (status,)

    def pawr_create(self):
        """Public wrapper for esl_lib_pawr_create"""
        (_, handle_out) = self._serialize_command("_pawr_create")
        return handle_out

    def _pawr_create(self):
        """Internal wrapper for esl_lib_pawr_create"""
        handle_out = elw.esl_lib_pawr_handle_t()
        status = elw.esl_lib_pawr_create(byref(handle_out))
        return (status, handle_out.value)

    def pawr_remove(self, pawr_handle):
        """Public wrapper for esl_lib_pawr_remove"""
        self._serialize_command("_pawr_remove", (pawr_handle,))

    def _pawr_remove(self, pawr_handle):
        """Internal wrapper for esl_lib_pawr_remove"""
        status = elw.esl_lib_pawr_remove(pawr_handle)
        return (status,)

    def pawr_enable(self, pawr_handle, enable: bool = True, advertise: bool = False):
        """Public wrapper for esl_lib_pawr_enable"""
        self._serialize_command("_pawr_enable", (pawr_handle, enable, advertise))

    def _pawr_enable(self, pawr_handle, enable: bool, advertise: bool):
        """Internal wrapper for esl_lib_pawr_enable"""
        status = elw.esl_lib_pawr_enable(pawr_handle, enable, advertise)
        return (status,)

    def pawr_set_data(
        self, pawr_handle, subevent: int, response_slot_max: int, payload: bytes
    ):
        """Public wrapper for esl_lib_pawr_set_data"""
        self._serialize_command(
            "_pawr_set_data", (pawr_handle, subevent, response_slot_max, payload)
        )

    def _pawr_set_data(
        self, pawr_handle, subevent: int, response_slot_max: int, payload: bytes
    ):
        """Internal wrapper for esl_lib_pawr_set_data"""
        c_payload = bytes_to_array(payload)
        status = elw.esl_lib_pawr_set_data(
            pawr_handle, subevent, response_slot_max, array_p(c_payload)
        )
        return (status,)

    def pawr_configure(
        self,
        pawr_handle,
        adv_interval_min=elw.ESL_LIB_PAWR_MIN_INTERVAL_DEFAULT,
        adv_interval_max=elw.ESL_LIB_PAWR_MAX_INTERVAL_DEFAULT,
        subevent_count=elw.ESL_LIB_PAWR_SUBEVENT_COUNT_DEFAULT,
        subevent_interval=elw.ESL_LIB_PAWR_SUBEVENT_INTERVAL_DEFAULT,
        response_slot_delay=elw.ESL_LIB_PAWR_RESPONSE_SLOT_DELAY_DEFAULT,
        response_slot_spacing=elw.ESL_LIB_PAWR_RESPONSE_SLOT_SPACING_DEFAULT,
        response_slot_count=elw.ESL_LIB_PAWR_RESPONSE_SLOT_COUNT_DEFAULT,
    ):
        """Public wrapper for esl_lib_pawr_configure"""
        self._serialize_command(
            "_pawr_configure",
            (
                pawr_handle,
                adv_interval_min,
                adv_interval_max,
                subevent_count,
                subevent_interval,
                response_slot_delay,
                response_slot_spacing,
                response_slot_count,
            ),
        )

    def _pawr_configure(
        self,
        pawr_handle,
        adv_interval_min,
        adv_interval_max,
        subevent_count,
        subevent_interval,
        response_slot_delay,
        response_slot_spacing,
        response_slot_count,
    ):
        """Internal wrapper for esl_lib_pawr_configure"""
        pawr_config = elw.esl_lib_pawr_config_t(
            elw.struct_esl_lib_pawr_config_adv_interval_s(
                adv_interval_min, adv_interval_max
            ),
            elw.struct_esl_lib_pawr_config_subevent_s(
                subevent_count, subevent_interval
            ),
            elw.struct_esl_lib_pawr_config_response_slot_s(
                response_slot_delay, response_slot_spacing, response_slot_count
            ),
        )
        status = elw.esl_lib_pawr_configure(pawr_handle, byref(pawr_config))
        return (status,)

    def get_pawr_status(self, pawr_handle):
        """Public wrapper for esl_lib_get_pawr_status"""
        self._serialize_command("_get_pawr_status", (pawr_handle,))

    def _get_pawr_status(self, pawr_handle):
        """Internal wrapper for esl_lib_get_pawr_status"""
        status = elw.esl_lib_get_pawr_status(pawr_handle)
        return (status,)

    def initiate_past(self, connection_handle, pawr_handle):
        """Public wrapper for esl_lib_initiate_past"""
        self._serialize_command("_initiate_past", (connection_handle, pawr_handle))

    def _initiate_past(self, connection_handle, pawr_handle):
        """Internal wrapper for esl_lib_initiate_past"""
        status = elw.esl_lib_initiate_past(connection_handle, pawr_handle)
        return (status,)

    def write_image(self, connection_handle, img_index, img_data: bytes):
        """Public wrapper for esl_lib_write_image"""
        self._serialize_command(
            "_write_image", (connection_handle, img_index, img_data)
        )

    def _write_image(self, connection_handle, img_index, img_data: bytes):
        """Internal wrapper for esl_lib_write_image"""
        status = elw.esl_lib_write_image(
            connection_handle, img_index, len(img_data), img_data
        )
        return (status,)

    def get_image_type(self, connection_handle, img_index):
        """Public wrapper for esl_lib_get_image_type"""
        self._serialize_command("_get_image_type", (connection_handle, img_index))

    def _get_image_type(self, connection_handle, img_index):
        """Internal wrapper for esl_lib_get_image_type"""
        status = elw.esl_lib_get_image_type(connection_handle, img_index)
        return (status,)

    def set_connection_mode(self, mode: elw.enum_esl_lib_connection_mode_e):
        """Public wrapper for esl_lib_set_connection_mode"""
        self._serialize_command("_set_connection_mode", (mode,))

    def _set_connection_mode(self, mode: elw.enum_esl_lib_connection_mode_e):
        """Internal wrapper for esl_lib_set_connection_mode"""
        status = elw.esl_lib_set_connection_mode(mode)
        return (status,)

    def get_connection_mode(self):
        """Public wrapper for esl_lib_get_connection_mode"""
        self._serialize_command("_get_connection_mode")

    def _get_connection_mode(self):
        """Internal wrapper for esl_lib_get_connection_mode"""
        status = elw.esl_lib_get_connection_mode()
        return (status,)

    def general_command(self, cmd_code, data: bytes = None):
        """Public wrapper for esl_lib_general_cmd"""
        self._serialize_command("_general_command", (cmd_code, data))

    def _general_command(self, cmd_code, data: bytes):
        """Internal wrapper for esl_lib_general_cmd"""
        c_data = bytes_to_long_array(data)
        status = elw.esl_lib_general_cmd(cmd_code, long_array_p(c_data))
        return (status,)
