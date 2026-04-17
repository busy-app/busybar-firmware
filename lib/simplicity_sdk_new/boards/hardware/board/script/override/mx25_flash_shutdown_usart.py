from siliconlabs.slc.board_gen.util.board_gen_util import Req

def compatible(provides, board):
    if board.has_component('spiflash'):
        # check if all options solves
        if board.get_peripheral_options(Req('usart'), 'spiflash', {'copi' : 'TX', 'cipo' : 'RX', 'clk' : 'CLK'}):
            if board.get_peripheral_options(Req('gpio'), 'spiflash', {'cs_n' : None}):
                return 'spiflash'
    return False

def configure(project, board, instance_name):
    component = 'spiflash'
    req = project.requirement('SL_MX25_FLASH_SHUTDOWN')
    cs_req = project.requirement('SL_MX25_FLASH_SHUTDOWN_CS')

    board_map = {
        'copi'      :   'TX',
        'cipo'      :   'RX',
        'clk'       :   'CLK'
    }

    signal_options = board.get_peripheral_options(req, 'spiflash', board_map)
    # Select first matching peripheral
    locations = select_peripheral_instance(board, signal_options)
    locs = []
    for b_signal in board_map.keys():
        if b_signal in locations.keys():
            locs.append(locations[b_signal]['locations'][0])
    project.satisfy_requirement(req, locs)
    options = board.get_peripheral_options(cs_req, 'spiflash', {'cs_n' : None})
    pin = options[0]['cs_n']['pins'][0]
    project.satisfy_requirement(cs_req, [pin])

    # MX25 USART requirement should not be loaded in the Pin Tool GUI in Studio,
    # as this would effectively block usage of this USART for anything else,
    # since the MX25 driver is always present as part of board init.
    req.set_noload()


def select_peripheral_instance(board, options):
    # Default to the first match
    return options[0]
