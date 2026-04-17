"""
Comprehensive tests for pycalcmodel.core.variable module.

Tests cover ModelVariable creation, type handling, value assignment,
XML serialization/deserialization, and all supported data types.
"""

import sys
import unittest
from enum import Enum, IntEnum
from collections import OrderedDict

# Add the Package directory to the path for imports
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

from pycalcmodel.core.variable import (
    ModelVariable, ModelVariableFormat, ModelVariableInvalidValueType,
    ModelVariableEmptyValue, ModelVariableCannotForceValue,
    ModelVariableWriteAccess, CreateModelVariableEnum
)
from pycalcmodel.core.common import cast_value_from_xml, get_dummy_var_value, get_xml_str_values


class SampleEnum(Enum):
    OPTION_A = 1
    OPTION_B = 2
    OPTION_C = 3


class SampleIntEnum(IntEnum):
    LOW = 0
    MEDIUM = 5
    HIGH = 10


class TestModelVariableCreation(unittest.TestCase):
    """Test ModelVariable creation and basic properties."""
    
    def test_create_int_variable(self):
        """Test creating an integer variable."""
        var = ModelVariable(name="test_int", var_type=int, is_array=False)
        assert var.name == "test_int"
        assert var.var_type == int
        assert not var.is_array
        assert var.value is None  # Initially no value
    
    def test_create_float_variable(self):
        """Test creating a float variable."""
        var = ModelVariable(name="test_float", var_type=float, is_array=False)
        assert var.name == "test_float"
        assert var.var_type == float
        assert var.value is None
    
    def test_create_bool_variable(self):
        """Test creating a boolean variable."""
        var = ModelVariable(name="test_bool", var_type=bool, is_array=False)
        assert var.name == "test_bool"
        assert var.var_type == bool
        assert var.value is None
    
    def test_create_str_variable(self):
        """Test creating a string variable."""
        var = ModelVariable(name="test_str", var_type=str, is_array=False)
        assert var.name == "test_str"
        assert var.var_type == str
        assert var.value is None
    
    def test_create_complex_variable(self):
        """Test creating a complex variable."""
        var = ModelVariable(name="test_complex", var_type=complex, is_array=False)
        assert var.name == "test_complex"
        assert var.var_type == complex
        assert var.value is None
    
    def test_create_dict_variable(self):
        """Test creating a dictionary variable."""
        var = ModelVariable(name="test_dict", var_type=dict, is_array=False)
        assert var.name == "test_dict"
        assert var.var_type == dict
        assert var.value is None
    
    def test_create_enum_variable(self):
        """Test creating an enum variable."""
        var = ModelVariable(name="test_enum", var_type=Enum, is_array=False)
        var.var_enum = SampleEnum
        assert var.name == "test_enum"
        assert var.var_type == Enum
        assert var.var_enum == SampleEnum
        assert var.value is None
    
    def test_create_array_variable(self):
        """Test creating an array variable."""
        var = ModelVariable(name="test_array", var_type=int, is_array=True)
        assert var.name == "test_array"
        assert var.var_type == int
        assert var.value is None
        assert var.is_array is True


class TestModelVariableValueAssignment(unittest.TestCase):
    """Test value assignment and validation."""
    
    def test_assign_valid_int_value(self):
        """Test assigning valid integer values."""
        var = ModelVariable(name="test", var_type=int, is_array=False)
        var.value = 100
        assert var.value == 100
    
    def test_assign_valid_float_value(self):
        """Test assigning valid float values."""
        var = ModelVariable(name="test", var_type=float, is_array=False)
        var.value = 99.5
        assert var.value == 99.5
    
    def test_assign_valid_bool_value(self):
        """Test assigning valid boolean values."""
        var = ModelVariable(name="test", var_type=bool, is_array=False)
        var.value = True
        assert var.value is True
    
    def test_assign_valid_str_value(self):
        """Test assigning valid string values."""
        var = ModelVariable(name="test", var_type=str, is_array=False)
        var.value = "new_value"
        assert var.value == "new_value"
    
    def test_assign_valid_dict_value(self):
        """Test assigning valid dictionary values."""
        var = ModelVariable(name="test", var_type=dict, is_array=False)
        new_dict = {"a": 1, "b": [1, 2, 3], "c": {"nested": True}}
        var.value = new_dict
        assert var.value == new_dict
    
    def test_assign_valid_enum_value(self):
        """Test assigning valid enum values."""
        var = ModelVariable(name="test", var_type=Enum, is_array=False)
        var.var_enum = SampleEnum
        var.value = SampleEnum.OPTION_B
        assert var.value == SampleEnum.OPTION_B
    
    def test_assign_valid_array_value(self):
        """Test assigning valid array values."""
        var = ModelVariable(name="test", var_type=int, is_array=True)
        var.value = [10, 20, 30]
        assert var.value == [10, 20, 30]


