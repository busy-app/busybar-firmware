import numpy as np
import pandas as pd
from pandas.core.frame import DataFrame
import ast

class ExcelProcessor:
    """
    Class to process the WiSUN FAN 1.1 PHY Definitions Excel sheet and generate modulation and frequency channel dictionaries.
    """
    def __init__(self, file_path):
        self.file_path = file_path
        self.FILE_NAME = "WiSUN_FAN_1v1_PhyDefs.xlsx"
        self.SHEET_NAME = "WiSUN Configurator PHY Def List"
        self.df = pd.read_excel(f'{self.file_path}/{self.FILE_NAME}', sheet_name=self.SHEET_NAME, header=2)
        self.modulation: DataFrame | None = None
        self.freq : DataFrame | None = None

    def _create_modulation(self):
        """ Process the modulation section of the Excel sheet """
        modulation_columns: list[str] = ['Operating mode / Option', 'Modulation Index', 'Bitrate (kbits/s)',
                                         'MCS restriction']
        modulation: DataFrame = self.df[modulation_columns]
        modulation = modulation.replace({np.nan: None})
        modulation = modulation.drop_duplicates()
        modulation = modulation.dropna(how="all")
        modulation['Operating mode / Option'] = modulation['Operating mode / Option'].astype(str)
        modulation = modulation.set_index('Operating mode / Option')
        self.modulation = modulation.sort_index()

    def generate_modulation(self) -> dict[str, dict]:
        """
        Generate modulation dictionary from the modulation DataFrame
        :return: dictionary with modulation parameters as values and modulations as keys
        """
        if self.modulation is None:
            self._create_modulation()

        kbit_to_bit = 1000
        modulation_dict: dict[str, dict] = {}
        for index, row in self.modulation.iterrows():
            mode: str = str(index).lower()
            modulation_index: float | None = float(row["Modulation Index"]) \
                if row["Modulation Index"] is not None else None
            bitrates: list[float] = ast.literal_eval(row["Bitrate (kbits/s)"]) \
                if isinstance(row["Bitrate (kbits/s)"],str) else [row["Bitrate (kbits/s)"]]
            restrictions: list[int] | None = ast.literal_eval(row["MCS restriction"]) \
                if row["MCS restriction"] is not None else None

            bitrates = [int(bitrate * kbit_to_bit) for bitrate in bitrates]

            modulation_dict[mode] = {
                'modulation_index': modulation_index,
                'bitrates': bitrates,
                'restrictions': restrictions
            }

        return modulation_dict

    def _create_frequency_channel(self):
        """ Process the frequency channel section of the Excel sheet """
        frequency_columns: list[str] = ['Region abrev', 'ChPlanID', 'Total Nb Channel', 'Freq band start (MHz)',
                                        'Freq band end (MHz)', 'Operating class', 'FAN 1.0', 'FAN 1.1 (23Q4)']
        freq: DataFrame = self.df[frequency_columns]
        freq = freq.dropna(how="all")
        freq = freq.drop_duplicates()
        freq["Region abrev"] = freq["Region abrev"].fillna('NA')
        freq["ChPlanID"] = freq["ChPlanID"].astype(int).astype(str)
        freq["Total Nb Channel"] = freq["Total Nb Channel"].astype(int)
        self.freq = freq

    @staticmethod
    def _calculate_frequency_params(frequency: DataFrame) -> dict[str, dict]:
        """
        Calculate frequency parameters from the frequency DataFrame
        :param frequency: DataFrame with frequency parameters
        :return: dictionary with frequency parameters as values and <Region abrev>_<ChPlanID> as keys
        """
        frequency_dict: dict[str, dict] = {}
        for index, row in frequency.iterrows():
            key: str = str(index)
            channel_number: int = int(row["Total Nb Channel"])
            freq_band_start: float = float(row["Freq band start (MHz)"])
            freq_band_end: float = float(row["Freq band end (MHz)"])

            frequency_dict[key] = {
                'channel_number': channel_number,
                'freq_band_start': freq_band_start,
                'freq_band_end': freq_band_end
            }
        return frequency_dict

    def generate_frequency_channel_fan1v0(self) -> dict[str, dict]:
        """
        Generate frequency channel dictionary from the frequency DataFrame for FAN 1.0
        :return: dictionary with frequency parameters as values and <Region abrev>_<ChPlanID> as keys
        """
        if self.freq is None:
            self._create_frequency_channel()

        freq = self.freq[self.freq['FAN 1.0'].notna()].drop(columns=['FAN 1.0', 'FAN 1.1 (23Q4)'])
        freq["id"] = freq["Region abrev"] + "_" + freq["Operating class"].astype(str)
        freq = freq.drop(columns=["Region abrev", "ChPlanID", 'Operating class'])
        freq.set_index("id", inplace=True)
        freq = freq.sort_index()
        return self._calculate_frequency_params(freq)

    def generate_frequency_channel_fan1v1(self) -> dict[str, dict]:
        """
        Generate frequency channel dictionary from the frequency DataFrame for FAN 1.1
        :return: dictionary with frequency parameters as values and <Region abrev>_<ChPlanID> as keys
        """
        if self.freq is None:
            self._create_frequency_channel()

        freq = self.freq[self.freq['FAN 1.1 (23Q4)'].notna()].drop(columns=['FAN 1.0', 'FAN 1.1 (23Q4)'])
        freq["id"] = freq["Region abrev"] + "_" + freq["ChPlanID"]
        freq = freq.drop(columns=["Region abrev", "ChPlanID", 'Operating class'])
        freq.set_index("id", inplace=True)
        freq = freq.sort_index()

        return self._calculate_frequency_params(freq)
