import argparse
import re
import os
import yaml
import tabulate
from pathlib import Path
from typing import List, Optional

if not 'GSDK_DIR' in os.environ:
  raise Exception('Please Add GSDK_DIR to your environment.')

gsdk_path = Path(os.environ['GSDK_DIR'])

# hardware/board paths
hardware_board = gsdk_path / 'hardware/board/'
hardware_config = hardware_board / 'config'
board_component = hardware_board / 'component'

# platform/Device paths
platform_device = gsdk_path / 'platform/Device'
device_component = platform_device / 'component'


class Board:
  def __init__(self, board_name: str):
    self.board_name: str = board_name.lower()

    self.board_slcc: Path = self.get_board_slcc()
    if self.board_slcc is None:
      raise ValueError(f'No SLCC file found for {self.board_name}')

    with open(self.board_slcc) as board_file:
      self.board_content: str = board_file.read()
      self._board_data: Optional[dict] = None

    self.device_opn: str = self.get_device_opn()

    self.device_slcc: Optional[Path] = None
    self.device_content: str = ''
    self._device_data: Optional[dict] = None
    if (device_component / f'{self.device_opn}.slcc').is_file():
      self.device_slcc = device_component / f'{self.device_opn}.slcc'
      with open(self.device_slcc) as device_file:
        self.device_content = device_file.read()

  @property
  def device_data(self):
    if self._device_data is None:
      self._device_data = yaml.safe_load(self.device_content)
    return self._device_data

  @property
  def board_data(self):
    if self._board_data is None:
      self._board_data = yaml.safe_load(self.board_content)
    return self._board_data

  @staticmethod
  def board_regex_type(board_name):
    if re.match(r'brd\d{4}\w', board_name):
      board_name = board_name
    elif re.match(r'\d{4}\w', board_name):
      board_name = f'brd{board_name}'
    else:
      raise argparse.ArgumentTypeError("Incorrect board name")

    if not (board_component / f'{board_name}.slcc').is_file():
      raise argparse.ArgumentTypeError("No such board found")

    return board_name

  def get_board_slcc(self) -> Optional[Path]:
    try:
      with open(board_component / f'{self.board_name}.slcc') as board_file:
        content = board_file.read()
        match = re.search(r'board:device:(\w+)\n', content)
        if match is None:
          match = re.search(rf'id: ({self.board_name}_\w\d+)\n', content)
          if match is None:
            return None
          board_revision = match.group(1)
          return board_component / f'{board_revision}.slcc'
        else:
          return board_component / f'{self.board_name}.slcc'
    except FileNotFoundError:
      return None

  def get_device_opn(self) -> str:
    match = re.search(r'board:device:(\w+)\n', self.board_content)
    device_name = match.group(1)

    return device_name

  def get_device_sdid(self) -> Optional[int]:
    match = re.search(r'name: device_sdid_(\d+)\n', self.device_content)
    if match is None:
      return None

    return int(match.group(1))

  def get_device_series(self) -> Optional[int]:
    match = re.search(r'name: device_series_(\d+)\n', self.device_content)
    if match is None:
      return None

    return int(match.group(1))

  def get_device_cortex(self) -> Optional[str]:
    match = re.search(r'name: cortex(\w+)\n', self.device_content)

    if match is None:
      return None

    return match.group(1)

  def get_board_provides(self) -> List[str]:
    board_provides = list(filter(lambda item: item[0] == 'provides', self.board_data))[0][1]

    provides = list(map(lambda x: x['name'], board_provides))

    return provides

  def get_device_provides(self) -> List[str]:
    if self.device_data is None:
      return []
    device_provides = list(filter(lambda item: item[0] == 'provides', self.device_data))[0][1]

    provides = list(map(lambda x: x['name'], device_provides))

    return provides

  def get_device_ram_size(self) -> Optional[int]:
    if self.device_data is None:
      return None
    device_metadata = list(filter(lambda item: item[0] == 'template_contribution', self.device_data))[0][1]

    ram_size = list(filter(lambda item: item['name'] == 'device_ram_size', device_metadata))[0]['value']

    return ram_size

  def get_device_flash_size(self) -> Optional[int]:
    if self.device_data is None:
      return None
    device_metadata = list(filter(lambda item: item[0] == 'template_contribution', self.device_data))[0][1]

    flash_size = list(filter(lambda item: item['name'] == 'device_flash_size', device_metadata))[0]['value']

    return flash_size

  def get_device_page_size(self) -> Optional[int]:
    if self.device_data is None:
      return None
    device_metadata = list(filter(lambda item: item[0] == 'metadata', self.device_data))[0]

    memory_info = device_metadata[1]['device']['memory']
    page_size = list(filter(lambda item: 'page_size' in item, memory_info))[0]['page_size']
    return page_size

  def get_board_buttons(self) -> dict:
    buttons = {}

    board_config_folder = (hardware_config / f'{self.board_name}')
    if not board_config_folder.exists():
      board_config_folder = (hardware_config / f'{self.board_name}_brd4002a')
    if not board_config_folder.exists():
      return buttons

    for button_config_file_name in board_config_folder.glob(f'sl_simple_button_btn*_config.h'):
      with open(button_config_file_name) as button_config_file:
        content = button_config_file.read()
      ports = re.findall(rf'#define\s+SL_SIMPLE_BUTTON_BTN(\d+)_PORT\s+SL_GPIO_PORT_(\w)', content)
      for button_no, port in ports:
        pin = re.search(rf'#define\s+SL_SIMPLE_BUTTON_BTN{button_no}_PIN\s+(\d)', content)
        buttons[button_no] = f'P{port}{pin.group(1)}'

    return buttons


