"""
WiFi API client and Pydantic models.

Endpoints:
- GET /api/wifi/status
- GET /api/wifi/networks
- POST /api/wifi/connect
- POST /api/wifi/disconnect
"""

from __future__ import annotations

import os
from typing import Literal

from pydantic import BaseModel, field_validator

from .base import BaseAPI


# === Test Network Configuration ===
# Can be overridden via environment variables

TEST_WIFI_SSID = os.environ.get("TEST_WIFI_SSID", "")
TEST_WIFI_PASSWORD = os.environ.get("TEST_WIFI_PASSWORD", "")
TEST_WIFI_SECURITY = os.environ.get("TEST_WIFI_SECURITY", "WPA2")


# === Response Models ===


class WifiStatusResponse(BaseModel):
    """Response from GET /api/wifi/status."""

    state: Literal[
        "unknown", "disconnected", "connected",
        "connecting", "disconnecting", "reconnecting"
    ]


class WifiNetwork(BaseModel):
    """Single WiFi network in scan results."""

    ssid: str
    rssi: int | None = None
    security: str | None = None


class WifiNetworksResponse(BaseModel):
    """Response from GET /api/wifi/networks."""

    count: int
    networks: list[WifiNetwork]


class WifiResultResponse(BaseModel):
    """Generic WiFi operation result."""

    result: str

    @field_validator("result")
    @classmethod
    def validate_result(cls, v: str) -> str:
        """Validate result indicates success."""
        assert v, "Expected non-empty result"
        return v


# === Request Models ===


class WifiIpConfig(BaseModel):
    """IP configuration for WiFi connection."""

    ip_method: Literal["dhcp", "static"] = "dhcp"
    ip_address: str | None = None
    gateway: str | None = None
    netmask: str | None = None
    dns: str | None = None


class WifiConnectRequest(BaseModel):
    """Request for POST /api/wifi/connect."""

    ssid: str
    password: str
    security: str = "WPA2"
    ip_config: WifiIpConfig = WifiIpConfig()


# === API Client ===


class WifiAPI(BaseAPI):
    """
    WiFi API client.

    Endpoints:
    - GET /api/wifi/status - Get WiFi connection status
    - GET /api/wifi/networks - Scan for WiFi networks
    - POST /api/wifi/connect - Connect to a WiFi network
    - POST /api/wifi/disconnect - Disconnect from WiFi
    """

    def get_status(self) -> WifiStatusResponse:
        """Get WiFi connection status."""
        return self.get("/api/wifi/status", WifiStatusResponse)

    def get_networks(self, timeout: int = 30) -> WifiNetworksResponse:
        """Scan for available WiFi networks."""
        return self.get("/api/wifi/networks", WifiNetworksResponse, timeout=timeout)

    def connect(
        self,
        ssid: str,
        password: str,
        security: str = "WPA2",
        ip_method: str = "dhcp",
        timeout: int = 30,
    ):
        """
        Connect to a WiFi network.

        Args:
            ssid: Network SSID
            password: Network password
            security: Security type (default WPA2)
            ip_method: IP configuration method (dhcp or static)
            timeout: Request timeout in seconds

        Returns:
            Raw response (may return 200 or 400 depending on network availability)
        """
        req = WifiConnectRequest(
            ssid=ssid,
            password=password,
            security=security,
            ip_config=WifiIpConfig(ip_method=ip_method),
        )
        return self.post_raw(
            "/api/wifi/connect",
            json=req.model_dump(exclude_none=True),
            timeout=timeout,
        )

    def disconnect(self) -> WifiResultResponse:
        """Disconnect from WiFi."""
        return self.post("/api/wifi/disconnect", WifiResultResponse)

    def connect_to_test_network(self, timeout: int = 30):
        """
        Connect to the test WiFi network.

        Uses TEST_WIFI_SSID, TEST_WIFI_PASSWORD, TEST_WIFI_SECURITY
        which can be overridden via environment variables.

        Returns:
            Raw response from connect API
        """
        return self.connect(
            ssid=TEST_WIFI_SSID,
            password=TEST_WIFI_PASSWORD,
            security=TEST_WIFI_SECURITY,
            timeout=timeout,
        )
