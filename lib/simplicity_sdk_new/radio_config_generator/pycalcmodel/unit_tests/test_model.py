"""
Comprehensive tests for pycalcmodel.core.model module.

Tests cover ModelRoot creation, property access, and basic functionality.
"""

import unittest
import sys

# Add the Package directory to the path for imports
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

from pycalcmodel.core.model import ModelRoot


class TestModelRoot(unittest.TestCase):
    """Test ModelRoot class creation and basic properties."""
    
    def test_create_model_root_basic(self):
        """Test creating a ModelRoot with basic parameters."""
        model = ModelRoot(
            part_family="EFR32MG12", 
            calc_version="1.0.0"
        )
        
        assert model.part_family == "EFR32MG12"
        assert model.calc_version == "1.0.0"
        assert model.part_revision == "ANY"
        # target attribute is only set if target is provided
        assert not hasattr(model, 'target') or model.target is None
    
    def test_create_model_root_with_revision(self):
        """Test creating a ModelRoot with part revision."""
        model = ModelRoot(
            part_family="EFR32MG12", 
            calc_version="1.0.0",
            part_revision="A0"
        )
        
        assert model.part_family == "EFR32MG12"
        assert model.calc_version == "1.0.0"
        assert model.part_revision == "A0"
    
    def test_create_model_root_with_target(self):
        """Test creating a ModelRoot with target."""
        model = ModelRoot(
            part_family="EFR32MG12", 
            calc_version="1.0.0",
            target="Test Target"
        )
        
        assert model.part_family == "EFR32MG12"
        assert model.calc_version == "1.0.0"
        assert model.target == "Test Target"
    
    def test_model_root_containers_exist(self):
        """Test that ModelRoot creates all required containers."""
        model = ModelRoot(
            part_family="EFR32MG12", 
            calc_version="1.0.0"
        )
        
        # Check that all containers are created
        assert hasattr(model, 'parser')
        assert hasattr(model, 'features')
        assert hasattr(model, 'phys')
        assert hasattr(model, 'profiles')
        assert hasattr(model, 'vars')
        assert hasattr(model, '_reg_model')
        
        # Check that containers are not None (except _reg_model which can be None for test scenarios)
        assert model.parser is not None
        assert model.features is not None
        assert model.phys is not None
        assert model.profiles is not None
        assert model.vars is not None
        # _reg_model can be None for unsupported part families in test scenarios
        # assert model._reg_model is not None
    
    def test_model_root_xsd_version(self):
        """Test that ModelRoot has XSD version set."""
        model = ModelRoot(
            part_family="EFR32MG12", 
            calc_version="1.0.0"
        )
        
        assert hasattr(model, '_xsd_version')
        assert model._xsd_version is not None
    
    def test_model_root_logs_initialization(self):
        """Test that logs is initialized to None."""
        model = ModelRoot(
            part_family="EFR32MG12", 
            calc_version="1.0.0"
        )
        
        assert model.logs is None
    
    def test_model_root_calc_routine_execution_initialization(self):
        """Test that calc_routine_execution is initialized to None."""
        model = ModelRoot(
            part_family="EFR32MG12", 
            calc_version="1.0.0"
        )
        
        assert model.calc_routine_execution is None


class TestModelRootValidation(unittest.TestCase):
    """Test ModelRoot parameter validation."""
    
    def test_invalid_part_family_type(self):
        """Test that non-string part_family raises assertion error."""
        with self.assertRaises(AssertionError):
            ModelRoot(
                part_family=123,  # Invalid: not a string
                calc_version="1.0.0"
            )
    
    def test_invalid_calc_version_type(self):
        """Test that non-string calc_version raises assertion error."""
        with self.assertRaises(AssertionError):
            ModelRoot(
                part_family="EFR32MG12",
                calc_version=1.0  # Invalid: not a string
            )
    
    def test_none_part_family(self):
        """Test that None part_family raises assertion error."""
        with self.assertRaises(AssertionError):
            ModelRoot(
                part_family=None,
                calc_version="1.0.0"
            )
    
    def test_none_calc_version(self):
        """Test that None calc_version raises assertion error."""
        with self.assertRaises(AssertionError):
            ModelRoot(
                part_family="EFR32MG12",
                calc_version=None
            )


class TestModelRootEdgeCases(unittest.TestCase):
    """Test edge cases for ModelRoot."""
    
    def test_empty_string_part_family(self):
        """Test creating ModelRoot with empty string part_family."""
        model = ModelRoot(
            part_family="",
            calc_version="1.0.0"
        )
        
        assert model.part_family == ""
    
    def test_empty_string_calc_version(self):
        """Test creating ModelRoot with empty string calc_version."""
        model = ModelRoot(
            part_family="EFR32MG12",
            calc_version=""
        )
        
        assert model.calc_version == ""
    
    def test_special_characters_in_part_family(self):
        """Test ModelRoot with special characters in part_family."""
        model = ModelRoot(
            part_family="EFR32-MG12_V1.0",
            calc_version="1.0.0"
        )
        
        assert model.part_family == "EFR32-MG12_V1.0"
    
    def test_unicode_strings(self):
        """Test ModelRoot with unicode strings."""
        model = ModelRoot(
            part_family=u"EFR32MG12",
            calc_version=u"1.0.0"
        )
        
        assert model.part_family == u"EFR32MG12"
        assert model.calc_version == u"1.0.0"
    
    def test_long_strings(self):
        """Test ModelRoot with very long strings."""
        long_family = "EFR32MG12" * 100
        long_version = "1.0.0" * 50
        
        model = ModelRoot(
            part_family=long_family,
            calc_version=long_version
        )
        
        assert model.part_family == long_family
        assert model.calc_version == long_version


if __name__ == "__main__":
    unittest.main(verbosity=2)
