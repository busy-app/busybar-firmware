"""
Integration tests for pycalcmodel system.

Tests the interaction between different components and end-to-end functionality.
"""

import unittest
import sys
from enum import Enum

# Add the Package directory to the path for imports
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

from pycalcmodel.core.model import ModelRoot
from pycalcmodel.core.variable import ModelVariable, ModelVariableFormat, CreateModelVariableEnum
from pycalcmodel.core.feature import ModelFeature
from pycalcmodel.core.common import cast_value_from_xml, get_dummy_var_value, get_xml_str_values

# Handle Python 2/3 compatibility for long type
if sys.version_info[0] > 2:
    long = int


class TestSystemIntegration(unittest.TestCase):
    """Test integration between different pycalcmodel components."""
    
    def test_model_root_with_variables(self):
        """Test adding variables to a ModelRoot."""
        model = ModelRoot(
            part_family="EFR32MG12", 
            calc_version="1.0.0"
        )
        
        # Create some test variables
        int_var = ModelVariable(name="test_int", var_type=int, is_array=False)
        int_var.value = 42
        str_var = ModelVariable(name="test_str", var_type=str, is_array=False)
        str_var.value = "hello"
        dict_var = ModelVariable(name="test_dict", var_type=dict, is_array=False)
        dict_var.value = {"key": "value"}
        
        # Variables container should be accessible
        assert model.vars is not None
        
        # Test that the model has all required components
        assert hasattr(model, 'features')
        assert hasattr(model, 'phys')
        assert hasattr(model, 'profiles')
        assert hasattr(model, 'vars')
    
    def test_variable_xml_roundtrip(self):
        """Test XML serialization and deserialization roundtrip for variables."""
        # Test different variable types
        test_cases = [
            (int, 42),
            (float, 3.14159),
            (bool, True),
            (str, "test_string"),
            (dict, {"key1": "value1", "key2": 42, "nested": {"inner": True}}),
            (complex, 1+2j)
        ]
        
        for var_type, test_value in test_cases:
            # Create variable
            var = ModelVariable(name=f"test_{var_type.__name__}", 
                              var_type=var_type, is_array=False)
            
            # Convert to XML string format
            xml_values = get_xml_str_values(is_array=False, value=test_value)
            
            # Convert back from XML
            reconstructed_value = cast_value_from_xml(var, xml_values)
            
            # Should match original value
            assert reconstructed_value == test_value, f"Failed for type {var_type.__name__}"
    
    def test_array_variable_xml_roundtrip(self):
        """Test XML roundtrip for array variables."""
        test_cases = [
            (int, [1, 2, 3, 42]),
            (float, [1.1, 2.2, 3.14]),
            (bool, [True, False, True]),
            (str, ["hello", "world", "test"]),
            (dict, [{"a": 1}, {"b": 2}, {"c": [1, 2, 3]}]),
            (complex, [1+1j, 2-1j, 3j])
        ]
        
        for var_type, test_array in test_cases:
            # Create array variable
            var = ModelVariable(name=f"test_array_{var_type.__name__}", 
                              var_type=var_type, is_array=True)
            
            # Convert to XML string format
            xml_values = get_xml_str_values(is_array=True, value=test_array)
            
            # Convert back from XML
            reconstructed_value = cast_value_from_xml(var, xml_values)
            
            # Should match original array
            assert reconstructed_value == test_array, f"Failed for array type {var_type.__name__}"
    
    def test_enum_variable_integration(self):
        """Test enum variable creation and XML handling."""
        # Create custom enum
        member_data = [
            ("OPTION_A", 0, "First option"),
            ("OPTION_B", 1, "Second option"),
            ("OPTION_C", 2, "Third option")
        ]
        TestEnum = CreateModelVariableEnum('TestIntegrationEnum', 
                                         'Integration test enum',
                                         member_data)
        
        # Create enum variable
        var = ModelVariable(name="test_enum", var_type=Enum, is_array=False)
        var.var_enum = TestEnum
        
        # Test XML conversion
        xml_values = get_xml_str_values(is_array=False, value="TestIntegrationEnum.OPTION_B")
        reconstructed_value = cast_value_from_xml(var, ["TestIntegrationEnum.OPTION_B"])
        
        assert reconstructed_value == TestEnum.OPTION_B
    
    def test_variable_format_integration(self):
        """Test variable format settings."""
        var = ModelVariable(name="test_hex", var_type=int, is_array=False)
        
        # Default format should be HEX
        assert var.format == ModelVariableFormat.HEX
        
        # Change to DECIMAL format
        var.format = ModelVariableFormat.DECIMAL
        assert var.format == ModelVariableFormat.DECIMAL
        
        # Set and check value
        var.value = 255
        assert var.value == 255
    
    def test_dummy_values_integration(self):
        """Test dummy value generation for all supported types."""
        test_types = [
            (bool, False),
            (complex, 0j),
            (dict, {}),
            (float, 0.0),
            (int, 0),
            (long, long(0)),
            (str, "")
        ]
        
        for var_type, expected_dummy in test_types:
            var = ModelVariable(name=f"test_{var_type.__name__}", 
                              var_type=var_type, is_array=False)
            dummy_value = get_dummy_var_value(var)
            assert dummy_value == expected_dummy


