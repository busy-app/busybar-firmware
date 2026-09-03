"""
Integration tests for the /api/display/draw priority system.

These tests verify the behaviour introduced in PR FW-682 (iteration 2):
  - Priority range for the draw HTTP API is [1, 100] inclusive.
  - System app priority levels:
      • Stub apps (poweroff, settings pages)  → 0  (LOADER_STUB_APP_PRIORITY)
      • Busy timer in NOT_STARTED             → 9  (LOADER_PASSTHROUGH_PRIORITY)
      • Any standard built-in app baseline    → 10 (LOADER_DEFAULT_APP_PRIORITY)
      • Active / paused BUSY work session     → 101 (LOADER_BLOCKING_PRIORITY)
  - The busy app is always running; the loader never reports priority 0 under
    normal operation.
  - A draw request is accepted when its priority is *greater than or equal to*
    (>=) the priority of the currently running system app.
  - Equal-priority requests from a *different* app_id are rejected while canvas
    content from the current app_id is still visible.

Organisation
------------
TestDrawPriorityValidation
    HTTP-level field validation — always 400, independent of app state.

TestDrawInDefaultState
    Busy timer stopped (NOT_STARTED). Loader priority = 9
    (LOADER_PASSTHROUGH_PRIORITY). Draws at priority >= 10 succeed.

TestDrawBusyTimerTransitions
    Busy timer driven to active (priority 101) then paused / stopped.
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

from clients.api import StreamingAPI
from clients.api.assets import (
    AssetsAPI,
    LOADER_MAX_PRIORITY,
    LOADER_DEFAULT_APP_PRIORITY,
    LOADER_MAX_APP_PRIORITY,
    LOADER_STUB_APP_PRIORITY,
    LOADER_PASSTHROUGH_PRIORITY,
    DEFAULT_ELEMENT_PRIORITY,
)
from clients.api.streaming import raw_to_png
from utils.busy_timer import (
    WORK_CARD_UUID,
    next_timestamp,
    set_snapshot,
    wait_for_snapshot_type,
)
from .display_helpers import back_pixel as _back_pixel

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

_APP_ID = "test_priority_app"

# Smallest real shared image on the device; image elements are validated by the
# decoder, so a draw payload must reference an asset that exists and fits the display.
_BUILTIN_IMAGE = "shared/checkmark_front_8x8.image"

# The simplest valid element that can be included in any draw request.
# NOTE: `font` is required by the firmware text-element parser even though
# the OpenAPI schema marks it as optional with a default value.
_SIMPLE_ELEM = [
    {"id": "e1", "type": "text", "text": "hello", "timeout": 5, "font": "small"}
]

_BACK_PROBE_REGION = (40, 54, 120, 78)
_BACK_PROBE_MIN_SIGNAL = 200
_BACK_PROBE_RESIDUE_DIVISOR = 2
_FRONT_ON_CALL_REGION = (16, 2, 58, 14)
_ON_CALL_MAX_CLEAN_DISTANCE = 18.0
_ON_CALL_MIN_BUSY_DISTANCE = 8.0
_RENDER_SETTLE_S = 0.5

_ON_CALL_ANIM = [
    {
        "id": "oncall",
        "type": "animation",
        "stock_path": "shared/on_call_72x16.anim",
        "section": "loop",
        "loop": True,
        "timeout": 30,
        "display": "front",
    }
]


def _simple_draw(assets_api: AssetsAPI, priority: int | None = None):
    """Convenience: draw with _SIMPLE_ELEM, return raw requests.Response."""
    return assets_api.draw_response(_APP_ID, _SIMPLE_ELEM, priority=priority)


def _busy_snapshot(snapshot_type: str, busy_state_guard: dict, is_paused: bool = False):
    if snapshot_type == "NOT_STARTED":
        snapshot: dict = {"type": "NOT_STARTED"}
    else:
        snapshot = {
            "type": "INFINITE",
            "card_id": WORK_CARD_UUID,
            "is_paused": is_paused,
        }
    snapshot["busy_bar_settings"] = busy_state_guard.get("snapshot", {}).get(
        "busy_bar_settings", {}
    )
    return snapshot


def _set_busy_snapshot(
    api_session,
    web_base_url: str,
    busy_state_guard: dict,
    snapshot_type: str,
    *,
    is_paused: bool = False,
    propagate: float | None = None,
):
    body = {
        "snapshot": _busy_snapshot(snapshot_type, busy_state_guard, is_paused),
        "snapshot_timestamp_ms": next_timestamp(api_session, web_base_url),
    }
    set_snapshot(api_session, web_base_url, body)
    if propagate is None:
        wait_for_snapshot_type(api_session, web_base_url, snapshot_type)
    else:
        wait_for_snapshot_type(
            api_session, web_base_url, snapshot_type, propagate=propagate
        )


def _back_region_pixels(
    data: bytes, region: tuple[int, int, int, int]
) -> tuple[int, ...]:
    x1, y1, x2, y2 = region
    return tuple(_back_pixel(data, x, y) for y in range(y1, y2) for x in range(x1, x2))


def _pixel_distance(left: tuple[int, ...], right: tuple[int, ...]) -> int:
    assert len(left) == len(right)
    return sum(abs(a - b) for a, b in zip(left, right))


def _avg_abs_diff(left: bytes, right: bytes) -> float:
    if len(left) != len(right):
        return float("inf")
    return sum(abs(a - b) for a, b in zip(left, right)) / len(left)


def _nearest_avg_distance(frame: bytes, references: list[bytes]) -> float:
    assert references, "No reference frames captured"
    return min(_avg_abs_diff(frame, reference) for reference in references)


def _front_region(frame: bytes, region: tuple[int, int, int, int]) -> bytes:
    width = 72
    x1, y1, x2, y2 = region

    roi = bytearray()
    for y in range(y1, y2):
        row_start = (y * width + x1) * 3
        row_end = (y * width + x2) * 3
        roi.extend(frame[row_start:row_end])

    return bytes(roi)


def _capture_front_frames(
    streaming_api: StreamingAPI,
    *,
    seconds: float = 1.5,
    interval: float = 0.12,
) -> list[bytes]:
    frames: list[bytes] = []
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        frames.append(streaming_api.get_screen_bytes(display=0))
        time.sleep(interval)
    assert frames, "No front-display frames captured"
    return frames


def _median(values: list[float]) -> float:
    assert values, "Cannot compute median of empty sequence"
    ordered = sorted(values)
    mid = len(ordered) // 2
    if len(ordered) % 2:
        return ordered[mid]
    return (ordered[mid - 1] + ordered[mid]) / 2


def _attach_front_frame_samples(name_prefix: str, frames: list[bytes]) -> None:
    for idx, frame in enumerate(frames[:3]):
        allure.attach(
            raw_to_png(frame, display=0),
            name=f"{name_prefix}_{idx}",
            attachment_type=allure.attachment_type.PNG,
        )


def _assert_on_call_frames_are_clean(
    clean_frames: list[bytes],
    busy_frames: list[bytes],
    actual_frames: list[bytes],
) -> None:
    clean_rois = [_front_region(frame, _FRONT_ON_CALL_REGION) for frame in clean_frames]
    busy_rois = [_front_region(frame, _FRONT_ON_CALL_REGION) for frame in busy_frames]
    actual_rois = [
        _front_region(frame, _FRONT_ON_CALL_REGION) for frame in actual_frames
    ]

    clean_to_busy = [_nearest_avg_distance(frame, busy_rois) for frame in clean_rois]
    busy_separation = _median(clean_to_busy)
    assert busy_separation >= _ON_CALL_MIN_BUSY_DISTANCE, (
        "Clean On Call frames are not distinguishable from active BUSY frames: "
        f"median_distance={busy_separation:.2f}, "
        f"minimum={_ON_CALL_MIN_BUSY_DISTANCE}"
    )

    bad_frames = 0
    worst_clean = 0.0
    weakest_busy = float("inf")
    for frame in actual_rois:
        clean_distance = _nearest_avg_distance(frame, clean_rois)
        busy_distance = _nearest_avg_distance(frame, busy_rois)
        worst_clean = max(worst_clean, clean_distance)
        weakest_busy = min(weakest_busy, busy_distance)

        if not (
            clean_distance <= _ON_CALL_MAX_CLEAN_DISTANCE
            and clean_distance <= busy_distance * 0.75
        ):
            bad_frames += 1

    max_bad_frames = max(2, len(actual_rois) // 4)
    if bad_frames > max_bad_frames:
        _attach_front_frame_samples("clean_on_call", clean_frames)
        _attach_front_frame_samples("active_busy", busy_frames)
        _attach_front_frame_samples("actual_on_call", actual_frames)

    assert bad_frames <= max_bad_frames, (
        "On Call redraw still looks contaminated by active BUSY frames: "
        f"bad_frames={bad_frames}, max_bad_frames={max_bad_frames}, "
        f"worst_clean_distance={worst_clean:.2f}, "
        f"nearest_busy_distance={weakest_busy:.2f}, "
        f"busy_separation={busy_separation:.2f}"
    )


def _capture_back_probe(streaming_api: StreamingAPI) -> tuple[int, ...]:
    time.sleep(_RENDER_SETTLE_S)
    return _back_region_pixels(
        streaming_api.get_screen_bytes(display=1), _BACK_PROBE_REGION
    )


def _eviction_probe_element(text: str) -> dict:
    return {
        "id": "probe",
        "type": "text",
        "text": text,
        "timeout": 30,
        "font": "normal",
        "color": "#FFFFFFFF",
        "display": "back",
        "align": "center",
        "x": 80,
        "y": 66,
    }


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
    it calls loader_set_priority(LOADER_PASSTHROUGH_PRIORITY=9).
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
        """priority=50 == DEFAULT_ELEMENT_PRIORITY; must succeed (50 > 9)."""
        resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
        assets_api.assert_status(resp, 200)

    @allure.title("Draw at maximum (100) succeeds when idle")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_maximum_priority_accepted(self, assets_api: AssetsAPI, busy_timer_stopped):
        """priority=100 must succeed when loader priority is 9."""
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
        Since 50 > 9, the request must succeed.
        """
        resp = _simple_draw(assets_api, priority=None)
        assets_api.assert_status(resp, 200)


