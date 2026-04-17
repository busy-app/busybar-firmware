
from pycalcmodel.core.variable import ModelVariableFormat
from pyradioconfig.parts.common.calculators.calc_lookup_tables import CALC_LUT
from pyradioconfig.parts.common.lookuptables.frame_coding import FrameCodingLookupTable


class CALC_LUT_ocelot(CALC_LUT):
    def buildVariables(self, model):
        self._addModelVariable(model, 'generic_lookup_tables', dict, ModelVariableFormat.DECIMAL,
                               'Generic lookup tables dictionary')

    def calc_lookup_tables(self, model):
        model.vars.generic_lookup_tables.value = self._frame_coding(model)


    def _frame_coding(self, model):
        symbol_encoding = model.vars.symbol_encoding.value
        demod_select = model.vars.demod_select.value

        input_parameters = []


        if (symbol_encoding == model.vars.symbol_encoding.var_enum.Manchester or
            symbol_encoding == model.vars.symbol_encoding.var_enum.Inv_Manchester) and \
                demod_select == model.vars.demod_select.var_enum.TRECS_VITERBI:
            coding_array = model.vars.frame_coding_array.value
            width = model.vars.frame_coding_array_width.value
        else:
            return super()._frame_coding(model)

        input_parameters.append({
            'coding_array': coding_array,
            'width': width
        })
        # Create frame coding calculator using the interface
        calculator = FrameCodingLookupTable(
            input_parameters=input_parameters
        )
        return calculator.calculate_table() if coding_array else None


