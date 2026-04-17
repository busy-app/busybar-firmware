from siliconlabs.slc.board_gen.util.spi_util import SpiOverrideUtil
from siliconlabs.slc.board_gen.hardware import Hardware
from siliconlabs.slc.board_gen.project_config import ProjectConfig
from typing import Set, List

spi_override = SpiOverrideUtil('usart')
def compatible(provides: Set[str], board: Hardware) -> List[str]:
    spi_compatible_instances = spi_override.compatible(board)
    return spi_compatible_instances


def configure(project: ProjectConfig, board: Hardware, instance_name: str):
    spi_override.configure(project, instance_name, 'SL_SPIDRV_USART_{}'.format(instance_name.upper()),
                           select_peripheral_instance, spi_override)


# Filter function to select matching peripheral
def select_peripheral_instance(spi_override_util: SpiOverrideUtil):
    # Default to the first match
    return spi_override_util.signals_opt[0]
