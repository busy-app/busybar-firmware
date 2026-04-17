from siliconlabs.slc.board_gen.util.uart_util import *
from typing import Set

special_case = ['brd2608a', 'brd2710a']

cpc_secondary_uart = UartOverrideUtil('eusart')

def compatible(provides: Set[str], board: Hardware) -> List[str]:
    return cpc_secondary_uart.compatible(board)

def configure(project: Project_Config, board: Hardware, instance_name: str):
    cpc_secondary_uart.configure(project, instance_name, 'SL_CPC_DRV_UART_{}'.format(instance_name.upper()))

    config_prefix = 'SL_CPC_DRV_UART_{}'.format(instance_name.upper())
    is_special_case = any(board.provides(brd) for brd in special_case)
    if is_special_case:
        flow_control_type = 'eusartHwFlowControlNone'
        project.config(config_prefix + '_FLOW_CONTROL_TYPE').value = flow_control_type
