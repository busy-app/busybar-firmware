"""Shared Wi-Fi lifecycle and Fetch recovery helpers for hardware tests."""

from __future__ import annotations

import json
import logging
import time
import uuid
from dataclasses import asdict, dataclass
from ipaddress import IPv4Address, IPv4Interface, ip_interface
from typing import Any

import allure
import pytest

from clients.api import APIError, WIFI_SSID, WifiAPI, WifiStatusResponse
from clients.cli import SimpleCLIConnection
from utils.wait import wait_for


WIFI_LINK_STABLE_SAMPLES = 3
WIFI_LINK_POLL_INTERVAL_SECONDS = 0.5
WIFI_CONNECTION_ACTIVE_STATES = frozenset(
    (
        "connected",
        "connecting",
        "reconnecting",
    )
)
FETCH_NETWORK_ATTEMPTS = 2
FETCH_RECOVERY_TIMEOUT_SECONDS = 100.0
FETCH_ATTEMPT_TIMEOUT_SECONDS = 25.0
FETCH_LINK_TIMEOUT_SECONDS = 10.0
FETCH_CLEANUP_TIMEOUT_SECONDS = 5.0
FETCH_DISCONNECT_TIMEOUT_SECONDS = 8
# The firmware may spend up to 30 seconds waiting for DHCP in wifi_net_up().
FETCH_RECONNECT_TIMEOUT_SECONDS = 40.0
FETCH_TRANSIENT_NETWORK_ERRORS = (
    "Error: DNS error",
    "Error: Inactivity timeout",
)

logger = logging.getLogger(__name__)


@dataclass(frozen=True)
class FetchAttemptDiagnostic:
    """One Fetch attempt plus enough context to investigate a retry."""

    attempt: int
    wifi: dict[str, Any]
    fetch_timeout_seconds: float
    fetch_output: str
    fetch_error: str | None
    cleanup_output: str
    cleanup_error: str | None
    recovery_action: str | None


@dataclass(frozen=True)
class FetchRecoveryResult:
    """Successful Fetch output and all attempts that led to it."""

    output: str
    attempts: tuple[FetchAttemptDiagnostic, ...]

    @property
    def attempt_count(self) -> int:
        return len(self.attempts)


class FetchRecoveryError(AssertionError):
    """Fetch did not recover within its explicit attempts/time budget."""

    def __init__(
        self,
        message: str,
        attempts: list[FetchAttemptDiagnostic],
    ) -> None:
        self.attempts = tuple(attempts)
        super().__init__(message)


def wifi_connection_is_active(status: WifiStatusResponse) -> bool:
    """Return whether Wi-Fi is connected or converging to connected."""
    return status.state in WIFI_CONNECTION_ACTIVE_STATES


def wifi_connect_was_already_satisfied(status_code: int, body: str) -> bool:
    """Recognize the idempotent connect race reported by the firmware API."""
    return status_code == 400 and "already connected" in body.lower()


def wait_for_wifi_state(
    wifi_api: WifiAPI,
    states: list[str],
    timeout: float = 20,
) -> str:
    """Poll Wi-Fi status until it matches one of the expected states."""
    status = wait_for(
        f"Wi-Fi state to become one of {states!r}",
        wifi_api.get_status,
        lambda value: value.state in states,
        timeout=timeout,
        interval=1,
    )
    return status.state


def _usable_ipv4_address(status: WifiStatusResponse) -> IPv4Address | None:
    ip_config = status.ip_config
    if not ip_config or not ip_config.address:
        return None

    try:
        parsed = ip_interface(ip_config.address)
    except ValueError:
        return None

    if not isinstance(parsed, IPv4Interface) or parsed.ip.is_unspecified:
        return None
    return parsed.ip


