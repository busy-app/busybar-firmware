from pyradioconfig.protected_fields.base_protected_fields import ProtectedFieldsBase


class ProtectedFieldsPanther(ProtectedFieldsBase):

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
        'MODEM_IRCALCOEF_CIV',
        'MODEM_IRCALCOEF_CRV',
        'MODEM_IRCAL_MURSHF',
        'MODEM_IRCAL_MUISHF',
        'RAC_AUXADCTRIM_AUXADCRCTUNE',
        'RAC_AUXADCTRIM_AUXADCLDOVREFTRIM',
        'RAC_RFBIASCAL_RFBIASCALVREF',
        'RAC_RFBIASCAL_RFBIASCALBIAS',
        'RAC_PRECTRL_PREREGTRIM',
        'RAC_IFADCCAL_IFADCTUNERC',
        'RAC_PGATRIM_PGACTUNE',
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
        'SYNTH_LPFCTRL2RX_CALCRX',
        'SYNTH_LPFCTRL2TX_CALCTX',
        'SYNTH_VCDACCTRL_VCDACVAL',
        'SYNTH_VCOGAIN_VCOKVCOARSE',
        'SYNTH_VCOGAIN_VCOKVFINE',
    ]

    # : List of fields that are found in PTE list, but does not need to be protected by RAIL
    RAIL_PROTECTED_FIELD_EXCEPTIONS = [
    ]