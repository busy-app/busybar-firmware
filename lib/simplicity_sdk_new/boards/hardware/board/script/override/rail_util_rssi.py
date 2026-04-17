special_case = {
    # RSSI offset value for BRD4276A is updated to -13 as mentioned in RDMAP-6269
    "BRD4276A": {
        "offset": '-13'
    }
}


def compatible(provides, board):
    if board.board.board_no in special_case:
        return True

    return False


def configure(project, board, _):
    # For special cases, the offset value should override
    board_opn = board.board.board_no
    project.config('SL_RAIL_UTIL_RSSI_OFFSET').value = special_case[board_opn]['offset']
