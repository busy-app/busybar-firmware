#!/usr/bin/env python3
"""
Cross-platform test runner script for BSB automation
Provides easy way to run tests and upload to Allure TestOps on Windows, macOS, and Linux
"""

import argparse
import subprocess
import sys
import os
import logging
import platform
import shutil
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


def get_system_info():
    """Get system information for cross-platform compatibility"""
    system = platform.system().lower()
    return {
        'system': system,
        'is_windows': system == 'windows',
        'is_macos': system == 'darwin',
        'is_linux': system == 'linux',
        'shell': 'cmd' if system == 'windows' else 'bash'
    }


def run_command(cmd, description="", capture_output=True, stream_output=False, shell=None):
    """Run a shell command with cross-platform support"""
    if description:
        logger.info(f"🔄 {description}")

    # Determine shell based on system
    sys_info = get_system_info()
    if shell is None:
        shell = sys_info['is_windows']

    try:
        if stream_output:
            # Stream output in real-time for test execution
            if sys_info['is_windows']:
                # Windows command handling
                process = subprocess.Popen(
                    cmd,
                    shell=shell,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    universal_newlines=True,
                    bufsize=1
                )
            else:
                # Unix-like systems (macOS, Linux)
                process = subprocess.Popen(
                    cmd,
                    shell=shell,
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
            result = subprocess.run(cmd, shell=shell, check=True, capture_output=capture_output, text=True)
            if result.stdout and capture_output:
                logger.info(result.stdout)
            return True

    except subprocess.CalledProcessError as e:
        logger.error(f"❌ Error: {e}")
        if hasattr(e, 'stderr') and e.stderr:
            logger.error(f"Error details: {e.stderr}")
        return False
    except FileNotFoundError as e:
        logger.error(f"❌ Command not found: {e}")
        return False


def check_dependencies():
    """Check if required dependencies are available"""
    logger.info("🔍 Checking dependencies...")

    # Check Poetry
    try:
        result = subprocess.run(['poetry', '--version'], capture_output=True, text=True)
        if result.returncode == 0:
            logger.info(f"✅ Poetry found: {result.stdout.strip()}")
        else:
            logger.error("❌ Poetry not found. Please install Poetry first.")
            logger.info("💡 Install from: https://python-poetry.org/docs/#installation")
            return False
    except FileNotFoundError:
        logger.error("❌ Poetry not found. Please install Poetry first.")
        logger.info("💡 Install from: https://python-poetry.org/docs/#installation")
        return False

    # Check Python version
    python_version = sys.version_info
    if python_version.major == 3 and python_version.minor >= 9:
        logger.info(f"✅ Python version: {sys.version}")
    else:
        logger.error(f"❌ Python 3.9+ required, found {python_version.major}.{python_version.minor}")
        return False

    return True


def setup_allure_cli():
    """Set up Allure CLI based on the operating system"""
    sys_info = get_system_info()
    logger.info(f"Setting up Allure CLI for {sys_info['system']}")

    # Check if allure is already available
    try:
        result = subprocess.run(['allure', '--version'], capture_output=True, text=True)
        if result.returncode == 0:
            logger.info(f"Allure already installed: {result.stdout.strip()}")
            return True
    except FileNotFoundError:
        pass

    # Install Allure based on OS
    if sys_info['is_macos']:
        logger.info("Installing Allure via Homebrew...")
        return run_command("brew install allure", "Installing Allure on macOS")

    elif sys_info['is_linux']:
        logger.info("Installing Allure via download...")
        install_cmd = """
        curl -o allure-commandline.tgz -L https://repo.maven.apache.org/maven2/io/qameta/allure/allure-commandline/2.24.0/allure-commandline-2.24.0.tgz
        tar -xzf allure-commandline.tgz
        sudo mv allure-* /opt/allure
        sudo ln -s /opt/allure/bin/allure /usr/local/bin/allure
        rm allure-commandline.tgz
        """
        return run_command(install_cmd, "Installing Allure on Linux")

    elif sys_info['is_windows']:
        logger.info("Please install Allure manually on Windows")
        logger.info("Download from: https://github.com/allure-framework/allure2/releases")
        logger.info("Or use Scoop: scoop install allure")
        return False

    return False


def check_allurectl():
    """Check if allurectl is available"""
    try:
        result = subprocess.run(['allurectl', '--version'], capture_output=True, text=True)
        if result.returncode == 0:
            logger.info(f"AllureCtl found: {result.stdout.strip()}")
            return True
    except FileNotFoundError:
        pass

    logger.warning("AllureCtl not found")
    logger.info("Download from: https://github.com/allure-framework/allurectl/releases")
    return False


def clean_previous_results():
    """Clean previous test results"""
    logger.info("Cleaning previous test results...")
    results_dir = Path("allure-results")
    if results_dir.exists():
        try:
            shutil.rmtree(results_dir)
            results_dir.mkdir(exist_ok=True)
            logger.info("Previous results cleaned")
            return True
        except Exception as e:
            logger.warning(f"Could not clean previous results: {e}")
            return False
    else:
        results_dir.mkdir(exist_ok=True)
        return True


def create_launch_name(args):
    """Create launch name for test execution"""
    timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M")

    if args.launch_name:
        return args.launch_name
    else:
        return f"BSB {args.suite.title()} Tests - Local Run {timestamp}"


def build_pytest_command(args):
    """Build pytest command based on arguments"""
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
    pytest_cmd += " --tb=short"

    # Add Allure directory
    pytest_cmd += " --alluredir=allure-results"

    return pytest_cmd


def upload_to_testops(launch_name):
    """Upload results to Allure TestOps"""
    logger.info("Uploading results to Allure TestOps...")

    testops_url = os.getenv('ALLURE_TESTOPS_URL', "https://flipper.testops.cloud")
    testops_token = os.getenv('ALLURE_TESTOPS_TOKEN')
    project_id = os.getenv('ALLURE_PROJECT_ID', "232")

    if not testops_token:
        logger.error("ALLURE_TESTOPS_TOKEN not found in environment")
        return False

    if not check_allurectl():
        return False

    # Build upload command with proper escaping
    upload_cmd = [
        'allurectl', 'upload', 'allure-results',
        '--endpoint', testops_url,
        '--token', testops_token,
        '--project-id', project_id,
        '--launch-name', launch_name
    ]

    try:
        result = subprocess.run(upload_cmd, check=True, capture_output=True, text=True)
        logger.info("Results uploaded to Allure TestOps!")
        logger.info(f"View results: {testops_url}/project/{project_id}")
        return True
    except subprocess.CalledProcessError as e:
        logger.error(f"Failed to upload to TestOps: {e}")
        if e.stderr:
            logger.error(f"Error details: {e.stderr}")
        return False


def generate_local_report():
    """Generate and serve local Allure report"""
    logger.info("Generating local Allure report...")

    if not setup_allure_cli():
        logger.error("Could not setup Allure CLI")
        return False

    # Generate report
    if not run_command("allure generate allure-results -o allure-report --clean", "Generating report"):
        logger.error("Failed to generate Allure report")
        return False

    logger.info("Report generated!")
    logger.info("Report location: allure-report/index.html")

    # Try to serve the report
    try:
        logger.info("Starting local report server...")
        logger.info("Press Ctrl+C to stop the server")
        subprocess.run(["allure", "serve", "allure-results"], check=True)
    except KeyboardInterrupt:
        logger.info("Report server stopped")
    except subprocess.CalledProcessError:
        logger.warning("Could not start report server, but files are generated in allure-report/")

        # Try to open report directly
        report_path = Path("allure-report/index.html").absolute()
        if report_path.exists():
            logger.info(f"Open manually: file://{report_path}")

    return True


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
    parser.add_argument(
        "--skip-deps-check",
        action="store_true",
        help="Skip dependency checking"
    )

    args = parser.parse_args()

    # Check dependencies unless skipped
    if not args.skip_deps_check and not check_dependencies():
        sys.exit(1)

    # Load environment
    load_dotenv()

    # Get system info
    sys_info = get_system_info()
    logger.info(f"Running on {sys_info['system']} system")

    # Create launch name
    launch_name = create_launch_name(args)

    # Set environment variables for test run
    os.environ["LOG_LEVEL"] = args.log_level
    os.environ["LOG_TO_FILE"] = "true"

    # Clean previous results
    if not clean_previous_results():
        logger.warning("Could not clean previous results, continuing...")

    # Build pytest command
    pytest_cmd = build_pytest_command(args)

    # Run tests
    logger.info(f"Running {args.suite} tests...")
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
        logger.error("Tests failed!")
        if not args.report and not args.upload:
            sys.exit(1)
    else:
        logger.info("Tests completed successfully!")

    # Check if we have test results
    results_path = Path("allure-results")
    if not results_path.exists() or not any(results_path.iterdir()):
        logger.warning("No test results found to process")
        sys.exit(1)

    # Show summary of results
    json_files = list(results_path.glob("*-result.json"))
    logger.info(f"Generated {len(json_files)} test result files")

    # Upload to TestOps if requested
    if args.upload:
        upload_success = upload_to_testops(launch_name)
        if not upload_success:
            logger.error("Failed to upload to TestOps")

    # Generate local report if requested
    if args.report:
        generate_local_report()

    # Show logs location
    logs_dir = Path("logs")
    if logs_dir.exists():
        latest_log = logs_dir / "latest.log"
        if latest_log.exists():
            logger.info(f"Test logs available at: {latest_log}")

    logger.info("All operations completed!")
    if success:
        logger.info("Test run successful")
    else:
        logger.warning("Some tests may have failed - check the results")


if __name__ == "__main__":
    main()