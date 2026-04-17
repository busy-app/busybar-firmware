from siliconlabs.slc.board_gen.util.board_gen_util import Req

def compatible(provides,hw):
    instances = []
    # brd332a does not have spi connection thorugh exp header.But sidewalk team requires spi exp connection to use sx1262, so we are adding dummy exp instance.
    if hw.provides('brd4332a') and hw.has_component('sx'):
        required_signals = {
            'mosi': 'TX',
            'miso': 'RX',
            'sck': 'CLK',
            'nss': 'CS',
        }
        if hw.get_peripheral_options(Req('usart'), 'sx', required_signals):
            instances.append('exp')
    if hw.has_component('exp_h'):
        required_signals = {
            '4' : 'TX',
            '6' : 'RX',
            '8' : 'CLK',
            '10': 'CS',
        }
        if hw.get_peripheral_options(Req('usart'), 'exp_h', required_signals):
            # Usart available on exp 4/6/8/10
            instances.append('exp')
    if hw.has_component('mikroe'):
        required_signals = {
            'spi_mosi': 'TX',
            'spi_miso': 'RX',
            'spi_sck': 'CLK',
            'spi_cs_n': 'CS',
        }
        if hw.get_peripheral_options(Req('usart'), 'mikroe', required_signals):
            # Usart available on exp 4/6/8/10
            instances.append('mikroe')
    return instances

def configure(project, hw, instance_name):
    if hw.provides('brd4332a') and instance_name == 'exp':
        req = project.requirement('SL_SPIDRV_EXP')
        board_map = {
            'mosi': 'TX',
            'miso': 'RX',
            'sck': 'CLK',
            'nss': 'CS',
        }
        signal_options = hw.get_peripheral_options(req, 'sx', board_map)
        project.config('SL_SPIDRV_EXP_BITRATE').value = '8000000'
        project.config('SL_SPIDRV_EXP_CS_CONTROL').value = 'spidrvCsControlApplication'
    else:
        if instance_name == 'exp':
            req = project.requirement('SL_SPIDRV_EXP')
            board_map = {
                '4' : 'TX',
                '6' : 'RX',
                '8' : 'CLK',
                '10': 'CS',
            }
            signal_options = hw.get_peripheral_options(req, 'exp_h', board_map)
        elif instance_name == 'mikroe':
            req = project.requirement('SL_SPIDRV_MIKROE')
            board_map = {
                'spi_mosi': 'TX',
                'spi_miso': 'RX',
                'spi_sck': 'CLK',
                'spi_cs_n': 'CS',
            }
            signal_options = hw.get_peripheral_options(req, 'mikroe', board_map)


    # Select first matching Peripheral
    locations = select_peripheral_instance(hw, signal_options)
    locs = []

    for b_signal in board_map.keys():
        if b_signal in locations.keys():
            locs.append(locations[b_signal]['locations'][0])

    project.satisfy_requirement(req, locs)


def select_peripheral_instance(hw, options):
    if hw.provides('brd4332a'):
        for option in options:
            # Prefer USART2 if available
            if option['peripheral'] == 'USART2':
                return option

    # Default to the first match
    return options[0]