def wait_for_wifi_link_stable(
    wifi_api: WifiAPI,
    timeout: float = 30,
    *,
    expected_ssid: str | None = None,
    stable_samples: int = WIFI_LINK_STABLE_SAMPLES,
    poll_interval: float = WIFI_LINK_POLL_INTERVAL_SECONDS,
) -> WifiStatusResponse:
    """Wait for a stable connected link with the same usable IPv4 address."""
    if stable_samples < 1:
        raise ValueError("stable_samples must be at least 1")

    last_fingerprint: tuple[str | None, IPv4Address] | None = None
    observed_stable_samples = 0

    def is_stably_connected(status: WifiStatusResponse) -> bool:
        nonlocal last_fingerprint, observed_stable_samples

        address = _usable_ipv4_address(status)
        ssid_matches = expected_ssid is None or status.ssid == expected_ssid
        if status.state != "connected" or address is None or not ssid_matches:
            last_fingerprint = None
            observed_stable_samples = 0
            return False

        fingerprint = (status.ssid, address)
        if fingerprint == last_fingerprint:
            observed_stable_samples += 1
        else:
            last_fingerprint = fingerprint
            observed_stable_samples = 1

        return observed_stable_samples >= stable_samples

    return wait_for(
        "Wi-Fi link to remain connected with a stable usable IPv4 address",
        wifi_api.get_status,
        is_stably_connected,
        timeout=timeout,
        interval=poll_interval,
    )


def ensure_disconnected(wifi_api: WifiAPI, timeout: float = 20) -> None:
    """Bring the device into the disconnected state, tolerating transitions."""
    if wifi_api.get_status().state == "disconnected":
        return

    disconnect_error: APIError | None = None
    try:
        wifi_api.disconnect()
    except APIError as error:
        disconnect_error = error
        logger.warning(
            "Wi-Fi disconnect returned HTTP %s; waiting for state convergence",
            error.status_code,
        )

    try:
        wait_for_wifi_state(wifi_api, ["disconnected"], timeout=timeout)
    except AssertionError as error:
        if disconnect_error is None:
            raise
        body = disconnect_error.response.text.strip() or "(empty body)"
        raise AssertionError(
            f"{error}; disconnect request failed with HTTP "
            f"{disconnect_error.status_code}: {body}"
        ) from error


def connect_to_test_network_or_fail(
    wifi_api: WifiAPI,
    timeout: float = 30,
) -> WifiStatusResponse:
    """Connect to the test SSID and wait for a stable usable link."""
    deadline = time.monotonic() + timeout
    initial_status = wifi_api.get_status()
    if wifi_connection_is_active(initial_status):
        logger.info(
            "Wi-Fi is already active (%s); waiting for a stable link",
            initial_status.state,
        )
    else:
        response = wifi_api.connect_to_test_network(timeout=timeout)
        already_connected = wifi_connect_was_already_satisfied(
            response.status_code,
            response.text,
        )
        if response.status_code != 200 and not already_connected:
            body = response.text.strip() or "(empty body)"
            pytest.fail(
                f"POST /api/wifi/connect to {WIFI_SSID!r} failed: "
                f"HTTP {response.status_code} — {body}"
            )
        if response.status_code != 200:
            logger.info(
                "Wi-Fi became active before POST /api/wifi/connect completed; "
                "waiting for a stable link"
            )

    remaining = deadline - time.monotonic()
    if remaining <= 0:
        pytest.fail(
            f"Connected to {WIFI_SSID!r}, but the {timeout:.1f}s connection "
            "budget was exhausted before link validation"
        )
    return wait_for_wifi_link_stable(
        wifi_api,
        timeout=remaining,
        expected_ssid=WIFI_SSID or None,
    )


def _attempts_json(attempts: list[FetchAttemptDiagnostic]) -> str:
    return json.dumps([asdict(attempt) for attempt in attempts], indent=2)


