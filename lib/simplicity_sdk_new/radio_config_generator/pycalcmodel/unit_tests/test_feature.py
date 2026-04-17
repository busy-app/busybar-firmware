"""
Comprehensive tests for pycalcmodel.core.feature module.

Tests cover ModelFeature creation, properties, and ModelFeatureContainer functionality.
"""

import unittest
import sys

# Add the Package directory to the path for imports
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

from pycalcmodel.core.feature import ModelFeature, ModelFeatureContainer
from pycalcmodel.core.parser import ModelParser


class TestModelFeature(unittest.TestCase):
    """Test ModelFeature class creation and properties."""
    
    def test_create_feature_basic(self):
        """Test creating a ModelFeature with basic parameters."""
        feature = ModelFeature(name="test_feature")
        
        assert feature.name == "test_feature"
        assert feature.desc == ""
        assert feature.value is False
        assert feature.zz_parser_update is None
    
    def test_create_feature_with_desc(self):
        """Test creating a ModelFeature with description."""
        feature = ModelFeature(
            name="test_feature", 
            desc="This is a test feature"
        )
        
        assert feature.name == "test_feature"
        assert feature.desc == "This is a test feature"
        assert feature.value is False
    
    def test_create_feature_with_value(self):
        """Test creating a ModelFeature with initial value."""
        feature = ModelFeature(
            name="test_feature",
            desc="Test feature",
            value=True
        )
        
        assert feature.name == "test_feature"
        assert feature.desc == "Test feature"
        assert feature.value is True
    
    def test_feature_name_property(self):
        """Test feature name property setter and getter."""
        feature = ModelFeature(name="initial_name")
        assert feature.name == "initial_name"
        
        feature.name = "new_name"
        assert feature.name == "new_name"
    
    def test_feature_desc_property(self):
        """Test feature description property setter and getter."""
        feature = ModelFeature(name="test", desc="initial desc")
        assert feature.desc == "initial desc"
        
        feature.desc = "new description"
        assert feature.desc == "new description"
    
    def test_feature_value_property(self):
        """Test feature value property setter and getter."""
        feature = ModelFeature(name="test", value=False)
        assert feature.value is False
        
        feature.value = True
        assert feature.value is True
        
        feature.value = False
        assert feature.value is False
    
    def test_feature_string_representation(self):
        """Test __str__ method of ModelFeature."""
        feature = ModelFeature(
            name="test_feature",
            desc="Test description",
            value=True
        )
        
        str_repr = str(feature)
        assert "test_feature" in str_repr
        assert "Test description" in str_repr
        assert "True" in str_repr
    
    def test_register_parser_function(self):
        """Test registering parser update function."""
        feature = ModelFeature(name="test_feature")
        
        # Mock parser update function
        update_calls = []
        def mock_parser_update(name, value):
            update_calls.append((name, value))
        
        feature.register_parser(mock_parser_update)
        assert feature.zz_parser_update is not None
        
        # Test that parser update is called when value changes
        feature.value = True
        assert len(update_calls) == 1
        assert update_calls[0] == ("test_feature", True)
        
        feature.value = False
        assert len(update_calls) == 2
        assert update_calls[1] == ("test_feature", False)


class TestModelFeatureValidation(unittest.TestCase):
    """Test ModelFeature parameter validation."""
    
    def test_invalid_name_type(self):
        """Test that non-string name raises assertion error."""
        with self.assertRaises(AssertionError):
            ModelFeature(name=123)
    
    def test_invalid_desc_type(self):
        """Test that non-string description raises assertion error."""
        feature = ModelFeature(name="test")
        with self.assertRaises(AssertionError):
            feature.desc = 123
    
    def test_invalid_value_type(self):
        """Test that non-bool value raises assertion error."""
        feature = ModelFeature(name="test")
        with self.assertRaises(AssertionError):
            feature.value = "not_a_bool"
        
        with self.assertRaises(AssertionError):
            feature.value = 1
        
        with self.assertRaises(AssertionError):
            feature.value = None
    
    def test_none_name(self):
        """Test that None name raises assertion error."""
        with self.assertRaises(AssertionError):
            ModelFeature(name=None)
    
    def test_invalid_parser_registration(self):
        """Test that non-callable parser function raises assertion error."""
        feature = ModelFeature(name="test")
        with self.assertRaises(AssertionError):
            feature.register_parser("not_callable")
        
        with self.assertRaises(AssertionError):
            feature.register_parser(123)


