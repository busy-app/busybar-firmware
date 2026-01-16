#!/usr/bin/env python3
"""
Script to generate Swagger UI static files for BSB firmware web server.

This script downloads the latest Swagger UI distribution and creates
a customized HTML page that serves the API schema from the embedded device.
"""

import logging
import shutil
import tempfile
import urllib.error
import urllib.request
import zipfile
import ssl
import sys
from pathlib import Path

from flipper.app import App


class Main(App):
    SWAGGER_UI_URL_BASE = (
        "https://github.com/swagger-api/swagger-ui/archive/refs/tags/v{version}.zip"
    )
    REQUIRED_STATIC_FILES = [
        "swagger-ui-bundle.js",
        "swagger-ui.css",
        "swagger-ui-standalone-preset.js",
        "favicon-16x16.png",
        "favicon-32x32.png",
    ]

    def init(self):  # type: ignore[override]
        self.parser.add_argument(
            "api_spec",
            help="Full path to OpenAPI specification file",
            type=str,
        )
        self.parser.add_argument(
            "--version",
            help="Swagger UI version to download",
            default="5.30.1",
            required=False,
        )
        self.parser.add_argument(
            "--dist-dir",
            help="Directory containing cached Swagger UI distributions",
            default="swagger-dist",
            required=False,
        )
        self.parser.add_argument(
            "-o",
            "--target-dir",
            help="Target directory for generated files",
            default=None,
            required=False,
        )
        self.parser.add_argument(
            "--clean",
            help="Clean target directory before generation",
            action="store_true",
            default=True,
        )
        self.parser.add_argument(
            "--download-only",
            help="Download the requested Swagger UI version to the cache directory and exit",
            action="store_true",
            default=False,
        )
        self.parser.add_argument(
            "-q",
            "--quiet",
            help="Suppress output messages",
            action="store_true",
            default=False,
        )

        self.parser.set_defaults(func=self.generate)

    def generate(self):
        if self.args.quiet:
            self.logger.setLevel(logging.WARNING)

        dist_cache_dir = Path(self.args.dist_dir).expanduser()
        dist_cache_dir.mkdir(parents=True, exist_ok=True)

        # Determine target directory
        if self.args.target_dir:
            target_dir = Path(self.args.target_dir)
        else:
            target_dir = None

        # Configuration
        swagger_ui_version = self.args.version
        swagger_ui_url = self.SWAGGER_UI_URL_BASE.format(version=swagger_ui_version)

        try:
            swagger_zip_path = self._get_or_download_swagger_zip(
                dist_cache_dir, swagger_ui_url, swagger_ui_version, allow_download=True
            )
        except Exception as e:
            self.logger.error(f"Failed to prepare Swagger UI distribution: {e}")
            return 1

        if self.args.download_only:
            self.logger.info(
                f"Swagger UI v{swagger_ui_version} is available at: {swagger_zip_path}"
            )
            return 0

        # Get API specification file path (now a required positional argument)
        api_spec_path = Path(self.args.api_spec)

        if not api_spec_path.exists():
            self.logger.error(f"OpenAPI specification file not found: {api_spec_path}")
            return 1

        self.logger.info(f"Using OpenAPI specification: {api_spec_path}")

        if target_dir is None:
            target_dir = api_spec_path.parent / "docs"
            self.logger.info(f"No target directory specified, using: {target_dir}")

        # Extract just the filename for API spec relative path
        api_yaml_path = api_spec_path.name

        # Clean and create target directory
        if self.args.clean and target_dir.exists():
            self.logger.info(f"Cleaning existing docs directory: {target_dir}")
            shutil.rmtree(target_dir)

        target_dir.mkdir(parents=True, exist_ok=True)
        self.logger.info(f"Created docs directory: {target_dir}")

        try:
            with tempfile.TemporaryDirectory() as temp_dir:
                temp_path = Path(temp_dir)

                # Extract Swagger UI assets from cached archive
                swagger_dist_dir = self._extract_swagger_dist(
                    swagger_zip_path, temp_path, swagger_ui_version
                )

                # Copy Swagger UI assets
                self._copy_swagger_assets(swagger_dist_dir, target_dir)

                # Create the main Swagger UI HTML file
                self._create_swagger_html(target_dir, api_yaml_path)

                # Create API index redirect
                self._create_api_index_redirect(target_dir)

            self.logger.info("✅ Swagger UI generation completed successfully!")

            return 0

        except Exception as e:
            self.logger.error(f"Error: {e}")
            return 1

    def _get_or_download_swagger_zip(
        self,
        cache_dir: Path,
        swagger_ui_url: str,
        swagger_ui_version: str,
        allow_download: bool,
    ) -> Path:
        """Ensure the requested Swagger UI archive is available locally."""

        zip_path = cache_dir / f"swagger-ui-{swagger_ui_version}.zip"

        if zip_path.exists():
            self.logger.info(f"Using cached Swagger UI archive: {zip_path}")
            return zip_path

        if not allow_download:
            raise FileNotFoundError(
                f"Swagger UI v{swagger_ui_version} not found in cache and downloads disabled"
            )

        self.logger.info(f"Downloading Swagger UI v{swagger_ui_version}...")
        self._download_swagger_ui_archive(swagger_ui_url, zip_path)
        return zip_path

    def _download_swagger_ui_archive(self, swagger_ui_url: str, zip_path: Path) -> None:
        """Download Swagger UI archive to the specified path."""

        try:
            urllib.request.urlretrieve(swagger_ui_url, zip_path)
        except urllib.error.URLError as e:
            if "CERTIFICATE_VERIFY_FAILED" in str(e):
                self.logger.warning(
                    "SSL certificate verification failed. Trying with verification disabled..."
                )
                context = ssl.create_default_context()
                context.check_hostname = False
                context.verify_mode = ssl.CERT_NONE

                with urllib.request.urlopen(
                    swagger_ui_url, context=context
                ) as response, zip_path.open("wb") as out_file:
                    shutil.copyfileobj(response, out_file)
            else:
                raise

    def _extract_swagger_dist(
        self, zip_path: Path, temp_dir: Path, swagger_ui_version: str
    ) -> Path:
        """Extract Swagger UI distribution to a temporary directory."""

        with zipfile.ZipFile(zip_path, "r") as zip_ref:
            zip_ref.extractall(temp_dir)

        extracted_dir = temp_dir / f"swagger-ui-{swagger_ui_version}"
        if not extracted_dir.exists():
            raise FileNotFoundError(f"Extracted directory not found: {extracted_dir}")

        dist_dir = extracted_dir / "dist"
        if not dist_dir.exists():
            raise FileNotFoundError(f"Swagger UI dist directory not found: {dist_dir}")

        return dist_dir

    def _copy_swagger_assets(self, swagger_dist_dir: Path, target_dir: Path) -> None:
        """Copy necessary Swagger UI assets."""

        self.logger.info("Copying Swagger UI assets...")

        for filename in self.REQUIRED_STATIC_FILES:
            src_file = swagger_dist_dir / filename
            dst_file = target_dir / filename

            if src_file.exists():
                shutil.copy2(src_file, dst_file)
                self.logger.info(f"  Copied {filename}")
            else:
                self.logger.warning(
                    f"  Warning: {filename} not found in Swagger UI distribution"
                )

    def _create_swagger_html(self, target_dir: Path, api_yaml_path: str) -> None:
        """Create a customized Swagger UI HTML file."""

        # for now, we assume the API spec location
        api_url_path = api_yaml_path

        html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>BSB Firmware API Documentation</title>
    <link rel="stylesheet" type="text/css" href="swagger-ui.css">
    <style>
        html {{
            box-sizing: border-box;
            overflow: -moz-scrollbars-vertical;
            overflow-y: scroll;
        }}

        *, *:before, *:after {{
            box-sizing: inherit;
        }}

        body {{
            margin: 0;
            background: #fafafa;
        }}

        .swagger-ui .topbar {{
            background-color: #2c3e50;
        }}

        .swagger-ui .topbar .link {{
            color: #ffffff;
        }}

        .swagger-ui .info hgroup.main .title {{
            color: #2c3e50;
        }}

        .swagger-ui .scheme-container {{
            background: #ffffff;
            box-shadow: 0 1px 2px 0 rgba(0,0,0,.15);
            border-radius: 4px;
            margin: 20px 0;
            padding: 10px;
        }}
    </style>