def fetch_over_wifi_with_recovery(
    cli: SimpleCLIConnection,
    wifi_api: WifiAPI,
    url: str,
    *,
    max_attempts: int = FETCH_NETWORK_ATTEMPTS,
    total_timeout: float = FETCH_RECOVERY_TIMEOUT_SECONDS,
    fetch_timeout: float = FETCH_ATTEMPT_TIMEOUT_SECONDS,
    link_timeout: float = FETCH_LINK_TIMEOUT_SECONDS,
    cleanup_timeout: float = FETCH_CLEANUP_TIMEOUT_SECONDS,
    disconnect_timeout: float = FETCH_DISCONNECT_TIMEOUT_SECONDS,
    reconnect_timeout: float = FETCH_RECONNECT_TIMEOUT_SECONDS,
) -> FetchRecoveryResult:
    """Fetch a URL with bounded retries for known transient Wi-Fi errors."""
    if max_attempts < 1:
        raise ValueError("max_attempts must be at least 1")
    if (
        min(
            total_timeout,
            fetch_timeout,
            link_timeout,
            cleanup_timeout,
            disconnect_timeout,
            reconnect_timeout,
        )
        <= 0
    ):
        raise ValueError("Fetch recovery timeouts must be positive")

    deadline = time.monotonic() + total_timeout
    attempts: list[FetchAttemptDiagnostic] = []

    for attempt_number in range(1, max_attempts + 1):
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            break

        status = wait_for_wifi_link_stable(
            wifi_api,
            timeout=min(link_timeout, remaining),
        )
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            break

        attempt_fetch_timeout = min(
            fetch_timeout,
            max(remaining - cleanup_timeout, 0.1),
        )
        output_path = f"/ext/_test_fetch_{uuid.uuid4().hex[:8]}.json"
        output = ""
        fetch_error = None
        cleanup_output = ""
        cleanup_error = None

        with allure.step(f"Fetch attempt {attempt_number}/{max_attempts}"):
            try:
                output = cli.execute_command(
                    f"fetch {url} -o {output_path}",
                    timeout=attempt_fetch_timeout,
                    slow_command=True,
                )
            except Exception as error:
                fetch_error = f"{type(error).__name__}: {error}"
            finally:
                cleanup_remaining = deadline - time.monotonic()
                try:
                    cleanup_output = cli.execute_command(
                        f"storage remove {output_path}",
                        timeout=min(cleanup_timeout, max(cleanup_remaining, 0.1)),
                    )
                except Exception as error:
                    cleanup_error = f"{type(error).__name__}: {error}"

            completed = "Downloaded: 100%" in output
            is_transient = any(
                error in output for error in FETCH_TRANSIENT_NETWORK_ERRORS
            )
            will_reconnect = (
                not fetch_error
                and not cleanup_error
                and not completed
                and is_transient
                and attempt_number < max_attempts
            )
            diagnostic = FetchAttemptDiagnostic(
                attempt=attempt_number,
                wifi=status.model_dump(mode="json"),
                fetch_timeout_seconds=attempt_fetch_timeout,
                fetch_output=output,
                fetch_error=fetch_error,
                cleanup_output=cleanup_output,
                cleanup_error=cleanup_error,
                recovery_action=(
                    "disconnect_and_reconnect_test_network" if will_reconnect else None
                ),
            )
            attempts.append(diagnostic)
            allure.attach(
                json.dumps(asdict(diagnostic), indent=2),
                name=f"Fetch network attempt {attempt_number}",
                attachment_type=allure.attachment_type.JSON,
            )

            if fetch_error or cleanup_error:
                raise FetchRecoveryError(
                    "Fetch attempt raised an unexpected command/cleanup error; "
                    f"attempts={_attempts_json(attempts)}",
                    attempts,
                )

            if completed:
                if attempt_number > 1:
                    logger.warning(
                        "Fetch recovered after Wi-Fi reconnect on attempt %s",
                        attempt_number,
                    )
                    allure.dynamic.tag("FETCH_RECOVERED_AFTER_WIFI_RECONNECT")
                return FetchRecoveryResult(output=output, attempts=tuple(attempts))

            if not is_transient:
                raise FetchRecoveryError(
                    "Fetch failed with a non-transient error; "
                    f"attempts={_attempts_json(attempts)}",
                    attempts,
                )

        if will_reconnect:
            logger.warning(
                "Fetch attempt %s hit a transient network error; reconnecting Wi-Fi",
                attempt_number,
            )
            allure.dynamic.tag("FETCH_WIFI_RECONNECT_REQUIRED")
            with allure.step("Reconnect Wi-Fi after transient Fetch failure"):
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    break
                ensure_disconnected(
                    wifi_api,
                    timeout=min(disconnect_timeout, remaining),
                )

                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    break
                connect_to_test_network_or_fail(
                    wifi_api,
                    timeout=min(reconnect_timeout, remaining),
                )

    reason = (
        f"within {max_attempts} attempts"
        if len(attempts) >= max_attempts
        else f"within the {total_timeout:.1f}s recovery budget"
    )
    raise FetchRecoveryError(
        f"Wi-Fi data path did not recover {reason}; "
        f"attempts={_attempts_json(attempts)}",
        attempts,
    )
