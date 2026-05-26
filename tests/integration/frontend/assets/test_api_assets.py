from pathlib import Path
from time import sleep

import allure
import pytest
import requests

from clients.api import AssetsAPI, SettingsAPI, StorageAPI

ASSETS_DIR = Path(__file__).resolve().parents[2] / "assets"
REQUIRED_SHARED_FONTS = [
    "busy_bold_10.font",
    "busy_bold_7.font",
    "busy_condensed_7.font",
    "busy_regular_14.font",
    "busy_regular_5.font",
    "busy_regular_7.font",
    "busy_regular_9.font",
    "busy_superscript_7.font",
    "busy_tiny.font",
]


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
                "font": "normal",
                "color": "#FFFFFFFF",
                "display": "front",
            }
        ]

        assets_api.draw("test_app", elements)
        assets_api.clear_display()

    @allure.id("2690")
    @allure.title("POST /api/display/draw (image)")
    @pytest.mark.api
    @pytest.mark.frontend
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
            assets_api.clear_display()
        finally:
            try:
                assets_api.delete_assets(test_app_id)
            except requests.exceptions.RequestException as exc:
                allure.attach(
                    f"Asset cleanup failed: {exc}",
                    name="Asset Cleanup Error",
                    attachment_type=allure.attachment_type.TEXT,
                )

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
    def test_api_audio_play(self, assets_api: AssetsAPI, settings_api: SettingsAPI):
        """Test POST /api/audio/play endpoint"""
        test_app_id = "test_audio_play1"
        test_audio_file = "smb_powerup.snd"
        audio_path = ASSETS_DIR / test_audio_file
        # Set volume to a low level for testing to avoid loud audio during test runs
        settings_api.set_volume(10)

        assert audio_path.exists(), f"Test audio file not found: {audio_path}"

        with allure.step(f"Upload test audio: {test_audio_file}"):
            with open(audio_path, "rb") as f:
                audio_content = f.read()

            assets_api.upload_asset(
                test_app_id, test_audio_file, audio_content
            )

        try:
            sleep(0.5)
            assets_api.play_audio(test_app_id, test_audio_file)
            sleep(2)
            # 200 if audio is still playing; 410 if the file finished before stop was called
            assets_api.delete_raw("/api/audio/play")
        finally:
            try:
                assets_api.delete_assets(test_app_id)
            except requests.exceptions.RequestException as exc:
                allure.attach(
                    f"Asset cleanup failed: {exc}",
                    name="Asset Cleanup Error",
                    attachment_type=allure.attachment_type.TEXT,
                )

    @allure.id("2656")
    @allure.title("DELETE /api/audio/play")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_audio_stop(self, assets_api: AssetsAPI):
        """Test DELETE /api/audio/play endpoint returns 410 when no audio is playing."""
        response = assets_api.delete_raw("/api/audio/play")
        assert response.status_code == 410

    @allure.id("2657")
    @allure.title("DELETE /api/audio/play while audio is playing")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_audio_stop_while_playing(
        self, assets_api: AssetsAPI, settings_api: SettingsAPI
    ):
        """
        Test that DELETE /api/audio/play returns 200 when audio is playing.

        The connection is held by the server until AudioEventPlayEnd fires (async),
        so this also verifies the async stop mechanism end-to-end.
        Stop is issued during the 100ms holdoff period, exercising the holdoff
        cancellation path added to audio.c.
        """
        test_app_id = "test_audio_stop_playing"
        test_audio_file = "ping.snd"
        audio_path = ASSETS_DIR / test_audio_file

        settings_api.set_volume(10)
        assert audio_path.exists(), f"Test audio file not found: {audio_path}"

        with allure.step(f"Upload {test_audio_file}"):
            with open(audio_path, "rb") as f:
                assets_api.upload_asset(test_app_id, test_audio_file, f.read())

        try:
            with allure.step("Play audio"):
                assets_api.play_audio(test_app_id, test_audio_file)

            with allure.step("Stop audio while playing and assert 200"):
                # Called within the 100ms holdoff window; the server holds the
                # connection until PlayEnd fires, then responds 200.
                assets_api.stop_audio()
        finally:
            try:
                assets_api.delete_assets(test_app_id)
            except requests.exceptions.RequestException as exc:
                allure.attach(
                    f"Asset cleanup failed: {exc}",
                    name="Asset Cleanup Error",
                    attachment_type=allure.attachment_type.TEXT,
                )

    @allure.id("2658")
    @allure.title("POST /api/audio/play (stock_path)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_audio_play_stock_path(
        self, assets_api: AssetsAPI, settings_api: SettingsAPI
    ):
        """Test POST /api/audio/play with stock_path plays a built-in sound."""
        settings_api.set_volume(10)

        with allure.step("Play stock sound shared/volume_change.snd"):
            response = assets_api.post_raw(
                "/api/audio/play",
                json={
                    "application_name": "test_audio_stock",
                    "stock_path": "shared/volume_change.snd",
                },
            )
        assert response.status_code == 200

        # cleanup — audio may already be done; either 200 or 410 is acceptable
        assets_api.delete_raw("/api/audio/play")
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


@allure.feature("5. Web Frontend")
@allure.story("Assets")
@pytest.mark.api
@pytest.mark.frontend
@pytest.mark.regression
class TestSharedAssetRegressions:
    @allure.title("External shared font assets are deployed")
    @pytest.mark.parametrize("font_name", REQUIRED_SHARED_FONTS)
    def test_shared_font_asset_exists(self, storage_api: StorageAPI, font_name: str):
        response = storage_api.read(f"/ext/apps_assets/shared/fonts/{font_name}")

        assert response.status_code == 200
        assert len(response.content) > 0

    @allure.title("External shared font assets have plausible binary content")
    @pytest.mark.parametrize("font_name", REQUIRED_SHARED_FONTS)
    def test_shared_font_asset_has_plausible_content(
        self, storage_api: StorageAPI, font_name: str
    ):
        response = storage_api.read(f"/ext/apps_assets/shared/fonts/{font_name}")

        assert response.status_code == 200
        assert 8 <= len(response.content) < 65536
        assert len(set(response.content[:64])) > 1

    @allure.title("Text draw smoke covers deployed font renderer path")
    def test_text_draw_font_renderer_smoke(self, assets_api: AssetsAPI, streaming_api):
        elements = [
            {
                "id": "font_smoke",
                "timeout": 5,
                "type": "text",
                "text": "FONT",
                "x": 36,
                "y": 8,
                "align": "center",
                "font": "small",
                "color": "#FFFFFFFF",
                "display": "front",
            }
        ]

        try:
            assets_api.draw("font_renderer_regression", elements, priority=90)
            sleep(1)
            frame = streaming_api.get_screen_bytes(display=0)
            assert any(frame)
        finally:
            assets_api.clear_display_by_app("font_renderer_regression")
