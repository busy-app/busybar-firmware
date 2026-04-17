from pyradioconfig.parts.common.phys.phy_common import PHY_COMMON_FRAME_BLE, PHY_COMMON_FRAME_BLE_CODED
from pyradioconfig.parts.panther.phys.PHY_internal_base import Phy_Internal_Base
from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy
from pyradioconfig.parts.panther.calculators.calc_global import CALC_Global_panther

from py_2_and_3_compatibility import *


class PhysRAILBaseStandardBluetoothLEPanther(IPhy):

    #FIXME: there seem to be lots of extra overrides in these PHYs. Wentao needs to go over these and remove them.

    ################################################################################################
    # "base" definitions - allows usage in multiple PHYs

    def Bluetooth_LE_Viterbi_BLEIQDSA_base(self, phy, model):

        PHY_COMMON_FRAME_BLE(phy, model)
        Phy_Internal_Base.AGC_FAST_LOOP_base(phy, model)

        phy.profile_inputs.bandwidth_hz.value = 1099233
        phy.profile_inputs.base_frequency_hz.value = long(2402000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 1000
        phy.profile_inputs.bitrate.value = 1000000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.dsa_enable.value = True
        phy.profile_inputs.dsss_chipping_code.value = long(0)
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0
        phy.profile_inputs.freq_offset_hz.value = 0
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.if_frequency_hz.value = 1370000
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_1000KHz
        phy.profile_inputs.pll_bandwidth_rx.value = model.vars.pll_bandwidth_rx.var_enum.BW_250KHz
        phy.profile_inputs.rx_xtal_error_ppm.value = 20
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5
        phy.profile_inputs.symbols_in_timing_window.value = 0
        phy.profile_inputs.target_osr.value = 4
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.src1_range_available_minimum.value = 133

		# Additional overrides introduced when Series 2 AGC calculations added. These prevent the PHY from changing versus what was used during Validation.
        phy.profile_outputs.AGC_GAINRANGE_BOOSTLNA.override = 1
        phy.profile_outputs.AGC_GAINRANGE_LNABWADJ.override = 1

        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        # phy.profile_outputs.FRC_RXCTRL_RXFRAMEENDAHEADBYTES.override = 4
        phy.profile_outputs.MODEM_BLEIQDSA_BLEIQDSADIFFTH1.override = 1000
        phy.profile_outputs.MODEM_BLEIQDSA_BLEIQDSAEN.override = 1
        phy.profile_outputs.MODEM_BLEIQDSA_BLEIQDSAIIRCOEFPWR.override = 7
        phy.profile_outputs.MODEM_BLEIQDSA_BLEIQDSATH.override = 2500
        phy.profile_outputs.MODEM_BLEIQDSAEXT1_BLEIQDSAADDRBIAS.override = 4
        phy.profile_outputs.MODEM_BLEIQDSAEXT1_BLEIQDSATHCOMB.override = 3000
        phy.profile_outputs.MODEM_BLEIQDSAEXT1_CHPWRFIRAVGEN.override = 1
        phy.profile_outputs.MODEM_BLEIQDSAEXT1_CHPWRFIRAVGVAL.override = 3
        phy.profile_outputs.MODEM_BLEIQDSAEXT1_CORRIIRAVGMULFACT.override = 2
        phy.profile_outputs.MODEM_BLEIQDSAEXT1_FREQSCALEIQDSA.override = 2
        # phy.profile_outputs.MODEM_CF_CFOSR.override = 1 # Calc SRC
        # phy.profile_outputs.MODEM_CF_DEC0.override = 2 # Calc SRC
        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 65535
        phy.profile_outputs.MODEM_CTRL0_FRAMEDETDEL.override = 2
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3
        phy.profile_outputs.MODEM_CTRL2_DATAFILTER.override = 6
        phy.profile_outputs.MODEM_CTRL3_TSAMPDEL.override = 2 # Was missed
        phy.profile_outputs.MODEM_DSACTRL_ARRTOLERTHD0.override = 1
        phy.profile_outputs.MODEM_DSACTRL_ARRTOLERTHD1.override = 2
        phy.profile_outputs.MODEM_DSACTRL_FREQAVGSYM.override = 0
        phy.profile_outputs.MODEM_DSACTRL_GAINREDUCDLY.override = 2
        phy.profile_outputs.MODEM_DSATHD0_FDEVMAXTHD.override = 160
        phy.profile_outputs.MODEM_DSATHD0_FDEVMINTHD.override = 32
        phy.profile_outputs.MODEM_DSATHD0_SPIKETHD.override = 90
        phy.profile_outputs.MODEM_DSATHD0_UNMODTHD.override = 16
        phy.profile_outputs.MODEM_DSATHD1_AMPFLTBYP.override = 0
        phy.profile_outputs.MODEM_DSATHD1_FREQSCALE.override = 1
        phy.profile_outputs.MODEM_DSATHD1_POWABSTHD.override = 2000
        phy.profile_outputs.MODEM_DSATHD1_POWRELTHD.override = 2
        phy.profile_outputs.MODEM_DSATHD1_PWRDETDIS.override = 0
        phy.profile_outputs.MODEM_DSATHD2_FDADJTHD.override = 20
        phy.profile_outputs.MODEM_DSATHD2_FREQESTTHD.override = 16
        phy.profile_outputs.MODEM_DSATHD2_INTERFERDET.override = 1
        phy.profile_outputs.MODEM_DSATHD2_JUMPDETEN.override = 0
        phy.profile_outputs.MODEM_DSATHD2_PMDETFORCE.override = 1
        phy.profile_outputs.MODEM_DSATHD2_PMDETPASSTHD.override = 4
        phy.profile_outputs.MODEM_DSATHD2_POWABSTHDLOG.override = 241
        phy.profile_outputs.MODEM_DSATHD3_FDEVMAXTHDLO.override = 160
        phy.profile_outputs.MODEM_DSATHD3_FDEVMINTHDLO.override = 32
        phy.profile_outputs.MODEM_DSATHD3_SPIKETHDLO.override = 90
        phy.profile_outputs.MODEM_DSATHD3_UNMODTHDLO.override = 16
        phy.profile_outputs.MODEM_DSATHD4_ARRTOLERTHD0LO.override = 1
        phy.profile_outputs.MODEM_DSATHD4_ARRTOLERTHD1LO.override = 2
        phy.profile_outputs.MODEM_DSATHD4_POWABSTHDLO.override = 2000
        phy.profile_outputs.MODEM_DSATHD4_SWTHD.override = 1
        phy.profile_outputs.MODEM_LONGRANGE1_HYSVAL.override = 2
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH1.override = 16
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH2.override = 22
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH3.override = 28
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH4.override = 34
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH5.override = 40
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH6.override = 46
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH7.override = 52
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH8.override = 58
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH1.override = 1
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH2.override = 2
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH3.override = 3
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH4.override = 4
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH10.override = 70
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH9.override = 64
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH10.override = 10
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH11.override = 11
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH5.override = 5
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH6.override = 6
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH7.override = 7
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH8.override = 8
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH9.override = 9
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRSH12.override = 11
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRTH11.override = 76
        #phy.profile_outputs.MODEM_MODINDEX_MODINDEXM.override = 27
        phy.profile_outputs.MODEM_PRE_PREERRORS.override = 1
        phy.profile_outputs.MODEM_RXBR_RXBRDEN.override = 4

        self.BLE_TX_Shaping_PantherZ_Compatible_Coeffs(phy, model)

        phy.profile_outputs.MODEM_SRCCHF_BWSEL.override = 1
        phy.profile_outputs.MODEM_TIMING_OFFSUBDEN.override = 0 # Was missed
        phy.profile_outputs.MODEM_TIMING_OFFSUBNUM.override = 0 # Was missed
        phy.profile_outputs.MODEM_TIMING_TIMTHRESH.override = 140
        phy.profile_outputs.MODEM_TXBR_TXBRDEN.override = 5
        phy.profile_outputs.MODEM_TXBR_TXBRNUM.override = 24
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI1.override = 62
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI2.override = 42
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 30
        phy.profile_outputs.MODEM_VITERBIDEMOD_VTDEMODEN.override = 1
        phy.profile_outputs.MODEM_VITERBIDEMOD_DISDEMODOF.override = 1
        phy.profile_outputs.MODEM_VTBLETIMING_TIMINGDELAY.override = 60
        phy.profile_outputs.MODEM_VTBLETIMING_VTBLETIMINGSEL.override = 1
        phy.profile_outputs.MODEM_VTCORRCFG1_BUFFHEAD.override = 10
        phy.profile_outputs.MODEM_VTCORRCFG1_CORRSHFTLEN.override = 48
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPSYNCLEN.override = 148
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 100
        phy.profile_outputs.MODEM_VTTRACK_FREQTRACKMODE.override = 1

        phy.profile_outputs.RAC_SYNTHCTRL_MMDPOWERBALANCEDISABLE.override = 1
        phy.profile_outputs.RAC_SYNTHREGCTRL_MMDLDOVREFTRIM.override = 3
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1BWCAL.override = 11
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1COMPCAL.override = 14
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RFBVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RPVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RZVALCAL.override = 9

    def Bluetooth_LE_2M_Viterbi_BLEIQDSA_base(self, phy, model):

        PHY_COMMON_FRAME_BLE(phy, model)
        Phy_Internal_Base.AGC_FAST_LOOP_base(phy, model)

        phy.profile_inputs.bandwidth_hz.value = 2200000
        phy.profile_inputs.base_frequency_hz.value = long(2402000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 1000
        phy.profile_inputs.bitrate.value = 2000000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.deviation.value = 500000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.dsa_enable.value = True
        phy.profile_inputs.dsss_chipping_code.value = long(0)
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0
        phy.profile_inputs.freq_offset_hz.value = 0
        phy.profile_inputs.frequency_comp_mode.value = model.vars.frequency_comp_mode.var_enum.INTERNAL_ALWAYS_ON
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.if_frequency_hz.value = 1370000
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_1500KHz
        phy.profile_inputs.pll_bandwidth_rx.value = model.vars.pll_bandwidth_rx.var_enum.BW_250KHz
        phy.profile_inputs.preamble_length.value = 16
        phy.profile_inputs.rx_xtal_error_ppm.value = 20
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5
        phy.profile_inputs.target_osr.value = 4
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.xtal_frequency_hz.value = 38400000

        self.BLE_2M_TX_Shaping_Coeffs(phy, model)

        # Additional overrides introduced when Series 2 AGC calculations added. These prevent the PHY from changing versus what was used during Validation.
        phy.profile_outputs.AGC_GAINRANGE_BOOSTLNA.override = 1
        phy.profile_outputs.AGC_GAINRANGE_LNABWADJ.override = 1
        phy.profile_outputs.AGC_AGCPERIOD_PERIODHI.override = 10 # different in 1mbps vs 2mbps
        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        # phy.profile_outputs.FRC_RXCTRL_RXFRAMEENDAHEADBYTES.override = 4
        phy.profile_outputs.MODEM_BLEIQDSA_BLEIQDSADIFFTH1.override = 1050
        phy.profile_outputs.MODEM_BLEIQDSA_BLEIQDSAEN.override = 1
        phy.profile_outputs.MODEM_BLEIQDSA_BLEIQDSAIIRCOEFPWR.override = 7
        phy.profile_outputs.MODEM_BLEIQDSA_BLEIQDSATH.override = 2850
        phy.profile_outputs.MODEM_BLEIQDSAEXT1_BLEIQDSAADDRBIAS.override = 4
        phy.profile_outputs.MODEM_BLEIQDSAEXT1_BLEIQDSATHCOMB.override = 3300
        phy.profile_outputs.MODEM_BLEIQDSAEXT1_CHPWRFIRAVGEN.override = 1
        phy.profile_outputs.MODEM_BLEIQDSAEXT1_CHPWRFIRAVGVAL.override = 3
        phy.profile_outputs.MODEM_BLEIQDSAEXT1_CORRIIRAVGMULFACT.override = 2
        # phy.profile_outputs.MODEM_CF_CFOSR.override = 1 # Calc SRC
        # phy.profile_outputs.MODEM_CF_DEC0.override = 2 # Calc SRC
        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 65535
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3
        phy.profile_outputs.MODEM_CTRL1_RESYNCPER.override = 0
        phy.profile_outputs.MODEM_CTRL2_DATAFILTER.override = 6
        phy.profile_outputs.MODEM_CTRL3_TSAMPDEL.override = 3
        phy.profile_outputs.MODEM_CTRL3_TSAMPLIM.override = 50
        phy.profile_outputs.MODEM_LONGRANGE1_HYSVAL.override = 2
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH1.override = 18
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH2.override = 24
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH3.override = 30
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH4.override = 36
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH5.override = 42
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH6.override = 48
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH7.override = 54
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH8.override = 60
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH9.override = 66
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH10.override = 72
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRTH11.override = 78
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH1.override = 1
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH2.override = 2
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH3.override = 3
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH4.override = 4
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH5.override = 5
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH6.override = 6
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH7.override = 7
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH8.override = 8
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH9.override = 9
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH10.override = 10
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH11.override = 11
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRSH12.override = 12
        phy.profile_outputs.MODEM_MODINDEX_FREQGAINE.override = 2
        phy.profile_outputs.MODEM_RXBR_RXBRDEN.override = 4
        phy.profile_outputs.MODEM_SRCCHF_BWSEL.override = 1
        phy.profile_outputs.MODEM_TIMING_OFFSUBDEN.override = 0 # Was missed
        phy.profile_outputs.MODEM_TIMING_OFFSUBNUM.override = 0 # Was missed
        phy.profile_outputs.MODEM_TIMING_TIMTHRESH.override = 8
        #phy.profile_outputs.MODEM_TXBR_TXBRDEN.override = 105
        #phy.profile_outputs.MODEM_TXBR_TXBRNUM.override = 252
        phy.profile_outputs.MODEM_VITERBIDEMOD_VTDEMODEN.override = 1
        phy.profile_outputs.MODEM_VITERBIDEMOD_DISDEMODOF.override = 1
        phy.profile_outputs.MODEM_VTBLETIMING_TIMINGDELAY.override = 140 # Per Divyansh email
        phy.profile_outputs.MODEM_VTBLETIMING_VTBLETIMINGSEL.override = 1
        phy.profile_outputs.MODEM_VTCORRCFG1_BUFFHEAD.override = 4
        phy.profile_outputs.MODEM_VTCORRCFG1_CORRSHFTLEN.override = 42
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPSYNCLEN.override = 148
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 80
        phy.profile_outputs.MODEM_VTTRACK_FREQTRACKMODE.override = 1
        phy.profile_outputs.MODEM_VTTRACK_HIPWRTHD.override = 43

        phy.profile_outputs.RAC_PGACTRL_LNAMIXRFPKDTHRESHSEL.override = 2
        phy.profile_outputs.RAC_SYNTHCTRL_MMDPOWERBALANCEDISABLE.override = 1
        phy.profile_outputs.RAC_SYNTHREGCTRL_MMDLDOVREFTRIM.override = 3
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1BWCAL.override = 11
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1COMPCAL.override = 14
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RFBVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RPVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RZVALCAL.override = 9

    def Bluetooth_LE_Viterbi_base(self, phy, model):

        PHY_COMMON_FRAME_BLE(phy, model)
        Phy_Internal_Base.AGC_FAST_LOOP_base(phy, model)

        phy.profile_inputs.bandwidth_hz.value = 1099233
        phy.profile_inputs.base_frequency_hz.value = long(2402000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 1000
        phy.profile_inputs.bitrate.value = 1000000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.dsa_enable.value = True
        phy.profile_inputs.dsss_chipping_code.value = long(0)
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0
        phy.profile_inputs.errors_in_timing_window.value = 1
        phy.profile_inputs.freq_offset_hz.value = 0
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.if_frequency_hz.value = 1370000
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_1000KHz
        phy.profile_inputs.pll_bandwidth_rx.value = model.vars.pll_bandwidth_rx.var_enum.BW_250KHz
        phy.profile_inputs.preamble_length.value = 8
        phy.profile_inputs.rx_xtal_error_ppm.value = 20
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5
        phy.profile_inputs.symbols_in_timing_window.value = 0
        phy.profile_inputs.target_osr.value = 4
        phy.profile_inputs.timing_detection_threshold.value = 140
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.xtal_frequency_hz.value = 38400000

        # Additional overrides introduced when Series 2 AGC calculations added. These prevent the PHY from changing versus what was used during Validation.
        phy.profile_outputs.AGC_GAINRANGE_BOOSTLNA.override = 1
        phy.profile_outputs.AGC_GAINRANGE_LNABWADJ.override = 1

        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        # phy.profile_outputs.FRC_RXCTRL_RXFRAMEENDAHEADBYTES.override = 4

        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 65535
        phy.profile_outputs.MODEM_RXBR_RXBRDEN.override = 4
        phy.profile_outputs.MODEM_SRCCHF_BWSEL.override = 1
        phy.profile_outputs.MODEM_TIMING_OFFSUBDEN.override = 0
        phy.profile_outputs.MODEM_TIMING_OFFSUBNUM.override = 0
        phy.profile_outputs.MODEM_TXBR_TXBRDEN.override = 5
        phy.profile_outputs.MODEM_TXBR_TXBRNUM.override = 24
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3
        phy.profile_outputs.MODEM_CTRL2_DATAFILTER.override = 6
        phy.profile_outputs.MODEM_DIGMIXCTRL_DIGMIXMODE.override = 1
        phy.profile_outputs.MODEM_MIXCTRL_DIGIQSWAPEN.override = 1
        phy.profile_outputs.MODEM_MODINDEX_FREQGAINE.override = 2
        phy.profile_outputs.MODEM_MODINDEX_FREQGAINM.override = 4
        #### phase DSA settings
        phy.profile_outputs.MODEM_BLEIQDSA_BLEIQDSAEN.override = 0
        phy.profile_outputs.MODEM_DSACTRL_ARRTHD.override = 4
        phy.profile_outputs.MODEM_DSACTRL_ARRTOLERTHD0.override = 1
        phy.profile_outputs.MODEM_DSACTRL_ARRTOLERTHD1.override = 2
        phy.profile_outputs.MODEM_DSACTRL_DSAMODE.override = 1
        phy.profile_outputs.MODEM_DSACTRL_FREQAVGSYM.override = 0
        phy.profile_outputs.MODEM_DSACTRL_GAINREDUCDLY.override = 2
        phy.profile_outputs.MODEM_DSATHD0_FDEVMAXTHD.override = 90
        phy.profile_outputs.MODEM_DSATHD0_FDEVMINTHD.override = 16
        phy.profile_outputs.MODEM_DSATHD0_SPIKETHD.override = 60
        phy.profile_outputs.MODEM_DSATHD0_UNMODTHD.override = 8
        phy.profile_outputs.MODEM_DSATHD1_AMPFLTBYP.override = 0
        phy.profile_outputs.MODEM_DSATHD1_FREQSCALE.override = 1
        phy.profile_outputs.MODEM_DSATHD1_POWABSTHD.override = 3000
        phy.profile_outputs.MODEM_DSATHD1_POWRELTHD.override = 2
        phy.profile_outputs.MODEM_DSATHD1_PWRDETDIS.override = 0
        phy.profile_outputs.MODEM_DSATHD2_FDADJTHD.override = 20
        phy.profile_outputs.MODEM_DSATHD2_FREQESTTHD.override = 16
        phy.profile_outputs.MODEM_DSATHD2_INTERFERDET.override = 1
        phy.profile_outputs.MODEM_DSATHD2_JUMPDETEN.override = 0
        phy.profile_outputs.MODEM_DSATHD2_PMDETFORCE.override = 1
        phy.profile_outputs.MODEM_DSATHD2_PMDETPASSTHD.override = 4
        phy.profile_outputs.MODEM_DSATHD2_POWABSTHDLOG.override = 248
        phy.profile_outputs.MODEM_DSATHD3_FDEVMAXTHDLO.override = 160
        phy.profile_outputs.MODEM_DSATHD3_FDEVMINTHDLO.override = 8
        phy.profile_outputs.MODEM_DSATHD3_SPIKETHDLO.override = 90
        phy.profile_outputs.MODEM_DSATHD3_UNMODTHDLO.override = 4
        phy.profile_outputs.MODEM_DSATHD4_ARRTOLERTHD0LO.override = 1
        phy.profile_outputs.MODEM_DSATHD4_ARRTOLERTHD1LO.override = 3
        phy.profile_outputs.MODEM_DSATHD4_POWABSTHDLO.override = 1000
        phy.profile_outputs.MODEM_DSATHD4_SWTHD.override = 1
        #### Viterbi Demod settings
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI1.override = 62
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI2.override = 42
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 30
        phy.profile_outputs.MODEM_VITERBIDEMOD_VTDEMODEN.override = 1
        phy.profile_outputs.MODEM_VITERBIDEMOD_DISDEMODOF.override = 1
        phy.profile_outputs.MODEM_VTBLETIMING_TIMINGDELAY.override = 60
        phy.profile_outputs.MODEM_VTBLETIMING_VTBLETIMINGSEL.override = 1
        phy.profile_outputs.MODEM_VTCORRCFG1_BUFFHEAD.override = 10
        phy.profile_outputs.MODEM_VTCORRCFG1_CORRSHFTLEN.override = 60
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPSYNCLEN.override = 148
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 100
        phy.profile_outputs.MODEM_VTTRACK_FREQTRACKMODE.override = 1
        phy.profile_outputs.MODEM_VTTRACK_HIPWRTHD.override = 1

        self.BLE_TX_Shaping_Coeffs(phy, model)

        phy.profile_outputs.RAC_SYNTHCTRL_MMDPOWERBALANCEDISABLE.override = 1
        phy.profile_outputs.RAC_SYNTHREGCTRL_MMDLDOVREFTRIM.override = 3
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1BWCAL.override = 11
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1COMPCAL.override = 14
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RFBVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RPVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RZVALCAL.override = 9


    def Bluetooth_LE_2M_Viterbi_base(self, phy, model):

        PHY_COMMON_FRAME_BLE(phy, model)
        Phy_Internal_Base.AGC_FAST_LOOP_base(phy, model)

        phy.profile_inputs.bandwidth_hz.value = 2500000
        phy.profile_inputs.base_frequency_hz.value = long(2402000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 1000
        phy.profile_inputs.bitrate.value = 2000000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.deviation.value = 500000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.dsa_enable.value = True
        phy.profile_inputs.dsss_chipping_code.value = long(0)
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0
        phy.profile_inputs.errors_in_timing_window.value = 1
        phy.profile_inputs.freq_offset_hz.value = 0
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.if_frequency_hz.value = 1370000
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_1500KHz
        phy.profile_inputs.pll_bandwidth_rx.value = model.vars.pll_bandwidth_rx.var_enum.BW_250KHz
        phy.profile_inputs.preamble_length.value = 16
        phy.profile_inputs.rx_xtal_error_ppm.value = 20
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5
        phy.profile_inputs.symbols_in_timing_window.value = 0
        phy.profile_inputs.target_osr.value = 4
        phy.profile_inputs.timing_detection_threshold.value = 140
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.src1_range_available_minimum.value = 133


        self.BLE_2M_TX_Shaping_Coeffs(phy, model)

		# Additional overrides introduced when Series 2 AGC calculations added. These prevent the PHY from changing versus what was used during Validation.
        phy.profile_outputs.AGC_GAINRANGE_BOOSTLNA.override = 1
        phy.profile_outputs.AGC_GAINRANGE_LNABWADJ.override = 1

        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        # phy.profile_outputs.FRC_RXCTRL_RXFRAMEENDAHEADBYTES.override = 4

        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 65535
        phy.profile_outputs.MODEM_RXBR_RXBRDEN.override = 4
        phy.profile_outputs.MODEM_SRCCHF_BWSEL.override = 1
        phy.profile_outputs.MODEM_TIMING_OFFSUBDEN.override = 0
        phy.profile_outputs.MODEM_TIMING_OFFSUBNUM.override = 0
        #phy.profile_outputs.MODEM_TXBR_TXBRDEN.override = 5
        #phy.profile_outputs.MODEM_TXBR_TXBRNUM.override = 24
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3
        phy.profile_outputs.MODEM_CTRL2_DATAFILTER.override = 6
        phy.profile_outputs.MODEM_DIGMIXCTRL_DIGMIXMODE.override = 1
        phy.profile_outputs.MODEM_MIXCTRL_DIGIQSWAPEN.override = 1
        phy.profile_outputs.MODEM_MODINDEX_FREQGAINE.override = 2
        phy.profile_outputs.MODEM_MODINDEX_FREQGAINM.override = 4

        phy.profile_outputs.MODEM_SRCCHF_BWSEL.override = 1
        phy.profile_outputs.MODEM_SRCCHF_INTOSR.override = 1
        phy.profile_outputs.MODEM_SRCCHF_SRCENABLE1.override = 1
        phy.profile_outputs.MODEM_SRCCHF_SRCENABLE2.override = 1
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3
        phy.profile_outputs.MODEM_CTRL2_DATAFILTER.override = 6
        phy.profile_outputs.MODEM_DIGMIXCTRL_DIGMIXMODE.override = 1
        phy.profile_outputs.MODEM_MIXCTRL_DIGIQSWAPEN.override = 1
        phy.profile_outputs.MODEM_MODINDEX_FREQGAINE.override = 2
        phy.profile_outputs.MODEM_MODINDEX_FREQGAINM.override = 4
        phy.profile_outputs.MODEM_CF_CFOSR.override = 1
        phy.profile_outputs.MODEM_CF_DEC0.override = 2

        #### phase DSA settings
        phy.profile_outputs.MODEM_BLEIQDSA_BLEIQDSAEN.override = 0
        phy.profile_outputs.MODEM_DSACTRL_ARRTHD.override = 5
        phy.profile_outputs.MODEM_DSACTRL_ARRTOLERTHD0.override = 1
        phy.profile_outputs.MODEM_DSACTRL_ARRTOLERTHD1.override = 2
        phy.profile_outputs.MODEM_DSACTRL_DSAMODE.override = 1
        phy.profile_outputs.MODEM_DSACTRL_FREQAVGSYM.override = 0
        phy.profile_outputs.MODEM_DSACTRL_GAINREDUCDLY.override = 2
        phy.profile_outputs.MODEM_DSATHD0_FDEVMAXTHD.override = 180
        phy.profile_outputs.MODEM_DSATHD0_FDEVMINTHD.override = 16
        phy.profile_outputs.MODEM_DSATHD0_SPIKETHD.override = 60
        phy.profile_outputs.MODEM_DSATHD0_UNMODTHD.override = 8
        phy.profile_outputs.MODEM_DSATHD1_AMPFLTBYP.override = 0
        phy.profile_outputs.MODEM_DSATHD1_FREQSCALE.override = 1
        phy.profile_outputs.MODEM_DSATHD1_POWABSTHD.override = 3000
        phy.profile_outputs.MODEM_DSATHD1_POWRELTHD.override = 2
        phy.profile_outputs.MODEM_DSATHD1_PWRDETDIS.override = 0
        phy.profile_outputs.MODEM_DSATHD2_FDADJTHD.override = 20
        phy.profile_outputs.MODEM_DSATHD2_FREQESTTHD.override = 16
        phy.profile_outputs.MODEM_DSATHD2_INTERFERDET.override = 1
        phy.profile_outputs.MODEM_DSATHD2_JUMPDETEN.override = 0
        phy.profile_outputs.MODEM_DSATHD2_PMDETFORCE.override = 1
        phy.profile_outputs.MODEM_DSATHD2_PMDETPASSTHD.override = 4
        phy.profile_outputs.MODEM_DSATHD2_POWABSTHDLOG.override = 248
        phy.profile_outputs.MODEM_DSATHD3_FDEVMAXTHDLO.override = 180
        phy.profile_outputs.MODEM_DSATHD3_FDEVMINTHDLO.override = 8
        phy.profile_outputs.MODEM_DSATHD3_SPIKETHDLO.override = 90
        phy.profile_outputs.MODEM_DSATHD3_UNMODTHDLO.override = 4
        phy.profile_outputs.MODEM_DSATHD4_ARRTOLERTHD0LO.override = 1
        phy.profile_outputs.MODEM_DSATHD4_ARRTOLERTHD1LO.override = 3
        phy.profile_outputs.MODEM_DSATHD4_POWABSTHDLO.override = 1500
        phy.profile_outputs.MODEM_DSATHD4_SWTHD.override = 1
        #### Viterbi Demod settings
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI1.override = 64
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI2.override = 48
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 32
        phy.profile_outputs.MODEM_VITERBIDEMOD_VTDEMODEN.override = 1
        phy.profile_outputs.MODEM_VITERBIDEMOD_DISDEMODOF.override = 1
        phy.profile_outputs.MODEM_VTBLETIMING_TIMINGDELAY.override = 220
        phy.profile_outputs.MODEM_VTBLETIMING_VTBLETIMINGSEL.override = 1
        phy.profile_outputs.MODEM_VTBLETIMING_FLENOFF.override = 4
        phy.profile_outputs.MODEM_VTCORRCFG1_BUFFHEAD.override = 4
        phy.profile_outputs.MODEM_VTCORRCFG1_CORRSHFTLEN.override = 52
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPSYNCLEN.override = 148
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 220
        phy.profile_outputs.MODEM_VTTRACK_FREQTRACKMODE.override = 1
        phy.profile_outputs.MODEM_VTTRACK_HIPWRTHD.override = 43

        phy.profile_outputs.RAC_SYNTHCTRL_MMDPOWERBALANCEDISABLE.override = 1
        phy.profile_outputs.RAC_SYNTHREGCTRL_MMDLDOVREFTRIM.override = 3
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1BWCAL.override = 11
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1COMPCAL.override = 14
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RFBVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RPVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RZVALCAL.override = 9



    def Bluetooth_LE_base(self, phy, model):

        PHY_COMMON_FRAME_BLE(phy, model)
        Phy_Internal_Base.AGC_FAST_LOOP_base(phy, model)

        phy.profile_inputs.bandwidth_hz.value = 1260000
        phy.profile_inputs.base_frequency_hz.value = long(2450000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 1000
        phy.profile_inputs.bitrate.value = 1000000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.dsa_enable.value = True
        phy.profile_inputs.dsss_chipping_code.value = long(0)
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0
        phy.profile_inputs.errors_in_timing_window.value = 1
        phy.profile_inputs.freq_offset_hz.value = 0
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.if_frequency_hz.value = 1370000
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_1000KHz
        phy.profile_inputs.pll_bandwidth_rx.value = model.vars.pll_bandwidth_rx.var_enum.BW_250KHz
        phy.profile_inputs.preamble_length.value = 8
        phy.profile_inputs.rssi_period.value = 3
        phy.profile_inputs.rx_xtal_error_ppm.value = 20
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5
        phy.profile_inputs.symbols_in_timing_window.value = 0
        phy.profile_inputs.target_osr.value = 5
        phy.profile_inputs.timing_detection_threshold.value = 140
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.xtal_frequency_hz.value = 38400000

		# Additional overrides introduced when Series 2 AGC calculations added. These prevent the PHY from changing versus what was used during Validation.
        phy.profile_outputs.AGC_GAINRANGE_BOOSTLNA.override = 1
        phy.profile_outputs.AGC_GAINRANGE_LNABWADJ.override = 1

        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        # phy.profile_outputs.FRC_RXCTRL_RXFRAMEENDAHEADBYTES.override = 4
        phy.profile_outputs.MODEM_CTRL0_FRAMEDETDEL.override = 2
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3
        phy.profile_outputs.MODEM_CTRL2_SQITHRESH.override = 200
        phy.profile_outputs.MODEM_DSACTRL_DSAMODE.override = 0
        phy.profile_outputs.MODEM_TIMING_OFFSUBNUM.override = 7

        self.BLE_TX_Shaping_Coeffs(phy, model)

        phy.profile_outputs.RAC_SYNTHCTRL_MMDPOWERBALANCEDISABLE.override = 1
        phy.profile_outputs.RAC_SYNTHREGCTRL_MMDLDOVREFTRIM.override = 3
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1BWCAL.override = 11
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1COMPCAL.override = 14
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RFBVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RPVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RZVALCAL.override = 9

    def Bluetooth_LongRange_base(self, phy, model):

        PHY_COMMON_FRAME_BLE_CODED(phy, model)
        Phy_Internal_Base.AGC_FAST_LOOP_base(phy, model)

        phy.profile_inputs.bandwidth_hz.value = 1230000
        phy.profile_inputs.base_frequency_hz.value = long(2450000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 1000
        phy.profile_inputs.bitrate.value = 1000000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.dsa_enable.value = True
        phy.profile_inputs.dsss_chipping_code.value = long(0)
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0
        phy.profile_inputs.errors_in_timing_window.value = 1
        phy.profile_inputs.freq_offset_hz.value = 0
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.header_addtrailtxdata_en.value = True        # We need to add Trail TX Data to the header (subframe 0), which is the Coding/Rate Indication (CI/RI)
        phy.profile_inputs.header_calc_crc.value = False                # The header (subframe 0) does not include a CRC
        phy.profile_inputs.header_excludesubframewcnt_en.value = True   # With Dynamic frame length (DFL) mode, we need to exclude the header (subframe 0) from Word Counter
        phy.profile_inputs.header_white_en.value = False                # No whitening in the header (subframe 0)
        phy.profile_inputs.if_frequency_hz.value = 1370000
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_1000KHz
        phy.profile_inputs.pll_bandwidth_rx.value = model.vars.pll_bandwidth_rx.var_enum.BW_200KHz
        phy.profile_inputs.preamble_length.value = 20
        phy.profile_inputs.rx_xtal_error_ppm.value = 20
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.NONE
        phy.profile_inputs.shaping_filter_param.value = 0.5
        phy.profile_inputs.symbols_in_timing_window.value = 0
        phy.profile_inputs.target_osr.value = 4
        phy.profile_inputs.timing_detection_threshold.value = 140
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.src1_range_available_minimum.value = 128
        # Explicitly disallow DEC0 dec-by-3 or dec-by-8 per Yan's findings
        phy.profile_inputs.input_decimation_filter_allow_dec3.value = 0
        phy.profile_inputs.input_decimation_filter_allow_dec8.value = 0
        phy.profile_outputs.MODEM_VTTRACK_TIMEACQUTHD.override = 32

        self.BLE_TX_Shaping_Coeffs(phy, model)

		# Additional overrides introduced when Series 2 AGC calculations added. These prevent the PHY from changing versus what was used during Validation.
        phy.profile_outputs.AGC_GAINRANGE_BOOSTLNA.override = 1
        phy.profile_outputs.AGC_GAINRANGE_LNABWADJ.override = 1

        phy.profile_outputs.FRC_CTRL_RXFCDMODE.override = 0             # Override the RXFCDMODE to FCDMODE0 (FCD2 is reloaded when SCNT reaches 0)
        # phy.profile_outputs.FRC_RXCTRL_RXFRAMEENDAHEADBYTES.override = 4

        # NOTE: Currently the 'header_include_crc' variable is not part of the profile,
        # but if it ever gets added, we should set it to False
        # phy.profile_inputs.header_include_crc.value = False
        # For now, we need to directly override the associated profile output
        phy.profile_outputs.FRC_FCD0_INCLUDECRC.override = 0

        phy.profile_outputs.FRC_FCD0_WORDS.override = 3                 # SYNCWORD size (4 bytes) - 1 = 3
        phy.profile_outputs.FRC_FCD2_ADDTRAILTXDATA.override = 0
        phy.profile_outputs.FRC_FCD2_CALCCRC.override = 1
        phy.profile_outputs.FRC_FCD2_EXCLUDESUBFRAMEWCNT.override = 0
        phy.profile_outputs.FRC_FCD2_INCLUDECRC.override = 1
        phy.profile_outputs.FRC_FCD2_WORDS.override = 2 ** phy.profile_outputs.FRC_FCD2_WORDS.var.get_bit_width() - 1
        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        phy.profile_outputs.FRC_CONVGENERATOR_GENERATOR0.override = 13
        phy.profile_outputs.FRC_CONVGENERATOR_GENERATOR1.override = 15
        phy.profile_outputs.FRC_FECCTRL_CONVMODE.override = 1
        phy.profile_outputs.FRC_PUNCTCTRL_PUNCT0.override = 1
        phy.profile_outputs.FRC_PUNCTCTRL_PUNCT1.override = 1
        phy.profile_outputs.FRC_TRAILTXDATACTRL_TRAILTXDATACNT.override = 4
        phy.profile_outputs.MODEM_AFC_AFCRXMODE.override = 0 # Was missed
        phy.profile_outputs.MODEM_BLEIQDSAEXT1_BLEIQDSATHCOMB.override = 1
        phy.profile_outputs.MODEM_CTRL0_CODING.override = 3
        phy.profile_outputs.MODEM_CTRL0_FRAMEDETDEL.override = 1
        phy.profile_outputs.MODEM_CTRL1_COMPMODE.override = 3
        phy.profile_outputs.MODEM_CTRL1_SYNCDATA.override = 1
        phy.profile_outputs.MODEM_CTRL2_RATESELMODE.override = 2
        phy.profile_outputs.MODEM_CTRL3_TSAMPDEL.override =2 # Was missed
        phy.profile_outputs.MODEM_DSACTRL_DSAMODE.override = 0  # Was missed
        phy.profile_outputs.MODEM_LONGRANGE1_AVGWIN.override = 3
        phy.profile_outputs.MODEM_LONGRANGE1_CHPWRACCUDEL.override = 3
        phy.profile_outputs.MODEM_LONGRANGE1_HYSVAL.override = 3
        phy.profile_outputs.MODEM_LONGRANGE1_LOGICBASEDLRDEMODGATE.override = 1
        phy.profile_outputs.MODEM_LONGRANGE1_LOGICBASEDPUGATE.override = 1
        phy.profile_outputs.MODEM_LONGRANGE1_LRSS.override = 4
        phy.profile_outputs.MODEM_LONGRANGE1_LRTIMEOUTTHD.override = 400
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH1.override = 8
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH2.override = 14
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH3.override = 20
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH4.override = 26
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH5.override = 32
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH6.override = 38
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH7.override = 44
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH8.override = 50
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH2.override = 1
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH3.override = 2
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH4.override = 3
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH10.override = 62
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH9.override = 56
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH10.override = 9
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH11.override = 10
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH5.override = 4
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH6.override = 5
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH7.override = 6
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH8.override = 7
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH9.override = 8
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRSPIKETH.override = 29
        phy.profile_outputs.MODEM_LONGRANGE6_LRSPIKETHD.override = 95
        phy.profile_outputs.MODEM_LONGRANGE_LRBLE.override = 1
        phy.profile_outputs.MODEM_LONGRANGE_LRBLEDSA.override = 1
        phy.profile_outputs.MODEM_LONGRANGE_LRCORRTHD.override = 1100
        phy.profile_outputs.MODEM_LONGRANGE_LRDEC.override = 3
        phy.profile_outputs.MODEM_LONGRANGE_LRTIMCORRTHD.override = 1000
        phy.profile_outputs.MODEM_MODINDEX_FREQGAINE.override = 2
        phy.profile_outputs.MODEM_MODINDEX_FREQGAINM.override = 2
        phy.profile_outputs.MODEM_TXBR_TXBRDEN.override = 5
        phy.profile_outputs.MODEM_TXBR_TXBRNUM.override = 24
        phy.profile_outputs.MODEM_VITERBIDEMOD_VTDEMODEN.override = 1
        phy.profile_outputs.MODEM_VITERBIDEMOD_DISDEMODOF.override = 1
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 110

        phy.profile_outputs.RAC_SYNTHCTRL_MMDPOWERBALANCEDISABLE.override = 1
        phy.profile_outputs.RAC_SYNTHREGCTRL_MMDLDOVREFTRIM.override = 3
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1BWCAL.override = 11
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1COMPCAL.override = 14
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RFBVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RPVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RZVALCAL.override = 9

    def Bluetooth_LongRange_500kbps_base(self, phy, model):

        self.Bluetooth_LongRange_base(phy, model)

        phy.profile_outputs.FRC_CTRL_RATESELECT.override = 2
        phy.profile_outputs.FRC_TRAILTXDATACTRL_TRAILTXDATA.override = 1
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH1.override = 10
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH2.override = 16
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH3.override = 22
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH4.override = 28
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH5.override = 34
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH6.override = 40
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH7.override = 46
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH8.override = 52
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH10.override = 64
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH9.override = 58
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRSH12.override = 10
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRTH11.override = 72

    ################################################################################################
    # "PHY" definitions - official PHY created here, simulation PHYs in Phys_sim_tests.py

    def PHY_Bluetooth_LE_Viterbi_BLEIQDSA(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Official BLE 1Mbps PHY for Panther', phy_name=phy_name)

        self.Bluetooth_LE_Viterbi_BLEIQDSA_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M

        phy.profile_outputs.rx_sync_delay_ns.override = 50000
        phy.profile_outputs.rx_eof_delay_ns.override = 9500
        phy.profile_outputs.tx_sync_delay_ns.override = 500
        phy.profile_outputs.tx_eof_delay_ns.override = 500

        return phy

    def PHY_Bluetooth_LE_2M_Viterbi_BLEIQDSA(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Official 2Mbps BLE IQ-DSA PHY for Panther', phy_name=phy_name)

        self.Bluetooth_LE_2M_Viterbi_BLEIQDSA_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_2M

        phy.profile_outputs.rx_sync_delay_ns.override = 50000
        phy.profile_outputs.rx_eof_delay_ns.override = 5500
        phy.profile_outputs.tx_sync_delay_ns.override = 1500
        phy.profile_outputs.tx_eof_delay_ns.override = 1500

    def PHY_Bluetooth_LE_Viterbi(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Bluetooth LE Viterbi phase-DSA', phy_name=phy_name)

        self.Bluetooth_LE_Viterbi_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M
        return phy

    def PHY_Bluetooth_LE_2M_Viterbi(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Official 2Mbps BLE phase-DSA PHY for Panther', phy_name=phy_name)

        self.Bluetooth_LE_2M_Viterbi_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_2M
        return phy

    def PHY_Bluetooth_LE(self, model,phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Official BLE 1Mbps Legacy PHY for Panther',phy_name=phy_name)

        self.Bluetooth_LE_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M

        return phy

    def PHY_Bluetooth_LongRange_dsa_125kbps(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Bluetooth LongRange DSA 125kbps', phy_name=phy_name)

        self.Bluetooth_LongRange_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CODED_125K

        phy.profile_outputs.rx_sync_delay_ns.override = 187125
        phy.profile_outputs.rx_eof_delay_ns.override = 9500
        phy.profile_outputs.tx_sync_delay_ns.override = 500
        phy.profile_outputs.tx_eof_delay_ns.override = 500

    def PHY_Bluetooth_LongRange_dsa_500kbps(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Bluetooth LongRange DSA 500kbps', phy_name=phy_name)

        self.Bluetooth_LongRange_500kbps_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CODED_500K

        phy.profile_outputs.rx_sync_delay_ns.override = 49125
        phy.profile_outputs.rx_eof_delay_ns.override = 9500
        phy.profile_outputs.tx_sync_delay_ns.override = 500
        phy.profile_outputs.tx_eof_delay_ns.override = 500

    def PHY_Bluetooth_LongRange_NOdsa_125kbps(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Bluetooth LongRange DSA 125kbps', phy_name=phy_name)

        self.Bluetooth_LongRange_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CODED_125K
        phy.profile_outputs.MODEM_LONGRANGE_LRBLEDSA.override = 0
        return phy

    def PHY_Bluetooth_LongRange_NOdsa_500kbps(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Bluetooth LongRange DSA 500kbps', phy_name=phy_name)

        self.Bluetooth_LongRange_500kbps_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CODED_500K
        phy.profile_outputs.MODEM_LONGRANGE_LRBLEDSA.override = 0
        return phy

    def PHY_Bluetooth_1M_AOX(self, model, phy_name=None):
        # PHY_Bluetooth_LE
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE 1Mbps AOX PHY for Panther', phy_name=phy_name)

        self.Bluetooth_LE_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.AOX_1M

    def PHY_Bluetooth_1M(self, model, phy_name=None):
        # PHY_Bluetooth_LE_Viterbi phase DSA

        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE 1Mbps PHY for Panther', phy_name=phy_name)

        self.Bluetooth_LE_Viterbi_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M

        return phy

    def PHY_Bluetooth_LR_125k(self, model, phy_name=None):
        # PHY_Bluetooth_LongRange_dsa_125kbps
        phy = self._makePhy(model, model.profiles.Base, readable_name='Production BLE LongRange 125kbps PHY for Panther', phy_name=phy_name)

        self.Bluetooth_LongRange_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CODED_125K


    def PHY_Bluetooth_LR_500k(self, model, phy_name=None):
        # PHY_Bluetooth_LongRange_dsa_500kbps
        phy = self._makePhy(model, model.profiles.Base, readable_name='Production BLE LongRange 500kbps PHY for Panther', phy_name=phy_name)

        self.Bluetooth_LongRange_500kbps_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CODED_500K

    @staticmethod
    def BLE_TX_Shaping_Coeffs(phy, model):
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3
        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override = 0
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override = 2
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override = 5
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override = 14
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override = 28
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override = 47
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override = 67
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override = 83
        phy.profile_outputs.MODEM_SHAPING2_COEFF8.override = 98
        phy.profile_outputs.MODEM_SHAPING2_COEFF9.override = 105
        phy.profile_outputs.MODEM_SHAPING2_COEFF10.override = 104
        phy.profile_outputs.MODEM_SHAPING2_COEFF11.override = 97
        phy.profile_outputs.MODEM_SHAPING3_COEFF12.override = 85
        phy.profile_outputs.MODEM_SHAPING3_COEFF13.override = 67
        phy.profile_outputs.MODEM_SHAPING3_COEFF14.override = 50
        phy.profile_outputs.MODEM_SHAPING3_COEFF15.override = 4
        phy.profile_outputs.MODEM_SHAPING4_COEFF16.override = 22
        phy.profile_outputs.MODEM_SHAPING4_COEFF17.override = 16
        phy.profile_outputs.MODEM_SHAPING4_COEFF18.override = 14
        phy.profile_outputs.MODEM_SHAPING4_COEFF19.override = 14
        phy.profile_outputs.MODEM_SHAPING4_COEFF20.override = 13
        phy.profile_outputs.MODEM_SHAPING4_COEFF21.override = 12
        phy.profile_outputs.MODEM_SHAPING5_COEFF22.override = 10
        phy.profile_outputs.MODEM_SHAPING5_COEFF23.override = 8
        phy.profile_outputs.MODEM_SHAPING5_COEFF24.override = 7
        phy.profile_outputs.MODEM_SHAPING5_COEFF25.override = 6
        phy.profile_outputs.MODEM_SHAPING5_COEFF26.override = 5
        phy.profile_outputs.MODEM_SHAPING5_COEFF27.override = 4
        phy.profile_outputs.MODEM_SHAPING5_COEFF28.override = 3
        phy.profile_outputs.MODEM_SHAPING5_COEFF29.override = 3
        phy.profile_outputs.MODEM_SHAPING6_COEFF30.override = 2
        phy.profile_outputs.MODEM_SHAPING6_COEFF31.override = 2
        phy.profile_outputs.MODEM_SHAPING6_COEFF32.override = 2
        phy.profile_outputs.MODEM_SHAPING6_COEFF33.override = 1
        phy.profile_outputs.MODEM_SHAPING6_COEFF34.override = 1
        phy.profile_outputs.MODEM_SHAPING6_COEFF35.override = 1
        phy.profile_outputs.MODEM_SHAPING6_COEFF36.override = 1
        phy.profile_outputs.MODEM_SHAPING6_COEFF37.override = 1
        phy.profile_outputs.MODEM_SHAPING6_COEFF38.override = 1
        phy.profile_outputs.MODEM_SHAPING6_COEFF39.override = 0

    @staticmethod
    def BLE_TX_Shaping_PantherZ_Compatible_Coeffs(phy, model):

        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3

        #New coeffs that are required to pass mod char with PantherZ
        #Also applied for Panther so that Panther/PantherZ PHYs are identical
        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override = 0
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override = 2
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override = 5
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override = 13
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override = 28
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override = 47
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override = 66
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override = 83
        phy.profile_outputs.MODEM_SHAPING2_COEFF8.override = 98
        phy.profile_outputs.MODEM_SHAPING2_COEFF9.override = 105
        phy.profile_outputs.MODEM_SHAPING2_COEFF10.override = 104
        phy.profile_outputs.MODEM_SHAPING2_COEFF11.override = 96
        phy.profile_outputs.MODEM_SHAPING3_COEFF12.override = 83
        phy.profile_outputs.MODEM_SHAPING3_COEFF13.override = 64
        phy.profile_outputs.MODEM_SHAPING3_COEFF14.override = 46
        phy.profile_outputs.MODEM_SHAPING3_COEFF15.override = 31
        phy.profile_outputs.MODEM_SHAPING4_COEFF16.override = 17
        phy.profile_outputs.MODEM_SHAPING4_COEFF17.override = 10
        phy.profile_outputs.MODEM_SHAPING4_COEFF18.override = 8
        phy.profile_outputs.MODEM_SHAPING4_COEFF19.override = 8
        phy.profile_outputs.MODEM_SHAPING4_COEFF20.override = 8
        phy.profile_outputs.MODEM_SHAPING4_COEFF21.override = 8
        phy.profile_outputs.MODEM_SHAPING5_COEFF22.override = 7
        phy.profile_outputs.MODEM_SHAPING5_COEFF23.override = 6
        phy.profile_outputs.MODEM_SHAPING5_COEFF24.override = 5
        phy.profile_outputs.MODEM_SHAPING5_COEFF25.override = 4
        phy.profile_outputs.MODEM_SHAPING5_COEFF26.override = 3
        phy.profile_outputs.MODEM_SHAPING5_COEFF27.override = 3
        phy.profile_outputs.MODEM_SHAPING5_COEFF28.override = 2
        phy.profile_outputs.MODEM_SHAPING5_COEFF29.override = 2
        phy.profile_outputs.MODEM_SHAPING6_COEFF30.override = 2
        phy.profile_outputs.MODEM_SHAPING6_COEFF31.override = 1
        phy.profile_outputs.MODEM_SHAPING6_COEFF32.override = 1
        phy.profile_outputs.MODEM_SHAPING6_COEFF33.override = 1
        phy.profile_outputs.MODEM_SHAPING6_COEFF34.override = 1
        phy.profile_outputs.MODEM_SHAPING6_COEFF35.override = 1
        phy.profile_outputs.MODEM_SHAPING6_COEFF36.override = 0
        phy.profile_outputs.MODEM_SHAPING6_COEFF37.override = 0
        phy.profile_outputs.MODEM_SHAPING6_COEFF38.override = 0
        phy.profile_outputs.MODEM_SHAPING6_COEFF39.override = 0

    @staticmethod
    def BLE_2M_TX_Shaping_Coeffs(phy, model):

        # from Bluetooth_LE_2M_Viterbi_BLEIQDSA_base
        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override =  1
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override =  2
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override =  7
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override =  17
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override =  33
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override =  55
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override =  75
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override =  90
        phy.profile_outputs.MODEM_SHAPING2_COEFF8.override =  103
        phy.profile_outputs.MODEM_SHAPING2_COEFF9.override =  105
        phy.profile_outputs.MODEM_SHAPING2_COEFF10.override = 98
        phy.profile_outputs.MODEM_SHAPING2_COEFF11.override = 84
        phy.profile_outputs.MODEM_SHAPING3_COEFF12.override = 64
        phy.profile_outputs.MODEM_SHAPING3_COEFF13.override = 41
        phy.profile_outputs.MODEM_SHAPING3_COEFF14.override = 21
        phy.profile_outputs.MODEM_SHAPING3_COEFF15.override = 5
        phy.profile_outputs.MODEM_SHAPING4_COEFF16.override = 0
        phy.profile_outputs.MODEM_SHAPING4_COEFF17.override = 0
        phy.profile_outputs.MODEM_SHAPING4_COEFF18.override = 0
        phy.profile_outputs.MODEM_SHAPING4_COEFF19.override = 0
        phy.profile_outputs.MODEM_SHAPING4_COEFF20.override = 1
        phy.profile_outputs.MODEM_SHAPING4_COEFF21.override = 2
        phy.profile_outputs.MODEM_SHAPING5_COEFF22.override = 3
        phy.profile_outputs.MODEM_SHAPING5_COEFF23.override = 4
        phy.profile_outputs.MODEM_SHAPING5_COEFF24.override = 4
        phy.profile_outputs.MODEM_SHAPING5_COEFF25.override = 4
        phy.profile_outputs.MODEM_SHAPING5_COEFF26.override = 3
        phy.profile_outputs.MODEM_SHAPING5_COEFF27.override = 3
        phy.profile_outputs.MODEM_SHAPING5_COEFF28.override = 3
        phy.profile_outputs.MODEM_SHAPING5_COEFF29.override = 3
        phy.profile_outputs.MODEM_SHAPING6_COEFF30.override = 3
        phy.profile_outputs.MODEM_SHAPING6_COEFF31.override = 2
        phy.profile_outputs.MODEM_SHAPING6_COEFF32.override = 2
        phy.profile_outputs.MODEM_SHAPING6_COEFF33.override = 2
        phy.profile_outputs.MODEM_SHAPING6_COEFF34.override = 2
        phy.profile_outputs.MODEM_SHAPING6_COEFF35.override = 2
        phy.profile_outputs.MODEM_SHAPING6_COEFF36.override = 2
        phy.profile_outputs.MODEM_SHAPING6_COEFF37.override = 1
        phy.profile_outputs.MODEM_SHAPING6_COEFF38.override = 1
        phy.profile_outputs.MODEM_SHAPING6_COEFF39.override = 1



