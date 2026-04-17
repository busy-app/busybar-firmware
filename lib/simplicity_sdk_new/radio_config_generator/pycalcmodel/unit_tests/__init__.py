"""
pycalcmodel unit tests package.

This package contains comprehensive unit tests for the pycalcmodel system,
covering variable types, XML serialization/deserialization, model creation,
features, and integration testing.

Test modules:
- test_variable: Tests for ModelVariable class and variable handling
- test_common: Tests for common utility functions and XML handling
- test_model: Tests for ModelRoot and basic model functionality  
- test_feature: Tests for ModelFeature and feature handling
- test_integration: Integration tests across components
- test_dict_support: Dictionary type support tests
- test_dict_standalone: Standalone dictionary functionality tests

Usage:
    # Run all tests
    python -m pytest

    # Run specific test module
    python -m pytest test_variable.py

    # Run with coverage
    python -m pytest --cov=pycalcmodel

    # Use the test runner
    python run_tests.py
"""

__version__ = '1.0.0'
__author__ = 'pycalcmodel team'