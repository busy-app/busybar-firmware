import logging
from siliconlabs.slc.board_gen.util.board_gen_util import Req

logger = logging.getLogger(__name__)

board_map = {
    'i2c_scl'   : 'SCL',
    'i2c_sda'   : 'SDA',
}

def compatible(provides, board):
    instances = []

    for sensor in board.get_components_by_type('sensor'):
        if board.get_peripheral_options(Req('i2c'), sensor.id, board_map):
            # Override Si7021-A20 as Si7021 sensor component
            if sensor.part_number.lower() == "si7021-a20":
                sensor.part_number = "Si7021"
            # Check if sensor is supported
            if not board.has_tag('hardware:has:sensor:{}'.format(sensor.part_number).lower()):
                return []

            # Use bus_name in instance name if sensor component has this property
            instance_name = 'sensor_{}'.format(sensor.bus_name.lower()) if hasattr(sensor, 'bus_name') else 'sensor'
            if instance_name not in instances:
                instances.append(instance_name)
                if instance_name == "sensor_env":
                    instances.append("sensor")
                    # Thunderboard sense 2 requires a generic sensor instance
                    # which is a copy of sensor_env instance
    if board.has_component('mikroe'):
        instances.append('mikroe')
    if board.has_component('qwiic'):
        instances.append('qwiic')
    return instances

def configure(project, board, instance_name):
    if instance_name in ['qwiic', 'mikroe']:
        sensor_component_id = instance_name
    else:
        # Find sensor component ID
        for sensor in board.get_components_by_type('sensor'):
            if hasattr(sensor, 'bus_name'):
                if sensor.bus_name.lower() == instance_name.split('_', 1)[1]:
                    sensor_component_id = sensor.id
                    break
        else:
            sensor_component_id = sensor.id

    config_prefix = 'SL_I2CSPM_{}'.format(instance_name.upper())
    req = project.requirement(config_prefix)

    signal_options = board.get_peripheral_options(req, sensor_component_id, board_map)

    # Select first matching peripheral
    if signal_options:
        locations = signal_options[0]
        locs = []
        for b_signal in board_map.keys():
            if b_signal in locations.keys():
                locs.append(locations[b_signal]['locations'][0])

        project.satisfy_requirement(req, locs)
    else:
        logger.warning("No signals found for i2c component- {}".format(instance_name))