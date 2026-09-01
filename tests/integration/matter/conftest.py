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

import allure
import pytest
import requests

from clients.api import SmartHomeAPI, SystemAPI, WifiAPI
from clients.ha_matter import HAMatterClient
from integration.matter.helpers import establish_idle_lamp_off
from utils.wait import wait_for
from utils.wifi_helpers import connect_to_test_network_or_fail

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
        fabrics_with_leftover = matter_smart_home_api.get_pairing().fabric_count
        ha.remove_device(leftover["id"])
        if fabrics_with_leftover:
            _wait_for_fabric_count(
                matter_smart_home_api, maximum=fabrics_with_leftover - 1, timeout=30
            )

    fabrics_before = matter_smart_home_api.get_pairing().fabric_count

    device = None
    commissioning_started = False
    try:
        payload = matter_smart_home_api.start_pairing()
        assert payload.status_code == 200, payload.text
        commissioning_started = True
        ha.commission(payload.json()["manual_code"])

        device = wait_for(
            f"device with serial {serial} in HA registry",
            lambda: ha.device_by_serial(serial),
            bool,
            timeout=60,
            interval=0.5,
        )

        # switch_entity raises until the entity appears; wait_for tolerates that.
        entity = wait_for(
            f"light/switch entity for device {device['id']}",
            lambda: ha.switch_entity(device["id"]),
            bool,
            timeout=30,
            interval=0.5,
        )

        yield {"device_id": device["id"], "switch_entity": entity}
    finally:
        if device is None and commissioning_started:
            try:
                device = wait_for(
                    "possibly-commissioned device in HA registry",
                    lambda: ha.device_by_serial(serial),
                    bool,
                    timeout=5,
                    interval=0.5,
                )
            except AssertionError:
                device = None
        if device:
            ha.remove_device(device["id"])

        _wait_for_fabric_count(
            matter_smart_home_api, maximum=fabrics_before, timeout=30
        )


@pytest.fixture()
def idle_lamp_off(ha, matter_smart_home_api, commissioned_device, busy_api):
    """Give each stateful test an idle timer and synchronized OFF switch."""
    entity = commissioned_device["switch_entity"]

    with allure.step("Establish an idle timer and lamp-off precondition"):
        establish_idle_lamp_off(ha, matter_smart_home_api, busy_api, entity)

    yield entity

    with allure.step("Restore the idle timer and lamp-off state"):
        establish_idle_lamp_off(ha, matter_smart_home_api, busy_api, entity)


def _wait_for_fabric_count(
    smart_home_api: SmartHomeAPI, maximum: int, timeout: float
) -> int:
    return wait_for(
        f"fabric count to drop to at most {maximum}",
        lambda: smart_home_api.get_pairing().fabric_count,
        lambda count: count <= maximum,
        timeout=timeout,
        interval=0.5,
    )


def _ensure_wifi_connected(wifi_api: WifiAPI) -> None:
    if wifi_api.get_status().state == "connected":
        return
    if not os.getenv("WIFI_SSID"):
        pytest.skip("device WiFi is down and WIFI_SSID is not set")
    connect_to_test_network_or_fail(wifi_api, timeout=90)
