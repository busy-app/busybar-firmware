from pyradioconfig.parts.common.phys.phy_common import PHY_COMMON_FRAME_154
from pyradioconfig.parts.lynx.phys.Phys_RAIL_Base_Standard_IEEE802154 import PhysRAILBaseStandardIEEE802154Lynx
from pyradioconfig.parts.common.phys.phy_common import PHY_COMMON_FRAME_INTERNAL
from pyradioconfig.parts.panther.phys.PHY_internal_base import Phy_Internal_Base
from py_2_and_3_compatibility import *


class PhysInternalBaseStandardIEEE802154Ocelot(PhysRAILBaseStandardIEEE802154Lynx):
    # For now, inherit all from Lynx

    # Manual copy/paste of sub-GHz 802.15.4 PHYs from Jumbo
    # These do not inherit from Lynx, as Lynx excludes sub-GHz
    #
    # Useful to copy here as many register overrides do not exist on Series 2

    def _set_xtal_frequency(self, phy, xtal_freq=None):
        if xtal_freq is None:
            phy.profile_inputs.xtal_frequency_hz.value = 38400000
        else:
            phy.profile_inputs.xtal_frequency_hz.value = xtal_freq

    def IEEE802154_Base(self, phy, model):
        # Inputs
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.preamble_pattern.value = 0
        phy.profile_inputs.preamble_pattern_len.value = 4
        phy.profile_inputs.rx_xtal_error_ppm.value = 0
        phy.profile_inputs.symbol_encoding.value = model.vars.symbol_encoding.var_enum.DSSS
        phy.profile_inputs.syncword_0.value = long(0xe5)
        phy.profile_inputs.syncword_1.value = long(0x0)
        phy.profile_inputs.syncword_length.value = 8
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        self._set_xtal_frequency(phy)

        # Add 15.4 Packet Configuration
        PHY_COMMON_FRAME_154(phy, model)

        self._part_specific_phy_overrides(phy, model)

    def PHY_IEEE802154_780MHz_OQPSK(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='IEEE 802.15.4 780MHz OQPSK', phy_name=phy_name)

        self.IEEE802154_Base(phy, model)

        phy.profile_inputs.agc_hysteresis.value = 0
        phy.profile_inputs.agc_power_target.value = -15
        phy.profile_inputs.bandwidth_hz.value = 1262400
        phy.profile_inputs.base_frequency_hz.value =  long(780000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 8000
        phy.profile_inputs.bitrate.value = 250000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.dsss_chipping_code.value =  long(0xA47C)
        phy.profile_inputs.dsss_len.value = 16
        phy.profile_inputs.dsss_spreading_factor.value = 4
        phy.profile_inputs.frequency_comp_mode.value = model.vars.frequency_comp_mode.var_enum.INTERNAL_ALWAYS_ON
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.OQPSK
        phy.profile_inputs.number_of_timing_windows.value = 3
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_1200KHz
        phy.profile_inputs.preamble_length.value = 32
        phy.profile_inputs.preamble_pattern_len.value = 4
        phy.profile_inputs.rssi_period.value = 7
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Custom_OQPSK
        phy.profile_inputs.shaping_filter_param.value = 0.0
        phy.profile_inputs.syncword_length.value = 8
        phy.profile_inputs.timing_detection_threshold.value = 35

        phy.profile_outputs.AGC_GAINSTEPLIM0_CFLOOPSTEPMAX.override = 4

        return phy

    def IEEE802154_915MHz_OQPSK(self, phy, model):
        #phy.profile_inputs.agc_power_target.value = -15 # : TODO remove
        phy.profile_inputs.bandwidth_hz.value = 1262400
        phy.profile_inputs.base_frequency_hz.value = long(906000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 8000
        phy.profile_inputs.bitrate.value = 250000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.dsss_chipping_code.value = long(0xA47C)
        phy.profile_inputs.dsss_len.value = 16
        phy.profile_inputs.dsss_spreading_factor.value = 4
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.OQPSK
        phy.profile_inputs.preamble_length.value = 32
        phy.profile_inputs.preamble_pattern.value = 0
        phy.profile_inputs.preamble_pattern_len.value = 4
        #phy.profile_inputs.rssi_period.value = 7 # : TODO remove
        phy.profile_inputs.rx_xtal_error_ppm.value = 25
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Custom_OQPSK
        phy.profile_inputs.shaping_filter_param.value = 0.0
        phy.profile_inputs.symbol_encoding.value = model.vars.symbol_encoding.var_enum.DSSS
        phy.profile_inputs.syncword_0.value = long(0xe5)
        phy.profile_inputs.syncword_1.value = long(0x0)
        phy.profile_inputs.syncword_length.value = 8
        #phy.profile_inputs.timing_detection_threshold.value = 35 # : TODO remove
        #phy.profile_inputs.timing_sample_threshold.value = 0 # : TODO remove
        phy.profile_inputs.tx_xtal_error_ppm.value = 25
        phy.profile_inputs.xtal_frequency_hz.value = 39000000

        # Add 15.4 Packet Configuration
        PHY_COMMON_FRAME_154(phy, model)


    def PHY_IEEE802154_915MHz_OQPSK(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='IEEE 802.15.4 915MHz OQPSK', phy_name=phy_name)

        self.IEEE802154_915MHz_OQPSK(phy, model)
        phy.profile_inputs.demod_select.value = model.vars.demod_select.var_enum.LEGACY
        phy.profile_inputs.number_of_timing_windows.value = 3

        return phy

    def PHY_IEEE802154_915MHz_OQPSK_coh(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='IEEE 802.15.4 915MHz OQPSK Coherent Demod', phy_name=phy_name, tags="-FPGA")

        self.IEEE802154_915MHz_OQPSK(phy, model)
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.demod_select.value = model.vars.demod_select.var_enum.COHERENT
        phy.profile_inputs.timing_detection_threshold.value = 35
        #phy.profile_outputs.AGC_AGCPERIOD0_MAXHICNTTHD.override = 13
        #phy.profile_outputs.AGC_AGCPERIOD0_PERIODHI.override = 14
        #phy.profile_outputs.AGC_AGCPERIOD1_PERIODLOW.override = 42
        #phy.profile_outputs.AGC_AGCPERIOD0_SETTLETIMERF.override = 14
        # phy.profile_outputs.AGC_CTRL0_DISPNGAINUP.override = 0
        # phy.profile_outputs.AGC_CTRL0_MODE.override = 2
        phy.profile_outputs.AGC_CTRL0_PWRTARGET.override = 20
        phy.profile_outputs.AGC_CTRL1_RSSIPERIOD.override = 3
        # phy.profile_outputs.AGC_CTRL2_REHICNTTHD.override = 7
        # phy.profile_outputs.AGC_CTRL2_SAFEMODE.override = 0
        # phy.profile_outputs.AGC_CTRL2_SAFEMODETHD.override = 3
        # phy.profile_outputs.AGC_CTRL3_IFPKDDEB.override = 1
        # phy.profile_outputs.AGC_CTRL3_IFPKDDEBPRD.override = 40
        # phy.profile_outputs.AGC_CTRL3_IFPKDDEBRST.override = 10
        # phy.profile_outputs.AGC_CTRL3_IFPKDDEBTHD.override = 1
        # phy.profile_outputs.AGC_CTRL3_RFPKDDEB.override = 0
        # phy.profile_outputs.AGC_CTRL3_RFPKDDEBTHD.override = 1
        # phy.profile_outputs.AGC_CTRL4_PERIODRFPKD.override = 4000
        # phy.profile_outputs.AGC_CTRL4_RFPKDPRDGEAR.override = 4
        # phy.profile_outputs.AGC_CTRL4_RFPKDSEL.override = 1
        # phy.profile_outputs.AGC_CTRL4_RFPKDSYNCSEL.override = 1
        # phy.profile_outputs.AGC_CTRL5_PNUPDISTHD.override = 32
        # phy.profile_outputs.AGC_CTRL5_PNUPRELTHD.override = 4
        # phy.profile_outputs.AGC_CTRL6_SEQRFPKDEN.override = 0
        # phy.profile_outputs.AGC_GAINRANGE_BOOSTLNA.override = 1
        # phy.profile_outputs.AGC_GAINRANGE_HIPWRTHD.override = 3
        # phy.profile_outputs.AGC_GAINRANGE_LATCHEDHISTEP.override = 0
        # phy.profile_outputs.AGC_GAINRANGE_PNGAINSTEP.override = 4
        # phy.profile_outputs.AGC_GAINSTEPLIM0_CFLOOPDEL.override = 50
        # phy.profile_outputs.AGC_GAINSTEPLIM0_HYST.override = 3
        # phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION0.override = 8
        # phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION1.override = 10
        # phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION2.override = 12
        # phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION3.override = 12
        # phy.profile_outputs.AGC_HICNTREGION1_HICNTREGION4.override = 13
        # phy.profile_outputs.AGC_MANGAIN_MANGAINIFPGA.override = 7
        # phy.profile_outputs.AGC_MANGAIN_MANGAINLNA.override = 1
        # phy.profile_outputs.AGC_MANGAIN_MANGAINPN.override = 1
        # phy.profile_outputs.AGC_STEPDWN_STEPDWN2.override = 3
        # phy.profile_outputs.AGC_STEPDWN_STEPDWN4.override = 3
        phy.profile_outputs.MODEM_AFCADJLIM_AFCADJLIM.override = 658
        phy.profile_outputs.MODEM_COH0_COHCHPWRTH1.override = 19
        phy.profile_outputs.MODEM_COH0_COHCHPWRTH2.override = 79
        phy.profile_outputs.MODEM_COH0_COHDYNAMICBBSSEN.override = 1
        phy.profile_outputs.MODEM_COH0_COHDYNAMICPRETHRESH.override = 1
        phy.profile_outputs.MODEM_COH0_COHDYNAMICSYNCTHRESH.override = 0
        phy.profile_outputs.MODEM_COH1_SYNCTHRESH0.override = 31
        phy.profile_outputs.MODEM_COH1_SYNCTHRESH1.override = 21
        phy.profile_outputs.MODEM_COH1_SYNCTHRESH2.override = 21
        phy.profile_outputs.MODEM_COH1_SYNCTHRESH3.override = 46
        phy.profile_outputs.MODEM_COH2_SYNCTHRESHDELTA2.override = 5
        phy.profile_outputs.MODEM_COH3_CDSS.override = 4
        phy.profile_outputs.MODEM_COH3_COHDSAADDWNDSIZE.override = 864 #-160
        phy.profile_outputs.MODEM_COH3_COHDSAEN.override = 1
        phy.profile_outputs.MODEM_COH3_DYNIIRCOEFOPTION.override = 3
        phy.profile_outputs.MODEM_CTRL1_COMPMODE.override = 0
        phy.profile_outputs.MODEM_CTRL1_PHASEDEMOD.override = 2
        phy.profile_outputs.MODEM_CTRL2_DATAFILTER.override = 7
        phy.profile_outputs.MODEM_CTRL5_LINCORR.override = 0
        phy.profile_outputs.MODEM_CTRL5_BBSS.override = 5
        phy.profile_outputs.MODEM_CTRL5_DSSSCTD.override = 1
        phy.profile_outputs.MODEM_CTRL5_FOEPREAVG.override = 7
        phy.profile_outputs.MODEM_CTRL5_POEPER.override = 1
        phy.profile_outputs.MODEM_CTRL6_ARW.override = 1
        phy.profile_outputs.MODEM_CTRL6_PSTIMABORT1.override = 1
        phy.profile_outputs.MODEM_CTRL6_RXBRCALCDIS.override = 1
        phy.profile_outputs.MODEM_CTRL6_TDREW.override = 32
        phy.profile_outputs.MODEM_CTRL6_TIMTHRESHGAIN.override = 1
        phy.profile_outputs.MODEM_CTRL6_PSTIMABORT0.override = 0
        phy.profile_outputs.MODEM_CTRL6_PSTIMABORT2.override = 0
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG0.override = 1
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG1.override = 2
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG2.override = 4
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG3.override = 4
        phy.profile_outputs.MODEM_LONGRANGE1_AVGWIN.override = 4
        phy.profile_outputs.MODEM_LONGRANGE1_CHPWRACCUDEL.override = 1
        phy.profile_outputs.MODEM_LONGRANGE1_HYSVAL.override = 3
        phy.profile_outputs.MODEM_LONGRANGE1_LRTIMEOUTTHD.override = 160
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH1.override = 19
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH2.override = 25
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH3.override = 31
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH4.override = 37
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH5.override = 43
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH6.override = 49
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH7.override = 55
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH8.override = 61
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH1.override = 3
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH2.override = 4
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH3.override = 5
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH4.override = 6
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH10.override = 73
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH9.override = 67
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH10.override = 12
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH11.override = 13
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH5.override = 7
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH6.override = 8
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH7.override = 9
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH8.override = 10
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH9.override = 11
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRSPIKETH.override = 127
        phy.profile_outputs.MODEM_LONGRANGE6_LRSPIKETHD.override = 50
        phy.profile_outputs.MODEM_MODINDEX_FREQGAINE.override = 7
        phy.profile_outputs.MODEM_MODINDEX_FREQGAINM.override = 1
        phy.profile_outputs.MODEM_PRE_PREERRORS.override = 15
        #phy.profile_outputs.MODEM_REALTIMCFE_MINCOSTTHD.override = 800
        #phy.profile_outputs.MODEM_REALTIMCFE_RTCFEEN.override = 1
        #phy.profile_outputs.MODEM_REALTIMCFE_RTSCHWIN.override = 10
        #phy.profile_outputs.MODEM_REALTIMCFE_VTAFCFRAME.override = 1
        phy.profile_outputs.MODEM_TIMING_ADDTIMSEQ.override = 6
        phy.profile_outputs.MODEM_TIMING_OFFSUBDEN.override = 5
        phy.profile_outputs.MODEM_TIMING_OFFSUBNUM.override = 7
        phy.profile_outputs.MODEM_TIMING_TIMINGBASES.override = 3
        phy.profile_outputs.MODEM_TIMING_TIMTHRESH.override = 25
        #phy.profile_outputs.MODEM_TRECPMDET_PMACQUINGWIN.override = 1
        #phy.profile_outputs.MODEM_TRECPMDET_PMMINCOSTTHD.override = 120
        #phy.profile_outputs.MODEM_TRECPMDET_PMTIMEOUTSEL.override = 2
        phy.profile_outputs.MODEM_CTRL1_RESYNCPER.override = 1
        phy.profile_outputs.MODEM_LONGRANGE1_PREFILTLEN.override = 1
        phy.profile_outputs.MODEM_PREFILTCOEFF_PREFILTCOEFF.override = 2736235287
        phy.profile_outputs.MODEM_COH3_COHDSACMPLX.override = 0
        phy.profile_outputs.MODEM_SYNCPROPERTIES_STATICSYNCTHRESHEN.override = 1
        phy.profile_outputs.MODEM_SYNCPROPERTIES_STATICSYNCTHRESH.override = 50
        phy.profile_outputs.MODEM_COH0_COHCHPWRLOCK.override = 0
        phy.profile_outputs.MODEM_COH0_COHCHPWRRESTART.override = 1


        return phy

    def PHY_IEEE802154_868MHz_OQPSK(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='IEEE 802.15.4 868MHz OQPSK', phy_name=phy_name)

        self.IEEE802154_915MHz_OQPSK(phy, model)
        phy.profile_inputs.bitrate.value = 100000
        phy.profile_inputs.timing_detection_threshold.value = 35
        phy.profile_inputs.number_of_timing_windows.value = 3
        phy.profile_inputs.deviation.value = 100000
        phy.profile_inputs.dsss_spreading_factor.value = 4
        phy.profile_inputs.dsss_len.value = 16
        phy.profile_inputs.bandwidth_hz.value = 504960
        phy.profile_inputs.demod_select.value = model.vars.demod_select.var_enum.LEGACY

        return phy

    # Owner: Young-Joon Choi
    # Jira Link: https://jira.silabs.com/browse/PGOCELOTVALTEST-161
    def PHY_IEEE802154_868MHz_OQPSK_coh(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='IEEE 802.15.4 868MHz OQPSK Coherent Demod',
                            phy_name=phy_name)

        self.IEEE802154_915MHz_OQPSK(phy, model)
        phy.profile_inputs.demod_select.value = model.vars.demod_select.var_enum.COHERENT
        phy.profile_inputs.base_frequency_hz.value = 868300000
        phy.profile_inputs.bitrate.value = 100000
        phy.profile_inputs.deviation.value = 100000
        phy.profile_inputs.bandwidth_hz.value = 504960

        """ Complex Correlation """
        phy.profile_outputs.MODEM_CTRL6_CPLXCORREN.override = 1
        phy.profile_outputs.MODEM_COH3_COHDSACMPLX.override = 1

        """ Channel Power Accumulator Setting """
        # : Average and delay
        phy.profile_outputs.MODEM_LONGRANGE1_AVGWIN.override = 1
        phy.profile_outputs.MODEM_LONGRANGE1_CHPWRACCUDEL.override = 0

        """ BBSS """
        # : BBSS Channel Power Thresholds
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH1.override = 12
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH2.override = 18
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH3.override = 28
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH4.override = 35
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH5.override = 42
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH6.override = 48
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH7.override = 54
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH8.override = 60
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH9.override = 65
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH10.override = 70
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRTH11.override = 78

        # : BBSS lookup table
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH1.override = 2
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH2.override = 3
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH3.override = 4
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH4.override = 4
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH5.override = 5
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH6.override = 6
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH7.override = 7
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH8.override = 8
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH9.override = 9
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH10.override = 10
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH11.override = 11
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRSH12.override = 12

        """ Datafilter """
        # : Per's recommended DATAFILTER value is 7; however, with cplxcorren =1, DATAFILTER = 4 gives improved sens.
        phy.profile_outputs.MODEM_CTRL2_DATAFILTER.override = 4

        """ DSA Prefilter """
        phy.profile_outputs.MODEM_PREFILTCOEFF_PREFILTCOEFF.override = 2736235287

        """ Coherent DSA Settings """
        # : Enable coherent demodulator DSA
        phy.profile_outputs.MODEM_COH3_COHDSAEN.override = 1
        phy.profile_outputs.MODEM_COH3_CDSS.override = 4

        # : This threshold determines whether to use fixed or dynamic threshold based on channel power.
        # : > 128, FIXED DSA THRESHOLD ALWAYS
        # : = 0  , DYNAMIC DSA THRESHOLD ALWAYS
        # : 0 < x < 128, hybrid - for low channel power, use fixed threshold. For high channel power, use dynamic.
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRSPIKETH.override = 35

        # : For FIXED DSA mode, this is the correlation threshold
        phy.profile_outputs.MODEM_LONGRANGE6_LRSPIKETHD.override = 120  # : below 130, floor issues

        # : For dynamic DSA threshold, this is the baseline threshold. Threshold will increase in addition to this
        # : baseline value dependent on the channel power.
        phy.profile_outputs.MODEM_COH2_FIXEDCDTHFORIIR.override = 120  # Above 120, blocking degradation
        # : Coefficients for IIR, 0 -> 2^-3, 1 -> 2^-4, 2 -> 2^-5 , 3 -> 2^-6, Higher the value slower averaging.
        phy.profile_outputs.MODEM_COH3_DYNIIRCOEFOPTION.override = 3

        phy.profile_outputs.MODEM_COH3_PEAKCHKTIMOUT.override = 18

        """ Timing Detection Settings """
        # : Offset between DSA and timing windows
        # : Calculate as OSR*no_of_chips_per_sym*m
        phy.profile_outputs.MODEM_COH3_COHDSAADDWNDSIZE.override = 864  #-160 ( 864 = 2^10 - 160 )

        """ AFC """
        # : Controls number of bauds to rewind after fixed window timing detection.
        # : baudrate offset range is dependent on this value. Setting to 32 narrows baudrate offset range
        phy.profile_outputs.MODEM_CTRL6_TDREW.override = 64

        """ Dynamic Preamble / Sync """
        phy.profile_outputs.MODEM_CTRL5_RESYNCBAUDTRANS.override = 0

        phy.profile_outputs.MODEM_COH0_COHCHPWRTH0.override = 35  # same as LRCHPWRTH3
        phy.profile_outputs.MODEM_COH0_COHCHPWRTH1.override = 42  # same as LRCHPWRTH4
        phy.profile_outputs.MODEM_COH0_COHCHPWRTH2.override = 127

        phy.profile_outputs.MODEM_COH1_SYNCTHRESH0.override = 29  # 33 < COHCHPWRTH0
        phy.profile_outputs.MODEM_COH1_SYNCTHRESH1.override = 32  # 36 COHCHPWRTH0 < x < COHCHPWRTH1
        phy.profile_outputs.MODEM_COH1_SYNCTHRESH2.override = 35  # 39 COHCHPWRTH1 < x < COHCHPWRTH2
        phy.profile_outputs.MODEM_COH1_SYNCTHRESH3.override = 127  # > COHCHPWRTH2

        phy.profile_outputs.MODEM_COH2_SYNCTHRESHDELTA0.override = 0  # < COHCHPWRTH0
        phy.profile_outputs.MODEM_COH2_SYNCTHRESHDELTA1.override = 2  # 2  # COHCHPWRTH0 < x < COHCHPWRTH1
        phy.profile_outputs.MODEM_COH2_SYNCTHRESHDELTA2.override = 4  # 4  # COHCHPWRTH1 < x < COHCHPWRTH2
        phy.profile_outputs.MODEM_COH2_SYNCTHRESHDELTA3.override = 0  # > COHCHPWRTH2
        return phy

    def PHY_IEEE802154_868MHz_BPSK(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Legacy IEEE 802.15.4 868MHz BPSK from Dumbo', phy_name=phy_name)

        self.IEEE802154_Base(phy, model)

        phy.profile_inputs.agc_hysteresis.value = 0
        phy.profile_inputs.agc_period.value = 3
        phy.profile_inputs.agc_power_target.value = -17
        phy.profile_inputs.base_frequency_hz.value =  868_000_000
        phy.profile_inputs.baudrate_tol_ppm.value = 1875
        phy.profile_inputs.bitrate.value = 20000
        phy.profile_inputs.channel_spacing_hz.value = 0
        phy.profile_inputs.deviation.value = 150000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.RE0
        phy.profile_inputs.dsss_chipping_code.value =  long(0x9AF)
        phy.profile_inputs.dsss_len.value = 15
        phy.profile_inputs.dsss_spreading_factor.value = 15
        phy.profile_inputs.frequency_comp_mode.value = model.vars.frequency_comp_mode.var_enum.INTERNAL_ALWAYS_ON
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.BPSK
        phy.profile_inputs.number_of_timing_windows.value = 11
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_2000KHz
        phy.profile_inputs.preamble_length.value = 128
        phy.profile_inputs.rssi_period.value = 7
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Raised_Cosine
        phy.profile_inputs.shaping_filter_param.value = 1.0
        phy.profile_inputs.timing_detection_threshold.value = 30
        phy.profile_inputs.timing_resync_period.value = 4

        phy.profile_outputs.AGC_CTRL7_SUBDEN.override = 7
        phy.profile_outputs.AGC_CTRL7_SUBINT.override = 4
        phy.profile_outputs.AGC_CTRL7_SUBNUM.override = 2
        phy.profile_outputs.AGC_CTRL7_SUBPERIOD.override = 1
        phy.profile_outputs.AGC_GAINSTEPLIM0_CFLOOPSTEPMAX.override = 4
        # Not present in Series 2
        # phy.profile_outputs.AGC_LOOPDEL_LNASLICESDEL.override = 4
        phy.profile_outputs.MODEM_CTRL2_DATAFILTER.override = 0
        phy.profile_outputs.MODEM_TIMING_FASTRESYNC.override = 1
        phy.profile_outputs.MODEM_CTRL4_PREDISTDEB.override = 1
        phy.profile_outputs.MODEM_CTRL4_PREDISTGAIN.override = 3

        phy.profile_outputs.SEQ_MISC_DIG_RAMP_EN.override = 1

        return phy

    def PHY_IEEE802154_915MHz_BPSK_40kbps(self, model, phy_name=None):
        phy = self.PHY_IEEE802154_868MHz_BPSK(model, 'PHY_IEEE802154_915MHz_BPSK_40kbps')

        phy.profile_inputs.base_frequency_hz.value = 915_000_000
        phy.profile_inputs.bitrate.value = 40000

        return phy

    def PHY_IEEE802154_868MHz_BPSK_20kbps_coh(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Coherent IEEE 802.15.4 868MHz BPSK PHY', phy_name=phy_name)

        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.preamble_pattern.value = 0
        phy.profile_inputs.preamble_pattern_len.value = 4
        phy.profile_inputs.rx_xtal_error_ppm.value = 0
        phy.profile_inputs.symbol_encoding.value = model.vars.symbol_encoding.var_enum.DSSS
        phy.profile_inputs.syncword_0.value = long(0xe5)
        phy.profile_inputs.syncword_1.value = long(0x0)
        phy.profile_inputs.syncword_length.value = 8
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.xtal_frequency_hz.value = 39000000
        phy.profile_inputs.channel_spacing_hz.value = 2_000_000

        # Add 15.4 Packet Configuration
        PHY_COMMON_FRAME_154(phy, model)

        self._part_specific_phy_overrides(phy, model)

        # Mandatory to use model.vars.demod_select to force the coherent demodulator, otherwise LEGACY
        # Cannot use profile_inputs because hidden inputs
        model.vars.demod_select.value_forced = model.vars.demod_select.var_enum.COHERENT

        phy.profile_inputs.base_frequency_hz.value = long(868000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 0
        phy.profile_inputs.bitrate.value = 20000
        phy.profile_inputs.channel_spacing_hz.value = 0
        phy.profile_inputs.deviation.value = 150000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.RE0
        phy.profile_inputs.dsss_chipping_code.value = long(0x9AF)
        phy.profile_inputs.dsss_len.value = 15
        phy.profile_inputs.dsss_spreading_factor.value = 15
        phy.profile_inputs.if_frequency_hz.value = 600000
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.BPSK
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_2000KHz
        phy.profile_inputs.preamble_length.value = 32
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Raised_Cosine
        phy.profile_inputs.shaping_filter_param.value = 1.0
        phy.profile_inputs.rx_xtal_error_ppm.value = 20
        phy.profile_inputs.tx_xtal_error_ppm.value = 20
        phy.profile_inputs.preamble_pattern_len.value = 1

        # Mandatory to enable static threshold to avoid false detection after the sync word
        model.vars.MODEM_SYNCPROPERTIES_STATICSYNCTHRESHEN.value_forced = 1  # Add static sync threadhold
        model.vars.MODEM_SYNCPROPERTIES_STATICSYNCTHRESH.value_forced = 40  # Static sync threshold = STATICSYNCTHRESH *2^TIMTHRESHGAIN

        """ Channel Power Accumulator Setting """
        # : Average and delay
        # model.vars.MODEM_LONGRANGE1_PREFILTEN.value_forced = 1 # DSA prefilter length
        # model.vars.MODEM_LONGRANGE1_LRSPIKETHADD.value_forced = 0  # DSA
        model.vars.MODEM_LONGRANGE1_AVGWIN.value_forced = 4  # Average window for channel power estimation
        model.vars.MODEM_LONGRANGE1_CHPWRACCUDEL.value_forced = 0  # Use accumulated channel power value, 0 -> DEL0 (no delay) floor(timing_window_actual*2/(2^(AVGWIN+2+PWRPERIOD)))
        model.vars.MODEM_LONGRANGE1_HYSVAL.value_forced = 3  # Hysteresis Value for BBSS

        """ BBSS """
        # : BBSS Channel Power Thresholds
        starting_LRCHPWRTH = 16  # BEST VALUE
        shifting_LRCHPWRTH = 8
        # Threshold for LRCHPWRSH
        model.vars.MODEM_LONGRANGE2_LRCHPWRTH1.value_forced = starting_LRCHPWRTH + (shifting_LRCHPWRTH * 2)  # 32 -> -105 dBm
        model.vars.MODEM_LONGRANGE2_LRCHPWRTH2.value_forced = starting_LRCHPWRTH + (shifting_LRCHPWRTH * 3)  # 40 -> -97 dBm
        model.vars.MODEM_LONGRANGE2_LRCHPWRTH3.value_forced = starting_LRCHPWRTH + (shifting_LRCHPWRTH * 4)  # 48 -> -89 dBm
        model.vars.MODEM_LONGRANGE2_LRCHPWRTH4.value_forced = starting_LRCHPWRTH + (shifting_LRCHPWRTH * 5)  # 56 -> -81 dBm
        model.vars.MODEM_LONGRANGE3_LRCHPWRTH5.value_forced = starting_LRCHPWRTH + (shifting_LRCHPWRTH * 6)  # 64 -> -73 dBm
        model.vars.MODEM_LONGRANGE3_LRCHPWRTH6.value_forced = starting_LRCHPWRTH + (shifting_LRCHPWRTH * 7)  # 72 -> -65 dBm
        model.vars.MODEM_LONGRANGE3_LRCHPWRTH7.value_forced = starting_LRCHPWRTH + (shifting_LRCHPWRTH * 8)  # 80 -> -57 dBm
        model.vars.MODEM_LONGRANGE3_LRCHPWRTH8.value_forced = starting_LRCHPWRTH + (shifting_LRCHPWRTH * 9)  # 88 -> -49 dBm
        model.vars.MODEM_LONGRANGE4_LRCHPWRTH9.value_forced = starting_LRCHPWRTH + (shifting_LRCHPWRTH * 10) + 2 # 96 -> -41 dBm
        model.vars.MODEM_LONGRANGE4_LRCHPWRTH10.value_forced = starting_LRCHPWRTH + (shifting_LRCHPWRTH * 11) + 4 # 104 -> -33 dBm
        model.vars.MODEM_LONGRANGE6_LRCHPWRTH11.value_forced = starting_LRCHPWRTH + (shifting_LRCHPWRTH * 12)  # 112-> -27 dBm

        # : BBSS lookup table
        starting_LRCHPWRSH = 3  # BEST VALUE, add one to increase sensibility
        model.vars.MODEM_LONGRANGE4_LRCHPWRSH1.value_forced = starting_LRCHPWRSH + 2  # Take this value if LRCHPWRTH1 > CHPWR
        model.vars.MODEM_LONGRANGE4_LRCHPWRSH2.value_forced = starting_LRCHPWRSH + 3  # Take this value if LRCHPWRTH2 > CHPWR > LRCHPWRTH1
        model.vars.MODEM_LONGRANGE4_LRCHPWRSH3.value_forced = starting_LRCHPWRSH + 5  # Take this value if LRCHPWRTH3 > CHPWR > LRCHPWRTH2
        model.vars.MODEM_LONGRANGE4_LRCHPWRSH4.value_forced = starting_LRCHPWRSH + 6  # Take this value if LRCHPWRTH4 > CHPWR > LRCHPWRTH3
        model.vars.MODEM_LONGRANGE5_LRCHPWRSH5.value_forced = starting_LRCHPWRSH + 8  # Take this value if LRCHPWRTH5 > CHPWR > LRCHPWRTH4
        model.vars.MODEM_LONGRANGE5_LRCHPWRSH6.value_forced = starting_LRCHPWRSH + 9  # Take this value if LRCHPWRTH6 > CHPWR > LRCHPWRTH5
        model.vars.MODEM_LONGRANGE5_LRCHPWRSH7.value_forced = starting_LRCHPWRSH + 10  # Take this value if LRCHPWRTH7 > CHPWR > LRCHPWRTH6
        model.vars.MODEM_LONGRANGE5_LRCHPWRSH8.value_forced = starting_LRCHPWRSH + 11  # Take this value if LRCHPWRTH8 > CHPWR > LRCHPWRTH7
        model.vars.MODEM_LONGRANGE5_LRCHPWRSH9.value_forced = starting_LRCHPWRSH + 12  # Take this value if LRCHPWRTH9 > CHPWR > LRCHPWRTH8
        model.vars.MODEM_LONGRANGE5_LRCHPWRSH10.value_forced = starting_LRCHPWRSH + 12  # Take this value if LRCHPWRTH10 > CHPWR > LRCHPWRTH9
        model.vars.MODEM_LONGRANGE5_LRCHPWRSH11.value_forced = starting_LRCHPWRSH + 12  # Take this value if LRCHPWRTH11 > CHPWR > LRCHPWRTH10
        model.vars.MODEM_LONGRANGE6_LRCHPWRSH12.value_forced = starting_LRCHPWRSH + 12  # : removes floor issue at high power max 15

        # : This threshold determines whether to use fixed or dynamic threshold based on channel power.
        model.vars.MODEM_LONGRANGE6_LRCHPWRSPIKETH.value_forced = 70  # DSA setting
        # : For FIXED DSA mode, this is the correlation threshold
        model.vars.MODEM_LONGRANGE6_LRSPIKETHD.value_forced = 40  # Below 130, floor issues DSA setting

        # base_value = -138dBm
        model.vars.MODEM_COH0_COHCHPWRTH0.value_forced = 25  # Channel power boundary between SYNCTHRESH 0 and 1
        model.vars.MODEM_COH0_COHCHPWRTH1.value_forced = 64  # Channel power boundary between SYNCTHRESH 1 and 2
        model.vars.MODEM_COH0_COHCHPWRTH2.value_forced = 127  # Channel power boundary between SYNCTHRESH 2 and 3

        model.vars.MODEM_COH0_COHDYNAMICBBSSEN.value_forced = 1  # SHOULD BE ENABLED, Set to enable the dynamic BBSS based on average channel power for coherent demodulator.
        model.vars.MODEM_COH0_COHDYNAMICPRETHRESH.value_forced = 1  # SHOULD BE ENABLED, Set to enable the dynamic preamble threshold based on average channel power for coherent demodulator and BBSS
        model.vars.MODEM_COH0_COHDYNAMICPRETHRESHSEL.value_forced = 0  # SHOULD BE DISABLED, Select the dynamic preamble threshold 0 -> 1x sync coeff
        model.vars.MODEM_COH0_COHDYNAMICSYNCTHRESH.value_forced = 0  # SHOULD BE DISABLED because COHDYNAMICPRETHRES enable
        model.vars.MODEM_COH0_COHCHPWRLOCK.value_forced = 0  # Set to TIMDET (0) or DSADET (1) when timing is detected
        model.vars.MODEM_COH0_COHCHPWRRESTART.value_forced = 0 # Set to enable automatic restart of Channel Power whenever a frame is received

        model.vars.MODEM_COH1_SYNCTHRESH0.value_forced = 17  # Minimum threshold syncword when CHPWR < COHCHPWRTH0
        model.vars.MODEM_COH1_SYNCTHRESH1.value_forced = 18  # Minimum threshold syncword when COHCHPWRTH0 < CHPWR < COHCHPWRTH1
        model.vars.MODEM_COH1_SYNCTHRESH2.value_forced = 24  # Minimum threshold syncword when COHCHPWRTH1 < CHPWR < COHCHPWRTH2
        model.vars.MODEM_COH1_SYNCTHRESH3.value_forced = 127  # Minimum threshold syncword when CHPWR > COHCHPWRTH2

        model.vars.MODEM_COH2_SYNCTHRESHDELTA0.value_forced = 1  # < COHCHPWRTH0
        model.vars.MODEM_COH2_SYNCTHRESHDELTA1.value_forced = 1  # COHCHPWRTH0 < x < COHCHPWRTH1
        model.vars.MODEM_COH2_SYNCTHRESHDELTA2.value_forced = 1  # COHCHPWRTH1 < x < COHCHPWRTH2
        model.vars.MODEM_COH2_SYNCTHRESHDELTA3.value_forced = 1  # > COHCHPWRTH2
        # : For dynamic DSA threshold, this is the baseline threshold. Threshold will increase in addition to this
        # : baseline value dependent on the channel power.
        model.vars.MODEM_COH2_FIXEDCDTHFORIIR.value_forced = 70  # Above 120, blocking degradation Coherent DSA Settings

        model.vars.MODEM_COH3_COHDSAEN.value_forced = 0  # Coherent DSA Settings
        model.vars.MODEM_COH3_PEAKCHKTIMOUT.value_forced = 18  # Coherent DSA Settings
        model.vars.MODEM_COH3_COHDSAADDWNDSIZE.value_forced = 80  # Coherent DSA Settings  OSR * no_of_chips_per_sym*m
        model.vars.MODEM_COH3_CDSS.value_forced = 4  # Coherent DSA Settings
        model.vars.MODEM_COH3_COHDSACMPLX.value_forced = 0  # Coherent DSA Settings SHOULD BE DISABLED BECAUSE Complex correlation PAGE 2009
        model.vars.MODEM_COH3_DYNIIRCOEFOPTION.value_forced = 3  # Coherent DSA Settings

        model.vars.MODEM_CTRL0_DUALCORROPTDIS.value_forced = 1  # # Disables default optimization for Fixed Window Timing Search when using dual correlation passes

        model.vars.MODEM_CTRL1_PHASEDEMOD.value_forced = 2  # : 2 - COH detection
        model.vars.MODEM_CTRL1_FREQOFFESTLIM.value_forced = 0  # Limit for frequency offset compensation
        model.vars.MODEM_CTRL1_FREQOFFESTPER.value_forced = 0  # Frequency offset estimation/compensation update period is 2^FREQOFFESTPER windows
        model.vars.MODEM_CTRL1_COMPMODE.value_forced = 1  # Enable compensation
        # Defines the timing resynchronization period. The timing update interval is RESYNCPER times the length of the timing sequence defined by TIMINGBASES
        model.vars.MODEM_CTRL1_RESYNCPER.value_forced = 2  # Defines the timing resynchronization period

        model.vars.MODEM_CTRL2_DATAFILTER.value_forced = 4  # Coherent detection is enabled

        model.vars.MODEM_CTRL3_TSAMPDEL.value_forced = 0  # Delay from detection of strong signals to enabling of timing search is 2^TSAMPDEL+1 samples
        model.vars.MODEM_CTRL3_TIMINGBASESGAIN.value_forced = 0  # Increase timing window to be TIMINGBASES * 2^TIMINGBASESGAIN

        model.vars.MODEM_CTRL4_OFFSETPHASEMASKING.value_forced = 1  # SHOULD BE ENABLED CHECK PAGE 2085, Enables masking of differentiated phase used to measure frequency offset during Timing Search and for AFC
        model.vars.MODEM_CTRL4_ADCSATLEVEL.value_forced = 6  # Define ADC Saturation Level to be used before indicating saturation to AGC
        model.vars.MODEM_CTRL4_ADCSATDENS.value_forced = 0  # The counter values increase the ADCSATDENS+1
        model.vars.MODEM_CTRL4_PHASECLICKFILT.value_forced = 1  # Phase click thresholds for phase click filter. Filter is disabled for PHASECLICKFILT=0.

        model.vars.MODEM_CTRL5_DSSSCTD.value_forced = 1  # After preamble detection, only detected symbol is used to qualify a valid preamble 4/bits-per-symbol
        model.vars.MODEM_CTRL5_POEPER.value_forced = 4  # Controls the POE period in number of DSSS symbols
        # Calibration baud rate during the preamble (earn many of tolerance)
        model.vars.MODEM_CTRL5_BRCALEN.value_forced = 0  # Loop BR enable
        model.vars.MODEM_CTRL5_BRCALMODE.value_forced = 0  # slopes et zeros (robustness)
        model.vars.MODEM_CTRL5_BRCALAVG.value_forced = 0  # Moderate average
        model.vars.MODEM_CTRL5_TDEDGE.value_forced = 1  # Increase timing robustness
        model.vars.MODEM_CTRL5_TREDGE.value_forced = 1  # Increase timing robustness
        model.vars.MODEM_CTRL5_RESYNCBAUDTRANS.value_forced = 0  # Allows resync timing during payload
        model.vars.MODEM_CTRL5_RESYNCLIMIT.value_forced = 1  # Limit unwanted resyncs at low SNR
        model.vars.MODEM_CTRL5_LINCORR.value_forced = 1  # Avoid timing detections where only part of the window is occupied by a valid signal
        model.vars.MODEM_CTRL5_BBSS.value_forced = 4  # Low BBSS values reduces quantization noise, but results in more limitation

        model.vars.MODEM_CTRL6_ARW.value_forced = 1  # If the difference between the end of next window and the current write address is less than half the RAM size
        model.vars.MODEM_CTRL6_TDREW.value_forced = 60  # Controls number of bauds to rewind after Fixed Window Timing Detection = timingbases*dsss_len*2/3
        model.vars.MODEM_CTRL6_CPLXCORREN.value_forced = 0  # Set if freq_limit > baudrate/8
        # Timing threshold = TIMTHRESH * 2^TIMTHRESHGAIN && sync threshold = STATICSYNCTHRESH * 2^TIMTHRESHGAIN
        model.vars.MODEM_CTRL6_PSTIMABORT0.value_forced = 1  # Timing is aborted during preamble search if maximum correlation is much higher than preamble correlation used for timing detection
        model.vars.MODEM_CTRL6_PSTIMABORT1.value_forced = 1  # Timing is aborted during preamble search if maximum correlation is not equal to current preamble correlation
        model.vars.MODEM_CTRL6_PSTIMABORT2.value_forced = 1  # Timing is aborted during preamble search if current preamble correlation is much higher than preamble correlation used for timing detection
        # Disable RX baudrate calculation used by AGC. Instead, assume OSR = 2 * RXBRFRAC
        model.vars.MODEM_CTRL6_RXBRCALCDIS.value_forced = 1  # DO NOT TOUCH
        model.vars.MODEM_CTRL6_PREBASES.value_forced = 8  # The window size can be set differently during Preamble Search than during Timing Search

        # Remove the slope between -40 to -26 dBm
        model.vars.AGC_CTRL1_PWRPERIOD.value_forced = 4  # This value controls the AGC power measure period. The period is 2^AGCPERIOD subperiods
        model.vars.AGC_CTRL0_PWRTARGET.value_forced = 188  # This value controls the AGC power measure period. The period is 2^AGCPERIOD subperiods
        model.vars.AGC_CTRL1_RSSIPERIOD.value_forced = 3  # The period is defined as 2^RSSIPERIOD subperiods
        model.vars.AGC_AGCPERIOD1_PERIODLOW.value_forced = 165  # 3 times AGC_AGCPERIOD0_PERIODHI
        model.vars.AGC_AGCPERIOD0_PERIODHI.value_forced = 55  #

        model.vars.AGC_RSSISTEPTHR_POSSTEPTHR.value_forced = 6  # When RSSIINT increases with more than POSSTEPTHR dB between two update periods
        model.vars.AGC_RSSISTEPTHR_DEMODRESTARTPER.value_forced = 6  # When this value is set differently from 0, a separate RSSI measurement is made based on 2^DEMODRESTARTPER subperiods

        model.vars.AGC_GAINSTEPLIM0_CFLOOPSTEPMAX.value_forced = 8  # Set max gain step for gain change using channel filter slow loop
        model.vars.AGC_GAINSTEPLIM0_CFLOOPDEL.value_forced = 45  # Sets the delay used in the channel filter loop

        model.vars.MODEM_PRE_PREERRORS.value_forced = 15  # Defines the maximum number of errors allowed within a timing sequence

        model.vars.MODEM_MODINDEX_MODINDEXE.value_forced = 29  # Modulation output is scaled by MODINDEXM * 2^MODINDEXE to ensure proper modulation characteristics
        model.vars.MODEM_MODINDEX_MODINDEXM.value_forced = 19  # Modulation output is scaled by MODINDEXM * 2^MODINDEXE to ensure proper modulation characteristics

        model.vars.MODEM_AFC_AFCGEAR.value_forced = 3  # The slow AFC gain applies after gear switching occurs

        model.vars.MODEM_CTRL5_FOEPREAVG.value_forced = 7  # If FOEPREAVG = 7, the averaging is set dynamically as given by MODEM_INTAFC
        model.vars.MODEM_INTAFC_FOEPREAVG0.value_forced = 1  # Frequency Offset Estimate Pre-Averaging for first estimate
        model.vars.MODEM_INTAFC_FOEPREAVG1.value_forced = 2  # Frequency Offset Estimate Pre-Averaging for second estimate
        model.vars.MODEM_INTAFC_FOEPREAVG2.value_forced = 4  # Frequency Offset Estimate Pre-Averaging for third estimate
        model.vars.MODEM_INTAFC_FOEPREAVG3.value_forced = 4  # Frequency Offset Estimate Pre-Averaging for fourth estimate

        # This is the maximum limit for AFC adjustment in RX and TX. The limit in Hz is AFCADJLIM * Synthesizer resolution.
        # If the register is set to 0, the limit is disabled.
        model.vars.MODEM_AFCADJLIM_AFCADJLIM.value_forced = 0  # Set to freq_limit*baudrate/2^13 with freq_limit=freq_offset_hz

        # : Controls additional offset averaging state for timing search and AFC.
        # : Additional windows averages over OFFSUBNUM/OFFSUBDEN samples to avoid DC balance issue
        # : MUST BE SET MANUALLY! NO CALCULATOR SUPPORT AVAILABLE FOR COH PHY
        # AFC update period = 2^AFCAVGPER × (OFFSUBNUM/OFFSUBDEN)
        model.vars.MODEM_TIMING_OFFSUBNUM.value_forced = 8
        model.vars.MODEM_TIMING_OFFSUBDEN.value_forced = 8

        # this is the number of times the first aligned window is processed during Preamble Search. Number of FOC updates is ADDTIMSEQ/2
        model.vars.MODEM_TIMING_ADDTIMSEQ.value_forced = 8  # Number of additional timing sequences to detect a valid preamble. Number of FOC updates is ADDTIMSEQ/2
        model.vars.MODEM_TIMING_FASTRESYNC.value_forced = 1  # Allow fast timing resynchronization (RESYNCPER = 1) in first part of frame
        model.vars.MODEM_TIMING_TIMINGBASES.value_forced = 9  # Defines the timing sequence used for Timing Search
        model.vars.MODEM_TIMING_TIMTHRESH.value_forced = 105  # Timing threshold = TIMTHRESH * 2^TIMTHRESHGAIN
        model.vars.MODEM_CTRL6_TIMTHRESHGAIN.value_forced = 0  # Timing threshold = TIMTHRESH * 2^TIMTHRESHGAIN

        # Tx override with digital ramping to be compliant on ACPR
        model.vars.MODEM_CTRL4_PREDISTDEB.value_forced = 1
        model.vars.MODEM_CTRL4_PREDISTGAIN.value_forced = 3
        model.vars.SEQ_MISC_DIG_RAMP_EN.value_forced = 1

        return phy

    # Jira Link: https://jira.silabs.com/browse/MCUW_RADIO_CFG-2755
    def PHY_IEEE802154_915MHz_BPSK_40kbps_coh(self, model, phy_name=None):
        phy = self.PHY_IEEE802154_868MHz_BPSK_20kbps_coh(model, phy_name=phy_name)

        phy.profile_inputs.base_frequency_hz.value = 915_000_000
        phy.profile_inputs.bitrate.value = 40_000
        phy.profile_inputs.rx_xtal_error_ppm.value = 20
        phy.profile_inputs.tx_xtal_error_ppm.value = 20

        """ Fix for waterfall and frequency offset tolerance """
        model.vars.MODEM_TIMING_TIMINGBASES.value_forced = 5
        model.vars.AGC_CTRL1_RSSIPERIOD.value_forced = 2
        model.vars.AGC_CTRL1_PWRPERIOD.value_forced = 4
        model.vars.MODEM_TIMING_ADDTIMSEQ.value_forced = 4
        model.vars.AGC_GAINSTEPLIM0_CFLOOPDEL.value_forced = 51
        model.vars.MODEM_COH1_SYNCTHRESH0.value_forced = 16
        model.vars.MODEM_COH1_SYNCTHRESH1.value_forced = 18
        model.vars.MODEM_COH1_SYNCTHRESH2.value_forced = 24
        model.vars.MODEM_COH0_COHCHPWRTH0.value_forced = 26
        model.vars.MODEM_COH0_COHCHPWRTH1.value_forced = 80
        model.vars.MODEM_COH2_SYNCTHRESHDELTA1.value_forced = 2

        return phy

    def PHY_IEEE802154g_MRFSK_OM1_16bitpre_oneshot_dsafoest(self, model, phy_name=None):
        pass

    def PHY_IEEE802154g_MRFSK_OM1_16bitpre_oneshot_legacyfoest(self, model, phy_name=None):
        pass

    def PHY_IEEE802154g_MRFSK_OM1_32bitpre_oneshot_dsafoest(self, model, phy_name=None):
        pass

    def PHY_IEEE802154g_MRFSK_OM1_32bitpre_oneshot_legacyfoest(self, model, phy_name=None):
        pass

    def PHY_IEEE802154g_MRFSK_OM1_16bitpre(self, model, phy_name=None):
        pass

    def PHY_IEEE802154_RSGFSK_868MHz_500kbps_mi0p76(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='IEEE 802.15.4 868MHz RS-GFSK PHY', phy_name=phy_name)

        phy.profile_inputs.base_frequency_hz.value =  long(868000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 0
        phy.profile_inputs.bitrate.value = 500000
        phy.profile_inputs.channel_spacing_hz.value = 1000000
        phy.profile_inputs.deviation.value = 190000   # 0.76*500000/2
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.dsss_chipping_code.value = long(0)
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_2500KHz
        phy.profile_inputs.preamble_length.value = 4*8   # phyRsGfskPreambleLength multiples of '01010101'
        phy.profile_inputs.preamble_pattern.value = 1
        phy.profile_inputs.preamble_pattern_len.value = 2
        phy.profile_inputs.rx_xtal_error_ppm.value = 25
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5
        phy.profile_inputs.symbol_encoding.value = model.vars.symbol_encoding.var_enum.NRZ
        phy.profile_inputs.syncword_0.value =  long(0x904E)
        phy.profile_inputs.syncword_1.value =  long(0x9af0)
        phy.profile_inputs.syncword_length.value = 16
        phy.profile_inputs.tx_xtal_error_ppm.value = 25
        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        # FIXME:  temporary force bandwidth to (0.76+1)*500000*0.75 + 2*(25+25)*868
        #phy.profile_inputs.bandwidth_hz.value = 650000
        # FIXME:  temporary force KSI until calculated in py
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI1.override = 97
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI2.override = 65 #72 for forced bw=650000
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 49 #50 for forced bw=650000

        return phy

    def IEEE802154_2p4GHz_cohdsa_base(self, phy, model):
        PHY_COMMON_FRAME_154(phy, model)
        # Override min length for 802.15.4E Seq# Suppression
        phy.profile_inputs.var_length_minlength.value = 4
        Phy_Internal_Base.AGC_FAST_LOOP_base(phy, model)
        phy.profile_inputs.demod_select.value = model.vars.demod_select.var_enum.COHERENT
        phy.profile_inputs.bandwidth_hz.value = 2524800
        phy.profile_inputs.base_frequency_hz.value = long(2450000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 0
        phy.profile_inputs.bitrate.value = 250000
        phy.profile_inputs.channel_spacing_hz.value = 5000000
        phy.profile_inputs.deviation.value = 500000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.dsss_chipping_code.value = long(0x744AC39B)
        phy.profile_inputs.dsss_len.value = 32
        phy.profile_inputs.dsss_spreading_factor.value = 8
        phy.profile_inputs.frequency_comp_mode.value = model.vars.frequency_comp_mode.var_enum.DISABLED
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.if_frequency_hz.value = 1370000
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.OQPSK
        phy.profile_inputs.number_of_timing_windows.value = 7
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_1500KHz
        phy.profile_inputs.pll_bandwidth_rx.value = model.vars.pll_bandwidth_rx.var_enum.BW_250KHz
        phy.profile_inputs.preamble_length.value = 32
        phy.profile_inputs.preamble_pattern.value = 0
        phy.profile_inputs.preamble_pattern_len.value = 4
        phy.profile_inputs.rssi_period.value = 3
        phy.profile_inputs.rx_xtal_error_ppm.value = 0
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Custom_OQPSK
        phy.profile_inputs.shaping_filter_param.value = 0.0
        phy.profile_inputs.symbol_encoding.value = model.vars.symbol_encoding.var_enum.DSSS
        phy.profile_inputs.symbols_in_timing_window.value = 12
        phy.profile_inputs.syncword_0.value = long(0xe5)
        phy.profile_inputs.syncword_1.value = long(0x0)
        phy.profile_inputs.syncword_length.value = 8
        phy.profile_inputs.target_osr.value = 5
        phy.profile_inputs.timing_detection_threshold.value = 65
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        # phy.profile_inputs.xtal_frequency_hz.value = 38400000
        self._set_xtal_frequency(phy)
        phy.profile_inputs.target_osr.value = 5  # Calc SRC

        # Additional overrides introduced when Series 2 AGC calculations added. These prevent the PHY from changing versus what was used during Validation.

        phy.profile_outputs.AGC_LNABOOST_LNABWADJ.override = 0
        phy.profile_outputs.AGC_CTRL0_PWRTARGET.override = 20
        phy.profile_outputs.AGC_CTRL1_PWRPERIOD.override = 4
        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        phy.profile_outputs.MODEM_AFC_AFCRXCLR.override = 1
        #phy.profile_outputs.MODEM_AFC_AFCSCALEM.override = 3
        phy.profile_outputs.MODEM_AFCADJLIM_AFCADJLIM.override = 2750
        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 0x1E00  # 9,10,11,12
        phy.profile_outputs.MODEM_COH0_COHCHPWRTH0.override = 33
        phy.profile_outputs.MODEM_COH0_COHCHPWRTH1.override = 40
        phy.profile_outputs.MODEM_COH0_COHCHPWRTH2.override = 100
        phy.profile_outputs.MODEM_COH0_COHDYNAMICBBSSEN.override = 1
        phy.profile_outputs.MODEM_COH0_COHDYNAMICPRETHRESH.override = 1
        phy.profile_outputs.MODEM_COH0_COHDYNAMICSYNCTHRESH.override = 1
        phy.profile_outputs.MODEM_COH1_SYNCTHRESH0.override = 20
        phy.profile_outputs.MODEM_COH1_SYNCTHRESH1.override = 23
        phy.profile_outputs.MODEM_COH1_SYNCTHRESH2.override = 26
        phy.profile_outputs.MODEM_COH2_DSAPEAKCHPWRTH.override = 200
        phy.profile_outputs.MODEM_COH2_FIXEDCDTHFORIIR.override = 105
        phy.profile_outputs.MODEM_COH2_SYNCTHRESHDELTA1.override = 2
        phy.profile_outputs.MODEM_COH2_SYNCTHRESHDELTA2.override = 4
        phy.profile_outputs.MODEM_COH3_CDSS.override = 4
        phy.profile_outputs.MODEM_COH3_COHDSAADDWNDSIZE.override = 80
        phy.profile_outputs.MODEM_COH3_COHDSAEN.override = 1
        phy.profile_outputs.MODEM_COH3_DSAPEAKINDLEN.override = 4
        phy.profile_outputs.MODEM_COH3_DYNIIRCOEFOPTION.override = 3
        phy.profile_outputs.MODEM_COH3_LOGICBASEDCOHDEMODGATE.override = 1
        phy.profile_outputs.MODEM_COH3_PEAKCHKTIMOUT.override = 18
        phy.profile_outputs.MODEM_CTRL1_PHASEDEMOD.override = 2
        phy.profile_outputs.MODEM_CTRL2_DATAFILTER.override = 7
        phy.profile_outputs.MODEM_CTRL2_SQITHRESH.override = 200
        phy.profile_outputs.MODEM_CTRL5_BBSS.override = 6
        phy.profile_outputs.MODEM_CTRL5_DSSSCTD.override = 1
        phy.profile_outputs.MODEM_CTRL5_FOEPREAVG.override = 7
        phy.profile_outputs.MODEM_CTRL5_LINCORR.override = 1
        phy.profile_outputs.MODEM_CTRL5_POEPER.override = 1
        phy.profile_outputs.MODEM_CTRL6_ARW.override = 1
        phy.profile_outputs.MODEM_CTRL6_DSSS3SYMBOLSYNCEN.override = 1
        phy.profile_outputs.MODEM_CTRL6_PSTIMABORT0.override = 1
        phy.profile_outputs.MODEM_CTRL6_PSTIMABORT1.override = 1
        phy.profile_outputs.MODEM_CTRL6_PSTIMABORT2.override = 1
        phy.profile_outputs.MODEM_CTRL6_RXBRCALCDIS.override = 1
        phy.profile_outputs.MODEM_CTRL6_TDREW.override = 64
        phy.profile_outputs.MODEM_CTRL6_TIMTHRESHGAIN.override = 2
        phy.profile_outputs.MODEM_DIGIGAINCTRL_DIGIGAINEN.override = 1
        phy.profile_outputs.MODEM_DIGIGAINCTRL_DIGIGAINSEL.override = 20
        # phy.profile_outputs.MODEM_DIGMIXCTRL_DIGMIXFREQ.override = 150020 # Calc SRC
        phy.profile_outputs.MODEM_DSACTRL_ARRTHD.override = 4  # Was missed
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG0.override = 1
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG1.override = 3
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG2.override = 5
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG3.override = 5
        phy.profile_outputs.MODEM_LONGRANGE1_AVGWIN.override = 4
        phy.profile_outputs.MODEM_LONGRANGE1_HYSVAL.override = 3
        phy.profile_outputs.MODEM_LONGRANGE1_PREFILTLEN.override = 3
        phy.profile_outputs.MODEM_LONGRANGE1_LRTIMEOUTTHD.override = 320
        phy.profile_outputs.MODEM_LONGRANGE1_PREFILTLEN.override = 3
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH1.override = 20
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH2.override = 26
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH3.override = 33
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH4.override = 40
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH5.override = 46
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH6.override = 52
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH7.override = 59
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH8.override = 66
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH1.override = 3
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH2.override = 4
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH3.override = 5
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH4.override = 5
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH10.override = 80
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH9.override = 73
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH10.override = 11
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH11.override = 12
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH5.override = 6
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH6.override = 7
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH7.override = 8
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH8.override = 9
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH9.override = 10
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRSPIKETH.override = 40
        phy.profile_outputs.MODEM_LONGRANGE6_LRSPIKETHD.override = 105

        # phy.profile_outputs.MODEM_SRCCHF_SRCRATIO2.override = 15689 # Calc SRC
        phy.profile_outputs.MODEM_TIMING_TIMTHRESH.override = 35

        #phy.profile_outputs.RAC_PGACTRL_LNAMIXRFPKDTHRESHSEL.override = 2
        phy.profile_outputs.RAC_PGACTRL_PGATHRPKDHISEL.override = 5
        phy.profile_outputs.RAC_PGACTRL_PGATHRPKDLOSEL.override = 1
        #phy.profile_outputs.RAC_SYNTHCTRL_MMDPOWERBALANCEDISABLE.override = 1
        phy.profile_outputs.RAC_SYNTHREGCTRL_MMDLDOVREFTRIM.override = 3
        phy.profile_outputs.RAC_PGACTRL_PGAENLATCHI.override = 1
        phy.profile_outputs.RAC_PGACTRL_PGAENLATCHQ.override = 1
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1BWCAL.override = 11
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1COMPCAL.override = 14
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RFBVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RPVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RZVALCAL.override = 9
        # Derived empirically
        # https://confluence.silabs.com/display/RAIL/Panther+Weaksymbols+WifiBlocker+Characterization
        phy.profile_outputs.MODEM_CTRL2_SQITHRESH.override = 56

    def PHY_IEEE802154_2p4GHz_cohdsa(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='802154 2p4GHz cohdsa', phy_name=phy_name)
        model.vars.zigbee_feature.value_forced = model.vars.zigbee_feature.var_enum.COHERENT

        self.IEEE802154_2p4GHz_cohdsa_base(phy, model)
        phy.profile_outputs.MODEM_TXBR_TXBRDEN.override = 105
        phy.profile_outputs.MODEM_TXBR_TXBRNUM.override = 252

        return phy

    def PHY_IEEE802154_2p4GHz_cohdsa_diversity(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='802154 2p4GHz cohdsa', phy_name=phy_name)
        model.vars.zigbee_feature.value_forced = model.vars.zigbee_feature.var_enum.ANTDIV

        self.IEEE802154_2p4GHz_cohdsa_base(phy, model)
        phy.profile_outputs.MODEM_TXBR_TXBRDEN.override = 105
        phy.profile_outputs.MODEM_TXBR_TXBRNUM.override = 252

        return phy

    def PHY_IEEE802154_2p4GHz(self, model,phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Legacy IEEE 802.15.4 2p4GHz PHY from Jumbo',phy_name=phy_name)
        self.IEEE802154_2p4GHz_base(phy, model)

        phy.profile_outputs.AGC_CTRL2_DISRFPKD.override = 1
        phy.profile_outputs.AGC_CTRL4_RFPKDCNTEN.override = 0
        phy.profile_inputs.demod_select.value = model.vars.demod_select.var_enum.LEGACY

        return phy

    def PHY_IEEE802154_2p4GHz_diversity(self, model,phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Legacy IEEE 802.15.4 2p4GHz PHY from Jumbo',phy_name=phy_name)
        self.IEEE802154_2p4GHz_base(phy, model)
        model.vars.zigbee_feature.value_forced = model.vars.zigbee_feature.var_enum.ANTDIV

        phy.profile_outputs.AGC_CTRL2_DISRFPKD.override = 1
        phy.profile_outputs.AGC_CTRL4_RFPKDCNTEN.override = 0
        phy.profile_inputs.demod_select.value = model.vars.demod_select.var_enum.LEGACY

        return phy

    def IEEE802154_Internal_915MHz_cohdsa_base(self, phy, model):
        """ Frequency Planning """
        phy.profile_inputs.base_frequency_hz.value = long(915e6)
        phy.profile_inputs.channel_spacing_hz.value = 5000000
        phy.profile_inputs.bitrate.value = 250000
        phy.profile_inputs.deviation.value = 500000
        phy.profile_inputs.if_frequency_hz.value = 1370000

        phy.profile_inputs.bandwidth_hz.value = 2524800
        phy.profile_inputs.baudrate_tol_ppm.value = 0

        PHY_COMMON_FRAME_154(phy, model)
        # Override min length for 802.15.4E Seq# Suppression
        phy.profile_inputs.var_length_minlength.value = 4
        # Phy_Internal_Base.AGC_FAST_LOOP_base(phy, model)

        """ Modulation """
        phy.profile_inputs.demod_select.value = model.vars.demod_select.var_enum.COHERENT
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.OQPSK

        """ Shaping filter """
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Custom_OQPSK
        phy.profile_inputs.shaping_filter_param.value = 0.0

        """ Symbol mapping and encoding """
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.symbol_encoding.value = model.vars.symbol_encoding.var_enum.DSSS
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED

        """ DSSS Configurations """
        phy.profile_inputs.dsss_chipping_code.value = long(0x744AC39B)
        phy.profile_inputs.dsss_len.value = 32
        phy.profile_inputs.dsss_spreading_factor.value = 8

        """ Preamble """
        phy.profile_inputs.preamble_length.value = 32
        phy.profile_inputs.preamble_pattern.value = 0
        phy.profile_inputs.preamble_pattern_len.value = 4

        """ Timing """
        phy.profile_inputs.symbols_in_timing_window.value = 12
        phy.profile_inputs.timing_detection_threshold.value = 65
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.number_of_timing_windows.value = 7

        """ Sync """
        phy.profile_inputs.syncword_0.value = long(0xe5)
        phy.profile_inputs.syncword_1.value = long(0x0)
        phy.profile_inputs.syncword_length.value = 8

        """ XO Configuration """
        phy.profile_inputs.xtal_frequency_hz.value = 39000000
        phy.profile_inputs.rx_xtal_error_ppm.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0

        """ Dynamic BBSS """
        phy.profile_outputs.MODEM_LONGRANGE1_AVGWIN.override = 4

        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH1.override = 20
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH2.override = 26
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH3.override = 33
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH4.override = 40
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH5.override = 46
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH6.override = 52
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH7.override = 59
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH8.override = 66
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH9.override = 73
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH10.override = 80

        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH1.override = 3
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH2.override = 4
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH3.override = 5
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH4.override = 5
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH5.override = 6
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH6.override = 7
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH7.override = 8
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH8.override = 9
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH9.override = 10
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH10.override = 11
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH11.override = 12

        """ Pre-filter """
        phy.profile_outputs.MODEM_LONGRANGE1_PREFILTLEN.override = 3

        """ Coherent DSA """
        phy.profile_outputs.MODEM_LONGRANGE1_LRTIMEOUTTHD.override = 320
        phy.profile_outputs.MODEM_COH2_DSAPEAKCHPWRTH.override = 200

        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRSPIKETH.override = 40
        phy.profile_outputs.MODEM_LONGRANGE6_LRSPIKETHD.override = 105
        phy.profile_outputs.MODEM_COH2_FIXEDCDTHFORIIR.override = 105

        """ Dynamic Pre/Sync Threshold """
        phy.profile_outputs.MODEM_COH0_COHCHPWRTH0.override = 33
        phy.profile_outputs.MODEM_COH0_COHCHPWRTH1.override = 40
        phy.profile_outputs.MODEM_COH0_COHCHPWRTH2.override = 100

        phy.profile_outputs.MODEM_COH1_SYNCTHRESH0.override = 20
        phy.profile_outputs.MODEM_COH1_SYNCTHRESH1.override = 23
        phy.profile_outputs.MODEM_COH1_SYNCTHRESH2.override = 26

        phy.profile_outputs.MODEM_COH2_SYNCTHRESHDELTA1.override = 2
        phy.profile_outputs.MODEM_COH2_SYNCTHRESHDELTA2.override = 4

        """  """
        phy.profile_outputs.MODEM_CTRL5_RESYNCBAUDTRANS.override = 0

        """ For Ocelot, need to force adc clock mode to be same as bobcat """
        phy.profile_inputs.adc_clock_mode.value = model.vars.adc_clock_mode.var_enum.VCODIV

        # Derived empirically to improve wifi blocking performance
        # https://confluence.silabs.com/display/RAIL/Panther+Weaksymbols+WifiBlocker+Characterization
        phy.profile_outputs.MODEM_CTRL2_SQITHRESH.override = 56

    # Owner : Young-Joon Choi
    # Internal IEEE802154 PHY to check Bobcat PHY in Ocelot Silicon
    def PHY_Internal_IEEE802154_915MHz_cohdsa(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='802154 915 MHz cohdsa', phy_name=phy_name)
        self.IEEE802154_Internal_915MHz_cohdsa_base(phy, model)
        return phy

    # Owner : Young-Joon Choi
    # Internal IEEE802154 PHY to check Bobcat PHY Antenna Diversity in Ocelot Silicon
    def PHY_Internal_IEEE802154_915MHz_cohdsa_antdiv(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='802154 915 MHz cohdsa AntDiv', phy_name=phy_name)
        self.IEEE802154_Internal_915MHz_cohdsa_base(phy, model)

        """ Antenna Diversity Settings """
        phy.profile_inputs.antdivmode.value = model.vars.antdivmode.var_enum.ANTSELCORR
        phy.profile_inputs.antdivrepeatdis.value = model.vars.antdivrepeatdis.var_enum.NOREPEATFIRST
        return phy