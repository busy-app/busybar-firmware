"""
BLE integration test fixtures.

These fixtures manage the lifecycle of BLE connections:
1. Remove stale pairing on the device via HTTP API
2. Enable BLE on the device via the HTTP API
3. Register a D-Bus pairing agent (Linux) for auto-accept
4. Scan for the device using bleak
5. Establish / tear down BLE connections
"""

from __future__ import annotations

import asyncio
import logging
import os
import platform
import subprocess
import sys
import time
import uuid
from typing import Callable

import allure
import pytest
import requests

from clients.api.base import APIError
from clients.api.ble import BleAPI
from clients.ble.client import BleDeviceClient
from clients.ble.models import ScannedDevice

logger = logging.getLogger("ble.conftest")


async def _wait_for_ble_ready_state(ble_api: BleAPI, timeout: float = 15.0) -> None:
    """Wait until the device returns to an advertising-capable BLE state."""
    deadline = time.time() + timeout
    last_status = "unknown"
    while time.time() < deadline:
        # The BLE stack answers 503 while it is reconfiguring right after
        # connect/disconnect churn — treat it as "not ready yet", not a crash.
        try:
            last_status = ble_api.get_status().status
        except APIError as exc:
            logger.debug("BLE status not available yet: %s", exc)
            last_status = "unavailable"
        if last_status in ("connectable", "enabled"):
            return
        await asyncio.sleep(1.0)
    pytest.fail(
        f"BLE did not return to a ready state within {timeout:.0f} s "
        f"(last status: {last_status})"
    )


async def _reset_linux_device_pairing(ble_api: BleAPI) -> None:
    """Reset device-side pairing before each Linux scan/connect cycle."""
    if platform.system() != "Linux":
        return

    response = ble_api.remove_pairing()
    if response.status_code not in (200, 503):
        pytest.fail(
            "Failed to remove BLE pairing before Linux scan/connect cycle: "
            f"{response.status_code}"
        )

    await asyncio.sleep(1.0)
    await _wait_for_ble_ready_state(ble_api)

# ---------------------------------------------------------------------------
# Session-scoped configuration
# ---------------------------------------------------------------------------


@pytest.fixture(scope="session")
def ble_suite_device_name(web_base_url: str) -> str:
    """Give the target device a suite-unique BLE name and restore it afterwards.

    The lab can have multiple BSBs advertising the same base name (for example
    ``ssecsdBAR#1`` and ``ssecsdBAR#2``). A suite-unique name makes all BLE scans
    target the HTTP-selected device instead of whichever matching BSB appears first.
    """
    session = requests.Session()
    session.headers.update(
        {"Accept": "application/json", "User-Agent": "BSB-AutoTest/1.0"}
    )
    original_name = session.get(f"{web_base_url}/api/name", timeout=10).json()["name"]
    suite_name = os.environ.get("BLE_SUITE_DEVICE_NAME") or f"BSB-{uuid.uuid4().hex[:8]}"

    logger.info(
        "Setting suite-unique BLE device name: %s -> %s",
        original_name,
        suite_name,
    )
    response = session.post(
        f"{web_base_url}/api/name",
        json={"name": suite_name},
        timeout=10,
    )
    assert response.status_code == 200, (
        f"Failed to set suite BLE device name: {response.status_code} {response.text}"
    )
    time.sleep(1.0)

    try:
        yield suite_name
    finally:
        logger.info("Restoring BLE device name: %s", original_name)
        try:
            session.post(
                f"{web_base_url}/api/name",
                json={"name": original_name},
                timeout=10,
            )
        except Exception as exc:
            logger.warning("Failed to restore BLE device name %r: %s", original_name, exc)
        session.close()


@pytest.fixture(scope="session")
def ble_device_name(ble_suite_device_name: str) -> str:
    """BLE device advertising name used by tests after suite isolation."""
    return ble_suite_device_name


@pytest.fixture(scope="session")
def ble_device_address() -> str | None:
    """Optional BLE device address (env: ``BLE_DEVICE_ADDRESS``)."""
    return os.environ.get("BLE_DEVICE_ADDRESS")


@pytest.fixture(scope="session")
def ble_adapter() -> str | None:
    """BLE adapter to use (env: ``BLE_ADAPTER``), e.g. ``hci1``. None = default."""
    return os.environ.get("BLE_ADAPTER")


@pytest.fixture(scope="module")
def _ble_api_module(web_base_url: str) -> BleAPI:
    """Module-scoped BLE HTTP API client (avoids scope mismatch)."""
    session = requests.Session()
    session.headers.update(
        {"Accept": "application/json", "User-Agent": "BSB-AutoTest/1.0"}
    )
    return BleAPI(session, web_base_url)


# ---------------------------------------------------------------------------
# Linux D-Bus pairing agent
# ---------------------------------------------------------------------------

