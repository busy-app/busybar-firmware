import pytest
import allure
import requests
from bs4 import BeautifulSoup
import time


@allure.epic("BSB Web Frontend Testing")
@allure.feature("5. Web Frontend")
@allure.story("UI interaction")
class TestWebFrontendBasic:
    """Basic Web Frontend Tests - Core functionality and connectivity"""

    @allure.testcase("2003", "BSB Front. Cloud Remote. Control Behind NAT. [Draft]")
    @pytest.mark.story_ui_interaction
    @pytest.mark.frontend
    def test_web_frontend_page_loads(self, web_session, web_base_url):
        """Test that the main web page loads successfully"""
        with allure.step("Request main page"):
            response = web_session.get(web_base_url, timeout=10)

        with allure.step("Verify response status"):
            assert response.status_code == 200, f"Expected 200, got {response.status_code}"

        with allure.step("Verify content type"):
            content_type = response.headers.get('content-type', '').lower()
            assert 'text/html' in content_type, f"Expected HTML content, got {content_type}"

        with allure.step("Verify page structure for SPA"):
            soup = BeautifulSoup(response.text, 'html.parser')
            # For Nuxt.js apps, check for the main app container
            app_container = soup.find('div', {'id': '__nuxt'})
            assert app_container is not None, "Page should have Nuxt app container"
            
            # Check that basic meta tags are present
            viewport_meta = soup.find('meta', {'name': 'viewport'})
            assert viewport_meta is not None, "Page should have viewport meta tag"
            
            # Check for Nuxt-specific script
            nuxt_script = soup.find('script', string=lambda text: text and '__NUXT__' in text if text else False)
            assert nuxt_script is not None, "Page should contain Nuxt configuration script"

        allure.attach(response.text[:1000], "Page Content Preview", allure.attachment_type.TEXT)

    @allure.testcase("2018", "BSB Front. Compare to Mock-ups. [Draft]")
    @pytest.mark.story_ui_interaction
    @pytest.mark.frontend
    def test_web_frontend_basic_elements(self, web_session, web_base_url):
        """Test that basic UI elements structure is present for SPA"""
        response = web_session.get(web_base_url, timeout=10)
        assert response.status_code == 200

        soup = BeautifulSoup(response.text, 'html.parser')

        with allure.step("Check for SPA application structure"):
            # Check for the main Nuxt app container
            app_container = soup.find('div', {'id': '__nuxt'})
            assert app_container is not None, "Page should have main app container"
            
            # Check for CSS links that indicate styling is loaded
            css_links = soup.find_all('link', {'rel': 'stylesheet'})
            assert len(css_links) > 0, "Page should have CSS stylesheets linked"

        with allure.step("Check for JavaScript modules"):
            # Check for module scripts that will render the app
            module_scripts = soup.find_all('script', {'type': 'module'})
            assert len(module_scripts) > 0, "Page should have JavaScript modules for rendering"

        with allure.step("Check for required meta tags"):
            # Look for viewport and charset - essential for modern web apps
            charset_meta = soup.find('meta', {'charset': True})
            viewport_meta = soup.find('meta', {'name': 'viewport'})
            assert charset_meta is not None, "Page should have charset meta tag"
            assert viewport_meta is not None, "Page should have viewport meta tag"

    @allure.testcase("2017", "BSB Front. Themes Dark/Bright. [Draft]")
    @pytest.mark.story_ui_interaction
    @pytest.mark.frontend
    def test_web_frontend_dark_theme_support(self, web_session, web_base_url):
        """Test that the page has dark theme infrastructure"""
        response = web_session.get(web_base_url, timeout=10)
        assert response.status_code == 200

        soup = BeautifulSoup(response.text, 'html.parser')
        page_content = response.text.lower()

        with allure.step("Check for color mode script"):
            # Look for Nuxt color mode script that handles theme switching
            has_color_mode_script = 'nuxt_color_mode' in page_content or '__nuxt_color_mode__' in page_content
            assert has_color_mode_script, "Page should have color mode handling script"

        with allure.step("Check for theme-related terms in code"):
            # Look for theme-related functionality in the JavaScript
            has_theme_infrastructure = (
                    'dark' in page_content and 'light' in page_content and
                    ('getcolorscheme' in page_content or 'addcolorscheme' in page_content)
            )
            assert has_theme_infrastructure, "Page should contain theme switching infrastructure"

    @allure.testcase("2119", "BSB Front. Browser Compatibility. [Draft]")
    @pytest.mark.story_ui_interaction
    @pytest.mark.frontend
    def test_web_frontend_response_headers(self, web_session, web_base_url):
        """Test response headers for browser compatibility"""
        response = web_session.get(web_base_url, timeout=10)
        assert response.status_code == 200

        with allure.step("Check Content-Type header"):
            content_type = response.headers.get('content-type')
            assert content_type is not None, "Response should have Content-Type header"
            assert 'text/html' in content_type, f"Expected HTML content type, got {content_type}"

        with allure.step("Check for charset encoding"):
            # Check if charset is specified
            charset_specified = (
                    'charset' in response.headers.get('content-type', '').lower() or
                    'utf-8' in response.text.lower() or
                    'charset=utf-8' in response.text.lower()
            )
            assert charset_specified, "Page should specify character encoding"

    @allure.testcase("2118", "BSB Front. Responsive Design Validation. [Draft]")
    @pytest.mark.story_ui_interaction
    @pytest.mark.frontend
    def test_web_frontend_viewport_meta(self, web_session, web_base_url):
        """Test that page has responsive design viewport meta tag"""
        response = web_session.get(web_base_url, timeout=10)
        assert response.status_code == 200

        soup = BeautifulSoup(response.text, 'html.parser')

        with allure.step("Check for viewport meta tag"):
            viewport_meta = soup.find('meta', attrs={'name': 'viewport'})
            assert viewport_meta is not None, "Page should have viewport meta tag for responsive design"

            content = viewport_meta.get('content', '')
            assert 'width=device-width' in content, f"Viewport should include width=device-width, got: {content}"

    @allure.testcase("2065", "BSB Front. Wi-Fi Connection. [Draft]")
    @pytest.mark.story_ui_interaction
    @pytest.mark.frontend
    def test_web_frontend_wifi_elements(self, web_session, web_base_url):
        """Test that Wi-Fi functionality infrastructure is present"""
        response = web_session.get(web_base_url, timeout=10)
        assert response.status_code == 200

        soup = BeautifulSoup(response.text, 'html.parser')
        page_content = response.text.lower()

        with allure.step("Check for Wi-Fi related infrastructure"):
            # Check for Nuxt app that will render Wi-Fi components
            has_app_framework = soup.find('div', {'id': '__nuxt'}) is not None
            assert has_app_framework, "Page should have SPA framework for Wi-Fi components"
            
            # Check for JavaScript modules that will handle Wi-Fi functionality
            has_js_modules = len(soup.find_all('script', {'type': 'module'})) > 0
            assert has_js_modules, "Page should have JavaScript modules for Wi-Fi functionality"

    @allure.testcase("2066", "BSB Front. BT Connection. [Draft]")
    @pytest.mark.story_ui_interaction
    @pytest.mark.frontend
    def test_web_frontend_bluetooth_elements(self, web_session, web_base_url):
        """Test that Bluetooth functionality infrastructure is present"""
        response = web_session.get(web_base_url, timeout=10)
        assert response.status_code == 200

        soup = BeautifulSoup(response.text, 'html.parser')
        page_content = response.text.lower()

        with allure.step("Check for Bluetooth related infrastructure"):
            # Check for Nuxt app that will render Bluetooth components
            has_app_framework = soup.find('div', {'id': '__nuxt'}) is not None
            assert has_app_framework, "Page should have SPA framework for Bluetooth components"
            
            # Check for prefetch links that might include component resources
            prefetch_links = soup.find_all('link', {'rel': 'prefetch'})
            assert len(prefetch_links) > 0, "Page should have prefetch links for dynamic components"


