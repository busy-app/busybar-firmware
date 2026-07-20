"""
Fixtures for Matter integration tests.

The Matter controller is an existing Home Assistant instance whose Matter
Server add-on shares L2 with the device's WiFi network (Matter advertises
over WiFi only, IPv6 — see README.md).

Env:
    HA_URL     e.g. http://10.46.21.151     (unset → suite skips)
    HA_TOKEN   long-lived access token      (unset → suite skips)
    WIFI_SSID / WIFI_PASSWORD / WIFI_SECURITY  device test WiFi network
"""

from __future__ import annotations

import os
import time

import pytest
import requests

from clients.api import SmartHomeAPI, SystemAPI, WifiAPI
from clients.ha_matter import HAMatterClient

HA_URL = os.getenv("HA_URL", "")
HA_TOKEN = os.getenv("HA_TOKEN", "")


@pytest.fixture(scope="module")
def matter_module_api_factory(web_base_url):
    session = requests.Session()
    yield lambda api_class: api_class(session, web_base_url)
    session.close()


@pytest.fixture(scope="module")
def matter_smart_home_api(matter_module_api_factory) -> SmartHomeAPI:
    return matter_module_api_factory(SmartHomeAPI)


@pytest.fixture(scope="module")
def ha(matter_module_api_factory) -> HAMatterClient:
    """Connected Home Assistant client; ensures device WiFi is up first."""
    if not (HA_URL and HA_TOKEN):
        pytest.skip("HA_URL / HA_TOKEN are not set")

    _ensure_wifi_connected(matter_module_api_factory(WifiAPI))

    client = HAMatterClient(HA_URL, HA_TOKEN)
    yield client
    client.close()


@pytest.fixture(scope="module")
def commissioned_device(ha, matter_smart_home_api, matter_module_api_factory) -> dict:
    """Commission the device into HA for this module; remove on teardown.

    Yields {"device_id": ..., "switch_entity": ...}.
    """
    serial = matter_module_api_factory(SystemAPI).get_device_info().serial_number

    # A leftover device from a previous run makes re-commissioning fail with
    # "NOC for a fabric that already exists" — remove ours (and only ours).
    leftover = ha.device_by_serial(serial)
    if leftover:
        ha.remove_device(leftover["id"])
        time.sleep(5)  # let RemoveFabric land on the device

    fabrics_before = matter_smart_home_api.get_pairing().fabric_count

    device = None
    commissioning_started = False
    try:
        payload = matter_smart_home_api.start_pairing()
        assert payload.status_code == 200, payload.text
        commissioning_started = True
        ha.commission(payload.json()["manual_code"])

        device = _wait_for_value(lambda: ha.device_by_serial(serial), timeout=60)
        assert device, f"device with serial {serial} did not appear in HA registry"

        def _entity_or_none():
            try:
                return ha.switch_entity(device["id"])
            except Exception:
                return None

        entity = _wait_for_value(_entity_or_none, timeout=30)
        assert entity, f"no light/switch entity appeared for device {device['id']}"

        yield {"device_id": device["id"], "switch_entity": entity}
    finally:
        if device is None and commissioning_started:
            device = _wait_for_value(lambda: ha.device_by_serial(serial), timeout=5)
        if device:
            ha.remove_device(device["id"])

        fabric_count = _wait_for_fabric_count(
            matter_smart_home_api, maximum=fabrics_before, timeout=30
        )
        assert fabric_count <= fabrics_before, (
            f"fabric count stayed at {fabric_count} after HA device removal; "
            f"expected at most {fabrics_before}"
        )


def _wait_for_value(getter, timeout: float, interval: float = 0.5):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        value = getter()
        if value:
            return value
        time.sleep(interval)
    return getter()


def _wait_for_fabric_count(
    smart_home_api: SmartHomeAPI,
    maximum: int,
    timeout: float,
    interval: float = 0.5,
) -> int:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        fabric_count = smart_home_api.get_pairing().fabric_count
        if fabric_count <= maximum:
            return fabric_count
        time.sleep(interval)
    return smart_home_api.get_pairing().fabric_count


def _ensure_wifi_connected(wifi_api: WifiAPI, timeout: float = 90.0) -> None:
    if wifi_api.get_status().state == "connected":
        return
    if not os.getenv("WIFI_SSID"):
        pytest.skip("device WiFi is down and WIFI_SSID is not set")
    wifi_api.connect_to_test_network()
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if wifi_api.get_status().state == "connected":
            return
        time.sleep(1)
    pytest.fail(f"device WiFi did not connect within {timeout}s")
