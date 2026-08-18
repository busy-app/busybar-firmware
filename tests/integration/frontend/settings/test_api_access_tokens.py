"""HTTP API contract and persistence tests for access tokens."""

from __future__ import annotations

import time
from collections.abc import Callable
from uuid import uuid4

import allure
import pytest

from clients.api import MintedAccessToken, SettingsAPI


_CURRENT_FLUSH_PERIOD_S = 30
_TOKEN_HEADER = "X-API-Token"


def _token_name(prefix: str) -> str:
    return f"{prefix}-{uuid4().hex[:10]}"


def _assert_minted_token_contract(token: MintedAccessToken, name: str) -> None:
    assert token.name == name, f"Expected token name {name!r}, got {token.name!r}"
    assert (
        len(token.token) == 32
    ), f"Expected a 32-character token, got length {len(token.token)}"
    assert (
        token.short_id == token.token[:8]
    ), f"Expected short_id={token.token[:8]!r}, got {token.short_id!r}"
    expected_display_id = f"{token.token[:8]}…{token.token[-6:]}"
    assert (
        token.display_id == expected_display_id
    ), f"Expected display_id={expected_display_id!r}, got {token.display_id!r}"
    assert (
        token.created_at.isdecimal()
    ), f"Expected decimal created_at, got {token.created_at!r}"
    assert (
        int(token.created_at) > 0
    ), f"Expected positive created_at, got {token.created_at!r}"
    assert (
        token.last_used_at.isdecimal()
    ), f"Expected decimal last_used_at, got {token.last_used_at!r}"


def _cleanup_unexpected_tokens(
    settings_api: SettingsAPI,
    short_ids: set[str],
) -> None:
    if short_ids:
        with allure.step("Clean up tokens created by the failed request"):
            for short_id in short_ids:
                response = settings_api.revoke_access_token_raw(short_id)
                assert response.status_code in (200, 404), (
                    f"Failed to clean up unexpected token {short_id!r}: "
                    f"HTTP {response.status_code}, body={response.text!r}"
                )