class TestModelVariableFormat(unittest.TestCase):
    """Test ModelVariable format property."""
    
    def test_default_format(self):
        """Test default format is HEX."""
        var = ModelVariable(name="test", var_type=int, is_array=False)
        assert var.format == ModelVariableFormat.HEX
    
    def test_set_decimal_format(self):
        """Test setting DECIMAL format."""
        var = ModelVariable(name="test", var_type=int, is_array=False)
        var.format = ModelVariableFormat.DECIMAL
        assert var.format == ModelVariableFormat.DECIMAL
    
    def test_set_ascii_format(self):
        """Test setting ASCII format."""
        var = ModelVariable(name="test", var_type=str, is_array=False)
        var.format = ModelVariableFormat.ASCII
        assert var.format == ModelVariableFormat.ASCII


class TestModelVariableDummyValues(unittest.TestCase):
    """Test dummy value generation."""
    
    def test_get_dummy_int_value(self):
        """Test getting dummy value for int."""
        var = ModelVariable(name="test", var_type=int, is_array=False)
        dummy_value = get_dummy_var_value(var)
        assert dummy_value == 0
    
    def test_get_dummy_float_value(self):
        """Test getting dummy value for float."""
        var = ModelVariable(name="test", var_type=float, is_array=False)
        dummy_value = get_dummy_var_value(var)
        assert dummy_value == 0.0
    
    def test_get_dummy_bool_value(self):
        """Test getting dummy value for bool."""
        var = ModelVariable(name="test", var_type=bool, is_array=False)
        dummy_value = get_dummy_var_value(var)
        assert dummy_value is False
    
    def test_get_dummy_str_value(self):
        """Test getting dummy value for str."""
        var = ModelVariable(name="test", var_type=str, is_array=False)
        dummy_value = get_dummy_var_value(var)
        assert dummy_value == ""
    
    def test_get_dummy_complex_value(self):
        """Test getting dummy value for complex."""
        var = ModelVariable(name="test", var_type=complex, is_array=False)
        dummy_value = get_dummy_var_value(var)
        assert dummy_value == 0j
    
    def test_get_dummy_dict_value(self):
        """Test getting dummy value for dict."""
        var = ModelVariable(name="test", var_type=dict, is_array=False)
        dummy_value = get_dummy_var_value(var)
        assert dummy_value == {}
    
    def test_get_dummy_enum_value(self):
        """Test getting dummy value for enum."""
        var = ModelVariable(name="test", var_type=Enum, is_array=False)
        var.var_enum = SampleEnum
        dummy_value = get_dummy_var_value(var)
        # Should return the value of the first enum member
        assert dummy_value == 1  # SampleEnum.OPTION_A.value


