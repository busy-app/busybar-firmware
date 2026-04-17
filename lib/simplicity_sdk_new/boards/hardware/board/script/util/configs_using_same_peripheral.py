import re
import argparse
from collections import defaultdict
from pathlib import Path
from typing import Optional, Tuple

override_signals = {
  'CC0': 'OUTPUT',  # CC0 and OUTPUT are aliases for TIMER peripherals
}

ignore_signals = [
  'CS',      # CS Pin will not cause conflicts
  'ENABLE',  # Alias for CS in some scripts
  'RX',      # Making an assumption that Tx and Rx have the same conflicts, so only one is required
  'CTS',     # Making an assumption that RTS and CTS have the same conflicts, so only one is required
]

ignore_peripherals_per_sdid = {
  200: [],          # Panther
  205: ['EUART0'],  # Lynx only has 1 EUART so conflicts are inevitable
  210: ['USART0'],  # Ocelot only has 1 USART so conflicts are inevitable
  215: ['USART0'],  # Bobcat only has 1 USART so conflicts are inevitable
  220: [],          # Sol
  225: [],          # Caracal
  230: ['EUSART0'], # Leopard only has 1 EUSART so conflicts are inevitable
  235: ['USART0'],  # Margay only has 1 USART so conflicts are inevitable
  240: [],          # Lion
  260: ['USART0'],  # EFR32FG2D only has 1 USART so conflicts are inevitable
}


def get_device_sdid(gsdk_path: Path, board_name: str) -> Optional[int]:
  """
  This function gets the SDID for a given board by opening its device component file and fetching SDID from the tags
  """
  try:
    with open((gsdk_path / 'hardware/board/component') / f'{board_name}.slcc') as board_file:
      content = board_file.read()

      # Try getting Board device from board component tags
      device_match = re.search(r'board:device:(\w+)\n', content)

      if device_match is None:
        # If board device not found in the component files, this board is split into revisions.
        # So get the recommended revision.
        device_match = re.search(rf'id: ({board_name}_\w\d+)\n', content)
        if device_match is None:
          return None

        board_revision = device_match.group(1)
        with open((gsdk_path / 'hardware/board/component') / f'{board_revision}.slcc') as board_revision_file:
          # Get Board device from the revisioned board component
          content = board_revision_file.read()
          device_match = re.search(r'board:device:(\w+)\n', content)

  except FileNotFoundError:
    return None

  device_name = device_match.group(1)

  if (gsdk_path / 'platform/Device/component' / f'{device_name}.slcc').exists():
    component_file = (gsdk_path / 'platform/Device/component' / f'{device_name}.slcc')
  elif (gsdk_path / 'platform/Device/component-internal' / f'{device_name}.slcc').exists():
    component_file = (gsdk_path / 'platform/Device/component-internal' / f'{device_name}.slcc')
  else:
    return None
 
  with open(component_file) as device_file:
    # Get the Device SDID from the Device component file tags
    content = device_file.read()
    sdid_match = re.search(r'name: device_sdid_(\d+)\n', content)

  if sdid_match is None:
    return None

  return int(sdid_match.group(1))


def get_peripheral_signals(config_file_path: Path) -> Tuple[Optional[str], Optional[dict]]:
  """
  This function parses a give Config file to find which Peripheral is used by the Config header and what pins are used
  by the peripheral.
  """
  with open(config_file_path) as config_file:
    content = config_file.read()

  # Matches something like `#define SL_CPC_DRV_SPI_EXP_PERIPHERAL            USART2`
  # To header = `SL_CPC_DRV_SPI_EXP_` and peripheral_name = `USART2`
  peripheral_match = re.search(r'#define\s+(\w+)_PERIPHERAL\s+(\w+)', content)

  if peripheral_match is None:
    # This Config file uses no Peripherals
    return None, None

  header = peripheral_match.group(1)
  peripheral_name = peripheral_match.group(2)

  # Using the header, we can now find the Ports for the different signals the peripheral might have

  # Matches something like `#define SL_CPC_DRV_SPI_EXP_TX_PORT               SL_GPIO_PORT_C`
  # to signal = `TX` and port = `C`
  ports = re.findall(rf'#define\s+{header}_(\w+)_PORT\s+SL_GPIO_PORT_(\w)', content)

  # Remove signals that should be ignored
  ports = [port for port in ports if port[0] not in ignore_signals]

  gpios = {}
  for signal, port in ports:
    # For each signal and port pair, find the relevant pin.

    # Matches something like `#define SL_CPC_DRV_SPI_EXP_TX_PIN                0`
    # to pin = `0`
    pin = re.search(rf'#define\s+{header}_{signal}_PIN\s+(\w+)', content)

    if signal in override_signals:
      signal = override_signals[signal]

    # Combine the port and pins to look something like `PC0`
    gpios[signal] = f'P{port}{pin.group(1)}'

  return peripheral_name, gpios


