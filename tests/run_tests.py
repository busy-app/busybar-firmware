#!/usr/bin/env python3
"""
Local test runner script for BSB automation
Provides easy way to run tests and upload to Allure TestOps
"""

import argparse
import subprocess
import sys
import os
import logging
from datetime import datetime
from pathlib import Path
from dotenv import load_dotenv

# Setup logging for runner
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[logging.StreamHandler(sys.stdout)]
)

logger = logging.getLogger(__name__)


def run_command(cmd, description="", capture_output=True, stream_output=False):
    """Run a shell command and handle errors"""
    if description:
        logger.info(f"🔄 {description}")

    try:
        if stream_output:
            # Stream output in real-time for test execution
            process = subprocess.Popen(
                cmd,
                shell=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                universal_newlines=True,
                bufsize=1
            )

            output_lines = []
            for line in process.stdout:
                print(line, end='')  # Print to console immediately
                output_lines.append(line)

            process.wait()

            if process.returncode != 0:
                logger.error(f"❌ Command failed with return code {process.returncode}")
                return False

            return True
        else:
            # Regular capture for non-test commands
            result = subprocess.run(cmd, shell=True, check=True, capture_output=capture_output, text=True)
            if result.stdout and capture_output:
                logger.info(result.stdout)
            return True

    except subprocess.CalledProcessError as e:
        logger.error(f"❌ Error: {e}")
        if hasattr(e, 'stderr') and e.stderr:
            logger.error(f"Error details: {e.stderr}")
        return False


