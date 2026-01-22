from pathlib import Path
from time import sleep

import allure
import pytest

from clients.api import AssetsAPI


ASSETS_DIR = Path(__file__).parent.parent / "assets"


@allure.feature("5. Web Frontend")
@allure.story("Assets")
class TestAssetsAPI:
    """Test cases for Assets API endpoints"""

    @allure.id("2651")
    @allure.title("POST /api/assets/upload")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_assets_upload(self, assets_api: AssetsAPI):
        """Test POST /api/assets/upload endpoint"""
        test_app_id = "test_app"
        test_filename = "test_asset.txt"
        test_content = b"Test asset content"

        assets_api.upload_asset(test_app_id, test_filename, test_content)

    @allure.id("2652")
    @allure.title("DELETE /api/assets/upload")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_assets_delete(self, assets_api: AssetsAPI):
        """Test DELETE /api/assets/upload endpoint"""
        test_app_id = "test_app"

        assets_api.delete_assets(test_app_id)

    @allure.id("2653")
    @allure.title("POST /api/display/draw")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_display_draw(self, assets_api: AssetsAPI):
        """Test POST /api/display/draw endpoint"""
        elements = [
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
        ]

        assets_api.draw("test_app", elements)

    @allure.id("2690")
    @allure.title("POST /api/display/draw (image)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.xfail(reason="Test image asset (img.png) not present in repository")
    def test_api_display_draw_image(self, assets_api: AssetsAPI):
        """Test POST /api/display/draw endpoint with image element"""
        test_app_id = "test_display_img"
        test_image_file = "img.png"
        image_path = ASSETS_DIR / test_image_file

        assert image_path.exists(), f"Test image not found: {image_path}"

        with allure.step(f"Upload test image: {test_image_file}"):
            with open(image_path, "rb") as f:
                image_content = f.read()

            assets_api.upload_asset(
                test_app_id, test_image_file, image_content, timeout=5
            )

        try:
            sleep(5)

            elements = [
                {
                    "id": "img",
                    "type": "image",
                    "path": test_image_file,
                    "x": 0,
                    "y": 0,
                }
            ]

            assets_api.draw(test_app_id, elements)
        finally:
            assets_api.delete_assets(test_app_id)

    @allure.id("2654")
    @allure.title("DELETE /api/display/draw")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_display_clear(self, assets_api: AssetsAPI):
        """Test DELETE /api/display/draw endpoint"""
        assets_api.clear_display()

    @allure.id("2655")
    @allure.title("POST /api/audio/play")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_audio_play(self, assets_api: AssetsAPI):
        """Test POST /api/audio/play endpoint"""
        test_app_id = "test_audio_play"
        test_audio_file = "ping.snd"
        audio_path = ASSETS_DIR / test_audio_file

        assert audio_path.exists(), f"Test audio file not found: {audio_path}"

        with allure.step(f"Upload test audio: {test_audio_file}"):
            with open(audio_path, "rb") as f:
                audio_content = f.read()

            assets_api.upload_asset(
                test_app_id, test_audio_file, audio_content
            )

        try:
            assets_api.play_audio(test_app_id, test_audio_file)
        finally:
            assets_api.delete_assets(test_app_id)

    @allure.id("2656")
    @allure.title("DELETE /api/audio/play")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_audio_stop(self, assets_api: AssetsAPI):
        """Test DELETE /api/audio/play endpoint"""
        assets_api.stop_audio()

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

        assert response.status_code == 400
