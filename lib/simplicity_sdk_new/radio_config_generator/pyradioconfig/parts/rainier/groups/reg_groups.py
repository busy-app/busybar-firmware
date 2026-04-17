# Notes:
#   * File name must be reg_groups.py
#   * Group names, as derived from the list names
#   * Class group MUST be named "PhyGroups"
#   * Can use regex expressions to match group registers
#   * Regex expression can only be at the register level
#   * Each register output can belong to None, 1 or more groups

# from .reg_groups_hack import PhyGroups as PhyGroupsBase

class PhyGroups(object):

    rx_critical = [
        # Double-buffered regs only
        "MODEM_CF",
        "MODEM_DIGMIXCTRL",
        "MODEM_LOG2X4",
        "MODEM_SICTRL0",
        "MODEM_SICTRL2",
        "MODEM_SIFASTDETECTCTRL",
        'MODEM_COH0',
        'MODEM_COCURRMODE',
        "MODEM_SRCCHF",
        "MODEM_SYNCWORDCTRL",
    ]

    rx_noncritical = [
        "FRC_AUTOCG",
        "FRC_CONVGENERATOR",
        "FRC_CONVRAMADDR",
        "FRC_CTRL",
        "FRC_DFLCTRL",
        "FRC_DSLCTRL",
        "FRC_FCD0",
        "FRC_FCD1",
        "FRC_FCD2",
        "FRC_FCD3",
        "FRC_FECCTRL",
        "FRC_MAXLENGTH",
        "FRC_PUNCTCTRL",
        "FRC_TRAILTXDATACTRL",
        "FRC_WCNTCMP0",
        "FRC_WCNTCMP1",
        "FRC_WCNTCMP3",
        "FRC_WCNTCMP4",
        "FRC_WHITECTRL",
        "FRC_WHITEINIT",
        "FRC_WHITEPOLY",
        "RFCRC_CTRL",
        "RFCRC_INIT",
        "RFCRC_POLY",
        "MODEM_EHDSSSCFG3",
        "MODEM_TRECSCFG"
    ]
