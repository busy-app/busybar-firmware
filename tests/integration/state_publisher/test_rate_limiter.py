import os
import time

import allure
import pytest


BSB_STATE_PUBLISHER_MAX_FRAMES_PER_WINDOW = 11
BSB_STATE_PUBLISHER_WINDOW_S = 1

@allure.feature("State Publisher")
@allure.story("Rate limiter")
@pytest.mark.api
@pytest.mark.state_publisher
@pytest.mark.rate_limiter
@pytest.mark.regression
class TestStatePublisherRateLimiter:
    @allure.title("Burst is bounded and stream recovers after idle window")
    def test_burst_is_limited_but_final_state_is_delivered(
        self, state_publisher, input_api
    ):

        max_frames = os.getenv("BSB_STATE_PUBLISHER_MAX_FRAMES_PER_WINDOW", BSB_STATE_PUBLISHER_MAX_FRAMES_PER_WINDOW)
        window_s = os.getenv("BSB_STATE_PUBLISHER_WINDOW_S", BSB_STATE_PUBLISHER_WINDOW_S)
        if not (max_frames and window_s):
            pytest.skip(
                "Configure BSB_STATE_PUBLISHER_MAX_FRAMES_PER_WINDOW and "
                "BSB_STATE_PUBLISHER_WINDOW_S for rate limiter assertions"
            )

        max_frames_i = int(max_frames)
        window_f = float(window_s)
        period_ms = window_f * 1000.0
        state_publisher.drain(duration=0.3)

        for _ in range(20):
            response = input_api.send_key("back")
            assert response.status_code == 200

        # Collect frames over a couple of windows so we capture the burst window
        # plus any spill-over into the next one.
        deadline = time.monotonic() + (2 * window_f + 0.5)
        frames = []
        while time.monotonic() < deadline:
            try:
                frames.append(
                    state_publisher.read_state(timeout=max(0.1, deadline - time.monotonic()))
                )
            except Exception:
                break

        # The firmware limiter (rate_limiter.c) is a *tumbling* window: at most
        # `max_frames` per `period_ms`, with boundaries anchored to the device
        # clock — not to our wall-clock read window.
        #
        # Only count frames that carry input updates.  Heartbeats and non-input
        # state updates legitimately share the same transport and can land in the
        # same device-clock bucket without violating the cap, so including them
        # produces false positives.
        #
        # We allow max_frames_i + 1 rather than max_frames_i because when the
        # test's anchor timestamp (t0 = first input frame) is slightly *after*
        # the rate-limiter window start (W0), the batch-delivery frame that fires
        # at W0+period_ms can fall into the same bucket as the individual frames.
        # A correct rate limiter still emits at most max_frames_i individual
        # frames plus one batch frame in that window, so the cap is
        # max_frames_i + 1.  Any regression that removes the rate limit entirely
        # would push 40+ input frames into one bucket, far exceeding this bound.
        assert frames, "burst produced no published frames"
        input_frames = [f for f in frames if "input" in f.update_kinds]
        assert input_frames, "burst produced no input frames"
        input_timestamps = [f.timestamp for f in input_frames if f.timestamp is not None]
        assert len(input_timestamps) == len(input_frames), (
            "every input frame must carry a device timestamp for per-window bucketing"
        )

        t0 = min(input_timestamps)
        buckets: dict[int, int] = {}
        for ts in input_timestamps:
            bucket = int((ts - t0) // period_ms)
            buckets[bucket] = buckets.get(bucket, 0) + 1
        worst = max(buckets.values())
        assert worst <= max_frames_i + 1, (
            f"rate limiter exceeded: {worst} input frames in one {window_f}s window "
            f"(cap {max_frames_i}); per-window counts={dict(sorted(buckets.items()))}"
        )

        time.sleep(window_f)

        response = input_api.send_key("back")
        assert response.status_code == 200
        frame = state_publisher.wait_for_update("input", timeout=max(5.0, window_f + 2))
        assert "input" in frame.update_kinds
