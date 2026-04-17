
# -*- coding: utf-8 -*-

__all__ = [ 'RM_Peripheral_SEQ_S' ]

from . static import Base_RM_Peripheral
from . SEQ_S_register import *

class RM_Peripheral_SEQ_S(Base_RM_Peripheral):
    def __init__(self, rmio, label):
        self.__dict__['zz_frozen'] = False
        super(RM_Peripheral_SEQ_S, self).__init__(rmio, label,
            0xA0000000, 'SEQ_S',
            u"",
            [])
        self.MMDDENOMINIT_CALC = RM_Register_SEQ_S_MMDDENOMINIT_CALC(self.zz_rmio, self.zz_label)
        self.zz_rdict['MMDDENOMINIT_CALC'] = self.MMDDENOMINIT_CALC
        self.MMDDENOMINIT_CALC_DOUBLED = RM_Register_SEQ_S_MMDDENOMINIT_CALC_DOUBLED(self.zz_rmio, self.zz_label)
        self.zz_rdict['MMDDENOMINIT_CALC_DOUBLED'] = self.MMDDENOMINIT_CALC_DOUBLED
        self.MODINDEX_CALC = RM_Register_SEQ_S_MODINDEX_CALC(self.zz_rmio, self.zz_label)
        self.zz_rdict['MODINDEX_CALC'] = self.MODINDEX_CALC
        self.MODINDEX_CALC_MODINDEXE_DOUBLED = RM_Register_SEQ_S_MODINDEX_CALC_MODINDEXE_DOUBLED(self.zz_rmio, self.zz_label)
        self.zz_rdict['MODINDEX_CALC_MODINDEXE_DOUBLED'] = self.MODINDEX_CALC_MODINDEXE_DOUBLED
        self.FSWCOREINFO = RM_Register_SEQ_S_FSWCOREINFO(self.zz_rmio, self.zz_label)
        self.zz_rdict['FSWCOREINFO'] = self.FSWCOREINFO
        self.MODEMINFO = RM_Register_SEQ_S_MODEMINFO(self.zz_rmio, self.zz_label)
        self.zz_rdict['MODEMINFO'] = self.MODEMINFO
        self.MISC = RM_Register_SEQ_S_MISC(self.zz_rmio, self.zz_label)
        self.zz_rdict['MISC'] = self.MISC
        self.PHYINFO = RM_Register_SEQ_S_PHYINFO(self.zz_rmio, self.zz_label)
        self.zz_rdict['PHYINFO'] = self.PHYINFO
        self.__dict__['zz_frozen'] = True