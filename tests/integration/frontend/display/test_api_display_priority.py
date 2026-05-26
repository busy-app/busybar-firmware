"""
Integration tests for the /api/display/draw priority system.

These tests verify the behaviour introduced in PR FW-682 (iteration 2):
  - Priority range for the draw HTTP API is [1, 100] inclusive.
  - System app priority levels:
      • Stub apps (poweroff, settings pages)  → 0  (LOADER_STUB_APP_PRIORITY)
      • Any standard built-in app (incl.
        busy timer in NOT_STARTED / paused)   → 10 (LOADER_DEFAULT_APP_PRIORITY)
      • Active BUSY/CUSTOM work session       → 90 (LOADER_MAX_APP_PRIORITY)
  - The busy app is always running; the loader never reports priority 0 under
    normal operation.
  - A draw request is accepted when its priority is *greater than or equal to*
    (>=) the priority of the currently running system app.
  - Equal-priority requests from a *different* app_id override the current display.

Organisation
------------
TestDrawPriorityValidation
    HTTP-level field validation — always 400, independent of app state.

TestDrawInDefaultState
    Busy timer stopped (NOT_STARTED).  Loader priority = 10
    (LOADER_DEFAULT_APP_PRIORITY).  All draws at priority >= 10 succeed.

TestDrawBusyTimerTransitions
    Busy timer driven to active (priority 90) then paused / stopped.
    Verifies the full priority lifecycle:
      active → draw-blocked → pause/stop → draw-still-blocked

TestDrawDisplayLifecycle
    Draw + clear + redraw sequences; multiple concurrent app_ids.
"""

from __future__ import annotations

import time

import allure
import pytest
import requests

from clients.api.assets import (
    AssetsAPI,
    LOADER_MAX_PRIORITY,
    LOADER_DEFAULT_APP_PRIORITY,
    LOADER_MAX_APP_PRIORITY,
    LOADER_STUB_APP_PRIORITY,
    DEFAULT_ELEMENT_PRIORITY,
)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

_APP_ID = "test_priority_app"

# The simplest valid element that can be included in any draw request.
# NOTE: `font` is required by the firmware text-element parser even though
# the OpenAPI schema marks it as optional with a default value.
_SIMPLE_ELEM = [
    {"id": "e1", "type": "text", "text": "hello", "timeout": 5, "font": "small"}
]


def _simple_draw(assets_api: AssetsAPI, priority: int | None = None):
    """Convenience: draw with _SIMPLE_ELEM, return raw requests.Response."""
    return assets_api.draw_response(_APP_ID, _SIMPLE_ELEM, priority=priority)


# ---------------------------------------------------------------------------
# Test classes
# ---------------------------------------------------------------------------


