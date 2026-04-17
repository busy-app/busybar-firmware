
# -*- coding: utf-8 -*-

from . static import Base_RM_Register
from . SEQ_NS_field import *


class RM_Register_SEQ_NS_MMDDENOMINIT_CALC(Base_RM_Register):
    def __init__(self, rmio, label):
        self.__dict__['zz_frozen'] = False
        super(RM_Register_SEQ_NS_MMDDENOMINIT_CALC, self).__init__(rmio, label,
            0xb0000000, 0x000,
            'MMDDENOMINIT_CALC', 'SEQ_NS.MMDDENOMINIT_CALC', 'read-write',
            u"",
            0x00000000, 0x00000000,
            0x00001000, 0x00002000,
            0x00003000)

        self.DENOMINIT0 = RM_Field_SEQ_NS_MMDDENOMINIT_CALC_DENOMINIT0(self)
        self.zz_fdict['DENOMINIT0'] = self.DENOMINIT0
        self.DENOMINIT1 = RM_Field_SEQ_NS_MMDDENOMINIT_CALC_DENOMINIT1(self)
        self.zz_fdict['DENOMINIT1'] = self.DENOMINIT1
        self.__dict__['zz_frozen'] = True


class RM_Register_SEQ_NS_MMDDENOMINIT_CALC_DOUBLED(Base_RM_Register):
    def __init__(self, rmio, label):
        self.__dict__['zz_frozen'] = False
        super(RM_Register_SEQ_NS_MMDDENOMINIT_CALC_DOUBLED, self).__init__(rmio, label,
            0xb0000000, 0x004,
            'MMDDENOMINIT_CALC_DOUBLED', 'SEQ_NS.MMDDENOMINIT_CALC_DOUBLED', 'read-write',
            u"",
            0x00000000, 0x00000000,
            0x00001000, 0x00002000,
            0x00003000)

        self.DENOMINIT0 = RM_Field_SEQ_NS_MMDDENOMINIT_CALC_DOUBLED_DENOMINIT0(self)
        self.zz_fdict['DENOMINIT0'] = self.DENOMINIT0
        self.DENOMINIT1 = RM_Field_SEQ_NS_MMDDENOMINIT_CALC_DOUBLED_DENOMINIT1(self)
        self.zz_fdict['DENOMINIT1'] = self.DENOMINIT1
        self.__dict__['zz_frozen'] = True


class RM_Register_SEQ_NS_MODINDEX_CALC(Base_RM_Register):
    def __init__(self, rmio, label):
        self.__dict__['zz_frozen'] = False
        super(RM_Register_SEQ_NS_MODINDEX_CALC, self).__init__(rmio, label,
            0xb0000000, 0x008,
            'MODINDEX_CALC', 'SEQ_NS.MODINDEX_CALC', 'read-write',
            u"",
            0x00000000, 0x00000000,
            0x00001000, 0x00002000,
            0x00003000)

        self.MODINDEXM = RM_Field_SEQ_NS_MODINDEX_CALC_MODINDEXM(self)
        self.zz_fdict['MODINDEXM'] = self.MODINDEXM
        self.MODINDEXE = RM_Field_SEQ_NS_MODINDEX_CALC_MODINDEXE(self)
        self.zz_fdict['MODINDEXE'] = self.MODINDEXE
        self.FREQGAINE = RM_Field_SEQ_NS_MODINDEX_CALC_FREQGAINE(self)
        self.zz_fdict['FREQGAINE'] = self.FREQGAINE
        self.FREQGAINM = RM_Field_SEQ_NS_MODINDEX_CALC_FREQGAINM(self)
        self.zz_fdict['FREQGAINM'] = self.FREQGAINM
        self.AMMODINDEXM = RM_Field_SEQ_NS_MODINDEX_CALC_AMMODINDEXM(self)
        self.zz_fdict['AMMODINDEXM'] = self.AMMODINDEXM
        self.AMMODINDEXE = RM_Field_SEQ_NS_MODINDEX_CALC_AMMODINDEXE(self)
        self.zz_fdict['AMMODINDEXE'] = self.AMMODINDEXE
        self.__dict__['zz_frozen'] = True


