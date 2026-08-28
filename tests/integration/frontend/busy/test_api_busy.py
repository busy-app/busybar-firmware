import json
import time

import allure
import pytest

from clients.api import BusyAPI, StorageAPI, StreamingAPI
from utils.busy_timer import (
    STATE_SETTLE_S,
    TS_MAX_FUTURE_MS,
    WORK_CARD_UUID,
    device_now_ms,
    next_timestamp,
    wait_for_snapshot_type,
)
from utils.wait import wait_for

BUSY_THEME_ANIMS = {
    "back_soon": "back_soon_72x16.anim",
    "booked": "booked_72x16.anim",
    "chill_time": "chill_time_72x16.anim",
    "dnd": "dnd_72x16.anim",
    "flow": "flow_72x16.anim",
    "keep_out": "keep_out_72x16.anim",
    "lunch": "lunch_72x16.anim",
    "meeting": "meeting_72x16.anim",
    "on_air": "on_air_72x16.anim",
    "on_call": "on_call_72x16.anim",
}

DEFAULT_BUSY_SETTINGS = {
    "theme": "busy",
    "show_work_phase_only": False,
    "trigger_smart_home": False,
    "show_work_time": True,
}


def _current_busy_settings(snapshot_doc: dict) -> dict:
    return (
        snapshot_doc.get("snapshot", {}).get("busy_bar_settings")
        or snapshot_doc.get("busy_bar_settings")
        or DEFAULT_BUSY_SETTINGS
    )


def _simple_snapshot(timestamp_ms: int, settings: dict, time_left_ms: int = 180000) -> dict:
    return {
        "snapshot": {
            "type": "SIMPLE",
            "card_id": WORK_CARD_UUID,
            "is_paused": True,
            "time_left_ms": time_left_ms,
            "busy_bar_settings": settings,
        },
        "snapshot_timestamp_ms": timestamp_ms,
    }


def _infinite_snapshot(timestamp_ms: int, settings: dict) -> dict:
    return {
        "snapshot": {
            "type": "INFINITE",
            "card_id": WORK_CARD_UUID,
            "is_paused": True,
            "busy_bar_settings": settings,
        },
        "snapshot_timestamp_ms": timestamp_ms,
    }


def _interval_snapshot(timestamp_ms: int, settings: dict) -> dict:
    return {
        "snapshot": {
            "type": "INTERVAL",
            "card_id": WORK_CARD_UUID,
            "is_paused": True,
            "current_interval": 0,
            "current_interval_time_total_ms": 300000,
            "current_interval_time_left_ms": 240000,
            "interval_settings": {
                "interval_work_ms": 300000,
                "interval_rest_ms": 300000,
                "interval_work_cycles_count": 2,
                "is_autostart_enabled": False,
            },
            "busy_bar_settings": settings,
        },
        "snapshot_timestamp_ms": timestamp_ms,
    }


def _profile_payload(
    profile,
    timestamp_ms: int,
    *,
    title: str | None = None,
    busy_bar_settings: dict | None = None,
) -> dict:
    return {
        "sort_order": profile.sort_order,
        "title": title or profile.title,
        "id": profile.id,
        "timer_settings": profile.timer_settings,
        "busy_bar_settings": (
            busy_bar_settings
            if busy_bar_settings is not None
            else profile.busy_bar_settings.model_dump()
        ),
        "profile_timestamp_ms": timestamp_ms,
    }


def _wait_for_profile_title(busy_api: BusyAPI, slot: str, title: str):
    return wait_for(
        f'BUSY profile "{slot}" title {title!r}',
        lambda: busy_api.get_profile(slot),
        lambda profile: profile.title == title,
        timeout=5.0,
        interval=0.1,
    )


def _front_frame_has_content(frame: bytes) -> bool:
    return any(frame)


