"""BusyBar DNS-SD discovery over the local network link."""

from __future__ import annotations

import threading
import time
from uuid import uuid4

import allure
import pytest

from clients.api import SettingsAPI
from zeroconf import IPVersion, ServiceBrowser, ServiceInfo, ServiceListener, Zeroconf


_DISCOVERY_SERVICE_TYPE = "_http._tcp.local."
_LOCAL_API_PORT = 80


class _ExpectedServiceListener(ServiceListener):
    def __init__(self, expected_instance: str):
        self.expected_instance = expected_instance.casefold()
        self.discovered: dict[str, str] = {}
        self.found = threading.Event()

    def _record(self, name: str) -> None:
        self.discovered[name.casefold()] = name
        if name.casefold() == self.expected_instance:
            self.found.set()

    def add_service(
        self,
        _zeroconf: Zeroconf,
        _service_type: str,
        name: str,
    ) -> None:
        self._record(name)

    def update_service(
        self,
        _zeroconf: Zeroconf,
        _service_type: str,
        name: str,
    ) -> None:
        self._record(name)

    def remove_service(
        self,
        _zeroconf: Zeroconf,
        _service_type: str,
        name: str,
    ) -> None:
        self.discovered.pop(name.casefold(), None)


def _discover(
    mdns_interface_ip: str,
    service_type: str,
    instance: str,
    timeout: float = 5,
) -> ServiceInfo:
    expected_instance = f"{instance}.{service_type}"
    listener = _ExpectedServiceListener(expected_instance)
    zeroconf = Zeroconf(
        interfaces=[mdns_interface_ip],
        ip_version=IPVersion.V4Only,
    )
    browser = ServiceBrowser(zeroconf, service_type, listener)
    deadline = time.monotonic() + timeout

    try:
        if not listener.found.wait(timeout):
            raise TimeoutError(
                f"Timed out browsing for {expected_instance} on "
                f"{mdns_interface_ip}; "
                f"discovered={sorted(listener.discovered.values())!r}"
            )

        actual_instance = listener.discovered[expected_instance.casefold()]
        remaining_ms = max(1, int((deadline - time.monotonic()) * 1000))
        service = zeroconf.get_service_info(
            service_type,
            actual_instance,
            timeout=remaining_ms,
        )
        if service is None:
            raise TimeoutError(
                f"Discovered {actual_instance!r} but did not resolve its "
                f"SRV/TXT/A records within {timeout}s"
            )
        return service
    finally:
        browser.cancel()
        zeroconf.close()


