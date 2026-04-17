from siliconlabs.slc.board_gen.util.board_gen_util import Req

def compatible(provides, board):
    if board.has_component('pti_0') and board.get_peripheral_options(Req('pti'), 'pti_0', {'data': 'DOUT'}):
        return True
    if board.has_component('pti') and board.get_peripheral_options(Req('pti'), 'pti', {'data': 'DOUT'}):
        return True

    return False

def configure(project, board, instance_name):
    req = project.requirement('SL_RAIL_UTIL_PTI')
    board_map = {
        'data': 'DOUT',
        'sync': '(DFRAME)',
    }
    pti_name = 'pti_0' if board.has_component('pti_0') else 'pti'
    signal_options = board.get_peripheral_options(req, pti_name, board_map)

    # Select first mathing peripheral
    locations = signal_options[0]

    locs = []
    for b_signal in board_map.keys():
        if b_signal in locations.keys():
            locs.append(locations[b_signal]['locations'][0])

    project.satisfy_requirement(req, locs)

    # Perhaps support SPI mode in the future
    # if 'clk' in locations.keys():
    #     mode = 'SL_RAIL_PTI_MODE_SPI'
    # elif 'sync' in locations.keys():
    if 'sync' in locations.keys():
        mode = 'SL_RAIL_PTI_MODE_UART'
    else:
        mode = 'SL_RAIL_PTI_MODE_UART_ONEWIRE'

    project.config('SL_RAIL_UTIL_PTI_MODE').value = mode