def main():
    parser = argparse.ArgumentParser(description="BSB Test Automation Runner")
    parser.add_argument(
        "--suite",
        choices=["all", "cli", "frontend"],
        default="all",
        help="Test suite to run (default: all)"
    )
    parser.add_argument(
        "--upload",
        action="store_true",
        help="Upload results to Allure TestOps"
    )
    parser.add_argument(
        "--report",
        action="store_true",
        help="Generate and serve local Allure report"
    )
    parser.add_argument(
        "--launch-name",
        type=str,
        help="Custom launch name for TestOps"
    )
    parser.add_argument(
        "--markers",
        type=str,
        help="Run tests with specific markers (e.g., 'story_commands_check')"
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Verbose test output"
    )
    parser.add_argument(
        "--log-level",
        choices=["DEBUG", "INFO", "WARNING", "ERROR"],
        default="INFO",
        help="Logging level for tests"
    )
    parser.add_argument(
        "--quiet-runner",
        action="store_true",
        help="Don't show live test output (capture mode)"
    )

    args = parser.parse_args()

    # Load environment
    load_dotenv()

    # Create timestamp for launch name
    timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M")

    # Determine launch name
    if args.launch_name:
        launch_name = args.launch_name
    else:
        launch_name = f"BSB {args.suite.title()} Tests - Local Run {timestamp}"

    # Set environment variables for test run
    os.environ["LOG_LEVEL"] = args.log_level
    os.environ["LOG_TO_FILE"] = "true"

    # Build pytest command
    pytest_cmd = "poetry run pytest"

    # Add test path based on suite
    if args.suite == "cli":
        pytest_cmd += " tests/cli/"
    elif args.suite == "frontend":
        pytest_cmd += " tests/frontend/"
    else:
        pytest_cmd += " tests/"

    # Add markers if specified
    if args.markers:
        pytest_cmd += f" -m {args.markers}"

    # Add verbose flag
    if args.verbose:
        pytest_cmd += " -v"

    # Add progress indicators
    pytest_cmd += " --tb=short"  # Shorter traceback format

    # Add Allure directory
    pytest_cmd += " --alluredir=allure-results"

    # Clean previous results
    logger.info("🧹 Cleaning previous test results...")
    if Path("allure-results").exists():
        import shutil
        try:
            shutil.rmtree("allure-results")
            Path("allure-results").mkdir(exist_ok=True)
            logger.info("✅ Previous results cleaned")
        except Exception as e:
            logger.warning(f"⚠️  Could not clean previous results: {e}")

    # Run tests
    logger.info(f"🚀 Running {args.suite} tests...")
    logger.info(f"Command: {pytest_cmd}")
    logger.info(f"Log Level: {args.log_level}")
    logger.info("=" * 80)

    # Determine if we should stream output
    stream_tests = not args.quiet_runner

    success = run_command(
        pytest_cmd,
        f"Executing {args.suite} test suite",
        capture_output=False,
        stream_output=stream_tests
    )

    logger.info("=" * 80)

    if not success:
        logger.error("❌ Tests failed!")
        if not args.report and not args.upload:
            sys.exit(1)
    else:
        logger.info("✅ Tests completed successfully!")

    # Check if we have test results
    results_path = Path("allure-results")
    if not results_path.exists() or not any(results_path.iterdir()):
        logger.warning("⚠️  No test results found to process")
        sys.exit(1)

    # Show summary of results
    json_files = list(results_path.glob("*-result.json"))
    logger.info(f"📊 Generated {len(json_files)} test result files")

    # Upload to TestOps if requested
    if args.upload:
        logger.info("📤 Uploading results to Allure TestOps...")

        testops_url = os.getenv('ALLURE_TESTOPS_URL', "https://flipper.testops.cloud")
        testops_token = os.getenv('ALLURE_TESTOPS_TOKEN', "29983f72-8d7f-40d6-8232-b778c9374e6e")
        project_id = os.getenv('ALLURE_PROJECT_ID', "232")

        print(testops_url, testops_token, project_id, launch_name)

        # Check if allurectl is installed (cross-platform)
        try:
            allurectl_check = subprocess.run(['allurectl', '--version'], capture_output=True)
            if allurectl_check.returncode != 0:
                logger.error("❌ allurectl is not installed. Please install it first.")
                logger.info("💡 You can download it from: https://github.com/allure-framework/allurectl/releases")
                sys.exit(1)
        except FileNotFoundError:
            logger.error("❌ allurectl is not installed. Please install it first.")
            logger.info("💡 You can download it from: https://github.com/allure-framework/allurectl/releases")
            sys.exit(1)
        
        logger.info("✅ allurectl is available")

        # Upload command (single line for better cross-platform compatibility)
        upload_cmd = f'allurectl upload allure-results --endpoint "{testops_url}" --token "{testops_token}" --project-id "{project_id}" --launch-name "{launch_name}"'

        if run_command(upload_cmd, "Uploading to Allure TestOps", capture_output=True):
            logger.info("✅ Results uploaded to Allure TestOps!")
            logger.info(f"🔗 View results: {testops_url}/project/{project_id}")
        else:
            logger.error("❌ Failed to upload to TestOps")

    # Generate local report if requested
    if args.report:
        logger.info("📊 Generating local Allure report...")

        # Install allure command line if not present
        allure_check = subprocess.run(['which', 'allure'], capture_output=True)
        if allure_check.returncode != 0:
            logger.info("📥 Installing Allure command line...")
            install_cmd = """
            curl -o allure-commandline.tgz -L https://repo.maven.apache.org/maven2/io/qameta/allure/allure-commandline/2.24.0/allure-commandline-2.24.0.tgz
            tar -xzf allure-commandline.tgz
            sudo mv allure-* /opt/allure
            sudo ln -s /opt/allure/bin/allure /usr/local/bin/allure
            rm allure-commandline.tgz
            """
            if not run_command(install_cmd, "Installing Allure CLI"):
                logger.error("❌ Failed to install Allure CLI")
                logger.info("💡 You can install it manually or use: brew install allure (on macOS)")
                sys.exit(1)

        # Generate and serve report
        if run_command("allure generate allure-results -o allure-report --clean", "Generating report"):
            logger.info("✅ Report generated!")
            logger.info("🌐 Starting local report server...")
            logger.info("📁 Report location: allure-report/index.html")

            # Try to serve the report
            try:
                logger.info("🔗 Opening report in browser...")
                logger.info("   Press Ctrl+C to stop the server")
                subprocess.run(["allure", "serve", "allure-results"], check=True)
            except KeyboardInterrupt:
                logger.info("\n👋 Report server stopped")
            except subprocess.CalledProcessError:
                logger.warning("⚠️  Could not start report server, but files are generated in allure-report/")

    # Show logs location
    logs_dir = Path("logs")
    if logs_dir.exists():
        latest_log = logs_dir / "latest.log"
        if latest_log.exists():
            logger.info(f"📋 Test logs available at: {latest_log}")

    logger.info("\n🎉 All operations completed!")
    if success:
        logger.info("✅ Test run successful")
    else:
        logger.warning("⚠️  Some tests may have failed - check the results")


if __name__ == "__main__":
    main()