class RM_Register_SEQ_NS_MODINDEX_CALC_MODINDEXE_DOUBLED(Base_RM_Register):
    def __init__(self, rmio, label):
        self.__dict__['zz_frozen'] = False
        super(RM_Register_SEQ_NS_MODINDEX_CALC_MODINDEXE_DOUBLED, self).__init__(rmio, label,
            0xb0000000, 0x00C,
            'MODINDEX_CALC_MODINDEXE_DOUBLED', 'SEQ_NS.MODINDEX_CALC_MODINDEXE_DOUBLED', 'read-write',
            u"",
            0x00000000, 0x00000000,
            0x00001000, 0x00002000,
            0x00003000)

        self.MODINDEXM = RM_Field_SEQ_NS_MODINDEX_CALC_MODINDEXE_DOUBLED_MODINDEXM(self)
        self.zz_fdict['MODINDEXM'] = self.MODINDEXM
        self.MODINDEXE = RM_Field_SEQ_NS_MODINDEX_CALC_MODINDEXE_DOUBLED_MODINDEXE(self)
        self.zz_fdict['MODINDEXE'] = self.MODINDEXE
        self.FREQGAINE = RM_Field_SEQ_NS_MODINDEX_CALC_MODINDEXE_DOUBLED_FREQGAINE(self)
        self.zz_fdict['FREQGAINE'] = self.FREQGAINE
        self.FREQGAINM = RM_Field_SEQ_NS_MODINDEX_CALC_MODINDEXE_DOUBLED_FREQGAINM(self)
        self.zz_fdict['FREQGAINM'] = self.FREQGAINM
        self.AMMODINDEXM = RM_Field_SEQ_NS_MODINDEX_CALC_MODINDEXE_DOUBLED_AMMODINDEXM(self)
        self.zz_fdict['AMMODINDEXM'] = self.AMMODINDEXM
        self.AMMODINDEXE = RM_Field_SEQ_NS_MODINDEX_CALC_MODINDEXE_DOUBLED_AMMODINDEXE(self)
        self.zz_fdict['AMMODINDEXE'] = self.AMMODINDEXE
        self.__dict__['zz_frozen'] = True


class RM_Register_SEQ_NS_FSWCOREINFO(Base_RM_Register):
    def __init__(self, rmio, label):
        self.__dict__['zz_frozen'] = False
        super(RM_Register_SEQ_NS_FSWCOREINFO, self).__init__(rmio, label,
            0xb0000000, 0x010,
            'FSWCOREINFO', 'SEQ_NS.FSWCOREINFO', 'read-write',
            u"",
            0x00000000, 0x00000000,
            0x00001000, 0x00002000,
            0x00003000)

        self.CONFIG = RM_Field_SEQ_NS_FSWCOREINFO_CONFIG(self)
        self.zz_fdict['CONFIG'] = self.CONFIG
        self.__dict__['zz_frozen'] = True


class RM_Register_SEQ_NS_MODEMINFO(Base_RM_Register):
    def __init__(self, rmio, label):
        self.__dict__['zz_frozen'] = False
        super(RM_Register_SEQ_NS_MODEMINFO, self).__init__(rmio, label,
            0xb0000000, 0x014,
            'MODEMINFO', 'SEQ_NS.MODEMINFO', 'read-write',
            u"",
            0x00000000, 0x00000000,
            0x00001000, 0x00002000,
            0x00003000)

        self.LEGACY_EN = RM_Field_SEQ_NS_MODEMINFO_LEGACY_EN(self)
        self.zz_fdict['LEGACY_EN'] = self.LEGACY_EN
        self.RXDC1CH_EN = RM_Field_SEQ_NS_MODEMINFO_RXDC1CH_EN(self)
        self.zz_fdict['RXDC1CH_EN'] = self.RXDC1CH_EN
        self.TRECS_EN = RM_Field_SEQ_NS_MODEMINFO_TRECS_EN(self)
        self.zz_fdict['TRECS_EN'] = self.TRECS_EN
        self.BCR_EN = RM_Field_SEQ_NS_MODEMINFO_BCR_EN(self)
        self.zz_fdict['BCR_EN'] = self.BCR_EN
        self.COHERENT_EN = RM_Field_SEQ_NS_MODEMINFO_COHERENT_EN(self)
        self.zz_fdict['COHERENT_EN'] = self.COHERENT_EN
        self.LONGRANGE_EN = RM_Field_SEQ_NS_MODEMINFO_LONGRANGE_EN(self)
        self.zz_fdict['LONGRANGE_EN'] = self.LONGRANGE_EN
        self.ENHDSSS_EN = RM_Field_SEQ_NS_MODEMINFO_ENHDSSS_EN(self)
        self.zz_fdict['ENHDSSS_EN'] = self.ENHDSSS_EN
        self.FSW_EN = RM_Field_SEQ_NS_MODEMINFO_FSW_EN(self)
        self.zz_fdict['FSW_EN'] = self.FSW_EN
        self.SOFTMODEM_EN = RM_Field_SEQ_NS_MODEMINFO_SOFTMODEM_EN(self)
        self.zz_fdict['SOFTMODEM_EN'] = self.SOFTMODEM_EN
        self.BTC_EN = RM_Field_SEQ_NS_MODEMINFO_BTC_EN(self)
        self.zz_fdict['BTC_EN'] = self.BTC_EN
        self.RXDC2CH_EN = RM_Field_SEQ_NS_MODEMINFO_RXDC2CH_EN(self)
        self.zz_fdict['RXDC2CH_EN'] = self.RXDC2CH_EN
        self.SPARE0 = RM_Field_SEQ_NS_MODEMINFO_SPARE0(self)
        self.zz_fdict['SPARE0'] = self.SPARE0
        self.__dict__['zz_frozen'] = True


