"""
API client library for BSB Test Automation.

This module provides domain-specific API clients with:
- Pydantic models for request/response validation
- Automatic Allure step integration
- Consistent logging

Usage:
    from api import SystemAPI, WifiAPI
    from api.system import VersionResponse

    def test_version(system_api: SystemAPI):
        response = system_api.get_version()
        assert response.api_semver >= "0.0.0"
"""

# Base
from .base import APIError, BaseAPI

# System API
from .system import (
    PowerInfo,
    ResultResponse,
    SetTimestampRequest,
    SetTimezoneRequest,
    StatusResponse,
    SystemAPI,
    SystemInfo,
    TimeResponse,
    VersionResponse,
)

# WiFi API
from .wifi import (
    TEST_WIFI_PASSWORD,
    TEST_WIFI_SECURITY,
    TEST_WIFI_SSID,
    WifiAPI,
    WifiConnectRequest,
    WifiNetwork,
    WifiNetworksResponse,
    WifiResultResponse,
    WifiStatusResponse,
)

# Storage API
from .storage import (
    StorageAPI,
    StorageItem,
    StorageListResponse,
    StorageResultResponse,
    StorageStatusResponse,
)

# Assets API (includes Display and Audio)
from .assets import (
    AssetResultResponse,
    AssetsAPI,
    DisplayDrawRequest,
    DisplayElement,
)

# Account API
from .account import (
    AccountAPI,
    AccountInfoResponse,
    AccountLinkResponse,
    AccountProfileResponse,
    AccountResultResponse,
    AccountStatusResponse,
    MQTTProfiles,
)

# BLE API
from .ble import (
    BleAPI,
    BleStatusResponse,
)

# Settings API
from .settings import (
    AccessResponse,
    BrightnessResponse,
    NameResponse,
    SetNameRequest,
    SettingsAPI,
    SettingsResultResponse,
    VolumeResponse,
)

# Input API
from .input import (
    InputAPI,
    InputErrorResponse,
    InputKeyRequest,
)

# Streaming API
from .streaming import (
    StreamingAPI,
)

# Update API
from .update import (
    CheckStatus,
    DownloadProgress,
    InstallStatus,
    UpdateAPI,
    UpdateResultResponse,
    UpdateStatusResponse,
)

__all__ = [
    # Base
    "BaseAPI",
    "APIError",
    # System API
    "SystemAPI",
    "VersionResponse",
    "StatusResponse",
    "SystemInfo",
    "PowerInfo",
    "TimeResponse",
    "ResultResponse",
    "SetTimestampRequest",
    "SetTimezoneRequest",
    # WiFi API
    "WifiAPI",
    "WifiStatusResponse",
    "WifiNetwork",
    "WifiNetworksResponse",
    "WifiResultResponse",
    "WifiConnectRequest",
    "TEST_WIFI_SSID",
    "TEST_WIFI_PASSWORD",
    "TEST_WIFI_SECURITY",
    # Storage API
    "StorageAPI",
    "StorageStatusResponse",
    "StorageItem",
    "StorageListResponse",
    "StorageResultResponse",
    # Assets API
    "AssetsAPI",
    "AssetResultResponse",
    "DisplayElement",
    "DisplayDrawRequest",
    # Account API
    "AccountAPI",
    "AccountInfoResponse",
    "AccountStatusResponse",
    "AccountProfileResponse",
    "AccountLinkResponse",
    "AccountResultResponse",
    "MQTTProfiles",
    # BLE API
    "BleAPI",
    "BleStatusResponse",
    # Settings API
    "SettingsAPI",
    "NameResponse",
    "AccessResponse",
    "BrightnessResponse",
    "VolumeResponse",
    "SettingsResultResponse",
    "SetNameRequest",
    # Input API
    "InputAPI",
    "InputKeyRequest",
    "InputErrorResponse",
    # Streaming API
    "StreamingAPI",
    # Update API
    "UpdateAPI",
    "UpdateStatusResponse",
    "UpdateResultResponse",
    "InstallStatus",
    "CheckStatus",
    "DownloadProgress",
]
