from siliconlabs.slc.board_gen.util.board_gen_util import *
from typing import Set

# List of boards where flow control of board controller is disabled by default
special_case = ['brd2608a', 'brd2710a']

# Temporarily added signal map for br4180b and brd4180a as exp_h 3 and 5 are not available
uart_signals_map_brd4180 = {
 Instance.EXP: [
    {
      '12': 'TX',
      '14': 'RX',
      '8': '(CTS)',
      '10': '(RTS)',
    }
 ],
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

uart_signals_map = {
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

uartdrv_usart = OverrideUtil(uart_signals_map, 'usart')
uartdrv_usart_brd4180 = OverrideUtil(uart_signals_map_brd4180, 'usart')


def compatible(provides: Set[str], board: Hardware) -> List[str]:
    if board.provides('brd4180b') or board.provides('brd4180a'):
        return uartdrv_usart_brd4180.compatible(board)
    else:
        return uartdrv_usart.compatible(board)


def configure(project, board, instance_name):
  config_prefix = f'SL_UARTDRV_USART_{instance_name.upper()}'
  if board.provides('brd4180b') or board.provides('brd4180a'):
      locations = uartdrv_usart_brd4180.configure(project, instance_name, config_prefix)
  else:
      locations = uartdrv_usart.configure(project, instance_name, config_prefix)

  is_special_case = any(board.provides(brd) for brd in special_case)

  #  Set flow controly type depending on available options
  flow_control_type = 'uartdrvFlowControlNone'
  if not is_special_case and 'cts_n_clk' in locations.keys() and 'rts_n_cs_n' in locations.keys():
    flow_control_type = 'uartdrvFlowControlHwUart'

  project.config(config_prefix + '_FLOW_CONTROL_TYPE').value = flow_control_type