def parse_arguments() -> argparse.Namespace:
  parser = argparse.ArgumentParser(
    description='Generic board-related helper functions',
  )

  parser.add_argument('-b', '--board', required=False, action='append', type=Board.board_regex_type,
                      help='Run command for specific boards. If not specified, all boards are selected. Can be given multiple times.')
  parser.add_argument('--opn', required=False, action='store', type=str, nargs='?', const='',
                      help='Print the OPNs inside the boards. A regex can be given instead to list all boards with matching OPNs.')
  parser.add_argument('--sdid', required=False, action='store', type=int, nargs='?', const=0,
                      help='Print the SDIDs inside the boards. One SDID can be specified to filter on it.')
  parser.add_argument('--series', required=False, action='store', type=int, nargs='?', const=-1,
                      help='Print the series information of the boards. One Series can be specified to filter on it.')
  parser.add_argument('--cortex', required=False, action='store', type=str, nargs='?', const='',
                      help='Print the cortex version used inside the boards. A regex can be given instead to filter boards on cortex version.')
  parser.add_argument('--ram', required=False, action='store', type=str, nargs='?', const='',
                      help='Print the RAM size of the boards. A filter can optionally be provided to filter based on ram size. '
                           'For example, ">32k" or "<1024".')
  parser.add_argument('--flash', required=False, action='store', type=str, nargs='?', const='',
                      help='Print the Flash size of the boards. A filter can optionally be provided to filter based on flash size. '
                           'For example, ">32k" or "<1024".')
  parser.add_argument('--page-size', required=False, action='store', type=str, nargs='?', const='',
                      help='Print the Flash page size of the boards. A filter can optionally be provided to filter based on ram size. '
                           'For example, ">2k" or "<1024".')
  parser.add_argument('--provides', required=False, action='append',
                      help='Filter boards that provide the given regex.')
  parser.add_argument('--hfxo', required=False, action='store', type=float, nargs='?', const=-1,
                      help='Print the hfxo freq of the boards. One freq can be specified to filter on it.')
  parser.add_argument('--buttons', required=False, action='store', type=str, nargs='?', const='',
                      help='Print the Flash page size of the boards.')

  return parser.parse_args()


def get_all_boards() -> List[str]:
  for component_path in board_component.glob('brd?????.slcc'):
    yield component_path.stem


