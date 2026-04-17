from siliconlabs.slc.board_gen.util.board_gen_util import Req

board_map = {
    'i2c_scl'   : 'SCL',
    'i2c_sda'   : 'SDA',
}

def compatible(provides, board):
    instances = []
    if board.has_component('efp'):
        if board.get_peripheral_options(Req('i2c'), 'efp', board_map):
            instances.append('efp0')
    return instances

def configure(project, board, instance_name):

    config_prefix = 'SL_EFP_{}_I2C'.format(instance_name.upper())
    req = project.requirement(config_prefix)

    signal_options = board.get_peripheral_options(req, 'efp', board_map)

    # Select first mathing I2C peripheral
    locations = signal_options[0]

    locs = []
    for b_signal in board_map.keys():
        if b_signal in locations.keys():
            locs.append(locations[b_signal]['locations'][0])

    project.satisfy_requirement(req, locs)

    int_req = project.requirement('SL_EFP_{}_IRQ'.format(instance_name.upper()))
    options = board.get_peripheral_options(int_req, 'efp', {'int' : None})
    location = options[0]['int']['pins']
    project.satisfy_requirement(int_req, location)
