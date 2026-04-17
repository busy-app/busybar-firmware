from dataclasses import dataclass

@dataclass
class BitrateModulation1v1:
    modulation: str
    modulation_index: float | None
    bitrates: list[int]
    restrictions: list[int] | None

@dataclass
class FrequencyChannel1v1:
    total_nb_channel: int
    freq_band_start: float
    freq_band_end: float

@dataclass
class BitrateModulation1v0:
    phy_mode_id_base : int
    modulation_index : float
    bitrates: list[int]

@dataclass
class FrequencyChannel1v0:
    total_nb_channel: int
    freq_band_start: float
    freq_band_end: float
