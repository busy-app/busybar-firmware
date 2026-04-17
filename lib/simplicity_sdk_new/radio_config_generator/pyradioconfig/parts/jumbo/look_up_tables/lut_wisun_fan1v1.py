from pycalcmodel.core.model import ModelRoot
from .lut_representation.wisun_1v1_luts import Wisun1v1Luts

class LutWisunFan1v1:
    MHZ_TO_HZ = 1_000_000

    @staticmethod
    def __generate_bitrate_key(code: int) -> str | None:
        if code == 0x01 or code == 0x11:
            return '1a'
        if code == 0x02 or code == 0x12:
            return '1b'
        if code == 0x03 or code == 0x13:
            return '2a'
        if code == 0x04 or code == 0x14:
            return '2b'
        if code == 0x05 or code == 0x15:
            return '3'
        if code == 0x06 or code == 0x16:
            return '4a'
        if code == 0x07 or code == 0x17:
            return '4b'
        if code == 0x08 or code == 0x18:
            return '5'

        if code >= 0x50 :
            return 'opt4'
        elif code >= 0x40:
            return 'opt3'
        elif code >= 0x30:
            return 'opt2'
        elif code >= 0x20:
            return 'opt1'

        return None

    @staticmethod
    def __generate_frequency_key(reg_domain: str, channel_plan_id: int) -> str | None:
        key = f"{reg_domain}_{channel_plan_id}"
        if key not in Wisun1v1Luts.frequency_channel_lut.keys():
            return None

        return key

    @staticmethod
    def get_modulation(code: int) -> str | None:
        key = LutWisunFan1v1.__generate_bitrate_key(code)
        if key is None:
            return None

        modulation = Wisun1v1Luts.bitrate_modulation_lut[key].modulation
        return modulation

    @staticmethod
    def get_modulation_index(code: int) -> float | None:
        key = LutWisunFan1v1.__generate_bitrate_key(code)
        if key is None:
            return None

        modulation_index = Wisun1v1Luts.bitrate_modulation_lut[key].modulation_index
        return modulation_index

    @staticmethod
    def get_bitrates(code: int) -> list[int] | None:
        key = LutWisunFan1v1.__generate_bitrate_key(code)
        if key is None:
            return None

        bitrates = Wisun1v1Luts.bitrate_modulation_lut[key].bitrates
        return bitrates

    @staticmethod
    def get_mcs_restrictions(code: int) -> list[int] | None:
        key = LutWisunFan1v1.__generate_bitrate_key(code)
        if key is None:
            return None

        restrictions = Wisun1v1Luts.bitrate_modulation_lut[key].restrictions
        return restrictions

    @staticmethod
    def get_freq_band_start(reg_domain: str, channel_plan_id: int) -> int | None:
        key = LutWisunFan1v1.__generate_frequency_key(reg_domain, channel_plan_id)
        if key is None:
            return None

        freq_band_start = int(Wisun1v1Luts.frequency_channel_lut[key].freq_band_start * LutWisunFan1v1.MHZ_TO_HZ)
        return freq_band_start

    @staticmethod
    def get_freq_band_end(reg_domain: str, channel_plan_id: int) -> int | None:
        key = LutWisunFan1v1.__generate_frequency_key(reg_domain, channel_plan_id)
        if key is None:
            return None

        freq_band_end = int(Wisun1v1Luts.frequency_channel_lut[key].freq_band_end * LutWisunFan1v1.MHZ_TO_HZ)
        return freq_band_end

    @staticmethod
    def __get_right_phy_mode_id(phy_mode_id: int) -> int:
        if phy_mode_id < 0x20:
            return phy_mode_id

        if phy_mode_id > 0x50:
            return 0x50
        if phy_mode_id > 0x40:
            return  0x40
        if phy_mode_id > 0x30:
            return 0x30
        if phy_mode_id > 0x20:
            return 0x20

    @staticmethod
    def get_start_channel_number(reg_domain: str, channel_plan_id: int, phy_mode_id: int) -> int | None:
        key = LutWisunFan1v1.__generate_frequency_key(reg_domain, channel_plan_id)
        if key is None:
            return None

        real_phy_mode = LutWisunFan1v1.__get_right_phy_mode_id(phy_mode_id)
        start_channel_number = real_phy_mode * 256
        return start_channel_number

    @staticmethod
    def get_end_channel_number(reg_domain: str, channel_plan_id: int, phy_mode_id: int) -> int | None:
        key = LutWisunFan1v1.__generate_frequency_key(reg_domain, channel_plan_id)
        if key is None:
            return None

        start_channel_number = LutWisunFan1v1.get_start_channel_number(reg_domain, channel_plan_id, phy_mode_id)
        if start_channel_number is None:
            return None

        total_channel_number = Wisun1v1Luts.frequency_channel_lut[key].total_nb_channel
        end_channel_number = start_channel_number + total_channel_number - 1

        return end_channel_number

    @staticmethod
    def get_group(reg_domain: str, channel_plan_id: int, model: ModelRoot):
        #TODO: Refactor, currently hardcoded
        key = LutWisunFan1v1.__generate_frequency_key(reg_domain, channel_plan_id)
        if key is None:
            return None

        if reg_domain == 'AZ_NZ':
            return model.vars.meta_group.var_enum.AZ_NZ
        if reg_domain == 'BZ':
            return model.vars.meta_group.var_enum.BZ
        if reg_domain == 'HK':
            return model.vars.meta_group.var_enum.HK
        if reg_domain == 'IN':
            return model.vars.meta_group.var_enum.IN
        if reg_domain == 'JP':
            return model.vars.meta_group.var_enum.JP
        if reg_domain == 'KR':
            return model.vars.meta_group.var_enum.KR
        if reg_domain == 'MX':
            return model.vars.meta_group.var_enum.MX
        if reg_domain == 'MY':
            return model.vars.meta_group.var_enum.MY
        if reg_domain == 'NA':
            return model.vars.meta_group.var_enum.NA
        if reg_domain == 'PH':
            return model.vars.meta_group.var_enum.PH
        if reg_domain == 'TH':
            return model.vars.meta_group.var_enum.TH
        if reg_domain == 'VN':
            return model.vars.meta_group.var_enum.VN
        if reg_domain == 'WW':
            return model.vars.meta_group.var_enum.WW

        if reg_domain == 'CN':
            if channel_plan_id in [160]:
                return model.vars.meta_group.var_enum.CN1
            if channel_plan_id in [128]:
                return model.vars.meta_group.var_enum.CN2

        if reg_domain == 'EU':
            if channel_plan_id in [32,33]:
                return model.vars.meta_group.var_enum.EU1
            if channel_plan_id in [34,35]:
                return model.vars.meta_group.var_enum.EU2
            if channel_plan_id in [36,37]:
                return model.vars.meta_group.var_enum.EU3

        if reg_domain == 'SG':
            if channel_plan_id in [41,42,43]:
                return model.vars.meta_group.var_enum.SG1
            if channel_plan_id in [98,99]:
                return model.vars.meta_group.var_enum.SG2

        raise ValueError(f"{reg_domain=} is not an acceptable value")
