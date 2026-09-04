"""Integration coverage for selective display deletion and element layering."""

from __future__ import annotations

import allure
import pytest
import requests

from clients.api import AssetsAPI, StreamingAPI
from clients.api.assets import DEFAULT_ELEMENT_PRIORITY
from clients.api.streaming import (
    FRONT_DISPLAY_HEIGHT,
    FRONT_DISPLAY_WIDTH,
)
from .display_helpers import (
    BGR_BLACK,
    BGR_GREEN,
    BGR_RED,
    FILL_GREEN,
    FILL_RED,
    capture_stable_front_frame as _capture_stable_front_frame,
    frame_digest as _frame_digest,
    solid_rectangle as _solid_rectangle,
    wait_for_front_frame,
)


_APP_NAME = "display_elements_test"
_OTHER_APP_NAME = "display_elements_other"


@pytest.fixture(autouse=True)
def _clear_test_application_canvases(assets_api: AssetsAPI):
    """Keep module-owned canvas elements isolated between tests."""
    app_names = (_APP_NAME, _OTHER_APP_NAME)

    for app_name in app_names:
        assets_api.clear_display_raw({}, app_name=app_name)

    yield

    for app_name in app_names:
        assets_api.clear_display_raw({}, app_name=app_name)


def _expected_front_frame(
    spans: list[tuple[int, int, bytes]],
) -> bytes:
    """Build an exact 72x16 raw front frame (BGR) from horizontal spans."""
    frame = bytearray(BGR_BLACK * FRONT_DISPLAY_WIDTH * FRONT_DISPLAY_HEIGHT)
    for x, width, bgr in spans:
        assert len(bgr) == 3, f"Expected a BGR triplet, got {len(bgr)} bytes"
        for y in range(FRONT_DISPLAY_HEIGHT):
            start = (y * FRONT_DISPLAY_WIDTH + x) * 3
            frame[start : start + width * 3] = bgr * width
    return bytes(frame)


def _wait_for_raw_front_frame(
    streaming_api: StreamingAPI,
    predicate,
    description: str,
) -> bytes:
    """Adapter for whole-frame byte comparisons over wait_for_front_frame."""
    frame = wait_for_front_frame(
        streaming_api,
        lambda current: predicate(current.raw),
        description,
    )
    return frame.raw


def _draw_and_capture(
    assets_api: AssetsAPI,
    streaming_api: StreamingAPI,
    elements: list[dict],
) -> bytes:
    response = assets_api.draw_response(
        _APP_NAME,
        elements,
        priority=DEFAULT_ELEMENT_PRIORITY,
    )
    assert (
        response.status_code == 200
    ), f"Expected draw status 200, got {response.status_code}: {response.text[:200]}"
    return _capture_stable_front_frame(streaming_api).raw


def _assert_rejected_without_frame_change(
    response: requests.Response,
    before: bytes,
    after: bytes,
) -> None:
    errors: list[str] = []
    if response.status_code != 400:
        errors.append(
            f"expected status 400, got {response.status_code}: {response.text[:200]}"
        )
    if after != before:
        errors.append(
            "canvas changed after rejected deletion: "
            f"before_sha256={_frame_digest(before)}, "
            f"after_sha256={_frame_digest(after)}"
        )
    assert not errors, "; ".join(errors)


def _assert_accepted_without_frame_change(
    response: requests.Response,
    before: bytes,
    after: bytes,
) -> None:
    errors: list[str] = []
    if response.status_code != 200:
        errors.append(
            f"expected status 200, got {response.status_code}: {response.text[:200]}"
        )
    if after != before:
        errors.append(
            "canvas changed after a no-op deletion: "
            f"before_sha256={_frame_digest(before)}, "
            f"after_sha256={_frame_digest(after)}"
        )
    assert not errors, "; ".join(errors)