def main(args: argparse.Namespace):
  if not args.board:
    args.board = get_all_boards()

  full_table = []
  for board_name in args.board:
    board_info = {'Board': board_name}
    try:
      board = Board(board_name)
    except ValueError:
      continue

    # OPN filtering
    if args.opn is not None:
      opn = board.device_opn
      if args.opn != '' and (opn is None or not re.search(args.opn, opn)):
        continue
      board_info['OPN'] = opn

    # SDID filtering
    if args.sdid is not None:
      sdid = board.get_device_sdid()
      if args.sdid != 0 and sdid != args.sdid:
        continue
      board_info['SDID'] = sdid

    # Series filtering
    if args.series is not None:
      series = board.get_device_series()
      if args.series != -1 and series != args.series:
        continue
      board_info['Series'] = series

    # ARM-Cortex filtering
    if args.cortex is not None:
      cortex = board.get_device_cortex()
      if args.cortex != '' and (cortex is None or not re.search(args.cortex, cortex)):
        continue
      board_info['Cortex'] = cortex

    # Ram size filtering
    if args.ram is not None:
      ram = board.get_device_ram_size()

      if args.ram != '':
        if ram is None:
          continue

        conditions: str = args.ram.strip()
        if not conditions.startswith('ram'):
          # For conditions such as ">1024"
          conditions = f'ram{conditions}'
        conditions = conditions.replace('k', '*1024')  # this makes conditions like "==32k" possible

        result = eval(conditions)
        if not result:
          continue

      if ram is not None:
        board_info['Ram size'] = f'{ram // 1024}k'
      else:
        board_info['Ram size'] = ''

    # Flash size filtering
    if args.flash is not None:
      flash = board.get_device_flash_size()

      if args.flash != '':
        if flash is None:
          continue
        conditions: str = args.flash.strip()
        if not conditions.startswith('flash'):
          # For conditions such as ">1024"
          conditions = f'flash{conditions}'
        conditions = conditions.replace('k', '*1024')  # this makes conditions like "==32k" possible

        result = eval(conditions)
        if not result:
          continue

      if flash is not None:
        board_info['Flash size'] = f'{flash // 1024}k'
      else:
        board_info['Flash size'] = ''

    # Flash page size filtering
    if args.page_size is not None:
      page_size = board.get_device_page_size()

      if args.page_size != '':
        if page_size is None:
          continue

        conditions: str = args.page_size.strip()
        if not conditions.startswith('page_size'):
          # For conditions such as ">1024"
          conditions = f'page_size{conditions}'
        conditions = conditions.replace('k', '*1024')  # this makes conditions like "==1k" possible

        result = eval(conditions)
        if not result:
          continue

      board_info['Flash page size'] = page_size

    # Button filtering
    if args.buttons is not None:
      buttons = board.get_board_buttons()
      board_info['Buttons'] = buttons

    # Provides filtering
    if args.provides:
      board_provides = board.get_board_provides()
      device_provides = board.get_device_provides()
      provides = board_provides + device_provides

      if not all([any(re.search(provide, board_provide) for board_provide in provides) for provide in args.provides]):
        continue

    # Get HFXO Freq
    if args.hfxo is not None:
      board_provides = board.get_board_provides()

      hfxo_freq_provide = filter(lambda provide: re.match(r'hardware_board_has_hfxo_freq_[\d.]+mhz', provide),
                                 board_provides)
      hfxo_freqs = list(map(
        lambda provide: float(re.match(r'hardware_board_has_hfxo_freq_([\d.]+)mhz', provide).group(1)),
        hfxo_freq_provide
      ))

      if (args.hfxo != -1 and args.hfxo not in hfxo_freqs) or len(hfxo_freqs) == 0:
        continue

      board_info['HFXO Freq'] = hfxo_freqs

    full_table.append(board_info)

  print(tabulate.tabulate(full_table, headers='keys', tablefmt="github"))


if __name__ == '__main__':
  main(parse_arguments())
