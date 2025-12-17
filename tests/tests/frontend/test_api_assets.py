import base64
import json
from pathlib import Path
from time import sleep

import allure
import pytest

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
            params = {"app_id": test_app_id, "file": test_filename}
            response = api_session.post(
                f"{web_base_url}/api/assets/upload",
                params=params,
                data=test_content,
                headers={"Content-Type": "application/octet-stream"},
                timeout=10,
            )

        with allure.step("Verify upload response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Asset Upload Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"

    @allure.id("2652")
    @allure.title("DELETE /api/assets/upload")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_assets_delete(self, api_session, web_base_url):
        """Test DELETE /api/assets/upload endpoint"""

        test_app_id = "test_app"

        with allure.step(f"Delete assets for app {test_app_id}"):
            params = {"app_id": test_app_id}
            response = api_session.delete(
                f"{web_base_url}/api/assets/upload", params=params, timeout=10
            )

        with allure.step("Verify delete response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Asset Delete Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"

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
            response = api_session.post(
                f"{web_base_url}/api/display/draw", json=display_data, timeout=10
            )

        with allure.step("Verify draw response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Display Draw Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"

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

        with allure.step(f"Upload test image: {test_image_file}"):
            with open(image_path, "rb") as f:
                image_content = f.read()
                #frame = "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"
                #image_content = base64.b64decode(frame)

            upload_response = api_session.post(
                f"{web_base_url}/api/assets/upload",
                params={"app_id": test_app_id, "file": test_image_file},
                data=image_content,
                headers={"Content-Type": "application/octet-stream"},
                timeout=5,
            )

            assert (
                upload_response.status_code == 200
            ), f"Expected 200 for upload, got {upload_response.status_code}"

            allure.attach(
                json.dumps(upload_response.json(), indent=2),
                "Image Upload Response",
                allure.attachment_type.JSON,
            )
        sleep(5)
        try:
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
                response = api_session.post(
                    f"{web_base_url}/api/display/draw", json=display_data, timeout=10
                )

            with allure.step("Verify draw response"):
                assert (
                    response.status_code == 200
                ), f"Expected 200, got {response.status_code}"

                response_data = response.json()
                allure.attach(
                    json.dumps(response_data, indent=2),
                    "Display Draw Image Response",
                    allure.attachment_type.JSON,
                )
                assert (
                    "result" in response_data
                ), "Success response should contain 'result' field"
        finally:
            with allure.step(f"Clean up uploaded asset for app {test_app_id}"):
                cleanup_response = api_session.delete(
                    f"{web_base_url}/api/assets/upload",
                    params={"app_id": test_app_id},
                    timeout=10,
                )
                allure.attach(
                    json.dumps(cleanup_response.json(), indent=2),
                    "Asset Cleanup Response",
                    allure.attachment_type.JSON,
                )

    @allure.id("2654")
    @allure.title("DELETE /api/display/draw")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="https://flipper.atlassian.net/browse/FW-500")
    def test_api_display_clear(self, api_session, web_base_url):
        """Test DELETE /api/display/draw endpoint"""

        with allure.step("Clear display"):
            response = api_session.delete(
                f"{web_base_url}/api/display/draw", timeout=10
            )

        with allure.step("Verify clear response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Display Clear Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"

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

        with allure.step(f"Upload test audio: {test_audio_file}"):
            with open(audio_path, "rb") as f:
                audio_content = f.read()

            params = {"app_id": test_app_id, "file": test_audio_file}
            upload_response = api_session.post(
                f"{web_base_url}/api/assets/upload",
                params=params,
                data=audio_content,
                headers={"Content-Type": "application/octet-stream"},
                timeout=10,
            )

            assert (
                upload_response.status_code == 200
            ), f"Expected 200 for upload, got {upload_response.status_code}"

            allure.attach(
                json.dumps(upload_response.json(), indent=2),
                "Audio Upload Response",
                allure.attachment_type.JSON,
            )

        try:
            with allure.step("Play audio file"):
                params = {"app_id": test_app_id, "path": test_audio_file}
                response = api_session.post(
                    f"{web_base_url}/api/audio/play", params=params, timeout=10
                )

            with allure.step("Verify play response"):
                assert (
                    response.status_code == 200
                ), f"Expected 200, got {response.status_code}"

                response_data = response.json()
                allure.attach(
                    json.dumps(response_data, indent=2),
                    "Audio Play Response",
                    allure.attachment_type.JSON,
                )
                assert (
                    "result" in response_data
                ), "Success response should contain 'result' field"
        finally:
            with allure.step(f"Clean up uploaded asset for app {test_app_id}"):
                cleanup_response = api_session.delete(
                    f"{web_base_url}/api/assets/upload",
                    params={"app_id": test_app_id},
                    timeout=10,
                )
                allure.attach(
                    json.dumps(cleanup_response.json(), indent=2),
                    "Asset Cleanup Response",
                    allure.attachment_type.JSON,
                )

    @allure.id("2656")
    @allure.title("DELETE /api/audio/play")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="https://flipper.atlassian.net/browse/FW-505")
    def test_api_audio_stop(self, api_session, web_base_url):
        """Test DELETE /api/audio/play endpoint"""

        with allure.step("Stop audio playback"):
            response = api_session.delete(f"{web_base_url}/api/audio/play", timeout=10)

        with allure.step("Verify stop response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Audio Stop Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"

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