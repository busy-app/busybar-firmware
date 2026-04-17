from siliconlabs.slc.board_gen.util.board_gen_util import Req

mic_signals = {
    'data': 'RX',
    'bclk': 'CLK',
    'lrclk': 'CS'
}

mic_names = [
    'microphone',
    'microphone_0',
    'microphone_1'
]

def get_mic_name(board):
    """ Return the name of the first microphone found on the board (in case of stereo mic). None if no microphone on the board """

    for name in mic_names:
        # Verify the board has the component and all signals are connected
        if board.has_component(name) and board.get_peripheral_options(Req('usart'), name, mic_signals):
            return name
    return None

def compatible(provides, board):
    return get_mic_name(board) != None


def configure(project, board, instance_name):
    component = get_mic_name(board)
    req = project.requirement('SL_MIC_I2S')

    signal_options = board.get_peripheral_options(req, component, mic_signals)
    # Select first matching peripheral
    locations = signal_options[0]
    locs = []
    for b_signal in mic_signals.keys():
        if b_signal in locations.keys():
            locs.append(locations[b_signal]['locations'][0])
    project.satisfy_requirement(req, locs)
