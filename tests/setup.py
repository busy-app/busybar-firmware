#!/usr/bin/env python3
"""
BSB Test Automation Setup Script
Helps initialize the project and verify connections
"""

import asyncio
import logging
import os
import subprocess
import sys
from pathlib import Path

import requests
import telnetlib3
from dotenv import load_dotenv

# Setup basic logging for setup script
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s",
    handlers=[logging.StreamHandler(sys.stdout)],
)

logger = logging.getLogger(__name__)


def check_poetry():
    """Check if poetry is installed"""
    try:
        result = subprocess.run(["poetry", "--version"], capture_output=True, text=True)
        logger.info(f"✓ Poetry found: {result.stdout.strip()}")
        return True
    except FileNotFoundError:
        logger.error("✗ Poetry not found. Please install Poetry first:")
        logger.info("  curl -sSL https://install.python-poetry.org | python3 -")
        return False


def setup_environment():
    """Set up the environment file"""
    env_template = Path(".env.template")
    env_file = Path(".env")

    if not env_file.exists() and env_template.exists():
        logger.info("📄 Creating .env file from template...")
        import shutil

        shutil.copy(env_template, env_file)
        logger.info(
            "✓ Created .env file. Please update it with your Allure TestOps token."
        )
        return True
    elif env_file.exists():
        logger.info("✓ .env file already exists")
        return True
    else:
        logger.warning("⚠️  No .env template found")
        return False


async def test_cli_connection():
    """Test CLI connection"""
    load_dotenv()

    host = os.getenv("CLI_HOST", "busybar.local")
    port = int(os.getenv("CLI_PORT", "23"))

    logger.info(f"🔌 Testing CLI connection to {host}:{port}...")

    try:
        reader, writer = await asyncio.wait_for(
            telnetlib3.open_connection(host, port), timeout=10.0
        )

        # Read welcome message
        welcome = await asyncio.wait_for(reader.read(2048), timeout=5.0)

        if "Welcome to BUSY Bar" in welcome:
            logger.info("✓ CLI connection successful!")
            logger.info("✓ BUSY Bar welcome message received")

            # Test a simple command
            writer.write("help\n")
            await writer.drain()

            response = await asyncio.wait_for(reader.read(1024), timeout=5.0)
            if response:
                logger.info("✓ CLI command execution working")

            writer.close()
            try:
                await writer.wait_closed()
            except AttributeError:
                # telnetlib3 TelnetWriterUnicode doesn't have wait_closed method
                pass
            return True
        else:
            logger.warning(f"⚠️  Unexpected welcome message: {welcome[:100]}...")
            return False

    except asyncio.TimeoutError:
        logger.error("✗ CLI connection timeout")
        logger.error(f"  Check if device is accessible at {host}:{port}")
        return False
    except Exception as e:
        logger.error(f"✗ CLI connection failed: {e}")
        return False


def test_web_connection():
    """Test web frontend connection"""
    load_dotenv()

    base_url = os.getenv("WEB_BASE_URL", "http://10.0.4.20/")

    logger.info(f"🌐 Testing web frontend connection to {base_url}...")

    try:
        response = requests.get(base_url, timeout=10)
        if response.status_code == 200:
            logger.info("✓ Web frontend connection successful!")
            logger.info(f"✓ HTTP {response.status_code} response received")
            return True
        else:
            logger.warning(f"⚠️  Web frontend returned HTTP {response.status_code}")
            return False
    except requests.exceptions.RequestException as e:
        logger.error(f"✗ Web frontend connection failed: {e}")
        return False


def install_dependencies():
    """Install project dependencies"""
    logger.info("📦 Installing dependencies with Poetry...")
    try:
        result = subprocess.run(
            ["poetry", "install"], check=True, capture_output=True, text=True
        )
        logger.info("✓ Dependencies installed successfully")
        return True
    except subprocess.CalledProcessError as e:
        logger.error(f"✗ Failed to install dependencies: {e}")
        if e.stderr:
            logger.error(f"Error output: {e.stderr}")
        return False


def create_test_directories():
    """Create test directory structure"""
    directories = [
        "tests",
        "tests/cli",
        "tests/frontend",
        "utils",
        "logs",
        "allure-results",
        "allure-report",
    ]

    for directory in directories:
        Path(directory).mkdir(exist_ok=True)

    # Create __init__.py files for Python packages
    init_files = [
        "tests/__init__.py",
        "tests/cli/__init__.py",
        "tests/frontend/__init__.py",
        "utils/__init__.py",
    ]

    for init_file in init_files:
        Path(init_file).touch()

    logger.info("✓ Test directory structure created")


def check_allure_testops_config():
    """Check Allure TestOps configuration"""
    load_dotenv()

    testops_url = os.getenv("ALLURE_TESTOPS_URL")
    testops_token = os.getenv("ALLURE_TESTOPS_TOKEN")
    project_id = os.getenv("ALLURE_PROJECT_ID")

    if all([testops_url, testops_token, project_id]):
        logger.info("✓ Allure TestOps configuration complete")
        logger.info(f"  URL: {testops_url}")
        logger.info(f"  Project ID: {project_id}")
        return True
    else:
        logger.warning("⚠️  Allure TestOps configuration incomplete")
        if not testops_token:
            logger.warning("  Missing ALLURE_TESTOPS_TOKEN in .env file")
        logger.info("  Please update .env file with your TestOps credentials")
        return False


def check_github_actions_setup():
    """Check if GitHub Actions workflow is set up"""
    workflow_dir = Path("../.github/workflows")
    workflow_files = ["smoke-tests.yml", "run-tests.yml"]

    for workflow_file in workflow_files:
        if (workflow_dir / workflow_file).exists():
            logger.info(f"✓ GitHub Actions workflow found: {workflow_file}")
            return True

    logger.warning("⚠️  No GitHub Actions workflow found")
    logger.info("  Consider setting up CI/CD for automated testing")
    return False


async def main():
    """Main setup function"""
    logger.info("🚀 BSB Test Automation Setup")
    logger.info("=" * 40)

    # Check prerequisites
    if not check_poetry():
        sys.exit(1)

    # Setup environment
    setup_environment()

    # Install dependencies
    if not install_dependencies():
        sys.exit(1)

    # Create directories
    create_test_directories()

    # Test connections
    logger.info("\n🔍 Testing Connections")
    logger.info("-" * 30)

    cli_ok = await test_cli_connection()
    web_ok = test_web_connection()

    # Check configurations
    logger.info("\n⚙️  Configuration Check")
    logger.info("-" * 30)
    testops_ok = check_allure_testops_config()
    github_ok = check_github_actions_setup()

    # Summary
    logger.info("\n📋 Setup Summary")
    logger.info("-" * 20)
    logger.info(f"CLI Connection:     {'✓' if cli_ok else '✗'}")
    logger.info(f"Web Connection:     {'✓' if web_ok else '✗'}")
    logger.info(f"TestOps Config:     {'✓' if testops_ok else '⚠️'}")
    logger.info(f"GitHub Actions:     {'✓' if github_ok else '⚠️'}")

    if cli_ok and web_ok:
        logger.info("\n🎉 Setup complete! You can now run tests:")
        logger.info("  poetry run pytest tests/ -v")
        logger.info("  poetry run pytest tests/cli/ -m cli")
        logger.info("  poetry run pytest tests/frontend/ -m frontend")
        logger.info("  make test  # if using Makefile")
        logger.info("  python run_tests.py --suite all --upload  # with TestOps upload")
    else:
        logger.warning("\n⚠️  Please fix connection issues before running tests")
        sys.exit(1)


if __name__ == "__main__":
    asyncio.run(main())
