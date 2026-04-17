from pyradioconfig.parts.bobcat.calculators.calc_demodulator import Calc_Demodulator_Bobcat
from pycalcmodel.core.variable import ModelVariableFormat, CreateModelVariableEnum
from enum import Enum
from math import *
import numpy as np
import numpy.matlib
from scipy import signal as sp
from pyradioconfig.parts.rainier.calculators.calc_shaping import CalcShapingRainier
from pyradioconfig.calculator_model_framework.Utils.LogMgr import LogMgr

class CalcDemodulatorRainier(Calc_Demodulator_Bobcat):

    # override inhereited variable defining threshold for _channel_filter_clocks_valid
    chf_required_clks_per_sample = 3  # fixed in PGVIPER-248

    def buildVariables(self, model):
        """Populates a list of needed variables for this calculator
        Args:
           model (ModelRoot) : Builds the variables specific to this calculator
        """
        super().buildVariables(model)
        self._build_rxdc_variables(model)
        self._addModelVariable(model, 'synchronous_ifadc_clk', bool, ModelVariableFormat.DECIMAL,
                               desc='Flag used treat ifadc_clk as synchronous to clk_demod and bypass afifo')
        member_data = [
            ['NONE', 0xFF, 'None'],
            ['STANDARD', 0, 'Standard'],
            ['ANTDIV', 1, 'Antenna Diversity'],
            ['COEX', 2, 'Antenna Diversity'],
            ['ANTDIV_COEX', 3, 'Antenna Diversity'],
            ['FEM', 8, 'External LNA'],
            ['ANTDIV_FEM', 9, 'Antenna Diversity with External LNA'],
            ['COEX_FEM', 10, 'External LNA'],
            ['ANTDIV_COEX_FEM', 11, 'External LNA'],
            ['HDR_2M', 13, 'Mode Switch to 2M'],
            ['FCS', 14, 'Fast channel switch'],
            ['HDR_1M_FEC', 15, 'Mode Switch to 1M with FEC'],
            ['FCS_HDR_2M', 16, 'Fast Channel Switching and 2M'],
            ['FCS_HDR_1M_FEC', 17, 'Fast Channel Switching and 1M with FEC'],
            ['RXDC', 18, 'Rx Duty Cycling'],
            ['GB868_863', 133, 'UK Metering 863 MHz Band'],
            ['GB868_915', 134, 'UK Metering 915 MHz Band'],
            ['NA915_R23', 135, 'NA R23 915 MHz Band'],
            # We use the thousands place (decimal) to indicate different PHY (different demod) for same feature
            # See https://jira.silabs.com/browse/MCUW_RADIO_CFG-2862
            ['LEGACY', 1000, 'Legacy Demod'],
            ['COHERENT', 2000, 'Coherent Demod'],
            ['ENHANCED', 3000, 'Enhanced Demod'],
        ]
        model.vars.zigbee_feature.var_enum = CreateModelVariableEnum(
            'ZigbeeFeatureEnum',
            'List of supported zigbee PHY features',
            member_data)

        member_data = [
            ['NONE', 0xFF, 'None'],
            ['LE_1M', 0, 'Bluetooth LE 1Mbps'],
            ['LE_2M', 1, 'Bluetooth LE 2Mbps'],
            ['CODED_125K', 2, 'Bluetooth LE Coded 125Kbps'],
            ['CODED_500K', 3, 'Bluetooth LE Coded 500Kbps'],
            ['CONCURRENT', 4, 'Bluetooth Concurrent'],
            ['AOX_2M', 5, 'Bluetooth LE AoX 2Mbps'],
            ['HADM_1M', 7, 'Bluetooth LE HADM (Channel Sounding) 1Mbps'],
            ['HADM_2M', 8, 'Bluetooth LE HADM (Channel Sounding) 2Mbps'],
            ['AOX_1M', 9, 'Bluetooth LE AoX 1Mbps'],
            ['HADM_2M_2BT', 10, 'Bluetooth LE AoX 2Mbps 2BT'],
            ['FASTSW', 12, 'Fast switching'],
        ]
        model.vars.ble_feature.var_enum = CreateModelVariableEnum(
            'BleFeatureEnum',
            'List of supported Bluetooth LE PHY features',
            member_data)

    def _build_rxdc_variables(self, model):
        """Build Rx Duty Cycle variables:

        rxdc_power_save_mode --> controls which blocks (Demod, RF, Synth) are duty cycled (default: DISABLED).
        rxdc_power_save_time_us --> controls the duration of power saving time.
        rxdc_on_time_us --> Calculated RxDC ON time. During this period, the duty cycled blocks are ON.
        rxdc_off_time_us --> Calculated RxDC OFF time. During this period, the duty cycled blocks are OFF."""

        self._addModelVariable(model, 'rxdc_power_save_mode', Enum, ModelVariableFormat.DECIMAL, 'Rx Duty Cycle mode')
        member_data = [
            ['DISABLED', 0, 'Rx Duty Cycle is disabled (default)'],
            ['DEMOD', 1, 'Demod block is duty-cycled'],
            ['RF', 2, 'Demod and RF blocks are duty-cycled'],
            ['SYNTH', 3, 'Demod, RF and Synth blocks are duty-cycled'],
        ]
        model.vars.rxdc_power_save_mode.var_enum = CreateModelVariableEnum(
            'RxDutyCycleEnum',
            'List of supported duty cycle modes',
            member_data)
        self._addModelVariable(model, 'rxdc_power_save_time_us', int, ModelVariableFormat.DECIMAL, 'RxDC power save time in us')
        self._addModelVariable(model, 'rxdc_on_time_us_actual', int, ModelVariableFormat.DECIMAL, 'Calculated RxDC ON time in us')
        self._addModelVariable(model, 'rxdc_off_time_us_actual', int, ModelVariableFormat.DECIMAL, 'Calculated RxDC OFF time in us')

    def _add_demod_select_variable(self, model):

        #Add ENHANCED_DSSS demod
        model.vars.demod_select.var_enum = CreateModelVariableEnum(
            enum_name='DemodSelectEnum',
            enum_desc='Demod Selection',
            member_data=[
                ['LEGACY', 0, 'Legacy Demod'],
                ['COHERENT', 1, 'Coherent Demod'],
                ['TRECS_VITERBI', 2, 'TRecS + Viterbi Demod'],
                ['TRECS_SLICER', 3, 'TRecS + HD Demod'],
                ['BCR', 4, 'PRO2 BCR Demod'],
                ['LONGRANGE', 5, 'BLE Long Range Demod'],
                ['ENHANCED_DSSS', 6, 'Enhanced OQPSK+DSSS Demod'],
                ['HDT', 8, 'BLE Higher Data Rate Demod']
            ])

    def get_limits(self, demod_select, withremod, relaxsrc2, model):

        if demod_select == model.vars.demod_select.var_enum.ENHANCED_DSSS or \
                demod_select == model.vars.demod_select.var_enum.HDT:
            osr_list = [4]
            min_osr = 4
            max_osr = 4
            min_src2 = 0.8
            max_src2 = 1.2
            min_dec2 = 1
            max_dec2 = 1
            min_bwsel = 0.15
            target_bwsel = 0.4
            max_bwsel = 0.4
            min_chfilt_osr = None
            max_chfilt_osr = None

        else:
            min_bwsel, max_bwsel, min_chfilt_osr, max_chfilt_osr, min_src2, max_src2, min_dec2, max_dec2, min_osr, max_osr, target_bwsel, osr_list = \
                super().get_limits(demod_select, withremod, relaxsrc2, model)

        # save to use in other functions
        model.vars.min_bwsel.value = min_bwsel  # min value for normalized channel filter bandwidth
        model.vars.max_bwsel.value = max_bwsel  # max value for normalized channel filter bandwidth
        model.vars.min_src2.value = min_src2  # min value for SRC2
        model.vars.max_src2.value = max_src2  # max value for SRC2
        model.vars.max_dec2.value = max_dec2
        model.vars.min_dec2.value = min_dec2

        return min_bwsel, max_bwsel, min_chfilt_osr, max_chfilt_osr, min_src2, max_src2, min_dec2, max_dec2, min_osr, max_osr, target_bwsel, osr_list

    def calc_syncacqwin_actual(self, model):
        """ set syc word acquisition window for TRECS basd on register value

        Args:
            model (ModelRoot) : Data model to read and write variables from
        """
        model.vars.syncacqwin_actual.value = (model.vars.MODEM_REALTIMCFE_SYNCACQWIN.value + 1)

    #
    def calc_syncbits_actual(self, model):
        syncword_bits = model.vars.syncword_length.value ##fix me
        model.vars.syncbits_actual.value = syncword_bits

    def calc_target_demod_bandwidth(self, model):

        demod_select = model.vars.demod_select.value
        baudrate = model.vars.baudrate.value

        if demod_select == model.vars.demod_select.var_enum.ENHANCED_DSSS:

            model.vars.demod_bandwidth_hz.value = int(baudrate * 0.9)
        else:
            super().calc_target_demod_bandwidth(model)

    def calc_lock_bandwidth(self, model, softmodem_narrowing=False):

        demod_select = model.vars.demod_select.value
        bw_demod = model.vars.demod_bandwidth_hz.value

        if demod_select == model.vars.demod_select.var_enum.ENHANCED_DSSS:

            model.vars.lock_bandwidth_hz.value = int(bw_demod)
        else:
            super().calc_lock_bandwidth(model)

    def return_osr_dec0_dec1(self, model, demod_select, withremod=False, relaxsrc2=False, quitatfirstvalid=True):

        # Load model variables into local variables
        bandwidth = model.vars.bandwidth_hz.value  # from calc_target_bandwidth
        adc_freq = model.vars.adc_freq_actual.value
        baudrate = model.vars.baudrate.value  # We don't know the actual bandrate yet
        modtype = model.vars.modulation_type.value
        mi = model.vars.modulation_index.value
        if_frequency_hz = model.vars.if_frequency_hz.value
        etsi_cat1_compatability = model.vars.etsi_cat1_compatible.value
        bw_var = model.vars.bandwidth_tol.value
        xtal_frequency_hz = model.vars.xtal_frequency_hz.value

        # set limits based on selected demod
        [min_bwsel, max_bwsel, min_chfilt_osr, max_chfilt_osr, min_src2, max_src2, min_dec2, \
            max_dec2, min_osr, max_osr, target_bwsel, osr_list] = self.get_limits(demod_select, withremod, relaxsrc2, model)

        # read from advanced variable
        if (model.vars.target_bwsel._value_forced != None):
            target_bwsel = model.vars.target_bwsel._value_forced

        # initialize output
        best_bwsel_error = 1e9
        best_osr = 0
        best_dec0 = 0
        best_dec1 = 0

        # Setup for osr loop
        # osr_list is a prioritized list, where first value with valid config will be returned
        if (model.vars.target_osr._value_forced != None):
            osr_forced = model.vars.target_osr._value_forced
            osr_list = [osr_forced]

        # Setup for dec0 loop
        # dec0_list is a prioritized list, where ties in best bwsel go to earlier value in list
        dec0_list = self.return_dec0_list(if_frequency_hz,adc_freq)

        # Search values of osr, dec0, dec1 to find solution
        # Exit on first osr with valid dec0 and dec1
        for osr in osr_list:
            for dec0 in dec0_list:

                # define integer range for dec1
                min_dec1 = int(max(1, ceil(float(adc_freq) * min_bwsel / (8 * dec0 * bandwidth*(1+bw_var)))))
                max_dec1 = int(min(11500, floor(float(adc_freq) * max_bwsel / (8 * dec0 * bandwidth*(1-bw_var)))))
                if min_dec1 <= max_dec1:
                    # Order list from highest to lowest, bwsel from highest to lowest
                    dec1_list = range(max_dec1,min_dec1-1,-1)
                else:
                    # No solution
                    continue

                for dec1 in dec1_list:

                    # check configuration does trigger IPMCUSRW-876 channel filter issue when input sample rate
                    # is too fast relative to the processing clock cycles needed
                    # if not self._channel_filter_clocks_valid(model, dec0, dec1):
                    #     continue

                    # calculated dec2 range
                    if demod_select == model.vars.demod_select.var_enum.BCR:
                        calc_min_dec2 = 1
                        calc_max_dec2 = 1
                        chfilt_osr_actual = float(adc_freq) / (8 * dec0 * dec1 * baudrate)

                        if (modtype == model.vars.modulation_type.var_enum.OOK) or \
                                (modtype == model.vars.modulation_type.var_enum.ASK):
                            if chfilt_osr_actual < osr or chfilt_osr_actual > osr + 1.0:
                                continue
                        else:
                            if (chfilt_osr_actual < min_chfilt_osr) or (chfilt_osr_actual > max_chfilt_osr):
                                # not a solution, next value of dec1 loop
                                continue

                    elif demod_select == model.vars.demod_select.var_enum.TRECS_SLICER or \
                         demod_select == model.vars.demod_select.var_enum.TRECS_VITERBI:
                        # forcing integer osr condition, which applies to TRECS
                        # check for TRECS minimum clk cycle requirements
                        calc_min_dec2 = ceil(min_src2 * float(adc_freq) / (osr * dec0 * dec1 * 8 * baudrate))
                        calc_max_dec2 = floor(max_src2 * float(adc_freq) / (osr * dec0 * dec1 * 8 * baudrate))

                        # trecs_src_interp_okay = self._check_trecs_required_clk_cycles(adc_freq, baudrate, osr, dec0,
                        #                                                              dec1, xtal_frequency_hz, relaxsrc2, model)
                        # if not trecs_src_interp_okay:
                        #     # not a solution due to trecs clocking constraints, continue
                        #     continue
                    else:
                        # forcing integer osr condition, which applies to LEGACY, COHERENT
                        calc_min_dec2 = ceil(min_src2 * float(adc_freq) / (osr * dec0 * dec1 * 8 * baudrate))
                        calc_max_dec2 = floor(max_src2 * float(adc_freq) / (osr * dec0 * dec1 * 8 * baudrate))

                    if (calc_min_dec2 <= calc_max_dec2) and (calc_min_dec2 <= max_dec2) and \
                            (calc_max_dec2 >= min_dec2):

                        # calculation of dec1 has constrained bwsel to range bwsel_min to bwsel_max
                        bwsel = bandwidth * (8 * dec0 * dec1) / float(adc_freq)

                        bwsel_error = abs(bwsel - target_bwsel)

                        # Select largest bwsel as best result
                        if (bwsel_error < best_bwsel_error):
                            best_bwsel_error = bwsel_error
                            best_osr = osr
                            best_dec0 = dec0
                            best_dec1 = dec1
                            best_bwsel = bwsel

            if best_osr > 0 and quitatfirstvalid:

                # break out of the osr loop on first successful configuration
                break

        return best_osr, best_dec0, best_dec1, min_osr, max_osr

    def calc_synchronous_ifadc_clk(self, model):
        # default to disabled, only HADM PHYs will set the profile input
        model.vars.synchronous_ifadc_clk.value = False

    def calc_synchronous_ifadc_clk_regs(self, model):
        if not model.vars.synchronous_ifadc_clk.value:
            # normal async front filtering
            self._reg_write(model.vars.MODEM_CTRL5_FEFILTCLKSEL, 0)
            self._reg_write(model.vars.MODEM_CTRL5_AFIFOBYP, 0)
        else:
            self._reg_write(model.vars.MODEM_CTRL5_FEFILTCLKSEL, 1)
            self._reg_write(model.vars.MODEM_CTRL5_AFIFOBYP, 1)

    def calc_log2x4fwsel_reg(self, model):
        # set to use LOG2X4 register value for log2 calculation in DEC1 rather than HW calculation
        self._reg_write(model.vars.MODEM_LOG2X4_LOG2X4FWSEL, 1)

    def calc_log2x4_reg(self, model):
        # given DEC1 decimation ratio calculate log2 in 4Q2 format
        dec1 = model.vars.dec1_actual.value

        val = floor(4.0*log2(dec1))

        self._reg_write(model.vars.MODEM_LOG2X4_LOG2X4, val)

    def calc_log2x4_actual(self, model):
        # Read in model vars
        log2x4fwsel = model.vars.MODEM_LOG2X4_LOG2X4FWSEL.value
        log2x4_reg = model.vars.MODEM_LOG2X4_LOG2X4.value
        dec1_actual = model.vars.dec1_actual.value

        if log2x4fwsel == 1:
            log2x4_actual = log2x4_reg
        else:
            log2x4_actual = floor(4*log2(dec1_actual))

        # Write the model var
        model.vars.log2x4_actual.value = log2x4_actual

    def calc_modeminfo_reg(self, model):
        # See https://jira.silabs.com/browse/RAIL_LIB-11922
        coherent_used = model.vars.MODEM_CTRL1_PHASEDEMOD.value == 2
        trecs_used = model.vars.MODEM_VITERBIDEMOD_VTDEMODEN.value
        longrange_used = model.vars.MODEM_LONGRANGE_LRBLE.value
        enhdsss_used = model.vars.MODEM_EHDSSSCTRL_EHDSSSEN.value
        hop_enable = model.vars.hop_enable.value == model.vars.hop_enable.var_enum.ENABLED
        rxdc_enable = model.vars.rxdc_power_save_mode.value != model.vars.rxdc_power_save_mode.var_enum.DISABLED
        protocol_is_zb = (model.vars.protocol_id.value == model.vars.protocol_id.var_enum.Zigbee) and \
                         (model.vars.ble_feature.value == model.vars.ble_feature.var_enum.NONE) # check if 2ZB and not BLE2ZB

        rxdc2ch_used = 0
        rxdc1ch_used = 0
        fsw_used = 0
        if hop_enable and rxdc_enable:  # Dual channel duty Cycle
            rxdc2ch_used = 1
        elif hop_enable and protocol_is_zb:     # 2ZB uses same algorithm as 2ZBDC
            rxdc2ch_used = 1
        elif rxdc_enable:  # Single channel duty Cycle
            rxdc1ch_used = 1
        elif hop_enable:  # Dual channel no duty cycle
            fsw_used = 1

        if trecs_used or coherent_used or longrange_used or enhdsss_used:
            legacy_used = 0
        else:
            legacy_used = 1

        # Unused in Rainier
        bcr_used = 0
        soft_modem_used = 0
        btc_used = 0

        self._reg_write(model.vars.SEQ_MODEMINFO_LEGACY_EN, int(legacy_used))
        self._reg_write(model.vars.SEQ_MODEMINFO_RXDC1CH_EN, int(rxdc1ch_used))
        self._reg_write(model.vars.SEQ_MODEMINFO_TRECS_EN, int(trecs_used))
        self._reg_write(model.vars.SEQ_MODEMINFO_BCR_EN, int(bcr_used))
        self._reg_write(model.vars.SEQ_MODEMINFO_COHERENT_EN, int(coherent_used))
        self._reg_write(model.vars.SEQ_MODEMINFO_LONGRANGE_EN, int(longrange_used))
        self._reg_write(model.vars.SEQ_MODEMINFO_ENHDSSS_EN, int(enhdsss_used))
        self._reg_write(model.vars.SEQ_MODEMINFO_FSW_EN, int(fsw_used))
        self._reg_write(model.vars.SEQ_MODEMINFO_SOFTMODEM_EN, int(soft_modem_used))
        self._reg_write(model.vars.SEQ_MODEMINFO_BTC_EN, int(btc_used))
        self._reg_write(model.vars.SEQ_MODEMINFO_RXDC2CH_EN, int(rxdc2ch_used))
        self._reg_write(model.vars.SEQ_MODEMINFO_SPARE0, 0)

    def calc_fswcoreinfo_reg(self, model):
        # See https://jira.silabs.com/browse/RAIL_LIB-11922
        rxdc_power_save_mode = model.vars.rxdc_power_save_mode.value
        rxdc_power_save_time = model.vars.rxdc_power_save_time_us.value

        reg = rxdc_power_save_time | rxdc_power_save_mode << 16

        self._reg_write(model.vars.SEQ_FSWCOREINFO_CONFIG, reg)

    def return_ksi2_ksi3_calc(self, model, ksi1):
        # get parameters
        lock_bwsel = model.vars.lock_bwsel.value # use the lock bw
        bwsel = model.vars.bwsel.value  # use the lock bw
        osr = int(round(model.vars.oversampling_rate_actual.value))
        shaping_filter = model.vars.shaping_filter.value
        # not using this model variable but keeping it here. Will remove it once, sure.
        # shaping_coeff_override = model.vars.shaping_coeff_ksi_calc_override.value

        # calculate only if needed - ksi1 would be already calculated if that is the case
        if (ksi1 == 0):
            best_ksi2 = 0
            best_ksi3 = 0
            best_ksi3wb = 0
        else:
            # get shaping filter and it oversampling rate with respect to baudrate

            # If no shaping_filter is a generic shape, use generic shape values for ksi calculation.
            # If shaping_filter is custom, assumption is that shaping_filter will be set to NONE, and manual overrides
            # will be used. In that case, check if there are any non-zero shaping coeffs, and use those coeffs for
            # ksi calculation
            sf = CalcShapingRainier().get_shaping_filter(model)/1.0
            if shaping_filter.value == model.vars.shaping_filter._var_enum.NONE.value:
                if np.count_nonzero(sf) == 64:
                    LogMgr.Error("ERROR: No pulse shaping being used, please select a pulse shape in shaping_filter")
            else:
                # if shaping_filter is set to a generic shape (e.g. gaussian, rc, rrc),
                # use generic filter to calculate ksi
                sf, shaping = CalcShapingRainier().run_shaping_filter_calc(model)
            sfosr = 8 # shaping filter coeffs are sampled at 8x

            # get channel filter and expend the symmetric part
            cfh = np.asarray(self.return_coeffs(lock_bwsel))
            cf = np.block([cfh, cfh[-2::-1]])/1.0
            cfh = np.asarray(self.return_coeffs(bwsel))
            cfwb = np.block([cfh, cfh[-2::-1]])/1.0

            # base sequences for +1 and -1
            a = np.array([ 1.0, 0, 0, 0, 0, 0, 0, 0])
            b = np.array([-1.0, 0, 0, 0, 0, 0, 0, 0])

            # generate frequency signal for periodic 1 1 1 0 0 0 sequence for ksi1
            x1 = np.matlib.repmat(np.append(np.matlib.repmat(a, 1, 3),np.matlib.repmat(b, 1, 3)), 1, 4)
            f1 = self.gen_frequency_signal( x1[0], sf, cf, sfosr, model)

            # generate frequency signal for periodic 1 1 0 0 1 1 sequence for ksi2
            x2 = np.matlib.repmat(np.append(np.matlib.repmat(a, 1, 2), np.matlib.repmat(b, 1, 2)), 1, 6)
            f2 = self.gen_frequency_signal( x2[0], sf, cf, sfosr, model)

            # generate frequency signal for periodic 1 0 1 0 1 0 sequence for ksi3
            x3 = np.matlib.repmat(np.append(np.matlib.repmat(a, 1, 1), np.matlib.repmat(b, 1, 1)), 1, 12)
            f3 = self.gen_frequency_signal( x3[0], sf, cf, sfosr, model)

            # generate frequency signal for periodic 1 0 1 0 1 0 sequence for ksi3 but with aqcusition channel filter
            f3wb = self.gen_frequency_signal( x3[0], sf, cfwb, sfosr, model)

            # find scaling needed to get f1 to the desired ksi1 value and apply it to f2 and f3
            ind = osr - 1
            scaler = ksi1 / np.max(np.abs(f1[ind + 8 * osr - 1: - 2 * osr: osr]))
            f2 = scaler * f2
            f3 = scaler * f3
            f3wb = scaler * f3wb

            # from matplotlib import pyplot as plt
            # plt.plot(f1*scaler,'x-')
            # plt.show()
            # plt.plot(f2,'x-')
            # plt.plot(f3,'x-')
            # plt.plot(f3wb,'x-')

            # search for best phase to sample to get ksi3 value.
            # best phase is the phase that gives largest eye opening
            best_ksi3 = 0
            for ph in range(osr):
                ksi3 = np.max(np.round(np.abs(f3[ - 6 * osr + ph: - 2 * osr: osr])))
                if ksi3 > best_ksi3:
                    best_ksi3 = ksi3

            best_ksi3wb = 0
            for ph in range(osr):
                ksi3wb = np.max(np.round(np.abs(f3wb[ - 6 * osr + ph: - 2 * osr: osr])))
                if ksi3wb > best_ksi3wb:
                    best_ksi3wb = ksi3wb

            # ksi2 is tricky depending if we sampled perfectly (symmetric around a
            # pulse we should see the same value for 1 1 0 and 0 1 1 sequence but
            # most of the time we cannot sample perfectly since can go as low as 4x
            # oversampling for Viterbi PHYs. In this case we have 2 ksi values which we
            # average to get the ksi2 value
            best_cost = 1e9
            for ph in range(osr):
                x = np.round(np.abs(f2[- 6 * osr + ph: - 2 * osr: osr]))
                cost = np.sum(np.abs(x - np.mean(x)))
                if cost < best_cost:
                    best_cost = cost
                    best_ksi2 = np.round(np.mean(x))

        # ensure that ksi1 >= ksi2 >= ksi3
        # this code should only be needed in the extreme case when ksi1 = ksi2 = ksi3 and
        # small variation can cause one to be larger than the other
        best_ksi2 = ksi1 if best_ksi2 > ksi1 else best_ksi2
        best_ksi3 = best_ksi2 if best_ksi3 > best_ksi2 else best_ksi3
        best_ksi3wb = best_ksi2 if best_ksi3wb > best_ksi2 else best_ksi3wb

        return best_ksi2, best_ksi3, best_ksi3wb

    def calc_rssi_rf_adjust_db(self, model):
        model.vars.rssi_rf_adjust_db.value = -15.8

    def calc_dsss_concurrent_reg(self, model):
        trecs_used = model.vars.MODEM_VITERBIDEMOD_VTDEMODEN.value
        enhdsss_used = model.vars.MODEM_EHDSSSCTRL_EHDSSSEN.value
        # https://jira.silabs.com/browse/MCUW_RADIO_CFG-2587
        # making this calculation makes sure concurrent detection is disabled for standalone PHYs
        if trecs_used and enhdsss_used:
            self._reg_write(model.vars.MODEM_COCURRMODE_DSSSCONCURRENT, 1)
        else:
            self._reg_write(model.vars.MODEM_COCURRMODE_DSSSCONCURRENT, 0)

    def calc_softd_reg(self, model):
        trecs_used = model.vars.MODEM_VITERBIDEMOD_VTDEMODEN.value
        fec_enabled = model.vars.fec_enabled.value
        is_ble_longrange = model.vars.MODEM_LONGRANGE_LRBLE.value
        # https://jira.silabs.com/browse/MCUW_RADIO_CFG-2587
        # For the most PHYs, FEC is not selected in PHY's definition. So, TRECSCFG_SOFTD = 0 is as the default.
        # and hardcode to Viterbi decoder in FRC. So, we set TRECSCFG_SOFTD = 1 to get better performance.

        if not(is_ble_longrange) and (trecs_used and fec_enabled):
            self._reg_write(model.vars.MODEM_TRECSCFG_SOFTD, 1)
        else:
            self._reg_write(model.vars.MODEM_TRECSCFG_SOFTD, 0)

    def calc_freq_gain_virtual_reg(self,model):
        freqgain_e = model.vars.MODEM_MODINDEX_FREQGAINE.value
        freqgain_m = model.vars.MODEM_MODINDEX_FREQGAINM.value
        self._reg_write(model.vars.SEQ_MODINDEX_CALC_FREQGAINM, freqgain_m)
        self._reg_write(model.vars.SEQ_MODINDEX_CALC_FREQGAINE, freqgain_e)
        self._reg_write(model.vars.SEQ_MODINDEX_CALC_MODINDEXE_DOUBLED_FREQGAINM, freqgain_m)
        self._reg_write(model.vars.SEQ_MODINDEX_CALC_MODINDEXE_DOUBLED_FREQGAINE, freqgain_e)

    def calc_chmutetimer_reg(self, model):
        hop_enable = model.vars.hop_enable.value == model.vars.hop_enable.var_enum.ENABLED
        duty_cycled = model.vars.rxdc_power_save_mode.value != model.vars.rxdc_power_save_mode.var_enum.DISABLED

        if duty_cycled:
            chmutetimer = 280  # For BLE: 280 - For ZB: 170
        elif hop_enable:
            chmutetimer = 195           # Default value for 2ZB hopping
        else:
            chmutetimer = 0

        self._reg_write(model.vars.MODEM_SRCCHF_CHMUTETIMER, chmutetimer)

    def calc_fast_switching_regs(self, model):
        hop_enable = True if model.vars.hop_enable.value == model.vars.hop_enable.var_enum.ENABLED else False
        protocol_id = model.vars.protocol_id.value
        ble2zb_hop = model.vars.MODEM_COCURRMODE_DSSSCONCURRENT.value

        # Enable DTIMLOSS feature for BLE2ZB fast switching PHYs
        # See https://jira.silabs.com/browse/MCUW_RADIO_CFG-2525
        if hop_enable and ble2zb_hop and protocol_id == model.vars.protocol_id.var_enum.Zigbee:
            self._reg_write(model.vars.MODEM_TRECSCFG_DTIMLOSSEN, 1)
            self._reg_write(model.vars.MODEM_TRECSCFG_DTIMLOSSTHD, 200)
        elif hop_enable and ble2zb_hop and protocol_id == model.vars.protocol_id.var_enum.BLE:
            self._reg_write(model.vars.MODEM_TRECSCFG_DTIMLOSSEN, 1)
            self._reg_write(model.vars.MODEM_TRECSCFG_DTIMLOSSTHD, 1000)
        else:
            self._reg_write(model.vars.MODEM_TRECSCFG_DTIMLOSSEN, 0)
            self._reg_write(model.vars.MODEM_TRECSCFG_DTIMLOSSTHD, 0)

    def calc_spare_regs(self, model):
        hop_enable = model.vars.hop_enable.value == model.vars.hop_enable.var_enum.ENABLED

        # revB0+ SPARE register is defined as follows:
        # dsss_dsa_unqualified_sel=  spare[0]
        # lr_eof_sel_force0=         spare[1]
        # lrble_compound_muxctrl=    spare[2]

        if hop_enable:
            # : if fixed timeout is used to control hopping, the hopping controller requires unqualified DSA signal
            # : Qualified DSA signal may take longer than 32 us timeout, causing an incorrect channel switch at the end
            # : of the timeout.
            dsss_dsa_unqualified_sel = 1
        else:
            dsss_dsa_unqualified_sel = 0

        # : if additional spare fields needs to be set, bitshift the values here
        reg_val = dsss_dsa_unqualified_sel

        self._reg_write(model.vars.MODEM_SPARE_SPARE, reg_val)

    def calc_rx_duty_cycle_vars(self, model):
        model.vars.rxdc_power_save_time_us.value = 0
        model.vars.rxdc_power_save_mode.value = model.vars.rxdc_power_save_mode.var_enum.DISABLED
        model.vars.rxdc_on_time_us_actual.value = 0
        model.vars.rxdc_off_time_us_actual.value = 0

    def calc_fasthopping_regs(self, model):
        duty_cycled = model.vars.rxdc_power_save_mode.value is not model.vars.rxdc_power_save_mode.var_enum.DISABLED
        hop_enabled = model.vars.hop_enable.value == model.vars.hop_enable.var_enum.ENABLED
        noise_det_en = model.vars.MODEM_SICTRL0_SIMODE.value != 0

        fasthoppingen = 0       # FASTHOPPINGEN should be controlled by firmware - we should remove this bit

        if hop_enabled:
            fwhopping = 0

            if noise_det_en:
                hoppingsrc = 0
            else:
                hoppingsrc = 2

        else:
            hoppingsrc = 0

            if noise_det_en and duty_cycled:
                fwhopping = 1  # Required for NOISEDET IF to trigger ISR for duty-cycle + noise detector
            else:
                fwhopping = 0

        self._reg_write(model.vars.MODEM_PHDMODCTRL_FASTHOPPINGEN, fasthoppingen)
        self._reg_write(model.vars.MODEM_DIGMIXCTRL_HOPPINGSRC, hoppingsrc)
        self._reg_write(model.vars.MODEM_DIGMIXCTRL_FWHOPPING, fwhopping)

    # Due to the inheriting the CALC_Demodulator_bobcat class from prior parts, the functions below are copied in order to maintain functionality
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
        else:
            interpolation_gain = txbrnum / 128.0
        
        # following the rtl
        interpolation_gain = floor(interpolation_gain)

        # calculate phase interpolation gain for OQPSK cases
        if modformat == model.vars.modulation_type.var_enum.OQPSK:
            interpolation_gain = 2 ** (ceil(log(interpolation_gain, 2)))

        #Load local variables back into model variables
        model.vars.interpolation_gain_actual.value = float(interpolation_gain)

