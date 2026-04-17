from siliconlabs.slc.board_gen.util.board_gen_util import Req

def compatible(provides, hw):
    microphones = hw.get_components_by_type('microphone')
    if microphones:
        if (hw.get_peripheral_options(Req('pdm'), microphones[0].id, {'data' : 'DAT0', 'clk' : 'CLK'})):
            return True
    return False

def configure(project, hw, _):
    microphone = hw.get_components_by_type('microphone')[0]
    req = project.requirement('SL_MIC_PDM')
    board_map = {'data' :'DAT0', 'clk' : 'CLK'}
    signal_options = hw.get_peripheral_options(req, microphone.id, board_map)

    locations = signal_options[0]
    locs = []
    for b_signal in board_map.keys():
        if b_signal in locations.keys():
            locs.append(locations[b_signal]['locations'][0])
    project.satisfy_requirement(req, locs)
