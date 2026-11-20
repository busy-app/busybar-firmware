"""Core authorization checks for access tokens through the local API."""

from __future__ import annotations

from collections.abc import Callable, Iterator
from urllib.parse import urlencode
from uuid import uuid4

import allure
import pytest
import requests

from clients.api import MintedAccessToken, SettingsAPI
from utils.simple_websocket import websocket_upgrade, websocket_url
from utils.wait import wait_for


pytestmark = [
    pytest.mark.bsb_metadata(
        suite="Contract",
        level="Component",
        intent="Security",
        interfaces=("Device API",),
        transports=("LAN REST",),
    ),
]


_ADMIN_ACCESS_KEY = "73916482"
_CHANGED_ACCESS_KEY = "NewKeyA9" * 4
_MAX_LENGTH_ACCESS_KEY = "AbCd0123" * 4
_TOKEN_HEADER = "X-API-Token"


def _name(prefix: str) -> str:
    return f"{prefix}-{uuid4().hex[:10]}"


def _headers(token: str) -> dict[str, str]:
    return {_TOKEN_HEADER: token}


def _websocket_status(web_base_url: str, credential: str) -> int:
    url = websocket_url(web_base_url, "/api/status/ws")
    query = urlencode({"x-api-token": credential})
    return websocket_upgrade(f"{url}?{query}").status_code


def _assert_forbidden(response: requests.Response, case: str) -> None:
    with allure.step(f"Verify {case} is rejected"):
        assert response.status_code == 403, (
            f"Expected HTTP 403 for {case}, got {response.status_code}: "
            f"{response.text!r}"
        )


@pytest.fixture
def local_token_api(
    settings_api: SettingsAPI,
) -> Iterator[SettingsAPI]:
    """Configure key mode for explicit local-API credential checks."""
    original_access = settings_api.get_access()
    with allure.step("Configure key access mode"):
        settings_api.set_access("key", _ADMIN_ACCESS_KEY)

    yield settings_api

    with allure.step("Restore original access mode"):
        if original_access.mode == "key":
            # GET /api/access intentionally never exposes the previous key.
            settings_api.set_access("enabled")
        else:
            settings_api.set_access(original_access.mode)