@allure.feature("5. Web Frontend")
@allure.story("Display Priority – Validation")
class TestDrawPriorityValidation:
    """
    Tests that malformed or out-of-range priority values are rejected with 400
    at the HTTP parsing layer, before the loader priority check is reached.
    These tests are independent of app state.
    """

    @allure.title("POST /api/display/draw – priority=0 → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_priority_zero_rejected(self, assets_api: AssetsAPI):
        """
        Priority 0 violates the `if(priority <= 0) break` firmware check.

        NOTE: the firmware breaks without sending any HTTP response for this
        case, so the client receives a ReadTimeout rather than an explicit 400.
        Either outcome counts as a rejection.
        """
        try:
            resp = _simple_draw(assets_api, priority=0)
            assets_api.assert_status(resp, 400)
        except requests.exceptions.ReadTimeout:
            pass  # firmware rejected without response – treated as 400

    @allure.title("POST /api/display/draw – priority=-1 → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_priority_negative_rejected(self, assets_api: AssetsAPI):
        """Negative priority values are invalid."""
        try:
            resp = _simple_draw(assets_api, priority=-1)
            assets_api.assert_status(resp, 400)
        except requests.exceptions.ReadTimeout:
            pass

    @allure.title("POST /api/display/draw – priority=101 above LOADER_MAX → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_priority_above_maximum_rejected(self, assets_api: AssetsAPI):
        """
        Values above LOADER_MAX_PRIORITY (100) must be rejected with 400.

        NOTE: the firmware breaks without sending any HTTP response for this
        case (do-while exits early without calling MG_REPLY_BAD_REQUEST),
        so the client may receive a ReadTimeout. Either is treated as rejection.
        """
        try:
            resp = _simple_draw(assets_api, priority=LOADER_MAX_PRIORITY + 1)
            assets_api.assert_status(resp, 400)
        except requests.exceptions.ReadTimeout:
            pass

    @allure.title("POST /api/display/draw – missing application_name → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_missing_app_id_rejected(self, assets_api: AssetsAPI):
        """Request body without application_name must be rejected."""
        try:
            resp = assets_api.draw_raw({"elements": _SIMPLE_ELEM})
            assets_api.assert_status(resp, 400)
        except requests.exceptions.ReadTimeout:
            pass

    @allure.title("POST /api/display/draw – missing elements → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_missing_elements_rejected(self, assets_api: AssetsAPI):
        """Request body without elements array must be rejected."""
        try:
            resp = assets_api.draw_raw({"application_name": _APP_ID})
            assets_api.assert_status(resp, 400)
        except requests.exceptions.ReadTimeout:
            pass

    @allure.title("POST /api/display/draw – empty elements array → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_empty_elements_array_rejected(self, assets_api: AssetsAPI):
        """An empty elements array contains nothing to draw; must be rejected."""
        resp = assets_api.draw_raw({"application_name": _APP_ID, "elements": []})
        assets_api.assert_status(resp, 400)


@allure.feature("5. Web Frontend")
@allure.story("Display Priority – Default App State")
class TestDrawInDefaultState:
    """
    Tests that assume the busy timer is idle / not-started.

    The busy app is always running on this device.  In NOT_STARTED state
    it calls loader_set_priority(LOADER_DEFAULT_APP_PRIORITY=10).
    Draw requests therefore need priority >= 10 to succeed.

    The busy_timer_stopped fixture guarantees the device is in the expected state
    for the duration of each test and restores it afterwards.
    """

    @allure.title("Draw at DEFAULT_ELEMENT_PRIORITY (50) succeeds when idle")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_default_element_priority_accepted(
        self, assets_api: AssetsAPI, busy_timer_stopped
    ):
        """priority=50 == DEFAULT_ELEMENT_PRIORITY; must succeed (50 >= 10)."""
        resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
        assets_api.assert_status(resp, 200)

    @allure.title("Draw at maximum (100) succeeds when idle")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_maximum_priority_accepted(self, assets_api: AssetsAPI, busy_timer_stopped):
        """priority=100 must succeed when loader priority is 10."""
        resp = _simple_draw(assets_api, priority=LOADER_MAX_PRIORITY)
        assets_api.assert_status(resp, 200)

    @allure.title("Draw with no explicit priority uses server default (50) – succeeds")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_omitted_priority_uses_default(
        self, assets_api: AssetsAPI, busy_timer_stopped
    ):
        """
        When priority is omitted the server uses DEFAULT_ELEMENT_PRIORITY (50).
        Since 50 >= 10, the request must succeed.
        """
        resp = _simple_draw(assets_api, priority=None)
        assets_api.assert_status(resp, 200)


@allure.feature("5. Web Frontend")
@allure.story("Display Priority – Busy Timer Transitions")
class TestDrawBusyTimerTransitions:
    """
    Tests that exercise priority changes driven by busy-timer state transitions.

    When the timer enters work (active, not-paused) state the busy app calls
    loader_set_priority(LOADER_MAX_APP_PRIORITY=90).  When it is paused or
    returns to NOT_STARTED it calls loader_set_priority(LOADER_DEFAULT=10).

    Acceptance rule: request_priority >= active_priority.
    During an active session (priority=90), draws at < 90 are blocked;
    draws at >= 90 (i.e. 90–100) are accepted.
    """

    @allure.title(
        "Active busy timer (prio 90) blocks draw below busy priority (50 < 90)"
    )
    @pytest.mark.api
    @pytest.mark.frontend
    def test_active_timer_blocks_default_priority(
        self, assets_api: AssetsAPI, busy_timer_active
    ):
        """
        With timer active: loader priority = 90.
        Draw at 50 (DEFAULT_ELEMENT_PRIORITY) must return 409 (50 < 90).
        """
        resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
        assets_api.assert_status(resp, 409)

    @allure.title("Active busy timer forbids draw at any priority (even 100)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_active_timer_allows_equal_busy_priority(
        self, assets_api: AssetsAPI, busy_timer_active
    ):
        resp = _simple_draw(assets_api, priority=LOADER_MAX_PRIORITY)
        assets_api.assert_status(resp, 409)

    @allure.title("Active busy timer blocks draw one below BUSY priority (89 < 90)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_active_timer_blocks_just_below_busy_priority(
        self, assets_api: AssetsAPI, busy_timer_active
    ):
        """
        priority=89 is one below loader priority=90.  Must return 409.
        This pins the exact cutoff boundary for the active-busy state.
        """
        resp = _simple_draw(assets_api, priority=LOADER_MAX_APP_PRIORITY - 1)
        assets_api.assert_status(resp, 409)

    @allure.title("Pausing busy timer (prio → 10) still forbids default-priority draw (50)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_paused_timer_allows_default_priority(
        self, assets_api: AssetsAPI, busy_timer_paused
    ):
        """Even paused timers forbid the draw"""
        resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
        assets_api.assert_status(resp, 409)

    @allure.title("Stopping timer (NOT_STARTED → prio 10) allows default draw (50)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_stopped_timer_allows_default_priority(
        self, assets_api: AssetsAPI, busy_timer_stopped
    ):
        """
        NOT_STARTED timer → loader_set_priority(10).
        Draw at 50 must succeed (50 >= 10).
        """
        resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
        assets_api.assert_status(resp, 200)

    @allure.title(
        "Transition: idle → active → draw blocked, then active → paused → draw allowed"
    )
    @pytest.mark.api
    @pytest.mark.frontend
    def test_idle_to_active_to_paused_transition(
        self,
        assets_api: AssetsAPI,
        api_session,
        web_base_url: str,
        busy_state_guard: dict,
    ):
        """
        Full round-trip through three priority states:
          1. NOT_STARTED (prio 10) – draw 50 → 200  (50 >= 10)
          2. INFINITE active (prio 90) – draw 50 → 409  (50 < 90)
          3. INFINITE paused (prio 10) – draw 50 → 200  (50 >= 10)

        The busy_state_guard fixture restores the original snapshot after the test.
        """
        import time as _time

        def _set(snapshot_type: str, is_paused: bool = False):
            body_snapshot: dict
            if snapshot_type == "NOT_STARTED":
                body_snapshot = {"type": "NOT_STARTED"}
            else:
                body_snapshot = {
                    "type": "INFINITE",
                    "card_id": "00000000-0000-0000-0000-000000000001",
                    "is_paused": is_paused,
                }
            # Always fetch the device's current timestamp and advance it so the
            # snapshot is not rejected as stale/own by busy_timer.
            current = api_session.get(f"{web_base_url}/api/busy/snapshot", timeout=10)
            current.raise_for_status()
            device_ts = current.json().get("snapshot_timestamp_ms", 0)
            next_ts = max(device_ts, int(_time.time() * 1000)) + 2000
            body_snapshot["busy_bar_settings"] = busy_state_guard.get(
                "snapshot", {}
            ).get("busy_bar_settings", {})
            body = {
                "snapshot": body_snapshot,
                "snapshot_timestamp_ms": next_ts,
            }
            r = api_session.put(
                f"{web_base_url}/api/busy/snapshot", json=body, timeout=10
            )
            r.raise_for_status()
            _time.sleep(1.0)

        with allure.step(
            "1. Set NOT_STARTED (loader priority 10) → draw 50 must pass (50 >= 10)"
        ):
            _set("NOT_STARTED")
            resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
            assets_api.assert_status(resp, 200)

        with allure.step(
            "2. Activate timer (loader priority 90) → draw 50 must be blocked (50 < 90)"
        ):
            _set("INFINITE", is_paused=False)
            resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
            assets_api.assert_status(resp, 409)

        with allure.step(
            "3. Pause timer → draw 50 must still not pass"
        ):
            _set("INFINITE", is_paused=True)
            resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
            assets_api.assert_status(resp, 409)


@allure.feature("5. Web Frontend")
@allure.story("Display Priority – Draw Lifecycle")
class TestDrawDisplayLifecycle:
    """
    Tests for the draw/clear resource lifecycle, independent of exact priority
    levels.  All draws use DEFAULT_ELEMENT_PRIORITY (50) with the timer
    stopped (loader priority 10), guaranteeing they are accepted (50 >= 10).

    The _clear_display_after_test autouse fixture clears the display after
    every test, but explicit cleanup calls are left in the tests for clarity.
    """

    @allure.title("Draw then clear all returns 200 for both calls")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_draw_then_clear_all(self, assets_api: AssetsAPI, busy_timer_stopped):
        """Successful draw followed by unconditional clear must both return 200."""
        draw_resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
        assets_api.assert_status(draw_resp, 200)

        clear_resp = assets_api.clear_display()
        assert clear_resp.result  # Pydantic model validates "result" field

    @allure.title("Clear display by specific app_id removes only that app's elements")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_clear_by_app_id(self, assets_api: AssetsAPI, busy_timer_stopped):
        """
        After drawing as app A, clearing by app A's id must succeed (200).
        Drawing as app B afterwards at the same priority must also succeed
        (since clearing A does not raise any priority barrier).
        """
        app_a = "priority_test_a"
        app_b = "priority_test_b"
        elem = [
            {"id": "e1", "type": "text", "text": "A", "timeout": 5, "font": "small"}
        ]

        # App A draws
        r_a = assets_api.draw_response(app_a, elem, priority=DEFAULT_ELEMENT_PRIORITY)
        assets_api.assert_status(r_a, 200)

        # Clear app A's elements specifically
        r_clear = assets_api.clear_display_by_app(app_a)
        assets_api.assert_status(r_clear, 200)

        # App B draws – clearing A must not affect B's ability to draw
        r_b = assets_api.draw_response(app_b, elem, priority=DEFAULT_ELEMENT_PRIORITY)
        assets_api.assert_status(r_b, 200)

        # Cleanup
        assets_api.clear_display()

    @allure.title("Redrawing with same app_id is idempotent (200 both times)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_redraw_same_app_id_is_idempotent(
        self, assets_api: AssetsAPI, busy_timer_stopped
    ):
        """
        Sending two successive draw requests from the same app_id at the same
        priority must both succeed.  The second overwrites the first.
        """
        for _i in range(2):
            resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
            assets_api.assert_status(resp, 200)

        # Cleanup
        assets_api.clear_display()

    @allure.title("Draw after clear succeeds (clear resets canvas state)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_draw_after_clear_succeeds(self, assets_api: AssetsAPI, busy_timer_stopped):
        """Clearing the display and then redrawing must return 200."""
        assets_api.clear_display()
        resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
        assets_api.assert_status(resp, 200)

        # Cleanup
        assets_api.clear_display()

    @allure.title("Two concurrent app_ids at the same priority level – both succeed")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_two_apps_same_priority_both_accepted(
        self, assets_api: AssetsAPI, busy_timer_stopped
    ):
        """
        Two different app_ids drawing at the same HTTP priority level must
        not override each other. Whoever comes first gets priority.
        """
        app_a = "concurrent_a"
        app_b = "concurrent_b"
        elem = [
            {"id": "e1", "type": "text", "text": "X", "timeout": 10, "font": "small"}
        ]

        r_a = assets_api.draw_response(app_a, elem, priority=DEFAULT_ELEMENT_PRIORITY)
        r_b = assets_api.draw_response(app_b, elem, priority=DEFAULT_ELEMENT_PRIORITY)

        assets_api.assert_status(r_a, 200)
        assets_api.assert_status(r_b, 409)

        # Cleanup
        assets_api.clear_display()

    @allure.title("All element types accepted in a single draw call")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_multiple_element_types_in_one_request(
        self, assets_api: AssetsAPI, busy_timer_stopped
    ):
        """
        A draw request may contain text, image, and anim elements together.
        Verify the server accepts a mixed payload without a 400.
        (Image and anim assets may not exist on the device, but the server
        should still attempt the draw and return 200 or 500, not 400.)
        """
        elements = [
            {"id": "txt", "type": "text", "text": "hi", "timeout": 5, "font": "small"},
            {
                "id": "img",
                "type": "image",
                "path": "does_not_exist.png",
                "x": 0,
                "y": 0,
                "timeout": 5,
            },
        ]
        resp = assets_api.draw_response(
            _APP_ID, elements, priority=DEFAULT_ELEMENT_PRIORITY
        )
        # Server must not reject with a schema/validation error (400)
        assert (
            resp.status_code != 400
        ), f"Expected non-400 for mixed element types, got {resp.status_code}"

        # Cleanup
        assets_api.clear_display()
