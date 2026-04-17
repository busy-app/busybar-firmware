"""
Comprehensive tests for pycalcmodel.core.common module.

Tests cover XML value casting, dummy value generation, and utility functions.
"""

import unittest
import sys
from enum import Enum

# Add the Package directory to the path for imports
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

from pycalcmodel.core.variable import ModelVariable
from pycalcmodel.core.common import (
    cast_value_from_xml, get_dummy_var_value, get_xml_str_values, DUMMY_VAR_VALUE
)

# Python 2/3 compatibility
try:
    long
except NameError:
    long = int


class TestDummyVarValue(unittest.TestCase):
    """Test DUMMY_VAR_VALUE dictionary and get_dummy_var_value function."""
    
    def test_dummy_var_value_constants(self):
        """Test that DUMMY_VAR_VALUE contains expected constants."""
        expected_constants = {
            bool: False,
            complex: 0j,
            dict: {},
            float: 0.0,
            int: 0,
            long: long(0),
            str: '',
        }
        
        for var_type, expected_value in expected_constants.items():
            assert var_type in DUMMY_VAR_VALUE
            assert DUMMY_VAR_VALUE[var_type] == expected_value
    
    def test_get_dummy_var_value_basic_types(self):
        """Test get_dummy_var_value for basic types."""
        test_cases = [
            (bool, False),
            (int, 0),
            (float, 0.0),
            (str, ''),
            (complex, 0j),
            (dict, {}),
            (long, long(0)),
        ]
        
        for var_type, expected_value in test_cases:
            var = ModelVariable(name="test", var_type=var_type, is_array=False)
            result = get_dummy_var_value(var)
            assert result == expected_value

    def test_get_dummy_var_value_enum(self):
        """Test get_dummy_var_value for enum types."""
        class TestEnum(Enum):
            FIRST = 100
            SECOND = 200
            THIRD = 300
        
        var = ModelVariable(name="test", var_type=Enum, is_array=False)
        var.var_enum = TestEnum
        result = get_dummy_var_value(var)
        # Should return the value of the first enum member
        assert result == 100  # TestEnum.FIRST.value


class TestXMLStringValues(unittest.TestCase):
    """Test get_xml_str_values function."""
    
    def test_single_value_not_array(self):
        """Test converting single value to XML string (not array)."""
        test_cases = [
            (42, ['42']),
            (3.14, ['3.14']),
            ('hello', ['hello']),
            (True, ['True']),
            (1+2j, ['(1+2j)']),
        ]
        
        for value, expected in test_cases:
            result = get_xml_str_values(False, value)
            assert result == expected
    
    def test_array_values(self):
        """Test converting array values to XML strings."""
        test_cases = [
            ([1, 2, 3], ['1', '2', '3']),
            ([1.0, 2.5], ['1.0', '2.5']),
            (['a', 'b'], ['a', 'b']),
            ([True, False], ['True', 'False']),
        ]
        
        for value, expected in test_cases:
            result = get_xml_str_values(True, value)
            assert result == expected
    
    def test_none_value(self):
        """Test handling of None values."""
        result = get_xml_str_values(False, None)
        assert result is None
        
        result = get_xml_str_values(True, None)
        assert result is None


