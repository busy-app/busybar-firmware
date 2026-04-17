from pyradioconfig.parts.bobcat.calculators.calc_viterbi import Calc_Viterbi_Bobcat
from pyradioconfig.calculator_model_framework.Utils.LogMgr import LogMgr
from py_2_and_3_compatibility import *

class CalcViterbiRainier(Calc_Viterbi_Bobcat):
    acqwin_unit = 1

    def calc_trecs_optimize_cost_thd(self, model):
        trecs_enabled = model.vars.trecs_enabled.value
        protocol_id = model.vars.protocol_id.value
        is_ble = protocol_id == model.vars.protocol_id.var_enum.BLE

        enable_opt = trecs_enabled and not is_ble
        model.vars.trecs_optimize_cost_thd.value = enable_opt

    def calc_trecs_weak_syncword_optimization(self, model):
        trecs_enabled = model.vars.trecs_enabled.value
        protocol_id = model.vars.protocol_id.value
        is_ble = protocol_id == model.vars.protocol_id.var_enum.BLE

        enable_opt = trecs_enabled and not is_ble
        model.vars.trecs_weak_syncword_optimization.value = enable_opt

    def calc_pmoffset_reg(self, model):

        afc_oneshot_enabled = (model.vars.MODEM_AFC_AFCONESHOT.value == 1)
        rtschmode = model.vars.MODEM_REALTIMCFE_RTSCHMODE.value
        osr = model.vars.MODEM_TRECSCFG_TRECSOSR.value

        if (rtschmode == 1) and afc_oneshot_enabled:
            # Special case for dual syncword detection case where hard slicing on syncword is required
            # In this case we choose a minimal PMOFFSET to avoid a bad estimate due to AFC transient
            pmoffset = 2
        else:
            # + 2 for processing delay. Always a function of OSR as per Wentao
            pmoffset = osr * 2 + 2

        self._reg_write(model.vars.MODEM_TRECSCFG_PMOFFSET, pmoffset)

    def calc_realtimcfe_extenschbyp_reg(self, model):
        pass

    def calc_realtimcfe_rtschmode_reg(self, model):
        # This function calculates the RTSCHMODE register field for TRECS

        # Read in model variables
        dualsync = model.vars.syncword_dualsync.value
        demod_select = model.vars.demod_select.value
        ber_force_fdm0 = model.vars.ber_force_fdm0.value
        pmdeten = model.vars.MODEM_PHDMODCTRL_PMDETEN.value
        fast_detect_enable = (model.vars.fast_detect_enable.value == model.vars.fast_detect_enable.var_enum.ENABLED)

        # Calculate the register value based on whether we are using TRECS and whether dual syncword detect is enabled
        if demod_select == model.vars.demod_select.var_enum.TRECS_VITERBI or \
                demod_select == model.vars.demod_select.var_enum.TRECS_SLICER:
            # If dual syncword detection is enabled, then stop using CFE and hard slice syncword w TRECS
            if fast_detect_enable:
                rtschmode = 1
            else:
                rtschmode = 0  # 0 means detect timing again using syncword
        else:
            rtschmode = 0

        # Write the register
        self._reg_write(model.vars.MODEM_REALTIMCFE_RTSCHMODE, rtschmode)

    def calc_swcoeffen_reg(self, model):

        afc1shot_en = model.vars.MODEM_AFC_AFCONESHOT.value
        aox_en = model.vars.aox_enable.value == model.vars.aox_enable.var_enum.ENABLED
        enhanced_en = model.vars.demod_select.value == model.vars.demod_select.var_enum.ENHANCED_DSSS
        # ksi3swen is do not care if afc1shot_en is False - MCUW_RADIO_CFG-1901
        ksi3swen_donotccare = not afc1shot_en

        if afc1shot_en and aox_en:
            # both AFC oneshot and AoX cannot be simultaneously enabled as they both use the second CHF coefficient set
            LogMgr.Error('both AFC oneshot and AoX cannot be simultaneously enabled')

        swcoeffen = 1 if (afc1shot_en or aox_en or enhanced_en) else 0 # affects the channel filter switching only
        ksi3swenable = afc1shot_en
        # don't switch for aox, as KSI3 switch mechanism is based on dsa/preamble, but the aox channel switch is based on the CTE
        # Don't care about the demodulated data during CTE, so just leave it on the KSI3

        self._reg_write(model.vars.MODEM_CHFCTRL_SWCOEFFEN, swcoeffen)
        self._reg_write(model.vars.MODEM_VTCORRCFG1_KSI3SWENABLE, ksi3swenable, do_not_care=ksi3swen_donotccare)

    def calc_vtdemoden_reg(self, model):
        demod_sel = model.vars.demod_select.value
        dssscfe_combo = model.vars.MODEM_DIGMIXCTRL_DSSSCFECOMBO.value

        # VTDEMODEN is set if: the selected demod uses the TRECS
        # Or if the CFE of the TRECS is used along the DSSS correlator for timing and detection (dssscfecombo)
        if (demod_sel == model.vars.demod_select.var_enum.TRECS_VITERBI
                or demod_sel == model.vars.demod_select.var_enum.TRECS_SLICER
                or dssscfe_combo):
            reg = 1
        else:
            reg = 0

        self._reg_write(model.vars.MODEM_VITERBIDEMOD_VTDEMODEN, reg)

    def calc_pmexpectpatt_reg(self, model):
        pre_str = model.vars.preamble_string_actual.value
        mapfsk = model.vars.MODEM_CTRL0_MAPFSK.value
        demod_sel = model.vars.demod_select.value
        trecs_effective_preamble_len = model.vars.trecs_effective_preamble_len.value

        # Only calculate pmexpectpatt for TRECS or BCR (BCR reuses this reg)
        if demod_sel == model.vars.demod_select.var_enum.TRECS_VITERBI or demod_sel == model.vars.demod_select.var_enum.TRECS_SLICER or demod_sel == model.vars.demod_select.var_enum.BCR:

            #We can use the TX preamble string for this, becuase we only use a small number of bits corresponding to the eff preamble len
            effective_pre_str = pre_str[:trecs_effective_preamble_len] #This is the preamble once some bits are shifted to the syncword
            zero_filler_str = '0'*32 #Add 32 zeroes to the end to make sure we have a long enough string
            combined_str = effective_pre_str + zero_filler_str

            # if PM search is enabled set pattern to preamble string
            # then convert binary string to integer to write into register field
            reg = int(combined_str[0:32],2)
            # if MAPFSK is 1 mapping is inverted so invert the expected pattern to match
            if mapfsk:
                reg ^= 0xFFFFFFFF
        else:
            reg = 0

        self._reg_write(model.vars.MODEM_TRECPMPATT_PMEXPECTPATT, reg)

    def calc_demod_expect_patt_value(self, model):

        demod_select = model.vars.demod_select.value
        syncword0 = model.vars.MODEM_SYNC0_SYNC0.value
        mapfsk = model.vars.MODEM_CTRL0_MAPFSK.value
        trecs_pre_bits_to_syncword = model.vars.trecs_pre_bits_to_syncword.value
        preamble_string = model.vars.preamble_string_actual.value
        syncword_len = model.vars.syncword_length.value
        ber_force_sync = model.vars.ber_force_sync.value

        if demod_select == model.vars.demod_select.var_enum.TRECS_VITERBI or \
            demod_select == model.vars.demod_select.var_enum.TRECS_SLICER:

            if ber_force_sync:
                #If BER test mode is enabled then set the expected pattern to the first 32-bits of PN9 sequence
                patt = 0x052bcbb8

            else:
                syncword_str_part = '{:032b}'.format(syncword0)[-syncword_len:] #Read the rightmost characters

                #Need to check for zero because python treats -0 the same as 0 in terms of list slicing
                if trecs_pre_bits_to_syncword > 0:
                    #We can use the full TX preamble string for this because we are reading only the rightmost characters anyway
                    preamble_str_part = preamble_string[-trecs_pre_bits_to_syncword:]  # Read the rightmost characters
                else:
                    preamble_str_part = ""

                effective_syncword_str = preamble_str_part+syncword_str_part[::-1]+'0'*32 #reverse syncword part only

                #HW will add head and tail for correlation computation
                viterbi_demod_expect_patt = int(effective_syncword_str[0:32],2)
                patt = viterbi_demod_expect_patt

            # if MAPFSK is 1 mapping is inverted so invert the expected pattern to match
            if mapfsk:
                patt ^= 0xFFFFFFFF
        else:
            # set to default reset value
            patt = long(0x123556B7)

        model.vars.viterbi_demod_expect_patt.value = patt