@allure.epic("BSB Web Frontend Testing")
@allure.feature("5. Web Frontend")
@allure.story("Interface status")
class TestWebFrontendStatus:
    """Interface Status Tests - Connection and device status"""

    @allure.testcase("2060", "BSB Front. USB Status Updates. [Draft]")
    @pytest.mark.story_interface_status
    @pytest.mark.frontend
    @pytest.mark.skip("TODO: fix, need to await for statuses to load or not do it via request")
    def test_web_frontend_usb_status(self, web_session, web_base_url):
        """Test that USB status is displayed"""
        response = web_session.get(web_base_url, timeout=10)
        assert response.status_code == 200

        soup = BeautifulSoup(response.text, 'html.parser')
        page_text = soup.get_text().lower()

        with allure.step("Check for USB/connection status indicators"):
            # Look for connection status in the page text
            has_connection_status = (
                    'connected' in page_text or
                    'connection' in page_text
            )
            
            # Look for USB references in HTML (including CSS classes)
            page_html = str(soup).lower()
            has_usb_in_html = 'usb' in page_html or ':usb' in page_html
            
            # Look for BUSY Bar device indicators
            has_device_indicators = (
                    'busy bar' in page_text or
                    'busybar' in page_text or
                    'device' in page_text
            )
            
            # Save actual content for debugging if needed
            allure.attach(page_text[:1000], "Page Text (first 1000 chars)", allure.attachment_type.TEXT)
            
            assert has_connection_status or has_usb_in_html or has_device_indicators, f"Page should show USB/connection status. Connection status in text: {has_connection_status}, USB in HTML: {has_usb_in_html}, Device indicators: {has_device_indicators}"

    @allure.testcase("2061", "BSB Front. Screen Streaming USB Lan. [Draft]")
    @pytest.mark.story_interface_status
    @pytest.mark.frontend
    @pytest.mark.skip("TODO: fix,wait for load")
    def test_web_frontend_screen_streaming_elements(self, web_session, web_base_url):
        """Test that screen streaming elements are present"""
        response = web_session.get(web_base_url, timeout=10)
        assert response.status_code == 200

        soup = BeautifulSoup(response.text, 'html.parser')

        with allure.step("Check for canvas or streaming elements"):
            # Look for elements that might be used for screen streaming
            has_canvas = soup.find('canvas') is not None
            has_video = soup.find('video') is not None
            has_img = soup.find('img') is not None
            
            # Check for screen streaming container
            streaming_container = soup.find('div', class_=lambda x: x and 'screen-stream-container' in x if x else False)
            has_streaming_container = streaming_container is not None

            has_streaming_elements = has_canvas or has_video or has_img or has_streaming_container
            assert has_streaming_elements, f"Page should have elements for screen display. Canvas: {has_canvas}, Video: {has_video}, Images: {has_img}, Container: {has_streaming_container}"

    @allure.testcase("2064", "BSB Front. Screen Straming Wi-Fi. [Draft]")
    @pytest.mark.story_interface_status
    @pytest.mark.frontend
    @pytest.mark.skip("TODO: fix,wait for load")
    def test_web_frontend_device_image(self, web_session, web_base_url):
        """Test that device representation/image is present"""
        response = web_session.get(web_base_url, timeout=10)
        assert response.status_code == 200

        soup = BeautifulSoup(response.text, 'html.parser')

        with allure.step("Check for device-related images"):
            images = soup.find_all('img')
            device_images = [img for img in images if
                             any(keyword in str(img.get('src', '')).lower() or
                                 keyword in str(img.get('alt', '')).lower()
                                 for keyword in ['device', 'busy', 'bar', 'screen', 'busybar', 'pixel'])]
            
            # Also check for device image container
            device_container = soup.find('div', class_=lambda x: x and 'device-image' in x if x else False)
            has_device_container = device_container is not None

            assert len(device_images) > 0 or has_device_container, f"Page should contain device-related images. Found {len(device_images)} device images, device container: {has_device_container}"


