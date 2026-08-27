"""Fixtures for local-link mDNS discovery tests."""

from __future__ import annotations

import socket
from collections.abc import Iterator
from urllib.parse import urlsplit

import allure
import pytest

from clients.api import SettingsAPI, SystemAPI


@pytest.fixture
def mdns_device_ip(web_base_url: str) -> str:
    device_ip = urlsplit(web_base_url).hostname
    if device_ip is None:
        raise ValueError(f"WEB_BASE_URL has no hostname: {web_base_url!r}")
    return device_ip


@pytest.fixture
def mdns_interface_ip(mdns_device_ip: str) -> str:
    """Select the interface whose route reaches the device local API."""
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as route_socket:
        route_socket.connect((mdns_device_ip, 80))
        return route_socket.getsockname()[0]


@pytest.fixture
def discovery_instance(system_api: SystemAPI) -> str:
    usb_mac = system_api.get_status().device.usb_mac
    device_id = usb_mac.replace(":", "").lower()
    return f"busybar-{device_id}"


@pytest.fixture
def preserve_device_name(settings_api: SettingsAPI) -> Iterator[None]:
    original_name = settings_api.get_name().name
    if not original_name:
        pytest.skip("Name-change discovery test requires a restorable device name")

    yield

    if settings_api.get_name().name != original_name:
        with allure.step(f"Restore device name to {original_name!r}"):
            settings_api.set_name(original_name)
