import allure
import pytest


@allure.feature("State Publisher")
@allure.story("Transport contract")
@pytest.mark.api
@pytest.mark.state_publisher
class TestStatePublisherContract:
    @allure.title("State publisher transport emits parseable frames")
    def test_transport_connect_enable_receives_parseable_frame(self, state_publisher):
        frame = state_publisher.read_state()

        assert frame.timestamp is None or frame.timestamp > 0
        assert isinstance(frame.updates, list)
        assert isinstance(frame.errors, list)

    @allure.title("State publisher transport reconnect emits parseable frames")
    def test_transport_reconnect_receives_parseable_frame(self, state_transport):
        state_transport.enable()
        first = state_transport.read_state()
        state_transport.close()
        state_transport.connect()
        state_transport.enable()
        second = state_transport.read_state()

        assert first.raw
        assert second.raw
