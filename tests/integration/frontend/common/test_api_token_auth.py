"""Core authorization checks for access tokens through the local API."""

from __future__ import annotations

from collections.abc import Callable, Iterator
from uuid import uuid4

import allure
import pytest
import requests

from clients.api import MintedAccessToken, SettingsAPI
from utils.wait import wait_for


_ADMIN_ACCESS_KEY = "73916482"
_CHANGED_ACCESS_KEY = "28461739"
_TOKEN_HEADER = "X-API-Token"


def _name(prefix: str) -> str:
    return f"{prefix}-{uuid4().hex[:10]}"


def _headers(token: str) -> dict[str, str]:
    return {_TOKEN_HEADER: token}


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


@allure.feature("5. Web Frontend")
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
    def test_admin_access_key_can_manage_tokens(
        self,
        settings_api: SettingsAPI,
        local_token_api: SettingsAPI,
    ):
        admin_headers = _headers(_ADMIN_ACCESS_KEY)
        minted: MintedAccessToken | None = None
        try:
            with allure.step("Mint a token using the numeric access key"):
                minted = local_token_api.mint_access_token(
                    _name("admin"),
                    headers=admin_headers,
                )

            with allure.step("List tokens using the numeric access key"):
                listed = local_token_api.list_access_tokens(
                    headers=admin_headers
                ).tokens
                listed_ids = {entry.short_id for entry in listed}
                assert minted.short_id in listed_ids, (
                    f"Admin-minted token {minted.short_id!r} was not listed: "
                    f"{listed_ids!r}"
                )

            with allure.step("Revoke the token using the numeric access key"):
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

    @allure.title("Changing the numeric key does not invalidate a token")
    def test_key_change_keeps_token_valid(
        self,
        settings_api: SettingsAPI,
        local_token_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
    ):
        with allure.step("Create a token under the initial numeric key"):
            token = access_token_factory(_name("key-change"))

        with allure.step("Change the numeric access key"):
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
