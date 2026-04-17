from pyradioconfig.parts.ocelot.calculators.calc_wisun import CALC_WiSUN_Ocelot
from pycalcmodel.core.variable import ModelVariableFormat, CreateModelVariableEnum
from enum import Enum
from pyradioconfig.calculator_model_framework.CalcManager import CalcManager
from pycalcmodel.core.output import ModelOutputType
import re

class CALC_WiSUN_Sol(CALC_WiSUN_Ocelot):

    def buildVariables(self, model):
        super().buildVariables(model)

        # Re-enumerating to include OFDM
        model.vars.wisun_phy_mode_id_select.var_enum = CreateModelVariableEnum(
            enum_name='WiSUNPhyModeIdEnum',
            enum_desc='List of supported PHY Mode IDs defined in WiSUN FAN 1v1',
            member_data=[
                # OFDM modes (MCS0-6 works for all OPTs)
                ['PhyID_34_38_OFDM_option1_BW_1200k', 0x22, 'PhyID_34_38_OFDM_option1_BW_1200k'],

                ['PhyID_51_54_OFDM_option2_BW_800k', 0x33, 'PhyID_51_54_OFDM_option2_BW_800k'],

                ['PhyID_68_70_OFDM_option3_BW_400k', 0x44, 'PhyID_68_70_OFDM_option3_BW_400k'],

                ['PhyID_84_86_OFDM_option4_BW_200k', 0x54, 'PhyID_84_86_OFDM_option4_BW_200k'],

                # FSK PhyID_s
                ['PhyID_1_FSK_50kbps_mi0p5', 0x01, 'PhyID_1_FSK_50kbps_mi0p5'],
                ['PhyID_2_FSK_50kbps_mi1', 0x02, 'PhyID_2_FSK_50kbps_mi1'],
                ['PhyID_3_FSK_100kbps_mi0p5', 0x03, 'PhyID_3_FSK_100kbps_mi0p5'],
                ['PhyID_4_FSK_100kbps_mi1', 0x04, 'PhyID_4_FSK_100kbps_mi1'],
                ['PhyID_5_FSK_150kbps_mi0p5', 0x05, 'PhyID_5_FSK3_150kbps_mi0p5'],
                ['PhyID_6_FSK_200kbps_mi0p5', 0x06, 'PhyID_6_FSK_200kbps_mi0p5'],
                ['PhyID_7_FSK_200kbps_mi1', 0x07, 'PhyID_7_FSK_200kbps_mi1'],
                ['PhyID_8_FSK_300kbps_mi0p5', 0x08, 'PhyID_8_FSK_300kbps_mi0p5'],
                ['PhyID_17_FSK_50kbps_mi0p5_FEC', 0x11, 'PhyID_17_FSK_50kbps_mi0p5_FEC'],
                ['PhyID_18_FSK_50kbps_mi1_FEC', 0x12, 'PhyID_18_FSK_50kbps_mi1_FEC'],
                ['PhyID_19_FSK_100kbps_mi0p5_FEC', 0x13, 'PhyID_19_FSK_100kbps_mi0p5_FEC'],
                ['PhyID_20_FSK_100kbps_mi1_FEC', 0x14, 'PhyID_20_FSK_100kbps_mi1_FEC'],
                ['PhyID_21_FSK_150kbps_mi0p5_FEC', 0x15, 'PhyID_21_FSK_150kbps_mi0p5_FEC'],
                ['PhyID_22_FSK_200kbps_mi0p5_FEC', 0x16, 'PhyID_22_FSK_200kbps_mi0p5_FEC'],
                ['PhyID_23_FSK_200kbps_mi1_FEC', 0x17, 'PhyID_23_FSK_200kbps_mi1_FEC'],
                ['PhyID_24_FSK_300kbps_mi0p5_FEC', 0x18, 'PhyID_24_FSK_300kbps_mi0p5_FEC'],
            ])

        var = self._addModelVariable(model, 'conc_ofdm_option', Enum, ModelVariableFormat.DECIMAL,
                                     'Concurrent OFDM Option')
        member_data = [
            ['NONE', 0, 'No Concurrent OFDM'],
            ['OPT1', 1, 'Wi-SUN OFDM OPT1'],
            ['OPT2', 2, 'Wi-SUN OFDM OPT2'],
            ['OPT3', 3, 'Wi-SUN OFDM OPT3'],
            ['OPT4', 4, 'Wi-SUN OFDM OPT4'],
        ]
        var.var_enum = CreateModelVariableEnum(
            'ConcOfdmOptEnum',
            'List of concurrent OFDM Options',
            member_data)

        self._addModelVariable(model, 'alt_wisun_mode_switch_phr', int, ModelVariableFormat.ASCII,
                               desc='WiSUN Mode Switch PHR for alternate PHY', is_array=True)
        self._addModelVariable(model, 'alt_wisun_phy_mode_id', int, ModelVariableFormat.ASCII,
                               desc='WiSUN PhyModeID for alternate PHY', is_array=True)
        self._addModelVariable(model, 'ofdm_stf_length', int, ModelVariableFormat.DECIMAL,
                               desc='OFDM STF length (1 unit = 120us)')
        self._addModelVariable(model, 'alt_rssi_adjust_db', float, ModelVariableFormat.DECIMAL,
                               desc='RSSI compensation value for alternate PHY')
        self._addModelVariable(model, 'alt_agc_ctrl1_pwrperiod', int, ModelVariableFormat.DECIMAL,
                               desc='PWRPERIOD value for alternate PHY')
        self._addModelVariable(model, 'alt_agc_ctrl1_rssiperiod', int, ModelVariableFormat.DECIMAL,
                               desc='RSSIPERIOD value for alternate PHY')

    def calc_wisun_phy_mode_id(self, model):
        # This function calculates the PhyModeID for Wi-SUN OFDM and Wi-SUN FSK PHYs
        # For OFDM, the variable contains an array of MCS0-7 values
        # For FSK, the variable contains an array of FSK off/on values

        profile_name = model.profile.name.lower()
        modulation_type = model.vars.modulation_type.value
        softmodem_modulation_type = model.vars.softmodem_modulation_type.value
        sun_oqpsk_chiprate = model.vars.sun_oqpsk_chiprate.value
        sun_oqpsk_spreading_mode = model.vars.sun_oqpsk_spreading_mode.value
        header_size = model.vars.header_size.value
        fec_enabled = model.vars.fec_enabled.value

        if profile_name in ["wisun_fan_1_0","wisun_han"]:
            #Call method from Ocelot
            super().calc_wisun_phy_mode_id(model)
        elif profile_name in ["wisun_fan_1_1", "connect_ofdm", "sun_oqpsk"]:
            if modulation_type == model.vars.modulation_type.var_enum.FSK2:
                # Call method from Ocelot
                super().calc_wisun_phy_mode_id(model)
            elif softmodem_modulation_type == model.vars.softmodem_modulation_type.var_enum.SUN_OQPSK:
                # : Assign Wi-SUN OQPSK chip rates in phy_type
                if sun_oqpsk_chiprate == model.vars.sun_oqpsk_chiprate.var_enum._100_KCPS:
                    phy_type = 6
                elif sun_oqpsk_chiprate == model.vars.sun_oqpsk_chiprate.var_enum._1000_KCPS:
                    phy_type = 7
                else: #: _2000_KCPS
                    phy_type = 8

                # : Define DSSS vs. MDSSS in bit 3 of phy_mode
                if sun_oqpsk_spreading_mode == model.vars.sun_oqpsk_spreading_mode.var_enum.DSSS:
                    spreading_mode = 0
                else:
                    spreading_mode = 1

                # : put all supported rate modes
                # : IEEE_802_15_4 table 21-4
                wisun_phy_mode_id = [0] * 4 # : pre allocate 4 rate modes
                wisun_phy_mode_id[0] = (phy_type << 4) + (spreading_mode << 3) + 0
                wisun_phy_mode_id[1] = (phy_type << 4) + (spreading_mode << 3) + 1
                wisun_phy_mode_id[2] = (phy_type << 4) + (spreading_mode << 3) + 2
                wisun_phy_mode_id[3] = (phy_type << 4) + (spreading_mode << 3) + 3

                # Write the variable
                model.vars.wisun_phy_mode_id.value = wisun_phy_mode_id

            else:
                # This case is for Wi-SUN OFDM
                ofdm_option = model.vars.ofdm_option.value
                wisun_phy_mode_id = [0] * 8
                phy_type = int(ofdm_option) + 2
                wisun_phy_mode_id[0] = (phy_type << 4) + 0
                wisun_phy_mode_id[1] = (phy_type << 4) + 1
                wisun_phy_mode_id[2] = (phy_type << 4) + 2
                wisun_phy_mode_id[3] = (phy_type << 4) + 3
                wisun_phy_mode_id[4] = (phy_type << 4) + 4
                wisun_phy_mode_id[5] = (phy_type << 4) + 5
                wisun_phy_mode_id[6] = (phy_type << 4) + 6
                wisun_phy_mode_id[7] = (phy_type << 4) + 7

                #Write the variable
                model.vars.wisun_phy_mode_id.value = wisun_phy_mode_id
        elif profile_name == "connect":
            # : Connect stack team requests Connect SUN FSK phys have a different phy_mode_id compared to Connect OQPSK,
            # : Connect OFDM, and Legacy Connect PHYs. Thus, override stack_info if the PHY is Connect SUN-FSK
            # : https://jira.silabs.com/browse/MCUW_RADIO_CFG-2375
            if header_size == 2: # : Connect SUN-FSK is two byte header
                if fec_enabled:
                    model.vars.wisun_phy_mode_id.value = [1, 17]
                else:
                    model.vars.wisun_phy_mode_id.value = [1]

    def calc_conc_ofdm_option(self, model):
        #By default set this model variable to NONE
        model.vars.conc_ofdm_option.value = model.vars.conc_ofdm_option.var_enum.NONE

    def calc_ofdm_stf_length(self, model):
        #By default set this model variable to 4
        modulation_type = model.vars.modulation_type.value
        softmodem_antdivmode = model.vars.softmodem_antdivmode.value

        softmodem_antdivmode_en = softmodem_antdivmode != model.vars.softmodem_antdivmode.var_enum.DISABLE\
            and softmodem_antdivmode != model.vars.softmodem_antdivmode.var_enum.ANTENNA1

        # Note that ofdm_stf_length is only set and used for OFDM PHYs (avoid clutter)
        if modulation_type == model.vars.modulation_type.var_enum.OFDM:
            if softmodem_antdivmode_en:
                # Antenna diversity mode need longer STF length
                ofdm_stf_length = 10
            else:
                # 4 * 120 us symbol
                ofdm_stf_length = 4

            model.vars.ofdm_stf_length.value = ofdm_stf_length

    def calc_ofdm_preemph_sub_filter_number(self, model):
        # By default set this model variable to 0
        model.vars.ofdm_preemph_sub_filter_number.value = 0