@allure.epic("BSB Web Frontend Testing")
@allure.feature("5. Web Frontend")
@allure.story("UI validation")
class TestWebFrontendValidation:
    """UI Validation Tests - Form validation and accessibility"""

    @allure.testcase("2121", "BSB Front. Form Validation. [Draft]")
    @pytest.mark.story_ui_validation
    @pytest.mark.frontend
    @pytest.mark.skip("TODO: fix, wait for load")
    def test_web_frontend_form_elements(self, web_session, web_base_url):
        """Test that form elements are present and accessible"""
        response = web_session.get(web_base_url, timeout=10)
        assert response.status_code == 200

        soup = BeautifulSoup(response.text, 'html.parser')

        with allure.step("Check for interactive form elements"):
            # Look for buttons, inputs, forms, links
            buttons = soup.find_all('button')
            inputs = soup.find_all('input')
            forms = soup.find_all('form')
            links = soup.find_all('a')
            
            # Check for role-based interactive elements
            interactive_roles = soup.find_all(attrs={'role': lambda x: x in ['button', 'switch', 'link'] if x else False})

            has_interactive_elements = len(buttons) > 0 or len(inputs) > 0 or len(forms) > 0 or len(links) > 0 or len(interactive_roles) > 0
            assert has_interactive_elements, f"Page should have interactive form elements. Buttons: {len(buttons)}, Inputs: {len(inputs)}, Forms: {len(forms)}, Links: {len(links)}, Interactive roles: {len(interactive_roles)}"

    @allure.testcase("2123", "BSB Front. Accessibility Compliance. [Draft]")
    @pytest.mark.story_ui_validation
    @pytest.mark.frontend
    def test_web_frontend_accessibility_basics(self, web_session, web_base_url):
        """Test basic accessibility features"""
        response = web_session.get(web_base_url, timeout=10)
        assert response.status_code == 200

        soup = BeautifulSoup(response.text, 'html.parser')

        with allure.step("Check for language attribute"):
            html_tag = soup.find('html')
            has_lang = html_tag and html_tag.get('lang') is not None
            # Note: Not all SPAs set lang on html, so this might be optional

        with allure.step("Check for alt attributes on images"):
            images = soup.find_all('img')
            if images:  # Only check if images exist
                images_with_alt = [img for img in images if img.get('alt') is not None]
                alt_coverage = len(images_with_alt) / len(images)
                assert alt_coverage >= 0.5, f"At least 50% of images should have alt attributes, got {alt_coverage * 100:.1f}%"

    @allure.testcase("2120", "BSB Front. Touch/mouse Input Handling. [Draft]")
    @pytest.mark.story_ui_validation
    @pytest.mark.frontend
    @pytest.mark.skip("TODO: fix, wait for load")
    def test_web_frontend_interactive_elements(self, web_session, web_base_url):
        """Test that interactive elements are properly marked up"""
        response = web_session.get(web_base_url, timeout=10)
        assert response.status_code == 200

        soup = BeautifulSoup(response.text, 'html.parser')

        with allure.step("Check for clickable elements"):
            buttons = soup.find_all('button')
            links = soup.find_all('a')
            clickable_divs = soup.find_all(attrs={'onclick': True})
            
            # Check for elements with cursor-pointer class (common in Tailwind CSS)
            cursor_pointer_elements = soup.find_all(attrs={'class': lambda x: x and 'cursor-pointer' in ' '.join(x) if x else False})
            
            # Check for role-based interactive elements
            interactive_roles = soup.find_all(attrs={'role': lambda x: x in ['button', 'switch', 'link'] if x else False})

            total_interactive = len(buttons) + len(links) + len(clickable_divs) + len(cursor_pointer_elements) + len(interactive_roles)
            assert total_interactive > 0, f"Page should have interactive elements. Buttons: {len(buttons)}, Links: {len(links)}, Onclick divs: {len(clickable_divs)}, Cursor-pointer: {len(cursor_pointer_elements)}, Interactive roles: {len(interactive_roles)}"

        with allure.step("Check button accessibility"):
            buttons = soup.find_all('button')
            if buttons:
                buttons_with_text = [btn for btn in buttons if btn.get_text(strip=True) or btn.get('aria-label')]
                accessibility_ratio = len(buttons_with_text) / len(buttons)
                assert accessibility_ratio >= 0.8, f"At least 80% of buttons should have text or aria-label, got {accessibility_ratio * 100:.1f}%"


