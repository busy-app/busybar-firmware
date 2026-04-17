
from typing import List, Any

from pyradioconfig.parts.common.lookuptables.lookup_table_interface import LookupTableInterface


class FrameCodingLookupTable(LookupTableInterface):
    """
    Implementation of TableInterface for Packed Frame Coding calculations.
    """

    def __init__(self, input_parameters: List[Any]):
        super().__init__(input_parameters=input_parameters)

    def _calculate_table_impl(self):
        """
        Calculate packed framed coding values for all channels in the lookup table.
        Returns a dictionary where keys are table names and values are lists of uint32 values.
        """
        packed_list = list()
        total_bits = 0
        word_being_built = 0

        for input_dict in self.input_parameters:
            width = input_dict['width']
            for list_item in input_dict['coding_array']:
                # word_being_built = (word_being_built << width) | list_item                # Use this line for Big Endian
                word_being_built = (word_being_built >> width) | (
                            list_item << (32 - width))  # Use this line for Little Endian
                total_bits += width
                if total_bits % 32 == 0:
                    packed_list.append(word_being_built)
                    word_being_built = 0

        
        # Return dictionary format expected by generic lookup table implementation
        return {
            'frameCodingTable': packed_list
        }
