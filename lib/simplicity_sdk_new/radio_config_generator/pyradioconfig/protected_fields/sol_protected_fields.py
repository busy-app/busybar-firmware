from pyradioconfig.protected_fields.base_protected_fields import ProtectedFieldsBase


class ProtectedFieldsSol(ProtectedFieldsBase):

    """
    PTE has two types of protected fields
    1) cal
    2) set and forget

    For radio configurator,
    1) Protect all PTE cal fields (do not write)
    2) Ensure set and forget fields are same as PTE if radio configurator is also programming the values

    For rail_script,
    1) protect all PTE cal fields
    2) Protect set and forget field, if register contains pte cal field or radio configurator writes to the field

    """

    # From https://confluence.silabs.com/pages/viewpage.action?spaceKey=PGSOL&title=Test+Plan+-+Feature+Write
    # Omitting CMU, ULFRCO/LFRCO/HFRCO, EMU, DCDC, USB, ACMP, IADC, VDAC, cal fields
    PTE_PROTECTED_FIELD_DICT = {
        # "DPLL0_OFFSET_K0": "cal",
        # "PLL_VTRTRIM_CALTC": 0x1A,
        # "PLL_VTRTRIM_CALVREF": "cal",
        # "PLL_VTRTRIM_CALBIAS": "cal",
        # "PLL_TRIM_DCOTEMPADJ": "cal",
        "RFFPLL0_RFBIASCAL_CALVREF": "cal",
        "RFFPLL0_RFBIASCAL_CALBIAS": "cal",
        "RFFPLL0_RFBIASCAL1_LDOHIGHCURRENT": "cal",
        "RFFPLL0_RFBIASCAL1_NONFLASHMODE": "cal",
        # "HFXO0_TRIM_SHUNTLVLANA": "cal",
        # "HFXO0_TRIM_VTRREGTRIMANA": "cal",
        # "HFXO0_TRIM_VTRCORETRIMANA": "cal",
        # "HFXO0_LOWPWRCTRL_SHUNTBIASANA": "cal",
        # "HFXO0_INTERNALCTRL_ENCLKMULTANA": 0x1,
        # "HFXO0_INTERNALCTRL_SQBUFFILTANA": 0x2,
        # "HFXO0_XTALCTRL_CTUNEFIXANA": 0x2,
        # "HFXO0_XTALCTRL1_COREBIASBUFOUTDELTA": 0x7,
        # "HFXO0_XTALCTRL1_CTUNEXIBUFOUTDELTA": 0x3,
        # "HFXO0_PKDETCTRL_PKDDETTHHIGH": 0x6,
        # "HFXO0_XOUTTRIM_VREGBIASTRIMIBCOREANA": "cal",
        # "HFXO0_XOUTTRIM_VREGBIASTRIMIBNDIOANA": "cal",
        # "HFXO0_BUFOUTTRIM_VTRTRIMANA": "cal",
        "RAC_RFBIASCAL_RFBIASCALTC": 0x1A,
        "RAC_RFBIASCAL_RFBIASCALVREF": "cal",
        "RAC_RFBIASCAL_RFBIASCALBIAS": "cal",
        "RAC_RFBIASCTRL_RFBIASNONFLASHMODE": 0x1,
        "RAC_PRECTRL_PREREGTRIM": "cal",
        "RAC_AUXADCTRIM_AUXADCRCTUNE": "cal",
        "RAC_AUXADCTRIM_AUXADCTSENSETRIMVBE2": 0x1,
        "RAC_IFADCTRIM0_IFADCLDOSHUNTAMPLVL1": 0x1,
        "RAC_IFADCTRIM0_IFADCLDOSHUNTAMPLVL2": 0x1,
        "RAC_IFADCTRIM0_IFADCLDOSHUNTCURLVL1": 0x7,
        "RAC_IFADCTRIM0_IFADCLDOSHUNTCURLVL2": 0x2,
        "RAC_IFADCCAL_IFADCTUNERC": "cal",
        "RAC_PGACTRL_PGAPOWERMODE": 0x2,
        "RAC_PGATRIM_PGAVCMOUTTRIM": 0x4,
        "RAC_PGATRIM_PGACTUNE": "cal",
        "RAC_CLKMULTEN0_CLKMULTREG2ADJV": 0x0,
        "SYNTH_LPFCTRL2TX_LPFINCAPTX": 0x2,
        "SYNTH_LPFCTRL2TX_CALCTX": "cal",
        "SYNTH_LPFCTRL2RX_LPFINCAPRX": 0x2,
        "SYNTH_LPFCTRL2RX_CALCRX": "cal",
        "SYNTH_VCDACCTRL_VCDACVAL": "cal",
        "SYNTH_VCOGAIN_VCOKVFINE": "cal",
        "SYNTH_VCOGAIN_VCOKVCOARSE": "cal",
        "RAC_SYTRIM0_SYTRIMCHPREGAMPBIAS": 0x0,
        "RAC_SYTRIM0_SYTRIMCHPREGAMPBW": 0x3,
        "RAC_SYTRIM1_SYTRIMMMDREGAMPBIAS": 0x1,
        "RAC_SYTRIM1_SYTRIMMMDREGAMPBW": 0x3,
        "RAC_SYTRIM1_SYLODIVTLOTRIMDELAY": "cal",
        "RAC_LNAMIXTRIM2_LNAMIXLOWCUR": 0x3,
        "RAC_LNAMIXTRIM3_LNAMIXIBIASADJN": "cal",
        "RAC_LNAMIXTRIM3_LNAMIXIBIASADJP": "cal",
        "RAC_LNAMIXTRIM3_LNAMIXIBIASRANGEADJN": "cal",
        "RAC_LNAMIXTRIM3_LNAMIXIBIASRANGEADJP": "cal",
        "RAC_LNAMIXTRIM4_LNAMIXRFPKDCALCMHI": "cal",
        "RAC_LNAMIXTRIM4_LNAMIXRFPKDCALCMLO": "cal",
        "RAC_PGACAL_PGAOFFCALQ": "cal",
        "RAC_PGACAL_PGAOFFCALI": "cal",
        "RAC_PATRIM1_TXTRIMHPAVPCAS": 0x7,
        "RAC_PATRIM1_TXTRIMXPAVPB": 0x3,
        "RAC_PATRIM1_TXTRIMXPAVNB": 0x3,
        "RAC_PATRIM2_TXTRIMCLKGENNOVRISE": 0x2,
        "RAC_PATRIM2_TXTRIMCLKGENNOVFALL": 0x2,
        "RAC_PATRIM2_TXTRIMCLKGENDUTYP": "cal",
        "RAC_PATRIM2_TXTRIMCLKGENDUTYN": "cal",
        "RAC_PATRIM3_TXTRIMOREGPSR": 0x1,
        "RAC_PATRIM3_TXTRIMOREG": "cal",
        "RAC_PATRIM3_TXTRIMRREG": "cal",
        "RAC_PATRIM3_TXTRIMDREG": "cal",
        "RAC_PATRIM10_TXTRIMPAPBIAS": "cal",
        "RAC_PATRIM10_TXTRIMPANBIAS": "cal",
        "RAC_TXITC_TXSELITCPALEVEL": 0x0,
        "RAC_TXITC_TXSELITCDACLEVEL": 0x4,
        "RAC_TXCALITC_TXCALITCCORETC": "cal",
        "RAC_TXCALITC_TXCALITCPAIB": "cal",
        "RAC_TXCALITC_TXCALITCDACIB": "cal",
        "RAC_TXRFPKD_TXRFPKDCALCODE": "cal",
        "FEFILT0_IRCALCOEFWR0_CRVWEN": 0x1,
        "FEFILT0_IRCALCOEFWR0_CIVWEN": 0x1,
        "FEFILT0_IRCALCOEFWR0_CRVWD": "cal",
        "FEFILT0_IRCALCOEFWR0_CIVWD": "cal",
        "FEFILT0_IRCALCOEFWR1_CRVWEN": 0x1,
        "FEFILT0_IRCALCOEFWR1_CIVWEN": 0x1,
        "FEFILT0_IRCALCOEFWR1_CRVWD": "cal",
        "FEFILT0_IRCALCOEFWR1_CIVWD": "cal",
        "FEFILT0_IRCAL_IRCALEN": 0x1,
        "FEFILT0_IRCAL_IRCORREN": 0x1,
        "FEFILT0_IRCAL_MURSHF": 0x0,
        "FEFILT0_IRCAL_MUISHF": 0x0
    }

    # : List of fields found in PTE, but allow radio configurator to override the value
    PTE_PROTECTED_FIELD_EXCEPTIONS = [
        "RAC_SYTRIM0_SYTRIMCHPREGAMPBIAS", # : PTE Set and forget value. Radio Configurator also sets to the same val.
        "RAC_SYTRIM0_SYTRIMCHPREGAMPBW", # : PTE Set and forget value. Radio Configurator also sets to the same val.
        "RAC_SYTRIM1_SYTRIMMMDREGAMPBIAS", # : PTE Set and forget value. Radio Configurator also sets to the same val.
        "RAC_SYTRIM1_SYTRIMMMDREGAMPBW", # : PTE Set and forget value. Radio Configurator also sets to the same val.
        "RAC_TXITC_TXSELITCPALEVEL", # : PTE Set and forget value. Radio Configurator also sets to the same val.
        "RAC_TXITC_TXSELITCDACLEVEL", # : PTE Set and forget value. Radio Configurator also sets to the same val.
        "RAC_PGACAL_PGAOFFCALQ",  #: 'cal',  # : cal by fw
        "RAC_PGACAL_PGAOFFCALI",  #: 'cal',  # : cal by fw
        "FEFILT0_IRCALCOEFWR0_CRVWD", # : set by RAIL
        "FEFILT0_IRCALCOEFWR0_CIVWD", # : set by RAIL
        "FEFILT0_IRCALCOEFWR1_CRVWD",  # : set by RAIL
        "FEFILT0_IRCALCOEFWR1_CIVWD",  # : set by RAIL
    ]

    # : List of fields protected by RAIL
    RAIL_PROTECTED_FIELDS = [
        'AGC_CTRL0_RSSISHIFT',
        'AGC_CTRL1_CCATHRSH',
        'FRC_RXCTRL_BUFRESTORERXABORTED',
        'FRC_RXCTRL_BUFRESTOREFRAMEERROR',
        'FRC_RXCTRL_BUFCLEAR',
        'FRC_RXCTRL_TRACKABFRAME',
        'FRC_RXCTRL_ACCEPTBLOCKERRORS',
        'FRC_RXCTRL_ACCEPTCRCERRORS',
        'FRC_RXCTRL_STORECRC',
        'RAC_AUXADCTRIM_AUXADCLDOVREFTRIM',
        'RAC_PGATRIM_PGACTUNE',
        'RAC_SYEN_SYENVCOBIAS',
        'RAC_SYEN_SYENMMDREG',
        'RAC_SYEN_SYENCHPREG',
        'RAC_SYEN_SYENCHPREPLICA',
        'RAC_SYEN_SYENMMDREPLICA1',
        'RAC_SYEN_SYENVCOPFET',
        'RAC_SYEN_SYENVCOREG',
        'RAC_SYEN_SYCHPEN',
        'RAC_SOFTMCTRL_CLKEN',
        'RAC_TX_TXENPADDCFORCE',
        'RAC_TX_TXMODEPMOSOFF',
        'RAC_TX_TXENXDRVVMID',
        'RFFPLL0_RFFPLLCAL1_DIVR',
        'RFFPLL0_RFFPLLCTRL1_DIVN',
        'RFFPLL0_RFFPLLCTRL1_DIVX',
        'RFFPLL0_RFFPLLCTRL1_DIVY',
        'RFFPLL0_RFFPLLCTRL1_DIVXDACSEL',
        'RFFPLL0_RFFPLLCTRL1_DIVXMODEMSEL',
        'RFFPLL0_RFFPLLCTRL1_DIVYSEL',
        'SYNTH_LPFCTRL2TX_LPFINCAPTX',
        'SYNTH_LPFCTRL2RX_LPFINCAPRX',
        'RAC_IFADCTRIM0_IFADCLDOSHUNTAMPLVL1',
        'RAC_IFADCTRIM0_IFADCLDOSHUNTAMPLVL2',
        'RAC_IFADCTRIM0_IFADCLDOSHUNTCURLVL1',
        'RAC_PGACTRL_PGAPOWERMODE',
        'MODEM_ANARAMPCTRL_ANARAMPLUTODEV',
        # : TODO These fields should be protected but it is missing from the rail_script list
        'RAC_PATRIM3_TXTRIMOREGPSR',
        'RAC_CLKMULTEN0_CLKMULTREG2ADJV',
    ]

    # : List of fields that are found in PTE list, but does not need to be protected by RAIL
    RAIL_PROTECTED_FIELD_EXCEPTIONS = [
    ]