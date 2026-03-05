import allure
import pytest

from clients.api import BusyAPI


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

    @allure.title("GET /api/busy/profiles/custom")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_busy_profile_get_custom(self, busy_api: BusyAPI):
        """Test GET /api/busy/profiles/custom returns valid profile"""
        response = busy_api.get_profile("custom")

        assert response.title
        assert response.id
        assert response.timer_settings is not None

    @allure.title("PUT /api/busy/profiles/custom")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_busy_profile_put_custom(self, busy_api: BusyAPI):
        """Test PUT /api/busy/profiles/custom accepts valid profile data"""
        # Save original
        original = busy_api.get_profile("custom")

        test_profile = {
            "sort_order": 0,
            "title": "test_profile",
            "id": original.id,
            "timer_settings": {
                "type": "SIMPLE",
                "total_time_ms": 600000,
            },
            "busy_bar_settings": {
                "theme": "busy",
                "show_work_phase_only": False,
                "trigger_smart_home": False,
            },
            "profile_timestamp_ms": 0,
        }

        try:
            with allure.step("Set test profile"):
                response = busy_api.set_profile_raw("custom", test_profile)
                assert response.status_code == 200

            with allure.step("Verify profile is readable after PUT"):
                updated = busy_api.get_profile("custom")
                assert updated.timer_settings is not None
        finally:
            with allure.step("Restore original profile"):
                restore_data = {
                    "sort_order": original.sort_order,
                    "title": original.title,
                    "id": original.id,
                    "timer_settings": original.timer_settings,
                    "busy_bar_settings": original.busy_bar_settings.model_dump(),
                    "profile_timestamp_ms": original.profile_timestamp_ms,
                }
                busy_api.set_profile_raw("custom", restore_data)

    @allure.title("GET /api/busy/profiles/{invalid_slot}")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_busy_profile_invalid_slot(self, busy_api: BusyAPI):
        """Test GET /api/busy/profiles with invalid slot returns error"""
        response = busy_api.get_profile_raw("invalid_slot")
        assert response.status_code == 400
