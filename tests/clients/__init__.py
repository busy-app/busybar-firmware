"""
Clients for BSB automation tests.

This module provides HTTP API clients and CLI clients for interacting with the BSB device.
"""

from .api import (
    AccountAPI,
    AssetsAPI,
    BleAPI,
    InputAPI,
    SettingsAPI,
    StorageAPI,
    StreamingAPI,
    SystemAPI,
    UpdateAPI,
    WifiAPI,
)

__all__ = [
    "SystemAPI",
    "WifiAPI",
    "StorageAPI",
    "AssetsAPI",
    "AccountAPI",
    "BleAPI",
    "SettingsAPI",
    "InputAPI",
    "StreamingAPI",
    "UpdateAPI",
]
