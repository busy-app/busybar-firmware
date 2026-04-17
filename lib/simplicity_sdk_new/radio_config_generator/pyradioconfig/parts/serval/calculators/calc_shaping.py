import numpy as np

from pyradioconfig.parts.margay.calculators.calc_shaping import CALC_Shaping_Margay


class calc_shaping_serval(CALC_Shaping_Margay):

    def calc_ookshapingen_reg(self, model):
        modulation_type = model.vars.modulation_type.value

        if modulation_type == model.vars.modulation_type.var_enum.ASK:
            ookshapingen = 1
        else:
            ookshapingen = 0

        self._reg_write(model.vars.MODEM_OOKSHAPING_OOKSHAPINGEN, ookshapingen)

    def calc_ookshaping_lutsize_step(self, model):
        ookshapingen = model.vars.MODEM_OOKSHAPING_OOKSHAPINGEN.value

        if ookshapingen == 1:
            interpolation_rate = 8
            ookshapinglutsize = 7 #programmed as actual LUT size - 1
            ookshapingstep = (ookshapinglutsize+1)*interpolation_rate # 64
        else:
            ookshapinglutsize = 0
            ookshapingstep = 0

        self._reg_write(model.vars.MODEM_OOKSHAPING_OOKSHAPINGLUTSIZE, ookshapinglutsize)
        self._reg_write(model.vars.MODEM_OOKSHAPING_OOKSHAPINGSTEP, ookshapingstep)

    def calc_shaping_reg(self, model):
        ookshapingen = model.vars.MODEM_OOKSHAPING_OOKSHAPINGEN.value
        ask_mod_depth_perc = model.vars.ask_mod_depth_perc.value
        max_filter_taps = model.vars.max_filter_taps.value
        coeff = np.zeros(max_filter_taps)

        # If OOK shaping is enabled then we set the shaping coeffs according to this feature
        # PA_max = (LUT_max * OOKSHAPINGSTEP * MODINDEX)  # 127 * 64 * MODINDEX, so scale modindex accordingly
        # PA_min = (LUT_min * OOKSHAPINGSTEP * MODINDEX)

        if ookshapingen:
            high_level = 127  # Set statically
            ask_low_power_frac = ((100-ask_mod_depth_perc)/100.0)**2 #Mod depth is defined in voltage but PA codes are in power
            low_level = int(high_level * ask_low_power_frac)

            coeff[0] = coeff[1] = coeff[2] = coeff[3] = low_level
            coeff[4] = coeff[5] = coeff[6] = coeff[7] = high_level
            coeff[8] = 0
            shaping = 0

            self.write_coeff_registers(model, coeff, shaping)

        else:
            super().calc_shaping_reg(model)
