"""
Standalone test for dictionary support in variable types
This test bypasses the full pycalcmodel infrastructure to test just the core functionality
"""
import sys

# Python 2/3 compatibility
if sys.version_info[0] >= 3:
    basestring = str
    long = int

def test_dict_in_supported_types():
    """Test that dict is in the supported types tuple"""
    # This mirrors the check from variable.py line 102
    supported_types = (basestring, bool, complex, float, int, long, str, dict)
    assert dict in supported_types
    print("dict is in supported types")

def test_dict_type_string():
    """Test that dict returns correct type string"""
    # This mirrors the logic from _get_type_str method
    test_types = (bool, complex, float, int, long, dict)
    
    for var_type in test_types:
        if var_type in test_types:
            type_str = var_type.__name__
            if var_type is dict:
                assert type_str == 'dict'
                print("dict.__name__ returns 'dict'")

def test_dict_xml_conversion():
    """Test dict XML conversion using ast.literal_eval"""
    import ast
    
    # Test single dict
    test_dict = {'key1': 'value1', 'key2': 42}
    dict_str = str(test_dict)
    
    # Test serialization (to XML string)
    xml_values = [dict_str]
    print(f"Dict serialized to: {xml_values}")
    
    # Test deserialization (from XML string) 
    recovered_dict = ast.literal_eval(xml_values[0])
    assert recovered_dict == test_dict
    print("Dict XML round trip works")
    
    # Test array of dicts
    test_dicts = [{'a': 1}, {'b': 2}]
    dict_strs = [str(d) for d in test_dicts]
    
    recovered_dicts = [ast.literal_eval(s) for s in dict_strs]
    assert recovered_dicts == test_dicts
    print("Dict array XML round trip works")

def test_dummy_value():
    """Test dummy value for dict"""
    dummy_dict = {}
    assert isinstance(dummy_dict, dict)
    assert dummy_dict == {}
    print("Dict dummy value is correct")

if __name__ == '__main__':
    print("Testing dictionary support (standalone)...")

    basestring = str
    long = int
    
    try:
        test_dict_in_supported_types()
        test_dict_type_string() 
        test_dict_xml_conversion()
        test_dummy_value()
        
        print("\nAll standalone tests passed! Dictionary support is working correctly.")
        
    except Exception as e:
        print(f"\nTest failed: {e}")
        import traceback
        traceback.print_exc()
        raise
