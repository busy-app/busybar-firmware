from siliconlabs.slc.board_gen.util.board_gen_util import *
from typing import Set

# List of boards where flow control of board controller is disabled by default
special_case = ['brd2608a', 'brd2710a']

euart_signals_map = {
  Instance.EXP: [
    {
      '12': 'TX',
      '14': 'RX',
      '3': '(CTS)',
      '5': '(RTS)',
    },
    {
      '4': 'TX',
      '6': 'RX',
      '8': '(CTS)',
      '10': '(RTS)',
    },
  ],
  Instance.MIKROE: {
    'uart_tx': 'TX',
    'uart_rx': 'RX',
  },
  Instance.EXP2: {
    '11': 'TX',
    '13': 'RX',
  },
  Instance.VCOM: {
    'rx_cipo': 'RX',
    'tx_copi': 'TX',
    'cts_n_clk': '(CTS)',
    'rts_n_cs_n': '(RTS)',
  },
}

uartdrv_eusart = OverrideUtil(euart_signals_map, 'eusart')


def compatible(provides: Set[str], board: Hardware) -> List[str]:
  return uartdrv_eusart.compatible(board)


def configure(project, board, instance_name):
  config_prefix = f'SL_UARTDRV_EUSART_{instance_name.upper()}'
  locations = uartdrv_eusart.configure(project, instance_name, config_prefix)

  is_special_case = any(board.provides(brd) for brd in special_case)

  #  Set flow controly type depending on available options
  flow_control_type = 'uartdrvFlowControlNone'
  if not is_special_case and 'cts_n_clk' in locations.keys() and 'rts_n_cs_n' in locations.keys():
    flow_control_type = 'uartdrvFlowControlHwUart'

  project.config(config_prefix + '_FLOW_CONTROL_TYPE').value = flow_control_type
