import allure
import pytest


EVENT_NAMES = ["input", "timer", "brightness", "timezone", "autoupdate"]


@allure.feature("State Publisher")
@allure.story("Published events")
@pytest.mark.api
@pytest.mark.state_publisher
@pytest.mark.regression
class TestStatePublisherEvents:
    @allure.title("State publisher emits expected update after device event")
    @pytest.mark.parametrize("event_name", EVENT_NAMES)
    def test_state_event_published(self, state_publisher, state_event_driver, event_name):
        event = state_event_driver[event_name]
        state_publisher.drain(duration=0.3)

        event.trigger()

        frame = state_publisher.wait_for_update(event.expected_kind)
        assert event.expected_kind in frame.update_kinds
