from pyradioconfig.calculator_model_framework.interfaces.icalculator import ICalculator
from pycalcmodel.core.variable import ModelVariableFormat


"""
Calculations interface file
"""

class IPCalculator(ICalculator):

    _peripheral_name = ""
    _reg_field_list = []

    def __init__(self, peripheral_name):
        self._peripheral_name = peripheral_name

    def buildVariables(self, modem_model):
        # All IP calculators automatically add regs from the _reg_field_list to model vars
        self._add_registers_to_model_variable(modem_model, self._reg_field_list)

    def _add_registers_to_model_variable(self, model, reg_field_list):
        for reg_field in reg_field_list:
            reg_field_name = self._peripheral_name + "." + reg_field
            self._addModelRegister(model, reg_field_name, int, ModelVariableFormat.HEX)

    def _ip_reg_write(self, model, reg_field_name, field_value=None,
                      default=False, do_not_care=False, limit_upper=None, limit_lower=None,
                      check_saturation=False, allow_neg=False, neg_twos_comp=True):
        """
        varname, value=None, default=False, do_not_care=False, limit_upper=None,
                   limit_lower=None, check_saturation=False, allow_neg=False, neg_twos_comp=True

        :param model:
        :param reg_field_name:
        :param field_value:
        :return:
        """
        reg_model = getattr(model.vars, self._peripheral_name + "_" + reg_field_name)
        self._reg_write(reg_model, field_value, default, do_not_care, limit_upper,
                   limit_lower, check_saturation, allow_neg, neg_twos_comp)

    def _ip_reg_write_default(self, model, reg_field_name):
        reg_model = getattr(model.vars, self._peripheral_name + "_" + reg_field_name)
        self._reg_write_default(reg_model)
