
import siliconlabs.slc.board_gen.board.board as Board
import siliconlabs.slc.board_gen.project_config as Project_Config
import siliconlabs.slc.board_gen.hardware as Hardware

from siliconlabs.slc.board_gen.util.board_gen_util import Req

from typing import List, Dict, Any, Tuple, Set

# NCP SPI works with exp header
brd_component = 'exp_h'

# SPI Pin mapping
spi_signals = {
    '4': 'TX',
    '6': 'RX',
    '8': 'CLK',
    '10': 'CS',
}

# Required GPIO on exp header
required_gpio = {
    '7': 'LEGACY_NCP_SPI_HOST_INT',
    '9': 'LEGACY_NCP_SPI_WAKE_INT'
}


def compatible(provides: Set[str], board: Hardware) -> bool:
    if board.has_component(brd_component):
        # Usart available on exp 4/6/8/10
        if board.get_peripheral_options(Req('usart'), brd_component, spi_signals):
            # Gpio index on expansion header:
            for signal in required_gpio.keys():
                # Not compatible if we don't have all the gpio connected
                if not board.get_peripheral_options(Req('gpio'), brd_component, {signal: None, }):
                    return False

            # USART and both GPIO available on exp header
            return True

    return False

def configure(project: Project_Config, board: Hardware, _):

    # SPI
    req = project.requirement('LEGACY_NCP_SPI')
    signal_options = board.get_peripheral_options(req, brd_component, spi_signals)
    # Select first matching Peripheral
    locations = signal_options[0]
    locs = []

    for b_signal in spi_signals.keys():
        if b_signal in locations.keys():
            locs.append(locations[b_signal]['locations'][0])

    project.satisfy_requirement(req, locs)

    # GPIOs
    for signal, req_name in required_gpio.items():
        req = project.requirement(req_name)
        opts = board.get_peripheral_options(req, brd_component, {signal: None, })
        if opts:
            pin = opts[0][signal]['pins'][0]
            project.satisfy_requirement(req, [pin])