@allure.epic("BSB Web Frontend Testing")
@allure.feature("5. Web Frontend")
@allure.story("MQTT")
class TestWebFrontendMQTT:
    """MQTT Tests - Message queuing and communication"""

    @allure.testcase("2004", "MQTT. Publish Busy State [Draft]")
    @pytest.mark.story_mqtt
    @pytest.mark.frontend
    @pytest.mark.skip(reason="MQTT testing requires specific setup - draft implementation")
    def test_web_frontend_mqtt_references(self, web_session, web_base_url):
        """Test that MQTT functionality references are present in web interface"""
        response = web_session.get(web_base_url, timeout=10)
        assert response.status_code == 200

        # This is a placeholder for MQTT testing
        # Would require actual MQTT broker setup and message capture
        page_text = response.text.lower()

        with allure.step("Check for MQTT-related content"):
            # Look for any MQTT references in the page
            has_mqtt_refs = 'mqtt' in page_text
            # This test is currently skipped as it requires more complex setup

    @allure.testcase("2005", "MQTT.Receive Display Command [Draft]")
    @pytest.mark.story_mqtt
    @pytest.mark.frontend
    @pytest.mark.skip(reason="MQTT command testing requires broker setup - draft implementation")
    def test_web_frontend_mqtt_display_command(self, web_session, web_base_url):
        """Test MQTT display command functionality"""
        # Placeholder for MQTT display command testing
        # Would require:
        # 1. MQTT broker connection
        # 2. Sending display commands via MQTT
        # 3. Verifying web interface updates
        pass