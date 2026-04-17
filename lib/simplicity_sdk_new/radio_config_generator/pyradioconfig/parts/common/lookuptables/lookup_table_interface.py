from abc import ABC, abstractmethod
from typing import Any, Union, TypedDict


class LookupTableResult(TypedDict):
    """
    Type definition for the expected format of lookup table results.
    Keys should be register/table names (strings) and values should be lists of integers.
    """
    # This allows any string key with list[int] values
    # Example: {'reg1': [1, 2, 3], 'reg2': [4, 5, 6]}
    pass


class LookupTableInterface(ABC):
    """
    Abstract base class (interface) that defines the contract for look up tables. All implementing classes will be
    required to implement the calculate_table method.
    """
    def __init__(self, input_parameters: list[Any]):
        self.input_parameters = input_parameters

    @abstractmethod
    def _calculate_table_impl(self) -> dict[str, list[int]]:
        """
        Abstract method that must be implemented by all subclasses.
        This method should perform the actual calculations on the table data.
        
        Returns:
            dict[str, list[int]]: A dictionary where:
                - Keys are register/table names (strings) that will become C variable names
                - Values are lists of integers that will become uint32_t arrays in C code
                - Optionally include metadata keys in format '_<tablename>_metadata' or '_<table1>_<table2>_metadata'
                
        Example:
            {
                'timingReg20mLookupTable': [0, 1, 3, 4],
                'freqPlanReg085C0Table': [513, 1025, 1537, 2049],
                '_freqPlanReg085C0Table_timingReg20mLookupTable_metadata': {
                    'calculation_method': 'combined_calculation',
                    'version': '1.0',
                    'notes': 'Generated multiple tables from same input parameters'
                }
            }
            
        Note: If no metadata is provided, a shared metadata entry will be automatically added
        using the pattern '_<sorted_table_names>_metadata' with table_count and table_names.
        When multiple input parameter sets are provided, additional '_index_0', '_index_1', etc. 
        entries will be added to map array indices to specific inputs.
        """
        pass

    def calculate_table(self) -> dict[str, Union[list[int], dict[str, Any]]]:
        """
        Public method that calls the implementation and automatically validates the result.
        This method is final - subclasses should implement _calculate_table_impl() instead.
        
        Returns:
            dict[str, Union[list[int], dict[str, Any]]]: The validated lookup table result with metadata
        """
        result = self._calculate_table_impl()
        return self.validate_result(result)

    def validate_result(self, result: Any) -> dict[str, Union[list[int], dict[str, Any]]]:
        """
        Validates that the result from calculate_table() matches the expected format.
        Now supports metadata section for input tracking.
        
        Args:
            result: The result to validate
            
        Returns:
            dict[str, Union[list[int], dict[str, Any]]]: The validated result with metadata
            
        Raises:
            TypeError: If the result format is incorrect
            ValueError: If the result contains invalid data
        """
        if not isinstance(result, dict):
            raise TypeError(f"calculate_table() must return a dictionary, got {type(result)}")
        
        validated_result = {}
        table_keys = []
        
        for key, value in result.items():
            if key.startswith('_') and key.endswith('_metadata'):
                # Validate metadata structure (handles both single table and concatenated table metadata)
                if not isinstance(value, dict):
                    raise TypeError(f"{key} must be a dictionary")
                validated_result[key] = value
                continue
                
            if not isinstance(key, str):
                raise TypeError(f"Dictionary keys must be strings (table names), got {type(key)} for key: {key}")
            
            if not isinstance(value, list):
                raise TypeError(f"Dictionary values must be lists, got {type(value)} for key: {key}")
            
            # Convert all values to integers and validate
            try:
                validated_values = [int(v) for v in value]
                validated_result[key] = validated_values
                table_keys.append(key)
            except (ValueError, TypeError) as e:
                raise ValueError(f"All values in list must be convertible to integers for key '{key}': {e}")
        
        # Add input parameters to metadata if not already present
        # Use a single shared metadata entry for all tables generated from the same inputs
        if table_keys:
            # Create metadata key by concatenating all table names
            metadata_key = f"_{'_'.join(sorted(table_keys))}_metadata"
            
            if metadata_key not in validated_result:
                # Check if any table-specific metadata already exists
                has_table_specific_metadata = any(f'_{key}_metadata' in validated_result for key in table_keys)
                
                if not has_table_specific_metadata:
                    # Create indexed metadata for multiple input parameter sets
                    metadata_dict = {
                        'table_count': len(table_keys),
                        'table_names': sorted(table_keys)
                    }
                    
                    # If we have multiple input parameter sets, add index mapping instead of input_parameters
                    if isinstance(self.input_parameters, list) and len(self.input_parameters) > 1:
                        for i, input_param in enumerate(self.input_parameters):
                            metadata_dict[f'_index_{i}'] = input_param
                    else:
                        # For single input parameter set, use the traditional input_parameters field
                        metadata_dict['input_parameters'] = self.input_parameters
                    
                    validated_result[metadata_key] = metadata_dict
        
        return validated_result