@allure.epic("BusyBar & Local Control")
@allure.feature("Device HTTP API")
@allure.story("API token authentication")
@pytest.mark.api
@pytest.mark.frontend
class TestAPITokenAuthentication:
    @allure.title("Local API request with an explicit invalid token is rejected")
    @pytest.mark.parametrize(
        ("credential", "case"),
        [
            ("not-a-valid-token", "invalid token"),
            ("x" * 31, "31-character token"),
            ("x" * 33, "33-character token"),
        ],
    )
    def test_protected_endpoint_rejects_invalid_credentials(
        self,
        local_token_api: SettingsAPI,
        credential: str,
        case: str,
    ):
        with allure.step(f"Request protected endpoint with {case}"):
            response = local_token_api.get_raw(
                "/api/status",
                headers=_headers(credential),
            )

        _assert_forbidden(response, case)

    @allure.title("Valid token authorizes local API and updates last_used_at")
    def test_valid_token_authorizes_and_records_usage(
        self,
        settings_api: SettingsAPI,
        local_token_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
    ):
        with allure.step("Create an access token"):
            token = access_token_factory(_name("valid-auth"))
            initial_last_used = int(token.last_used_at)

        with allure.step("Authorize a protected request with the token"):
            response = local_token_api.get_raw(
                "/api/status",
                headers=_headers(token.token),
            )
            assert response.status_code == 200, (
                f"Expected valid token to return HTTP 200, got "
                f"{response.status_code}: {response.text!r}"
            )

        with allure.step("Verify last_used_at is updated"):
            updated_last_used = wait_for(
                "the token last_used_at timestamp to advance",
                lambda: next(
                    entry.last_used_at
                    for entry in settings_api.list_access_tokens().tokens
                    if entry.short_id == token.short_id
                ),
                lambda value: int(value) > initial_last_used,
                timeout=5,
                interval=0.25,
            )
            assert int(updated_last_used) > initial_last_used, (
                f"Expected last_used_at > {initial_last_used}, "
                f"got {updated_last_used!r}"
            )

    @allure.title("Token with an appended suffix is rejected")
    def test_token_suffix_is_rejected(
        self,
        local_token_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
    ):
        with allure.step("Create a valid 32-character token"):
            token = access_token_factory(_name("suffix"))

        with allure.step("Append a suffix and request a protected endpoint"):
            response = local_token_api.get_raw(
                "/api/status",
                headers=_headers(f"{token.token}suffix"),
            )

        _assert_forbidden(response, "valid token with appended suffix")

    @allure.title(
        "32-character credentials require an exact match over HTTP and WebSocket"
    )
    @pytest.mark.parametrize("transport", ["http", "websocket"])
    @pytest.mark.parametrize("credential_kind", ["access-key", "token"])
    @pytest.mark.parametrize(
        "suffix", ["", "X", "X" * 32], ids=["exact-32", "suffix-33", "suffix-64"]
    )
    def test_credential_length_is_not_truncated(
        self,
        local_token_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
        web_base_url: str,
        transport: str,
        credential_kind: str,
        suffix: str,
    ):
        with allure.step("Configure a valid 32-character credential"):
            if credential_kind == "access-key":
                credential = _MAX_LENGTH_ACCESS_KEY
                local_token_api.set_access("key", credential)
            else:
                credential = access_token_factory(_name("length-boundary")).token
            assert (
                len(credential) == 32
            ), f"Unexpected credential length: {len(credential)}"

        with allure.step(
            "Authenticate with the exact credential or an appended suffix"
        ):
            supplied = credential + suffix
            if transport == "websocket":
                status = _websocket_status(web_base_url, supplied)
                success_status = 101
            else:
                response = local_token_api.get_raw(
                    "/api/status", headers=_headers(supplied)
                )
                status = response.status_code
                success_status = 200

        with allure.step("Verify that only the exact credential is accepted"):
            expected_status = 403 if suffix else success_status
            assert status == expected_status, (
                f"Expected {credential_kind} of length {len(supplied)} over "
                f"{transport} to return HTTP {expected_status}, got {status}"
            )

    @allure.title("Alphanumeric access keys are case-sensitive")
    def test_access_key_is_case_sensitive(self, local_token_api: SettingsAPI):
        with allure.step("Configure a mixed-case key and verify it works"):
            local_token_api.set_access("key", _MAX_LENGTH_ACCESS_KEY)
            response = local_token_api.get_raw(
                "/api/status", headers=_headers(_MAX_LENGTH_ACCESS_KEY)
            )
            assert response.status_code == 200, (
                f"Expected the original key to authenticate, got "
                f"HTTP {response.status_code}: {response.text!r}"
            )

        with allure.step("Reject the same key with its letter case changed"):
            response = local_token_api.get_raw(
                "/api/status", headers=_headers(_MAX_LENGTH_ACCESS_KEY.swapcase())
            )
            _assert_forbidden(response, "access key with changed letter case")

    @allure.title("Token may list token metadata")
    def test_token_can_list_tokens(
        self,
        local_token_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
    ):
        with allure.step("Create actor and peer tokens"):
            actor = access_token_factory(_name("list-actor"))
            other = access_token_factory(_name("list-peer"))

        with allure.step("List tokens using token authorization"):
            response = local_token_api.list_access_tokens_raw(
                headers=_headers(actor.token)
            )
            assert response.status_code == 200, (
                f"Expected token list HTTP 200, got {response.status_code}: "
                f"{response.text!r}"
            )

        with allure.step("Verify metadata is returned without full tokens"):
            entries = response.json()["tokens"]
            listed_ids = {entry["short_id"] for entry in entries}
            assert {
                actor.short_id,
                other.short_id,
            } <= listed_ids, f"Token list is missing expected entries: {listed_ids!r}"
            assert all(
                "token" not in entry for entry in entries
            ), f"GET returned full token data: {entries!r}"

    @allure.title("Token cannot mint another token")
    def test_token_cannot_mint_token(
        self,
        settings_api: SettingsAPI,
        local_token_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
    ):
        with allure.step("Create the actor token and capture existing IDs"):
            actor = access_token_factory(_name("mint-actor"))
            before_ids = {
                entry.short_id for entry in settings_api.list_access_tokens().tokens
            }

        unexpected_short_id: str | None = None
        try:
            with allure.step("Attempt to mint using token authorization"):
                response = local_token_api.mint_access_token_raw(
                    json={"name": _name("unauthorized-mint")},
                    headers=_headers(actor.token),
                )
                if response.ok:
                    unexpected_short_id = response.json().get("short_id")
        finally:
            if unexpected_short_id:
                settings_api.revoke_access_token_raw(unexpected_short_id)

        _assert_forbidden(response, "token minting another token")
        with allure.step("Verify no token was added"):
            after_ids = {
                entry.short_id for entry in settings_api.list_access_tokens().tokens
            }
            assert after_ids == before_ids, (
                f"Unauthorized mint changed tokens: before={before_ids!r}, "
                f"after={after_ids!r}"
            )

    @allure.title("Token cannot revoke another token")
    def test_token_cannot_revoke_other_token(
        self,
        settings_api: SettingsAPI,
        local_token_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
    ):
        with allure.step("Create actor and peer tokens"):
            actor = access_token_factory(_name("revoke-actor"))
            other = access_token_factory(_name("revoke-peer"))

        with allure.step("Attempt to revoke the peer token"):
            response = local_token_api.revoke_access_token_raw(
                other.short_id,
                headers=_headers(actor.token),
            )
        _assert_forbidden(response, "token revoking another token")

        with allure.step("Verify both tokens remain available"):
            remaining_ids = {
                entry.short_id for entry in settings_api.list_access_tokens().tokens
            }
            assert {
                actor.short_id,
                other.short_id,
            } <= remaining_ids, f"Unauthorized revoke changed tokens: {remaining_ids!r}"

    @allure.title("Token cannot revoke all tokens")
    def test_token_cannot_revoke_all_tokens(
        self,
        settings_api: SettingsAPI,
        local_token_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
    ):
        with allure.step("Create actor and peer tokens"):
            actor = access_token_factory(_name("revoke-all-actor"))
            other = access_token_factory(_name("revoke-all-peer"))

        with allure.step("Attempt to revoke all tokens"):
            response = local_token_api.revoke_all_access_tokens_raw(
                headers=_headers(actor.token)
            )
        _assert_forbidden(response, "token revoking all tokens")

        with allure.step("Verify both tokens remain available"):
            remaining_ids = {
                entry.short_id for entry in settings_api.list_access_tokens().tokens
            }
            assert {
                actor.short_id,
                other.short_id,
            } <= remaining_ids, (
                f"Unauthorized revoke-all changed tokens: {remaining_ids!r}"
            )

    @allure.title("Token may revoke itself and is rejected afterwards")
    def test_token_can_revoke_itself(
        self,
        settings_api: SettingsAPI,
        local_token_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
    ):
        with allure.step("Create a token"):
            token = access_token_factory(_name("self-revoke"))
            token_headers = _headers(token.token)

        with allure.step("Revoke the token using its own credential"):
            revoke = local_token_api.revoke_access_token_raw(
                token.short_id,
                headers=token_headers,
            )
            assert revoke.status_code == 200, (
                f"Expected self-revoke HTTP 200, got {revoke.status_code}: "
                f"{revoke.text!r}"
            )

        with allure.step("Request a protected endpoint with the revoked token"):
            protected = local_token_api.get_raw(
                "/api/status",
                headers=token_headers,
            )
        _assert_forbidden(protected, "a revoked token")

        with allure.step("Verify the revoked token is not listed"):
            listed_ids = {
                entry.short_id for entry in settings_api.list_access_tokens().tokens
            }
            assert (
                token.short_id not in listed_ids
            ), f"Self-revoked token is still listed: {listed_ids!r}"

    @allure.title("Administrator access key can manage tokens through local API")
    @pytest.mark.parametrize(
        "access_key",
        [_ADMIN_ACCESS_KEY, _MAX_LENGTH_ACCESS_KEY],
        ids=["legacy-numeric", "maximum-alphanumeric"],
    )
    def test_admin_access_key_can_manage_tokens(
        self,
        settings_api: SettingsAPI,
        local_token_api: SettingsAPI,
        access_key: str,
    ):
        local_token_api.set_access("key", access_key)
        admin_headers = _headers(access_key)
        minted: MintedAccessToken | None = None
        try:
            with allure.step("Mint a token using the access key"):
                minted = local_token_api.mint_access_token(
                    _name("admin"),
                    headers=admin_headers,
                )

            with allure.step("List tokens using the access key"):
                listed = local_token_api.list_access_tokens(
                    headers=admin_headers
                ).tokens
                listed_ids = {entry.short_id for entry in listed}
                assert minted.short_id in listed_ids, (
                    f"Admin-minted token {minted.short_id!r} was not listed: "
                    f"{listed_ids!r}"
                )

            with allure.step("Revoke the token using the access key"):
                revoke = local_token_api.revoke_access_token(
                    minted.short_id,
                    headers=admin_headers,
                )
                assert (
                    revoke.result == "OK"
                ), f"Expected admin revoke result 'OK', got {revoke.result!r}"
        finally:
            if minted is not None:
                settings_api.revoke_access_token_raw(minted.short_id)

    @allure.title(
        "Changing to a 32-character alphanumeric key does not invalidate a token"
    )
    def test_key_change_keeps_token_valid(
        self,
        settings_api: SettingsAPI,
        local_token_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
    ):
        with allure.step("Create a token under the initial numeric key"):
            token = access_token_factory(_name("key-change"))

        with allure.step("Change to a 32-character alphanumeric access key"):
            settings_api.set_access("key", _CHANGED_ACCESS_KEY)

        with allure.step("Authorize using the token created before the key change"):
            response = local_token_api.get_raw(
                "/api/status",
                headers=_headers(token.token),
            )
            assert response.status_code == 200, (
                f"Expected token to survive key change, got HTTP "
                f"{response.status_code}: {response.text!r}"
            )


