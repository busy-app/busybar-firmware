from siliconlabs.slc.board_gen.util.uart_util import *
from typing import Set

cpc_primary_uart = UartOverrideUtil('usart', ignore_instances=['vcom'])

def compatible(provides: Set[str], board: Hardware) -> List[str]:
    return cpc_primary_uart.compatible(board)

def configure(project: Project_Config, board: Hardware, instance_name: str):
    cpc_primary_uart.configure(project, instance_name, 'SL_CPC_DRV_UART_{}'.format(instance_name.upper()))
