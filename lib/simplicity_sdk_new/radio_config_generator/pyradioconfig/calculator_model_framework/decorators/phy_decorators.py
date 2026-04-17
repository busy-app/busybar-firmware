import re
from enum import Enum
from functools import wraps
from pyradioconfig.calculator_model_framework.CalcManager import CalcManager
from pycalcmodel.core.output import ModelOutputType

def phy_guid(guid):
    def inner_decorator(f):
        @wraps(f)
        def wrapped(*args, **kwargs):
            # print('before function')
            class_ref = args[0]
            model = args[1]
            if not hasattr(class_ref, 'phy_guids') or class_ref.phy_guids is None:
                class_ref.phy_guids = dict()
            class_ref.phy_guids[model.part_family, f.__name__] = guid
            response = f(*args, **kwargs)
            # print('after function')
            return response
        # print('decorating', f, 'with argument', guid)
        # wrapped.phy_guid = guid
        setattr(wrapped, 'phy_guid', guid)
        return wrapped
    return inner_decorator

def do_not_inherit_phys(cls):

    # Get a list of all of the PHY methods in the class (including inherited)
    inherited_methods = cls().getPhyList()
    inherited_method_names = [method.__name__ for method in inherited_methods]

    # Get a list of methods locally defined in the child class
    child_methods = cls.__dict__.items()
    child_method_names = [method[0] for method in child_methods if 'phy_' in method[0].lower()]

    # Form a list of methods we need to pass
    pass_method_names = [inherited_method_name for inherited_method_name in inherited_method_names if
                         inherited_method_name not in child_method_names]

    # Now add each pass method to the class
    for method_name in pass_method_names:
        setattr(cls, method_name, _phypass)

    return cls

def do_not_inherit_prod_phys(cls):

    # Get a list of all of the PHY methods in the class (including inherited)
    # assuming we only inherit from one class, or have a single parent, extract all methods from the parent class
    inherited_methods = cls.__bases__[0]().getPhyList()
    inherited_method_names = [method.__name__ for method in inherited_methods]

    # Get a list of methods locally defined in the child class
    child_methods = cls.__dict__.items()
    child_method_names = [method[0] for method in child_methods if 'phy_' in method[0].lower()]

    # Form a list of methods we need to pass all PHYs with name prod in them not defined in the child class
    pass_method_names = [prod_phy_names for prod_phy_names in inherited_method_names if
                         ((prod_phy_names not in child_method_names) and str(prod_phy_names).__contains__('_prod'))]

    # Now add each pass method to the class
    for method_name in pass_method_names:
        setattr(cls, method_name, _phypass)

    return cls

def _phypass(self, model, phy_name=None):
    pass

def concurrent_phy(phy_name,reg_field_list,override_dict=None): #decorator maker
    if override_dict is None:
        override_dict = {}
    def inner_decorator(f):
        @wraps(f)
        def wrapped(*args, **kwargs):

            #First call the decorated PHY method (the PHY is not calculated yet, just inputs loaded)
            class_ref = args[0]
            model = args[1]
            phy = f(*args, **kwargs)

            #Now generate a calculator model for the second (concurrent) PHY
            concurrent_model = CalcManager(part_family=model.part_family, part_rev=model.part_revision, target=model.target).calculate_phy(phy_name=phy_name,optional_inputs=override_dict)

            #Assign special concurrent PHY variables
            if hasattr(concurrent_model.vars,'min_if_hz'):
                model.vars.alt_min_if_hz.value_forced = concurrent_model.vars.min_if_hz.value
                model.vars.alt_softmodem_used.value_forced = 1 if \
                    concurrent_model.vars.demod_select.value == concurrent_model.vars.demod_select.var_enum.SOFT_DEMOD else 0
                model.vars.alt_wisun_mode_switch_phr.value_forced = concurrent_model.vars.wisun_mode_switch_phr.value
                model.vars.alt_wisun_phy_mode_id.value_forced = concurrent_model.vars.wisun_phy_mode_id.value
                model.vars.alt_stack_info.value_forced = concurrent_model.vars.stack_info.value
                model.vars.alt_rssi_adjust_db.value_forced = concurrent_model.vars.rssi_adjust_db.value
                model.vars.softmodem_antdivmode.value_forced = concurrent_model.vars.softmodem_antdivmode.value

            #Finally loop go through the Profile Outputs for the concurrent PHY and copy them over
            for reg_field in reg_field_list:
                for profile_output in concurrent_model.profile.outputs:
                    if (profile_output.output_type == ModelOutputType.SVD_REG_FIELD) and (re.search("^"+reg_field.lower(),profile_output.var_name.lower())):
                        original_phy_profile_output = getattr(phy.profile_outputs, profile_output.var_name)
                        if original_phy_profile_output.override is None: #If already forced then use that value
                            original_phy_profile_output.override = profile_output.var_value
            # Set this just in case to ensure consistency
            for override_name, override_value in override_dict.items():
                if hasattr(phy.profile_inputs, override_name):
                    profile_input = getattr(phy.profile_inputs, override_name)
                    if profile_input._var.var_type == Enum:
                        enum_val = getattr(profile_input._var.var_enum, override_value)
                        profile_input.value = enum_val
                    else:
                        profile_input.value = override_value

            return phy
        return wrapped
    return inner_decorator

