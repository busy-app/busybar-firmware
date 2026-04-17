from siliconlabs.slc.board_gen.util.board_gen_util import *
from typing import Set

uart_signals_map = {
  Instance.EXP: [
    {
      '12': 'TX',
      '14': 'RX',
    },
    {
      '4': 'TX',
      '6': 'RX',
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

# List of boards where flow control of board controller is disabled by default
special_case = ['brd2608a', 'brd2710a']

iostream_usart = OverrideUtil(uart_signals_map, 'usart')


def compatible(provides: Set[str], board: Hardware) -> List[str]:
  return iostream_usart.compatible(board)


def configure(project: Project_Config, board: Hardware, instance_name: str):
  config_prefix = f'SL_IOSTREAM_USART_{instance_name.upper()}'
  locations = iostream_usart.configure(project, instance_name, config_prefix)

  is_special_case = any(board.provides(brd) for brd in special_case)

  flow_control_type = 'usartHwFlowControlNone'
  if board.provides('device_has_radio') and not board.provides('device_supports_zwave') and not is_special_case:
    # Only use flow control on radio boards
    if 'cts_n_clk' in locations.keys() and 'rts_n_cs_n' in locations.keys():
      flow_control_type = 'usartHwFlowControlCtsAndRts'
    elif 'cts_n_clk' in locations.keys():
      flow_control_type = 'usartHwFlowControlCts'
    elif 'rts_n_cs_n' in locations.keys():
      flow_control_type = 'usartHwFlowControlRts'

  project.config(config_prefix + '_FLOW_CONTROL_TYPE').value = flow_control_type
