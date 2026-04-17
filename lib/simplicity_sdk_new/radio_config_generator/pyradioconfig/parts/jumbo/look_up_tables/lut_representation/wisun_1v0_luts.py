# THIS IS GENERATED CODE - DO NOT EDIT
from pyradioconfig.parts.jumbo.look_up_tables.lut_dataclasses.wisun_dataclasses import BitrateModulation1v0, \
    FrequencyChannel1v0

class Wisun1v0Luts:
    """
    Class to hold the Wisun 1.0 look-up tables (LUTs) representations.
    This class is used to define the LUTs for the Wisun 1.0 standard.
    """
    bitrate_modulation_lut: dict[str, BitrateModulation1v0] = {
        'Mode1a': BitrateModulation1v0(
            phy_mode_id_base=0x01,
            modulation_index=0.5,
            bitrates=[50_000]
        ),
        'Mode1b': BitrateModulation1v0(
            phy_mode_id_base=0x02,
            modulation_index=1.0,
            bitrates=[50_000]
        ),
        'Mode2a': BitrateModulation1v0(
            phy_mode_id_base=0x03,
            modulation_index=0.5,
            bitrates=[100_000]
        ),
        'Mode2b': BitrateModulation1v0(
            phy_mode_id_base=0x04,
            modulation_index=1.0,
            bitrates=[100_000]
        ),
        'Mode3': BitrateModulation1v0(
            phy_mode_id_base=0x05,
            modulation_index=0.5,
            bitrates=[150_000]
        ),
        'Mode4a': BitrateModulation1v0(
            phy_mode_id_base=0x06,
            modulation_index=0.5,
            bitrates=[200_000]
        ),
        'Mode4b': BitrateModulation1v0(
            phy_mode_id_base=0x07,
            modulation_index=1.0,
            bitrates=[200_000]
        ),
        'Mode5': BitrateModulation1v0(
            phy_mode_id_base=0x08,
            modulation_index=0.5,
            bitrates=[300_000]
        ),
    }
    frequency_channel_lut: dict[str, FrequencyChannel1v0] = {
        'AZ_NZ_1': FrequencyChannel1v0(
            total_nb_channel=64,
            freq_band_start=915.0,
            freq_band_end=928.0
        ),
        'AZ_NZ_2': FrequencyChannel1v0(
            total_nb_channel=32,
            freq_band_start=915.0,
            freq_band_end=928.0
        ),
        'BZ_1': FrequencyChannel1v0(
            total_nb_channel=129,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'BZ_2': FrequencyChannel1v0(
            total_nb_channel=64,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'BZ_3': FrequencyChannel1v0(
            total_nb_channel=42,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'CN_1': FrequencyChannel1v0(
            total_nb_channel=199,
            freq_band_start=470.0,
            freq_band_end=510.0
        ),
        'CN_4': FrequencyChannel1v0(
            total_nb_channel=16,
            freq_band_start=920.5,
            freq_band_end=924.5
        ),
        'EU_1': FrequencyChannel1v0(
            total_nb_channel=69,
            freq_band_start=863.0,
            freq_band_end=870.0
        ),
        'EU_2': FrequencyChannel1v0(
            total_nb_channel=35,
            freq_band_start=863.0,
            freq_band_end=870.0
        ),
        'EU_3': FrequencyChannel1v0(
            total_nb_channel=55,
            freq_band_start=870.0,
            freq_band_end=876.0
        ),
        'EU_4': FrequencyChannel1v0(
            total_nb_channel=27,
            freq_band_start=870.0,
            freq_band_end=876.0
        ),
        'HK_1': FrequencyChannel1v0(
            total_nb_channel=24,
            freq_band_start=920.0,
            freq_band_end=925.0
        ),
        'HK_2': FrequencyChannel1v0(
            total_nb_channel=12,
            freq_band_start=920.0,
            freq_band_end=925.0
        ),
        'IN_1': FrequencyChannel1v0(
            total_nb_channel=29,
            freq_band_start=865.0,
            freq_band_end=867.0
        ),
        'IN_2': FrequencyChannel1v0(
            total_nb_channel=15,
            freq_band_start=865.0,
            freq_band_end=867.0
        ),
        'JP_1': FrequencyChannel1v0(
            total_nb_channel=38,
            freq_band_start=920.0,
            freq_band_end=928.0
        ),
        'JP_2': FrequencyChannel1v0(
            total_nb_channel=18,
            freq_band_start=920.0,
            freq_band_end=928.0
        ),
        'JP_3': FrequencyChannel1v0(
            total_nb_channel=12,
            freq_band_start=920.0,
            freq_band_end=928.0
        ),
        'KR_1': FrequencyChannel1v0(
            total_nb_channel=32,
            freq_band_start=917.0,
            freq_band_end=923.5
        ),
        'KR_2': FrequencyChannel1v0(
            total_nb_channel=16,
            freq_band_start=917.0,
            freq_band_end=923.5
        ),
        'MX_1': FrequencyChannel1v0(
            total_nb_channel=129,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'MX_2': FrequencyChannel1v0(
            total_nb_channel=64,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'MY_1': FrequencyChannel1v0(
            total_nb_channel=19,
            freq_band_start=919.0,
            freq_band_end=923.0
        ),
        'MY_2': FrequencyChannel1v0(
            total_nb_channel=10,
            freq_band_start=919.0,
            freq_band_end=923.0
        ),
        'NA_1': FrequencyChannel1v0(
            total_nb_channel=129,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'NA_2': FrequencyChannel1v0(
            total_nb_channel=64,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'NA_3': FrequencyChannel1v0(
            total_nb_channel=42,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'PH_1': FrequencyChannel1v0(
            total_nb_channel=64,
            freq_band_start=915.0,
            freq_band_end=918.0
        ),
        'PH_2': FrequencyChannel1v0(
            total_nb_channel=32,
            freq_band_start=915.0,
            freq_band_end=918.0
        ),
        'SG_1': FrequencyChannel1v0(
            total_nb_channel=29,
            freq_band_start=866.0,
            freq_band_end=869.0
        ),
        'SG_2': FrequencyChannel1v0(
            total_nb_channel=15,
            freq_band_start=866.0,
            freq_band_end=869.0
        ),
        'SG_3': FrequencyChannel1v0(
            total_nb_channel=7,
            freq_band_start=866.0,
            freq_band_end=869.0
        ),
        'SG_4': FrequencyChannel1v0(
            total_nb_channel=39,
            freq_band_start=917.0,
            freq_band_end=925.0
        ),
        'SG_5': FrequencyChannel1v0(
            total_nb_channel=19,
            freq_band_start=917.0,
            freq_band_end=925.0
        ),
        'TH_1': FrequencyChannel1v0(
            total_nb_channel=24,
            freq_band_start=920.0,
            freq_band_end=925.0
        ),
        'TH_2': FrequencyChannel1v0(
            total_nb_channel=12,
            freq_band_start=920.0,
            freq_band_end=925.0
        ),
        'VN_1': FrequencyChannel1v0(
            total_nb_channel=24,
            freq_band_start=920.0,
            freq_band_end=925.0
        ),
        'VN_2': FrequencyChannel1v0(
            total_nb_channel=12,
            freq_band_start=920.0,
            freq_band_end=925.0
        ),
        'WW_1': FrequencyChannel1v0(
            total_nb_channel=416,
            freq_band_start=2400.0,
            freq_band_end=2483.5
        ),
        'WW_2': FrequencyChannel1v0(
            total_nb_channel=207,
            freq_band_start=2400.0,
            freq_band_end=2483.5
        ),
    }