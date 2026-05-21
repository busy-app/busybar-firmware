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
        # clock — not to our wall-clock read window. Counting every frame over a
        # fixed 1 s read window is therefore wrong: an unaligned window can
        # straddle two device windows and legitimately see up to ~2x the cap.
        # Instead bucket frames by their device-side timestamp (State.timestamp,
        # in ms) into period-sized bins and assert no single window exceeds it.
        assert frames, "burst produced no published frames"
        timestamps = [f.timestamp for f in frames if f.timestamp is not None]
        assert len(timestamps) == len(frames), (
            "every published frame must carry a device timestamp for per-window bucketing"
        )

        t0 = min(timestamps)
        buckets: dict[int, int] = {}
        for ts in timestamps:
            bucket = int((ts - t0) // period_ms)
            buckets[bucket] = buckets.get(bucket, 0) + 1
        worst = max(buckets.values())
        assert worst <= max_frames_i, (
            f"rate limiter exceeded: {worst} frames in one {window_f}s window "
            f"(cap {max_frames_i}); per-window counts={dict(sorted(buckets.items()))}"
        )

        time.sleep(window_f)

        response = input_api.send_key("back")
        assert response.status_code == 200
        frame = state_publisher.wait_for_update("input", timeout=max(5.0, window_f + 2))
        assert "input" in frame.update_kinds
