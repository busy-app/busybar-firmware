
# -*- coding: utf-8 -*-

__all__ = [ 'RM_Peripheral_SEQ_NS' ]

from . static import Base_RM_Peripheral
from . SEQ_NS_register import *

class RM_Peripheral_SEQ_NS(Base_RM_Peripheral):
    def __init__(self, rmio, label):
        self.__dict__['zz_frozen'] = False
        super(RM_Peripheral_SEQ_NS, self).__init__(rmio, label,
            0xB0000000, 'SEQ_NS',
            u"",
            [])
        self.MMDDENOMINIT_CALC = RM_Register_SEQ_NS_MMDDENOMINIT_CALC(self.zz_rmio, self.zz_label)
        self.zz_rdict['MMDDENOMINIT_CALC'] = self.MMDDENOMINIT_CALC
        self.MMDDENOMINIT_CALC_DOUBLED = RM_Register_SEQ_NS_MMDDENOMINIT_CALC_DOUBLED(self.zz_rmio, self.zz_label)
        self.zz_rdict['MMDDENOMINIT_CALC_DOUBLED'] = self.MMDDENOMINIT_CALC_DOUBLED
        self.MODINDEX_CALC = RM_Register_SEQ_NS_MODINDEX_CALC(self.zz_rmio, self.zz_label)
        self.zz_rdict['MODINDEX_CALC'] = self.MODINDEX_CALC
        self.MODINDEX_CALC_MODINDEXE_DOUBLED = RM_Register_SEQ_NS_MODINDEX_CALC_MODINDEXE_DOUBLED(self.zz_rmio, self.zz_label)
        self.zz_rdict['MODINDEX_CALC_MODINDEXE_DOUBLED'] = self.MODINDEX_CALC_MODINDEXE_DOUBLED
        self.FSWCOREINFO = RM_Register_SEQ_NS_FSWCOREINFO(self.zz_rmio, self.zz_label)
        self.zz_rdict['FSWCOREINFO'] = self.FSWCOREINFO
        self.MODEMINFO = RM_Register_SEQ_NS_MODEMINFO(self.zz_rmio, self.zz_label)
        self.zz_rdict['MODEMINFO'] = self.MODEMINFO
        self.MISC = RM_Register_SEQ_NS_MISC(self.zz_rmio, self.zz_label)
        self.zz_rdict['MISC'] = self.MISC
        self.PHYINFO = RM_Register_SEQ_NS_PHYINFO(self.zz_rmio, self.zz_label)
        self.zz_rdict['PHYINFO'] = self.PHYINFO
        self.__dict__['zz_frozen'] = True