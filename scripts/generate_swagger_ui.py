#!/usr/bin/env python3
"""
Script to generate Swagger UI static files for BSB firmware web server.

This script downloads the latest Swagger UI distribution and creates
a customized HTML page that serves the API schema from the embedded device.
"""

import shutil
import tempfile
import urllib.request
import zipfile
from pathlib import Path

from flipper.app import App


class Main(App):
    def init(self):
        """Initialize the argument parser."""
        self.parser.add_argument(
            "--version",
            help="Swagger UI version to download",
            default="5.10.5",
            required=False,
        )
        self.parser.add_argument(
            "--target-dir",
            help="Target directory for generated files",
            default=None,
            required=False,
        )
        self.parser.add_argument(
            "--api-spec",
            help="Path to OpenAPI specification file relative to web root",
            default="openapi.yaml",
            required=False,
        )
        self.parser.add_argument(
            "--clean",
            help="Clean target directory before generation",
            action="store_true",
            default=True,
        )

        self.parser.set_defaults(func=self.generate)

    def generate(self):
        """Main function to generate Swagger UI files."""
        self.logger.info("BSB Firmware Swagger UI Generator")
        self.logger.info("=" * 40)

        # Determine target directory
        if self.args.target_dir:
            target_dir = Path(self.args.target_dir)
        else:
            target_dir = (
                Path(__file__).parent.parent
                / "applications/services/web_server/resources/apps_assets/web_server/www/docs"
            )

        # Configuration
        swagger_ui_version = self.args.version
        swagger_ui_url = f"https://github.com/swagger-api/swagger-ui/archive/refs/tags/v{swagger_ui_version}.zip"
        api_yaml_path = self.args.api_spec

        # Clean and create target directory
        if self.args.clean and target_dir.exists():
            self.logger.info(f"Cleaning existing docs directory: {target_dir}")
            shutil.rmtree(target_dir)

        target_dir.mkdir(parents=True, exist_ok=True)
        self.logger.info(f"Created docs directory: {target_dir}")

        # Check if the specified API specification file exists
        www_root = target_dir.parent

        # Construct the full path to the API spec file based on the provided argument
        api_spec_full_path = www_root / api_yaml_path

        if not api_spec_full_path.exists():
            self.logger.error(
                f"OpenAPI specification file not found: {api_spec_full_path}"
            )
            self.logger.error(
                f"Please ensure '{api_yaml_path}' exists in the www directory."
            )

            # If the user specified a custom path, also suggest the default
            if api_yaml_path != "openapi.yaml":
                default_path = www_root / "openapi.yaml"
                if default_path.exists():
                    self.logger.info(
                        f"Note: Found default 'openapi.yaml' at {default_path}"
                    )
                    self.logger.info(
                        "Consider using the default or copying your spec file to the expected location."
                    )

            return 1

        self.logger.info(f"Using OpenAPI specification: {api_spec_full_path}")

        # Validate that the API spec file is readable and appears to be valid YAML
        try:
            with open(api_spec_full_path, "r", encoding="utf-8") as f:
                content = f.read(100)  # Read first 100 chars to check if it's readable
                if not content.strip():
                    self.logger.warning(
                        f"API specification file appears to be empty: {api_spec_full_path}"
                    )
        except (IOError, UnicodeDecodeError) as e:
            self.logger.error(f"Failed to read API specification file: {e}")
            return 1

        try:
            with tempfile.TemporaryDirectory() as temp_dir:
                temp_path = Path(temp_dir)

                # Download and extract Swagger UI
                swagger_dist_dir = self._download_swagger_ui(
                    temp_path, swagger_ui_url, swagger_ui_version
                )

                # Copy Swagger UI assets
                self._copy_swagger_assets(swagger_dist_dir, target_dir)

                # Create the main Swagger UI HTML file
                self._create_swagger_html(target_dir, api_yaml_path)

                # Create API index redirect
                self._create_api_index_redirect(target_dir)

            self.logger.info("=" * 40)
            self.logger.info("✅ Swagger UI generation completed successfully!")

            return 0

        except Exception as e:
            self.logger.error(f"Error: {e}")
            return 1

    def _download_swagger_ui(
        self, temp_dir: Path, swagger_ui_url: str, swagger_ui_version: str
    ) -> Path:
        """Download and extract Swagger UI."""
        self.logger.info(f"Downloading Swagger UI v{swagger_ui_version}...")

        zip_path = temp_dir / "swagger-ui.zip"

        # Download the zip file
        urllib.request.urlretrieve(swagger_ui_url, zip_path)

        # Extract the zip file
        with zipfile.ZipFile(zip_path, "r") as zip_ref:
            zip_ref.extractall(temp_dir)

        # Find the extracted directory
        extracted_dir = temp_dir / f"swagger-ui-{swagger_ui_version}"
        if not extracted_dir.exists():
            raise FileNotFoundError(f"Extracted directory not found: {extracted_dir}")

        return extracted_dir / "dist"

    def _copy_swagger_assets(self, swagger_dist_dir: Path, target_dir: Path) -> None:
        """Copy necessary Swagger UI assets."""

        # List of files we need from Swagger UI
        required_files = [
            "swagger-ui-bundle.js",
            "swagger-ui.css",
            "swagger-ui-standalone-preset.js",
            "favicon-16x16.png",
            "favicon-32x32.png",
        ]

        self.logger.info("Copying Swagger UI assets...")

        for filename in required_files:
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
            const apiSpecUrl = baseUrl + '/{api_yaml_path}';
            
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
