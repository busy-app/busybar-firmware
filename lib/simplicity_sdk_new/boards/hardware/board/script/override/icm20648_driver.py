from siliconlabs.slc.board_gen.util.board_gen_util import Req

def compatible(provides, board):
    if board.has_component('spisensor_0'):
        if board.get_peripheral_options(Req('usart'), 'spisensor_0', {'copi' : 'TX', 'cipo' : 'RX', 'clk' : 'CLK'}):
            return 'spisensor_0'
    return False

def configure(project, board, instance_name):
    component = 'spisensor_0'
    req = project.requirement('SL_ICM20648_SPI')

    board_map = {
        'copi'      :   'TX',
        'cipo'      :   'RX',
        'clk'       :   'CLK'
    }

    signal_options = board.get_peripheral_options(req, 'spisensor_0', board_map)
    # Select first matching peripheral
    locations = signal_options[0]
    locs = []
    for b_signal in board_map.keys():
        if b_signal in locations.keys():
            locs.append(locations[b_signal]['locations'][0])
    project.satisfy_requirement(req, locs)

    cs_req = project.requirement('SL_ICM20648_SPI_CS')
    options = board.get_peripheral_options(cs_req, 'spisensor_0', {'cs_n' : None})
    #location = options[0]['cs_n']['locations'][0]
    project.satisfy_requirement(cs_req, [options[0]['cs_n']['pins'][0]])

    int_req = project.requirement('SL_ICM20648_INT')
    options = board.get_peripheral_options(int_req, 'spisensor_0', {'int' : None})
    project.satisfy_requirement(int_req, [options[0]['int']['pins'][0]])
