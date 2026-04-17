from .lut_representation.wisun_1v0_luts import Wisun1v0Luts

class LutWisunFan1v0:

    MHZ_TO_HZ = 1_000_000

    @staticmethod
    def __check_bitrate_key_exist(mode: str) -> bool:
        if mode in Wisun1v0Luts.bitrate_modulation_lut.keys():
            return True

        return False

    @staticmethod
    def __generate_frequency_key(reg_abbrev: str, operating_class: int) -> str | None:
        key = f"{reg_abbrev}_{operating_class}"
        if key not in Wisun1v0Luts.frequency_channel_lut.keys():
            return None

        return key

    @staticmethod
    def get_modulation(mode: str) -> str | None:
        if not LutWisunFan1v0.__check_bitrate_key_exist(mode):
            return None

        return 'FSK'

    @staticmethod
    def get_modulation_index(mode: str) -> float | None:
        if not LutWisunFan1v0.__check_bitrate_key_exist(mode):
            return None

        modulation_index = Wisun1v0Luts.bitrate_modulation_lut[mode].modulation_index
        return  modulation_index

    @staticmethod
    def get_bitrates(mode: str) -> list[int] | None:
        if not LutWisunFan1v0.__check_bitrate_key_exist(mode):
            return None

        bitrates = Wisun1v0Luts.bitrate_modulation_lut[mode].bitrates
        return bitrates

    @staticmethod
    def get_mcs_restrictions(mode: str) -> None:
        """In fan1v0 mcs_restriction not exist, it's connected to OFDM and fan1v0 doesn't have OFDM option.
        Keep it to be consistent with fan1v1."""
        return None

    @staticmethod
    def get_freq_band_start(reg_abbrev: str, operating_class: int) -> int | None:
        key = LutWisunFan1v0.__generate_frequency_key(reg_abbrev, operating_class)
        if key is None:
            return None

        freq_band_start = int(Wisun1v0Luts.frequency_channel_lut[key].freq_band_start * LutWisunFan1v0.MHZ_TO_HZ)
        return freq_band_start

    @staticmethod
    def get_freq_band_end(reg_abbrev: str, operating_class: int) -> int | None:
        key = LutWisunFan1v0.__generate_frequency_key(reg_abbrev, operating_class)
        if key is None:
            return None

        freq_band_end = int(Wisun1v0Luts.frequency_channel_lut[key].freq_band_end * LutWisunFan1v0.MHZ_TO_HZ)
        return freq_band_end

    @staticmethod
    def get_start_channel_number(reg_abbrev: str, operating_class: int) -> int | None:
        key = LutWisunFan1v0.__generate_frequency_key(reg_abbrev, operating_class)
        if key is None:
            return None

        start_channel_number = 0
        return start_channel_number

    @staticmethod
    def get_end_channel_number(reg_abbrev: str, operating_class: int) -> int | None:
        key = LutWisunFan1v0.__generate_frequency_key(reg_abbrev, operating_class)
        if key is None:
            return None

        end_channel_number = Wisun1v0Luts.frequency_channel_lut[key].total_nb_channel - 1
        return end_channel_number
