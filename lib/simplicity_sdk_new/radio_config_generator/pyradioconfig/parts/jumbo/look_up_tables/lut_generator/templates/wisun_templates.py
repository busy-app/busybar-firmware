class Wisun1v0Templates:
    """
    This class contains the templates for Wisun 1.0 look-up table generation.
    """

    import_and_class: str = \
'''# THIS IS GENERATED CODE - DO NOT EDIT
from pyradioconfig.parts.jumbo.look_up_tables.lut_dataclasses.wisun_dataclasses import BitrateModulation1v0, \\
    FrequencyChannel1v0

class Wisun1v0Luts:
    """
    Class to hold the Wisun 1.0 look-up tables (LUTs) representations.
    This class is used to define the LUTs for the Wisun 1.0 standard.
    """
    '''.strip()

    bitrate_modulation_start: str = '''
    bitrate_modulation_lut: dict[str, BitrateModulation1v0] = {'''

    close: str = '''\n    }'''

    @staticmethod
    def bitrate_modulation(mode: str, modulation_index: float, bitrates: list[int])->str:
        phy_mode_id_bases: dict[str,int] = {
            'Mode1a': 0x01,
            'Mode1b': 0x02,
            'Mode2a': 0x03,
            'Mode2b': 0x04,
            'Mode3': 0x05,
            'Mode4a': 0x06,
            'Mode4b': 0x07,
            'Mode5': 0x08,
        }

        if mode not in phy_mode_id_bases:
            raise ValueError(f"Invalid mode '{mode}' for Wisun 1.0 bitrate modulation. "
                             f"Accepted modes are: {list(phy_mode_id_bases.keys())}")

        bitrates = _convert_bitrates(bitrates)

        return f"""
        '{mode}': BitrateModulation1v0(
            phy_mode_id_base=0x{phy_mode_id_bases[mode]:02x},
            modulation_index={modulation_index},
            bitrates={bitrates}
        ),"""

    frequency_channel_start: str = '''
    frequency_channel_lut: dict[str, FrequencyChannel1v0] = {'''

    @staticmethod
    def frequency_channel(key: str, channel_number: int, freq_band_start:float, freq_band_end: float) -> str:
        return f"""
        '{key}': FrequencyChannel1v0(
            total_nb_channel={channel_number},
            freq_band_start={freq_band_start},
            freq_band_end={freq_band_end}
        ),"""

class Wisun1v1Templates:
    """
    This class contains the templates for Wisun 1.1 look-up table generation.
    """

    import_and_class: str = '''
# THIS IS GENERATED CODE - DO NOT EDIT
from pyradioconfig.parts.jumbo.look_up_tables.lut_dataclasses.wisun_dataclasses import BitrateModulation1v1, \\
    FrequencyChannel1v1

class Wisun1v1Luts:
    """
    Class to hold the Wisun 1.1 look-up tables (LUTs) representations.
    This class is used to define the LUTs for the Wisun 1.1 standard.
    """'''.strip()

    bitrate_modulation_start: str = '''
    bitrate_modulation_lut: dict[str, BitrateModulation1v1] = {'''

    close: str = '''\n    }'''

    @staticmethod
    def bitrate_modulation(mode: str, modulation_index: float | None, bitrates: list[int],
                           restriction: list[int] | None) -> str:
        if modulation_index is None:
            modulation_index = "None"
        if restriction is None:
            restriction = "None"

        ofdm_options = ['opt1', 'opt2', 'opt3', 'opt4']
        fsk_options = ['1a', '1b', '2a', '2b', '3', '4a', '4b', '5']

        if mode in ofdm_options:
            modulation = "OFDM"
        elif mode in fsk_options:
            modulation = "FSK"
        else:
            raise ValueError(f"Invalid mode '{mode}' for Wisun 1.1 bitrate modulation."
                             f"Accepted modes are: {ofdm_options + fsk_options}")

        bitrates = _convert_bitrates(bitrates)

        return f"""
        '{mode}': BitrateModulation1v1(
            modulation='{modulation}',
            modulation_index={modulation_index},
            bitrates={bitrates},
            restrictions={restriction}
        ),"""

    frequency_channel_start: str = '''
    frequency_channel_lut: dict[str, FrequencyChannel1v1] = {'''

    @staticmethod
    def frequency_channel(key: str, channel_number: int, freq_band_start: float, freq_band_end: float) -> str:
        return f"""
        '{key}': FrequencyChannel1v1(
            total_nb_channel={channel_number},
            freq_band_start={freq_band_start},
            freq_band_end={freq_band_end}
        ),"""

def _convert_bitrates(bitrates: list[int])->str:
    new = f"[{', '.join(f'{bitrate:_}' for bitrate in bitrates)}]"
    return new

if __name__ == "__main__":
    # Example usage 1v0
    print(Wisun1v0Templates.import_and_class)
    print(Wisun1v0Templates.bitrate_modulation_start)
    print(Wisun1v0Templates.bitrate_modulation("Mode1a", 0.5, [1000, 2000, 3000]))
    print(Wisun1v0Templates.close)
    print(Wisun1v0Templates.frequency_channel_start)
    print(Wisun1v0Templates.frequency_channel("channel_example", 10, 2400.0, 2483.5))
    print(Wisun1v0Templates.close)
    # Example usage 1v1
    print(Wisun1v1Templates.import_and_class)
    print(Wisun1v1Templates.bitrate_modulation_start)
    print(Wisun1v1Templates.bitrate_modulation("opt1", 0.5, [1000, 2000], [1, 2]))
    print(Wisun1v1Templates.close)
    print(Wisun1v1Templates.frequency_channel_start)
    print(Wisun1v1Templates.frequency_channel("channel_example", 10, 2400.0, 2483.5))
    print(Wisun1v1Templates.close)