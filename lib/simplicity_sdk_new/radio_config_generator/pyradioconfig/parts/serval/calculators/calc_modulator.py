from pycalcmodel.core.variable import ModelVariableFormat
from pyradioconfig.parts.margay.calculators.calc_modulator import CALC_Modulator_Margay


class calc_modulator_serval(CALC_Modulator_Margay):

    def buildVariables(self, model):
        super().buildVariables(model)

        self._addModelVariable(model, 'ask_mod_depth_perc', float, ModelVariableFormat.DECIMAL, 'Percent attenuation from high to low amplitude level')

    def calc_ask_mod_depth_perc(self, model):
        modulation_type = model.vars.modulation_type.value

        if modulation_type == model.vars.modulation_type.var_enum.ASK:
            model.vars.ask_mod_depth_perc.value = 75.0
        else:
            model.vars.ask_mod_depth_perc.value = 0.0


    def calc_modindex_value(self, model):
        mod_type = model.vars.modulation_type.value
        shaping_filter_gain = model.vars.shaping_filter_gain_actual.value
        interpolation_gain = model.vars.interpolation_gain_actual.value

        if mod_type == model.vars.modulation_type.var_enum.ASK:
            modindex = self.max_pa_value * 16.0 / (shaping_filter_gain * interpolation_gain)
            model.vars.modindex.value = modindex
        else:
            super().calc_modindex_value(model)
