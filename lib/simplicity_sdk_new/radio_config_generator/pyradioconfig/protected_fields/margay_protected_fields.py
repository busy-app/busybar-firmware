from pyradioconfig.protected_fields.base_protected_fields import ProtectedFieldsBase


class ProtectedFieldsMargay(ProtectedFieldsBase):

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

    # From https://confluence.silabs.com/display/PGMARGAY/Test+Plan+-+Feature+Write
    # Omitting CMU, ULFRCO/LFRCO/HFRCO, EMU, DCDC, USB, ACMP, IADC, VDAC, cal fields
    PTE_PROTECTED_FIELD_DICT = {
        # 'HFXO0_TRIM_SHUNTLVLANA': 'cal',
        # 'HFXO0_TRIM_VTRREGTRIMANA': 'cal',
        # 'HFXO0_TRIM_VTRCORETRIMANA': 'cal',
        # 'HFXO0_LOWPWRCTRL_SHUNTBIASANA': 'cal',
        # 'HFXO0_XTALCFG_TIMEOUTSTEADY': 'cal',
        'RAC_RFBIASCAL_RFBIASCALTC': 0x1A,
        'RAC_RFBIASCAL_RFBIASCALVREF': 'cal',
        'RAC_RFBIASCAL_RFBIASCALBIAS': 'cal',
        'RAC_RFBIASCTRL_RFBIASNONFLASHMODE': 0x1,
        'RAC_PRECTRL_PREREGTRIM': 'cal',
        'RAC_AUXADCTRIM_AUXADCRCTUNE': 'cal',
        'RAC_AUXADCTRIM_AUXADCTSENSETRIMVBE2': 0x1,
        'RAC_IFADCCAL_IFADCTUNERC': 'cal',
        'RAC_IFADCTRIM0_IFADCLDOSHUNTAMPLVL1': 0x1,
        'RAC_IFADCTRIM0_IFADCLDOSHUNTAMPLVL2': 0x1,
        'RAC_IFADCTRIM0_IFADCLDOSHUNTCURLVL1': 0x7,
        'RAC_IFADCTRIM0_IFADCLDOSHUNTCURLVL2': 0x2,
        'RAC_PGACTRL_PGAPOWERMODE': 0x3,
        'RAC_PGATRIM_PGACTUNE': 'cal',
        'SYNTH_LPFCTRL2TX_LPFINCAPTX': 0x2,
        'SYNTH_LPFCTRL2TX_CALCTX': 'cal',
        'SYNTH_LPFCTRL2RX_LPFINCAPRX': 0x2,
        'SYNTH_LPFCTRL2RX_CALCRX': 'cal',
        'SYNTH_VCDACCTRL_VCDACVAL': 'cal',
        'SYNTH_VCOGAIN_VCOKVFINE': 'cal',
        'SYNTH_VCOGAIN_VCOKVCOARSE': 'cal',
        'RAC_SYTRIM0_SYTRIMCHPREGAMPBIAS': 0x0,
        'RAC_SYTRIM0_SYTRIMCHPREGAMPBW': 0x3,
        'RAC_SYTRIM1_SYTRIMMMDREGAMPBIAS': 0x1,
        'RAC_SYTRIM1_SYTRIMMMDREGAMPBW': 0x3,
        'RAC_SYTRIM1_SYLODIVTLOTRIMDELAY': 'cal',
        'RAC_LNAMIXTRIM3_LNAMIXIBIASADJN': 'cal',
        'RAC_LNAMIXTRIM3_LNAMIXIBIASADJP': 'cal',
        'RAC_LNAMIXTRIM3_LNAMIXIBIASRANGEADJN': 'cal',
        'RAC_LNAMIXTRIM3_LNAMIXIBIASRANGEADJP': 'cal',
        'RAC_LNAMIXTRIM4_LNAMIXRFPKDCALCMHI': 'cal',
        'RAC_LNAMIXTRIM4_LNAMIXRFPKDCALCMLO': 'cal',
        'RAC_PGACAL_PGAOFFPCALQ': 'cal',
        'RAC_PGACAL_PGAOFFPCALI': 'cal',
        'RAC_PGACAL_PGAOFFNCALQ': 'cal',
        'RAC_PGACAL_PGAOFFNCALI': 'cal',
        'RAC_PATRIM1_TXTRIMXPAVPB': 'cal',
        'RAC_PATRIM1_TXTRIMXPAVNB': 'cal',
        'RAC_PATRIM1_TXTRIMHPAVPCAS': 'cal',
        'RAC_PATRIM1_TXTRIMHPAVNCAS': 'cal',
        'RAC_PATRIM2_TXTRIMCLKGENNOVRISE': 0x2,
        'RAC_PATRIM2_TXTRIMCLKGENNOVFALL': 0x2,
        'RAC_PATRIM2_TXTRIMCLKGENDUTYP': 'cal',
        'RAC_PATRIM2_TXTRIMCLKGENDUTYN': 'cal',
        'RAC_PATRIM3_TXTRIMOREGFB': 'cal', # : PTE set and forget but set to 'cal' since value changes with LPA vs. HPA
        'RAC_PATRIM3_TXTRIMOREG': 'cal',
        'RAC_PATRIM3_TXTRIMRREG': 'cal',
        'RAC_PATRIM3_TXTRIMDREG': 'cal',
        'RAC_PATRIM5_TXTRIMXPAVNS': 'cal',
        'RAC_PATRIM5_TXTRIMXPAVPS': 'cal',
        'RAC_IFADCPLLDCO_IFADCPLLDCOTEMPADJ': 'cal',
        'MODEM_IRCALCOEFWR0_CRVWEN': 0x1,
        'MODEM_IRCALCOEFWR0_CIVWEN': 0x1,
        'MODEM_IRCALCOEFWR0_CRVWD': 'cal',
        'MODEM_IRCALCOEFWR0_CIVWD': 'cal',
        'MODEM_IRCALCOEFWR1_CRVWEN': 0x1,
        'MODEM_IRCALCOEFWR1_CIVWEN': 0x1,
        'MODEM_IRCALCOEFWR1_CRVWD': 'cal',
        'MODEM_IRCALCOEFWR1_CIVWD': 'cal',
        'MODEM_IRCAL_IRCALEN': 0x1,
        'MODEM_IRCAL_IRCORREN': 0x1,
        'MODEM_IRCAL_MURSHF': 0x0,
        'MODEM_IRCAL_MUISHF': 0x0,
        'RAC_CLKMULTEN0_CLKMULTREG2ADJV': 0x0,
    }

    # : List of fields found in PTE, but allow radio configurator to override the value
    PTE_PROTECTED_FIELD_EXCEPTIONS = [
        'RAC_PGACAL_PGAOFFPCALQ',  #: 'cal',  # : cal by fw
        'RAC_PGACAL_PGAOFFPCALI',  #: 'cal',  # : cal by fw
        'RAC_PGACAL_PGAOFFNCALQ',  #: 'cal',  # : cal by fw
        'RAC_PGACAL_PGAOFFNCALI',  #: 'cal',  # : cal by fw
        'MODEM_IRCALCOEFWR0_CRVWD', # : Set by RAIL
        'MODEM_IRCALCOEFWR0_CIVWD', # : Set by RAIL
        'MODEM_IRCALCOEFWR1_CRVWD',  # : Set by RAIL
        'MODEM_IRCALCOEFWR1_CIVWD',  # : Set by RAIL
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
        'MODEM_IRCALCOEF_CIV',
        'MODEM_IRCALCOEF_CRV',
        'MODEM_IRCAL_MURSHF',
        'MODEM_IRCAL_MUISHF',
        'RAC_SYEN_SYENVCOBIAS',
        'RAC_SYEN_SYENMMDREG',
        'RAC_SYEN_SYENCHPREG',
        'RAC_SYEN_SYENCHPREPLICA',
        'RAC_SYEN_SYENMMDREPLICA1',
        'RAC_SYEN_SYENVCOPFET',
        'RAC_SYEN_SYENVCOREG',
        'RAC_SYEN_SYCHPEN',
        'MODEM_ANARAMPCTRL_ANARAMPLUTODEV',
        'RAC_AUXADCTRIM_AUXADCLDOVREFTRIM',
        'RAC_IFADCTRIM0_IFADCLDOSHUNTAMPLVL1',
        'RAC_IFADCTRIM0_IFADCLDOSHUNTAMPLVL2',
        'RAC_IFADCTRIM0_IFADCLDOSHUNTCURLVL1',
        'RAC_PGACTRL_PGAPOWERMODE',
        'SYNTH_LPFCTRL2TX_LPFINCAPTX',
        'SYNTH_LPFCTRL2RX_LPFINCAPRX',
        'RAC_CLKMULTEN0_CLKMULTREG2ADJV',
    ]

    # : List of fields that are found in PTE list, but does not need to be protected by RAIL
    RAIL_PROTECTED_FIELD_EXCEPTIONS = [
    ]