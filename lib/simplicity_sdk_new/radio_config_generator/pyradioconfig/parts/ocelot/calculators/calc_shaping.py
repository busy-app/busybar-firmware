from pyradioconfig.parts.lynx.calculators.calc_shaping import CALC_Shaping_lynx
import numpy as np
from pyradioconfig.calculator_model_framework.Utils.LogMgr import LogMgr
from pycalcmodel.core.variable import ModelVariableFormat, CreateModelVariableEnum
from pyradioconfig.calculator_model_framework.Utils.CustomExceptions import CalculationException
from pyradioconfig.parts.common.utils.tinynumpy import tinynumpy
from pycalcmodel.py2_and_3_compatibility import py2round
import math


class CALC_Shaping_ocelot(CALC_Shaping_lynx):

    # In mode 1 and 2, I think, the calculator only generates filters with up to 8 coefficients anyway so no need to
    # update the gain calculations for those cases. If, however, we change shaping filter coefficient calculation we
    # might need to update gain calculations as well. -- Guner

    def buildVariables(self, model):
        super().buildVariables(model)
        self._addModelVariable(model, 'shaping_filter_taps', int, ModelVariableFormat.DECIMAL,
                               desc='Number of taps in the shaping filter')

    def calc_shaping_filter_gain_actual(self, model):
        # The Ocelot shaping filter registers have changed in the register map, so need to override this function

        shaping_filter_mode = model.vars.MODEM_CTRL0_SHAPING.value
        sf = self.get_shaping_filter(model)

        if shaping_filter_mode == 0:
            shaping_filter_gain = 127

        elif shaping_filter_mode == 1:
            shaping_filter_gain = max(sf[0] + sf[8] + sf[0], sf[1] + sf[7], sf[2] + sf[6], sf[3] + sf[5], sf[4] + sf[4])

        elif shaping_filter_mode == 2:
            shaping_filter_gain = max(sf[0] + sf[7], sf[1] + sf[6], sf[2] + sf[5], sf[3] + sf[4])

        else:
            shaping_filter_gain = max(sf[0] + sf[8] + sf[16] + sf[24] + sf[32] + sf[40] + sf[48] + sf[56],
                                      sf[1] + sf[9] + sf[17] + sf[25] + sf[33] + sf[41] + sf[49] + sf[57],
                                      sf[2] + sf[10] + sf[18] + sf[26] + sf[34] + sf[42] + sf[50] + sf[58],
                                      sf[3] + sf[11] + sf[19] + sf[27] + sf[35] + sf[43] + sf[51] + sf[59],
                                      sf[4] + sf[12] + sf[20] + sf[28] + sf[36] + sf[44] + sf[52] + sf[60],
                                      sf[5] + sf[13] + sf[21] + sf[29] + sf[37] + sf[45] + sf[53] + sf[61],
                                      sf[6] + sf[14] + sf[22] + sf[30] + sf[38] + sf[46] + sf[54] + sf[62],
                                      sf[7] + sf[15] + sf[23] + sf[31] + sf[39] + sf[47] + sf[55] + sf[63])

        model.vars.shaping_filter_gain_actual.value = int(shaping_filter_gain)

    def get_shaping_filter(self, model):
        # Construct shaping filter from register settings

        shaping_filter_mode = model.vars.MODEM_CTRL0_SHAPING.value
        c0 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING0_COEFF0.value, model.vars.MODEM_SHAPING0_COEFF1.get_bit_width())
        c1 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING0_COEFF1.value, model.vars.MODEM_SHAPING0_COEFF1.get_bit_width())
        c2 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING0_COEFF2.value, model.vars.MODEM_SHAPING0_COEFF2.get_bit_width())
        c3 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING0_COEFF3.value, model.vars.MODEM_SHAPING0_COEFF3.get_bit_width())
        c4 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING1_COEFF4.value, model.vars.MODEM_SHAPING1_COEFF4.get_bit_width())
        c5 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING1_COEFF5.value, model.vars.MODEM_SHAPING1_COEFF5.get_bit_width())
        c6 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING1_COEFF6.value, model.vars.MODEM_SHAPING1_COEFF6.get_bit_width())
        c7 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING1_COEFF7.value, model.vars.MODEM_SHAPING1_COEFF7.get_bit_width())
        c8 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING2_COEFF8.value, model.vars.MODEM_SHAPING2_COEFF8.get_bit_width())
        c9 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING2_COEFF9.value, model.vars.MODEM_SHAPING2_COEFF9.get_bit_width())
        c10 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING2_COEFF10.value, model.vars.MODEM_SHAPING2_COEFF10.get_bit_width())
        c11 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING2_COEFF11.value, model.vars.MODEM_SHAPING2_COEFF11.get_bit_width())
        c12 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING3_COEFF12.value, model.vars.MODEM_SHAPING3_COEFF12.get_bit_width())
        c13 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING3_COEFF13.value, model.vars.MODEM_SHAPING3_COEFF13.get_bit_width())
        c14 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING3_COEFF14.value, model.vars.MODEM_SHAPING3_COEFF14.get_bit_width())
        c15 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING3_COEFF15.value, model.vars.MODEM_SHAPING3_COEFF15.get_bit_width())
        c16 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING4_COEFF16.value, model.vars.MODEM_SHAPING4_COEFF16.get_bit_width())
        c17 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING4_COEFF17.value, model.vars.MODEM_SHAPING4_COEFF17.get_bit_width())
        c18 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING4_COEFF18.value, model.vars.MODEM_SHAPING4_COEFF18.get_bit_width())
        c19 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING4_COEFF19.value, model.vars.MODEM_SHAPING4_COEFF19.get_bit_width())
        c20 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING5_COEFF20.value, model.vars.MODEM_SHAPING5_COEFF20.get_bit_width())
        c21 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING5_COEFF21.value, model.vars.MODEM_SHAPING5_COEFF21.get_bit_width())
        c22 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING5_COEFF22.value, model.vars.MODEM_SHAPING5_COEFF22.get_bit_width())
        c23 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING5_COEFF23.value, model.vars.MODEM_SHAPING5_COEFF23.get_bit_width())
        c24 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING6_COEFF24.value, model.vars.MODEM_SHAPING6_COEFF24.get_bit_width())
        c25 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING6_COEFF25.value, model.vars.MODEM_SHAPING6_COEFF25.get_bit_width())
        c26 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING6_COEFF26.value, model.vars.MODEM_SHAPING6_COEFF26.get_bit_width())
        c27 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING6_COEFF27.value, model.vars.MODEM_SHAPING6_COEFF27.get_bit_width())
        c28 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING7_COEFF28.value, model.vars.MODEM_SHAPING7_COEFF28.get_bit_width())
        c29 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING7_COEFF29.value, model.vars.MODEM_SHAPING7_COEFF29.get_bit_width())
        c30 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING7_COEFF30.value, model.vars.MODEM_SHAPING7_COEFF30.get_bit_width())
        c31 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING7_COEFF31.value, model.vars.MODEM_SHAPING7_COEFF31.get_bit_width())
        c32 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING8_COEFF32.value, model.vars.MODEM_SHAPING8_COEFF32.get_bit_width())
        c33 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING8_COEFF33.value, model.vars.MODEM_SHAPING8_COEFF33.get_bit_width())
        c34 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING8_COEFF34.value, model.vars.MODEM_SHAPING8_COEFF34.get_bit_width())
        c35 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING8_COEFF35.value, model.vars.MODEM_SHAPING8_COEFF35.get_bit_width())
        c36 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING9_COEFF36.value, model.vars.MODEM_SHAPING9_COEFF36.get_bit_width())
        c37 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING9_COEFF37.value, model.vars.MODEM_SHAPING9_COEFF37.get_bit_width())
        c38 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING9_COEFF38.value, model.vars.MODEM_SHAPING9_COEFF38.get_bit_width())
        c39 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING9_COEFF39.value, model.vars.MODEM_SHAPING9_COEFF39.get_bit_width())
        c40 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING10_COEFF40.value, model.vars.MODEM_SHAPING10_COEFF40.get_bit_width())
        c41 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING10_COEFF41.value, model.vars.MODEM_SHAPING10_COEFF41.get_bit_width())
        c42 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING10_COEFF42.value, model.vars.MODEM_SHAPING10_COEFF42.get_bit_width())
        c43 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING10_COEFF43.value, model.vars.MODEM_SHAPING10_COEFF43.get_bit_width())
        c44 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING11_COEFF44.value, model.vars.MODEM_SHAPING11_COEFF44.get_bit_width())
        c45 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING11_COEFF45.value, model.vars.MODEM_SHAPING11_COEFF45.get_bit_width())
        c46 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING11_COEFF46.value, model.vars.MODEM_SHAPING11_COEFF46.get_bit_width())
        c47 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING11_COEFF47.value, model.vars.MODEM_SHAPING11_COEFF47.get_bit_width())
        c48 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING12_COEFF48.value, model.vars.MODEM_SHAPING12_COEFF48.get_bit_width())
        c49 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING12_COEFF49.value, model.vars.MODEM_SHAPING12_COEFF49.get_bit_width())
        c50 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING12_COEFF50.value, model.vars.MODEM_SHAPING12_COEFF50.get_bit_width())
        c51 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING12_COEFF51.value, model.vars.MODEM_SHAPING12_COEFF51.get_bit_width())
        c52 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING13_COEFF52.value, model.vars.MODEM_SHAPING13_COEFF52.get_bit_width())
        c53 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING13_COEFF53.value, model.vars.MODEM_SHAPING13_COEFF53.get_bit_width())
        c54 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING13_COEFF54.value, model.vars.MODEM_SHAPING13_COEFF54.get_bit_width())
        c55 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING13_COEFF55.value, model.vars.MODEM_SHAPING13_COEFF55.get_bit_width())
        c56 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING14_COEFF56.value, model.vars.MODEM_SHAPING14_COEFF56.get_bit_width())
        c57 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING14_COEFF57.value, model.vars.MODEM_SHAPING14_COEFF57.get_bit_width())
        c58 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING14_COEFF58.value, model.vars.MODEM_SHAPING14_COEFF58.get_bit_width())
        c59 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING14_COEFF59.value, model.vars.MODEM_SHAPING14_COEFF59.get_bit_width())
        c60 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING15_COEFF60.value, model.vars.MODEM_SHAPING15_COEFF60.get_bit_width())
        c61 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING15_COEFF61.value, model.vars.MODEM_SHAPING15_COEFF61.get_bit_width())
        c62 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING15_COEFF62.value, model.vars.MODEM_SHAPING15_COEFF62.get_bit_width())
        c63 = self._covert_twos_complement_to_signed(model.vars.MODEM_SHAPING15_COEFF63.value, model.vars.MODEM_SHAPING15_COEFF63.get_bit_width())

        if shaping_filter_mode == 0:
            shaping_filter = np.array([127, 127, 127, 127, 127, 127, 127, 127])

        elif shaping_filter_mode == 1:
            shaping_filter = np.array([c0, c1, c2, c3, c4, c5, c6, c7, c8, c7, c6, c5, c4, c3, c2, c1, c0])

        elif shaping_filter_mode == 2:
            shaping_filter = np.array([c0, c1, c2, c3, c4, c5, c6, c7, c7, c6, c5, c4, c3, c2, c1, c0])

        else:
            shaping_filter = np.array([ c0,  c1,  c2,  c3,  c4,  c5,  c6,  c7,  c8,  c9, c10, c11, c12, c13, c14, c15,
                            c16, c17, c18, c19, c20, c21, c22, c23, c24, c25, c26, c27, c28, c29, c30, c31,
                            c32, c33, c34, c35, c36, c37, c38, c39, c40, c41, c42, c43, c44, c45, c46, c47,
                            c48, c49, c50, c51, c52, c53, c54, c55, c56, c57, c58, c59, c60, c61, c62, c63])
        return shaping_filter

    def calc_filter_tap_length(self, model):
        """ Calculate shaping filter delay in samples """
        shaping = model.vars.MODEM_CTRL0_SHAPING.value

        taps = 0
        if shaping == 0: # : DISABLED
            taps = 8
        elif shaping == 1: # : ODDLENGTH
            taps = 17
        elif shaping == 2: # : EVENLENGTH
            taps = 16
        elif shaping == 3: # : Asymmetric
            taps = 64
        else: # : Unsupported selection
            LogMgr.Error(f"Error: Unsupported shaping value of {shaping}!")

        model.vars.shaping_filter_taps.value = taps

    def calc_max_available_filter_taps(self, model):
        # this is fixed for a family of parts
        model.vars.max_filter_taps.value = 64

    def write_coeff_registers(self, model, coeff, shaping):

        self._reg_write(model.vars.MODEM_SHAPING0_COEFF0, int(coeff[0]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING0_COEFF1, int(coeff[1]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING0_COEFF2, int(coeff[2]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING0_COEFF3, int(coeff[3]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING1_COEFF4, int(coeff[4]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING1_COEFF5, int(coeff[5]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING1_COEFF6, int(coeff[6]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING1_COEFF7, int(coeff[7]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING2_COEFF8, int(coeff[8]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING2_COEFF9, int(coeff[9]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING2_COEFF10, int(coeff[10]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING2_COEFF11, int(coeff[11]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING3_COEFF12, int(coeff[12]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING3_COEFF13, int(coeff[13]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING3_COEFF14, int(coeff[14]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING3_COEFF15, int(coeff[15]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING4_COEFF16, int(coeff[16]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING4_COEFF17, int(coeff[17]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING4_COEFF18, int(coeff[18]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING4_COEFF19, int(coeff[19]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING5_COEFF20, int(coeff[20]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING5_COEFF21, int(coeff[21]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING5_COEFF22, int(coeff[22]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING5_COEFF23, int(coeff[23]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING6_COEFF24, int(coeff[24]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING6_COEFF25, int(coeff[25]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING6_COEFF26, int(coeff[26]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING6_COEFF27, int(coeff[27]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING7_COEFF28, int(coeff[28]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING7_COEFF29, int(coeff[29]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING7_COEFF30, int(coeff[30]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING7_COEFF31, int(coeff[31]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING8_COEFF32, int(coeff[32]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING8_COEFF33, int(coeff[33]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING8_COEFF34, int(coeff[34]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING8_COEFF35, int(coeff[35]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING9_COEFF36, int(coeff[36]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING9_COEFF37, int(coeff[37]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING9_COEFF38, int(coeff[38]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING9_COEFF39, int(coeff[39]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING10_COEFF40, int(coeff[40]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING10_COEFF41, int(coeff[41]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING10_COEFF42, int(coeff[42]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING10_COEFF43, int(coeff[43]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING11_COEFF44, int(coeff[44]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING11_COEFF45, int(coeff[45]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING11_COEFF46, int(coeff[46]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING11_COEFF47, int(coeff[47]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING12_COEFF48, int(coeff[48]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING12_COEFF49, int(coeff[49]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING12_COEFF50, int(coeff[50]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING12_COEFF51, int(coeff[51]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING13_COEFF52, int(coeff[52]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING13_COEFF53, int(coeff[53]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING13_COEFF54, int(coeff[54]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING13_COEFF55, int(coeff[55]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING14_COEFF56, int(coeff[56]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING14_COEFF57, int(coeff[57]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING14_COEFF58, int(coeff[58]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING14_COEFF59, int(coeff[59]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING15_COEFF60, int(coeff[60]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING15_COEFF61, int(coeff[61]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING15_COEFF62, int(coeff[62]), allow_neg=True)
        self._reg_write(model.vars.MODEM_SHAPING15_COEFF63, int(coeff[63]), allow_neg=True)

        self._reg_write(model.vars.MODEM_CTRL0_SHAPING, shaping)

    def get_coeff_ceil(self, model):
        # all filter taps are 8 bit signed registers
        max_filter_taps = model.vars.max_filter_taps.value
        coeff_ceil = np.empty(max_filter_taps)
        # all filter taps are 8 bit signed registers
        coeff_ceil.fill((2 ** (8 - 1)) - 1)
        return coeff_ceil

    def floor_filter_coefficients(self, h):
        # return non negative h
        # for now not allowing any negative values until we are working on MCUW_RADIO_CFG-2007
        return h

    def root_raised_cosine_filter(self, model):
        """

        Args:
            model (ModelRoot) : Data model to read and write variables from
        """

        # for raised cosine pulse shapes pulse_shape_parameter holds roll off factor value
        bt = model.vars.shaping_filter_param.value
        max_filter_taps = model.vars.max_filter_taps.value

        if bt > 1.0 or bt < 0.0:
            LogMgr.Error("shaping_filter_param for Root Raised Cosine filter is treated as Roll-off factor. "
                         "Please ensure that 0 <= shaping_filter_param <= 1. "
                         "Overriding the current shaping_filter_param value to 0.")
            bt = 0.0

        pi = math.pi
        # create empty coefficient array
        h = [0.0] * max_filter_taps
        # for each coeff to be calculated
        for x in tinynumpy.arange(0, max_filter_taps):
            # get time index
            x = int(x)
            t = (x - int(max_filter_taps/2)) / 8.0
            # handle special cases and calculate the coeffs
            if t == 0.0:
                h[x] = (1-bt)+4*bt/pi
            elif bt != 0 and t == 1/(4*bt):
                h[x] = bt/math.sqrt(2) * ((1+2/pi)*math.sin(pi/(4*bt))+(1-2/pi)*math.cos(pi/(4*bt)))
            elif bt != 0 and t == -1/(4*bt):
                h[x] = bt/math.sqrt(2) * ((1+2/pi)*math.sin(pi/(4*bt))+(1-2/pi)*math.cos(pi/(4*bt)))
            else:
                h[x] = (math.sin(pi*t*(1-bt)) + 4*bt*t*math.cos(pi*t*(1+bt)))/( pi*t*(1-(4*bt*t)*(4*bt*t)) )

        # scale so that the peak tap is 127
        peak = max(h)
        for x in tinynumpy.arange(0, max_filter_taps):
            x = int(x)
            h[x] = py2round(h[x] / peak * 127 - 0.5)
            if h[x] < 0:
                h[x] = self.floor_filter_coefficients(h[x])

        return h

    def calc_shaping_reg(self, model):
        """
        given shaping filter input parameter set shaping filter coeffs and type

        Args:
            model (ModelRoot) : Data model to read and write variables from
        """

        shaping_filter_option = model.vars.shaping_filter.value
        max_filter_taps = model.vars.max_filter_taps.value
        coeff = np.zeros(max_filter_taps)
        shaping = 0
        if shaping_filter_option.value == model.vars.shaping_filter.var_enum.NONE.value:
            shaping = 0

        elif shaping_filter_option.value == model.vars.shaping_filter.var_enum.Gaussian.value:
            calc_coeff_error = True
            scaling_fac = 127
            # The loop here uses "check_coeff_write_error" to make sure we are not writing any filter tap coefficient
            # with a value greater than its bit-width capacity. If "check_coeff_write_error" returns True, we scale the
            # coefficients down and keep scaling the coefficients down untill all coefficients fit the respective
            # signed bit-wdith fields.
            # This is to handle the special case of Nerio, Nixi and Jumbo where not all filter taps coefficients are
            # 8-bit signed registers
            while calc_coeff_error:
                # adding an exit condition
                if scaling_fac < 0:
                    c = np.zeros(max_filter_taps)
                    shaping = 3
                else:
                    c, shaping = self.gaussian_shaping_filter(model, scaling_fac)
                    # Create temporary coeff
                    temp_coeff = np.zeros(max_filter_taps)
                    # read values in temporary coffs
                    temp_coeff = self.update_coeffs(model, c, temp_coeff)
                    calc_coeff_error = self.check_coeff_write_error(model, temp_coeff)
                    scaling_fac = scaling_fac - 10

            coeff = self.update_coeffs(model, c, coeff)

        elif shaping_filter_option.value == model.vars.shaping_filter.var_enum.Custom_OQPSK.value:
            # Not sure what filter this is
            coeff[0] = coeff[1] = 1
            coeff[2] = 16
            coeff[3] = 48
            coeff[4] = 80
            coeff[5] = 112
            coeff[6] = coeff[7] = 127
            coeff[8] = 0
            shaping = 2

        elif shaping_filter_option.value == model.vars.shaping_filter.var_enum.Raised_Cosine.value:
            c = self.raised_cosine_filter(model)
            shaping = 1
            coeff = self.update_coeffs(model, c, coeff)

        elif shaping_filter_option.value == model.vars.shaping_filter.var_enum.Root_Raised_Cosine.value:
            c = self.root_raised_cosine_filter(model)
            shaping = 3
            coeff = self.update_coeffs(model, c, coeff)

        elif shaping_filter_option.value == model.vars.shaping_filter.var_enum.Custom_PSK.value:
            coeff[0] = 51
            coeff[1] = 117
            coeff[2] = 96
            coeff[3] = 53
            coeff[4] = 20
            coeff[5] = 2
            coeff[6] = 0
            coeff[7] = 0
            coeff[8] = 0  # not used
            shaping = 3
        else:
            raise CalculationException("ERROR: Unrecognized shaping filter option")

        self.write_coeff_registers(model, coeff, shaping)