@allure.feature("5. Web Frontend")
@allure.story("Local API discovery")
@pytest.mark.frontend
@pytest.mark.mdns
class TestMdnsDiscovery:
    @allure.title("Device is discoverable by its persistent USB MAC identity")
    def test_device_is_discoverable(
        self,
        mdns_interface_ip: str,
        mdns_device_ip: str,
        discovery_instance: str,
    ):
        with allure.step(f"Browse for {_DISCOVERY_SERVICE_TYPE}"):
            service = _discover(
                mdns_interface_ip,
                _DISCOVERY_SERVICE_TYPE,
                discovery_instance,
            )

        with allure.step("Verify the discovered identity and local API address"):
            expected_name = f"{discovery_instance}.{_DISCOVERY_SERVICE_TYPE}"
            addresses = service.parsed_addresses(IPVersion.V4Only)
            assert (
                service.name == expected_name
            ), f"Expected service instance {expected_name!r}, got {service.name!r}"
            assert mdns_device_ip in addresses, (
                f"Expected discovery address {mdns_device_ip!r}, " f"got {addresses!r}"
            )

    @allure.title("BusyBar discovery service uses the local API port")
    def test_discovery_service_uses_local_api_port(
        self,
        mdns_interface_ip: str,
        discovery_instance: str,
    ):
        with allure.step("Resolve the unified BusyBar HTTP DNS-SD service"):
            service = _discover(
                mdns_interface_ip,
                _DISCOVERY_SERVICE_TYPE,
                discovery_instance,
            )

        with allure.step("Verify the service uses the local API port"):
            assert service.port == _LOCAL_API_PORT, (
                f"Expected {_DISCOVERY_SERVICE_TYPE} port {_LOCAL_API_PORT}, "
                f"got {service.port}"
            )

    @allure.title("HTTP service advertises a reachable local API endpoint")
    def test_http_service_advertises_local_api_endpoint(
        self,
        mdns_interface_ip: str,
        mdns_device_ip: str,
        discovery_instance: str,
        web_session,
    ):
        with allure.step("Resolve the unified BusyBar HTTP DNS-SD service"):
            service = _discover(
                mdns_interface_ip,
                _DISCOVERY_SERVICE_TYPE,
                discovery_instance,
            )

        with allure.step("Verify the HTTP record points to the local API"):
            expected_name = f"{discovery_instance}.{_DISCOVERY_SERVICE_TYPE}"
            addresses = service.parsed_addresses(IPVersion.V4Only)
            advertised_path = service.properties.get(b"path")
            assert service.name == expected_name, (
                f"Expected HTTP service instance {expected_name!r}, "
                f"got {service.name!r}"
            )
            assert service.port == _LOCAL_API_PORT, (
                f"Expected {_DISCOVERY_SERVICE_TYPE} port {_LOCAL_API_PORT}, "
                f"got {service.port}"
            )
            assert mdns_device_ip in addresses, (
                f"Expected HTTP discovery address {mdns_device_ip!r}, "
                f"got {addresses!r}"
            )
            assert advertised_path == b"/", (
                f"Expected HTTP TXT path=b'/', "
                f"got {advertised_path!r} from {service.properties!r}"
            )

        with allure.step("Verify the advertised HTTP endpoint responds"):
            endpoint = (
                f"http://{mdns_device_ip}:{service.port}" f"{advertised_path.decode()}"
            )
            response = web_session.get(endpoint)
            assert response.status_code == 200, (
                f"Expected discovered endpoint {endpoint!r} to return HTTP 200, "
                f"got {response.status_code}: {response.text[:200]!r}"
            )

    @allure.title("Discovery publishes the device name without display quoting")
    def test_service_advertises_device_name(
        self,
        mdns_interface_ip: str,
        discovery_instance: str,
        settings_api: SettingsAPI,
    ):
        with allure.step("Read the configured device name"):
            expected_name = settings_api.get_name().name

        with allure.step("Resolve the unified BusyBar HTTP TXT record"):
            service = _discover(
                mdns_interface_ip,
                _DISCOVERY_SERVICE_TYPE,
                discovery_instance,
            )

        with allure.step("Verify TXT name matches the configured name exactly"):
            advertised_name = service.properties.get(b"name")
            assert advertised_name == expected_name.encode(), (
                f"Expected TXT name={expected_name!r}, "
                f"got {advertised_name!r} from {service.properties!r}"
            )

    @allure.title("Changing the device name reannounces discovery metadata")
    @pytest.mark.usefixtures("preserve_device_name")
    def test_name_change_is_reannounced(
        self,
        mdns_interface_ip: str,
        discovery_instance: str,
        settings_api: SettingsAPI,
    ):
        new_name = f"Discovery {uuid4().hex[:8]}"

        with allure.step(f"Change the device name to {new_name!r}"):
            settings_api.set_name(new_name)

        with allure.step("Discover the service after the name change"):
            service = _discover(
                mdns_interface_ip,
                _DISCOVERY_SERVICE_TYPE,
                discovery_instance,
                timeout=10,
            )

        with allure.step("Verify updated metadata with stable identity"):
            advertised_name_bytes = service.properties.get(b"name")
            advertised_name = (advertised_name_bytes or b"").decode()
            expected_target = "".join(
                character.lower() for character in new_name if character.isalnum()
            )
            assert (
                advertised_name == new_name
            ), f"Expected reannounced name {new_name!r}, got {advertised_name!r}"
            assert service.server.lower() == f"{expected_target}.local.", (
                f"Expected renamed host {expected_target!r}.local., "
                f"got {service.server!r}"
            )
            assert (
                service.name == f"{discovery_instance}.{_DISCOVERY_SERVICE_TYPE}"
            ), f"Discovery identity changed after rename: {service.name!r}"

    @allure.title("Discovery identity survives a device reboot")
    @pytest.mark.long_running
    def test_identity_survives_reboot(
        self,
        mdns_interface_ip: str,
        discovery_instance: str,
        persistent_cli_connection,
        web_base_url: str,
    ):
        with allure.step("Discover the device before reboot"):
            before = _discover(
                mdns_interface_ip,
                _DISCOVERY_SERVICE_TYPE,
                discovery_instance,
            )

        with allure.step("Reboot the device and wait for the local API"):
            recovered = persistent_cli_connection.reboot_and_wait_for_api(
                web_base_url,
                timeout=60,
            )
            assert recovered, "Device did not recover after discovery reboot"

        with allure.step("Discover the device again after reboot"):
            after = _discover(
                mdns_interface_ip,
                _DISCOVERY_SERVICE_TYPE,
                discovery_instance,
                timeout=15,
            )

        with allure.step("Verify discovery identity and address are stable"):
            before_addresses = before.parsed_addresses(IPVersion.V4Only)
            after_addresses = after.parsed_addresses(IPVersion.V4Only)
            assert after.name == before.name, (
                f"Discovery identity changed across reboot: "
                f"before={before.name!r}, after={after.name!r}"
            )
            assert after_addresses == before_addresses, (
                f"Discovery addresses changed across reboot: "
                f"before={before_addresses!r}, after={after_addresses!r}"
            )