class TestXMLSerialization(unittest.TestCase):
    """Test XML serialization and deserialization."""
    
    def test_get_xml_str_values_single(self):
        """Test getting XML string values for single value."""
        result = get_xml_str_values(is_array=False, value=42)
        assert result == ["42"]
    
    def test_get_xml_str_values_array(self):
        """Test getting XML string values for array."""
        result = get_xml_str_values(is_array=True, value=[1, 2, 3])
        assert result == ["1", "2", "3"]
    
    def test_get_xml_str_values_none(self):
        """Test getting XML string values for None."""
        result = get_xml_str_values(is_array=False, value=None)
        assert result is None
    
    def test_cast_int_from_xml(self):
        """Test casting int from XML."""
        var = ModelVariable(name="test", var_type=int, is_array=False)
        result = cast_value_from_xml(var, ["42"])
        assert result == 42
    
    def test_cast_float_from_xml(self):
        """Test casting float from XML."""
        var = ModelVariable(name="test", var_type=float, is_array=False)
        result = cast_value_from_xml(var, ["3.14"])
        assert result == 3.14
    
    def test_cast_bool_from_xml(self):
        """Test casting bool from XML."""
        var = ModelVariable(name="test", var_type=bool, is_array=False)
        result = cast_value_from_xml(var, ["True"])
        assert result is True
        
        result = cast_value_from_xml(var, ["False"])
        assert result is False
    
    def test_cast_str_from_xml(self):
        """Test casting str from XML."""
        var = ModelVariable(name="test", var_type=str, is_array=False)
        result = cast_value_from_xml(var, ["hello"])
        assert result == "hello"
    
    def test_cast_dict_from_xml(self):
        """Test casting dict from XML."""
        var = ModelVariable(name="test", var_type=dict, is_array=False)
        result = cast_value_from_xml(var, ["{'key': 'value', 'num': 42}"])
        assert result == {'key': 'value', 'num': 42}
    
    def test_cast_complex_from_xml(self):
        """Test casting complex from XML."""
        var = ModelVariable(name="test", var_type=complex, is_array=False)
        result = cast_value_from_xml(var, ["(1+2j)"])
        assert result == 1+2j
    
    def test_cast_enum_from_xml(self):
        """Test casting enum from XML."""
        var = ModelVariable(name="test", var_type=Enum, is_array=False)
        var.var_enum = SampleEnum
        result = cast_value_from_xml(var, ["SampleEnum.OPTION_B"])
        assert result == SampleEnum.OPTION_B
    
    def test_cast_array_from_xml(self):
        """Test casting array from XML."""
        var = ModelVariable(name="test", var_type=int, is_array=True)
        result = cast_value_from_xml(var, ["1", "2", "3"])
        assert result == [1, 2, 3]
    
    def test_cast_empty_xml_value(self):
        """Test casting empty XML value."""
        var = ModelVariable(name="test", var_type=int, is_array=False)
        result = cast_value_from_xml(var, [])
        assert result is None
        
        result = cast_value_from_xml(var, [None])
        assert result is None


class TestModelVariableExceptions(unittest.TestCase):
    """Test exception handling."""
    
    def test_invalid_enum_xml_format(self):
        """Test invalid enum XML format raises ValueError."""
        var = ModelVariable(name="test", var_type=Enum, is_array=False)
        var.var_enum = SampleEnum
        
        # Missing dot separator
        with self.assertRaisesRegex(ValueError, "Invalid enum XML value"):
            cast_value_from_xml(var, ["OPTION_A"])
    
    def test_invalid_enum_name(self):
        """Test invalid enum name raises ValueError."""
        var = ModelVariable(name="test", var_type=Enum, is_array=False)
        var.var_enum = SampleEnum
        
        # Wrong enum name
        with self.assertRaisesRegex(ValueError, "Invalid enum name"):
            cast_value_from_xml(var, ["WrongEnum.OPTION_A"])
    
    def test_invalid_enum_member(self):
        """Test invalid enum member raises ValueError."""
        var = ModelVariable(name="test", var_type=Enum, is_array=False)
        var.var_enum = SampleEnum
        
        # Non-existent member
        with self.assertRaisesRegex(ValueError, "Invalid enum member"):
            cast_value_from_xml(var, ["SampleEnum.INVALID_OPTION"])


class TestModelVariableEnumCreation(unittest.TestCase):
    """Test enum creation utility."""
    
    def test_create_model_variable_enum(self):
        """Test creating enum with CreateModelVariableEnum."""
        # Test basic enum creation
        member_data = [
            ("FIRST", 0, "First option"),
            ("SECOND", 1, "Second option"),
            ("THIRD", 2, "Third option")
        ]
        TestDynamicEnum = CreateModelVariableEnum('TestDynamicEnum', 
                                                 'Test dynamic enum description', 
                                                 member_data)
        
        assert hasattr(TestDynamicEnum, 'FIRST')
        assert hasattr(TestDynamicEnum, 'SECOND')
        assert hasattr(TestDynamicEnum, 'THIRD')
        assert TestDynamicEnum.FIRST == 0
        assert TestDynamicEnum.SECOND == 1
        assert TestDynamicEnum.THIRD == 2
        
        # Test description methods
        assert hasattr(TestDynamicEnum, 'getDesc')
        assert TestDynamicEnum.getDesc() == 'Test dynamic enum description'
        assert hasattr(TestDynamicEnum, 'getMemberDesc')
        assert TestDynamicEnum.getMemberDesc('FIRST') == 'First option'
        assert TestDynamicEnum.getMemberDesc('SECOND') == 'Second option'
        assert TestDynamicEnum.getMemberDesc('THIRD') == 'Third option'


if __name__ == "__main__":
    unittest.main(verbosity=2)
