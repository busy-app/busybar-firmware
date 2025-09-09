# BSB Test Automation Makefile
# Modern pytest project with Allure TestOps integration

.PHONY: help setup install test test-cli test-frontend test-upload test-report clean lint format verify info ci-test dev-server requirements test-story test-markers test-allure

# Default target
help:
	@echo "🚀 BSB Test Automation - Available Commands"
	@echo "=========================================="
	@echo ""
	@echo "📦 Setup & Installation:"
	@echo "  make setup      - Full project setup (runs setup.py)"
	@echo "  make install    - Install dependencies with Poetry"
	@echo "  make verify     - Verify environment and connections"
	@echo ""
	@echo "🧪 Testing:"
	@echo "  make test       - Run all tests with pytest"
	@echo "  make test-cli   - Run CLI tests only"
	@echo "  make test-frontend - Run frontend tests only"
	@echo "  make test-upload   - Run tests and upload to TestOps"
	@echo "  make test-report   - Run tests and generate local report"
	@echo "  make test-allure   - Generate and serve Allure report"
	@echo ""
	@echo "🎯 Custom Test Runs:"
	@echo "  make test-story STORY=commands_check"
	@echo "  make test-markers MARKERS='cli and story_commands_check'"
	@echo ""
	@echo "🔧 Development:"
	@echo "  make lint       - Run code linting with flake8"
	@echo "  make format     - Format code with black"
	@echo "  make clean      - Clean test artifacts and cache"
	@echo "  make dev-server - Start local test web server"
	@echo ""
	@echo "📋 Information:"
	@echo "  make info       - Show project information"
	@echo "  make requirements - Generate requirements.txt"
	@echo "  make ci-test    - Simulate CI/CD pipeline"

# Setup and installation
setup:
	@echo "🚀 Running full BSB automation setup..."
	@python setup.py

install:
	@echo "📦 Installing dependencies with Poetry..."
	@poetry install --no-interaction

# Testing targets
test:
	@echo "🧪 Running all tests with pytest..."
	@poetry run pytest tests/ -v --alluredir=allure-results --clean-alluredir

test-cli:
	@echo "🖥️  Running CLI tests..."
	@poetry run pytest tests/cli/ -v -m cli --alluredir=allure-results --clean-alluredir

test-frontend:
	@echo "🌐 Running frontend tests..."
	@poetry run pytest tests/frontend/ -v -m frontend --alluredir=allure-results --clean-alluredir

test-upload:
	@echo "📤 Running tests with TestOps upload..."
	@poetry run python run_tests.py --suite all --upload --verbose

test-report:
	@echo "📊 Running tests with local report..."
	@poetry run python run_tests.py --suite all --report --verbose

test-allure:
	@echo "📈 Generating and serving Allure report..."
	@if [ ! -d "allure-results" ]; then \
		echo "⚠️  No test results found. Running tests first..."; \
		make test; \
	fi
	@poetry run allure serve allure-results

# Custom test runs
test-story:
	@if [ -z "$(STORY)" ]; then \
		echo "❌ Error: STORY parameter is required"; \
		echo "Usage: make test-story STORY=commands_check"; \
		exit 1; \
	fi
	@echo "📖 Running tests for story: $(STORY)"
	@poetry run pytest tests/ -v -m "story_$(STORY)" --alluredir=allure-results --clean-alluredir

test-markers:
	@if [ -z "$(MARKERS)" ]; then \
		echo "❌ Error: MARKERS parameter is required"; \
		echo "Usage: make test-markers MARKERS='cli and story_commands_check'"; \
		exit 1; \
	fi
	@echo "🏷️  Running tests with markers: $(MARKERS)"
	@poetry run pytest tests/ -v -m "$(MARKERS)" --alluredir=allure-results --clean-alluredir

# Development tools
lint:
	@echo "🔍 Running code linting with flake8..."
	@poetry run flake8 tests/ utils/ conftest.py setup.py run_tests.py \
		--max-line-length=88 \
		--ignore=E203,W503,E501 \
		--exclude=__pycache__,.git,venv,.venv

format:
	@echo "🎨 Formatting code with black..."
	@poetry run black tests/ utils/ conftest.py setup.py run_tests.py --line-length=88

format-check:
	@echo "🔍 Checking code formatting..."
	@poetry run black --check tests/ utils/ conftest.py setup.py run_tests.py --line-length=88

# Cleanup
clean:
	@echo "🧹 Cleaning test artifacts and cache..."
	@rm -rf allure-results/
	@rm -rf allure-report/
	@rm -rf .pytest_cache/
	@find . -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
	@find . -name "*.pyc" -delete 2>/dev/null || true
	@find . -name "*.pyo" -delete 2>/dev/null || true
	@find . -name ".coverage" -delete 2>/dev/null || true
	@find . -name "htmlcov" -type d -exec rm -rf {} + 2>/dev/null || true

