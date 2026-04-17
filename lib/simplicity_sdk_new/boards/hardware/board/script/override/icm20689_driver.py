from siliconlabs.slc.board_gen.util.board_gen_util import Req

sensor_names = ['spisensor', 'accelerometer']

board_map = {
    'copi': 'TX',
    'cipo': 'RX',
    'clk': 'SCLK',
    'cs_n': 'CS'
}


def compatible(provides, board):
    icm_20689_component = board.get_components_by_part_number("ICM-20689")

    for sensor_name in sensor_names:
        if board.has_component(sensor_name) and len(icm_20689_component) == 1:

            if board.get_peripheral_options(Req('eusart'), sensor_name, board_map):
                return True

    return False


def configure(project, board, instance_name):
    req = project.requirement('SL_ICM20689_SPI_EUSART')

    for sensor_name in sensor_names:
        if board.has_component(sensor_name):
            signal_options = board.get_peripheral_options(req, sensor_name, board_map)

            # Select first matching peripheral
            locations = signal_options[0]
            locs = []
            for b_signal in board_map.keys():
                if b_signal in locations.keys():
                    locs.append(locations[b_signal]['locations'][0])
            project.satisfy_requirement(req, locs)

            int_req = project.requirement('SL_ICM20689_INT')
            options = board.get_peripheral_options(int_req, sensor_name, {'int': None})
            project.satisfy_requirement(int_req, [options[0]['int']['pins'][0]])

            return

