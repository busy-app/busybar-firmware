from pathlib import Path
from time import sleep

import allure
import pytest

from utils import api_get, api_post, api_delete, attach_json, APITestContext

ASSETS_DIR = Path(__file__).parent.parent / "assets"


@allure.feature("5. Web Frontend")
@allure.story("Assets")
class TestAssetsAPI:
    """Test cases for Assets API endpoints"""

    @allure.id("2651")
    @allure.title("POST /api/assets/upload")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_assets_upload(self, api_session, web_base_url):
        """Test POST /api/assets/upload endpoint"""

        test_app_id = "test_app"
        test_filename = "test_asset.txt"
        test_content = b"Test asset content"

        with allure.step(f"Upload asset for app {test_app_id}"):
            response = api_post(
                api_session, web_base_url, "/api/assets/upload",
                params={"app_id": test_app_id, "file": test_filename},
                data=test_content,
                headers={"Content-Type": "application/octet-stream"},
            )

        with allure.step("Verify upload response"):
            response.assert_ok()
            response.assert_has_fields("result").attach_to_allure("Asset Upload Response")

    @allure.id("2652")
    @allure.title("DELETE /api/assets/upload")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_assets_delete(self, api_session, web_base_url):
        """Test DELETE /api/assets/upload endpoint"""

        test_app_id = "test_app"

        with allure.step(f"Delete assets for app {test_app_id}"):
            response = api_delete(
                api_session, web_base_url, "/api/assets/upload",
                params={"app_id": test_app_id}
            )

        with allure.step("Verify delete response"):
            response.assert_ok()
            response.assert_has_fields("result").attach_to_allure("Asset Delete Response")

    @allure.id("2653")
    @allure.title("POST /api/display/draw")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_display_draw(self, api_session, web_base_url):
        """Test POST /api/display/draw endpoint"""

        display_data = {
            "app_id": "test_app",
            "elements": [
                {
                    "id": "1",
                    "timeout": 5,
                    "type": "text",
                    "text": "Hello API Test",
                    "x": 36,
                    "y": 10,
                    "align": "center",
                    "font": "medium",
                    "color": "#FFFFFFFF",
                    "display": "front",
                }
            ],
        }

        with allure.step("Send display draw command"):
            response = api_post(
                api_session, web_base_url, "/api/display/draw",
                json=display_data
            )

        with allure.step("Verify draw response"):
            response.assert_ok()
            response.assert_has_fields("result").attach_to_allure("Display Draw Response")

    @allure.id("2726")
    @allure.title("POST /api/display/draw (image)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="https://flipper.atlassian.net/browse/FW-505")
    def test_api_display_draw_image(self, api_session, web_base_url):
        """Test POST /api/display/draw endpoint with image element"""

        test_app_id = "test_display_img"
        test_image_file = "img.png"
        image_path = ASSETS_DIR / test_image_file

        assert image_path.exists(), f"Test image not found: {image_path}"

        with APITestContext(api_session, web_base_url) as ctx:
            with allure.step(f"Upload test image: {test_image_file}"):
                with open(image_path, "rb") as f:
                    image_content = f.read()

                upload_response = api_post(
                    api_session, web_base_url, "/api/assets/upload",
                    params={"app_id": test_app_id, "file": test_image_file},
                    data=image_content,
                    headers={"Content-Type": "application/octet-stream"},
                    timeout=5,
                )
                upload_response.assert_ok()
                upload_response.attach_to_allure("Image Upload Response")

                # Register cleanup
                ctx.add_cleanup(
                    lambda: api_delete(
                        api_session, web_base_url, "/api/assets/upload",
                        params={"app_id": test_app_id}
                    )
                )

            sleep(5)

            display_data = {
                "app_id": test_app_id,
                "elements": [
                    {
                        "id": "img",
                        "type": "image",
                        "path": test_image_file,
                        "x": 0,
                        "y": 0,
                    }
                ],
            }

            with allure.step("Send display draw command with image"):
                response = api_post(
                    api_session, web_base_url, "/api/display/draw",
                    json=display_data
                )

            with allure.step("Verify draw response"):
                response.assert_ok()
                response.assert_has_fields("result").attach_to_allure("Display Draw Image Response")

    @allure.id("2654")
    @allure.title("DELETE /api/display/draw")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="https://flipper.atlassian.net/browse/FW-500")
    def test_api_display_clear(self, api_session, web_base_url):
        """Test DELETE /api/display/draw endpoint"""

        with allure.step("Clear display"):
            response = api_delete(api_session, web_base_url, "/api/display/draw")

        with allure.step("Verify clear response"):
            response.assert_ok()
            response.assert_has_fields("result").attach_to_allure("Display Clear Response")

    @allure.id("2655")
    @allure.title("POST /api/audio/play")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="https://flipper.atlassian.net/browse/FW-505")
    def test_api_audio_play(self, api_session, web_base_url):
        """Test POST /api/audio/play endpoint"""

        test_app_id = "test_audio_play"
        test_audio_file = "ping.snd"
        audio_path = ASSETS_DIR / test_audio_file

        assert audio_path.exists(), f"Test audio file not found: {audio_path}"

        with APITestContext(api_session, web_base_url) as ctx:
            with allure.step(f"Upload test audio: {test_audio_file}"):
                with open(audio_path, "rb") as f:
                    audio_content = f.read()

                upload_response = api_post(
                    api_session, web_base_url, "/api/assets/upload",
                    params={"app_id": test_app_id, "file": test_audio_file},
                    data=audio_content,
                    headers={"Content-Type": "application/octet-stream"},
                )
                upload_response.assert_ok()
                upload_response.attach_to_allure("Audio Upload Response")

                # Register cleanup
                ctx.add_cleanup(
                    lambda: api_delete(
                        api_session, web_base_url, "/api/assets/upload",
                        params={"app_id": test_app_id}
                    )
                )

            with allure.step("Play audio file"):
                response = api_post(
                    api_session, web_base_url, "/api/audio/play",
                    params={"app_id": test_app_id, "path": test_audio_file}
                )

            with allure.step("Verify play response"):
                response.assert_ok()
                response.assert_has_fields("result").attach_to_allure("Audio Play Response")

    @allure.id("2656")
    @allure.title("DELETE /api/audio/play")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="https://flipper.atlassian.net/browse/FW-505")
    def test_api_audio_stop(self, api_session, web_base_url):
        """Test DELETE /api/audio/play endpoint"""

        with allure.step("Stop audio playback"):
            response = api_delete(api_session, web_base_url, "/api/audio/play")

        with allure.step("Verify stop response"):
            response.assert_ok()
            response.assert_has_fields("result").attach_to_allure("Audio Stop Response")

    @allure.id("2672")
    @allure.title("POST /api/display/draw (malformed JSON)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_malformed_json(self, api_session, web_base_url):
        """Test API endpoints with malformed JSON"""

        malformed_json = '{"invalid": "json", "missing": quote}'

        with allure.step("Send malformed JSON to display/draw"):
            response = api_session.post(
                f"{web_base_url}/api/display/draw",
                data=malformed_json,
                headers={"Content-Type": "application/json"},
                timeout=10,
            )

        with allure.step("Verify malformed JSON handling"):
            assert (
                response.status_code == 400
            ), f"Expected 400 for malformed JSON, got {response.status_code}"
