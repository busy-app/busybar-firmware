from pyradioconfig.parts.nixi.calculators.calc_demodulator import CALC_Demodulator_nixi
from pycalcmodel.core.variable import ModelVariableFormat, CreateModelVariableEnum
from enum import Enum
import math
import itertools
from math import floor, log, ceil

from py_2_and_3_compatibility import *

class CALC_Demodulator_panther(CALC_Demodulator_nixi):

    def buildVariables(self, model):
        #Build all vars from inherited file
        super().buildVariables(model)

        self._addModelVariable(model, 'rssi_adjust_db', float, ModelVariableFormat.DECIMAL)
        self._addModelVariable(model, 'rssi_dig_adjust_db', float, ModelVariableFormat.DECIMAL)
        self._addModelVariable(model, 'rssi_rf_adjust_db', float, ModelVariableFormat.DECIMAL)
        self._addModelVariable(model, 'demod_select', Enum, ModelVariableFormat.DECIMAL)
        self._add_demod_select_variable(model)

        var = self._addModelVariable(model, 'zigbee_feature', Enum, ModelVariableFormat.DECIMAL, 'Zigbee Feature')
        member_data = [
            ['NONE', 0, 'None'],
            ['STANDARD', 1, 'Standard'],
            ['ANTDIV', 2, 'Antenna Diversity'],
            ['COEX', 3, 'Antenna Diversity'],
            ['ANTDIV_COEX', 4, 'Antenna Diversity'],
            ['FEM', 9, 'External LNA'],
            ['ANTDIV_FEM', 10, 'Antenna Diversity with External LNA'],
            ['COEX_FEM', 11, 'External LNA'],
            ['ANTDIV_COEX_FEM', 12, 'External LNA'],
            ['FCS', 14, 'Fast channel switch'],
            ['GB868_863', 133, 'UK Metering 863 MHz Band'],
            ['GB868_915', 134, 'UK Metering 915 MHz Band'],
            ['NA915_R23', 135, 'NA R23 915 MHz Band'],
            # We use the thousands place (decimal) to indicate different PHY (different demod) for same feature
            # See https://jira.silabs.com/browse/MCUW_RADIO_CFG-2862
            ['LEGACY', 1001, 'Legacy Demod'],
            ['COHERENT', 2001, 'Coherent Demod'],
            ['ENHANCED', 3001, 'Enhanced Demod'],
        ]
        var.var_enum = CreateModelVariableEnum(
            'ZigbeeFeatureEnum',
            'List of supported zigbee PHY features',
            member_data)

        var = self._addModelVariable(model, 'ble_feature', Enum, ModelVariableFormat.DECIMAL, 'Bluetooth LE Feature')
        member_data = [
            ['NONE', 0, 'None'],
            ['LE_1M', 1, 'Bluetooth LE 1Mbps'],
            ['LE_2M', 2, 'Bluetooth LE 2Mbps'],
            ['CODED_125K', 3, 'Bluetooth LE Coded 125Kbps'],
            ['CODED_500K', 4, 'Bluetooth LE Coded 500Kbps'],
            ['CONCURRENT', 5, 'Bluetooth Concurrent'],
            ['HADM_1M', 6, 'Bluetooth LE AoX 1Mbps'],
            ['HADM_2M', 7, 'Bluetooth LE AoX 2Mbps'],
            ['AOX_2M', 8, 'Bluetooth LE AoX 2Mbps'],
            ['AOX_1M', 10, 'Bluetooth LE AoX 1Mbps'],
            ['HADM_2M_2BT', 11, 'Bluetooth LE AoX 2Mbps 2BT'],
            ['FASTSW', 12, 'Fast switching'],
        ]
        var.var_enum = CreateModelVariableEnum(
            'BleFeatureEnum',
            'List of supported Bluetooth LE PHY features',
            member_data)

    def calc_default_feature_mode(self, model):
        model.vars.zigbee_feature.value = model.vars.zigbee_feature.var_enum.NONE
        model.vars.ble_feature.value = model.vars.ble_feature.var_enum.NONE

    def _add_demod_select_variable(self, model):
        model.vars.demod_select.var_enum = CreateModelVariableEnum(
            enum_name='DemodSelectEnum',
            enum_desc='Demod Selection',
            member_data=[
                ['LEGACY', 0, 'Legacy Demod'],
                ['COHERENT', 1, 'Coherent Demod'],
                ['TRECS_VITERBI', 2, 'TRecS + Viterbi Demod'],
                ['TRECS_SLICER', 3, 'TRecS + HD Demod'],
                ['LONGRANGE', 5, 'BLE Long Range Demod'],
            ])

    def calc_demod_sel(self, model):
        flag_using_Viterbi_demod = (model.vars.MODEM_VITERBIDEMOD_VTDEMODEN.value == 1)
        flag_using_BLELR = (model.vars.MODEM_LONGRANGE_LRBLE.value == 1)
        Viterbi_harddecision = model.vars.MODEM_VITERBIDEMOD_HARDDECISION.value
        flag_using_Coherent_demod = model.vars.MODEM_CTRL1_PHASEDEMOD.value == 2

        if flag_using_BLELR:
            demod_select = model.vars.demod_select.var_enum.LONGRANGE
        elif flag_using_Coherent_demod:
            demod_select = model.vars.demod_select.var_enum.COHERENT
        elif flag_using_Viterbi_demod and Viterbi_harddecision == 0:
            demod_select = model.vars.demod_select.var_enum.TRECS_VITERBI
        elif flag_using_Viterbi_demod and Viterbi_harddecision == 1:
            demod_select = model.vars.demod_select.var_enum.TRECS_SLICER
        else:
            demod_select = model.vars.demod_select.var_enum.LEGACY

        model.vars.demod_select.value = demod_select

    def calc_fxo_or_fdec8(self, model):
        if model.phy is not None:
            if (model.phy.name.find("CTUNE") > 0) or (model.phy.name.find("CW") > 0):
                # Leaving the legacy EFR32 90nm calculation here so the utility PHYs will calculate
                # Otherwise the utility PHYs will never resolve model.vars.xtal_frequency.value, so many, many calculations will never run.
                # TODO: Likely need a better solution here long term
                model.vars.fxo_or_fdec8.value = model.vars.xtal_frequency.value * 1.0

                # FIXME: Some 40nm PHYs must specify phy.profile_inputs.if_frequency_hz.value
                # Seems connected to a circular dependency in calc_fxo_or_fdec8 needing  model.vars.rx_synth_freq_actual

        # Note Panther will overload this to LO / 64
        # EFR32 90nm parts (Dumbo, Jumbo, Nerio, Nixi) use fxo here.
        model.vars.fxo_or_fdec8.value = model.vars.rx_synth_freq_actual.value / 64.0 # Panther-unique

    def calc_adc_freq_actual(self, model):
        fxo_or_fdec8 = model.vars.fxo_or_fdec8.value

        # For 2.4G series 2 parts, Fadc is based on divided down synth
        # Fixed div8 block is immediately after IFADC output
        adc_freq_actual = int(round(fxo_or_fdec8 * 8))

        model.vars.adc_freq_actual.value = adc_freq_actual

    def calc_src12_bit_widths(self, model):
        # Note Panther will overload this to reflect its different architecture and use 8, 15
        # EFR32 90nm parts (Dumbo, Jumbo, Nerio, Nixi) use different values
        model.vars.src1_bit_width.value = 8
        model.vars.src2_bit_width.value = 15

    def calc_ch_filt_bw_available(self, model):
        # Note Panther will overload this to reflect its different architecture.
        model.vars.ch_filt_bw_available = [0.263]

    def calc_src1_range_available(self, model):
        # EFR32 90nm parts (Dumbo, Jumbo, Nerio, Nixi)
        # Note Panther will overload this to reflect its different architecture.

        # optional input, 125 by default. Some PHYs may increase this so calculated value always matches RAIL
        model.vars.src1_range_available_minimum.value = 125
        src1_range_available_minimum = model.vars.src1_range_available_minimum.value

        model.vars.src1_range_available = xrange(src1_range_available_minimum, 156, 1)


    def calc_sqithresh_reg(self, model):
        """set sqithresh register - intermediate variable used to override as advanced input

        Args:
            model (ModelRoot) : Data model to read and write variables from
        """

        # Nixi would write this to 200...
        #self._reg_write(model.vars.MODEM_CTRL2_SQITHRESH, model.vars.sqi_threshold.value)
        pass

    def calc_viterbi_demod_expect_patt_value(self, model):

        flag_using_Viterbi_demod = (model.vars.MODEM_VITERBIDEMOD_VTDEMODEN.value == 1)
        flag_using_BLELR = (model.vars.MODEM_LONGRANGE_LRBLE.value == 1)
        syncword0 = model.vars.syncword_0.value
        preamble_pattern = model.vars.preamble_pattern.value

        if flag_using_Viterbi_demod and not flag_using_BLELR:
            # Last bit of preamble followed by all but one bit of syncword
            viterbi_demod_expect_patt = (syncword0 >> 1) | ((preamble_pattern & 1) << 31)
            model.vars.viterbi_demod_expect_patt.value = viterbi_demod_expect_patt
        else:
            # set to default reset value
            model.vars.viterbi_demod_expect_patt.value = long(0x123556B7)


    def calc_viterbi_demod_expect_patt_reg(self, model):

        viterbi_demod_expect_patt = model.vars.viterbi_demod_expect_patt.value


        self._reg_write(model.vars.MODEM_VTCORRCFG0_EXPECTPATT, viterbi_demod_expect_patt)

    def calc_viterbi_demod_expect_patt_head_tail_value(self, model):

        flag_using_Viterbi_demod = (model.vars.MODEM_VITERBIDEMOD_VTDEMODEN.value == 1)
        flag_using_BLELR = (model.vars.MODEM_LONGRANGE_LRBLE.value == 1)
        syncword0 = model.vars.syncword_0.value
        
        if flag_using_Viterbi_demod and not flag_using_BLELR:
            # Head and tail bits of syncword
            viterbi_demod_expect_patt_head_tail = (syncword0 & 1)<<1
            model.vars.viterbi_demod_expect_patt_head_tail.value = viterbi_demod_expect_patt_head_tail
        else:
            # set to default reset value
            model.vars.viterbi_demod_expect_patt_head_tail.value = 5


    def calc_viterbi_demod_expect_patt_head_tail_reg(self, model):

        viterbi_demod_expect_patt_head_tail = model.vars.viterbi_demod_expect_patt_head_tail.value


        self._reg_write(model.vars.MODEM_VTCORRCFG1_EXPECTHT, viterbi_demod_expect_patt_head_tail)

    def calc_viterbidemod_disdemodof_viterbi_demod_disable_overflow_detect_value(self, model):

        flag_using_Viterbi_demod = (model.vars.MODEM_VITERBIDEMOD_VTDEMODEN.value == 1)

        if flag_using_Viterbi_demod:
            model.vars.viterbidemod_disdemodof_viterbi_demod_disable_overflow_detect.value = 1
        else:
            # set to default reset value
            model.vars.viterbidemod_disdemodof_viterbi_demod_disable_overflow_detect.value = 0

    def calc_viterbidemod_disdemodof_viterbi_demod_disable_overflow_detect_reg(self, model):
        viterbidemod_disdemodof_viterbi_demod_disable_overflow_detect = model.vars.viterbidemod_disdemodof_viterbi_demod_disable_overflow_detect.value

        self._reg_write(model.vars.MODEM_VITERBIDEMOD_DISDEMODOF, viterbidemod_disdemodof_viterbi_demod_disable_overflow_detect)

    def calc_dec0_values_available(self, model):
        # Series 2 parts (Panther, Lynx, Ocelot) add optional inputs to disallow decimation by 3 or 8


        # Default to allow (the optional input)
        model.vars.input_decimation_filter_allow_dec3.value = 1
        model.vars.input_decimation_filter_allow_dec8.value = 1

        # Unless specified otherwise by optional inputs
        model.vars.input_decimation_filter_allow_dec3.value = model.vars.input_decimation_filter_allow_dec3.value
        model.vars.input_decimation_filter_allow_dec8.value = model.vars.input_decimation_filter_allow_dec8.value

    def calc_rssi_dig_adjust_db(self, model):
        #These variables are passed to RAIL so that RSSI corrections can be made to more accurately measure power

        #Read in model vars
        dec0gain = model.vars.MODEM_DIGIGAINCTRL_DEC0GAIN.value
        dec1_actual = model.vars.dec1_actual.value
        dec1gain_actual = model.vars.dec1gain_actual.value
        digigainen = model.vars.MODEM_DIGIGAINCTRL_DIGIGAINEN.value
        digigainsel = model.vars.MODEM_DIGIGAINCTRL_DIGIGAINSEL.value
        digigaindouble = model.vars.MODEM_DIGIGAINCTRL_DIGIGAINDOUBLE.value
        digigainhalf = model.vars.MODEM_DIGIGAINCTRL_DIGIGAINHALF.value

        #Calculate gains

        dec0_gain_db = 6.0*dec0gain

        dec1_gain_linear = (dec1_actual**4) * (2**(-1*math.floor(4*math.log2(dec1_actual)-4)))
        dec1_gain_db = 20*math.log10(dec1_gain_linear/16) + dec1gain_actual #Normalize so that dec1=0 gives gain=16

        if digigainen:
            digigain_db = -3+(digigainsel*0.25)
        else:
            digigain_db = 0
        digigain_db += 6*digigaindouble-6*digigainhalf

        # For consistency / simplicity, let's treat the rssi_adjust_db  output from the calculator like RAIL handles
        # EFR32_FEATURE_SW_CORRECTED_RSSI_OFFSET in that the value is thought to be added to the RSSI
        # So to compensate for the digital gain, the value should be the negative of the excess gain
        # Note that RSSISHIFT is actually subtracted from the RSSI, but EFR32_FEATURE_SW_CORRECTED_RSSI_OFFSET is
        # subtracted from the default RSSISHIFT so that the proper sign is maintained
        rssi_dig_adjust_db = -(dec0_gain_db + dec1_gain_db + digigain_db)

        #Write the vars
        model.vars.rssi_dig_adjust_db.value = rssi_dig_adjust_db

    def calc_rssi_rf_adjust_db(self, model):
        model.vars.rssi_rf_adjust_db.value = 0.0

    def calc_rssi_adjust_db(self, model):

        #Read in model vars
        rssi_dig_adjust_db = model.vars.rssi_dig_adjust_db.value
        rssi_rf_adjust_db = model.vars.rssi_rf_adjust_db.value

        #Add digital and RF adjustments
        rssi_adjust_db = rssi_dig_adjust_db + rssi_rf_adjust_db

        #Write the model var
        model.vars.rssi_adjust_db.value = rssi_adjust_db

    def calc_digmixfreq_reg(self, model):

        if_analog_hz = model.vars.if_frequency_hz_actual.value
        fxo = model.vars.fxo_or_fdec8.value
        dec0 = model.vars.dec0_actual.value

        digmixfreq = int(if_analog_hz * dec0 * pow(2, 20) / fxo)

        if digmixfreq > pow(2, 20) - 1:
            digmixfreq = pow(2, 20) - 1

        """From series 2 onwards, using negative DIGMIXFREQ for low-side injection (See https://jira.silabs.com/browse/MCUW_RADIO_CFG-1906)"""
        if model.vars.lo_injection_side.value == model.vars.lo_injection_side.var_enum.LOW_SIDE:
            digmixfreq *= -1

        self._reg_write(model.vars.MODEM_DIGMIXCTRL_DIGMIXFREQ, digmixfreq, allow_neg=True,
                        neg_twos_comp=True)

    def calc_if_center_digital_hz_actual(self, model):
        """Need to accomodate negative DIGMIXFREQ value for low-side injection"""
        fxo = model.vars.fxo_or_fdec8.value
        dec0 = model.vars.dec0_actual.value
        cfosr = model.vars.cfosr_actual.value
        
        if model.vars.lo_injection_side.value == model.vars.lo_injection_side.var_enum.HIGH_SIDE:
            digmixfreq = model.vars.MODEM_DIGMIXCTRL_DIGMIXFREQ.value
        else:
            model_var = model.vars.MODEM_DIGMIXCTRL_DIGMIXFREQ
            digmixfreq_regsize = model_var.rm.bitWidth
            digmixfreq = (2 ** digmixfreq_regsize) - model.vars.MODEM_DIGMIXCTRL_DIGMIXFREQ.value

        digmixmode = model.vars.MODEM_DIGMIXCTRL_DIGMIXMODE.value

        if digmixmode:
            if_frequency = fxo * digmixfreq / (dec0 * pow(2, 20))
        else:
            if_frequency = fxo / (dec0 * cfosr)

        model.vars.if_center_digital_hz_actual.value = int(if_frequency)

    # Add this function to modify the behavior of interpolation_gain provided to Ocelot (see MCUW_RADIO_CFG-3149)
    def calc_interpolation_gain_actual(self, model):
        #This function calculates the actual interpolation gain

        #Load model variables into local variables
        txbrnum = model.vars.MODEM_TXBR_TXBRNUM.value
        modformat = model.vars.modulation_type.value

        if txbrnum < 256:
            interpolation_gain = txbrnum / 1.0
        elif modformat == model.vars.modulation_type.var_enum.BPSK or \
             modformat == model.vars.modulation_type.var_enum.DBPSK:
            interpolation_gain = 16 * txbrnum * 2 ** (3-floor(log(txbrnum, 2)))
        elif txbrnum < 512:
            interpolation_gain = txbrnum / 2.0
        elif txbrnum < 1024:
            interpolation_gain = txbrnum / 4.0
        elif txbrnum < 2048:
            interpolation_gain = txbrnum / 8.0
        elif txbrnum < 4096:
            interpolation_gain = txbrnum / 16.0
        elif txbrnum < 8192:
            interpolation_gain = txbrnum / 32.0
        elif txbrnum < 16384:
            interpolation_gain = txbrnum / 64.0
        elif txbrnum < 32768:
            interpolation_gain = txbrnum / 128.0
        else:
            interpolation_gain = txbrnum / 256.0

        # calculate phase interpolation gain for OQPSK cases
        if modformat == model.vars.modulation_type.var_enum.OQPSK:
            interpolation_gain = 2 ** (ceil(log(interpolation_gain, 2)))

        #Load local variables back into model variables
        model.vars.interpolation_gain_actual.value = float(interpolation_gain)