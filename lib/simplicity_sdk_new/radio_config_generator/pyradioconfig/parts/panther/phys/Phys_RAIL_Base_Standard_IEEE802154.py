from pyradioconfig.parts.common.phys.phy_common import PHY_COMMON_FRAME_154
from pyradioconfig.parts.panther.phys.PHY_internal_base import Phy_Internal_Base
from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy
from py_2_and_3_compatibility import *


class PhysRAILBaseStandardIEEE802154Panther(IPhy):

    ################################################################################################
    # "base" definitions - allows usage in multiple PHYs

    def IEEE802154_2p4GHz_cohdsa_base(self, phy, model):

        PHY_COMMON_FRAME_154(phy, model)
        # Override min length for 802.15.4E Seq# Suppression
        phy.profile_inputs.var_length_minlength.value = 4
        Phy_Internal_Base.AGC_FAST_LOOP_base(phy, model)

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
        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.target_osr.value = 5 # Calc SRC

		# Additional overrides introduced when Series 2 AGC calculations added. These prevent the PHY from changing versus what was used during Validation.
        phy.profile_outputs.AGC_GAINRANGE_BOOSTLNA.override = 1

        phy.profile_outputs.AGC_GAINRANGE_LNABWADJ.override = 0
        phy.profile_outputs.AGC_CTRL0_PWRTARGET.override = 20
        phy.profile_outputs.AGC_CTRL1_PWRPERIOD.override = 4
        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        phy.profile_outputs.MODEM_AFC_AFCRXCLR.override = 1
        phy.profile_outputs.MODEM_AFC_AFCSCALEM.override = 3
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
        phy.profile_outputs.MODEM_DSACTRL_ARRTHD.override = 4 # Was missed
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG0.override = 1
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG1.override = 3
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG2.override = 5
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG3.override = 5
        phy.profile_outputs.MODEM_LONGRANGE1_AVGWIN.override = 4
        phy.profile_outputs.MODEM_LONGRANGE1_HYSVAL.override = 3
        phy.profile_outputs.MODEM_LONGRANGE1_LRTIMEOUTTHD.override = 320
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
        phy.profile_outputs.MODEM_TXBR_TXBRDEN.override = 105
        phy.profile_outputs.MODEM_TXBR_TXBRNUM.override = 252

        phy.profile_outputs.RAC_PGACTRL_LNAMIXRFPKDTHRESHSEL.override = 2
        phy.profile_outputs.RAC_PGACTRL_LNAMIXRFPKDTHRESHSEL.override = 2
        phy.profile_outputs.RAC_PGACTRL_PGATHRPKDHISEL.override = 5
        phy.profile_outputs.RAC_PGACTRL_PGATHRPKDLOSEL.override = 1
        phy.profile_outputs.RAC_SYNTHCTRL_MMDPOWERBALANCEDISABLE.override = 1
        phy.profile_outputs.RAC_SYNTHREGCTRL_MMDLDOVREFTRIM.override = 3
        phy.profile_outputs.RAC_PGACTRL_PGAENLATCHI.override = 1
        phy.profile_outputs.RAC_PGACTRL_PGAENLATCHQ.override = 1
        phy.profile_outputs.RFCRC_CTRL_INPUTINV.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1BWCAL.override = 11
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1COMPCAL.override = 14
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RFBVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RPVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RZVALCAL.override = 9
        # Derived empirically
        # https://confluence.silabs.com/display/RAIL/Panther+Weaksymbols+WifiBlocker+Characterization
        phy.profile_outputs.MODEM_CTRL2_SQITHRESH.override = 56


    def IEEE802154_2p4GHz_base(self, phy, model):

        PHY_COMMON_FRAME_154(phy, model)
        # Override min length for 802.15.4E Seq# Suppression
        phy.profile_inputs.var_length_minlength.value = 4
        Phy_Internal_Base.AGC_FAST_LOOP_base(phy, model)

        phy.profile_inputs.bandwidth_hz.value = 2524800
        phy.profile_inputs.base_frequency_hz.value = long(2405000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 4000
        phy.profile_inputs.bitrate.value = 250000
        phy.profile_inputs.channel_spacing_hz.value = 5000000
        phy.profile_inputs.deviation.value = 500000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.dsss_chipping_code.value = long(0x744AC39B)
        phy.profile_inputs.dsss_len.value = 32
        phy.profile_inputs.dsss_spreading_factor.value = 8
        phy.profile_inputs.frame_bitendian.value = model.vars.frame_bitendian.var_enum.LSB_FIRST
        phy.profile_inputs.frame_length_type.value = model.vars.frame_length_type.var_enum.VARIABLE_LENGTH
        phy.profile_inputs.frequency_comp_mode.value = model.vars.frequency_comp_mode.var_enum.INTERNAL_ALWAYS_ON
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.if_frequency_hz.value = 1370000
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.OQPSK
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
        phy.profile_inputs.syncword_0.value = long(0xe5)
        phy.profile_inputs.syncword_1.value = long(0x0)
        phy.profile_inputs.syncword_length.value = 8
        phy.profile_inputs.timing_detection_threshold.value = 65
        phy.profile_inputs.timing_resync_period.value = 2
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.xtal_frequency_hz.value = 38400000

        phy.profile_outputs.AGC_CTRL1_PWRPERIOD.override = 4
        # phy.profile_outputs.AGC_CTRL1_RSSIPERIOD.override = 3

        # Additional overrides introduced when Series 2 AGC calculations added. These prevent the PHY from changing versus what was used during Validation.
        phy.profile_outputs.AGC_GAINRANGE_BOOSTLNA.override = 1
        phy.profile_outputs.AGC_GAINRANGE_LNABWADJ.override = 1

        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 0x1003  # 0, 1, 12
        phy.profile_outputs.MODEM_TIMING_TIMTHRESH.override = 75
        # phy.profile_outputs.MODEM_SRCCHF_SRCENABLE1.override = 1 # Calc SRC

        phy.profile_outputs.RAC_PGACTRL_LNAMIXRFPKDTHRESHSEL.override = 2
        phy.profile_outputs.RAC_PGACTRL_PGATHRPKDHISEL.override = 5
        phy.profile_outputs.RAC_PGACTRL_PGATHRPKDLOSEL.override = 1
        phy.profile_outputs.RAC_SYNTHCTRL_MMDPOWERBALANCEDISABLE.override = 1
        phy.profile_outputs.RAC_SYNTHREGCTRL_MMDLDOVREFTRIM.override = 3
        phy.profile_outputs.RAC_PGACTRL_PGAENLATCHI.override = 1
        phy.profile_outputs.RAC_PGACTRL_PGAENLATCHQ.override = 1
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1BWCAL.override = 11
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1COMPCAL.override = 14
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RFBVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RPVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RZVALCAL.override = 9

    ################################################################################################
    # "PHY" definitions - official PHY created here, simulation PHYs in Phys_sim_tests.py

    def PHY_IEEE802154_2p4GHz_cohdsa(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='802154 2p4GHz cohdsa', phy_name=phy_name)

        self.IEEE802154_2p4GHz_cohdsa_base(phy, model)
        model.vars.zigbee_feature.value_forced = model.vars.zigbee_feature.var_enum.COHERENT

        phy.profile_outputs.rx_sync_delay_ns.override = 6625
        phy.profile_outputs.rx_eof_delay_ns.override = 6625

    def PHY_IEEE802154_2p4GHz(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Legacy IEEE 802.15.4 2p4GHz PHY from Jumbo', phy_name=phy_name)

        self.IEEE802154_2p4GHz_base(phy, model)
        model.vars.zigbee_feature.value_forced = model.vars.zigbee_feature.var_enum.LEGACY

    def PHY_IEEE802154_2p4GHz_antdiv(self, model, phy_name=None):
        #  PHY_IEEE802154_2p4GHz legacy, plus RSSI-based select best with antdiv repeat = NOREPEATFIRST
        phy = self._makePhy(model, model.profiles.Base, readable_name='IEEE802.15.4 2p4GHz antenna diversity PHY for Panther', phy_name=phy_name)

        self.IEEE802154_2p4GHz_base(phy, model)
        model.vars.zigbee_feature.value_forced = model.vars.zigbee_feature.var_enum.ANTDIV

        phy.profile_inputs.antdivmode.value = model.vars.antdivmode.var_enum.ANTSELRSSI
        phy.profile_inputs.antdivrepeatdis.value = model.vars.antdivrepeatdis.var_enum.NOREPEATFIRST

        phy.profile_outputs.rx_sync_delay_ns.override = 6625
        phy.profile_outputs.rx_eof_delay_ns.override = 6625

    def PHY_IEEE802154_2p4GHz_antdiv_fastswitch(self, model, phy_name=None):
        #  PHY_IEEE802154_2p4GHz legacy, plus RSSI-based select best with antdiv repeat = NOREPEATFIRST
        phy = self._makePhy(model, model.profiles.Base, readable_name='IEEE802.15.4 2p4GHz Concurrent PHY for Panther', phy_name=phy_name)

        ### same settings as PHY_IEEE802154_2p4GHz_antdiv_prod below
        self.IEEE802154_2p4GHz_base(phy, model)
        model.vars.zigbee_feature.value_forced = model.vars.zigbee_feature.var_enum.FCS

        phy.profile_inputs.antdivmode.value = model.vars.antdivmode.var_enum.ANTSELRSSI
        phy.profile_inputs.antdivrepeatdis.value = model.vars.antdivrepeatdis.var_enum.NOREPEATFIRST

        phy.profile_outputs.rx_sync_delay_ns.override = 6625
        phy.profile_outputs.rx_eof_delay_ns.override = 6625

        ### New PHY settings for 802154 fast channel switching below

        """ Bandwidth """
        phy.profile_inputs.pll_bandwidth_rx.value = model.vars.pll_bandwidth_rx.var_enum.FASTSWITCH

        # additional registers set in: RFHAL_IEEE802154_ConfigAntdivForRxChSwitching
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG2.override = 4
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG1.override = 2
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG0.override = 1

        phy.profile_outputs.MODEM_TIMING_TIMINGBASES.override = 2

        phy.profile_outputs.MODEM_CTRL6_TDREW.override = 0x20
        phy.profile_outputs.MODEM_CTRL6_PSTIMABORT0.override = 1
        phy.profile_outputs.MODEM_CTRL6_PSTIMABORT1.override = 1
        phy.profile_outputs.MODEM_CTRL6_PSTIMABORT2.override = 1
        phy.profile_outputs.MODEM_CTRL5_LINCORR.override = 0
        phy.profile_inputs.antdivmode.value = model.vars.antdivmode.var_enum.ANTSELFIRST

        phy.profile_outputs.MODEM_CTRL1_PHASEDEMOD.override = 0
        phy.profile_outputs.MODEM_TIMING_TIMTHRESH.override = 92
        phy.profile_outputs.SYNTH_DSMCTRLRX_DITHERDSMOUTPUTRX.override = 3

        # : improve +10 MHz cw blocking performance
        # : MCUW_RADIO_CFG-2298
        phy.profile_outputs.MODEM_DCCOMP_DCCOMPGEAR.override = 7