_dbus_agent_task = None


async def _start_dbus_pairing_agent():
    """Register a BlueZ D-Bus agent that auto-accepts pairing (Linux only)."""
    if platform.system() != "Linux":
        return None, None

    try:
        from dbus_fast.aio import MessageBus
        from dbus_fast.service import ServiceInterface, method
        from dbus_fast import BusType
    except ImportError:
        logger.warning("dbus-fast not installed, skipping pairing agent")
        return None, None

    AGENT_PATH = "/test/ble_autotest_agent"

    class AutoAcceptAgent(ServiceInterface):
        def __init__(self):
            super().__init__("org.bluez.Agent1")

        @method()
        def Release(self):
            pass

        @method()
        def RequestConfirmation(self, device: "o", passkey: "u"):
            logger.info("Auto-confirming passkey %d for %s", passkey, device)

        @method()
        def AuthorizeService(self, device: "o", uuid: "s"):
            logger.info("Auto-authorizing service %s for %s", uuid, device)

        @method()
        def RequestAuthorization(self, device: "o"):
            logger.info("Auto-authorizing device %s", device)

        @method()
        def Cancel(self):
            pass

        @method()
        def DisplayPasskey(self, device: "o", passkey: "u"):
            logger.info("Display passkey: %d for %s", passkey, device)

    try:
        bus = await MessageBus(bus_type=BusType.SYSTEM).connect()
        agent = AutoAcceptAgent()
        bus.export(AGENT_PATH, agent)

        intro = await bus.introspect("org.bluez", "/org/bluez")
        mgr = bus.get_proxy_object("org.bluez", "/org/bluez", intro)
        agent_mgr = mgr.get_interface("org.bluez.AgentManager1")

        await agent_mgr.call_register_agent(AGENT_PATH, "DisplayYesNo")
        await agent_mgr.call_request_default_agent(AGENT_PATH)
        logger.info("D-Bus pairing agent registered")
        return bus, agent_mgr
    except Exception as exc:
        logger.warning("Failed to register D-Bus pairing agent: %s", exc)
        return None, None


async def _stop_dbus_pairing_agent(bus, agent_mgr):
    """Unregister the D-Bus pairing agent."""
    if agent_mgr is None:
        return
    try:
        await agent_mgr.call_unregister_agent("/test/ble_autotest_agent")
    except Exception:
        pass
    if bus is not None:
        bus.disconnect()


# ---------------------------------------------------------------------------
# Module-scoped: enable / disable BLE via HTTP API
# ---------------------------------------------------------------------------


@pytest.fixture(scope="module")
def ble_enabled(_ble_api_module: BleAPI, ble_suite_device_name: str) -> None:
    """Enable BLE on the device and wait for a ready state.

    On Linux, removes stale pairing first (forces public address).
    Teardown disables BLE.
    """
    with allure.step("Prepare BLE on device"):
        # Device-side pairing is reset per-test in `_reset_linux_device_pairing`
        # (connect fixtures), so no module-level remove_pairing here — the extra
        # DELETE /api/ble/pairing only adds churn that can wedge the BLE stack.
        logger.info("Enabling BLE via HTTP API")
        # Right after boot the BLE stack needs ~10 s of warm-up and returns
        # transient 503 — retry instead of failing the whole module.
        enable_deadline = time.time() + 30.0
        while True:
            response = _ble_api_module.enable()
            if response.status_code == 200:
                break
            if time.time() >= enable_deadline:
                pytest.fail(
                    f"Failed to enable BLE within 30 s "
                    f"(last status: {response.status_code})"
                )
            logger.info("BLE enable returned %s, retrying", response.status_code)
            time.sleep(2.0)

        # Poll until the device reaches an advertising-capable state
        _READY_STATES = {"connectable", "enabled"}
        deadline = time.time() + 30.0
        last_status = "unknown"
        while time.time() < deadline:
            try:
                last_status = _ble_api_module.get_status().status
            except APIError as exc:
                logger.debug("BLE status not available yet: %s", exc)
                last_status = "unavailable"
            logger.debug("BLE status: %s", last_status)
            if last_status in _READY_STATES:
                break
            time.sleep(1.0)
        else:
            pytest.fail(
                f"BLE did not reach a ready state within 30 s "
                f"(last status: {last_status})"
            )

    yield

    with allure.step("Disable BLE via HTTP API"):
        logger.info("Disabling BLE via HTTP API")
        try:
            _ble_api_module.disable()
        except Exception as exc:
            logger.warning("BLE disable failed (ignored): %s", exc)


# ---------------------------------------------------------------------------
# Function-scoped: BLE client instances
# ---------------------------------------------------------------------------