</head>

<body>
    <div id="swagger-ui"></div>

    <script src="swagger-ui-bundle.js"></script>
    <script src="swagger-ui-standalone-preset.js"></script>
    <script>
        window.onload = function() {{
            // Determine the base URL dynamically
            const baseUrl = window.location.protocol + '//' + window.location.host;
            
            // Build the API spec URL
            const apiSpecUrl = baseUrl + '/{api_url_path}';
            
            // Initialize Swagger UI
            const ui = SwaggerUIBundle({{
                url: apiSpecUrl,
                dom_id: '#swagger-ui',
                deepLinking: true,
                presets: [
                    SwaggerUIBundle.presets.apis,
                    SwaggerUIStandalonePreset
                ],
                plugins: [
                    SwaggerUIBundle.plugins.DownloadUrl
                ],
                layout: "StandaloneLayout",
                defaultModelsExpandDepth: 2,
                defaultModelExpandDepth: 2,
                docExpansion: "list",
                supportedSubmitMethods: ['get', 'post', 'put', 'delete', 'patch'],
                onComplete: function() {{
                    console.log('Swagger UI initialized successfully');
                }},
                onFailure: function(error) {{
                    console.error('Failed to load API spec:', error);
                    document.getElementById('swagger-ui').innerHTML = 
                        '<div style="padding: 20px; color: red; text-align: center;">' +
                        '<h2>Failed to load API specification</h2>' +
                        '<p>Error: ' + error.message + '</p>' +
                        '<p>Make sure the API server is running and accessible.</p>' +
                        '</div>';
                }}
            }});
            
            window.ui = ui;
        }};
    </script>
</body>
</html>"""

        index_html_path = target_dir / "index.html"
        index_html_path.write_text(html_content, encoding="utf-8")
        self.logger.info(f"Created {index_html_path}")

    def _create_api_index_redirect(self, target_dir: Path) -> None:
        """Create a simple redirect page for /api/ to the Swagger UI."""

        api_index_content = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="refresh" content="0; url=../docs/">
    <title>API Documentation</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin-top: 50px;
            color: #333;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
            padding: 20px;
        }
        a {
            color: #2c3e50;
            text-decoration: none;
        }
        a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>API Documentation</h1>
        <p>Redirecting to API documentation...</p>
        <p>If you are not redirected automatically, <a href="../docs/">click here</a>.</p>
    </div>
</body>
</html>"""

        # Create api directory if it doesn't exist
        api_dir = target_dir.parent / "api"
        api_dir.mkdir(exist_ok=True)

        api_index_path = api_dir / "index.html"
        api_index_path.write_text(api_index_content, encoding="utf-8")
        self.logger.info(f"Created {api_index_path}")


if __name__ == "__main__":
    Main()()
