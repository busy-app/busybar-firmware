from pyradioconfig.parts.common.phys.phy_common import PHY_COMMON_FRAME_BLE, PHY_COMMON_FRAME_BLE_AOX
from pyradioconfig.parts.panther.phys.Phys_RAIL_Base_Standard_Bluetooth_LE import PhysRAILBaseStandardBluetoothLEPanther
from pyradioconfig.parts.lynx.phys.PHY_internal_base import Phy_Internal_Base_Lynx
from py_2_and_3_compatibility import *


class PhysRAILBaseStandardBluetoothLELynx(PhysRAILBaseStandardBluetoothLEPanther):
    # PHYs are inherited from Panther unless overridden

    def __init__(self):
        self.phy_internal_base = Phy_Internal_Base_Lynx()

    ### Constant shaping coefficients ###

    def BLE_TX_Shaping_Coeffs(self, phy, model):
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override =        3
        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override =      0
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override =      2
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override =      5
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override =      14
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override =      28
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override =      47
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override =      67
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override =      83
        phy.profile_outputs.MODEM_SHAPING2_COEFF8.override =      98
        phy.profile_outputs.MODEM_SHAPING2_COEFF9.override =      105
        phy.profile_outputs.MODEM_SHAPING2_COEFF10.override =     104
        phy.profile_outputs.MODEM_SHAPING2_COEFF11.override =     97
        phy.profile_outputs.MODEM_SHAPING3_COEFF12.override =     85
        phy.profile_outputs.MODEM_SHAPING3_COEFF13.override =     67
        phy.profile_outputs.MODEM_SHAPING3_COEFF14.override =     50
        phy.profile_outputs.MODEM_SHAPING3_COEFF15.override =     4
        phy.profile_outputs.MODEM_SHAPING4_COEFF16.override =     22
        phy.profile_outputs.MODEM_SHAPING4_COEFF17.override =     16
        phy.profile_outputs.MODEM_SHAPING4_COEFF18.override =     14
        phy.profile_outputs.MODEM_SHAPING4_COEFF19.override =     14
        phy.profile_outputs.MODEM_SHAPING5_COEFF20.override =     13
        phy.profile_outputs.MODEM_SHAPING5_COEFF21.override =     12
        phy.profile_outputs.MODEM_SHAPING5_COEFF22.override =     10
        phy.profile_outputs.MODEM_SHAPING5_COEFF23.override =     8
        phy.profile_outputs.MODEM_SHAPING6_COEFF24.override =     7
        phy.profile_outputs.MODEM_SHAPING6_COEFF25.override =     6
        phy.profile_outputs.MODEM_SHAPING6_COEFF26.override =     5
        phy.profile_outputs.MODEM_SHAPING6_COEFF27.override =     4
        phy.profile_outputs.MODEM_SHAPING7_COEFF28.override =     3
        phy.profile_outputs.MODEM_SHAPING7_COEFF29.override =     3
        phy.profile_outputs.MODEM_SHAPING7_COEFF30.override =     2
        phy.profile_outputs.MODEM_SHAPING7_COEFF31.override =     2
        phy.profile_outputs.MODEM_SHAPING8_COEFF32.override =     2
        phy.profile_outputs.MODEM_SHAPING8_COEFF33.override =     1
        phy.profile_outputs.MODEM_SHAPING8_COEFF34.override =     1
        phy.profile_outputs.MODEM_SHAPING8_COEFF35.override =     1
        phy.profile_outputs.MODEM_SHAPING9_COEFF36.override =     1
        phy.profile_outputs.MODEM_SHAPING9_COEFF37.override =     1
        phy.profile_outputs.MODEM_SHAPING9_COEFF38.override =     1
        phy.profile_outputs.MODEM_SHAPING9_COEFF39.override =     0
        phy.profile_outputs.MODEM_SHAPING10_COEFF40.override =    0
        phy.profile_outputs.MODEM_SHAPING10_COEFF41.override =    0
        phy.profile_outputs.MODEM_SHAPING10_COEFF42.override =    0
        phy.profile_outputs.MODEM_SHAPING10_COEFF43.override =    0
        phy.profile_outputs.MODEM_SHAPING11_COEFF44.override =    0
        phy.profile_outputs.MODEM_SHAPING11_COEFF45.override =    0
        phy.profile_outputs.MODEM_SHAPING11_COEFF46.override =    0
        phy.profile_outputs.MODEM_SHAPING11_COEFF47.override =    0

    def BLE_TX_Shaping_PantherZ_Compatible_Coeffs(self, phy, model):
        #For Lynx, just use the same BLE 1M coeffs
        self.BLE_TX_Shaping_Coeffs(phy, model)

    def BLE_2M_TX_Shaping_Coeffs(self, phy, model):
        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override      = 0
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override      = 2
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override      = 5
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override      = 13
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override      = 26
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override      = 45
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override      = 63
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override      = 80
        phy.profile_outputs.MODEM_SHAPING2_COEFF8.override      = 96
        phy.profile_outputs.MODEM_SHAPING2_COEFF9.override      = 104
        phy.profile_outputs.MODEM_SHAPING2_COEFF10.override     = 105
        phy.profile_outputs.MODEM_SHAPING2_COEFF11.override     = 99
        phy.profile_outputs.MODEM_SHAPING3_COEFF12.override     = 87
        phy.profile_outputs.MODEM_SHAPING3_COEFF13.override     = 70
        phy.profile_outputs.MODEM_SHAPING3_COEFF14.override     = 53
        phy.profile_outputs.MODEM_SHAPING3_COEFF15.override     = 37
        phy.profile_outputs.MODEM_SHAPING4_COEFF16.override     = 21
        phy.profile_outputs.MODEM_SHAPING4_COEFF17.override     = 13
        phy.profile_outputs.MODEM_SHAPING4_COEFF18.override     = 9
        phy.profile_outputs.MODEM_SHAPING4_COEFF19.override     = 8
        phy.profile_outputs.MODEM_SHAPING5_COEFF20.override     = 7
        phy.profile_outputs.MODEM_SHAPING5_COEFF21.override     = 6
        phy.profile_outputs.MODEM_SHAPING5_COEFF22.override     = 5
        phy.profile_outputs.MODEM_SHAPING5_COEFF23.override     = 5
        phy.profile_outputs.MODEM_SHAPING6_COEFF24.override     = 4
        phy.profile_outputs.MODEM_SHAPING6_COEFF25.override     = 4
        phy.profile_outputs.MODEM_SHAPING6_COEFF26.override     = 4
        phy.profile_outputs.MODEM_SHAPING6_COEFF27.override     = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF28.override     = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF29.override     = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF30.override     = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF31.override     = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF32.override     = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF33.override     = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF34.override     = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF35.override     = 2
        phy.profile_outputs.MODEM_SHAPING9_COEFF36.override     = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF37.override     = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF38.override     = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF39.override     = 1
        phy.profile_outputs.MODEM_SHAPING10_COEFF40.override    = 0
        phy.profile_outputs.MODEM_SHAPING10_COEFF41.override    = 0
        phy.profile_outputs.MODEM_SHAPING10_COEFF42.override    = 0
        phy.profile_outputs.MODEM_SHAPING10_COEFF43.override    = 0
        phy.profile_outputs.MODEM_SHAPING11_COEFF44.override    = 0
        phy.profile_outputs.MODEM_SHAPING11_COEFF45.override    = 0
        phy.profile_outputs.MODEM_SHAPING11_COEFF46.override    = 0
        phy.profile_outputs.MODEM_SHAPING11_COEFF47.override    = 0

    ### Production PHYs (redirect to other PHYs) ###

    ### Base function containing common overrides for BLE Viterbi noDSA PHYs ###

    def Bluetooth_LE_Viterbi_noDSA_base(self,phy,model):

        #No AGC overrides needed as this PHY does not inherit (Lynx defaults to fast loop settings)

        phy.profile_inputs.bandwidth_hz.value = 1099233
        phy.profile_inputs.base_frequency_hz.value = long(2402000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 1000
        phy.profile_inputs.bitrate.value = 1000000
        phy.profile_inputs.channel_spacing_hz.value = 1000000
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.dsa_enable.value = False
        phy.profile_inputs.dsss_chipping_code.value = long(0)
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0
        phy.profile_inputs.errors_in_timing_window.value = 1
        phy.profile_inputs.freq_offset_hz.value = 0
        phy.profile_inputs.frequency_comp_mode.value = model.vars.frequency_comp_mode.var_enum.INTERNAL_ALWAYS_ON
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
        phy.profile_inputs.target_osr.value = 4
        phy.profile_inputs.timing_detection_threshold.value = 140
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.xtal_frequency_hz.value = 38400000

        # Decimation and Demod Overrides
        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 56831
        phy.profile_outputs.MODEM_CTRL0_FRAMEDETDEL.override = 2
        phy.profile_outputs.MODEM_CTRL2_DATAFILTER.override = 6
        phy.profile_outputs.MODEM_CTRL3_TSAMPDEL.override = 2
        phy.profile_outputs.MODEM_CTRL6_RXBRCALCDIS.override = 1
        phy.profile_outputs.MODEM_DIGMIXCTRL_DIGMIXFB.override = 1
        phy.profile_outputs.MODEM_PRE_BASE.override = 2
        phy.profile_outputs.MODEM_TIMING_OFFSUBNUM.override = 0

        # Shaping overrides
        self.BLE_TX_Shaping_Coeffs(phy, model)

        # FRC Overrides
        PHY_COMMON_FRAME_BLE_AOX(phy, model)
        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        phy.profile_outputs.FRC_PUNCTCTRL_PUNCT0.override = 1
        phy.profile_outputs.FRC_PUNCTCTRL_PUNCT1.override = 1

        # This should be set only on DSA PHYs (PGLYNX-566), sowe can clear it.
        phy.profile_outputs.FRC_CTRL_WAITEOFEN.override = 0

        # Radio Controller Overrides
        phy.profile_outputs.RAC_SYNTHREGCTRL_MMDLDOVREFTRIM.override = 3
        phy.profile_outputs.RAC_SYNTHCTRL_MMDPOWERBALANCEDISABLE.override = 1

        # Viterbi Demod Overrides
        phy.profile_outputs.MODEM_VITERBIDEMOD_SYNTHAFC.override = 1
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI1.override = 65
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI2.override = 45
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 24
        phy.profile_outputs.MODEM_VITERBIDEMOD_VTDEMODEN.override = 1
        phy.profile_outputs.MODEM_VTBLETIMING_TIMINGDELAY.override = 60
        phy.profile_outputs.MODEM_VTBLETIMING_VTBLETIMINGSEL.override = 1
        phy.profile_outputs.MODEM_VTCORRCFG1_CORRSHFTLEN.override = 42
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPECTHT.override = 5
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPSYNCLEN.override = 146
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 100
        phy.profile_outputs.MODEM_VTTRACK_FREQTRACKMODE.override = 1
        phy.profile_outputs.MODEM_VTTRACK_TIMEOUTMODE.override = 0

        phy.profile_outputs.MODEM_TRECPMPATT_PMEXPECTPATT.override = 1431655765
        phy.profile_outputs.MODEM_VTCORRCFG0_EXPECTPATT.override = 1245787975
        phy.profile_outputs.MODEM_VITERBIDEMOD_CORRSTPSIZE.override = 0
        phy.profile_outputs.MODEM_TRECPMDET_PMTIMEOUTSEL.override = 0

        # DSA Overrides
        phy.profile_outputs.MODEM_DSACTRL_DSAMODE.override = 0

        # Other Overrides
        phy.profile_outputs.MODEM_REALTIMCFE_MINCOSTTHD.override = 600
        phy.profile_outputs.MODEM_REALTIMCFE_RTCFEEN.override = 1
        phy.profile_outputs.MODEM_REALTIMCFE_RTSCHWIN.override = 10
        phy.profile_outputs.MODEM_REALTIMCFE_TRACKINGWIN.override = 5
        phy.profile_outputs.MODEM_REALTIMCFE_VTAFCFRAME.override = 1

    ###BLE Viterbi NO DSA PHYs, do not inherit###
    def Bluetooth_LE_Viterbi_noDSA_halfrate_base(self,phy,model):
        self.Bluetooth_LE_Viterbi_noDSA_base(phy, model)
        phy.profile_inputs.if_frequency_hz.value = 700000

        phy.profile_outputs.MODEM_CF_DEC1.override = 0
        phy.profile_outputs.RAC_IFADCTRIM0_IFADCENHALFMODE.override = 1
        phy.profile_outputs.RAC_IFADCTRIM0_IFADCSIDETONEAMP.override = 3
        phy.profile_outputs.RAC_SYTRIM1_SYLODIVRLOADCCLKSEL.override = 1

    def Bluetooth_LE_2M_Viterbi_noDSA_base(self, phy, model):
        self.Bluetooth_LE_Viterbi_noDSA_base(phy, model)

        phy.profile_inputs.bitrate.value = 2000000
        phy.profile_inputs.bandwidth_hz.value = 2200000
        phy.profile_inputs.deviation.value = 500000
        phy.profile_inputs.preamble_length.value = 16
        phy.profile_inputs.src1_range_available_minimum.value = 133

    def PHY_Bluetooth_LE_Viterbi_noDSA_fullrate(self, model,phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi No DSA Fullrate PHY for Lynx',phy_name=phy_name)
        self.Bluetooth_LE_Viterbi_noDSA_base(phy,model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M

        phy.profile_outputs.AGC_CTRL2_DISRFPKD.override = 1
        phy.profile_outputs.AGC_CTRL4_RFPKDCNTEN.override = 0
        return phy

    def PHY_Bluetooth_1M_Concurrent(self, model, phy_name=None):

        #Start with the BLE LR 125k PHY
        phy = self.PHY_Bluetooth_LongRange_dsa_125kbps(model, phy_name='PHY_Bluetooth_1M_Concurrent')
        phy.profile_inputs.preamble_pattern.value = 0b10
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CONCURRENT

        #The concurrent PHY needs to have:
        #Enable concurrent mode
        #FRC = BLE LR 125k, with added sub-frame description for BLE 1M uncoded
        #Demod = BLE LR 125k
        #TRECS/Viterbi = BLE 1M
        #BLE LR = BLE LR 125k
        #DSA = BLE LR 125k

        #Enable concurrent mode
        phy.profile_outputs.MODEM_COCURRMODE_CONCURRENT.override = 1

        # Overrides for TRECS/Viterbi (copied from BLE 1M)
        phy.profile_outputs.MODEM_VITERBIDEMOD_SYNTHAFC.override = 1
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI1.override = 65
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI2.override = 45
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 24
        phy.profile_outputs.MODEM_VITERBIDEMOD_VTDEMODEN.override = 1
        phy.profile_outputs.MODEM_VTBLETIMING_TIMINGDELAY.override = 60
        phy.profile_outputs.MODEM_VTBLETIMING_VTBLETIMINGSEL.override = 1
        phy.profile_outputs.MODEM_VTCORRCFG1_CORRSHFTLEN.override = 42
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPECTHT.override = 5
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPSYNCLEN.override = 146
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 100
        phy.profile_outputs.MODEM_VTTRACK_FREQTRACKMODE.override = 1
        phy.profile_outputs.MODEM_VTTRACK_TIMEOUTMODE.override = 0
        phy.profile_outputs.MODEM_REALTIMCFE_MINCOSTTHD.override = 600
        phy.profile_outputs.MODEM_REALTIMCFE_RTCFEEN.override = 1
        phy.profile_outputs.MODEM_REALTIMCFE_RTSCHWIN.override = 10
        phy.profile_outputs.MODEM_REALTIMCFE_TRACKINGWIN.override = 5
        phy.profile_outputs.MODEM_REALTIMCFE_VTAFCFRAME.override = 1

        # Overrides for TX mode (copied from BLE 1M Fullrate)
        phy.profile_inputs.base_frequency_hz.value = long(2402000000)
        phy.profile_inputs.channel_spacing_hz.value = 1000000
        self.BLE_TX_Shaping_Coeffs(phy, model)
        phy.profile_outputs.FRC_FCD0_ADDTRAILTXDATA.override = 0
        phy.profile_outputs.FRC_FCD0_CALCCRC.override = 1
        phy.profile_outputs.FRC_FCD0_EXCLUDESUBFRAMEWCNT.override = 0
        phy.profile_outputs.FRC_FCD0_INCLUDECRC.override = 1
        phy.profile_outputs.FRC_FCD0_WORDS.override = 2 ** phy.profile_outputs.FRC_FCD0_WORDS.var.get_bit_width() - 1
        phy.profile_outputs.FRC_TRAILTXDATACTRL_TRAILTXDATACNT.override = 0
        phy.profile_outputs.MODEM_CTRL1_SYNCDATA.override = 0
        phy.profile_outputs.MODEM_CTRL2_RATESELMODE.override = 0
        phy.profile_outputs.MODEM_PRE_TXBASES.override = 4
        phy.profile_outputs.FRC_FECCTRL_CONVMODE.override = 0
        phy.profile_outputs.MODEM_CTRL0_CODING.override = 0

        # : Workaround for frequency error estimation bug
        phy.profile_outputs.MODEM_DSACTRL_SCHPRD.override = 1

        # : Disable oneshot AFC for Concurrent PHY
        phy.profile_outputs.MODEM_AFC_AFCONESHOT.override = 0
        phy.profile_outputs.MODEM_CTRL0_FRAMEDETDEL.override = 2
        phy.profile_outputs.MODEM_PRE_BASE.override = 2

        # : Copy from Bluetooth 1M Fullrate
        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 56831 # : copy clock gating settings from BLE 1M
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 32  # Needed because BW is wider than non-concurrent 1M

        phy.profile_outputs.MODEM_AFC_AFCSCALEE.override = 0
        phy.profile_outputs.MODEM_AFC_AFCSCALEM.override = 0

        # : needed as part of IPMCUSRW-777 workaround
        # : https://jira.silabs.com/browse/IPMCUSRW-771
        phy.profile_outputs.MODEM_LONGRANGE1_LOGICBASEDLRDEMODGATE.override = 0

        # : Resolve floor issues by disabling DSA threshold
        phy.profile_outputs.MODEM_LONGRANGE6_LRSPIKETHD.override = 0

        phy.profile_outputs.MODEM_CTRL2_DATAFILTER.override = 2
        phy.profile_outputs.MODEM_CTRL2_DEVWEIGHTDIS.override = 0
        phy.profile_outputs.MODEM_TRECPMDET_PMTIMEOUTSEL.override = 0
        phy.profile_outputs.MODEM_VITERBIDEMOD_CORRSTPSIZE.override = 0

        phy.profile_outputs.rx_sync_delay_ns.override = 50000
        phy.profile_outputs.rx_eof_delay_ns.override = 11750
        phy.profile_outputs.tx_sync_delay_ns.override = 2000
        phy.profile_outputs.tx_eof_delay_ns.override = 2000
        return phy

    def PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi 2M No DSA Fullrate PHY for Lynx',phy_name=phy_name)
        self.Bluetooth_LE_2M_Viterbi_noDSA_base(phy,model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_2M

        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_3000KHz
        self.BLE_2M_TX_Shaping_Coeffs(phy, model)

        phy.profile_outputs.AGC_CTRL2_DISRFPKD.override = 1
        phy.profile_outputs.AGC_CTRL4_RFPKDCNTEN.override = 0
        return phy

    def PHY_Bluetooth_LE_Viterbi_noDSA(self, model,phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi No DSA PHY for Lynx',phy_name=phy_name)
        self.Bluetooth_LE_Viterbi_noDSA_halfrate_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M

        phy.profile_outputs.AGC_CTRL2_DISRFPKD.override = 1
        phy.profile_outputs.AGC_CTRL4_RFPKDCNTEN.override = 0
        return phy

    ###BLE LONG RANGE DSA PHYs, inherited from Panther###

    def PHY_Bluetooth_LongRange_dsa_125kbps(self,model,phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Bluetooth LongRange DSA 125kbps PHY for Lynx',phy_name=phy_name)

        self.Bluetooth_LongRange_base(phy, model)
        self.phy_internal_base.AGC_MODIFIED_PANTHER_STYLE(phy, model)

        #Use BLE 1M shaping coeffs
        self.BLE_TX_Shaping_Coeffs(phy,model)

        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CODED_125K

        phy.profile_outputs.AGC_CTRL2_DISRFPKD.override = 1
        phy.profile_outputs.AGC_CTRL4_RFPKDCNTEN.override = 0

        phy.profile_outputs.MODEM_VITERBIDEMOD_SYNTHAFC.override = 0

        phy.profile_outputs.rx_sync_delay_ns.override = 187125
        phy.profile_outputs.rx_eof_delay_ns.override = 10000
        phy.profile_outputs.tx_sync_delay_ns.override = 300
        phy.profile_outputs.tx_eof_delay_ns.override = 300
        return phy

    def PHY_Bluetooth_LongRange_dsa_500kbps(self,model,phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Bluetooth LongRange DSA 500kbps PHY for Lynx',phy_name=phy_name)

        self.Bluetooth_LongRange_500kbps_base(phy, model)
        self.phy_internal_base.AGC_MODIFIED_PANTHER_STYLE(phy, model)

        # Use BLE 1M shaping coeffs
        self.BLE_TX_Shaping_Coeffs(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CODED_500K

        phy.profile_outputs.AGC_CTRL2_DISRFPKD.override = 1
        phy.profile_outputs.AGC_CTRL4_RFPKDCNTEN.override = 0

        phy.profile_outputs.MODEM_VITERBIDEMOD_SYNTHAFC.override = 0

        phy.profile_outputs.rx_sync_delay_ns.override = 49125
        phy.profile_outputs.rx_eof_delay_ns.override = 10000
        phy.profile_outputs.tx_sync_delay_ns.override = 450
        phy.profile_outputs.tx_eof_delay_ns.override = 450
        return phy

    ###Removing PHYs that are inherited but no longer directly used###

    def PHY_Bluetooth_LE(self, model, phy_name=None):
        pass

    def PHY_Bluetooth_LE_Viterbi(self, model):
        pass

    def PHY_Bluetooth_LE_2M_Viterbi(self,model):
        pass

    def PHY_Bluetooth_LE_Viterbi_BLEIQDSA(self,model):
        pass

    def PHY_Bluetooth_LE_2M_Viterbi_BLEIQDSA(self, model):
        pass

    def PHY_Bluetooth_LongRange_NOdsa_125kbps(self,model):
        pass

    def PHY_Bluetooth_LongRange_NOdsa_500kbps(self,model):
        pass
