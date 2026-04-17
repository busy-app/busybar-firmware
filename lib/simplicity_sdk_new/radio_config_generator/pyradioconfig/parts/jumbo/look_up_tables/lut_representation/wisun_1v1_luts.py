# THIS IS GENERATED CODE - DO NOT EDIT
from pyradioconfig.parts.jumbo.look_up_tables.lut_dataclasses.wisun_dataclasses import BitrateModulation1v1, \
    FrequencyChannel1v1

class Wisun1v1Luts:
    """
    Class to hold the Wisun 1.1 look-up tables (LUTs) representations.
    This class is used to define the LUTs for the Wisun 1.1 standard.
    """
    bitrate_modulation_lut: dict[str, BitrateModulation1v1] = {
        '1a': BitrateModulation1v1(
            modulation='FSK',
            modulation_index=0.5,
            bitrates=[50_000],
            restrictions=None
        ),
        '1b': BitrateModulation1v1(
            modulation='FSK',
            modulation_index=1.0,
            bitrates=[50_000],
            restrictions=None
        ),
        '2a': BitrateModulation1v1(
            modulation='FSK',
            modulation_index=0.5,
            bitrates=[100_000],
            restrictions=None
        ),
        '2b': BitrateModulation1v1(
            modulation='FSK',
            modulation_index=1.0,
            bitrates=[100_000],
            restrictions=None
        ),
        '3': BitrateModulation1v1(
            modulation='FSK',
            modulation_index=0.5,
            bitrates=[150_000],
            restrictions=None
        ),
        '4a': BitrateModulation1v1(
            modulation='FSK',
            modulation_index=0.5,
            bitrates=[200_000],
            restrictions=None
        ),
        '4b': BitrateModulation1v1(
            modulation='FSK',
            modulation_index=1.0,
            bitrates=[200_000],
            restrictions=None
        ),
        '5': BitrateModulation1v1(
            modulation='FSK',
            modulation_index=0.5,
            bitrates=[300_000],
            restrictions=None
        ),
        'opt1': BitrateModulation1v1(
            modulation='OFDM',
            modulation_index=None,
            bitrates=[100_000, 200_000, 400_000, 800_000, 1_200_000, 1_600_000, 2_400_000, 3_600_000],
            restrictions=[2, 3, 4, 5, 6, 7]
        ),
        'opt2': BitrateModulation1v1(
            modulation='OFDM',
            modulation_index=None,
            bitrates=[50_000, 100_000, 200_000, 400_000, 600_000, 800_000, 1_200_000, 1_800_000],
            restrictions=[3, 4, 5, 6, 7]
        ),
        'opt3': BitrateModulation1v1(
            modulation='OFDM',
            modulation_index=None,
            bitrates=[25_000, 50_000, 100_000, 200_000, 300_000, 400_000, 600_000, 900_000],
            restrictions=[4, 5, 6, 7]
        ),
        'opt4': BitrateModulation1v1(
            modulation='OFDM',
            modulation_index=None,
            bitrates=[12_500, 25_000, 50_000, 100_000, 150_000, 200_000, 300_000, 450_000],
            restrictions=[4, 5, 6, 7]
        ),
    }
    frequency_channel_lut: dict[str, FrequencyChannel1v1] = {
        'AZ_NZ_48': FrequencyChannel1v1(
            total_nb_channel=64,
            freq_band_start=915.0,
            freq_band_end=928.0
        ),
        'AZ_NZ_49': FrequencyChannel1v1(
            total_nb_channel=32,
            freq_band_start=915.0,
            freq_band_end=928.0
        ),
        'BZ_1': FrequencyChannel1v1(
            total_nb_channel=129,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'BZ_2': FrequencyChannel1v1(
            total_nb_channel=64,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'BZ_3': FrequencyChannel1v1(
            total_nb_channel=42,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'BZ_4': FrequencyChannel1v1(
            total_nb_channel=31,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'BZ_5': FrequencyChannel1v1(
            total_nb_channel=20,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'CN_128': FrequencyChannel1v1(
            total_nb_channel=16,
            freq_band_start=920.5,
            freq_band_end=924.5
        ),
        'CN_160': FrequencyChannel1v1(
            total_nb_channel=199,
            freq_band_start=470.0,
            freq_band_end=510.0
        ),
        'EU_32': FrequencyChannel1v1(
            total_nb_channel=69,
            freq_band_start=863.0,
            freq_band_end=870.0
        ),
        'EU_33': FrequencyChannel1v1(
            total_nb_channel=35,
            freq_band_start=863.0,
            freq_band_end=870.0
        ),
        'EU_34': FrequencyChannel1v1(
            total_nb_channel=55,
            freq_band_start=870.0,
            freq_band_end=876.0
        ),
        'EU_35': FrequencyChannel1v1(
            total_nb_channel=27,
            freq_band_start=870.0,
            freq_band_end=876.0
        ),
        'EU_36': FrequencyChannel1v1(
            total_nb_channel=125,
            freq_band_start=863.0,
            freq_band_end=876.0
        ),
        'EU_37': FrequencyChannel1v1(
            total_nb_channel=62,
            freq_band_start=863.0,
            freq_band_end=876.0
        ),
        'HK_64': FrequencyChannel1v1(
            total_nb_channel=24,
            freq_band_start=920.0,
            freq_band_end=925.0
        ),
        'HK_65': FrequencyChannel1v1(
            total_nb_channel=12,
            freq_band_start=920.0,
            freq_band_end=925.0
        ),
        'IN_39': FrequencyChannel1v1(
            total_nb_channel=29,
            freq_band_start=865.0,
            freq_band_end=867.0
        ),
        'IN_40': FrequencyChannel1v1(
            total_nb_channel=15,
            freq_band_start=865.0,
            freq_band_end=867.0
        ),
        'JP_21': FrequencyChannel1v1(
            total_nb_channel=38,
            freq_band_start=920.0,
            freq_band_end=928.0
        ),
        'JP_22': FrequencyChannel1v1(
            total_nb_channel=18,
            freq_band_start=920.0,
            freq_band_end=928.0
        ),
        'JP_23': FrequencyChannel1v1(
            total_nb_channel=12,
            freq_band_start=920.0,
            freq_band_end=928.0
        ),
        'JP_24': FrequencyChannel1v1(
            total_nb_channel=9,
            freq_band_start=920.0,
            freq_band_end=928.0
        ),
        'KR_96': FrequencyChannel1v1(
            total_nb_channel=32,
            freq_band_start=917.0,
            freq_band_end=923.5
        ),
        'KR_97': FrequencyChannel1v1(
            total_nb_channel=16,
            freq_band_start=917.0,
            freq_band_end=923.5
        ),
        'MX_1': FrequencyChannel1v1(
            total_nb_channel=129,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'MX_2': FrequencyChannel1v1(
            total_nb_channel=64,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'MY_80': FrequencyChannel1v1(
            total_nb_channel=19,
            freq_band_start=919.0,
            freq_band_end=923.0
        ),
        'MY_81': FrequencyChannel1v1(
            total_nb_channel=10,
            freq_band_start=919.0,
            freq_band_end=923.0
        ),
        'NA_1': FrequencyChannel1v1(
            total_nb_channel=129,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'NA_2': FrequencyChannel1v1(
            total_nb_channel=64,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'NA_3': FrequencyChannel1v1(
            total_nb_channel=42,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'NA_4': FrequencyChannel1v1(
            total_nb_channel=31,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'NA_5': FrequencyChannel1v1(
            total_nb_channel=20,
            freq_band_start=902.0,
            freq_band_end=928.0
        ),
        'PH_48': FrequencyChannel1v1(
            total_nb_channel=64,
            freq_band_start=915.0,
            freq_band_end=918.0
        ),
        'PH_49': FrequencyChannel1v1(
            total_nb_channel=32,
            freq_band_start=915.0,
            freq_band_end=918.0
        ),
        'SG_41': FrequencyChannel1v1(
            total_nb_channel=29,
            freq_band_start=866.0,
            freq_band_end=869.0
        ),
        'SG_42': FrequencyChannel1v1(
            total_nb_channel=15,
            freq_band_start=866.0,
            freq_band_end=869.0
        ),
        'SG_43': FrequencyChannel1v1(
            total_nb_channel=7,
            freq_band_start=866.0,
            freq_band_end=869.0
        ),
        'SG_98': FrequencyChannel1v1(
            total_nb_channel=39,
            freq_band_start=917.0,
            freq_band_end=925.0
        ),
        'SG_99': FrequencyChannel1v1(
            total_nb_channel=19,
            freq_band_start=917.0,
            freq_band_end=925.0
        ),
        'TH_64': FrequencyChannel1v1(
            total_nb_channel=24,
            freq_band_start=920.0,
            freq_band_end=925.0
        ),
        'TH_65': FrequencyChannel1v1(
            total_nb_channel=12,
            freq_band_start=920.0,
            freq_band_end=925.0
        ),
        'VN_64': FrequencyChannel1v1(
            total_nb_channel=24,
            freq_band_start=920.0,
            freq_band_end=925.0
        ),
        'VN_65': FrequencyChannel1v1(
            total_nb_channel=12,
            freq_band_start=920.0,
            freq_band_end=925.0
        ),
        'WW_112': FrequencyChannel1v1(
            total_nb_channel=416,
            freq_band_start=2400.0,
            freq_band_end=2483.5
        ),
        'WW_113': FrequencyChannel1v1(
            total_nb_channel=207,
            freq_band_start=2400.0,
            freq_band_end=2483.5
        ),
    }