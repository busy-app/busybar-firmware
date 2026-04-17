from py_2_and_3_compatibility import py2round
from pyradioconfig.parts.margay.calculators.calc_agc import CALC_AGC_Margay


class calc_agc_serval(CALC_AGC_Margay):

    def calc_agc_periodhi_periodlow(self, model):
        mod_format = model.vars.modulation_type.value
        modem_frequency_hz = model.vars.modem_frequency_hz.value
        f_if = model.vars.if_frequency_hz_actual.value
        baudrate = model.vars.baudrate.value

        if mod_format == model.vars.modulation_type.var_enum.ASK:
            periodlow = int(py2round((modem_frequency_hz / (baudrate * 0.9))))

            # period over which we count how many times we tripped the HI threshold - xtal PLL freq because AGC runs at this clock
            if f_if > 0:
                periodhi = int(py2round(modem_frequency_hz / (2 * f_if)))
            else:
                periodhi = 14  # for zero-IF used on FPGA tests fix periodhi to 14

            self._reg_write(model.vars.AGC_AGCPERIOD0_PERIODHI, int(round(periodhi)))
            self._reg_write(model.vars.AGC_AGCPERIOD1_PERIODLOW, int(round(periodlow)))
        else:
            super().calc_agc_periodhi_periodlow(model)
