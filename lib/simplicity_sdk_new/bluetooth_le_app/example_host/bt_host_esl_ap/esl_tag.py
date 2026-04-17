"""
ESL Tag.
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

from collections import namedtuple
from datetime import datetime as dt
from image_converter import XbmConverter
from ap_constants import (
    AUX_SYNC_IND_PDU_MAX_SKIP_COUNT,
    BASIC_STATE_FLAG_SERVICE_NEEDED,
    BASIC_STATE_FLAG_SYNCHRONIZED,
    BASIC_STATE_STRINGS,
    EAD_KEY_MATERIAL_SIZE,
    ESL_DISPLAY_TYPE_FULL_RGB,
    ESL_IMAGE_FLAGS_BIT_FLIP,
    ESL_IMAGE_FLAGS_CROP_FIT,
    ESL_IMAGE_FLAGS_FORMAT_MASK,
    ESL_IMAGE_FLAGS_ROTATE_180,
    ESL_IMAGE_FLAGS_ROTATE_270,
    ESL_IMAGE_FLAGS_ROTATE_90,
    ESL_IMAGE_FLAGS_ROTATION_MASK,
    ESL_WSTK_DISPLAY_TYPE,
    OTS_FEATURES_REQUIRED_MASK,
    SIG_VENDOR_ID_SILABS,
    TLV_OPCODE_FACTORY_RST,
    TLV_OPCODE_UNASSOCIATE,
    TLV_OPCODE_UPDATE_COMPLETE,
    TLV_RESPONSE_BASIC_STATE,
    VENDOR_ID_STRINGS,
)
from ap_config import IOP_TEST, ADVERTISING_TIMEOUT, CONNECTING_TIMEOUT, UNSUCCESSFUL_ONBOARDING_LIMIT
from ap_logger import getLogger
from ap_sensor import SENSOR_INFO_LENGTH_SHORT, SENSOR_INFO_LENGTH_LONG, SENSOR_TYPES
from ap_soft_timer import SoftTimer
import esl_lib_wrapper as elw
import esl_lib

PNP_VENDOR_ID_SOURCE_SIG = 1
DISPLAY_INFO_STRUCT_SIZE = 5


class InvalidTagStateError(Exception):
    """Invalid ESL Tag State error"""


class ImageUpdateFailed(Exception):
    """Image update failed"""


class ImageTypeRequired(Exception):
    """Image update failed"""


class PAwRSyncLostError(Exception):
    """PAwR sync lost occured"""
    def __init__(self, message, esl_id, group_id, ble_address):            
        super().__init__(message)
        self.esl_id = esl_id
        self.group_id = group_id
        self.ble_address = ble_address

class TagState(int):
    """Tag state from the point of view of the AP"""

    IDLE = 0
    CONNECTING = 1
    CONNECTED = 2

    def __str__(self) -> str:
        state_to_str = {
            self.IDLE: "Idle",
            self.CONNECTING: "Connecting",
            self.CONNECTED: "Connected",
        }
        try:
            return state_to_str[self]
        except KeyError:
            return f"Unknown internal state ({self})"


class EslState(int):
    """Tag state according to the ESL Profile specification"""

    UNASSOCIATED = 0
    CONFIGURING = 1
    SYNCHRONIZED = 2
    UPDATING = 3
    UNSYNCHRONIZED = 4

    def __str__(self) -> str:
        state_to_str = {
            self.UNASSOCIATED: "Unassociated",
            self.CONFIGURING: "Configuring",
            self.SYNCHRONIZED: "Synchronized",
            self.UPDATING: "Updating",
            self.UNSYNCHRONIZED: "Unsynchronized",
        }
        try:
            return state_to_str[self]
        except KeyError:
            return f"Unknown ESL state ({self})"


class Tag:
    """ESL Tag"""
    __slots__ = (
        "id",
        "lib",
        "_state",
        "_associated",
        "_full_config_to_write",
        "_state_timestamp",
        "_current_time_last_set",
        "_disconnection_counter",
        "_connection_handle",
        "ble_address",
        "ots_image_type",
        "ots_image_flags",
        "pending_unassociate",
        "_advertising_timer",
        "_connection_timer",
        "last_req_timestamp",
        "last_resp_timestamp",
        "basic_state_flags",
        "gatt_values",
        "gatt_write_values",
        "gattdb_handles",
        "auto_image_count",
        "_blocked",
        "_past_timer",
        "_past_subevents_max",
        "_advertising",
        "raw_image",
        "image_file",
        "rotation",
        "label",
        "xbm_converter",
        "busy",
        "_observers",
    )
    _counter = 0

    def __init__(self, lib: esl_lib.Lib, address: esl_lib.Address, dummy=False):
        self.id = type(self)._counter
        if not dummy:
            type(self)._counter += 1
        self.lib = lib
        self._state = TagState(TagState.IDLE)
        self._associated = False
        self._full_config_to_write = False
        self._state_timestamp = dt.now()
        self._current_time_last_set = dt.now().timestamp()
        self._disconnection_counter = 0
        self._connection_handle = None
        # ESL specific attributes
        self.ble_address = address
        self.ots_image_type = {}
        self.ots_image_flags = {}
        self.pending_unassociate = False
        self._advertising_timer = SoftTimer(
            ADVERTISING_TIMEOUT, self.__advertising_timeout
        )
        self._advertising_timer.daemon = True
        self._connection_timer = SoftTimer(
            CONNECTING_TIMEOUT, self.__connecting_timeout
        )
        self._connection_timer.daemon = True
        self.last_req_timestamp = dt.now().timestamp()
        self.last_resp_timestamp = dt.now().timestamp()
        self.basic_state_flags = 0
        self.gatt_values = {}
        self.gatt_write_values = {}
        self.gattdb_handles = None
        self.auto_image_count = 0
        self.connection_handle = None
        self._blocked = elw.ESL_LIB_STATUS_NO_ERROR
        # PAST timer, Note: interval is re-initialized properly after PA interval is set below
        self._past_timer = SoftTimer(10, self.__past_timeout)
        self._past_timer.daemon = True
        self._past_subevents_max = None
        self._advertising = False
        self.raw_image = None
        self.image_file = None
        self.rotation = None
        self.label = None
        self.xbm_converter = XbmConverter()
        self.busy = False

    # Logger
    @property
    def log(self):
        return getLogger("TAG")

    def get_value_as_bytes(self, key) -> bytes:
        try:
            return self.gatt_values[key]
        except KeyError:
            return None

    def get_value_as_int(self, key):
        try:
            return int.from_bytes(self.gatt_values[key], "little")
        except KeyError:
            return None

    @property
    def advertising(self):
        """Tell if the tag is advertising at the moment"""
        return self._advertising and self.state != TagState.CONNECTING

    @property
    def provisioned(self):
        """Tell if the tag is provisioned"""
        return (
            self._associated
            and self.response_key is not None
            and self.ap_sync_key is not None
            and self.time is not None
        )

    @property
    def was_provisioned(self):
        """Tell if the tag has been ever provisioned before"""
        return self._full_config_to_write

    @property
    def associated(self):
        """Tell if the tag is a new instance"""
        return self.esl_address is not None

    @property
    def synchronized(self):
        """Tell if the tag is associated"""
        return self.basic_state_flags & BASIC_STATE_FLAG_SYNCHRONIZED

    @property
    def blocked(self):
        """Tell if the tag is blocked"""
        return self._blocked

    @property
    def esl_address(self):
        return self.get_value_as_int(elw.ESL_LIB_DATA_TYPE_GATT_ESL_ADDRESS)

    @esl_address.setter
    def esl_address(self, esl_address: int):
        old_address = self.esl_address
        old_esl_state = self.esl_state
        self.gatt_values[elw.ESL_LIB_DATA_TYPE_GATT_ESL_ADDRESS] = esl_address.to_bytes(2, "little")
        if old_address != esl_address:
            self._notify("esl_address", old_address, esl_address)
        new_esl_state = self.esl_state
        if old_esl_state != new_esl_state:
            self._notify("esl_state", old_esl_state, new_esl_state)


    @property
    def esl_id(self):
        if self.esl_address is None:
            return None
        return self.esl_address & 0xFF

    @property
    def group_id(self):
        if self.esl_address is None:
            return None
        return (self.esl_address >> 8) & 0x7F

    @property
    def ap_sync_key(self):
        return self.get_value_as_bytes(elw.ESL_LIB_DATA_TYPE_GATT_AP_SYNC_KEY)

    @property
    def response_key(self):
        return self.get_value_as_bytes(elw.ESL_LIB_DATA_TYPE_GATT_RESPONSE_KEY)

    @property
    def time(self):
        """Current time in milliseconds"""
        return self.get_value_as_int(elw.ESL_LIB_DATA_TYPE_GATT_CURRENT_TIME)

    @property
    def led_type(self):
        """LED types represented on one byte per LED"""
        return self.get_value_as_bytes(elw.ESL_LIB_DATA_TYPE_GATT_LED_INFO)

    @property
    def max_image_index(self):
        return self.get_value_as_int(elw.ESL_LIB_DATA_TYPE_GATT_IMAGE_INFO)

    @property
    def sensor_info(self):
        try:
            value = self.gatt_values[elw.ESL_LIB_DATA_TYPE_GATT_SENSOR_INFO]
        except KeyError:
            return []
        index = 0
        sensor_info = []
        while index < len(value):
            if value[index] == 0:
                sensor_info.append(
                    int.from_bytes(
                        value[index + 1 : index + SENSOR_INFO_LENGTH_SHORT], "little"
                    )
                )
                index += SENSOR_INFO_LENGTH_SHORT
            else:
                sensor_info.append(
                    int.from_bytes(
                        value[index + 1 : index + SENSOR_INFO_LENGTH_LONG], "little"
                    )
                )
                index += SENSOR_INFO_LENGTH_LONG
        return sensor_info

    @property
    def display_info(self):
        try:
            value = self.gatt_values[elw.ESL_LIB_DATA_TYPE_GATT_DISPLAY_INFO]
        except KeyError:
            return []
        if len(value) % DISPLAY_INFO_STRUCT_SIZE != 0:
            self.log.error("Invalid display information")
            return []
        field_index = 0
        display_info = []
        TagDisplayInfo = namedtuple("TagDisplayInfo", ["width", "height", "type"])
        while field_index < len(value):
            display_info.append(
                TagDisplayInfo(
                    int.from_bytes(value[field_index : field_index + 2], "little"),
                    int.from_bytes(value[field_index + 2 : field_index + 4], "little"),
                    value[field_index + 4],
                )
            )
            field_index += DISPLAY_INFO_STRUCT_SIZE
        return display_info

    @property
    def display_count(self):
        return len(self.display_info)

    @property
    def has_image_transfer(self):
        try:
            value = self.gatt_values[elw.ESL_LIB_DATA_TYPE_GATT_OTS_FEATURE]
        except KeyError:
            return False
        # Cut the unnecessary parts of the byte array and use the mask
        # If the OTS Feature of the Tag supports truncation and write opcodes
        # then the Tag has image transfer
        if (
            int.from_bytes(value[0:5], "little") & OTS_FEATURES_REQUIRED_MASK
            == OTS_FEATURES_REQUIRED_MASK
        ):
            return True
        return False

    @property
    def pnp_vendor_id(self):
        try:
            value = self.gatt_values[elw.ESL_LIB_DATA_TYPE_GATT_PNP_ID]
        except KeyError:
            return None
        if value[0] == PNP_VENDOR_ID_SOURCE_SIG:
            return int.from_bytes(value[1:3], "little")
        return None

    @property
    def pnp_product_id(self):
        try:
            value = self.gatt_values[elw.ESL_LIB_DATA_TYPE_GATT_PNP_ID]
        except KeyError:
            return None
        if value[0] == PNP_VENDOR_ID_SOURCE_SIG:
            return int.from_bytes(value[3:5], "little")
        return None

    @property
    def pnp_product_version(self):
        try:
            value = self.gatt_values[elw.ESL_LIB_DATA_TYPE_GATT_PNP_ID]
        except KeyError:
            return None
        if value[0] == PNP_VENDOR_ID_SOURCE_SIG:
            return int.from_bytes(value[5:7], "little")
        return None

    @property
    def serial_number(self):
        return self.get_value_as_bytes(elw.ESL_LIB_DATA_TYPE_GATT_SERIAL_NUMBER)

    @property
    def esl_state(self):
        """ESL state getter"""
        if self.state in [TagState.IDLE, TagState.CONNECTING]:
            if not self.associated:
                return EslState(EslState.UNASSOCIATED)
            else:
                if not self.synchronized:
                    return EslState(EslState.UNSYNCHRONIZED)
                else:
                    return EslState(EslState.SYNCHRONIZED)
        else:
            if not self.provisioned:
                return EslState(EslState.CONFIGURING)
            else:
                return EslState(EslState.UPDATING)

    @property
    def state(self):
        """Tag state getter"""
        return self._state

    @state.setter
    def state(self, value: TagState):
        """Connection state setter - for class internal use, only!"""
        if self._state != value:
            now = dt.now()
            new_state = TagState(value)
            self.log.debug(
                "[%s] Tag state transition: %s -> %s, time spent in state: %s",
                self.ble_address,
                self._state,
                new_state,
                now - self._state_timestamp,
            )
            self._state_timestamp = now
            previous_state = self._state
            self._state = new_state
            if (new_state == TagState.IDLE):
                self.limit_connection_retries()
            self._notify("state", previous_state, new_state)

    @property
    def connection_handle(self):
        """Connection handle getter"""
        return self._connection_handle

    @property
    def past_initiated(self):
        """PAST procedure progress monitor"""
        return self._past_subevents_max is not None

    @connection_handle.setter
    def connection_handle(self, value):
        """Connection handle setter - for class internal use, only!"""
        # Setting the connection handle implicitly changes the connection status accordingly.
        if value is None:
            self.state = TagState.IDLE
        else:
            self.state = TagState.CONNECTED
        previous_handle = self._connection_handle
        self._connection_handle = value
        if previous_handle != value: # notify if changed
            self._notify("connection_handle", previous_handle, value)

    def __lt__(self, other):
        if None not in (self.esl_address, other.esl_address):
            return self.esl_address < other.esl_address
        return False

    def add_observer(self, callback):
        if not hasattr(self, "_observers"):
            self._observers = []
        self._observers.append(callback)

    def remove_observer(self, callback):
        if hasattr(self, "_observers"):
            self._observers.remove(callback)

    def _notify(self, field, old, new):
        if hasattr(self, "_observers"):
            for cb in self._observers:
                cb(self, field, old, new)

    def reset(self, keep_esl_address=False):
        """Reset object states"""
        old_esl_state = self.esl_state
        keys = [
            elw.ESL_LIB_DATA_TYPE_GATT_AP_SYNC_KEY,
            elw.ESL_LIB_DATA_TYPE_GATT_RESPONSE_KEY,
            elw.ESL_LIB_DATA_TYPE_GATT_CURRENT_TIME,
            elw.ESL_LIB_DATA_TYPE_GATT_ESL_ADDRESS,
        ]
        self.gattdb_handles = None
        self.pending_unassociate = False
        self.update_response_timestamp()
        # Modify only the SYNCHRONIZED bit without invoking the __update_flags(),
        # because that might cause an immediate esl_state change notification,
        # which will be handled separately.
        self.basic_state_flags = self.basic_state_flags & ~BASIC_STATE_FLAG_SYNCHRONIZED
        self.gatt_write_values = {
            key: value for key, value in self.gatt_values.items() if key in keys
        }
        self._full_config_to_write = bool(len(self.gatt_write_values) == len(keys))
        # Keep ESL Address if exists and the previous config has been finished successfully, delete the rest - also make the ESL object invalid for possible future re-discovery
        self.gatt_values = {
            key: value
            for key, value in self.gatt_values.items()
            if key == elw.ESL_LIB_DATA_TYPE_GATT_ESL_ADDRESS
            and (self._full_config_to_write or keep_esl_address)
        }
        if len(self.gatt_values) == 0:
            # Reset the image counter also in this corner case
            self.auto_image_count = 0
        # Reset timers
        self.reset_advertising()
        if self._past_timer.is_alive():
            self._past_timer.cancel()
        if self._connection_timer.is_alive():
            self._connection_timer.cancel()
        # Reset busy state
        self.busy = False
        self.connection_handle = None
        self._past_subevents_max = None
        self._associated = False
        new_esl_state = self.esl_state
        if new_esl_state != old_esl_state:
            self._notify("esl_state", old_esl_state, new_esl_state)

    def block(self, lib_status=elw.ESL_LIB_STATUS_UNSPECIFIED_ERROR):
        """Set blocked state if not set already"""
        if self._blocked == elw.ESL_LIB_STATUS_NO_ERROR:
            self.update_request_timestamp()
            self._blocked = lib_status

    def unblock(self):
        """Release from blocked state"""
        self._blocked = elw.ESL_LIB_STATUS_NO_ERROR
        self._disconnection_counter = 0

    def reset_advertising(self):
        """Enable re-discovery of an already known (reported) tag"""
        if self._advertising_timer.is_alive():
            self._advertising_timer.cancel()
        self._advertising = False

    def start_advertising_governor(self):
        """(Re)start a governor timeout for an advertising tag"""
        self._advertising_timer.restart(ADVERTISING_TIMEOUT)

    def unassociate(self):
        """Unassociate tag object"""
        self.gatt_values = {}
        self.reset()

    def unsynchronize(self):
        """Clear the BASIC_STATE_FLAG_SYNCHRONIZED flag internally to consider a tag unsynced"""
        # while clearing the bit it is still possible that the tag is actually synced so doing this allows it to recover silently
        self.__update_flags(BASIC_STATE_FLAG_SYNCHRONIZED, False)

    def limit_connection_retries(self):
        """Limit disconnection retries of unprovisioned tags"""
        if not self.provisioned:
            self._disconnection_counter += 1
            if self._disconnection_counter >= UNSUCCESSFUL_ONBOARDING_LIMIT:
                self.block(elw.ESL_LIB_STATUS_CONN_CONFIG_FAILED)
                self.log.error(
                    "Tag at address %s blocked due to exceeding maximum connection retries (%d)",
                    self.ble_address,
                    UNSUCCESSFUL_ONBOARDING_LIMIT,
                )
        else:
            self._disconnection_counter = 0

    def handle_response(self, data):
        """Handle TLV response"""
        # Basic state
        if data[0] == TLV_RESPONSE_BASIC_STATE:
            bs_bitmap = int.from_bytes(data[1:], byteorder="little")
            self.__update_flags(bs_bitmap, None)
        self.update_response_timestamp()

    def update_timestamps(self, timestamp=None):
        """Update both request and response timestamps"""
        self.update_request_timestamp(timestamp)
        self.update_response_timestamp(timestamp)

    def update_request_timestamp(self, timestamp=None):
        """Update last request timestamp"""
        if timestamp is None:
            timestamp = dt.now().timestamp()
        self.last_req_timestamp = timestamp

    def update_response_timestamp(self, timestamp=None):
        """Update last request timestamp"""
        if timestamp is None:
            timestamp = dt.now().timestamp()
        self.last_resp_timestamp = timestamp

    def timestamps_diff(self, timestamp):
        """Get timestamp difference"""
        return timestamp - self.last_req_timestamp, timestamp - self.last_resp_timestamp

    def __update_flags(self, flags, new_state=True):
        if flags is not None:
            old_esl_state = self.esl_state
            if new_state == True:
                self.basic_state_flags = self.basic_state_flags | flags
            elif new_state == False:
                self.basic_state_flags = self.basic_state_flags & ~flags
            else:
                self.basic_state_flags = (
                    flags  # raw data update from PAwR / ESL CP response
                )

            if not self.synchronized and not self.blocked:
                self.update_request_timestamp()

            if flags & BASIC_STATE_FLAG_SERVICE_NEEDED:
                self.log.warning(
                    "ESL ID %d in group %d needs attention, Service Needed flag is active!",
                    self.esl_id,
                    self.group_id,
                )
            new_esl_state = self.esl_state
            if old_esl_state != new_esl_state:
                self._notify("esl_state", old_esl_state, new_esl_state)

    def find_type_matching_display_index(self, display_type):
        for x in self.display_info:
            if x.type == display_type:
                return self.display_info.index(x)
        return None

    def get_info(self):
        """Get tag information"""
        justify_column = 17
        info = f"{'BLE Address:':{justify_column}}{self.ble_address}"
        info += f"\n{'ESL Address:':{justify_column}}"
        if self.esl_address is not None:
            info += f"{self.esl_address} ({self.esl_address:#06x})"
            info += f"\n{'ESL ID:':{justify_column}}{self.esl_id}"
            info += f"\n{'Group ID:':{justify_column}}{self.group_id}"
        else:
            info += "Not configured"
        info += f"\n{'AP sync key:':{justify_column}}"
        if self.ap_sync_key is not None:
            info += "**** (Set)" if not IOP_TEST else "0x" + self.ap_sync_key.hex()
        else:
            info += "Not set"
        info += f"\n{'Response key:':{justify_column}}"
        if self.response_key is not None:
            info += "**** (Set)" if not IOP_TEST else "0x" + self.response_key.hex()
        else:
            info += "Not set"
        info += f"\n{'Absolute Time:':{justify_column}}"
        if self.time is not None:
            info += f"Last time set to {self.time}"
        else:
            info += "Out of sync"

        info += f"\n{'Display:':{justify_column}}"
        if self.display_info is not None:
            items = []
            for i, display in enumerate(self.display_info):
                items.append(
                    f"[{i}] width: {display.width} height: {display.height} type: {display.type}"
                )
            info += ("\n" + " " * justify_column).join(items)
        else:
            info += "Not present"

        if self.max_image_index is not None:
            info += f"\n{'Max image index:':{justify_column}}" + str(
                self.max_image_index
            )
        else:
            info += "\n" + "No image support."

        info += f"\n{'Sensors:':{justify_column}}"
        if len(self.sensor_info) > 0:
            items = []
            for i, sensor in enumerate(self.sensor_info):
                items.append(
                    f"[{i}] {SENSOR_TYPES[sensor].desc if sensor in SENSOR_TYPES else hex(sensor)}"
                )
            info += ("\n" + " " * justify_column).join(items)
        else:
            info += "Not supported"

        info += f"\n{'LED Info:':{justify_column}}"
        if self.led_type is not None:
            items = []
            for i, b in enumerate(self.led_type):
                bin_num = bin(b)[2:].zfill(8)
                R, G, B = (
                    int(bin_num[2:4], 2),
                    int(bin_num[4:6], 2),
                    int(bin_num[6:8], 2),
                )
                if bin_num[0:2] == "00":
                    items.append(f"[{i}] Colored, last set color: RGB({R}{G}{B})")
                elif bin_num[0:2] == "01":
                    items.append(f"[{i}] Monochrome, closest color: RGB({R}{G}{B})")
                else:
                    items.append(f"[{i}] RFU")
            info += ("\n" + " " * justify_column).join(items)
        else:
            info += "Not available"

        info += f"\n{'PnP Info:':{justify_column}}"
        if self.pnp_vendor_id is not None:
            if self.pnp_vendor_id in VENDOR_ID_STRINGS:
                info += f"Vendor: {VENDOR_ID_STRINGS[self.pnp_vendor_id]}"
            else:
                info += f"Vendor_ID: {self.pnp_vendor_id:#x}"
            info += f" Product_ID: {self.pnp_product_id:#x} Product_version: {self.pnp_product_version:#x}"
        else:
            info += "Not available"

        if self.serial_number is not None:
            info += f"\n{'Serial Number:':{justify_column}}"
            info += str(self.serial_number)

        info += f"\n{'Last status:':{justify_column}}"
        bs_string = ", ".join(
            [
                value
                for key, value in BASIC_STATE_STRINGS.items()
                if self.basic_state_flags & key
            ]
        )
        if len(bs_string) == 0:
            bs_string = "No Basic State flag is set"
        info += f"{bs_string} ({self.basic_state_flags:#06x}) received at {dt.fromtimestamp(self.last_resp_timestamp, tz=None).strftime('%d/%b %H:%M:%S.%f')[:-3]}"
        return info

    def __str__(self):
        if self.esl_address is None:
            return f"BLE Address: {self.ble_address}, Unassociated"
        return f"BLE Address: {self.ble_address}, ESL ID {self.esl_id} in group {self.group_id} ({self.esl_address:#06x})"

    def handle_event(self, evt):
        """Handle event"""
        if isinstance(evt, esl_lib.EventConnectionOpened):
            if evt.address == self.ble_address:
                if self._connection_timer.is_alive():
                    self._connection_timer.cancel()
                self.reset_advertising()
                self._past_timer.cancel()
                self._past_subevents_max = None
                self.connection_handle = evt.connection_handle
                self.gattdb_handles = evt.gattdb_handles
                self.pending_unassociate = False
                if evt.status != elw.SL_STATUS_OK:
                    self.log.error(
                        "Tag at address %s connected with failure: %s",
                        self.ble_address,
                        esl_lib.get_sl_status_str(evt.status),
                    )
                    return
                if not self.provisioned:
                    self.log.info(
                        "Reading tag information from address %s", self.ble_address
                    )
                    self.get_tag_info()
                else:
                    self.log.info(
                        "Tag info already available, skipping discovery for %s",
                        self.ble_address,
                    )

                if self.esl_address is None:
                    self.log.info(
                        "Registering ESL Tag at BLE address: %s", self.ble_address
                    )
                if self.provisioned:
                    self.log.info(
                        "Already known Tag at BLE address: %s", self.ble_address
                    )
        elif isinstance(evt, esl_lib.EventBondingFinished):
            if self.blocked:
                self.unblock()  # clear the blocked state if it has been connected and bonded - this happened certainly manually
                self.block(
                    elw.ESL_LIB_STATUS_CONN_CONFIG_FAILED
                )  # change the reason until at least ESL Address is set
        elif isinstance(evt, esl_lib.EventConnectionRetry):
            self.state = TagState.CONNECTING
            self._connection_timer.restart()
            self.reset_advertising()  # will stop the advertiser watchdog, if there's any
            if self.esl_state != EslState.SYNCHRONIZED: # If it's synchronized, it's not advertising, and if it's advertising, it's not considered synchronized
                self._advertising = True  # necessary step for any connect requests to undetected advertisers!
            self.log.warning(
                "Reconnect to %s addr., reason: %s, %s, %d more left",
                self.ble_address,
                esl_lib.get_enum("ESL_LIB_CONNECTION_STATE_", evt.connection_state),
                esl_lib.get_sl_status_str(evt.reason),
                evt.retries_left,
            )
        elif isinstance(evt, esl_lib.EventConnectionClosed):
            if (
                evt.connection_handle == self.connection_handle
                or evt.address == self.ble_address
            ):
                self._past_timer.cancel()
                self.connection_handle = None
                if evt.reason == elw.SL_STATUS_BT_CTRL_REMOTE_USER_TERMINATED:
                    if self.provisioned and not self.pending_unassociate and self.past_initiated and self._past_subevents_max > self.group_id:
                        self.__update_flags(BASIC_STATE_FLAG_SYNCHRONIZED)
                    else:
                        self.__update_flags(BASIC_STATE_FLAG_SYNCHRONIZED, False)
                    self.update_timestamps()
                elif (
                    evt.reason
                    == elw.SL_STATUS_BT_CTRL_CONNECTION_TERMINATED_BY_LOCAL_HOST
                ):
                    self.__update_flags(BASIC_STATE_FLAG_SYNCHRONIZED, False)
                self._past_subevents_max = None
                self.log.info(
                    "Connection to %s closed with reason %s",
                    self.ble_address,
                    esl_lib.get_sl_status_str(evt.reason),
                )
        elif isinstance(evt, esl_lib.EventTagInfo):
            if evt.connection_handle == self.connection_handle:
                for ix, (tlv, value) in enumerate(evt.tlv_data.items()):
                    if (
                        len(value) == 0
                    ):  # According to the ESL Service Specification, none of the ESL Information Characteristics can be valid without data!
                        self.block(elw.ESL_LIB_STATUS_CONN_DISCOVERY_FAILED)
                        self.log.error(
                            "Tag at address %s blocked due to invalid zero length characteristic data for TLV type %d at data index %d",
                            self.ble_address,
                            tlv,
                            ix,
                        )
                        self.close_connection(force_close=True)
                self.gatt_values.update(evt.tlv_data)
                if elw.ESL_LIB_DATA_TYPE_GATT_PNP_ID in evt.tlv_data:
                    if self.pnp_vendor_id is None:
                        self.log.error(
                            "PnP characteristic not found - vendor opcodes support disabled"
                        )
                    elif self.pnp_vendor_id == SIG_VENDOR_ID_SILABS:
                        self.log.info(
                            "Silabs ESL found - one vendor opcode is allocated for PAwR interval skipping: 0x1Fnn"
                        )
                    else:
                        self.log.info(
                            "PnP characteristic '0x%02x' found for %s",
                            self.pnp_vendor_id,
                            self.ble_address,
                        )
                if elw.ESL_LIB_DATA_TYPE_GATT_SERIAL_NUMBER in evt.tlv_data:
                    self.log.info(
                        "Serial Number String '%s' present on %s",
                        str(self.serial_number),
                        self.ble_address,
                    )
                values = {}
                # Perform sanity check on mandatory GATT service (OTS) when ESL Image Information characteristic is reported
                if (
                    elw.ESL_LIB_DATA_TYPE_GATT_IMAGE_INFO in evt.tlv_data
                    and self.gattdb_handles.services.ots
                    == elw.ESL_LIB_INVALID_SERVICE_HANDLE
                ):
                    self.block(elw.ESL_LIB_STATUS_FEATURE_NOT_SUPPORTED)
                    self.log.error(
                        "Tag at address %s blocked due to missing mandatory GATT Service for image transfer!",
                        self.ble_address,
                    )
                    self.close_connection(force_close=True)
                else:
                    values = self.gatt_write_values
                self.busy = False
                if self.was_provisioned and self.associated:
                    if (
                        elw.ESL_LIB_DATA_TYPE_GATT_CURRENT_TIME
                        in self.gatt_write_values
                    ):
                        # compensate the absolute time value if it's present
                        absolute_time = int.from_bytes(
                            self.gatt_write_values[
                                elw.ESL_LIB_DATA_TYPE_GATT_CURRENT_TIME
                            ],
                            "little",
                        )
                        current_timestamp = dt.now().timestamp()
                        time_diff_ms = int(
                            1000 * (current_timestamp - self._current_time_last_set)
                        )
                        absolute_time += time_diff_ms
                        self.gatt_write_values[
                            elw.ESL_LIB_DATA_TYPE_GATT_CURRENT_TIME
                        ] = absolute_time.to_bytes(4, "little")
                    self.configure_tag(values)
        elif isinstance(evt, esl_lib.EventConfigureTagResponse):
            if evt.connection_handle == self.connection_handle:
                if evt.status == elw.SL_STATUS_OK:
                    self.gatt_values[evt.type] = self.gatt_write_values[evt.type]
                else:
                    self.log.error(
                        "Tag configuration failed for %s at addres %s with result %s!",
                        esl_lib.get_enum("ESL_LIB_DATA_TYPE_", evt.type),
                        self.ble_address,
                        esl_lib.get_sl_status_str(evt.status),
                    )
                old_esl_state = self.esl_state
                if evt.type == elw.ESL_LIB_DATA_TYPE_GATT_ESL_ADDRESS:
                    if evt.status == elw.SL_STATUS_OK:
                        self._associated = True
                        self.unblock()  # clear previous blocked state if the Tag becomes associated
                    else:
                        self._associated = False

                if self.provisioned:
                    self.log.info(
                        "Tag fully provisioned at address %s as ESL ID %d in group %d.",
                        self.ble_address,
                        self.esl_id,
                        self.group_id,
                    )
                new_esl_state = self.esl_state
                self.busy = False
                if new_esl_state != old_esl_state:
                    self._notify("esl_state", old_esl_state, new_esl_state)

        elif isinstance(evt, esl_lib.EventImageType):
            if evt.connection_handle == self.connection_handle:
                self.log.debug("Image type UUID received: %s", evt.type_data.hex())
                self.ots_image_flags[evt.img_index] = evt.type_data[2]
                self.ots_image_type[evt.img_index] = evt.type_data[3]
                self.busy = False
        elif isinstance(evt, esl_lib.EventImageTransferFinished):
            if evt.connection_handle == self.connection_handle:
                if (
                    evt.status == elw.SL_STATUS_OK
                    and evt.img_index > self.auto_image_count
                ):
                    self.auto_image_count = evt.img_index
                self.log.info(
                    "Image %d sent to device at address %s with result 0x%x",
                    evt.img_index,
                    self.ble_address,
                    evt.status,
                )
                self.busy = False
        elif isinstance(evt, esl_lib.EventControlPointResponse):
            if evt.connection_handle == self.connection_handle:
                self.log.info(
                    "Command: %s written successfully for %s",
                    evt.data_sent.hex(),
                    self.ble_address,
                )
                self.busy = False
                if evt.data_sent[0] == TLV_OPCODE_FACTORY_RST:
                    self.reset()
                    self.pending_unassociate = True  # need to revert pending_unassociate until AP core processes the same event!
                elif evt.data_sent[0] == TLV_OPCODE_UPDATE_COMPLETE and self.blocked:
                    self.unblock()  # clear the blocked state for any tag to which the update complete command is sent
        elif isinstance(evt, esl_lib.EventControlPointNotification):
            if evt.connection_handle == self.connection_handle:
                self.handle_response(evt.data)
        elif isinstance(evt, esl_lib.EventTagFound):
            if evt.address == self.ble_address:
                if self.state == TagState.IDLE:
                    if (
                        not self._advertising
                    ):  # note the internal check, not the property! (To print following message only once)
                        self.log.info(
                            "ESL service found at BLE address: %s with RSSI: %d dBm",
                            self.ble_address,
                            evt.rssi,
                        )
                    self._advertising = True  # setting this has to precede self.esl_state == EslState.SYNCHRONIZED check!
                if self.advertising:
                    if self.esl_state == EslState.SYNCHRONIZED:
                        self.reset()  # reset will clear _advertising state, too
                        self._advertising = True  # set _advertising back - since it is indeed advertising
                        raise PAwRSyncLostError("ESL sync lost!", self.esl_id, self.group_id, self.ble_address)
                    self.start_advertising_governor()
        elif isinstance(evt, esl_lib.EventError):
            self.reset_advertising()
            if evt.lib_status == elw.ESL_LIB_STATUS_BONDING_FAILED:
                self.state = TagState.IDLE
            elif evt.lib_status == elw.ESL_LIB_STATUS_CONN_SUBSCRIBE_FAILED:
                if (
                    evt.sl_status
                    == elw.SL_STATUS_BT_ATT_CLIENT_CHARACTERISTIC_CONFIGURATION_DESCRIPTOR_IMPROPERLY_CONFIGURED
                    and not self.blocked
                ):
                    self.block(evt.lib_status)
                    self.log.error(
                        "Tag at address %s blocked due to violation of ESL Profile / Service specification!",
                        self.ble_address,
                    )
                else:
                    self.log.warning(
                        "Failed subscription attempt to the ESL Control Point at address %s - connection will be terminated!",
                        self.ble_address,
                    )
            elif evt.lib_status == elw.ESL_LIB_STATUS_CONN_FAILED:
                if evt.sl_status is not elw.SL_STATUS_ALREADY_EXISTS:
                    self.connection_handle = None
                self.state = TagState.IDLE
                if (
                    evt.sl_status
                    in [
                        elw.SL_STATUS_NO_MORE_RESOURCE,
                        elw.SL_STATUS_BT_CTRL_CONNECTION_LIMIT_EXCEEDED,
                    ]
                    and not self.advertising
                ):
                    self._advertising = True  # set _advertising back - since it should advertising - to prevent re-report of already known tag
                    self.start_advertising_governor()  # the advertising governor will take care of it if it does not advertise as we expect
            elif evt.lib_status == elw.ESL_LIB_STATUS_CONN_CLOSE_FAILED:
                self.connection_handle = None
                self.busy = False
                if (
                    evt.sl_status == elw.SL_STATUS_TIMEOUT
                    and evt.data
                    in [
                        elw.ESL_LIB_CONNECTION_STATE_PAST_CLOSE_CONNECTION,
                        elw.ESL_LIB_CONNECTION_STATE_PAST_INIT,
                    ]
                    and self._past_timer.is_alive()
                ):
                    self._past_timer.cancel()
                self._past_subevents_max = None
                self.state = TagState.IDLE
            elif evt.lib_status == elw.ESL_LIB_STATUS_CONN_TIMEOUT:
                if self._past_timer.is_alive():
                    self._past_timer.cancel()
                self._past_subevents_max = None
                if evt.data in [
                    elw.ESL_LIB_CONNECTION_STATE_PAST_CLOSE_CONNECTION,
                    elw.ESL_LIB_CONNECTION_STATE_PAST_INIT,
                ]:
                    self.log.warning(
                        "Tag at address %s failed to sync!", self.ble_address
                    )
                    self.close_connection(force_close=True)
                    if not self.busy:
                        self.connection_handle = None
                        self.state = TagState.IDLE
                else:
                    self.connection_handle = None
            elif evt.lib_status == elw.ESL_LIB_STATUS_OTS_GOTO_FAILED:
                if evt.sl_status == elw.SL_STATUS_NOT_FOUND:
                    self.log.error(
                        "No object found with the requested Object ID for address %s",
                        self.ble_address,
                    )
                self.busy = False
            elif evt.lib_status == elw.ESL_LIB_STATUS_PAST_INIT_FAILED:
                self.busy = False
                self._past_subevents_max = None
                if self._past_timer.is_alive():
                    self._past_timer.cancel()
                if evt.sl_status in [
                    elw.SL_STATUS_BT_CTRL_COMMAND_DISALLOWED,
                    elw.SL_STATUS_INVALID_PARAMETER,
                ]:
                    self.log.info(
                        "PAST skipped for address %s by ESL already in Synchronized state.",
                        self.ble_address,
                    )
                else:
                    self.log.error(
                        "PAST was unsuccesssful, force closing connection to tag at address %s",
                        self.ble_address,
                    )
                    self.close_connection(force_close=True)
            elif evt.lib_status == elw.ESL_LIB_STATUS_OTS_INIT_FAILED:
                self.auto_image_count = 0

    def __connecting_timeout(self):
        if self.state == TagState.CONNECTING:
            self.log.warning(
                "Connection request to address %s expired!", self.ble_address
            )
            self._advertising = False  # make the tag discoverable by advertising again
            self.state = TagState.IDLE

    def __advertising_timeout(self):
        if self.advertising:
            self.log.warning(
                "Advertisements from address %s are no longer received!",
                self.ble_address,
            )
        self._advertising = False

    def __past_timeout(self):
        """Called on PAST timeout"""
        self.log.warning("PAST timeout for address %s, force close!", self.ble_address)
        try:
            self.close_connection(force_close=True)
        except Exception as e:
            self.log.error(e)

    def connect(
        self,
        pawr=None,
        identity: esl_lib.Address = None,
        key_type: int = elw.ESL_LIB_KEY_TYPE_NO_KEY,
        key: bytes = None,
        timeout: int = None,
    ):
        """Connect to the tag"""
        if self.state != TagState.IDLE:
            raise InvalidTagStateError(
                f"Invalid ESL object state: {self._state} at address {self.ble_address}"
            )
        try:
            self.lib.connect(
                address=self.ble_address,
                pawr=pawr,
                identity=identity,
                key_type=key_type,
                key=key,
                gattdb=self.gattdb_handles,
            )
            self._connection_handle = None  # silent but forced reset of handle
            self.state = TagState.CONNECTING
            self.reset_advertising()  # will stop timer thread immediately, if there's any!
            if pawr is None:
                self._advertising = True  # necessary step for any connect requests to undetected advertisers!
            self._connection_timer.restart(timeout)
        except Exception as e:
            self.log.error(e)

    def close_connection(self, force_close=False):
        """Disconnect from the tag"""
        if self.state != TagState.CONNECTED and not force_close:
            raise InvalidTagStateError(
                f"Invalid ESL object state: {self._state} at address {self.ble_address}"
            )
        self._past_subevents_max = None
        try:
            if force_close:
                self.log.debug("Abort connection to ESL at %s.", self.ble_address)
            status = self.lib.close_connection(self.connection_handle)
            self.busy = (status == elw.SL_STATUS_OK)
            if not self.busy:
                self.connection_handle = None
                self.reset_advertising()
        except esl_lib.CommandFailedError as e:
            self.log.error(e)

    def get_tag_info(self):
        """Get tag info"""
        if self.state != TagState.CONNECTED:
            raise InvalidTagStateError(
                f"Invalid ESL object state: {self._state} at address {self.ble_address}"
            )
        try:
            self.lib.get_tag_info(self.connection_handle)
            self.busy = True
        except esl_lib.CommandFailedError as e:
            self.log.error(e)

    def configure_tag(self, tlv_data: dict):
        """Configure tag using TLVs"""
        if self.state != TagState.CONNECTED:
            raise InvalidTagStateError(
                f"Invalid ESL object state: {self._state} at address {self.ble_address}"
            )
        # Clear anything previously configured but ESL Address before to be (re)written
        for key in tlv_data:
            if key != elw.ESL_LIB_DATA_TYPE_GATT_ESL_ADDRESS:
                if key == elw.ESL_LIB_DATA_TYPE_GATT_CURRENT_TIME:
                    self._current_time_last_set = dt.now().timestamp()
                try:
                    self.gatt_values.pop(key)
                except KeyError:
                    pass

        self.gatt_write_values.update(tlv_data)
        try:
            self.lib.configure_tag(self.connection_handle, tlv_data)
            self.busy = True
        except esl_lib.CommandFailedError as e:
            self.log.error(e)

    def response_key_update(self, response_key_material = bytes):
        """Update response key material of an already configured Tag"""
        if len(response_key_material) == EAD_KEY_MATERIAL_SIZE and elw.ESL_LIB_DATA_TYPE_GATT_RESPONSE_KEY in self.gatt_write_values:
            self.log.trace("Response key material change request accepted.")
            self.gatt_write_values[elw.ESL_LIB_DATA_TYPE_GATT_RESPONSE_KEY] = response_key_material
        else:
            self.log.warning("Response key material change request ignored because condition(s) not met!")

    def write_control_point(self, data: bytes, att_response: bool = True):
        """Write ESL Control Point"""
        if self.state != TagState.CONNECTED:
            raise InvalidTagStateError(
                f"Invalid ESL object state: {self._state} at address {self.ble_address}"
            )
        factory_reset = data[0] == TLV_OPCODE_FACTORY_RST
        if (data[0] == TLV_OPCODE_UNASSOCIATE or factory_reset) and data[
            1
        ] == self.esl_id:
            self.pending_unassociate = True
        try:
            self.lib.write_control_point(
                self.connection_handle, data, (att_response or factory_reset)
            )
            self.busy = True
        except esl_lib.CommandFailedError as e:
            self.log.error(e)

    def write_image(self, img_index, img_data: bytes):
        """Write image to the tag"""
        if self.state != TagState.CONNECTED:
            raise InvalidTagStateError(
                f"Invalid ESL object state: {self._state} at address {self.ble_address}"
            )
        try:
            self.lib.write_image(self.connection_handle, img_index, img_data)
            self.busy = True
        except esl_lib.CommandFailedError as e:
            self.log.error(e)

    def get_image_type(self, img_index):
        """Get image type"""
        if self.state != TagState.CONNECTED:
            raise InvalidTagStateError(
                f"Invalid ESL object state: {self._state} at address {self.ble_address}"
            )
        try:
            self.lib.get_image_type(self.connection_handle, img_index)
            self.busy = True
        except esl_lib.CommandFailedError as e:
            self.log.error(e)

    def image_update(
        self,
        image_index,
        file,
        raw=False,
        display_ind=None,
        label=None,
        rotation=None,
        cropfit=False,
    ):
        """Update image"""
        if self.state != TagState.CONNECTED:
            raise InvalidTagStateError(
                f"Invalid ESL object state: {self._state} at address {self.ble_address}"
            )

        if raw:
            self.log.info("Raw image file opened: %s", file)
            self.raw_image = open(file, "rb").read()
            # nothing to do with raw files except upload!
        else:
            # Open and convert image file, otherwise
            ots_object_type = None
            if display_ind is None:
                self.image_file = file
                self.rotation = rotation
                self.label = label
                try:
                    ots_object_type = self.ots_image_type[image_index]
                except KeyError:
                    ots_object_type = None
                if ots_object_type == "unknown":
                    self.log.error("Unable to read ots object type")
                elif ots_object_type == None:
                    self.get_image_type(image_index)
                    raise ImageTypeRequired(
                        "Image type required for address %s, image: %d",
                        self.ble_address,
                        image_index,
                    )
                else:
                    display_ind = self.find_type_matching_display_index(ots_object_type)
                    if display_ind is None:
                        raise ImageUpdateFailed(
                            f"Unable to find a valid display index for address {self.ble_address} and ots_object_type: "
                            + hex(ots_object_type)
                        )
            else:
                if self.display_count is None or display_ind >= self.display_count:
                    raise ImageUpdateFailed(
                        f"Invalid display index for address {self.ble_address}: "
                        + str(display_ind)
                    )
                ots_object_type = self.display_info[display_ind].type

            disp_size = (
                self.display_info[display_ind].width,
                self.display_info[display_ind].height,
            )
            disp_type = self.display_info[display_ind].type

            if ots_object_type and disp_type == ots_object_type:
                self.log.debug(
                    "Display type matches object type for address %s", self.ble_address
                )
                if type(file) == str:
                    self.xbm_converter.open(file)
                    if self.xbm_converter.image is not None:
                        self.log.info("Image file opened: %s", file)
                elif type(file) == bytes:
                    self.xbm_converter.open_frombytes(file)
                cropfit_ = cropfit ^ bool(
                    self.ots_image_flags[image_index] & ESL_IMAGE_FLAGS_CROP_FIT
                )
                bitflip_ = bool(
                    self.ots_image_flags[image_index] & ESL_IMAGE_FLAGS_BIT_FLIP
                )
                rotation_ = int(
                    self.ots_image_flags[image_index] & ESL_IMAGE_FLAGS_ROTATION_MASK
                )
                if rotation_ == ESL_IMAGE_FLAGS_ROTATE_90:
                    rotation_ = 90
                elif rotation_ == ESL_IMAGE_FLAGS_ROTATE_180:
                    rotation_ = 180
                elif rotation_ == ESL_IMAGE_FLAGS_ROTATE_270:
                    rotation_ = 270
                else:
                    rotation_ = 0
                if rotation is not None:
                    rotation_ = rotation_ + rotation
                image_fmt = int(
                    self.ots_image_flags[image_index] & ESL_IMAGE_FLAGS_FORMAT_MASK
                )
                converter_kwargs = {
                    "esl_display_type": disp_type,
                    "display_size": disp_size,
                    "label": label,
                    "cropfit": cropfit_,
                    "bitflip": bitflip_,
                    "rotation": rotation_,
                    "img_fmt": image_fmt,
                }
                if ots_object_type in range(
                    ESL_WSTK_DISPLAY_TYPE, ESL_DISPLAY_TYPE_FULL_RGB + 1
                ):
                    self.raw_image = self.xbm_converter.convert(
                        **converter_kwargs
                    )  # (display_size=disp_size, bw=True, label=label, rotation=rotation, cropfit=cropfit)
                # elif ots_object_type == ESL_EPAPER_DISPLAY_TYPE:
                # self.raw_image = self.xbm_converter.convert(**converter_kwargs) #(display_size=disp_size, bw=False, label=label, rotation=rotation, cropfit=cropfit)
                else:
                    self.raw_image = b""
                    raise ImageUpdateFailed(
                        f"Unknown OTS object type, automatic conversion can't be done. Please upload raw image data to address {self.ble_address}!"
                    )
            else:
                raise ImageUpdateFailed(
                    f"Cannot upload file: display type is not the same as object type for address {self.ble_address}!"
                )
        # Send file if raw input or converted result seems OK
        if len(self.raw_image) != 0:
            self.write_image(image_index, self.raw_image)
        else:
            raise ImageUpdateFailed(
                f"Cannot upload file: image conversion failed for address {self.ble_address}"
            )

    def initiate_past(self, pawr_handle, pa_interval, subevent_count):
        """Initiate PAST"""
        if self.state != TagState.CONNECTED:
            raise InvalidTagStateError(
                f"Invalid ESL object state: {self._state} at address {self.ble_address}!"
            )
        if self.past_initiated:
            raise InvalidTagStateError(
                f"PAST has been already initiated for ESL at address {self.ble_address}!"
            )
        if subevent_count <= self.group_id:
            self.close_connection(force_close=True)
            raise PAwRSyncLostError(
                f"PAST can't be initiated for ESL at address {self.ble_address} because of invalid Group ID config {self.group_id} for PAwR subevent count of {subevent_count}!",
                self.esl_id,
                self.group_id,
                self.ble_address,
            )
        try:
            self.lib.initiate_past(self.connection_handle, pawr_handle)
            self._past_subevents_max = subevent_count
            self._past_timer.restart()
            self.busy = True
        except esl_lib.CommandFailedError as e:
            # self.lib.initiate_past is OK to return SL_STATUS_BT_CTRL_UNKNOWN_CONNECTION_IDENTIFIER if the ESL is already synchronized.
            if e.sl_status != elw.SL_STATUS_BT_CTRL_UNKNOWN_CONNECTION_IDENTIFIER:
                self.log.error(e)