class TestCastValueFromXML(unittest.TestCase):
    """Test cast_value_from_xml function."""
    
    def test_cast_none_values(self):
        """Test casting None and empty values."""
        var = ModelVariable(name="test", var_type=int, is_array=False)
        
        # Test empty list
        result = cast_value_from_xml(var, [])
        assert result is None
        
        # Test list with None
        result = cast_value_from_xml(var, [None])
        assert result is None
    
    def test_cast_int_values(self):
        """Test casting integer values from XML."""
        var = ModelVariable(name="test", var_type=int, is_array=False)
        result = cast_value_from_xml(var, ['42'])
        assert result == 42
        
        # Test array
        var_array = ModelVariable(name="test_array", var_type=int, is_array=True)
        result = cast_value_from_xml(var_array, ['1', '2', '3'])
        assert result == [1, 2, 3]
    
    def test_cast_float_values(self):
        """Test casting float values from XML."""
        var = ModelVariable(name="test", var_type=float, is_array=False)
        result = cast_value_from_xml(var, ['3.14'])
        assert result == 3.14
        
        # Test array
        var_array = ModelVariable(name="test_array", var_type=float, is_array=True)
        result = cast_value_from_xml(var_array, ['1.0', '2.5'])
        assert result == [1.0, 2.5]
    
    def test_cast_str_values(self):
        """Test casting string values from XML."""
        var = ModelVariable(name="test", var_type=str, is_array=False)
        result = cast_value_from_xml(var, ['hello'])
        assert result == 'hello'
        
        # Test array
        var_array = ModelVariable(name="test_array", var_type=str, is_array=True)
        result = cast_value_from_xml(var_array, ['a', 'b', 'c'])
        assert result == ['a', 'b', 'c']
    
    def test_cast_long_values(self):
        """Test casting long values from XML."""
        var = ModelVariable(name="test", var_type=long, is_array=False)
        result = cast_value_from_xml(var, ['123456789'])
        assert result == long(123456789)
        
        # Test array
        var_array = ModelVariable(name="test_array", var_type=long, is_array=True)
        result = cast_value_from_xml(var_array, ['1', '2'])
        assert result == [long(1), long(2)]
    
    def test_cast_complex_values(self):
        """Test casting complex values from XML."""
        var = ModelVariable(name="test", var_type=complex, is_array=False)
        result = cast_value_from_xml(var, ['(1+2j)'])
        assert result == (1+2j)
        
        # Test array
        var_array = ModelVariable(name="test_array", var_type=complex, is_array=True)
        result = cast_value_from_xml(var_array, ['(1+0j)', '(0+1j)'])
        assert result == [(1+0j), (0+1j)]
    
    def test_cast_bool_values(self):
        """Test casting boolean values from XML using ast.literal_eval."""
        var = ModelVariable(name="test", var_type=bool, is_array=False)
        result = cast_value_from_xml(var, ['True'])
        assert result == True
        
        result = cast_value_from_xml(var, ['False'])
        assert result == False
        
        # Test array
        var_array = ModelVariable(name="test_array", var_type=bool, is_array=True)
        result = cast_value_from_xml(var_array, ['True', 'False'])
        assert result == [True, False]
    
    def test_cast_dict_values(self):
        """Test casting dictionary values from XML using ast.literal_eval."""
        var = ModelVariable(name="test", var_type=dict, is_array=False)
        result = cast_value_from_xml(var, ["{'key': 'value'}"])
        assert result == {'key': 'value'}
        
        # Test array
        var_array = ModelVariable(name="test_array", var_type=dict, is_array=True)
        result = cast_value_from_xml(var_array, ["{'a': 1}", "{'b': 2}"])
        assert result == [{'a': 1}, {'b': 2}]
    
    def test_cast_enum_values(self):
        """Test casting enum values from XML."""
        class TestEnum(Enum):
            FIRST = 100
            SECOND = 200
        
        var = ModelVariable(name="test", var_type=Enum, is_array=False)
        var.var_enum = TestEnum
        
        result = cast_value_from_xml(var, ['TestEnum.FIRST'])
        assert result == TestEnum.FIRST
        
        result = cast_value_from_xml(var, ['TestEnum.SECOND'])
        assert result == TestEnum.SECOND
    
    def test_cast_array_with_list_notation(self):
        """Test casting array values with list notation in XML."""
        var = ModelVariable(name="test", var_type=int, is_array=True)
        result = cast_value_from_xml(var, ['[1, 2, 3]'])
        assert result == [1, 2, 3]
    
    def test_empty_xml_values(self):
        """Test handling of empty XML values."""
        var = ModelVariable(name="test", var_type=int, is_array=False)
        result = cast_value_from_xml(var, [])
        assert result is None


