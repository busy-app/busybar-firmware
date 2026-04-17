from siliconlabs.slc.board_gen.util.spi_util import SpiOverrideUtil
from siliconlabs.slc.board_gen.hardware import Hardware
from siliconlabs.slc.board_gen.project_config import ProjectConfig

from typing import Set, List

spidrv_eusart_override = SpiOverrideUtil('eusart')
def compatible(provides: Set[str], board: Hardware) -> List[str]:
    spi_compatible_instances =  spidrv_eusart_override.compatible(board)
    return spi_compatible_instances

def configure(project: ProjectConfig, board: Hardware, instance_name: str):
    spidrv_eusart_override.configure(project, instance_name, 'SL_SPIDRV_EUSART_{}'.format(instance_name.upper()),
                                     select_peripheral_instance, spidrv_eusart_override)

def select_peripheral_instance(spi_override_util: SpiOverrideUtil):
    for option in spi_override_util.signals_opt:
        # Prefer EUSART1 if available
        if option['peripheral'] == 'EUSART1':
            return option

    # Default to the first match
    return spi_override_util.signals_opt[0]
