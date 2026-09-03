"""Shared fixtures for the CLI test suite (integration/cli/).

Plain helpers and constants live in utils/cli_helpers.py.
"""

import re
import threading

import allure
import pytest

from clients.api import WIFI_SSID, WifiAPI
from clients.cli import SimpleCLIConnection
from utils.cli_helpers import resync
from utils.fetch_http_server import FetchHTTPServer, FetchRequestHandler
from utils.js_test_runner import run_js_case
from utils.wait import wait_for
from utils.wifi_helpers import (
    wait_for_wifi_link_stable,
    wifi_connect_was_already_satisfied,
    wifi_connection_is_active,
)


JS_CLI_LOCAL_STORAGE_PATH = (
    "/ext/apps_data/jsrunner/app.busy.cli.localstorage.json"
)


@pytest.fixture(scope="module", autouse=True)
def cli_debug(persistent_cli_connection):
    """Debug mode on for the whole CLI module, and left on afterwards.

    The flag lives in NVM and survives reboots, so a suite that turned it off would
    leave the bench without the debug-gated commands (`gpio`, `otp`, `factory_reset`).
    Re-enable on teardown as well: `test_sysctl_debug_toggle` flips it off on purpose.
    """
    persistent_cli_connection.execute_command("sysctl debug 1")
    yield
    persistent_cli_connection.execute_command("sysctl debug 1")


@pytest.fixture(scope="class")
def sl_cli():
    """CLI in 917 (sl_cli) mode, shared by the whole 917 class.

    Its own connection, not `persistent_cli_connection`: `sl_cli` is exclusive and
    a test failing inside 917 mode must not leave the shared CLI at the `917>:`
    prompt. Entering 917 mode is slow, so do it once per class, not per test — the
    commands in there are read-only and cannot interfere with each other.
    """
    cli = SimpleCLIConnection()
    if not cli.connect():
        pytest.skip("Could not connect to CLI")
    try:
        cli.enter_sl_cli()
        yield cli
    finally:
        if cli._in_sl_cli:
            cli.exit_sl_cli()
        cli.disconnect()


@pytest.fixture
def storage_dir(persistent_cli_connection):
    """Empty `/ext/cli_test`, wiped again afterwards — whatever the test did or
    left half-done. Cleanup runs even when the test fails, and starts with a
    resync so it still works if a raw-protocol test died mid-command."""
    cli = persistent_cli_connection
    path = "/ext/cli_test"

    def rm_rf(target):
        # `storage remove` only unlinks files and *empty* dirs, so walk the tree
        # depth-first (`extract` leaves a whole subtree behind in out/)
        listing = cli.execute_command(f"storage list {target}")
        for kind, name in re.findall(r"\[([DF])\]\s+(\S+)", listing):
            child = f"{target}/{name}"
            rm_rf(child) if kind == "D" else cli.execute_command(
                f"storage remove {child}"
            )
        cli.execute_command(f"storage remove {target}")

    rm_rf(path)  # a previous run may have died before its own cleanup
    cli.execute_command(f"storage mkdir {path}")
    try:
        yield path
    finally:
        resync(cli)
        rm_rf(path)


@pytest.fixture
def http_server(persistent_cli_connection):
    """HTTP server on the pytest host, reachable from the device under test."""
    host_ip = persistent_cli_connection.tn.sock.getsockname()[0]
    server = FetchHTTPServer((host_ip, 0), FetchRequestHandler)
    server_thread = threading.Thread(target=server.serve_forever, daemon=True)
    server_thread.start()
    try:
        yield server
    finally:
        server.release_stall.set()
        server.shutdown()
        server_thread.join(timeout=2)
        server.server_close()


@pytest.fixture
def device_wifi_ready(wifi_api: WifiAPI):
    """Ensure the device has a connected Wi-Fi interface and usable IP."""
    with allure.step("Ensure the device is connected to Wi-Fi"):
        initial_status = wifi_api.get_status()
        if initial_status.state == "unknown":
            try:
                initial_status = wait_for(
                    "Wi-Fi service to leave the unknown state",
                    wifi_api.get_status,
                    lambda status: status.state != "unknown",
                    timeout=10,
                    interval=0.5,
                )
            except AssertionError as error:
                pytest.fail(f"Wi-Fi service did not initialize: {error}")

        if not wifi_connection_is_active(initial_status):
            if not WIFI_SSID:
                pytest.skip(
                    "Device Wi-Fi is not ready and WIFI_SSID is not "
                    f"configured; initial status={initial_status!r}"
                )
            response = wifi_api.connect_to_test_network(timeout=30)
            if response.status_code != 200 and not (
                wifi_connect_was_already_satisfied(
                    response.status_code,
                    response.text,
                )
            ):
                pytest.skip(
                    f"Device could not connect to Wi-Fi {WIFI_SSID!r}: "
                    f"HTTP {response.status_code}, {response.text[:200]!r}"
                )
            if response.status_code != 200:
                allure.attach(
                    response.text,
                    name="Wi-Fi connect race accepted",
                    attachment_type=allure.attachment_type.TEXT,
                )

        try:
            ready_status = wait_for_wifi_link_stable(
                wifi_api,
                timeout=45,
            )
        except AssertionError as error:
            pytest.skip(f"Device Wi-Fi did not become ready: {error}")

    return ready_status


@pytest.fixture
def js_case_runner(persistent_cli_connection, storage_api, storage_dir):
    """Upload and run an isolated JavaScript assertion case on the device."""

    def runner(case_name, body, timeout=25):
        return run_js_case(
            persistent_cli_connection,
            storage_api,
            storage_dir,
            case_name,
            body,
            timeout,
        )

    return runner


@pytest.fixture
def js_local_storage_clean(storage_api):
    """Remove the CLI app's persistent localStorage before and after a test."""
    storage_api.remove_raw(JS_CLI_LOCAL_STORAGE_PATH)
    try:
        yield JS_CLI_LOCAL_STORAGE_PATH
    finally:
        storage_api.remove_raw(JS_CLI_LOCAL_STORAGE_PATH)
