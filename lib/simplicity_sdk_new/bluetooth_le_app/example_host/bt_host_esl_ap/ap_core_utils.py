"""
Utilities for AP core.
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

import sys
import threading
from dataclasses import dataclass
from typing import Optional
import ap_json_helper
from esl_lib import Address

@dataclass
class ExclusiveMode:
    """
    Represents the exclusive mode configuration for the ESL network.

    This structure models three meaningful states of exclusive mode:
      - Exclusive mode disabled: enabled=False, source_file=None
      - Exclusive mode enabled without a known source file:
            enabled=True, source_file=None
      - Exclusive mode enabled and initialized from a configuration file:
            enabled=True, source_file="<path>"

    The 'enabled' flag indicates whether exclusive mode is active.
    The 'source_file' field stores the path of the JSON configuration file
    used during startup, or None if exclusive mode was activated manually
    or no file was provided.
    """
    enabled: bool = False             # True = exclusive mode active
    source_file: Optional[str] = None # None = no file known or given, string = file name
    _config: Optional[dict[str, dict[str, str]]] = None # to store parsed config data
    _address_set: Optional[set[str]] = None # boost lookup performance


    def __post_init__(self):
        # Ensure consistency at initialization
        if not self.enabled:
            self._config = None
            self._address_set = None
        elif self._config:
            self._rebuild_address_set()

    @property
    def config(self) -> Optional[dict[str, dict[str, str]]]:
        return self._config

    @config.setter
    def config(self, value: Optional[dict[str, dict[str, str]]]):
        # Config cannot be cleared in exclusive mode
        if self.enabled and not value:
            raise ValueError(
                "Cannot clear exclusive network configuration while exclusive mode is active."
            )

        # If exclusive mode is disabled, allow clearing
        if not self.enabled and not value:
            self._config = None
            self._address_set = None
            return

        # Must be a dictionary with at least one group - does not check contents further!
        if not isinstance(value, dict) or len(value) == 0:
            raise ValueError("Invalid network configuration.")

        # Accept the new config
        self._config = value
        self._rebuild_address_set()

    def _rebuild_address_set(self):
        # rebuild the address set for fast lookup
        self._address_set = {
            addr.lower()
            for group in self._config.values()
            for addr in group.values()
        }

    def contains_address(self, address) -> bool:
        if not self.enabled or not self._address_set:
            return False

        # Accept Address objects directly
        if isinstance(address, Address):
            address = repr(address)
        elif not isinstance(address, str):
            raise TypeError("'address' must be a string or Address instance!")

        return address.lower() in self._address_set

    @property
    def enabled(self):
        return self._enabled

    @enabled.setter
    def enabled(self, value: bool):
        # Clear the source file automatically when disabling exclusive mode
        self._enabled = value
        if not value:
            self.source_file = None

    @classmethod
    def from_file(cls, filename: Optional[str], lib, key_db, tag_db, logger):
        if not filename or not filename.strip():
            return cls(enabled=False, source_file=None)

        json = ap_json_helper.JSONHelper()

        try:
            loaded = json.import_network_config(lib, key_db, tag_db, filename)

        except Exception as e:
            logger.error(
                "Exclusive network mode not enabled; failed to import configuration file %s due to: %s",
                filename,
                e,
            )
            return cls(enabled=False, source_file=None)

        # Send error if no devices were loaded
        if not loaded:
            logger.warning(
                "Exclusive network mode not enabled; no devices could be loaded from file %s.",
                filename,
            )
            return cls(enabled=False, source_file=None)

        # Create instance with loaded config
        instance = cls(enabled=True, source_file=filename)
        instance.config = loaded  # setter will build an address set for fast lookup

        group_count = len(loaded)
        device_count = tag_db.device_count

        logger.warning(
            "Based on the data loaded from file %s, exclusive mode is enabled "
            "for %d device%s in %d group%s.",
            filename,
            device_count,
            "" if device_count == 1 else "s",
            group_count,
            "" if group_count == 1 else "s",
        )

        return instance

def skip_if_stopped(method):
    """Skip method execution if the stop_event is set"""
    def wrapper(self, *args, **kwargs):
        if hasattr(self, "stop_event") and self.stop_event.is_set():
            if hasattr(self, "log"):
                self.log.warning(
                    f"Skipped execution of AP method '{method.__name__}' due to shutdown"
                )
            return None
        return method(self, *args, **kwargs)
    return wrapper

def deep_size(obj, seen=None):
    if seen is None:
        seen = set()
    obj_id = id(obj)
    if obj_id in seen:
        return 0
    seen.add(obj_id)

    size = sys.getsizeof(obj)

    # Explore object attributes
    if hasattr(obj, "__dict__"):
        size += deep_size(obj.__dict__, seen)

    # Explore slots
    if hasattr(obj, "__slots__"):
        for slot in obj.__slots__:
            if hasattr(obj, slot):
                size += deep_size(getattr(obj, slot), seen)

    # Explore container types
    if isinstance(obj, dict):
        for k, v in obj.items():
            size += deep_size(k, seen)
            size += deep_size(v, seen)
    elif isinstance(obj, (list, tuple, set, frozenset)):
        for item in obj:
            size += deep_size(item, seen)

    return size

class LibProxy:
    """
    Proxy ESL library method calls to ensure controlled access.

    - Block method calls if AccessPoint's stop_event is set, except "stop".
    - Make all methods thread-safe, except "start" and "stop".
    - Log warnings for skipped calls if shutdown is already in progress.
    """
    def __init__(self, lib, access_point):
        self._lib = lib
        self._access_point = access_point
        self._lock = threading.Lock()  # Lock for thread-safety

    def _guarded_method(self, name, attr, *args, **kwargs):
        if name not in {"start", "stop"}:  # Exclude "start" and "stop" from thread-safety and stop checks
            with self._lock:  # Ensure thread-safety for all other methods
                if self._access_point.stop_event.is_set():
                    self._access_point.log.warning(
                        f"Skipped LIB call to 'esl_lib_{name}' due to shutdown in progress"
                    )
                    return None
                return attr(*args, **kwargs)
        else:
            return attr(*args, **kwargs)

    def __getattr__(self, name):
        attr = getattr(self._lib, name)
        if callable(attr):
            return lambda *args, **kwargs: self._guarded_method(name, attr, *args, **kwargs)
        return attr

class StackSizeManager:
    def __init__(self, logger):
        self.log = logger

    def configure_stack_size(self):
        """Configure the thread stack size, trying larger sizes first."""
        stack_size_base = 1024 * 1024  # 1 MB base size request
        # Try setting 8 MB stack size
        try:
            threading.stack_size(8 * stack_size_base)
            return  # Success, continue execution
        except (threading.ThreadError, ValueError) as e:
            # Log warning and try 2 MB
            self.log.warning("Unable to set 8MB thread stack size: %s. Attempting 2MB fallback, which may restrict the number of supported ESLs." % str(e))

        # Try setting 2 MB stack size
        try:
            threading.stack_size(2 * stack_size_base)
            return  # Success, continue execution
        except (threading.ThreadError, ValueError) as e:
            # Log critical error and exit
            self.log.critical("Failed to set 2MB thread stack size: %s. Cannot proceed safely." % str(e))
            sys.exit(-1)
