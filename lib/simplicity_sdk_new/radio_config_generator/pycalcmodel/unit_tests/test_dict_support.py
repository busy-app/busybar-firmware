"""
Simple test script to verify dictionary support in ModelVariable
"""
import sys
import os

# Add the Package directory to path so pycalcmodel is importable
package_dir = os.path.join(os.path.dirname(__file__), '..', '..')
sys.path.insert(0, package_dir)

from pycalcmodel.core.variable import ModelVariable, ModelVariableFormat
from pycalcmodel.core.common import get_xml_str_values, cast_value_from_xml, get_dummy_var_value


def test_dict_variable_creation():
    """Test creating a dict variable"""
    var = ModelVariable('test_dict', dict, False, desc='Test dictionary variable')
    assert var.var_type is dict
    print("Dict variable creation works")

def test_dict_value_assignment():
    """Test assigning dict values"""
    var = ModelVariable('test_dict', dict, False)
    test_dict = {'key1': 'value1', 'key2': 42}
    var.value = test_dict
    assert var.value == test_dict
    print("Dict value assignment works")

def test_dict_array_assignment():
    """Test assigning array of dicts"""
    var = ModelVariable('test_dict_array', dict, True)
    test_dicts = [{'a': 1}, {'b': 2}, {'c': 3}]
    var.value = test_dicts
    assert var.value == test_dicts
    print("Dict array assignment works")

def test_dict_xml_serialization():
    """Test dict to XML string conversion"""
    var = ModelVariable('test_dict', dict, False)
    test_dict = {'key1': 'value1', 'key2': 42}
    xml_str = get_xml_str_values(var.is_array, test_dict)
    expected = [str(test_dict)]
    assert xml_str == expected
    print("Dict XML serialization works")

def test_dict_xml_deserialization():
    """Test XML string to dict conversion"""
    var = ModelVariable('test_dict', dict, False)
    xml_value = ["{'key1': 'value1', 'key2': 42}"]
    result = cast_value_from_xml(var, xml_value)
    expected = {'key1': 'value1', 'key2': 42}
    assert result == expected
    print("Dict XML deserialization works")

def test_dict_array_xml_round_trip():
    """Test array of dicts XML round trip"""
    var = ModelVariable('test_dict_array', dict, True)
    original = [{'a': 1}, {'b': 2}]
    
    # Serialize
    xml_strs = get_xml_str_values(var.is_array, original)
    
    # Deserialize
    result = cast_value_from_xml(var, xml_strs)
    
    assert result == original
    print("Dict array XML round trip works")

def test_dict_dummy_value():
    """Test dummy value for dict type"""
    var = ModelVariable('test_dict', dict, False)
    dummy = get_dummy_var_value(var)
    assert dummy == {}
    print("Dict dummy value works")

def test_type_str():
    """Test _get_type_str returns 'dict'"""
    var = ModelVariable('test_dict', dict, False)
    assert var._get_type_str() == 'dict'
    print("Dict type string works")

if __name__ == '__main__':
    print("Testing dictionary support in ModelVariable...")
    
    try:
        test_dict_variable_creation()
        test_dict_value_assignment()
        test_dict_array_assignment()
        test_dict_xml_serialization()
        test_dict_xml_deserialization()
        test_dict_array_xml_round_trip()
        test_dict_dummy_value()
        test_type_str()
        
        print("\nAll tests passed! Dictionary support is working correctly.")
        
    except Exception as e:
        print(f"\nTest failed: {e}")
        raise