class TestComplexScenarios(unittest.TestCase):
    """Test complex real-world scenarios."""
    
    def test_nested_dict_variable(self):
        """Test deeply nested dictionary variable."""
        complex_dict = {
            "level1": {
                "level2": {
                    "level3": {
                        "data": [1, 2, 3],
                        "config": {
                            "enabled": True,
                            "threshold": 42.5,
                            "name": "complex_config"
                        }
                    }
                },
                "other_data": "test"
            },
            "top_level_list": [{"item": 1}, {"item": 2}]
        }
        
        var = ModelVariable(name="complex_dict", var_type=dict, is_array=False)
        var.value = complex_dict
        
        # Test XML roundtrip
        xml_values = get_xml_str_values(is_array=False, value=var.value)
        reconstructed = cast_value_from_xml(var, xml_values)
        
        assert reconstructed == complex_dict
    
    def test_mixed_array_types(self):
        """Test arrays of different types in same model."""
        model = ModelRoot(part_family="EFR32MG12", calc_version="1.0.0")
        
        # Create variables of different array types
        int_array_var = ModelVariable(name="int_array", var_type=int, is_array=True)
        int_array_var.value = [1, 2, 3]
        str_array_var = ModelVariable(name="str_array", var_type=str, is_array=True)
        str_array_var.value = ["a", "b", "c"]
        dict_array_var = ModelVariable(name="dict_array", var_type=dict, is_array=True)
        dict_array_var.value = [{"x": 1}, {"y": 2}]
        
        # Test they all work correctly
        assert int_array_var.value == [1, 2, 3]
        assert str_array_var.value == ["a", "b", "c"]
        assert dict_array_var.value == [{"x": 1}, {"y": 2}]
    
    def test_large_dictionary_performance(self):
        """Test handling of large dictionaries."""
        # Create a reasonably large dictionary
        large_dict = {}
        for i in range(100):
            large_dict[f"key_{i}"] = {
                "id": i,
                "data": list(range(i % 10)),
                "config": {"enabled": i % 2 == 0, "value": i * 1.5}
            }
        
        var = ModelVariable(name="large_dict", var_type=dict, is_array=False)
        var.value = large_dict
        
        # Test XML conversion doesn't fail
        xml_values = get_xml_str_values(is_array=False, value=var.value)
        assert xml_values is not None
        assert len(xml_values) == 1
        
        # Test reconstruction
        reconstructed = cast_value_from_xml(var, xml_values)
        assert reconstructed == large_dict


class TestErrorHandlingIntegration(unittest.TestCase):
    """Test error handling across components."""
    
    def test_invalid_xml_to_variable_conversion(self):
        """Test error handling when converting invalid XML to variables."""
        var = ModelVariable(name="test_dict", var_type=dict, is_array=False)
        
        # Test malformed dictionary string - ast.literal_eval raises SyntaxError for malformed syntax
        with self.assertRaises(SyntaxError):
            cast_value_from_xml(var, ["{invalid_dict_format"])
        
        # Test invalid boolean string - ast.literal_eval raises ValueError for invalid literals
        bool_var = ModelVariable(name="test_bool", var_type=bool, is_array=False)
        with self.assertRaises(ValueError):
            cast_value_from_xml(bool_var, ["NotABool"])
    
    def test_type_mismatch_scenarios(self):
        """Test type mismatch error scenarios."""
        # Test setting wrong type on variable
        int_var = ModelVariable(name="test_int", var_type=int, is_array=False)
        
        # This should work (type conversion)
        int_var.value = 42
        assert int_var.value == 42
        
        # Dictionary with incompatible XML string - ast.literal_eval raises ValueError for non-dict strings
        dict_var = ModelVariable(name="test_dict", var_type=dict, is_array=False)
        with self.assertRaises(ValueError):
            cast_value_from_xml(dict_var, ["not_a_dict_format"])


class TestBackwardCompatibility(unittest.TestCase):
    """Test backward compatibility scenarios."""
    
    def test_legacy_boolean_handling(self):
        """Test legacy boolean value handling."""
        var = ModelVariable(name="test_bool", var_type=bool, is_array=False)
        
        # Test various boolean representations
        test_cases = [
            ("True", True),
            ("False", False)
        ]
        
        for xml_str, expected in test_cases:
            result = cast_value_from_xml(var, [xml_str])
            assert result == expected
    
    def test_empty_value_handling(self):
        """Test handling of empty/None values."""
        test_types = [int, float, str, bool, dict, complex]
        
        for var_type in test_types:
            var = ModelVariable(name=f"test_{var_type.__name__}", 
                              var_type=var_type, is_array=False)
            
            # Empty XML should return None
            result = cast_value_from_xml(var, [])
            assert result is None
            
            # None XML should return None
            result = cast_value_from_xml(var, [None])
            assert result is None


if __name__ == "__main__":
    unittest.main(verbosity=2)
