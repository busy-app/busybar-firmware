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
        state_publisher.drain(duration=0.3)

        for _ in range(20):
            response = input_api.send_key("back")
            assert response.status_code == 200

        deadline = time.monotonic() + window_f
        frames = []
        while time.monotonic() < deadline:
            try:
                frames.append(
                    state_publisher.read_state(timeout=max(0.1, deadline - time.monotonic()))
                )
            except Exception:
                break

        assert len(frames) <= max_frames_i
        time.sleep(window_f)

        response = input_api.send_key("back")
        assert response.status_code == 200
        frame = state_publisher.wait_for_update("input", timeout=max(5.0, window_f + 2))
        assert "input" in frame.update_kinds