@pytest.fixture()
async def _ble_pairing_agent():
    """D-Bus pairing agent (Linux auto-accept, no-op on macOS).

    Function-scoped: pytest-asyncio 0.23 has no module-scoped event_loop
    here, so a module-scoped async fixture fails with 'event_loop not found'.
    """
    bus, agent_mgr = await _start_dbus_pairing_agent()
    yield
    await _stop_dbus_pairing_agent(bus, agent_mgr)


@pytest.fixture()
def ble_client(_ble_pairing_agent) -> BleDeviceClient:
    """Return a fresh (disconnected) :class:`BleDeviceClient`."""
    return BleDeviceClient()


@pytest.fixture()
def clear_linux_host_pairing() -> Callable[[str, str | None], None]:
    """Forget cached BlueZ pairing/bond state for a device on Linux."""

    def _clear(address: str, adapter: str | None = None) -> None:
        if platform.system() != "Linux":
            return

        bluez_adapter = adapter or "hci0"
        device_path = (
            f"/org/bluez/{bluez_adapter}/dev_{address.upper().replace(':', '_')}"
        )
        result = subprocess.run(
            [
                "busctl",
                "call",
                "org.bluez",
                f"/org/bluez/{bluez_adapter}",
                "org.bluez.Adapter1",
                "RemoveDevice",
                "o",
                device_path,
            ],
            capture_output=True,
            text=True,
            check=False,
        )
        output = (result.stdout + result.stderr).strip()

        if result.returncode == 0:
            logger.info(
                "Removed Linux host pairing cache for %s on %s",
                address,
                bluez_adapter,
            )
            return

        if "does not exist" in output.lower():
            logger.info(
                "Linux host pairing cache already absent for %s on %s",
                address,
                bluez_adapter,
            )
            return

        pytest.fail(
            "Failed to remove Linux host pairing cache for "
            f"{address} on {bluez_adapter}: {output or f'rc={result.returncode}'}"
        )

    return _clear


@pytest.fixture()
async def connected_ble_client(
    ble_enabled: None,
    _ble_api_module: BleAPI,
    _ble_pairing_agent: None,
    ble_device_name: str,
    ble_device_address: str | None,
    ble_adapter: str | None,
    clear_linux_host_pairing,
) -> BleDeviceClient:
    """Scan, connect, yield a connected client, then disconnect."""
    client = BleDeviceClient()

    with allure.step("Scan and connect to device"):
        await _reset_linux_device_pairing(_ble_api_module)
        devices = await BleDeviceClient.scan(
            name=ble_device_name,
            address=ble_device_address,
            adapter=ble_adapter,
        )
        if not devices:
            pytest.skip(
                f"BLE device '{ble_device_name}' not found after scanning"
            )

        device = devices[0]
        clear_linux_host_pairing(device.address, ble_adapter)
        if platform.system() == "Linux":
            devices = await BleDeviceClient.scan(
                name=ble_device_name,
                address=ble_device_address,
                adapter=ble_adapter,
                retries=2,
            )
            if not devices:
                pytest.skip(
                    f"BLE device '{ble_device_name}' not found after Linux bond reset"
                )
            refreshed = devices[0]
            if refreshed.address != device.address:
                logger.info(
                    "BLE address changed after Linux host pairing clear: %s -> %s",
                    device.address,
                    refreshed.address,
                )
            device = refreshed
        logger.info("Connecting to %s (%s)", device.name, device.address)
        await client.connect(device.connect_target, adapter=ble_adapter)

    try:
        yield client
    finally:
        with allure.step("Disconnect BLE client"):
            await client.disconnect()


@pytest.fixture()
async def ble_scanned_device(
    ble_enabled: None,
    _ble_api_module: BleAPI,
    ble_device_name: str,
    ble_device_address: str | None,
    ble_adapter: str | None,
    clear_linux_host_pairing,
) -> ScannedDevice:
    """Scan for the device and return the first match (no connection)."""
    await _reset_linux_device_pairing(_ble_api_module)
    devices = await BleDeviceClient.scan(
        name=ble_device_name,
        address=ble_device_address,
        adapter=ble_adapter,
    )
    if not devices:
        pytest.skip(
            f"BLE device '{ble_device_name}' not found after scanning"
        )
    device = devices[0]
    clear_linux_host_pairing(device.address, ble_adapter)
    if platform.system() == "Linux":
        devices = await BleDeviceClient.scan(
            name=ble_device_name,
            address=ble_device_address,
            adapter=ble_adapter,
            retries=2,
        )
        if not devices:
            pytest.skip(
                f"BLE device '{ble_device_name}' not found after Linux bond reset"
            )
        refreshed = devices[0]
        if refreshed.address != device.address:
            logger.info(
                "BLE address changed after Linux host pairing clear: %s -> %s",
                device.address,
                refreshed.address,
            )
        device = refreshed
    return device
