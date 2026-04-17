#!/usr/bin/env python
"""
Test runner for pycalcmodel unit tests.

This script runs all unit tests in the pycalcmodel test suite and provides
a comprehensive report of test results.
"""

import os
import sys
import subprocess

# Add the Package directory to the path for imports
test_dir = os.path.dirname(os.path.abspath(__file__))
package_dir = os.path.join(test_dir, '..', '..')
sys.path.insert(0, package_dir)

def run_tests():
    """Run all pycalcmodel unit tests."""
    print("=" * 70)
    print("PYCALCMODEL UNIT TEST SUITE")
    print("=" * 70)
    
    # Change to the test directory
    original_dir = os.getcwd()
    os.chdir(test_dir)
    
    try:
        # Try to use pytest first
        try:
            import pytest
            cmd = [
                sys.executable, '-m', 'pytest',
                '-v',                    # Verbose output
                '--tb=short',           # Short traceback format
                '--durations=10',       # Show 10 slowest tests
                '-x',                   # Stop on first failure
                '.'                     # Run all tests in current directory
            ]
            
            print(f"Running command: {' '.join(cmd)}")
            print("-" * 70)
            
            result = subprocess.run(cmd, capture_output=False)
            
            print("-" * 70)
            if result.returncode == 0:
                print("ALL TESTS PASSED!")
            else:
                print("SOME TESTS FAILED!")
            
            return result.returncode
            
        except ImportError:
            # Fallback to unittest
            print("pytest not available, using unittest...")
            return run_tests_with_unittest()
    finally:
        os.chdir(original_dir)

def run_tests_with_unittest():
    """Run tests using unittest module."""
    import unittest
    
    # Discover and run tests
    loader = unittest.TestLoader()
    suite = loader.discover('.', pattern='test_*.py')
    
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    if result.wasSuccessful():
        print("ALL TESTS PASSED!")
        return 0
    else:
        print("SOME TESTS FAILED!")
        return 1

def run_specific_test(test_file):
    """Run a specific test file."""
    print(f"Running specific test: {test_file}")
    
    test_dir = os.path.dirname(os.path.abspath(__file__))
    original_dir = os.getcwd()
    os.chdir(test_dir)
    
    try:
        cmd = [
            sys.executable, '-m', 'pytest',
            '-v',
            '--tb=short',
            test_file
        ]
        
        result = subprocess.run(cmd, capture_output=False)
        return result.returncode
        
    finally:
        os.chdir(original_dir)

def list_available_tests():
    """List all available test files."""
    test_dir = os.path.dirname(os.path.abspath(__file__))
    test_files = [f for f in os.listdir(test_dir) if f.startswith('test_') and f.endswith('.py')]
    
    print("Available test files:")
    for i, test_file in enumerate(test_files, 1):
        print(f"  {i}. {test_file}")
    
    return test_files

def main():
    """Main function for command line execution."""
    if len(sys.argv) == 1:
        # Run all tests
        return run_tests()
    
    elif len(sys.argv) == 2:
        arg = sys.argv[1]
        
        if arg in ['-h', '--help']:
            print("Usage:")
            print("  python run_tests.py              - Run all tests")
            print("  python run_tests.py <test_file>  - Run specific test")
            print("  python run_tests.py --list       - List available tests")
            print("  python run_tests.py --help       - Show this help")
            return 0
        
        elif arg == '--list':
            list_available_tests()
            return 0
        
        else:
            # Run specific test
            test_files = list_available_tests()
            if arg in test_files:
                return run_specific_test(arg)
            else:
                print(f"Test file '{arg}' not found.")
                print("Available tests:")
                for test_file in test_files:
                    print(f"  {test_file}")
                return 1
    
    else:
        print("Too many arguments. Use --help for usage information.")
        return 1

if __name__ == "__main__":
    sys.exit(main())