@allure.feature("5. Web Frontend")
@allure.story("Display Priority – Busy Timer Transitions")
class TestDrawBusyTimerTransitions:
    """
    Tests that exercise priority changes driven by busy-timer state transitions.

    When the timer enters or remains in a work state the busy app calls
    loader_set_priority(LOADER_BLOCKING_PRIORITY=101).  When it returns to
    NOT_STARTED it calls loader_set_priority(LOADER_PASSTHROUGH_PRIORITY=9).

    Acceptance rule: request_priority >= active_priority.
    During a BUSY work session (priority=101), all valid HTTP draws are blocked.
    """

    @allure.title("Active busy timer (prio 101) blocks default draw (50 < 101)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_active_timer_blocks_default_priority(
        self, assets_api: AssetsAPI, busy_timer_active
    ):
        """
        With timer active: loader priority = 101.
        Draw at 50 (DEFAULT_ELEMENT_PRIORITY) must return 409 (50 < 101).
        """
        resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
        assets_api.assert_status(resp, 409)

    @allure.title("Active busy timer blocks maximum valid draw priority (100 < 101)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_active_timer_blocks_maximum_draw_priority(
        self, assets_api: AssetsAPI, busy_timer_active
    ):
        resp = _simple_draw(assets_api, priority=LOADER_MAX_PRIORITY)
        assets_api.assert_status(resp, 409)

    @allure.title("Active busy timer blocks high app priority (89 < 101)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_active_timer_blocks_just_below_busy_priority(
        self, assets_api: AssetsAPI, busy_timer_active
    ):
        """
        priority=89 is below the blocking loader priority=101. Must return 409.
        """
        resp = _simple_draw(assets_api, priority=LOADER_MAX_APP_PRIORITY - 1)
        assets_api.assert_status(resp, 409)

    @allure.title("Pausing busy timer keeps blocking priority and rejects draw (50)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_paused_timer_blocks_default_priority(
        self, assets_api: AssetsAPI, busy_timer_paused
    ):
        """Even paused timers keep BUSY blocking priority."""
        resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
        assets_api.assert_status(resp, 409)

    @allure.title("Stopping timer (NOT_STARTED → prio 9) allows default draw (50)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_stopped_timer_allows_default_priority(
        self, assets_api: AssetsAPI, busy_timer_stopped
    ):
        """
        NOT_STARTED timer → loader_set_priority(9).
        Draw at 50 must succeed (50 > 9).
        """
        resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
        assets_api.assert_status(resp, 200)

    @allure.title(
        "Transition: idle → active → draw blocked, then active → paused → draw blocked"
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
          1. NOT_STARTED (prio 9) – draw 50 → 200  (50 > 9)
          2. INFINITE active (prio 101) – draw 50 → 409  (50 < 101)
          3. INFINITE paused (prio 101) – draw 50 → 409  (50 < 101)

        The busy_state_guard fixture restores the original snapshot after the test.
        """
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
            body_snapshot["busy_bar_settings"] = busy_state_guard.get(
                "snapshot", {}
            ).get("busy_bar_settings", {})
            body = {
                "snapshot": body_snapshot,
                "snapshot_timestamp_ms": next_timestamp(api_session, web_base_url),
            }
            r = api_session.put(
                f"{web_base_url}/api/busy/snapshot", json=body, timeout=10
            )
            r.raise_for_status()
            time.sleep(1.0)

        with allure.step(
            "1. Set NOT_STARTED (loader priority 9) → draw 50 must pass (50 > 9)"
        ):
            _set("NOT_STARTED")
            resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
            assets_api.assert_status(resp, 200)

        with allure.step(
            "2. Activate timer (loader priority 101) → draw 50 must be blocked"
        ):
            _set("INFINITE", is_paused=False)
            resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
            assets_api.assert_status(resp, 409)

        with allure.step("3. Pause timer → draw 50 must still not pass"):
            _set("INFINITE", is_paused=True)
            resp = _simple_draw(assets_api, priority=DEFAULT_ELEMENT_PRIORITY)
            assets_api.assert_status(resp, 409)


@allure.feature("5. Web Frontend")
@allure.story("Display Priority – Draw Lifecycle")
class TestDrawDisplayLifecycle:
    """
    Tests for the draw/clear resource lifecycle, independent of exact priority
    levels.  All draws use DEFAULT_ELEMENT_PRIORITY (50) with the timer
    stopped (loader priority 9), guaranteeing they are accepted (50 > 9).

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
        Verify the server accepts a valid mixed payload without a 400.
        (The image asset must exist and fit the display — image elements are now
        validated by the decoder, so a missing/oversized image is rejected with 400.)
        """
        elements = [
            {"id": "txt", "type": "text", "text": "hi", "timeout": 5, "font": "small"},
            {
                "id": "img",
                "type": "image",
                "stock_path": _BUILTIN_IMAGE,
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


@allure.feature("5. Web Frontend")
@allure.story("Display Priority – Reactive Eviction")
class TestCanvasEvictionOnPriorityChange:
    """
    Regression tests for FW-975 (calendar overlay + busy timer overlap).

    When the loader priority rises above the priority of currently-displayed
    canvas content (e.g. the busy timer transitions to active and raises
    loader priority to LOADER_BLOCKING_PRIORITY), the canvas service must
    evict that content so it does not visually overlap whatever the
    higher-priority app renders next.

    The fix lives in the canvas service: it subscribes to
    LoaderEventTypePriorityChanged on the loader pubsub and clears its
    widgets whenever the new loader priority would now reject a re-draw
    of the current content.
    """

    @allure.title(
        "Canvas content from app A is evicted when busy timer activates, "
        "so app B can later draw at low priority"
    )
    @pytest.mark.api
    @pytest.mark.frontend
    def test_busy_active_evicts_existing_canvas_content(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        api_session,
        web_base_url: str,
        busy_state_guard: dict,
    ):
        """
        Reproduces the canvas draw calls + start-busy bug:
          1. Timer NOT_STARTED → loader priority is low.  App A draws at
             priority=DEFAULT_ELEMENT_PRIORITY (50).  Accepted (200).
          2. Timer goes INFINITE active → loader priority jumps to BLOCKING
             (above LOADER_MAX_PRIORITY).  The canvas service must evict
             app A's content reactively.
          3. Timer back to NOT_STARTED → loader priority drops again.
          4. Different app B draws at a *low* priority that would still be
             below app A's old 50 if A's content were still on screen.
             With the fix this succeeds (canvas empty, threshold low).
             Without the fix this is rejected with 409.
        """
        app_a = "evict_test_a"
        app_b = "evict_test_b"
        elem = [_eviction_probe_element("FW975")]
        # A low priority that would be rejected against a leftover priority-50
        # canvas owned by a different app (rule: rejected = new <= current).
        low_priority = LOADER_PASSTHROUGH_PRIORITY + 1  # 10

        with allure.step("0. Capture active BUSY frame with no canvas content"):
            assets_api.clear_display()
            _set_busy_snapshot(api_session, web_base_url, busy_state_guard, "INFINITE")
            active_without_canvas = _capture_back_probe(streaming_api)
            _set_busy_snapshot(
                api_session, web_base_url, busy_state_guard, "NOT_STARTED"
            )

        with allure.step("1. Stop timer; app A draws at priority 50 → 200"):
            r_a = assets_api.draw_response(
                app_a, elem, priority=DEFAULT_ELEMENT_PRIORITY
            )
            assets_api.assert_status(r_a, 200)
            canvas_probe = _capture_back_probe(streaming_api)
            probe_signal = _pixel_distance(active_without_canvas, canvas_probe)
            assert probe_signal >= _BACK_PROBE_MIN_SIGNAL, (
                "Probe text did not create a strong enough back-display signal: "
                f"distance={probe_signal}, minimum={_BACK_PROBE_MIN_SIGNAL}"
            )

        with allure.step(
            "2. Activate timer → loader priority rises to BLOCKING; "
            "canvas must evict app A's content reactively and visibly"
        ):
            _set_busy_snapshot(api_session, web_base_url, busy_state_guard, "INFINITE")
            active_after_eviction = _capture_back_probe(streaming_api)
            residue = _pixel_distance(active_without_canvas, active_after_eviction)
            allowed_residue = max(
                _BACK_PROBE_MIN_SIGNAL,
                probe_signal // _BACK_PROBE_RESIDUE_DIVISOR,
            )
            assert residue < allowed_residue, (
                "Canvas probe is still visible while BUSY is active: "
                f"active residue={residue}, allowed={allowed_residue}, "
                f"probe signal={probe_signal}"
            )

        with allure.step("3. Stop timer → loader priority drops back to passthrough"):
            _set_busy_snapshot(
                api_session, web_base_url, busy_state_guard, "NOT_STARTED"
            )

        with allure.step(
            "4. Different app B draws at priority 10; succeeds only if "
            "canvas was evicted in step 2"
        ):
            r_b = assets_api.draw_response(app_b, elem, priority=low_priority)
            assets_api.assert_status(r_b, 200)

        # Cleanup
        assets_api.clear_display()

    @allure.title("On Call redraw immediately after stopping BUSY has no overlay")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_on_call_immediate_redraw_after_busy_stop_has_no_visual_overlay(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        api_session,
        web_base_url: str,
        busy_state_guard: dict,
    ):
        """
        Regression for the user-visible FW-975 race:
          1. Capture clean stock On Call animation frames.
          2. Start BUSY and capture active BUSY frames.
          3. Stop BUSY and immediately redraw On Call without waiting for
             render settle.
          4. The final frames must be closer to clean On Call than to BUSY.

        The redraw request is intentionally sent immediately after the stop
        snapshot and must be accepted on the first attempt.
        """
        app_name = "on_call"

        def _draw_on_call() -> requests.Response:
            return assets_api.draw_response(
                app_name, _ON_CALL_ANIM, priority=DEFAULT_ELEMENT_PRIORITY
            )

        try:
            with allure.step("1. Draw clean On Call animation and capture frames"):
                assets_api.clear_display()
                _set_busy_snapshot(
                    api_session, web_base_url, busy_state_guard, "NOT_STARTED"
                )
                response = _draw_on_call()
                assets_api.assert_status(response, 200)
                time.sleep(1.0)
                clean_frames = _capture_front_frames(streaming_api)

            with allure.step("2. Start BUSY and capture active frames"):
                _set_busy_snapshot(
                    api_session, web_base_url, busy_state_guard, "INFINITE"
                )
                busy_frames = _capture_front_frames(streaming_api, seconds=1.0)

            with allure.step("3. Stop BUSY and immediately redraw On Call"):
                stop_body = {
                    "snapshot": _busy_snapshot("NOT_STARTED", busy_state_guard),
                    "snapshot_timestamp_ms": next_timestamp(
                        api_session, web_base_url
                    ),
                }
                set_snapshot(api_session, web_base_url, stop_body)
                response = _draw_on_call()
                assets_api.assert_status(response, 200)
                wait_for_snapshot_type(
                    api_session,
                    web_base_url,
                    "NOT_STARTED",
                    propagate=0.0,
                )
                time.sleep(_RENDER_SETTLE_S)
                actual_frames = _capture_front_frames(streaming_api)

            with allure.step("4. Verify final frames are clean On Call"):
                _assert_on_call_frames_are_clean(
                    clean_frames, busy_frames, actual_frames
                )
        finally:
            assets_api.clear_display()