def _wait_for_front_frame_content(
    streaming_api: StreamingAPI,
    *,
    display: int = 0,
    attempts: int = 15,
    poll_interval_s: float = 0.2,
) -> bytes:
    """Retry screen capture until the front display contains non-zero pixels."""
    last_frame = b""
    for _ in range(attempts):
        last_frame = streaming_api.get_screen_bytes(display=display)
        if _front_frame_has_content(last_frame):
            return last_frame
        time.sleep(poll_interval_s)
    return last_frame


@allure.feature("5. Web Frontend")
@allure.story("Busy Timer")
class TestBusySnapshotAPI:
    """Test cases for Busy Timer Snapshot API endpoints"""

    @allure.title("GET /api/busy/snapshot")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_busy_snapshot_get(self, busy_api: BusyAPI):
        """Test GET /api/busy/snapshot returns valid snapshot"""
        response = busy_api.get_snapshot()

        assert response.snapshot is not None
        assert "type" in response.snapshot
        assert response.snapshot["type"] in [
            "NOT_STARTED", "INFINITE", "SIMPLE", "INTERVAL"
        ]
        assert response.snapshot_timestamp_ms >= 0

    @allure.title("GET /api/busy/snapshot (verify NOT_STARTED structure)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_busy_snapshot_not_started(self, busy_api: BusyAPI):
        """Test that NOT_STARTED snapshot has expected structure"""
        response = busy_api.get_snapshot()

        if response.snapshot.get("type") != "NOT_STARTED":
            pytest.skip("Timer is currently running, cannot test NOT_STARTED state")

        assert response.snapshot["type"] == "NOT_STARTED"

    @allure.title("PUT /api/busy/snapshot")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_busy_snapshot_put(self, busy_api: BusyAPI):
        """Test PUT /api/busy/snapshot accepts valid snapshot data"""
        # Save original snapshot
        original = busy_api.get_snapshot()

        test_snapshot = {
            "snapshot": {
                "type": "INFINITE",
                "card_id": "00000000-0000-0000-0000-000000000000",
                "is_paused": True,
                "busy_bar_settings": {
                    "theme": "busy",
                    "show_work_phase_only": False,
                    "trigger_smart_home": False,
                    "show_work_time": True,
                },
            },
            "snapshot_timestamp_ms": 0,
        }

        try:
            with allure.step("Set test snapshot"):
                response = busy_api.set_snapshot_raw(test_snapshot)
                assert response.status_code == 200

            with allure.step("Verify snapshot is readable after PUT"):
                updated = busy_api.get_snapshot()
                assert updated.snapshot is not None
                assert "type" in updated.snapshot
        finally:
            with allure.step("Restore original snapshot"):
                busy_api.set_snapshot_raw({
                    "snapshot": original.snapshot,
                    "snapshot_timestamp_ms": original.snapshot_timestamp_ms,
                })

    @allure.title("PUT /api/busy/snapshot (invalid data)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_busy_snapshot_put_invalid(self, busy_api: BusyAPI):
        """Test PUT /api/busy/snapshot rejects invalid data"""
        response = busy_api.set_snapshot_raw({"invalid": "data"})
        assert response.status_code == 400


@allure.feature("5. Web Frontend")
@allure.story("Busy Timer")
class TestBusyProfileAPI:
    """Test cases for Busy Timer Profile API endpoints"""

    @allure.title("GET /api/busy/profiles/busy")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_busy_profile_get_busy(self, busy_api: BusyAPI):
        """Test GET /api/busy/profiles/busy returns valid profile"""
        response = busy_api.get_profile("busy")

        assert response.title
        assert response.id
        assert response.timer_settings is not None
        assert "type" in response.timer_settings
        assert response.timer_settings["type"] in ["INFINITE", "SIMPLE", "INTERVAL"]
        assert response.busy_bar_settings is not None
        assert isinstance(response.busy_bar_settings.show_work_time, bool), (
            "Expected show_work_time to be bool, got "
            f"{response.busy_bar_settings.show_work_time!r}"
        )

    @allure.title("GET /api/busy/profiles/custom")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_busy_profile_get_custom(self, busy_api: BusyAPI):
        """Test GET /api/busy/profiles/custom returns valid profile"""
        response = busy_api.get_profile("custom")

        assert response.title
        assert response.id
        assert response.timer_settings is not None
        assert isinstance(response.busy_bar_settings.show_work_time, bool), (
            "Expected show_work_time to be bool, got "
            f"{response.busy_bar_settings.show_work_time!r}"
        )

    @allure.title("PUT /api/busy/profiles/custom")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_busy_profile_put_custom(self, busy_api: BusyAPI):
        """Test PUT /api/busy/profiles/custom accepts valid profile data"""
        # Save original
        original = busy_api.get_profile("custom")
        test_timestamp_ms = original.profile_timestamp_ms + 1
        test_title = f"show_work_time_off_{test_timestamp_ms}"

        test_profile = {
            "sort_order": 0,
            "title": test_title,
            "id": original.id,
            "timer_settings": {
                "type": "SIMPLE",
                "total_time_ms": 600000,
            },
            "busy_bar_settings": {
                "theme": "busy",
                "show_work_phase_only": False,
                "trigger_smart_home": False,
                "show_work_time": False,
            },
            "profile_timestamp_ms": test_timestamp_ms,
        }

        try:
            with allure.step("Set test profile"):
                response = busy_api.set_profile_raw("custom", test_profile)
                assert (
                    response.status_code == 200
                ), f"Expected 200, got {response.status_code}: {response.text[:200]}"

            with allure.step("Verify the disabled value round-trips"):
                updated = _wait_for_profile_title(busy_api, "custom", test_title)
                assert updated.busy_bar_settings.show_work_time is False, (
                    "Expected show_work_time=False, got "
                    f"{updated.busy_bar_settings.show_work_time!r}"
                )
                assert (
                    updated.timer_settings["type"] == "SIMPLE"
                ), f"Expected SIMPLE, got {updated.timer_settings!r}"
        finally:
            with allure.step("Restore original profile"):
                current = busy_api.get_profile("custom")
                restore_data = _profile_payload(
                    original,
                    current.profile_timestamp_ms + 1,
                )
                restore_response = busy_api.set_profile_raw("custom", restore_data)
                assert restore_response.status_code == 200, (
                    "Failed to restore the custom profile: "
                    f"{restore_response.status_code}: {restore_response.text[:200]}"
                )
                _wait_for_profile_title(busy_api, "custom", original.title)

    @allure.title("PUT /api/busy/profiles/custom accepts legacy busy bar settings")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_busy_profile_put_custom_without_show_work_time(
        self, busy_api: BusyAPI
    ):
        """Test that an omitted show_work_time value defaults to true."""
        original = busy_api.get_profile("custom")
        legacy_settings = original.busy_bar_settings.model_dump()
        legacy_settings.pop("show_work_time")
        test_timestamp_ms = original.profile_timestamp_ms + 1
        test_title = f"legacy_show_work_time_{test_timestamp_ms}"

        test_profile = _profile_payload(
            original,
            test_timestamp_ms,
            title=test_title,
            busy_bar_settings=legacy_settings,
        )

        try:
            with allure.step("Set a profile without show_work_time"):
                response = busy_api.set_profile_raw("custom", test_profile)
                assert (
                    response.status_code == 200
                ), f"Expected 200, got {response.status_code}: {response.text[:200]}"

            with allure.step("Verify the firmware emits the default value"):
                _wait_for_profile_title(busy_api, "custom", test_title)
                raw_response = busy_api.get_profile_raw("custom")
                assert (
                    raw_response.status_code == 200
                ), f"Expected 200, got {raw_response.status_code}: {raw_response.text[:200]}"
                raw_settings = raw_response.json()["busy_bar_settings"]
                assert (
                    raw_settings.get("show_work_time") is True
                ), f"Expected show_work_time=True, got {raw_settings!r}"
        finally:
            with allure.step("Restore original profile"):
                current = busy_api.get_profile("custom")
                restore_data = _profile_payload(
                    original,
                    current.profile_timestamp_ms + 1,
                )
                restore_response = busy_api.set_profile_raw("custom", restore_data)
                assert restore_response.status_code == 200, (
                    "Failed to restore the custom profile: "
                    f"{restore_response.status_code}: {restore_response.text[:200]}"
                )
                _wait_for_profile_title(busy_api, "custom", original.title)

    @allure.title("GET /api/busy/profiles/{invalid_slot}")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_busy_profile_invalid_slot(self, busy_api: BusyAPI):
        """Test GET /api/busy/profiles with invalid slot returns error"""
        response = busy_api.get_profile_raw("invalid_slot")
        assert response.status_code == 400


@allure.feature("5. Web Frontend")
@allure.story("Busy Timer")
@pytest.mark.api
@pytest.mark.frontend
@pytest.mark.regression
class TestBusyThemeRegressions:
    @allure.title("BUSY theme configs point to deployed shared animations")
    @pytest.mark.parametrize("theme,anim_name", sorted(BUSY_THEME_ANIMS.items()))
    def test_busy_theme_config_references_existing_shared_animation(
        self, storage_api: StorageAPI, theme: str, anim_name: str
    ):
        config_path = f"/ext/apps_assets/busy/themes/{theme}/theme.json"
        config_response = storage_api.read(config_path)
        assert config_response.status_code == 200

        config = json.loads(config_response.content.decode("utf-8"))
        expected_bg_path = f"/ext/apps_assets/shared/animations/{anim_name}"
        assert set(config).issubset({"bg_path", "order"})
        assert config["bg_path"] == expected_bg_path
        assert config["bg_path"]
        if "order" in config:
            assert isinstance(config["order"], int)

        anim_response = storage_api.read(expected_bg_path)
        assert anim_response.status_code == 200
        assert len(anim_response.content) > 0

    @allure.title("Missing BUSY theme config is not readable")
    def test_missing_busy_theme_config_is_not_readable(self, storage_api: StorageAPI):
        response = storage_api.read("/ext/apps_assets/busy/themes/not_a_theme/theme.json")

        assert response.status_code in {400, 404}

    @allure.title("BUSY theme render smoke uses shared theme assets")
    def test_busy_theme_render_smoke(
        self,
        busy_api: BusyAPI,
        streaming_api: StreamingAPI,
        api_session,
        web_base_url,
        busy_state_guard,
    ):
        settings = dict(_current_busy_settings(busy_state_guard))
        settings["theme"] = "on_air"
        body = _simple_snapshot(
            next_timestamp(api_session, web_base_url),
            settings,
            time_left_ms=180000,
        )
        body["snapshot"]["is_paused"] = False

        assert busy_api.set_snapshot_raw(body).status_code == 200
        wait_for_snapshot_type(api_session, web_base_url, "SIMPLE")

        frame = _wait_for_front_frame_content(streaming_api, display=0)
        assert _front_frame_has_content(frame)


@allure.feature("5. Web Frontend")
@allure.story("Busy Timer")
@pytest.mark.api
@pytest.mark.frontend
@pytest.mark.regression
class TestBusySnapshotRegressions:
    @allure.title("BUSY {expected_type} snapshot round-trips")
    @pytest.mark.parametrize(
        "factory,expected_type",
        [(_simple_snapshot, "SIMPLE"), (_interval_snapshot, "INTERVAL")],
    )
    def test_busy_timer_snapshot_round_trip(
        self,
        busy_api: BusyAPI,
        api_session,
        web_base_url,
        busy_state_guard,
        factory,
        expected_type,
    ):
        settings = _current_busy_settings(busy_state_guard)
        body = factory(next_timestamp(api_session, web_base_url), settings)

        with allure.step(f"Submit a {expected_type} snapshot"):
            response = busy_api.set_snapshot_raw(body)
            assert response.status_code == 200
            time.sleep(STATE_SETTLE_S)

        with allure.step(f"Verify the {expected_type} snapshot round-trips"):
            updated = busy_api.get_snapshot()
            assert updated.snapshot["type"] == expected_type
            assert isinstance(updated.snapshot["is_paused"], bool)
            assert updated.snapshot["busy_bar_settings"] == settings
            if expected_type == "SIMPLE":
                assert (
                    0
                    < updated.snapshot["time_left_ms"]
                    <= body["snapshot"]["time_left_ms"]
                )

    @allure.title("BUSY timer ignores stale snapshots")
    def test_busy_timer_stale_snapshot_is_ignored(
        self, busy_api: BusyAPI, api_session, web_base_url, busy_state_guard
    ):
        settings = _current_busy_settings(busy_state_guard)
        accepted_ts = next_timestamp(api_session, web_base_url)
        accepted = _simple_snapshot(accepted_ts, settings, time_left_ms=240000)
        stale = {
            "snapshot": {
                "type": "NOT_STARTED",
                "busy_bar_settings": settings,
            },
            "snapshot_timestamp_ms": accepted_ts - 1,
        }

        with allure.step("Establish an accepted SIMPLE snapshot baseline"):
            assert busy_api.set_snapshot_raw(accepted).status_code == 200
            time.sleep(STATE_SETTLE_S)
            current = busy_api.get_snapshot()

        with allure.step("Submit an older NOT_STARTED snapshot"):
            stale["snapshot_timestamp_ms"] = current.snapshot_timestamp_ms - 1
            assert busy_api.set_snapshot_raw(stale).status_code == 200
            time.sleep(STATE_SETTLE_S)

        with allure.step("Verify the stale snapshot is ignored and state is preserved"):
            updated = busy_api.get_snapshot()
            assert updated.snapshot_timestamp_ms == current.snapshot_timestamp_ms
            assert updated.snapshot["type"] == "SIMPLE"
            assert 0 < updated.snapshot["time_left_ms"] <= 240000

    @allure.title("BUSY timer rejects snapshots more than 60 seconds in the future")
    def test_busy_timer_future_snapshot_is_rejected(
        self, busy_api: BusyAPI, api_session, web_base_url, busy_state_guard
    ):
        settings = _current_busy_settings(busy_state_guard)
        baseline = _simple_snapshot(
            next_timestamp(api_session, web_base_url), settings, time_left_ms=240000
        )
        with allure.step("Establish an accepted SIMPLE snapshot baseline"):
            assert busy_api.set_snapshot_raw(baseline).status_code == 200
            time.sleep(STATE_SETTLE_S)
            before = busy_api.get_snapshot()

        # anchor to the device RTC: the firmware checks the window against its own
        # clock, so a host-clock timestamp lands inside the window whenever the RTC
        # runs ahead of the runner — and the device would accept what we expect it
        # to reject
        with allure.step("Submit a snapshot beyond the future timestamp limit"):
            future_ts = (
                device_now_ms(api_session, web_base_url) + TS_MAX_FUTURE_MS + 15000
            )
            future = _infinite_snapshot(future_ts, settings)
            response = busy_api.set_snapshot_raw(future)

        with allure.step(
            "Verify the future snapshot is rejected and state is preserved"
        ):
            assert response.status_code == 400, (
                "Expected BUSY snapshot parser to reject timestamps more than "
                f"{TS_MAX_FUTURE_MS}ms in the future, got {response.status_code}: "
                f"{response.text[:200]}"
            )
            after = busy_api.get_snapshot()
            assert after.snapshot_timestamp_ms == before.snapshot_timestamp_ms
            assert after.snapshot == before.snapshot

    @allure.title(
        "BUSY timer invalid semantic snapshot {param_id} does not change current state"
    )
    @pytest.mark.parametrize(
        "mutator",
        [
            lambda body: body["snapshot"].update({"card_id": "not-a-uuid"}),
            lambda body: body["snapshot"].update({"time_left_ms": 999999999}),
            lambda body: body["snapshot"].update({"type": "UNKNOWN"}),
        ],
    )
    def test_busy_timer_invalid_simple_snapshots_do_not_apply(
        self, busy_api: BusyAPI, api_session, web_base_url, busy_state_guard, mutator
    ):
        settings = _current_busy_settings(busy_state_guard)
        baseline = _infinite_snapshot(
            next_timestamp(api_session, web_base_url), settings
        )
        with allure.step("Establish an accepted INFINITE snapshot baseline"):
            assert busy_api.set_snapshot_raw(baseline).status_code == 200
            time.sleep(STATE_SETTLE_S)
            before = busy_api.get_snapshot()

        with allure.step("Submit the invalid SIMPLE snapshot"):
            invalid = _simple_snapshot(
                next_timestamp(api_session, web_base_url), settings
            )
            mutator(invalid)
            response = busy_api.set_snapshot_raw(invalid)
            assert response.status_code in {200, 400}
            time.sleep(STATE_SETTLE_S)

        with allure.step("Verify the invalid snapshot does not change current state"):
            after = busy_api.get_snapshot()
            assert after.snapshot_timestamp_ms == before.snapshot_timestamp_ms
            assert after.snapshot == before.snapshot

    @allure.title("BUSY timer invalid INTERVAL state does not change current state")
    def test_busy_timer_invalid_interval_snapshot_does_not_apply(
        self, busy_api: BusyAPI, api_session, web_base_url, busy_state_guard
    ):
        settings = _current_busy_settings(busy_state_guard)
        baseline = _infinite_snapshot(
            next_timestamp(api_session, web_base_url), settings
        )
        with allure.step("Establish an accepted INFINITE snapshot baseline"):
            assert busy_api.set_snapshot_raw(baseline).status_code == 200
            time.sleep(STATE_SETTLE_S)
            before = busy_api.get_snapshot()

        with allure.step("Submit an INTERVAL snapshot with time left beyond its total"):
            invalid = _interval_snapshot(
                next_timestamp(api_session, web_base_url), settings
            )
            invalid["snapshot"]["current_interval_time_left_ms"] = (
                invalid["snapshot"]["current_interval_time_total_ms"] + 1
            )
            response = busy_api.set_snapshot_raw(invalid)
            assert response.status_code in {200, 400}
            time.sleep(STATE_SETTLE_S)

        with allure.step("Verify the invalid snapshot does not change current state"):
            after = busy_api.get_snapshot()
            assert after.snapshot_timestamp_ms == before.snapshot_timestamp_ms
            assert after.snapshot == before.snapshot

    @allure.title("BUSY timer invalid snapshot field {param_id} does not change state")
    @pytest.mark.parametrize(
        "factory,mutator",
        [
            (
                _simple_snapshot,
                lambda body: body["snapshot"].pop("card_id"),
            ),
            pytest.param(
                _simple_snapshot,
                lambda body: body["snapshot"].update({"time_left_ms": -1}),
                marks=pytest.mark.xfail(
                    reason="Known firmware validation gap: negative SIMPLE time is accepted",
                    strict=False,
                ),
            ),
            (
                _interval_snapshot,
                lambda body: body["snapshot"]["interval_settings"].update(
                    {"interval_work_cycles_count": 0}
                ),
            ),
            pytest.param(
                _interval_snapshot,
                lambda body: body["snapshot"].update({"current_interval": 9}),
                marks=pytest.mark.xfail(
                    reason="Known firmware validation gap: out-of-range INTERVAL index is accepted",
                    strict=False,
                ),
            ),
        ],
    )
    def test_busy_timer_invalid_field_matrix_does_not_apply(
        self,
        busy_api: BusyAPI,
        api_session,
        web_base_url,
        busy_state_guard,
        factory,
        mutator,
    ):
        settings = _current_busy_settings(busy_state_guard)
        baseline = _infinite_snapshot(
            next_timestamp(api_session, web_base_url), settings
        )
        with allure.step("Establish an accepted INFINITE snapshot baseline"):
            assert busy_api.set_snapshot_raw(baseline).status_code == 200
            time.sleep(STATE_SETTLE_S)
            before = busy_api.get_snapshot()

        with allure.step("Submit a snapshot with an invalid field value"):
            invalid = factory(next_timestamp(api_session, web_base_url), settings)
            mutator(invalid)
            response = busy_api.set_snapshot_raw(invalid)
            assert response.status_code in {200, 400}
            time.sleep(STATE_SETTLE_S)

        with allure.step("Verify the invalid snapshot does not change current state"):
            after = busy_api.get_snapshot()
            assert after.snapshot_timestamp_ms == before.snapshot_timestamp_ms
            assert after.snapshot == before.snapshot

    @allure.title("BUSY timer newer snapshot timestamp wins")
    def test_busy_timer_newer_snapshot_timestamp_wins(
        self, busy_api: BusyAPI, api_session, web_base_url, busy_state_guard
    ):
        settings = _current_busy_settings(busy_state_guard)
        older = _simple_snapshot(
            next_timestamp(api_session, web_base_url), settings, time_left_ms=240000
        )

        with allure.step("Submit the older SIMPLE snapshot"):
            assert busy_api.set_snapshot_raw(older).status_code == 200
            time.sleep(STATE_SETTLE_S)

        with allure.step("Submit a newer SIMPLE snapshot"):
            newer = _simple_snapshot(
                next_timestamp(api_session, web_base_url), settings, time_left_ms=120000
            )
            assert busy_api.set_snapshot_raw(newer).status_code == 200
            time.sleep(STATE_SETTLE_S)

        with allure.step("Verify the newer snapshot wins"):
            updated = busy_api.get_snapshot()
            assert updated.snapshot_timestamp_ms >= newer["snapshot_timestamp_ms"]
            assert updated.snapshot["type"] == "SIMPLE"
            assert 0 < updated.snapshot["time_left_ms"] <= 120000

    @allure.title("BUSY timer snapshot preserves non-default busy bar settings")
    def test_busy_timer_snapshot_preserves_busy_bar_settings(
        self, busy_api: BusyAPI, api_session, web_base_url, busy_state_guard
    ):
        settings = dict(_current_busy_settings(busy_state_guard))
        settings.update(
            {
                "theme": "on_call",
                "show_work_phase_only": True,
                "trigger_smart_home": False,
                "show_work_time": False,
            }
        )
        body = _simple_snapshot(next_timestamp(api_session, web_base_url), settings)

        with allure.step("Submit a snapshot with non-default busy bar settings"):
            assert busy_api.set_snapshot_raw(body).status_code == 200
            time.sleep(STATE_SETTLE_S)

        with allure.step("Verify the busy bar settings round-trip unchanged"):
            updated = busy_api.get_snapshot()
            assert updated.snapshot["busy_bar_settings"] == settings

    @allure.title("Paused BUSY SIMPLE snapshot is restored after reboot")
    def test_busy_timer_paused_simple_snapshot_persists_after_reboot(
        self,
        busy_api: BusyAPI,
        api_session,
        web_base_url,
        busy_state_guard,
        persistent_cli_connection,
    ):
        settings = _current_busy_settings(busy_state_guard)
        expected_time_left_ms = 123000
        body = _simple_snapshot(
            next_timestamp(api_session, web_base_url),
            settings,
            time_left_ms=expected_time_left_ms,
        )

        with allure.step("Submit a paused SIMPLE snapshot"):
            assert busy_api.set_snapshot_raw(body).status_code == 200
            time.sleep(STATE_SETTLE_S)

        with allure.step("Reboot device via CLI and wait for API"):
            assert persistent_cli_connection.reboot_and_wait_for_api(
                web_base_url, timeout=90
            ), "Device did not come back after CLI reboot"

        with allure.step("Verify snapshot persisted after reboot"):
            updated = busy_api.get_snapshot()
            assert updated.snapshot_timestamp_ms == body["snapshot_timestamp_ms"]
            assert updated.snapshot["type"] == "SIMPLE"
            assert updated.snapshot["is_paused"] is True
            assert updated.snapshot["time_left_ms"] == expected_time_left_ms
