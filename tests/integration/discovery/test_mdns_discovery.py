"""BusyBar DNS-SD discovery over the local USB network link."""

from __future__ import annotations

import threading
import time
from uuid import uuid4

import allure
import pytest

from clients.api import SettingsAPI
from zeroconf import IPVersion, ServiceBrowser, ServiceInfo, ServiceListener, Zeroconf


_BUSYBAR_SERVICE_TYPE = "_busybar._tcp.local."
_BUSYBAR_DISCOVERY_PORT = 0
_HTTP_SERVICE_TYPE = "_http._tcp.local."
_HTTP_INSTANCE = "httpd"
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
        with allure.step(f"Browse for {_BUSYBAR_SERVICE_TYPE}"):
            service = _discover(
                mdns_interface_ip,
                _BUSYBAR_SERVICE_TYPE,
                discovery_instance,
            )

        with allure.step("Verify the discovered identity and local API address"):
            expected_name = f"{discovery_instance}.{_BUSYBAR_SERVICE_TYPE}"
            addresses = service.parsed_addresses(IPVersion.V4Only)
            assert (
                service.name == expected_name
            ), f"Expected service instance {expected_name!r}, got {service.name!r}"
            assert mdns_device_ip in addresses, (
                f"Expected discovery address {mdns_device_ip!r}, " f"got {addresses!r}"
            )

    @allure.title("BusyBar identity service uses the discovery-only port")
    def test_busybar_service_uses_discovery_only_port(
        self,
        mdns_interface_ip: str,
        discovery_instance: str,
    ):
        with allure.step("Resolve the BusyBar DNS-SD service"):
            service = _discover(
                mdns_interface_ip,
                _BUSYBAR_SERVICE_TYPE,
                discovery_instance,
            )

        with allure.step("Verify the identity record uses marker port 0"):
            assert service.port == _BUSYBAR_DISCOVERY_PORT, (
                f"Expected {_BUSYBAR_SERVICE_TYPE} discovery-only port "
                f"{_BUSYBAR_DISCOVERY_PORT}, got {service.port}"
            )

    @allure.title("HTTP service advertises a reachable local API endpoint")
    def test_http_service_advertises_local_api_endpoint(
        self,
        mdns_interface_ip: str,
        mdns_device_ip: str,
        discovery_instance: str,
        web_session,
    ):
        with allure.step("Resolve the BusyBar identity service"):
            busybar_service = _discover(
                mdns_interface_ip,
                _BUSYBAR_SERVICE_TYPE,
                discovery_instance,
            )

        with allure.step("Resolve the HTTP DNS-SD service"):
            http_service = _discover(
                mdns_interface_ip,
                _HTTP_SERVICE_TYPE,
                _HTTP_INSTANCE,
            )

        with allure.step("Verify the HTTP record points to the local API"):
            expected_name = f"{_HTTP_INSTANCE}.{_HTTP_SERVICE_TYPE}"
            addresses = http_service.parsed_addresses(IPVersion.V4Only)
            advertised_path = http_service.properties.get(b"path")
            assert http_service.name == expected_name, (
                f"Expected HTTP service instance {expected_name!r}, "
                f"got {http_service.name!r}"
            )
            assert (
                http_service.server.casefold() == busybar_service.server.casefold()
            ), (
                "BusyBar identity and HTTP services point to different hosts: "
                f"busybar={busybar_service.server!r}, "
                f"http={http_service.server!r}"
            )
            assert http_service.port == _LOCAL_API_PORT, (
                f"Expected {_HTTP_SERVICE_TYPE} port {_LOCAL_API_PORT}, "
                f"got {http_service.port}"
            )
            assert mdns_device_ip in addresses, (
                f"Expected HTTP discovery address {mdns_device_ip!r}, "
                f"got {addresses!r}"
            )
            assert advertised_path == b"/", (
                f"Expected HTTP TXT path=b'/', "
                f"got {advertised_path!r} from {http_service.properties!r}"
            )

        with allure.step("Verify the advertised HTTP endpoint responds"):
            endpoint = (
                f"http://{mdns_device_ip}:{http_service.port}"
                f"{advertised_path.decode()}"
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

        with allure.step("Resolve the BusyBar TXT record"):
            service = _discover(
                mdns_interface_ip,
                _BUSYBAR_SERVICE_TYPE,
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
                _BUSYBAR_SERVICE_TYPE,
                discovery_instance,
                timeout=10,
            )

        with allure.step("Verify updated metadata with stable identity"):
            advertised_name = service.properties.get(b"name", b"").decode()
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
                service.name == f"{discovery_instance}.{_BUSYBAR_SERVICE_TYPE}"
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
                _BUSYBAR_SERVICE_TYPE,
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
                _BUSYBAR_SERVICE_TYPE,
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
