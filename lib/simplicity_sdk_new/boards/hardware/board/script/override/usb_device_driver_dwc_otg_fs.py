from siliconlabs.slc.board_gen.util.board_gen_util import Req

def compatible(provides, board):
    if board.provides('device_has_usb'):
        options = board.get_peripheral_options(Req('gpio'), 'usb', {'vbus_sense': None})
        # If vbus connected to a GPIO, the netlist format will be e.g. [efr32.pc02, usb.vbus] in the <board>.yaml file.
        # In that case, a valid information about the pin connection will be obtained. Otherwise, no information.
        if options:
            return True

    return False

def configure(project, board, instance):
    # Get requirement object from the USB device driver config file // <gpio> SL_USBD_DRIVER_VBUS_SENSE
    vbus_req = project.requirement('SL_USBD_DRIVER_VBUS_SENSE')

    # Get peripheral/pinout options for USB MICRO-B
    options = board.get_peripheral_options(vbus_req, 'usb', {'vbus_sense': None})


    # Satisfy the requirement using the single pin VBUS
    pin = options[0]['vbus_sense']['pins'][0]
    project.satisfy_requirement(vbus_req, [pin])
