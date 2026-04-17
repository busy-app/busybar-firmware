import logging

logger = logging.getLogger(__name__)


def compatible(provides, hw):
    """
    Search the board for a display that is supported by the memlcd driver. The
    memlcd driver currently supports LS013B7DH03, LS013B7DH06 and LPM013M126A.
    """
    displays = hw.get_components_by_type('display')
    if not displays:
        return False

    display_available = False
    for display in displays:
        part_number = getattr(display, 'part_number', None)
        if part_number in ['LS013B7DH03', 'LS013B7DH06', 'LPM013M126A']:
            display_available = True

    if display_available:
        # Check if something is connected to the SI pin of the display
        si_pin = hw.board.get_device_connection('display', 'si')[0]
        if not si_pin:
            display_available = False

    return display_available and hw.provides('device_has_usart')

def configure(project, hw, instance):
    """
    The memlcd driver config has 3 "requirements" that need to be resolved
    by pintool. These are called SL_MEMLCD_SPI, SL_MEMLCD_SPI_CS and
    SL_MEMLCD_EXTCOMIN.

    Requirements:
    SL_MEMLCD_SPI
      This is a requirement that needs to be resolved by finding a SPI
      peripheral and connecting SPI signals to SI (TX) and SCLK (CLK).

    SL_MEMLCD_SPI_CS
      This requirement is a GPIO pin which is used to control the chip select
      signal SCS (CS).

    SL_MEMLCD_EXTCOMIN
      This requirement is a GPIO pin which is toggled periodically when the
      display is on.

    Mapping of names
    Signal Name | Schema Name | Config Name
    --------------------------------------------
    si            SI            TX
    sclk          SCLK          CLK
    scs           SCS           CS
    extcomin      EXTCOMIN      EXTCOMIN

    The enable signal is controlled by the board control component so its not
    in use by this driver.
    """

    req = project.requirement('SL_MEMLCD_SPI')
    board_map = {
        'si': 'TX',
        'sclk': 'CLK',
    }
    signal_options = hw.get_peripheral_options(req, 'display', board_map)

    if not signal_options:
        raise RuntimeError('Unable to find connection that match')
    # Choose the best option
    locations = select_peripheral_instance(hw, signal_options)

    # Satisfy requirement means that we insert values for TX and CLK.
    project.satisfy_requirement(req, [locations['si']['locations'][0], locations['sclk']['locations'][0]])

    connect_gpio(project, hw, 'SL_MEMLCD_SPI_CS', 'display', 'scs')
    connect_gpio(project, hw, 'SL_MEMLCD_EXTCOMIN', 'display', 'extcomin')

def connect_gpio(project, hw, macro_id, component_id, signal_id):
    """
    Find a gpio from a signal name and connect it to the correct pin.
    """
    req = project.requirement(macro_id)
    signal_options = hw.get_peripheral_options(req, component_id, {signal_id: None})
    if signal_options:
        project.satisfy_requirement(req, [signal_options[0][signal_id]['pins'][0]])
    elif is_optional(req):
        logger.warning('Unable to find pin to use for optional {} macro.'.format(macro_id))
    else:
        raise RuntimeError('Unable to find pin for {} on board'.format(macro_id))

def is_optional(req):
    optional = req.properties.get('optional', ['false'])
    return optional[0] == 'true'


def select_peripheral_instance(board, options):
    # Default to the first match
    return options[0]