@allure.epic("BusyBar & Local Control")
@allure.feature("Device HTTP API")
@allure.story("API access-key persistence")
@pytest.mark.api
@pytest.mark.frontend
class TestAPIAccessKeyPersistence:
    @allure.title("A 32-character alphanumeric key survives a reboot")
    def test_maximum_length_key_survives_reboot(
        self,
        local_token_api: SettingsAPI,
        persistent_cli_connection,
        web_base_url: str,
    ):
        with allure.step("Configure the maximum-length alphanumeric access key"):
            local_token_api.set_access("key", _MAX_LENGTH_ACCESS_KEY)

        with allure.step("Reboot and wait for the API"):
            recovered = persistent_cli_connection.reboot_and_wait_for_api(
                web_base_url, timeout=60
            )
            assert (
                recovered
            ), "Device did not recover after the access-key persistence reboot"

        with allure.step("Verify the persisted key remains valid"):
            access = local_token_api.get_access()
            assert (
                access.mode == "key" and access.key_valid
            ), f"The persisted access key was discarded: {access}"
            response = local_token_api.get_raw(
                "/api/status", headers=_headers(_MAX_LENGTH_ACCESS_KEY)
            )
            assert response.status_code == 200, (
                f"Persisted key failed HTTP authentication: "
                f"HTTP {response.status_code}, body={response.text!r}"
            )
            status = _websocket_status(web_base_url, _MAX_LENGTH_ACCESS_KEY)
            assert (
                status == 101
            ), f"Persisted key failed WebSocket authentication: HTTP {status}"