# Quick verification
verify:
	@echo "✅ Verifying BSB automation setup..."
	@echo "=================================="
	@echo "📦 Poetry version:" && poetry --version
	@echo "🐍 Python version:" && poetry run python --version
	@echo "🧪 Pytest version:" && poetry run pytest --version
	@echo "📊 Allure version:" && poetry run allure --version 2>/dev/null || echo "⚠️  Allure not installed"
	@echo ""
	@echo "🔌 Testing imports..."
	@poetry run python -c "import telnetlib, requests, allure; from bs4 import BeautifulSoup; print('✓ All imports successful')" || echo "❌ Import test failed"
	@echo ""
	@echo "🔧 Testing CLI fixture..."
	@poetry run python -c "from conftest import CLIFixture; cli = CLIFixture(); print('✓ CLI fixture OK')" || echo "❌ CLI fixture test failed"
	@echo ""
	@echo "🌐 Testing web imports..."
	@poetry run python -c "import requests; print('✓ Web imports OK')" || echo "❌ Web imports failed"
	@echo ""
	@echo "📁 Checking directory structure..."
	@test -d tests && echo "✓ tests/ directory exists" || echo "❌ tests/ directory missing"
	@test -d tests/cli && echo "✓ tests/cli/ directory exists" || echo "❌ tests/cli/ directory missing"
	@test -d tests/frontend && echo "✓ tests/frontend/ directory exists" || echo "❌ tests/frontend/ directory missing"
	@test -d utils && echo "✓ utils/ directory exists" || echo "❌ utils/ directory missing"

# Development server (for testing the test framework)
dev-server:
	@echo "🌐 Starting development web server for testing..."
	@echo "📍 Server will be available at http://localhost:8080"
	@echo "⚠️  Note: This is for testing the test framework, not the actual BSB device"
	@poetry run python -c "import http.server, socketserver; PORT = 8080; Handler = http.server.SimpleHTTPRequestHandler; httpd = socketserver.TCPServer(('', PORT), Handler); print(f'🚀 Serving at http://localhost:{PORT}'); print('Press Ctrl+C to stop'); httpd.serve_forever()"

# Generate requirements.txt (for environments that don't use Poetry)
requirements:
	@echo "📄 Generating requirements.txt..."
	@poetry export -f requirements.txt --output requirements.txt --without-hashes
	@echo "✅ requirements.txt generated"

# Show project info
info:
	@echo "📋 BSB Test Automation - Project Information"
	@echo "==========================================="
	@echo "📦 Poetry Environment:"
	@poetry --version
	@echo ""
	@echo "🐍 Python Environment:"
	@poetry run python --version
	@poetry run which python
	@echo ""
	@echo "📚 Dependencies:"
	@poetry show --tree --no-dev
	@echo ""
	@echo "🔧 Development Dependencies:"
	@poetry show --tree --only dev
	@echo ""
	@echo "⚙️  Environment Variables:"
	@echo "  CLI_HOST (default: busybar.local)"
	@echo "  CLI_PORT (default: 23)" 
	@echo "  WEB_BASE_URL (default: http://10.0.4.20/)"
	@echo "  ALLURE_TESTOPS_URL (default: https://flipper.testops.cloud)"
	@echo "  ALLURE_TESTOPS_TOKEN (required for upload)"
	@echo "  ALLURE_PROJECT_ID (default: 232)"
	@echo "  LOG_LEVEL (default: INFO)"
	@echo "  LOG_TO_FILE (default: true)"
	@echo ""
	@echo "🏷️  Available Test Markers:"
	@poetry run pytest --markers | head -20
	@echo "  ... (run 'poetry run pytest --markers' for full list)"
	@echo ""
	@echo "📁 Project Structure:"
	@echo "  tests/cli/          - CLI command tests"
	@echo "  tests/frontend/     - Web frontend tests"
	@echo "  utils/              - Utility modules"
	@echo "  conftest.py         - Pytest configuration & fixtures"
	@echo "  run_tests.py        - Test runner script"
	@echo "  setup.py            - Setup script"

# CI simulation (runs what CI would run)
ci-test:
	@echo "🤖 Simulating CI/CD pipeline..."
	@echo "============================="
	@echo "1️⃣  Cleaning previous artifacts..."
	@make clean
	@echo ""
	@echo "2️⃣  Installing dependencies..."
	@make install
	@echo ""
	@echo "3️⃣  Running code formatting check..."
	@make format-check
	@echo ""
	@echo "4️⃣  Running code linting..."
	@make lint
	@echo ""
	@echo "5️⃣  Running all tests..."
	@poetry run pytest tests/ -v --alluredir=allure-results --clean-alluredir --tb=short
	@echo ""
	@echo "6️⃣  Generating test report..."
	@if [ -d "allure-results" ]; then \
		echo "📊 Test results available in allure-results/"; \
		echo "📈 Run 'make test-allure' to view the report"; \
	else \
		echo "⚠️  No test results generated"; \
	fi
	@echo ""
	@echo "✅ CI simulation complete!"

# Quick test run (for development)
quick:
	@echo "⚡ Running quick test suite..."
	@poetry run pytest tests/ -x --tb=short --maxfail=3

# Test specific test file
test-file:
	@if [ -z "$(FILE)" ]; then \
		echo "❌ Error: FILE parameter is required"; \
		echo "Usage: make test-file FILE=tests/cli/test_commands.py"; \
		exit 1; \
	fi
	@echo "📄 Running tests from file: $(FILE)"
	@poetry run pytest $(FILE) -v --alluredir=allure-results --clean-alluredir

# Show test statistics
stats:
	@echo "📊 Test Statistics:"
	@echo "=================="
	@echo "Total test files:"
	@find tests/ -name "test_*.py" | wc -l
	@echo ""
	@echo "CLI test files:"
	@find tests/cli/ -name "test_*.py" | wc -l
	@echo ""
	@echo "Frontend test files:"
	@find tests/frontend/ -name "test_*.py" | wc -l
	@echo ""
	@echo "Total test functions:"
	@poetry run pytest --collect-only -q | grep "test_" | wc -l