@allure.feature("5. Web Frontend")
@allure.story("Draw API – selective deletion")
class TestSelectiveDisplayDeletion:
    @allure.title("DELETE with matching application_name clears the owned canvas")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_all_with_matching_app_name(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        with allure.step("Draw and verify an owned full-screen element"):
            before = _draw_and_capture(
                assets_api,
                streaming_api,
                [_solid_rectangle("owned_element", FILL_RED)],
            )
            expected = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_RED)])
            assert before == expected, (
                f"Expected owned frame sha256={_frame_digest(expected)}, "
                f"got sha256={_frame_digest(before)}"
            )

        with allure.step("Delete all elements using the matching application name"):
            response = assets_api.clear_display_by_app(_APP_NAME)
            assert response.status_code == 200, (
                f"Expected status 200, got {response.status_code}: "
                f"{response.text[:200]}"
            )

        with allure.step("Verify the owned frame disappears"):
            after = _wait_for_raw_front_frame(
                streaming_api,
                lambda frame: frame != before,
                "the owned canvas frame to disappear",
            )
            assert after != before, (
                f"Expected canvas to change from sha256={_frame_digest(before)}, "
                f"got unchanged sha256={_frame_digest(after)}"
            )

    @allure.title("DELETE with an empty JSON object clears all elements")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_empty_object_deletes_all_elements(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        with allure.step("Draw and verify an owned full-screen element"):
            before = _draw_and_capture(
                assets_api,
                streaming_api,
                [_solid_rectangle("owned_element", FILL_RED)],
            )
            expected = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_RED)])
            assert before == expected, (
                f"Expected owned frame sha256={_frame_digest(expected)}, "
                f"got sha256={_frame_digest(before)}"
            )

        with allure.step("Delete using an empty JSON object"):
            response = assets_api.clear_display_raw({}, app_name=_APP_NAME)
            assert response.status_code == 200, (
                f"Expected status 200, got {response.status_code}: "
                f"{response.text[:200]}"
            )

        with allure.step("Verify the owned frame disappears"):
            after = _wait_for_raw_front_frame(
                streaming_api,
                lambda frame: frame != before,
                "the owned canvas frame to disappear",
            )
            assert after != before, (
                f"Expected canvas to change from sha256={_frame_digest(before)}, "
                f"got unchanged sha256={_frame_digest(after)}"
            )

    @allure.title("DELETE with an empty element_ids array is a no-op")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_empty_element_ids_is_no_op(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        with allure.step("Draw and verify an element that must remain"):
            before = _draw_and_capture(
                assets_api,
                streaming_api,
                [_solid_rectangle("owned_element", FILL_RED)],
            )
            expected = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_RED)])
            assert before == expected, (
                f"Expected owned frame sha256={_frame_digest(expected)}, "
                f"got sha256={_frame_digest(before)}"
            )

        with allure.step("Delete an empty list of element IDs"):
            response = assets_api.clear_display_elements_response(
                [],
                app_name=_APP_NAME,
            )
            after = streaming_api.get_screen_bytes(display=0)

        with allure.step("Verify status 200 and unchanged canvas"):
            _assert_accepted_without_frame_change(response, before, after)

    @allure.title("DELETE removes only the requested display element")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        "app_name",
        [_APP_NAME, None],
        ids=["with-owner-check", "without-owner-check"],
    )
    def test_delete_selected_element(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
        app_name: str | None,
    ):
        keep = _solid_rectangle(
            "keep_right",
            FILL_GREEN,
            x=FRONT_DISPLAY_WIDTH // 2,
            width=FRONT_DISPLAY_WIDTH // 2,
        )
        remove = _solid_rectangle(
            "remove_left",
            FILL_RED,
            width=FRONT_DISPLAY_WIDTH // 2,
        )

        expected = _expected_front_frame(
            [(FRONT_DISPLAY_WIDTH // 2, FRONT_DISPLAY_WIDTH // 2, BGR_GREEN)]
        )
        expected_before = _expected_front_frame(
            [
                (0, FRONT_DISPLAY_WIDTH // 2, BGR_RED),
                (FRONT_DISPLAY_WIDTH // 2, FRONT_DISPLAY_WIDTH // 2, BGR_GREEN),
            ]
        )

        with allure.step("Draw and verify both independent elements pixel-for-pixel"):
            before = _draw_and_capture(assets_api, streaming_api, [remove, keep])
            assert before == expected_before, (
                f"Expected initial frame sha256={_frame_digest(expected_before)}, "
                f"got sha256={_frame_digest(before)}"
            )

        with allure.step("Delete only remove_left"):
            result = assets_api.clear_display_elements(
                ["remove_left"],
                app_name=app_name,
            )
            assert result.result, f"Expected non-empty result, got {result.result!r}"

        with allure.step("Verify keep_right remains and remove_left disappears"):
            actual = _wait_for_raw_front_frame(
                streaming_api,
                lambda frame: frame == expected,
                "the kept-element reference after selective deletion",
            )
            assert actual == expected, (
                f"Expected frame sha256={_frame_digest(expected)}, "
                f"got sha256={_frame_digest(actual)}"
            )

    @allure.title("DELETE with a mismatched application_name is rejected")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_with_wrong_app_name(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        with allure.step("Draw a stable frame owned by the test application"):
            before = _draw_and_capture(
                assets_api,
                streaming_api,
                [_solid_rectangle("owned_element", FILL_RED)],
            )
            expected = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_RED)])
            assert before == expected, (
                f"Expected owned frame sha256={_frame_digest(expected)}, "
                f"got sha256={_frame_digest(before)}"
            )

        with allure.step("Attempt full deletion using another application name"):
            response = assets_api.clear_display_by_app(_OTHER_APP_NAME)
            after = streaming_api.get_screen_bytes(display=0)

        with allure.step("Verify status 400 and unchanged canvas"):
            _assert_rejected_without_frame_change(response, before, after)

    @allure.title("DELETE honors application_name from the request body")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_with_wrong_app_name_in_body(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        with allure.step("Draw a stable frame owned by the test application"):
            before = _draw_and_capture(
                assets_api,
                streaming_api,
                [_solid_rectangle("owned_element", FILL_RED)],
            )
            expected = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_RED)])
            assert before == expected, (
                f"Expected owned frame sha256={_frame_digest(expected)}, "
                f"got sha256={_frame_digest(before)}"
            )

        with allure.step("Attempt deletion using another application name in JSON"):
            response = assets_api.clear_display_raw(
                {"application_name": _OTHER_APP_NAME}
            )
            after = streaming_api.get_screen_bytes(display=0)

        with allure.step("Verify status 400 and unchanged canvas"):
            _assert_rejected_without_frame_change(response, before, after)

    @allure.title("Selective DELETE with a mismatched application_name is rejected")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_selected_with_wrong_app_name(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        with allure.step("Draw a stable frame owned by the test application"):
            before = _draw_and_capture(
                assets_api,
                streaming_api,
                [_solid_rectangle("owned_element", FILL_RED)],
            )
            expected = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_RED)])
            assert before == expected, (
                f"Expected owned frame sha256={_frame_digest(expected)}, "
                f"got sha256={_frame_digest(before)}"
            )

        with allure.step("Attempt selective deletion using another application name"):
            response = assets_api.clear_display_elements_response(
                ["owned_element"],
                app_name=_OTHER_APP_NAME,
            )
            after = streaming_api.get_screen_bytes(display=0)

        with allure.step("Verify status 400 and unchanged canvas"):
            _assert_rejected_without_frame_change(response, before, after)

    @allure.title("Selective DELETE is atomic when one element ID does not exist")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_is_atomic_for_unknown_element_id(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        with allure.step("Draw a stable element"):
            before = _draw_and_capture(
                assets_api,
                streaming_api,
                [_solid_rectangle("existing_element", FILL_RED)],
            )
            expected = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_RED)])
            assert before == expected, (
                f"Expected initial frame sha256={_frame_digest(expected)}, "
                f"got sha256={_frame_digest(before)}"
            )

        with allure.step("Request deletion of one existing and one unknown ID"):
            response = assets_api.clear_display_elements_response(
                ["existing_element", "missing_element"],
                app_name=_APP_NAME,
            )
            after = streaming_api.get_screen_bytes(display=0)

        with allure.step("Verify status 400 and atomic state preservation"):
            _assert_rejected_without_frame_change(response, before, after)

    @allure.title("DELETE removes multiple requested elements")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_multiple_elements(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        keep = _solid_rectangle(
            "keep_middle",
            FILL_GREEN,
            x=FRONT_DISPLAY_WIDTH // 3,
            width=FRONT_DISPLAY_WIDTH // 3,
        )
        remove_left = _solid_rectangle(
            "remove_left",
            FILL_RED,
            width=FRONT_DISPLAY_WIDTH // 3,
        )
        remove_right = _solid_rectangle(
            "remove_right",
            FILL_RED,
            x=(FRONT_DISPLAY_WIDTH // 3) * 2,
            width=FRONT_DISPLAY_WIDTH // 3,
        )

        expected = _expected_front_frame(
            [(FRONT_DISPLAY_WIDTH // 3, FRONT_DISPLAY_WIDTH // 3, BGR_GREEN)]
        )
        expected_before = _expected_front_frame(
            [
                (0, FRONT_DISPLAY_WIDTH // 3, BGR_RED),
                (FRONT_DISPLAY_WIDTH // 3, FRONT_DISPLAY_WIDTH // 3, BGR_GREEN),
                ((FRONT_DISPLAY_WIDTH // 3) * 2, FRONT_DISPLAY_WIDTH // 3, BGR_RED),
            ]
        )

        with allure.step("Draw and verify all three elements pixel-for-pixel"):
            before = _draw_and_capture(
                assets_api,
                streaming_api,
                [remove_left, keep, remove_right],
            )
            assert before == expected_before, (
                f"Expected initial frame sha256={_frame_digest(expected_before)}, "
                f"got sha256={_frame_digest(before)}"
            )

        with allure.step("Delete both outer elements"):
            result = assets_api.clear_display_elements(
                ["remove_left", "remove_right"],
                app_name=_APP_NAME,
            )
            assert result.result, f"Expected non-empty result, got {result.result!r}"

        with allure.step("Verify only the middle element remains"):
            actual = _wait_for_raw_front_frame(
                streaming_api,
                lambda frame: frame == expected,
                "the middle-element reference after selective deletion",
            )
            assert actual == expected, (
                f"Expected frame sha256={_frame_digest(expected)}, "
                f"got sha256={_frame_digest(actual)}"
            )

    @allure.title("Duplicate element IDs are deleted once")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_duplicate_element_ids_are_idempotent(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        keep = _solid_rectangle(
            "keep_right",
            FILL_GREEN,
            x=FRONT_DISPLAY_WIDTH // 2,
            width=FRONT_DISPLAY_WIDTH // 2,
        )
        remove = _solid_rectangle(
            "remove_left",
            FILL_RED,
            width=FRONT_DISPLAY_WIDTH // 2,
        )
        expected = _expected_front_frame(
            [(FRONT_DISPLAY_WIDTH // 2, FRONT_DISPLAY_WIDTH // 2, BGR_GREEN)]
        )

        with allure.step("Draw both independent elements"):
            _draw_and_capture(assets_api, streaming_api, [remove, keep])

        with allure.step("Request the same element ID twice"):
            response = assets_api.clear_display_elements_response(
                ["remove_left", "remove_left"],
                app_name=_APP_NAME,
            )
            assert response.status_code == 200, (
                f"Expected status 200, got {response.status_code}: "
                f"{response.text[:200]}"
            )

        with allure.step(
            "Verify the duplicate was deleted once and keep_right remains"
        ):
            actual = _wait_for_raw_front_frame(
                streaming_api,
                lambda frame: frame == expected,
                "the kept-element frame after duplicate-ID deletion",
            )
            assert actual == expected, (
                f"Expected frame sha256={_frame_digest(expected)}, "
                f"got sha256={_frame_digest(actual)}"
            )

    @allure.title("DELETE rejects a non-array element_ids value")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_rejects_non_array_element_ids(
        self,
        assets_api: AssetsAPI,
        busy_timer_stopped,
    ):
        response = assets_api.clear_display_raw(
            {"element_ids": "owned_element"},
            app_name=_APP_NAME,
        )
        assert (
            response.status_code == 400
        ), f"Expected status 400, got {response.status_code}: {response.text[:200]}"

    @allure.title("DELETE rejects a non-string element ID without changing canvas")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_rejects_non_string_element_id(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        with allure.step("Draw and verify an element that must be preserved"):
            expected = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_RED)])
            before = _draw_and_capture(
                assets_api,
                streaming_api,
                [_solid_rectangle("owned_element", FILL_RED)],
            )
            assert before == expected, (
                f"Expected initial frame sha256={_frame_digest(expected)}, "
                f"got sha256={_frame_digest(before)}"
            )

        with allure.step("Send a mixed string and non-string element ID list"):
            response = assets_api.clear_display_raw(
                {"element_ids": ["owned_element", 123]},
                app_name=_APP_NAME,
            )
            after = streaming_api.get_screen_bytes(display=0)

        with allure.step("Verify status 400 and unchanged canvas"):
            _assert_rejected_without_frame_change(response, before, after)

    @allure.title("DELETE rejects null element_ids without changing canvas")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_rejects_null_element_ids(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        with allure.step("Draw and verify an element that must be preserved"):
            before = _draw_and_capture(
                assets_api,
                streaming_api,
                [_solid_rectangle("owned_element", FILL_RED)],
            )

        with allure.step("Send element_ids as null"):
            response = assets_api.clear_display_raw(
                {"element_ids": None},
                app_name=_APP_NAME,
            )
            after = streaming_api.get_screen_bytes(display=0)

        with allure.step("Verify status 400 and unchanged canvas"):
            _assert_rejected_without_frame_change(response, before, after)

    @allure.title("DELETE rejects an unknown element ID without changing canvas")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_rejects_unknown_element_id(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        with allure.step("Draw and verify an element that must be preserved"):
            before = _draw_and_capture(
                assets_api,
                streaming_api,
                [_solid_rectangle("owned_element", FILL_RED)],
            )

        with allure.step("Request deletion of an unknown ID"):
            response = assets_api.clear_display_elements_response(
                ["missing_element"],
                app_name=_APP_NAME,
            )
            after = streaming_api.get_screen_bytes(display=0)

        with allure.step("Verify status 400 and unchanged canvas"):
            _assert_rejected_without_frame_change(response, before, after)

    @allure.title("DELETE rejects an invalid element ID without changing canvas")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_rejects_invalid_element_id(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        with allure.step("Draw and verify an element that must be preserved"):
            before = _draw_and_capture(
                assets_api,
                streaming_api,
                [_solid_rectangle("owned_element", FILL_RED)],
            )

        with allure.step("Request deletion using an ID outside the schema pattern"):
            response = assets_api.clear_display_elements_response(
                ["invalid element/id"],
                app_name=_APP_NAME,
            )
            after = streaming_api.get_screen_bytes(display=0)

        with allure.step("Verify status 400 and unchanged canvas"):
            _assert_rejected_without_frame_change(response, before, after)

    @allure.title("DELETE rejects malformed JSON when a body is provided")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_rejects_malformed_json(
        self,
        assets_api: AssetsAPI,
        busy_timer_stopped,
    ):
        response = assets_api.clear_display_body_response(
            '{"element_ids": ["owned_element"]',
            app_name=_APP_NAME,
        )
        assert (
            response.status_code == 400
        ), f"Expected status 400, got {response.status_code}: {response.text[:200]}"

    @allure.title("Deleting the last element closes canvas for another application")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_last_element_closes_canvas(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        with allure.step("Draw and verify the only canvas element"):
            before = _draw_and_capture(
                assets_api,
                streaming_api,
                [_solid_rectangle("only_element", FILL_RED)],
            )
            expected_before = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_RED)])
            assert before == expected_before, (
                f"Expected initial frame sha256={_frame_digest(expected_before)}, "
                f"got sha256={_frame_digest(before)}"
            )

        with allure.step("Delete the last element selectively"):
            result = assets_api.clear_display_elements(
                ["only_element"],
                app_name=_APP_NAME,
            )
            assert result.result, f"Expected non-empty result, got {result.result!r}"
            _wait_for_raw_front_frame(
                streaming_api,
                lambda frame: frame != before,
                "the canvas frame to disappear after its last element is deleted",
            )

        with allure.step("Verify another application can acquire and draw the canvas"):
            expected_after = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_GREEN)])
            response = assets_api.draw_response(
                _OTHER_APP_NAME,
                [_solid_rectangle("new_owner", FILL_GREEN)],
                priority=DEFAULT_ELEMENT_PRIORITY,
            )
            assert response.status_code == 200, (
                f"Expected draw status 200, got {response.status_code}: "
                f"{response.text[:200]}"
            )
            actual = _capture_stable_front_frame(streaming_api).raw
            assert actual == expected_after, (
                f"Expected new owner frame sha256={_frame_digest(expected_after)}, "
                f"got sha256={_frame_digest(actual)}"
            )


@allure.feature("5. Web Frontend")
@allure.story("Draw API – z-index")
class TestDisplayZIndex:
    @allure.title("Higher z_index is rendered on top regardless of payload order")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_higher_z_index_wins_over_payload_order(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        lower = _solid_rectangle("lower", FILL_RED, z_index=10)
        higher = _solid_rectangle("higher", FILL_GREEN, z_index=100)
        lower_reference = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_RED)])
        higher_reference = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_GREEN)])

        with allure.step("Verify the two exact reference frames are distinct"):
            assert lower_reference != higher_reference, (
                "Negative control failed: colored references are identical, "
                f"sha256={_frame_digest(lower_reference)}"
            )

        with allure.step("Draw higher first and lower second"):
            actual = _draw_and_capture(assets_api, streaming_api, [higher, lower])

        with allure.step("Verify the higher z-index frame is visible"):
            assert actual == higher_reference, (
                f"Expected higher frame sha256={_frame_digest(higher_reference)}, "
                f"got sha256={_frame_digest(actual)}; "
                f"lower_sha256={_frame_digest(lower_reference)}"
            )

    @allure.title("z_index ordering is preserved across separate draw requests")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_z_index_is_global_across_draw_requests(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        higher = _solid_rectangle("higher", FILL_GREEN, z_index=100)
        lower = _solid_rectangle("lower", FILL_RED, z_index=10)
        higher_reference = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_GREEN)])

        with allure.step("Draw and verify the higher z-index element"):
            actual = _draw_and_capture(assets_api, streaming_api, [higher])
            assert actual == higher_reference, (
                f"Expected higher frame sha256={_frame_digest(higher_reference)}, "
                f"got sha256={_frame_digest(actual)}"
            )

        with allure.step("Draw the lower z-index element in a later request"):
            response = assets_api.draw_response(
                _APP_NAME,
                [lower],
                priority=DEFAULT_ELEMENT_PRIORITY,
            )
            assert response.status_code == 200, (
                f"Expected draw status 200, got {response.status_code}: "
                f"{response.text[:200]}"
            )
            actual = _capture_stable_front_frame(streaming_api).raw

        with allure.step("Verify the existing higher z-index element remains on top"):
            assert actual == higher_reference, (
                f"Expected higher frame sha256={_frame_digest(higher_reference)}, "
                f"got sha256={_frame_digest(actual)}"
            )

    @allure.title("Elements without z_index preserve their payload order")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_default_z_index_preserves_payload_order(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        first = _solid_rectangle("first", FILL_RED)
        last = _solid_rectangle("last", FILL_GREEN)
        last_reference = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_GREEN)])

        with allure.step("Draw both elements without explicit z-indexes"):
            actual = _draw_and_capture(assets_api, streaming_api, [first, last])

        with allure.step("Verify the later payload element is visible on top"):
            assert actual == last_reference, (
                f"Expected last frame sha256={_frame_digest(last_reference)}, "
                f"got sha256={_frame_digest(actual)}"
            )

    @allure.title(
        "Implicit element {implicit_count} receives the expected default z_index"
    )
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        ("implicit_count", "explicit_z_index", "implicit_is_top"),
        [(1, 1, False), (2, 5, True), (3, 15, True)],
        ids=["default-0", "default-10", "default-20"],
    )
    def test_default_z_index_values_start_at_zero(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
        implicit_count: int,
        explicit_z_index: int,
        implicit_is_top: bool,
    ):
        implicit_elements = [
            _solid_rectangle(f"implicit_{index}", FILL_RED)
            for index in range(1, implicit_count)
        ]
        implicit_elements.append(
            _solid_rectangle(f"implicit_{implicit_count}", FILL_GREEN)
        )
        explicit_lower = _solid_rectangle(
            "explicit_lower",
            FILL_RED,
            z_index=explicit_z_index,
        )
        expected_rgb = BGR_GREEN if implicit_is_top else BGR_RED
        expected = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, expected_rgb)])

        with allure.step(
            f"Draw {implicit_count} implicit elements followed by explicit "
            f"z_index={explicit_z_index}"
        ):
            actual = _draw_and_capture(
                assets_api,
                streaming_api,
                [*implicit_elements, explicit_lower],
            )

        with allure.step(
            f"Verify implicit element {implicit_count} uses "
            f"z_index={(implicit_count - 1) * 10}"
        ):
            assert actual == expected, (
                f"Expected frame sha256={_frame_digest(expected)}, "
                f"got sha256={_frame_digest(actual)}"
            )

    @allure.title("Explicit z_index does not consume the next default z_index")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_explicit_z_index_does_not_advance_default_sequence(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        first_implicit = _solid_rectangle("implicit_0", FILL_RED)
        explicit = _solid_rectangle("explicit_15", FILL_RED, z_index=15)
        second_implicit = _solid_rectangle("implicit_10", FILL_GREEN)
        expected = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_RED)])

        with allure.step(
            "Draw implicit, explicit z_index=15, and another implicit element"
        ):
            actual = _draw_and_capture(
                assets_api,
                streaming_api,
                [first_implicit, explicit, second_implicit],
            )

        with allure.step("Verify assigned z-indexes are 0, 15, and 10"):
            assert actual == expected, (
                f"Expected explicit z_index=15 frame sha256={_frame_digest(expected)}, "
                f"got sha256={_frame_digest(actual)}; "
                "the second implicit element should use z_index=10"
            )

    @allure.title("Explicit z_index takes precedence over default payload ordering")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_explicit_z_index_with_default_z_index(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        explicit_higher = _solid_rectangle("explicit", FILL_GREEN, z_index=100)
        implicit_later = _solid_rectangle("implicit", FILL_RED)
        expected = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_GREEN)])

        with allure.step("Draw explicit higher element before an implicit element"):
            actual = _draw_and_capture(
                assets_api,
                streaming_api,
                [explicit_higher, implicit_later],
            )

        with allure.step("Verify explicit z-index remains on top pixel-for-pixel"):
            assert actual == expected, (
                f"Expected explicit frame sha256={_frame_digest(expected)}, "
                f"got sha256={_frame_digest(actual)}"
            )

    @allure.title("Updating existing elements applies their new z_index ordering")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_updating_elements_reorders_z_index(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        red_lower = _solid_rectangle("red", FILL_RED, z_index=10)
        green_higher = _solid_rectangle("green", FILL_GREEN, z_index=100)
        expected_green = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_GREEN)])
        expected_red = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_RED)])

        with allure.step("Draw and verify the initial z-index ordering"):
            initial = _draw_and_capture(
                assets_api,
                streaming_api,
                [red_lower, green_higher],
            )
            assert initial == expected_green, (
                f"Expected initial frame sha256={_frame_digest(expected_green)}, "
                f"got sha256={_frame_digest(initial)}"
            )

        with allure.step("Swap z-indexes while updating the same element IDs"):
            response = assets_api.draw_response(
                _APP_NAME,
                [
                    _solid_rectangle("red", FILL_RED, z_index=200),
                    _solid_rectangle("green", FILL_GREEN, z_index=0),
                ],
                priority=DEFAULT_ELEMENT_PRIORITY,
            )
            assert response.status_code == 200, (
                f"Expected draw status 200, got {response.status_code}: "
                f"{response.text[:200]}"
            )
            actual = _capture_stable_front_frame(streaming_api).raw

        with allure.step("Verify the updated higher element is now visible"):
            assert actual == expected_red, (
                f"Expected reordered frame sha256={_frame_digest(expected_red)}, "
                f"got sha256={_frame_digest(actual)}"
            )

    @allure.title("Omitted z_index resets an existing element to the default value")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_updating_element_without_z_index_uses_default(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        red_lower = _solid_rectangle("red", FILL_RED, z_index=100)
        green_higher = _solid_rectangle("green", FILL_GREEN, z_index=200)
        expected_green = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_GREEN)])
        expected_red = _expected_front_frame([(0, FRONT_DISPLAY_WIDTH, BGR_RED)])

        with allure.step("Draw and verify the initial explicit z-index ordering"):
            initial = _draw_and_capture(
                assets_api,
                streaming_api,
                [red_lower, green_higher],
            )
            assert initial == expected_green, (
                f"Expected initial frame sha256={_frame_digest(expected_green)}, "
                f"got sha256={_frame_digest(initial)}"
            )

        with allure.step("Update the higher element without a z_index field"):
            response = assets_api.draw_response(
                _APP_NAME,
                [_solid_rectangle("green", FILL_GREEN)],
                priority=DEFAULT_ELEMENT_PRIORITY,
            )
            assert response.status_code == 200, (
                f"Expected draw status 200, got {response.status_code}: "
                f"{response.text[:200]}"
            )
            actual = _capture_stable_front_frame(streaming_api).raw

        with allure.step("Verify the omitted z_index was reset to default 0"):
            assert actual == expected_red, (
                f"Expected z_index=100 frame sha256={_frame_digest(expected_red)}, "
                f"got sha256={_frame_digest(actual)}; "
                "the updated element should use default z_index=0"
            )

    @allure.title("Invalid z_index value {z_index!r} is rejected")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        "z_index",
        [-1, "10"],
        ids=["negative", "string"],
    )
    def test_invalid_z_index_is_rejected(
        self,
        assets_api: AssetsAPI,
        busy_timer_stopped,
        z_index: int | str,
    ):
        response = assets_api.draw_response(
            _APP_NAME,
            [_solid_rectangle("invalid_z", FILL_RED, z_index=z_index)],
            priority=DEFAULT_ELEMENT_PRIORITY,
        )
        assert response.status_code == 400, (
            f"Expected status 400 for z_index={z_index!r}, "
            f"got {response.status_code}: {response.text[:200]}"
        )

    @allure.title("Boundary z_index value {z_index} is accepted")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("z_index", [0, 2147483647], ids=["minimum", "maximum"])
    def test_boundary_z_index_is_accepted(
        self,
        assets_api: AssetsAPI,
        busy_timer_stopped,
        z_index: int,
    ):
        response = assets_api.draw_response(
            _APP_NAME,
            [_solid_rectangle("boundary_z", FILL_RED, z_index=z_index)],
            priority=DEFAULT_ELEMENT_PRIORITY,
        )
        assert response.status_code == 200, (
            f"Expected status 200 for z_index={z_index}, "
            f"got {response.status_code}: {response.text[:200]}"
        )
