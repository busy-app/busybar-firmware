# BSB Test Automation Makefile

.PHONY: help setup install test test-cli test-frontend test-upload test-report clean lint format

# Default target
help:
	@echo "BSB Test Automation Commands:"
	@echo ""
	@echo "Setup & Installation:"
	@echo "  make setup      - Full project setup (runs setup.py)"
	@echo "  make install    - Install dependencies only"
	@echo ""
	@echo "Testing:"
	@echo "  make test       - Run all tests"
	@echo "  make test-cli   - Run CLI tests only"
	@echo "  make test-frontend - Run frontend tests only"
	@echo "  make test-upload   - Run tests and upload to TestOps"
	@echo "  make test-report   - Run tests and generate local report"
	@echo ""
	@echo "Development:"
	@echo "  make lint       - Run code linting"
	@echo "  make format     - Format code with black"
	@echo "  make clean      - Clean test artifacts"
	@echo ""
	@echo "Custom runs:"
	@echo "  make test-story STORY=commands_check"
	@echo "  make test-markers MARKERS='cli and story_commands_check'"

# Setup and installation
setup:
	@echo "🚀 Running full BSB automation setup..."
	python setup.py

install:
	@echo "📦 Installing dependencies..."
	poetry install

# Testing targets
test:
	@echo "🧪 Running all tests..."
	poetry run python run_tests.py --suite all --verbose

test-cli:
	@echo "🖥️  Running CLI tests..."
	poetry run python run_tests.py --suite cli --verbose

test-frontend:
	@echo "🌐 Running frontend tests..."
	poetry run python run_tests.py --suite frontend --verbose

test-upload:
	@echo "📤 Running tests with TestOps upload..."
	poetry run python run_tests.py --suite all --upload --verbose

test-report:
	@echo "📊 Running tests with local report..."
	poetry run python run_tests.py --suite all --report --verbose

# Custom test runs
test-story:
	@echo "📖 Running tests for story: $(STORY)"
	poetry run python run_tests.py --markers "story_$(STORY)" --verbose

test-markers:
	@echo "🏷️  Running tests with markers: $(MARKERS)"
	poetry run python run_tests.py --markers "$(MARKERS)" --verbose

# Development tools
lint:
	@echo "🔍 Running code linting..."
	poetry run flake8 tests/ conftest.py setup.py run_tests.py --max-line-length=88 --ignore=E203,W503

format:
	@echo "🎨 Formatting code..."
	poetry run black tests/ conftest.py setup.py run_tests.py --line-length=88

# Cleanup
clean:
	@echo "🧹 Cleaning test artifacts..."
	rm -rf allure-results/
	rm -rf allure-report/
	rm -rf .pytest_cache/
	rm -rf **/__pycache__/
	rm -rf **/*.pyc

# Quick verification
verify:
	@echo "✅ Verifying setup..."
	@echo "Poetry version:" && poetry --version
	@echo "Python version:" && poetry run python --version
	@echo "Pytest version:" && poetry run pytest --version
	@echo "Testing CLI connection..."
	@poetry run python -c "import asyncio; from conftest import CLIFixture; cli = CLIFixture(); print('✓ CLI imports OK')"
	@echo "Testing web imports..."
	@poetry run python -c "import requests; print('✓ Web imports OK')"

# Development server (if you want to run a local test web server)
dev-server:
	@echo "🌐 Starting development web server for testing..."
	@echo "Note: This is for testing the test framework, not the actual BSB device"
	poetry run python -c "import http.server; import socketserver; PORT = 8080; Handler = http.server.SimpleHTTPRequestHandler; httpd = socketserver.TCPServer(('', PORT), Handler); print(f'Serving at http://localhost:{PORT}'); httpd.serve_forever()"

# Generate requirements.txt (for environments that don't use Poetry)
requirements:
	@echo "📄 Generating requirements.txt..."
	poetry export -f requirements.txt --output requirements.txt --without-hashes

# Show project info
info:
	@echo "📋 Project Information:"
	@echo "Project: BSB Test Automation"
	@echo "Poetry:"
	@poetry --version
	@echo ""
	@echo "Dependencies:"
	@poetry show --tree
	@echo ""
	@echo "Environment variables needed:"
	@echo "  CLI_HOST (default: busybar.local)"
	@echo "  CLI_PORT (default: 23)" 
	@echo "  WEB_BASE_URL (default: http://10.0.4.20/)"
	@echo "  ALLURE_TESTOPS_TOKEN (required for upload)"
	@echo ""
	@echo "Test markers available:"
	@poetry run pytest --markers

# CI simulation (runs what CI would run)
ci-test:
	@echo "🤖 Simulating CI test run..."
	make clean
	make install
	poetry run pytest tests/ -v --alluredir=allure-results
	@echo "✅ CI simulation complete"