def main():
  parser = argparse.ArgumentParser()

  parser.add_argument('gsdk_path', type=Path, help='Path to gsdk')
  parser.add_argument('-b', '--boards', required=False, help='Comma separated list of boards to check')

  args = parser.parse_args()

  device_sdid_per_board = {}
  board_peripheral_pins = defaultdict(lambda: defaultdict(lambda: defaultdict(dict)))

  for config_file_path in (args.gsdk_path / 'hardware/board/config').rglob('**/*.h'):
    board_name = config_file_path.parent.stem.split('_')[0]

    if args.boards is not None and board_name not in args.boards:
      # If we were specified to only check a few boards, skip the other boards
      continue

    device_sdid = get_device_sdid(args.gsdk_path, board_name)

    print(f"[DEBUG] Processing board '{board_name}', device_sdid={device_sdid}")

    if device_sdid is None:
        print(f"[DEBUG] Skipping '{board_name}' — device_sdid is None")
      # The generic family wasn't found
        continue
    elif not device_sdid >= 200:
      print(f"[DEBUG] Skipping '{board_name}' — Series 1 device (sdid={device_sdid})")
      # For now, we're only testing Series 2 devices
      continue

    # Keep track of SDIDs per board
    device_sdid_per_board[board_name] = device_sdid

    peripheral, gpios = get_peripheral_signals(config_file_path)


    print(f"[DEBUG] Peripheral '{peripheral}' found for board '{board_name}'")


    if device_sdid not in ignore_peripherals_per_sdid:
      print(f"[DEBUG] SDID {device_sdid} not in ignore_peripherals_per_sdid — skipping ignore check")
    elif peripheral in ignore_peripherals_per_sdid[device_sdid]:
      print(f"[DEBUG] Skipping peripheral '{peripheral}' for SDID {device_sdid} — in ignore list")
      continue

    # # Ignore certain peripherals for certain SDIDs
    # if peripheral in ignore_peripherals_per_sdid[device_sdid]:
    #   continue

    # If there are no signals on the peripheral, continue
    if gpios is None:
        print(f"[DEBUG] Skipping peripheral '{peripheral}' — no GPIOs found")
        continue

    # Keep track of all the peripherals used by the boards and which config file uses them
    board_peripheral_pins[board_name][peripheral][config_file_path.stem] = gpios

  # At this point, the dict board_peripheral_pins looks something like:
  '''
  {
    "brd2503a": {
      "USART0": {
        'btl_spi_controller_usart_driver_cfg': { 'CLK': 'PC2',  'TX': 'PC0' },
        'iot_flash_cfg_exp': { ... },
        ...
      },
      "I2C1": { ... },
      "TIMER0": { ... },
      ...
    },
    "brd4108a": {
      ...
    },
    "brd4108b": {
      ...
    },
    ...
  }
  '''

  for board_name in board_peripheral_pins:
    peripherals = board_peripheral_pins[board_name]
    conflicts_by_peripheral = defaultdict(list)

    for peripheral in peripherals:

      # Get a list of all the signals that the given peripheral is using for the board
      # For example, if cpc uses RX,TX, uartdrv uses RX,TX,RTS,CTS and memlcd uses TX,CLK,
      # this returns RX,TX,RTS,CTS,CLK as all_signals
      all_signals = set.union(*[set(peripherals[peripheral][config]) for config in peripherals[peripheral]])

      for signal in all_signals:
        # Filter out the specific configs that use one particular signal
        filtered_configs = dict(filter(lambda pair: signal in pair[1], peripherals[peripheral].items()))

        # Group the configs by which gpio pin they use for the specific signal
        configs_group_by_gpio = defaultdict(list)
        for config in filtered_configs:
          configs_group_by_gpio[filtered_configs[config][signal]].append(config)

        # If more than one gpio is used for a signal, there are conflicts
        if len(configs_group_by_gpio) > 1:

          # Change how the conflicts are represented so that they can be properly formatted on stash
          common_configs_repr = [f'[_{"_, _".join(config)}_]' for config in configs_group_by_gpio.values()]

          # Note down the conflict for processing later
          conflicts_by_peripheral[peripheral].append(
            {
              'signal': signal,
              'configs': common_configs_repr
            }
          )

    # If there are conflicts for the board, print them in a format so that it renders markdown on stash
    if len(conflicts_by_peripheral):
      # Print the Board name as a header 3
      print('###', f'{board_name} (SDID: {device_sdid_per_board[board_name]})')
      for peripheral in conflicts_by_peripheral:
        # The peripheral name is the first nested list
        print(' ' * 0, '*', peripheral)
        for conflict in conflicts_by_peripheral[peripheral]:
          # As the second nested list, print which signal causes the conflict
          print(' ' * 2, '*', f'Conflicts due to **{conflict["signal"]}** between')
          for common_configs in conflict['configs']:
            # As the innermost list, print the different configs that use the same gpio for given signal
            print(' ' * 4, '*', common_configs)
      print('')


if __name__ == '__main__':
  main()
