from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy
from pyradioconfig.parts.common.phys.phy_common import PHY_COMMON_FRAME_154
from pyradioconfig.parts.ocelot.phys.Phys_Studio_Base import PHYS_Studio_Base_Ocelot
from py_2_and_3_compatibility import *

class PHYS_Studio_Base_Sigfox_Ocelot(IPhy):

    # Owner: Olivier Souloumiac
    # JIRA Link: https://jira.silabs.com/browse/MCUW_RADIO_CFG-2413
    def PHY_Sigfox_915MHz_2GFSK_600bps_RXOnly(self, model,phy_name=None):
        '''
        Sigfox downlink (Rx):
        The sigfox Radio specification requires to meet -126dBm 10% PER spec, current measurement is around -133dBm.
        Note an accurate and stable frequency (HFXO) is required (TCXO is strongly recommended).

        The Rx configuration has a special feature.
        As the base station measures the frequency on the uplink
        packet and adjusts the downlink frequency to it, there is no need to use any
        AFC algorithm in the receiver and the narrowest possible filter bandwidth can
        be configured.
        '''
        phy = PHYS_Studio_Base_Ocelot().PHY_Studio_868M_2GFSK_600bps_800(model,
                                                                         phy_name='PHY_Sigfox_915MHz_2GFSK_600bps_RXOnly',
                                                                         readable_name="Sigfox RX 915MHz 2GFSK 600bps")

        phy.profile_inputs.rx_xtal_error_ppm.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.xtal_frequency_hz.value = 39000000
        # The Sigfox Downlink GFSK 600bps is specified with center frequency close to 915MHz for RC2, RC3, RC4, RC5
        # see Sigfox Radio specification EP-SPECS Rev.1.7
        phy.profile_inputs.base_frequency_hz.value = long(905200000)

        # BT=1
        # Note the BT is not provided in Sigfox Radio specification EP-SPECS Rev.1.7 (keep what is coming from series 1)
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.74

        # The Sigfox Downlink GFSK 600bps is specified with 91-bit (the last bits is 1 and added in the synchro word)
        # 5 bytes of 0x55 preamble
        # see Sigfox Radio specification EP-SPECS Rev.1.7
        phy.profile_inputs.preamble_pattern.value = 1
        phy.profile_inputs.preamble_pattern_len.value = 2
        phy.profile_inputs.preamble_length.value = 90

        # 13 bit sync word of b1001 0001 0011 1 (with 1 bit from preamble this will provide 14bits b1100 1000 1001 11)
        # see Sigfox Radio specification EP-SPECS Rev.1.7
        phy.profile_inputs.syncword_0.value = long(0x3227)
        phy.profile_inputs.syncword_1.value = long(0x0)
        phy.profile_inputs.syncword_length.value = 14

        # payload byte wise LSB first
        phy.profile_inputs.frame_bitendian.value = model.vars.frame_bitendian.var_enum.LSB_FIRST

        # Packet Inputs
        # 15 bytes payload (see Sigfox Radio specification EP-SPECS Rev.1.7)
        phy.profile_inputs.frame_length_type.value = model.vars.frame_length_type.var_enum.FIXED_LENGTH
        phy.profile_inputs.fixed_length_size.value = 15
        phy.profile_inputs.payload_white_en.value = False
        phy.profile_inputs.payload_crc_en.value = True

        # Variable length includes header
        phy.profile_inputs.header_en.value = False
        phy.profile_inputs.header_size.value = 1
        phy.profile_inputs.header_calc_crc.value = False
        phy.profile_inputs.header_white_en.value = False

        # CRC Inputs
        # It is not as in the Sigfox Radio specification EP-SPECS Rev.1.7: but needed for PER
        phy.profile_inputs.crc_poly.value = model.vars.crc_poly.var_enum.CRC_16  # CRC polynomial:   CRC_16
        phy.profile_inputs.crc_seed.value = long(0x00000000)  # CRC seed:         0x0000
        phy.profile_inputs.crc_byte_endian.value = model.vars.crc_byte_endian.var_enum.MSB_FIRST  # CRC byte order:   MSB first
        phy.profile_inputs.crc_bit_endian.value = model.vars.crc_bit_endian.var_enum.LSB_FIRST  # CRC bit order:    LSB first
        phy.profile_inputs.crc_input_order.value = model.vars.crc_input_order.var_enum.LSB_FIRST  # CRC engine feed:  LSB first
        phy.profile_inputs.crc_pad_input.value = False
        phy.profile_inputs.crc_invert.value = False

        # Misc frame configuration
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0

        return phy

    # Owner: Olivier Souloumiac
    # JIRA Link: https://jira.silabs.com/browse/MCUW_RADIO_CFG-2413
    def PHY_Sigfox_868MHz_2GFSK_600bps_RXOnly(self, model,phy_name=None):
        '''
        Sigfox downlink (Rx):
        The sigfox Radio specification requires to meet -126dBm 10% PER spec, current measurement is around -133dBm.
        Note an accurate and stable frequency (HFXO) is required (TCXO is strongly recommended).

        The Rx configuration has a special feature.
        As the base station measures the frequency on the uplink
        packet and adjusts the downlink frequency to it, there is no need to use any
        AFC algorithm in the receiver and the narrowest possible filter bandwidth can
        be configured.
        '''
        phy = PHYS_Studio_Base_Ocelot().PHY_Studio_868M_2GFSK_600bps_800(model,
                                                                         phy_name='PHY_Sigfox_868MHz_2GFSK_600bps_RXOnly',
                                                                         readable_name="Sigfox RX 868MHz 2GFSK 600bps")

        phy.profile_inputs.rx_xtal_error_ppm.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.xtal_frequency_hz.value = 39000000
        # The Sigfox Downlink 2GFSK 600bps is specified with center frequency of 869.525 for RC1 and close for RC6 and RC7
        # see Sigfox Radio specification EP-SPECS Rev.1.7
        phy.profile_inputs.base_frequency_hz.value = long(869525000)

        # BT=1
        # Note the BT is not provided in Sigfox Radio specification EP-SPECS Rev.1.7 (keep what is coming from series 1)
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.74

        # The Sigfox Downlink GFSK 600bps is specified with 91-bit (the last bits is 1 and added in the synchro word)
        # 5 bytes of 0x55 preamble
        # see Sigfox Radio specification EP-SPECS Rev.1.7
        phy.profile_inputs.preamble_pattern.value = 1
        phy.profile_inputs.preamble_pattern_len.value = 2
        phy.profile_inputs.preamble_length.value = 90

        # 13 bit sync word of b1001 0001 0011 1 (with 1 bit from preamble this will provide 14bits b1100 1000 1001 11)
        # see Sigfox Radio specification EP-SPECS Rev.1.7
        phy.profile_inputs.syncword_0.value = long(0x3227)
        phy.profile_inputs.syncword_1.value = long(0x0)
        phy.profile_inputs.syncword_length.value = 14

        # payload byte wise LSB first
        phy.profile_inputs.frame_bitendian.value = model.vars.frame_bitendian.var_enum.LSB_FIRST

        # Packet Inputs
        # 15 bytes payload (see Sigfox Radio specification EP-SPECS Rev.1.7)
        phy.profile_inputs.frame_length_type.value = model.vars.frame_length_type.var_enum.FIXED_LENGTH
        phy.profile_inputs.fixed_length_size.value = 15
        phy.profile_inputs.payload_white_en.value = False
        phy.profile_inputs.payload_crc_en.value = True

        # Variable length includes header
        phy.profile_inputs.header_en.value = False
        phy.profile_inputs.header_size.value = 1
        phy.profile_inputs.header_calc_crc.value = False
        phy.profile_inputs.header_white_en.value = False

        # CRC Inputs
        # It is not as in the Sigfox Radio specification EP-SPECS Rev.1.7: but needed for PER
        phy.profile_inputs.crc_poly.value = model.vars.crc_poly.var_enum.CRC_16  # CRC polynomial:   CRC_16
        phy.profile_inputs.crc_seed.value = long(0x00000000)  # CRC seed:         0x0000
        phy.profile_inputs.crc_byte_endian.value = model.vars.crc_byte_endian.var_enum.MSB_FIRST  # CRC byte order:   MSB first
        phy.profile_inputs.crc_bit_endian.value = model.vars.crc_bit_endian.var_enum.LSB_FIRST  # CRC bit order:    LSB first
        phy.profile_inputs.crc_input_order.value = model.vars.crc_input_order.var_enum.LSB_FIRST  # CRC engine feed:  LSB first
        phy.profile_inputs.crc_pad_input.value = False
        phy.profile_inputs.crc_invert.value = False

        # Misc frame configuration
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0

        return phy