class TestCastValueFromXMLErrors(unittest.TestCase):
    """Test error handling in cast_value_from_xml function."""
    
    def test_invalid_var_parameter(self):
        """Test that non-ModelVariable var parameter raises assertion error."""
        with self.assertRaisesRegex(AssertionError, "var must be ModelVariable instance"):
            cast_value_from_xml("not_a_variable", ["42"])
    
    def test_invalid_xml_value_parameter(self):
        """Test that non-list xml_value parameter raises assertion error."""
        var = ModelVariable(name="test", var_type=int, is_array=False)
        with self.assertRaisesRegex(AssertionError, "xml_value should be a list"):
            cast_value_from_xml(var, "not_a_list")
    
    def test_invalid_enum_format(self):
        """Test invalid enum XML format raises ValueError."""
        class TestEnum(Enum):
            VALUE = 42
        
        var = ModelVariable(name="test", var_type=Enum, is_array=False)
        var.var_enum = TestEnum
        
        with self.assertRaisesRegex(ValueError, "Invalid enum XML value"):
            cast_value_from_xml(var, ['InvalidFormat'])
    
    def test_invalid_enum_name(self):
        """Test invalid enum name raises ValueError."""
        class TestEnum(Enum):
            VALUE = 42
        
        var = ModelVariable(name="test", var_type=Enum, is_array=False)
        var.var_enum = TestEnum
        
        with self.assertRaisesRegex(ValueError, "Invalid enum name"):
            cast_value_from_xml(var, ['WrongEnum.VALUE'])
    
    def test_invalid_enum_member(self):
        """Test invalid enum member raises ValueError."""
        class TestEnum(Enum):
            VALUE = 42
        
        var = ModelVariable(name="test", var_type=Enum, is_array=False)
        var.var_enum = TestEnum
        
        with self.assertRaisesRegex(ValueError, "Invalid enum member"):
            cast_value_from_xml(var, ['TestEnum.INVALID'])
    
    def test_unsupported_var_type(self):
        """Test unsupported variable type raises AssertionError during ModelVariable creation."""
        with self.assertRaisesRegex(AssertionError, "Unsupported class for var_type"):
            ModelVariable(name="test", var_type=list, is_array=False)  # list is not supported


class TestComprehensiveIntegration(unittest.TestCase):
    """Integration tests covering multiple functions together."""
    
    def test_roundtrip_xml_conversion(self):
        """Test roundtrip conversion: value -> XML -> value."""
        test_cases = [
            (int, False, 42),
            (float, False, 3.14),
            (str, False, 'hello'),
            (bool, False, True),
            (dict, False, {'key': 'value'}),
            (int, True, [1, 2, 3]),
            (str, True, ['a', 'b', 'c']),
        ]
        
        for var_type, is_array, original_value in test_cases:
            var = ModelVariable(name="test", var_type=var_type, is_array=is_array)
            
            # Convert to XML strings
            xml_strings = get_xml_str_values(is_array, original_value)
            
            # Convert back from XML
            restored_value = cast_value_from_xml(var, xml_strings)
            
            assert restored_value == original_value
    
    def test_enum_roundtrip(self):
        """Test enum roundtrip conversion."""
        class TestEnum(Enum):
            FIRST = 100
            SECOND = 'hello'
            THIRD = [1, 2, 3]
        
        var = ModelVariable(name="test", var_type=Enum, is_array=False)
        var.var_enum = TestEnum
        
        for enum_value in TestEnum:
            # Convert to XML string (manual since get_xml_str_values doesn't handle enums)
            xml_strings = [f"{TestEnum.__name__}.{enum_value.name}"]
            
            # Convert back from XML
            restored_value = cast_value_from_xml(var, xml_strings)
            
            assert restored_value == enum_value


if __name__ == '__main__':
    unittest.main()
