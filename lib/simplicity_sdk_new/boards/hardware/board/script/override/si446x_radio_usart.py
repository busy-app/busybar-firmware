from siliconlabs.slc.board_gen.util.board_gen_util import Req

def compatible(provides, board):
    if board.has_component('si44xx'):
        # check if all options solves
        if board.get_peripheral_options(Req('usart'), 'si44xx', {'copi' : 'TX', 'cipo' : 'RX', 'clk' : 'CLK'}):
            if board.get_peripheral_options(Req('gpio'), 'si44xx', {'cs_n' : None}):
                if board.get_peripheral_options(Req('gpio'), 'si44xx', {'irq_n' : None}):
                    if board.get_peripheral_options(Req('gpio'), 'si44xx', {'sdn' : None}):
                        return 'si446x_radio'
    return False

def configure(project, board, instance_name):
    component = 'si446x_radio'
    req = project.requirement('SL_SI446X_RADIO')

    board_map = {
        'copi'      :   'TX',
        'cipo'      :   'RX',
        'clk'       :   'CLK'
    }

    signal_options = board.get_peripheral_options(req, 'si44xx', board_map)
    # Select first matching peripheral
    locations = select_peripheral_instance(board, signal_options)
    locs = []
    for b_signal in board_map.keys():
        if b_signal in locations.keys():
            locs.append(locations[b_signal]['locations'][0])
    project.satisfy_requirement(req, locs)

    # Get requirement for CS
    cs_req = project.requirement('SL_SI446X_RADIO_CS')
    options = board.get_peripheral_options(cs_req, 'si44xx', {'cs_n' : None})
    pin = options[0]['cs_n']['pins'][0]
    project.satisfy_requirement(cs_req, [pin])

    # Get requirement for INT
    int_req = project.requirement('SL_SI446X_RADIO_INT')
    options = board.get_peripheral_options(int_req, 'si44xx', {'irq_n' : None})
    pin = options[0]['irq_n']['pins'][0]
    project.satisfy_requirement(int_req, [pin])

    # Get requirement for SND
    sdn_req = project.requirement('SL_SI446X_RADIO_SDN')
    options = board.get_peripheral_options(sdn_req, 'si44xx', {'sdn' : None})
    pin = options[0]['sdn']['pins'][0]
    project.satisfy_requirement(sdn_req, [pin])

def select_peripheral_instance(board, options):
    # Default to the first match
    return options[0]