@allure.feature("5. Web Frontend")
@allure.story("API access tokens")
@pytest.mark.api
@pytest.mark.frontend
class TestAccessTokensAPI:
    @allure.title("Minted token is available through the list endpoint")
    def test_mint_and_list_contract(
        self,
        settings_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
    ):
        name = _token_name("contract")

        with allure.step("Mint an access token"):
            minted = access_token_factory(name)
            _assert_minted_token_contract(minted, name)

        with allure.step("List access tokens"):
            listed = settings_api.list_access_tokens().tokens
            stored_matches = [
                entry for entry in listed if entry.short_id == minted.short_id
            ]
            assert len(stored_matches) == 1, (
                f"Expected one stored entry for {minted.short_id!r}, "
                f"got {stored_matches!r}"
            )
            stored = stored_matches[0]
            assert (
                stored.name == name
            ), f"Expected stored name {name!r}, got {stored.name!r}"
            assert stored.display_id == minted.display_id, (
                f"Expected display_id {minted.display_id!r}, "
                f"got {stored.display_id!r}"
            )

    @allure.title("Tokens are unique and one token can be revoked independently")
    def test_mint_multiple_and_revoke_one(
        self,
        settings_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
    ):
        with allure.step("Mint two unique tokens"):
            first = access_token_factory(_token_name("first"))
            second = access_token_factory(_token_name("second"))
            assert (
                first.token != second.token
            ), "Expected two minted full tokens to be unique"
            assert (
                first.short_id != second.short_id
            ), f"Expected unique short IDs, both were {first.short_id!r}"

        with allure.step("Revoke the first token"):
            response = settings_api.revoke_access_token(first.short_id)
            assert (
                response.result == "OK"
            ), f"Expected revoke result 'OK', got {response.result!r}"

        with allure.step("Verify only the second token remains"):
            remaining_ids = {
                entry.short_id for entry in settings_api.list_access_tokens().tokens
            }
            assert first.short_id not in remaining_ids, (
                f"Revoked token {first.short_id!r} is still listed: "
                f"{remaining_ids!r}"
            )
            assert second.short_id in remaining_ids, (
                f"Unrelated token {second.short_id!r} disappeared: "
                f"{remaining_ids!r}"
            )

        with allure.step("Verify repeated revoke returns 404"):
            repeated = settings_api.revoke_access_token_raw(first.short_id)
            assert repeated.status_code == 404, (
                f"Expected HTTP 404 for repeated revoke, got "
                f"{repeated.status_code}: {repeated.text!r}"
            )

    @allure.title("Duplicate token names are preserved")
    def test_duplicate_names(
        self,
        settings_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
    ):
        name = "Home Assistant"
        with allure.step("Mint two tokens with the same name"):
            first = access_token_factory(name)
            second = access_token_factory(name)

        with allure.step("Verify both names are returned unchanged"):
            entries = settings_api.list_access_tokens().tokens
            matching = [
                entry
                for entry in entries
                if entry.short_id in {first.short_id, second.short_id}
            ]
            assert len(matching) == 2, f"Expected two matching tokens, got {matching!r}"
            assert all(
                entry.name == name for entry in matching
            ), f"Expected name {name!r}, got {[entry.name for entry in matching]!r}"

    @allure.title("Revoke rejects malformed and unknown short IDs")
    @pytest.mark.parametrize(
        "short_id",
        [
            "1234567",
            "123456789",
            "unknown0",
        ],
    )
    def test_revoke_rejects_unknown_short_id(
        self,
        settings_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
        short_id: str,
    ):
        with allure.step("Create a token and capture the current token IDs"):
            token = access_token_factory(_token_name("revoke-boundary"))
            before_ids = {
                entry.short_id for entry in settings_api.list_access_tokens().tokens
            }

        with allure.step(f"Attempt to revoke short ID {short_id!r}"):
            response = settings_api.revoke_access_token_raw(short_id)
            assert response.status_code == 404, (
                f"Expected HTTP 404 for {short_id!r}, got "
                f"{response.status_code}: {response.text!r}"
            )

        with allure.step("Verify the existing token remains available"):
            after_ids = {
                entry.short_id for entry in settings_api.list_access_tokens().tokens
            }
            assert after_ids == before_ids, (
                f"Invalid revoke changed tokens: before={before_ids!r}, "
                f"after={after_ids!r}"
            )
            assert (
                token.short_id in after_ids
            ), f"Existing token {token.short_id!r} disappeared: {after_ids!r}"

    @allure.title("Mint rejects missing and malformed JSON bodies")
    @pytest.mark.parametrize(
        ("request_kwargs", "case"),
        [
            ({"data": b""}, "empty body"),
            (
                {
                    "data": "{",
                    "headers": {"Content-Type": "application/json"},
                },
                "malformed JSON",
            ),
            ({"json": {"owner": "wrong-field"}}, "missing name"),
            ({"json": {"name": None}}, "null name"),
            ({"json": {"name": 123}}, "numeric name"),
            ({"json": {"name": ["array"]}}, "array name"),
            ({"json": {"name": {"nested": "object"}}}, "object name"),
        ],
    )
    def test_mint_rejects_invalid_body(
        self,
        settings_api: SettingsAPI,
        request_kwargs: dict,
        case: str,
    ):
        with allure.step("Capture token IDs before the invalid request"):
            before_ids = {
                entry.short_id for entry in settings_api.list_access_tokens().tokens
            }

        with allure.step(f"Submit mint request with {case}"):
            response = settings_api.mint_access_token_raw(**request_kwargs)

        with allure.step("Capture token IDs after the invalid request"):
            after_ids = {
                entry.short_id for entry in settings_api.list_access_tokens().tokens
            }
        unexpected_ids = after_ids - before_ids
        _cleanup_unexpected_tokens(settings_api, unexpected_ids)

        with allure.step("Verify the request is rejected without side effects"):
            assert response.status_code == 400, (
                f"Expected HTTP 400 for {case}, got {response.status_code}: "
                f"{response.text!r}"
            )
            assert after_ids == before_ids, (
                f"Invalid {case} changed tokens: before={before_ids!r}, "
                f"after={after_ids!r}"
            )

    @allure.title("Invalid nested token path returns 404 without side effects")
    def test_invalid_nested_path_does_not_mint(
        self,
        settings_api: SettingsAPI,
    ):
        with allure.step("Capture token IDs before the invalid path request"):
            before_ids = {
                entry.short_id for entry in settings_api.list_access_tokens().tokens
            }

        with allure.step("POST to an unsupported nested token path"):
            response = settings_api.post_raw(
                "/api/access/tokens/not-a-resource",
                json={"name": _token_name("invalid-path")},
            )

        with allure.step("Capture token IDs after the invalid path request"):
            after_ids = {
                entry.short_id for entry in settings_api.list_access_tokens().tokens
            }
        unexpected_ids = after_ids - before_ids
        _cleanup_unexpected_tokens(settings_api, unexpected_ids)

        with allure.step("Verify 404 response without token creation"):
            assert (
                response.status_code == 404
            ), f"Expected HTTP 404, got {response.status_code}: {response.text!r}"
            assert not unexpected_ids, (
                "POST /api/access/tokens/not-a-resource returned 404 but created "
                f"tokens: {unexpected_ids!r}"
            )

    @allure.title("Delete-all revokes all tokens when the device starts clean")
    def test_revoke_all_on_clean_device(
        self,
        settings_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
    ):
        with allure.step("Require a clean token list"):
            existing = settings_api.list_access_tokens().tokens
            if existing:
                pytest.skip(
                    "Delete-all test will not remove pre-existing tokens: "
                    f"short_ids={[token.short_id for token in existing]!r}"
                )

        with allure.step("Mint two tokens"):
            access_token_factory(_token_name("delete-all-first"))
            access_token_factory(_token_name("delete-all-second"))

        with allure.step("Revoke all tokens"):
            response = settings_api.revoke_all_access_tokens()
            assert (
                response.result == "OK"
            ), f"Expected delete-all result 'OK', got {response.result!r}"

        with allure.step("Verify the token list is empty"):
            remaining = settings_api.list_access_tokens().tokens
            assert remaining == [], f"Expected no tokens, got {remaining!r}"