class TestModelFeatureContainer(unittest.TestCase):
    """Test ModelFeatureContainer class."""
    
    def test_create_feature_container(self):
        """Test creating a ModelFeatureContainer."""
        parser = ModelParser()
        container = ModelFeatureContainer(parser)
        
        assert container.zz_parser is parser
        assert hasattr(container, 'ZZ_FEATURE_KEYS')
    
    def test_feature_container_iteration(self):
        """Test that container is iterable."""
        parser = ModelParser()
        container = ModelFeatureContainer(parser)
        
        # Should be able to iterate even when empty
        features = list(container)
        assert features == []
    
    def test_feature_container_contains(self):
        """Test that container supports 'in' operator."""
        parser = ModelParser()
        container = ModelFeatureContainer(parser)
        
        # Test with empty container
        assert "nonexistent" not in container
    
    def test_invalid_parser_type(self):
        """Test that non-ModelParser raises assertion error."""
        with self.assertRaises(AssertionError):
            ModelFeatureContainer("not_a_parser")
        
        with self.assertRaises(AssertionError):
            ModelFeatureContainer(None)


class TestModelFeatureXMLSerialization(unittest.TestCase):
    """Test XML serialization methods."""
    
    def test_to_type_xml(self):
        """Test to_type_xml method."""
        feature = ModelFeature(
            name="test_feature",
            desc="Test description"
        )
        
        xml_obj = feature.to_type_xml()
        assert xml_obj is not None
        # The actual XML object properties would depend on the model_type module
        # We can test that the method runs without error
    
    def test_to_instance_xml(self):
        """Test to_instance_xml method."""
        feature = ModelFeature(
            name="test_feature",
            desc="Test description",
            value=True
        )
        
        xml_obj = feature.to_instance_xml()
        assert xml_obj is not None
        # The actual XML object properties would depend on the model_inst module
        # We can test that the method runs without error


class TestModelFeatureEdgeCases(unittest.TestCase):
    """Test edge cases for ModelFeature."""
    
    def test_empty_string_name(self):
        """Test creating ModelFeature with empty string name."""
        feature = ModelFeature(name="")
        assert feature.name == ""
    
    def test_empty_string_desc(self):
        """Test creating ModelFeature with empty string description."""
        feature = ModelFeature(name="test", desc="")
        assert feature.desc == ""
    
    def test_unicode_strings(self):
        """Test ModelFeature with unicode strings."""
        feature = ModelFeature(
            name=u"unicode_feature",
            desc=u"Unicode description with special chars: äöü"
        )
        
        assert feature.name == u"unicode_feature"
        assert feature.desc == u"Unicode description with special chars: äöü"
    
    def test_long_strings(self):
        """Test ModelFeature with very long strings."""
        long_name = "feature_" * 100
        long_desc = "This is a very long description. " * 50
        
        feature = ModelFeature(name=long_name, desc=long_desc)
        
        assert feature.name == long_name
        assert feature.desc == long_desc
    
    def test_special_characters_in_strings(self):
        """Test ModelFeature with special characters."""
        feature = ModelFeature(
            name="feature-with_special.chars@domain",
            desc="Description with newlines\nand tabs\tand quotes'\"and symbols!@#$%"
        )
        
        assert "special" in feature.name
        assert "newlines" in feature.desc
    
    def test_multiple_parser_updates(self):
        """Test multiple parser update calls."""
        feature = ModelFeature(name="test_feature")
        
        update_calls = []
        def mock_parser_update(name, value):
            update_calls.append((name, value))
        
        feature.register_parser(mock_parser_update)
        
        # Multiple value changes should trigger multiple updates
        for i in range(5):
            feature.value = i % 2 == 0  # Alternate between True and False
        
        assert len(update_calls) == 5


if __name__ == "__main__":
    unittest.main(verbosity=2)
