from fetcher import Fetcher
from pathlib import Path
from excel_processor import ExcelProcessor
from templates.wisun_templates import Wisun1v0Templates, Wisun1v1Templates


class Generator:
    """
    The Generator class automates the process of updating a submodule, processing Excel data,
    generating lookup tables (LUTs) for different Wi-SUN FAN versions, and saving the results
    to Python files. It dynamically locates the repository root, manages file paths, and
    coordinates the use of supporting classes for data fetching and template rendering.
    """
    def __init__(self):
        self.TAG = "v1.2.0"
        self.PATH = Generator._create_path()
        self.lut_wisun_fan1v0: str = ""
        self.lut_wisun_fan1v1: str = ""

    @staticmethod
    def _create_path() -> Path:
        """
        Locates the repository root by searching for a `.git` directory upwards from the current file,
        then appends the `Package/wisun_phydefs` subdirectory to the path.
        Returns:
            Path: The full path to the `wisun_phydefs` directory within the repository.
        """
        def find_repo_root(start_path: Path = Path(__file__).resolve()) -> Path:
            for parent in [start_path] + list(start_path.parents):
                if (parent / '.git').exists():
                    return parent
            raise FileNotFoundError("Repository root with .git not found.")

        wisun = 'Package/wisun_phydefs'
        repo_root = find_repo_root()
        return Path(repo_root / wisun)

    def update_submodule(self):
        """
        Updates the submodule at the specified path to the given tag using the Fetcher class.
        """
        fetcher = Fetcher(self.PATH)
        fetcher.update_submodule(tag=self.TAG)

    def process_excel(self):
        """
        Processes the Excel file using ExcelProcessor to generate modulation and frequency dictionaries,
        then generates LUTs for both Wisun 1v0 and 1v1.
        """
        processor = ExcelProcessor(self.PATH)
        modulation_dict = processor.generate_modulation()
        freq_dict_fan1v0 = processor.generate_frequency_channel_fan1v0()
        freq_dict_fan1v1 = processor.generate_frequency_channel_fan1v1()
        # Generate Wisun1v0 LUTs
        self._generate_wisun1v0_luts(modulation_dict, freq_dict_fan1v0)
        # Generate Wisun1v1 LUTs
        self._generate_wisun1v1_luts(modulation_dict, freq_dict_fan1v1)

    def _generate_wisun1v0_luts(self, modulation_dict: dict[str,dict], freq_dict_fan1v0: dict[str,dict]):
        """
        Generates LUTs for Wisun 1v0 using the provided modulation and frequency dictionaries,
        and stores the result in `self.lut_wisun_fan1v0`.
        Args:
            modulation_dict (dict): Modulation parameters.
            freq_dict_fan1v0 (dict): Frequency channel parameters for Wisun 1v0.
        """
        lut = Wisun1v0Templates.import_and_class
        lut += Wisun1v0Templates.bitrate_modulation_start
        for mode, params in modulation_dict.items():
            if 'opt' in mode:
                continue
            lut += Wisun1v0Templates.bitrate_modulation(
                mode=f"Mode{mode}",
                modulation_index=params['modulation_index'],
                bitrates=params['bitrates'],
            )
        lut += Wisun1v0Templates.close
        lut += Wisun1v0Templates.frequency_channel_start
        for key, params in freq_dict_fan1v0.items():
            lut += Wisun1v0Templates.frequency_channel(
                key=key,
                channel_number=params['channel_number'],
                freq_band_start=params['freq_band_start'],
                freq_band_end=params['freq_band_end']
            )
        lut += Wisun1v0Templates.close
        self.lut_wisun_fan1v0 = lut

    def _generate_wisun1v1_luts(self, modulation_dict: dict[str, dict], freq_dict_fan1v1: dict[str, dict]):
        """
        Generates LUTs for Wisun 1v1 using the provided modulation and frequency dictionaries,
        and stores the result in `self.lut_wisun_fan1v1`.
        Args:
            modulation_dict (dict): Modulation parameters.
            freq_dict_fan1v1 (dict): Frequency channel parameters for Wisun 1v1.
        """
        lut = Wisun1v1Templates.import_and_class
        lut += Wisun1v1Templates.bitrate_modulation_start
        for mode, params in modulation_dict.items():
            lut += Wisun1v1Templates.bitrate_modulation(
                mode=mode,
                modulation_index=params['modulation_index'],
                bitrates=params['bitrates'],
                restriction=params['restrictions']
            )
        lut += Wisun1v1Templates.close
        lut += Wisun1v1Templates.frequency_channel_start
        for key, params in freq_dict_fan1v1.items():
            lut += Wisun1v1Templates.frequency_channel(
                key=key,
                channel_number=params['channel_number'],
                freq_band_start=params['freq_band_start'],
                freq_band_end=params['freq_band_end']
            )
        lut += Wisun1v1Templates.close
        self.lut_wisun_fan1v1 = lut

    def save(self):
        """
        Saves the generated LUTs to files in the `lut_representation` directory,
        creating the directory if it does not exist.
        """
        directory = Path.cwd().parent / 'lut_representation'
        fan1v0_path = directory / 'wisun_1v0_luts.py'
        fan1v1_path = directory / 'wisun_1v1_luts.py'

        directory.mkdir(parents=True, exist_ok=True)
        with open(fan1v0_path, 'w') as f:
            f.write(self.lut_wisun_fan1v0)
        with open(fan1v1_path, 'w') as f:
            f.write(self.lut_wisun_fan1v1)
        print(f"Files written to {fan1v0_path} and {fan1v1_path}")

    def run(self):
        """
        Runs the full generation process: updates the submodule, processes the Excel file,
        generates LUTs, and saves them to files.
        """
        print("Updating submodule...")
        self.update_submodule()
        print("Processing excel...")
        self.process_excel()
        print("Writing files...")
        self.save()

if __name__ == "__main__":
    generator = Generator()
    generator.run()