from siliconlabs.slc.board_gen.util.spi_util import *
from siliconlabs.slc.board_gen.util.gpio_util import *
from typing import Set

gpio_signals_map = {
    Instance.EXP: [
        '7', '9', '11', '13', '15',
    ]
}

ot_ncp_spidrv_spi_usart = SpiOverrideUtil('usart')
ot_ncp_spidrv_spi_eusart = SpiOverrideUtil('eusart')
ot_ncp_spidrv_spi_gpio = GpioOverrideUtil(gpio_signals_map)


def compatible(provides: Set[str], board: Hardware) -> List[str]:
    if board.provides("device_series_2"):
        spi_compatible_instances = ot_ncp_spidrv_spi_usart.compatible(board)
    elif board.provides("device_series_3"):
        spi_compatible_instances = ot_ncp_spidrv_spi_eusart.compatible(board)

    gpio_compatible_instances = ot_ncp_spidrv_spi_gpio.compatible(board)

    return list(set(spi_compatible_instances).intersection(gpio_compatible_instances))


def configure(project: Project_Config, board: Hardware, instance_name: str):
    if board.provides("device_series_2"):
        ot_ncp_spidrv_spi_usart.configure(project, "exp", 'SL_NCP_SPIDRV_USART')

        ot_ncp_spidrv_spi_gpio.configure(project, "exp", 'SL_NCP_SPIDRV_USART_HOST_INT')

        # GPIO interrupt numbers:
        # The CS pin is the 4th pin in the requirement signal list (i.e. spi_signals), using this pin number to determine
         # group in which the pin is located
        cs_pin_nr = int(ot_ncp_spidrv_spi_usart.locs[3].pin.index)
        # Interrupt are grouped by 4, select the first compatible interrupt number of the group
        int_no = cs_pin_nr // 4 * 4

        project.config('SL_NCP_SPIDRV_USART_CS_FALLING_EDGE_INT_NO').value = f"{int_no}"
        project.config('SL_NCP_SPIDRV_USART_CS_RISING_EDGE_INT_NO').value = f"{int_no + 1}"

    elif board.provides("device_series_3"):
        ot_ncp_spidrv_spi_eusart.configure(project, "exp", 'SL_NCP_SPIDRV_EUSART')

        ot_ncp_spidrv_spi_gpio.configure(project, "exp", 'SL_NCP_SPIDRV_EUSART_HOST_INT')

        # GPIO interrupt numbers:
        # The CS pin is the 4th pin in the requirement signal list (i.e. spi_signals), using this pin number to determine
        # group in which the pin is located
        cs_pin_nr = int(ot_ncp_spidrv_spi_eusart.locs[3].pin.index)
        # Interrupt are grouped by 4, select the first compatible interrupt number of the group
        int_no = cs_pin_nr // 4 * 4

        project.config('SL_NCP_SPIDRV_EUSART_CS_FALLING_EDGE_INT_NO').value = f"{int_no}"
        project.config('SL_NCP_SPIDRV_EUSART_CS_RISING_EDGE_INT_NO').value = f"{int_no + 1}"
