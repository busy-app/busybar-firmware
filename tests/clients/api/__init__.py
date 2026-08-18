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
    DeviceInfo,
    FirmwareInfo,
    PowerInfo,
    ResultResponse,
    SetTimestampRequest,
    SetTimezoneRequest,
    StatusResponse,
    SystemAPI,
    SystemInfo,
    TimeResponse,
    TimezoneItem,
    TimezoneListResponse,
    TimezoneResponse,
    VersionResponse,
)

# WiFi API
from .wifi import (
    WIFI_PASSWORD,
    WIFI_SECURITY,
    WIFI_SSID,
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
    DisplayDeleteRequest,
    DisplayDrawRequest,
    DisplayElement,
)

# Account API
from .account import (
    AccountAPI,
    AccountBackend,
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
    AccessTokensResponse,
    BrightnessResponse,
    CreateAccessTokenRequest,
    MintedAccessToken,
    NameResponse,
    SetNameRequest,
    SettingsAPI,
    SettingsResultResponse,
    StoredAccessToken,
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
    AutoupdateSettings,
    CheckStatus,
    DownloadProgress,
    InstallStatus,
    UpdateAPI,
    UpdateResultResponse,
    UpdateStatusResponse,
)

# Busy Timer API
from .busy import (
    BusyAPI,
    BusyBarSettings,
    BusyProfileResponse,
    BusyResultResponse,
    BusySnapshotResponse,
)

# Smart Home API
from .matter import (
    PairingStatus,
    SmartHomeAPI,
    SmartHomePairingPayload,
    SmartHomePairingResponse,
    SmartHomeSwitchState,
    SmartHomeResultResponse,
)

__all__ = [
    "BaseAPI",
    "APIError",
    "SystemAPI",
    "VersionResponse",
    "StatusResponse",
    "SystemInfo",
    "PowerInfo",
    "TimeResponse",
    "ResultResponse",
    "SetTimestampRequest",
    "SetTimezoneRequest",
    "WifiAPI",
    "WifiStatusResponse",
    "WifiNetwork",
    "WifiNetworksResponse",
    "WifiResultResponse",
    "WifiConnectRequest",
    "WIFI_SSID",
    "WIFI_PASSWORD",
    "WIFI_SECURITY",
    "StorageAPI",
    "StorageStatusResponse",
    "StorageItem",
    "StorageListResponse",
    "StorageResultResponse",
    "AssetsAPI",
    "AssetResultResponse",
    "DisplayElement",
    "DisplayDrawRequest",
    "DisplayDeleteRequest",
    "AccountAPI",
    "AccountBackend",
    "AccountInfoResponse",
    "AccountStatusResponse",
    "AccountProfileResponse",
    "AccountLinkResponse",
    "AccountResultResponse",
    "MQTTProfiles",
    "BleAPI",
    "BleStatusResponse",
    "SettingsAPI",
    "NameResponse",
    "AccessResponse",
    "StoredAccessToken",
    "MintedAccessToken",
    "AccessTokensResponse",
    "BrightnessResponse",
    "VolumeResponse",
    "SettingsResultResponse",
    "SetNameRequest",
    "CreateAccessTokenRequest",
    "InputAPI",
    "InputKeyRequest",
    "InputErrorResponse",
    "StreamingAPI",
    "UpdateAPI",
    "UpdateStatusResponse",
    "UpdateResultResponse",
    "InstallStatus",
    "CheckStatus",
    "DownloadProgress",
    "AutoupdateSettings",
    "DeviceInfo",
    "FirmwareInfo",
    "TimezoneResponse",
    "TimezoneItem",
    "TimezoneListResponse",
    "BusyAPI",
    "BusySnapshotResponse",
    "BusyProfileResponse",
    "BusyBarSettings",
    "BusyResultResponse",
    "SmartHomeAPI",
    "SmartHomePairingResponse",
    "SmartHomePairingPayload",
    "SmartHomeSwitchState",
    "SmartHomeResultResponse",
    "PairingStatus",
]
