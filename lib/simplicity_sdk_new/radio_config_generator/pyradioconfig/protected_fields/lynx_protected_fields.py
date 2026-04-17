from pyradioconfig.protected_fields.base_protected_fields import ProtectedFieldsBase


class ProtectedFieldsLynx(ProtectedFieldsBase):

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

    PTE_PROTECTED_FIELD_DICT = {
        'RAC_PATRIM1_TX0DBMTRIMREGVREF': 'cal',
        'RAC_PATRIM1_TX0DBMTRIMREGFB': 'cal',
        'RAC_PATRIM0_TX0DBMTRIMDUTYCYP': 'cal',
        'RAC_PATRIM0_TX0DBMTRIMDUTYCYN': 'cal',
        'RAC_PATRIM0_TX0DBMTRIMBIASP': 'cal',
        'RAC_PATRIM0_TX0DBMTRIMBIASN': 'cal',
        'RAC_PATRIM3_TX6DBMTRIMREGVREF': 'cal',
        'RAC_PATRIM3_TX6DBMTRIMREGFB': 'cal',
        'RAC_PATRIM3_TX6DBMTRIMPREDRVREGVREF': 'cal',
        'RAC_PATRIM3_TX6DBMTRIMPREDRVREGFB': 'cal',
        'RAC_PATRIM2_TX6DBMTRIMDUTYCYP': 'cal',
        'RAC_PATRIM2_TX6DBMTRIMDUTYCYN': 'cal',
        'RAC_PATRIM2_TX6DBMTRIMBIASP': 'cal',
        'RAC_PATRIM2_TX6DBMTRIMBIASN': 'cal',
        'SYNTH_LPFCTRL2RX_CALCRX': 'cal',
        'SYNTH_LPFCTRL2TX_CALCTX': 'cal',
        'SYNTH_VCDACCTRL_VCDACVAL': 'cal',
        'SYNTH_VCOGAIN_VCOKVCOARSE': 'cal',
        'SYNTH_VCOGAIN_VCOKVFINE': 'cal',
        'RAC_PGACTRL_PGAPOWERMODE': 'cal',
        'RAC_IFADCTRIM0_IFADCLDOSHUNTAMPLVL1': 'cal',
        'RAC_IFADCTRIM0_IFADCLDOSHUNTAMPLVL2': 'cal',
        'RAC_IFADCTRIM0_IFADCLDOSHUNTCURLVL1': 'cal',
        'RAC_IFADCTRIM0_IFADCLDOSHUNTCURLVL2': 'cal',
        'RAC_RFBIASCAL_RFBIASCALVREF': 'cal',
        'RAC_RFBIASCAL_RFBIASCALBIAS': 'cal',
        'RAC_RFBIASCAL_RFBIASCALTC': 'cal',
        'RAC_RFBIASCTRL_RFBIASNONFLASHMODE': 'cal',
        'RAC_LNAMIXTRIM1_LNAMIXIBIASADJ': 'cal',
        'RAC_LNAMIXTRIM0_LNAMIXRFPKDCALCM': 'cal',
        'RAC_PRECTRL_PREREGTRIM': 'cal',
        'RAC_IFADCCAL_IFADCTUNERC': 'cal',
        'RAC_PGATRIM_PGACTUNE': 'cal',
        'RAC_AUXADCTRIM_AUXADCRCTUNE': 'cal',
        'RAC_AUXADCTRIM_AUXADCLDOVREFTRIM': 'cal',
        'RAC_AUXADCTRIM_AUXADCTSENSETRIMVBE2': 'cal',
        'MODEM_IRCALCOEF_CIV': 'cal',
        'MODEM_IRCALCOEF_CRV': 'cal',
        'MODEM_IRCAL_MURSHF': 'cal',
        'MODEM_IRCAL_MUISHF': 'cal'
    }

    # : List of fields found in PTE, but allow radio configurator to override the value
    PTE_PROTECTED_FIELD_EXCEPTIONS = [
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
        'RAC_SYEN_SYENVCOBIAS',
        'RAC_SYEN_SYENMMDREG',
        'RAC_SYEN_SYLODIVLDOBIASEN',
        'RAC_SYEN_SYENCHPREG',
        'RAC_SYEN_SYENCHPREPLICA',
        'RAC_SYEN_SYENMMDREPLICA1',
        'RAC_SYEN_SYENVCOPFET',
        'RAC_SYEN_SYENVCOREG',
        'RAC_SYEN_SYLODIVLDOEN',
        'RAC_SYEN_SYCHPEN',
        'RAC_SYEN_SYLODIVEN',
    ]

    # : List of fields that are found in PTE list, but does not need to be protected by RAIL
    RAIL_PROTECTED_FIELD_EXCEPTIONS = [
        'RAC_PGACTRL_PGAPOWERMODE',
        'RAC_IFADCTRIM0_IFADCLDOSHUNTAMPLVL1',
        'RAC_IFADCTRIM0_IFADCLDOSHUNTAMPLVL2',
        'RAC_IFADCTRIM0_IFADCLDOSHUNTCURLVL1',
        'RAC_IFADCTRIM0_IFADCLDOSHUNTCURLVL2',
        'RAC_RFBIASCTRL_RFBIASNONFLASHMODE',
        'RAC_LNAMIXTRIM0_LNAMIXRFPKDCALCM',
        'RAC_AUXADCTRIM_AUXADCTSENSETRIMVBE2',
        'RAC_RFBIASCAL_RFBIASCALTC',
    ]