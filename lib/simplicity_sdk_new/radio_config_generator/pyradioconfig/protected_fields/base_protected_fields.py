

class ProtectedFieldsBase():

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
    PTE_PROTECTED_FIELD_DICT = {}

    # : List of fields found in PTE, but allow radio configurator to override the value
    PTE_PROTECTED_FIELD_EXCEPTIONS = []

    # : List of registers that Radio Configurator should not write to based on RAIL request
    RAIL_PROTECTED_REGS = [
        'SYNTH_CHCTRL',
        'FRC_BLOCKRAMADDR'
    ]

    # : List of fields protected by RAIL
    RAIL_PROTECTED_FIELDS = []

    # : List of fields that are found in PTE list, but does not need to be protected by RAIL
    RAIL_PROTECTED_FIELD_EXCEPTIONS = []

    def get_rail_protected_field_dict(self):
        """
        rail_script expects protected fields in dictionary form
        key = register, i.e. "PERPH.REG"
        value = list of field values, i.e. ["Field1", "Field2"]

        Returns: dictionary

        """
        rail_protected_list = self.get_rail_protected_field_list()
        rail_protected_dict = {}
        for reg_field_name in rail_protected_list:
            reg_name = str.split(reg_field_name, "_")[0]+"."+str.split(reg_field_name, "_")[1]
            field_name = str.split(reg_field_name, "_")[2]

            if reg_name not in rail_protected_dict.keys():
                rail_protected_dict[reg_name] = []

            rail_protected_dict[reg_name].append(field_name)
        return rail_protected_dict

    def get_rail_protected_field_list(self):
        """
        Get list of protected fields, i.e.
        ["REG_PERPH_FIELD1", "REG_PERPH_FIELD2"]

        Returns: list

        """
        rail_protected_list = []

        # : Create list containing all PTE cals + RAIL protected fields
        for reg_field_name_list in [self.get_pte_cal_field_list(), self.RAIL_PROTECTED_FIELDS]:
            for reg_field_name in reg_field_name_list:
                if reg_field_name not in self.RAIL_PROTECTED_FIELD_EXCEPTIONS:
                    rail_protected_list.append(reg_field_name)

        return rail_protected_list

    def get_pte_cal_field_list(self, remove_exceptions=True):
        """
        Get list of all PTE calibrated fields, i.e.
        ["REG_PERPH_FIELD1", "REG_PERPH_FIELD2"]

        Returns: list

        """
        pte_cal_field_list = []

        for reg_field_name, protected_type in self.PTE_PROTECTED_FIELD_DICT.items():
            if protected_type == "cal":
                if remove_exceptions:
                    add_to_list = reg_field_name not in self.PTE_PROTECTED_FIELD_EXCEPTIONS
                else:
                    add_to_list = True
                if add_to_list:
                    pte_cal_field_list.append(reg_field_name)

        return pte_cal_field_list

    def get_pte_set_and_forget_list(self, remove_exceptions=True):
        """
        Get list of all PTE set and forget fields, i.e.
        ["REG_PERPH_FIELD1", "REG_PERPH_FIELD2"]

        Returns:

        """
        pte_set_and_forget_field_list = []

        for reg_field_name, protected_type in self.PTE_PROTECTED_FIELD_DICT.items():
            if protected_type != "cal":
                if remove_exceptions:
                    add_to_list = reg_field_name not in self.PTE_PROTECTED_FIELD_EXCEPTIONS
                else:
                    add_to_list = True
                if add_to_list:
                    pte_set_and_forget_field_list.append(reg_field_name)

        return pte_set_and_forget_field_list