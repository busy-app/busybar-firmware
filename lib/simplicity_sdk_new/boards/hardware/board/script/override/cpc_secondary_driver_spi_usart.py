from siliconlabs.slc.board_gen.util.spi_util import *
from siliconlabs.slc.board_gen.util.gpio_util import *
from siliconlabs.slc.board_gen.hardware import Hardware
from siliconlabs.slc.board_gen.project_config import ProjectConfig
from typing import Set, List

gpio_signals_map = {
    Instance.EXP: [
        '7', '9', '11', '13', '15',
    ],
    Instance.MIKROE: [
        'int'
    ]
}

cpc_secondary_spi = SpiOverrideUtil('usart')
cpc_secondary_spi_gpio = GpioOverrideUtil(gpio_signals_map)

def compatible(provides: Set[str], board: Hardware) -> List[str]:
    spi_compatible_instances =  cpc_secondary_spi.compatible(board)
    gpio_compatible_instances = cpc_secondary_spi_gpio.compatible(board)
    return list(set(spi_compatible_instances).intersection(gpio_compatible_instances))

def configure(project: ProjectConfig, board: Hardware, instance_name: str):
    cpc_secondary_spi.configure(project, instance_name, 'SL_CPC_DRV_SPI_{}'.format(instance_name.upper()),
                                                         select_peripheral_instance,
                                                         cpc_secondary_spi)
    cpc_secondary_spi_gpio.configure(project, instance_name, 'SL_CPC_DRV_SPI_{}_IRQ'.format(instance_name.upper()))


# Filter function to select matching peripheral
def select_peripheral_instance(cpc_secondary_spi: SpiOverrideUtil):
    # Default to the first match
    return cpc_secondary_spi.signals_opt[0]