class RM_Register_SEQ_NS_MISC(Base_RM_Register):
    def __init__(self, rmio, label):
        self.__dict__['zz_frozen'] = False
        super(RM_Register_SEQ_NS_MISC, self).__init__(rmio, label,
            0xb0000000, 0x018,
            'MISC', 'SEQ_NS.MISC', 'read-write',
            u"",
            0x00000000, 0x00000000,
            0x00001000, 0x00002000,
            0x00003000)

        self.RESERVED10 = RM_Field_SEQ_NS_MISC_RESERVED10(self)
        self.zz_fdict['RESERVED10'] = self.RESERVED10
        self.SYNTH_MODE_TX00D_EN = RM_Field_SEQ_NS_MISC_SYNTH_MODE_TX00D_EN(self)
        self.zz_fdict['SYNTH_MODE_TX00D_EN'] = self.SYNTH_MODE_TX00D_EN
        self.IFPKDOFF_AFTER_FRAME_EN = RM_Field_SEQ_NS_MISC_IFPKDOFF_AFTER_FRAME_EN(self)
        self.zz_fdict['IFPKDOFF_AFTER_FRAME_EN'] = self.IFPKDOFF_AFTER_FRAME_EN
        self.RFPKDOFF_AFTER_FRAME_EN = RM_Field_SEQ_NS_MISC_RFPKDOFF_AFTER_FRAME_EN(self)
        self.zz_fdict['RFPKDOFF_AFTER_FRAME_EN'] = self.RFPKDOFF_AFTER_FRAME_EN
        self.RESERVED2 = RM_Field_SEQ_NS_MISC_RESERVED2(self)
        self.zz_fdict['RESERVED2'] = self.RESERVED2
        self.RESERVED1 = RM_Field_SEQ_NS_MISC_RESERVED1(self)
        self.zz_fdict['RESERVED1'] = self.RESERVED1
        self.RESERVED0 = RM_Field_SEQ_NS_MISC_RESERVED0(self)
        self.zz_fdict['RESERVED0'] = self.RESERVED0
        self.__dict__['zz_frozen'] = True


class RM_Register_SEQ_NS_PHYINFO(Base_RM_Register):
    def __init__(self, rmio, label):
        self.__dict__['zz_frozen'] = False
        super(RM_Register_SEQ_NS_PHYINFO, self).__init__(rmio, label,
            0xb0000000, 0x01C,
            'PHYINFO', 'SEQ_NS.PHYINFO', 'read-write',
            u"",
            0x00000000, 0x00000000,
            0x00001000, 0x00002000,
            0x00003000)

        self.ADDRESS = RM_Field_SEQ_NS_PHYINFO_ADDRESS(self)
        self.zz_fdict['ADDRESS'] = self.ADDRESS
        self.__dict__['zz_frozen'] = True


