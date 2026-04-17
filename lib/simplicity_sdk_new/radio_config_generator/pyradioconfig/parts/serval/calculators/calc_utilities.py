import math

from pyradioconfig.parts.margay.calculators.calc_utilities import CALC_Utilities_Margay


class calc_utilities_serval(CALC_Utilities_Margay):

    def calc_target_sensitivity(self, model):
        bitrate = model.vars.bitrate.value
        freq = model.vars.base_frequency_hz.value
        modformat = model.vars.modulation_type.value

        if modformat == model.vars.modulation_type.var_enum.ASK:
            ebno = 19.0
            if freq < 500e6:
                nf = 3.5
            else:
                nf = 4.0
            target_sensitivity = -173.9 + 10 * math.log(bitrate, 10) + ebno + nf
            model.vars.sensitivity.value = target_sensitivity
        else:
            super().calc_target_sensitivity(model)