@allure.feature("5. Web Frontend")
@allure.story("API access-token persistence")
@pytest.mark.api
@pytest.mark.frontend
@pytest.mark.long_running
class TestAccessTokenPersistence:
    @allure.title("Minted tokens survive an immediate reboot")
    @allure.issue("https://flipper.atlassian.net/browse/FW-1099")
    @pytest.mark.skip(reason="FW-1099: https://flipper.atlassian.net/browse/FW-1099")
    def test_minted_tokens_survive_immediate_reboot(
        self,
        settings_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
        persistent_cli_connection,
        web_base_url: str,
    ):
        with allure.step("Mint three tokens"):
            minted = [
                access_token_factory(_token_name(f"persist-{index}"))
                for index in range(3)
            ]
            expected_ids = {token.short_id for token in minted}

        with allure.step("Immediately reboot after minting"):
            recovered = persistent_cli_connection.reboot_and_wait_for_api(
                web_base_url,
                timeout=60,
            )
            assert recovered, "Device did not recover after mint persistence reboot"

        with allure.step("Verify token metadata survived the reboot"):
            listed_ids = {
                entry.short_id for entry in settings_api.list_access_tokens().tokens
            }
            assert expected_ids <= listed_ids, (
                f"Minted tokens were not persisted: expected={expected_ids!r}, "
                f"listed={listed_ids!r}"
            )

        with allure.step("Verify every persisted token still authenticates"):
            for token in minted:
                authorized = settings_api.get_raw(
                    "/api/status",
                    headers={_TOKEN_HEADER: token.token},
                )
                assert authorized.status_code == 200, (
                    f"Persisted token {token.short_id!r} failed authentication: "
                    f"HTTP {authorized.status_code}, body={authorized.text!r}"
                )

    @allure.title("Revoked token remains revoked after an immediate reboot")
    @allure.issue("https://flipper.atlassian.net/browse/FW-1099")
    @pytest.mark.skip(reason="FW-1099: https://flipper.atlassian.net/browse/FW-1099")
    def test_revoked_token_stays_revoked_after_reboot(
        self,
        settings_api: SettingsAPI,
        access_token_factory: Callable[[str], MintedAccessToken],
        persistent_cli_connection,
        web_base_url: str,
    ):
        with allure.step("Mint a token"):
            token = access_token_factory(_token_name("revoke-persist"))

        with allure.step("Allow the current firmware flush timer to persist it"):
            # Establish a durable baseline without depending on FW-1099.
            time.sleep(_CURRENT_FLUSH_PERIOD_S + 1)

        with allure.step("Reboot and verify the token persistence baseline"):
            recovered = persistent_cli_connection.reboot_and_wait_for_api(
                web_base_url,
                timeout=60,
            )
            assert recovered, "Device did not recover after baseline reboot"
            baseline_ids = {
                entry.short_id for entry in settings_api.list_access_tokens().tokens
            }
            assert token.short_id in baseline_ids, (
                f"Baseline token {token.short_id!r} was not persisted: "
                f"{baseline_ids!r}"
            )

        with allure.step("Revoke the token"):
            response = settings_api.revoke_access_token(token.short_id)
            assert (
                response.result == "OK"
            ), f"Expected revoke result 'OK', got {response.result!r}"

        with allure.step("Immediately reboot after revoking"):
            recovered = persistent_cli_connection.reboot_and_wait_for_api(
                web_base_url,
                timeout=60,
            )
            assert recovered, "Device did not recover after revoke persistence reboot"

        with allure.step("Verify the revoked token did not reappear"):
            listed_ids = {
                entry.short_id for entry in settings_api.list_access_tokens().tokens
            }
            assert token.short_id not in listed_ids, (
                f"Revoked token {token.short_id!r} reappeared after reboot: "
                f"{listed_ids!r}"
            )

        with allure.step("Verify the revoked credential is rejected"):
            revoked_auth = settings_api.get_raw(
                "/api/status",
                headers={_TOKEN_HEADER: token.token},
            )
            assert revoked_auth.status_code == 403, (
                f"Revoked token authenticated after reboot: HTTP "
                f"{revoked_auth.status_code}, body={revoked_auth.text!r}"
            )
