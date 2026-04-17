from pycalcmodel.core.variable import ModelVariableFormat
from pyradioconfig.calculator_model_framework.interfaces.icalculator import ICalculator
from pyradioconfig.calculator_model_framework.Utils.CustomExceptions import CalculationException
from pyradioconfig.parts.common.lookuptables.frame_coding import FrameCodingLookupTable


class CALC_LUT(ICalculator):
    def buildVariables(self, model):
        self._addModelVariable(model, 'generic_lookup_tables', dict, ModelVariableFormat.DECIMAL,
                               'Generic lookup tables dictionary')

    def calc_lookup_tables(self, model):
        model.vars.generic_lookup_tables.value = self._frame_coding(model)

    def _frame_coding(self, model):
        input_parameters = []
        width = 0
        coding_array = []

        frame_coding = model.vars.frame_coding.value
        if frame_coding != model.vars.frame_coding.var_enum.NONE and \
                frame_coding != model.vars.frame_coding.var_enum.UART_NO_VAL:

            coding_array = model.vars.frame_coding_array.value
            width = model.vars.frame_coding_array_width.value
            if width == 0:
                return None
            elif width == 8:
                if (len(coding_array) % 4) != 0:
                    raise CalculationException("Frame coding array not word aligned!")
            elif width == 16:
                if (len(coding_array) % 2) != 0:
                    raise CalculationException("Frame coding array not word aligned!")
            else:
                raise CalculationException("Unexpected frame coding array width of %s!" % width)

        input_parameters.append({
            'coding_array': coding_array,
            'width': width
        })
        # Create frame coding calculator using the interface
        calculator = FrameCodingLookupTable(
            input_parameters=input_parameters
        )
        return calculator.calculate_table() if coding_array else None


