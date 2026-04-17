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

cpc_secondary_spi = SpiOverrideUtil('eusart')
cpc_secondary_spi_gpio = GpioOverrideUtil(gpio_signals_map)

def compatible(provides: Set[str], board: Hardware) -> List[str]:
    spi_compatible_instances =  cpc_secondary_spi.compatible(board)
    gpio_compatible_instances = cpc_secondary_spi_gpio.compatible(board)
    return list(set(spi_compatible_instances).intersection(gpio_compatible_instances))

def configure(project: ProjectConfig, board: Hardware, instance_name: str):
    locations = cpc_secondary_spi.configure(project, instance_name, 'SL_CPC_DRV_SPI_{}'.format(instance_name.upper()))
    cpc_secondary_spi_gpio.configure(project, instance_name, 'SL_CPC_DRV_SPI_{}_IRQ'.format(instance_name.upper()))

    if 'spi_cs_n' in locations:
        cs_pin_location = locations['spi_cs_n']['locations'][0]
    elif '10' in locations:
        cs_pin_location = locations['10']['locations'][0]
    else:
        raise Exception('CS Pin not found')

    cs_pin = int(cs_pin_location.pin.index)
    # Don't save config file if the CS pin location is not between 0 to 7
    if cs_pin > 7:
        return False
    else:
        project.config(f'SL_CPC_DRV_SPI_{instance_name.upper()}_CS_EXTI_NUMBER').value = cs_pin_location.pin.index
