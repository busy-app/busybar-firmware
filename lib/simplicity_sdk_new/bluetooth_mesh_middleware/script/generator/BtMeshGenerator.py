import enum
import json
import re
import traceback
from pathlib import Path
from typing import (Callable, Dict, Iterable, List, NamedTuple, Optional, Set,
                    Union)

from jinja2 import FileSystemLoader
from jinja2.environment import Environment

if __name__ == '__main__':
    from BtMeshLogger import ErrorLevel, GeneratorLogger
else:
    from .BtMeshLogger import ErrorLevel, GeneratorLogger

BTMESH_GEN_DIR_PATH = Path(__file__).parent
BTMESH_SIG_MDL_DESC_NAME = "btmesh_sig_model_descriptors.json"
BTMESH_SIG_MDL_DESC_PATH = BTMESH_GEN_DIR_PATH / BTMESH_SIG_MDL_DESC_NAME


def to_c_macro(name):
    macro = re.sub("\W", "_", name)
    macro = macro + ("_" if macro[0].isdigit() else "")
    return macro.upper()


class ModelUtil:
    CID_SIG_MDL = 0xFFFF
    MULTIPLICITY_UNLIMITED = 0xFFFFFFFF
    CORRESPONDING_GROUP_MAX_COUNT = 256
    MDL_ITEM_SHORT_ELEM_OFFSET_MIN = -4
    MDL_ITEM_SHORT_ELEM_OFFSET_MAX = 3
    MDL_ITEM_SHORT_MDL_IDX_MIN = 0
    MDL_ITEM_SHORT_MDL_IDX_MAX = 31
    MDL_ITEM_LONG_ELEM_OFFSET_MIN = -128
    MDL_ITEM_LONG_ELEM_OFFSET_MAX = 127
    MDL_ITEM_LONG_MDL_IDX_MIN = 0
    MDL_ITEM_LONG_MDL_IDX_MAX = 255

    @classmethod
    def validate_model_item_short_elem_offset(cls, elem_offset: int):
        if (
            elem_offset < cls.MDL_ITEM_SHORT_ELEM_OFFSET_MIN
            or elem_offset > cls.MDL_ITEM_SHORT_ELEM_OFFSET_MAX
        ):
            GeneratorLogger.add_marker(ErrorLevel.ERROR,
                f"Model item element offset ({elem_offset}) is not in supported "
                f"[{cls.MDL_ITEM_SHORT_ELEM_OFFSET_MIN}, {cls.MDL_ITEM_SHORT_ELEM_OFFSET_MAX}]"
                f" range of short format."
            )

    @classmethod
    def validate_model_item_long_elem_offset(cls, elem_offset: int):
        if (
            elem_offset < cls.MDL_ITEM_LONG_ELEM_OFFSET_MIN
            or elem_offset > cls.MDL_ITEM_LONG_ELEM_OFFSET_MAX
        ):
            GeneratorLogger.add_marker(ErrorLevel.ERROR,
                f"Model item element offset ({elem_offset}) is not in supported "
                f"[{cls.MDL_ITEM_LONG_ELEM_OFFSET_MIN}, {cls.MDL_ITEM_LONG_ELEM_OFFSET_MAX}]"
                f" range of long format."
            )

    @classmethod
    def validate_model_item_short_model_index(cls, mdl_idx: int):
        if (
            mdl_idx < cls.MDL_ITEM_SHORT_MDL_IDX_MIN
            or mdl_idx > cls.MDL_ITEM_SHORT_MDL_IDX_MAX
        ):
            GeneratorLogger.add_marker(ErrorLevel.ERROR,
                f"Model item element offset ({mdl_idx}) is not in supported "
                f"[{cls.MDL_ITEM_SHORT_MDL_IDX_MIN}, {cls.MDL_ITEM_SHORT_MDL_IDX_MAX}]"
                f" range of short format."
            )

    @classmethod
    def validate_model_item_long_model_index(cls, mdl_idx: int):
        if (
            mdl_idx < cls.MDL_ITEM_LONG_MDL_IDX_MIN
            or mdl_idx > cls.MDL_ITEM_LONG_MDL_IDX_MAX
        ):
            GeneratorLogger.add_marker(ErrorLevel.ERROR,
                f"Model item element offset ({mdl_idx}) is not in supported "
                f"[{cls.MDL_ITEM_LONG_MDL_IDX_MIN}, {cls.MDL_ITEM_LONG_MDL_IDX_MAX}]"
                f" range of long format."
            )

    @staticmethod
    def twos_complement(value, bit_count):
        """Compute the 2's complement of int value val"""
        MIN_VALUE = -(1 << (bit_count - 1))
        MAX_VALUE = (1 << (bit_count - 1)) - 1
        if not (MIN_VALUE <= value <= MAX_VALUE):
            GeneratorLogger.add_marker(ErrorLevel.ERROR,
                f"The {value} signed integer value can't be "
                f"represented in {bit_count} bits."
            )
        if value < 0:
            value = (1 << bit_count) + value
        return value


class ModelDescriptorRelation:
    def __init__(self, element_constraint: str, model_name: str = None, mid: str = None, cid: str = hex(ModelUtil.CID_SIG_MDL)):
        if model_name:
            self.model_name = model_name.strip()
        else:
            self.model_name = ""
            self.mid = int(mid,16)
            self.cid = int(cid,16)
        self.element_constraint = element_constraint.strip()

    def __repr__(self) -> str:
        return (
            f"{type(self).__name__}("
            f"model_name={self.model_name}, "
            f"element_constraint={self.element_constraint})"
        )


class ModelDescriptor(object):
    def __init__(
        self,
        mid: int,
        name: str,
        multiplicity: str,
        element_constraint: str,
        publication_support: bool = True,
        subscription_support: bool = True,
        security: str = "mixed",
        corresponds=[],
        extends=[],
        cid: int = None,
        metadata_headers: List[str] = [],
        metadata_info: Dict[str, int] = {}
    ):
        # Model ID conversion to int from string (bin, oct, dec, hex)
        self.mid = int(mid, 0)
        if cid is None:
            # If company ID is not provided then it is interpreted as an SIG Model
            self.cid = ModelUtil.CID_SIG_MDL
        else:
            # Company ID conversion to int from string (bin, oct, dec, hex)
            self.cid = int(cid, 0)
        # Model name from specification
        self.name = name.strip()
        self.multiplicity = multiplicity.strip()
        self.element_constraint = element_constraint.strip()
        self.publication_support = publication_support
        self.subscription_support = subscription_support
        self.security = security.strip()
        self.corresponds = []
        self.extends = []
        for c in corresponds:
            if c.get("cid"):
                # If CID is present, it's most likely a vendor model
                self.corresponds.append(ModelDescriptorRelation(c["element_constraint"], None, c["mid"], c["cid"]))
            elif c.get("mid"):
                # If CID is not present, it's an SIG model
                self.corresponds.append(ModelDescriptorRelation(c["element_constraint"], None, c["mid"]))
            elif c.get("model"):
                # If only model name is present, it should be an SIG model (but still work if all Vendor Models have unique names)
                self.corresponds.append(ModelDescriptorRelation(c["element_constraint"], c["model"]))
        for e in extends:
            if e.get("cid"):
                # If CID is present, it's most likely a vendor model
                self.extends.append(ModelDescriptorRelation(e["element_constraint"], None, e["mid"], e["cid"]))
            elif e.get("mid"):
                # If CID is not present, it's an SIG model
                self.extends.append(ModelDescriptorRelation(e["element_constraint"], None, e["mid"]))
            elif e.get("model"):
                # If only model name is present, it should be an SIG model (but still work if all Vendor Models have unique names)
                self.extends.append(ModelDescriptorRelation(e["element_constraint"], e["model"]))
        
        # Metadata header is the (list of) .h files ("sl_btmesh_xy_config.h")
        self.metadata_headers = metadata_headers
        # Metadata info is a dict of {"MACRO_NAME" : metadata lenght} pairs
        self.metadata_info = metadata_info

    def __str__(self) -> str:
        return f"{self.name} (cid=0x{self.cid:04X}, mid=0x{self.mid:04X})"


class ModelElemConstraint(enum.Enum):
    PRIMARY = 0
    PRIMARY_ANY = 1
    ANY = 2


class ModelRelationElemConstraint(enum.Enum):
    SAME = 0
    OTHER_SMALLER_ELEM_INDEX = 1
    OTHER_LARGER_ELEM_INDEX = 2


class ModelSecurity(enum.Enum):
    DEVKEY = 0
    APPKEY = 1
    MIXED = 3


class ModelRelationType(enum.Enum):
    CORRESPONDS = 0
    EXTENDS = 1


class VendorModelId(NamedTuple):
    cid: int
    mid: int

    @property
    def value(self) -> int:
        return self.mid << 16 + self.cid

    def __repr__(self) -> str:
        return f"{type(self).__name__}(cid=0x{self.cid:04X}, mid=0x{self.mid:04X})"


class ModelInstId(NamedTuple):
    elem_index: int
    cid: int
    mid: int

    def __repr__(self) -> str:
        return (
            f"{type(self).__name__}(elem_index={self.elem_index}, "
            f"cid=0x{self.cid:04X}, mid=0x{self.mid:04X})"
        )


class ModelRelation:
    def __init__(
        self,
        model: "Model",
        elem_constraint: ModelRelationElemConstraint,
        relation_type: ModelRelationType,
    ):
        self.model = model
        self.elem_constraint = elem_constraint
        self.relation_type = relation_type


class Model:
    def __init__(
        self,
        mid: int,
        name: str,
        multiplicity_lower: int = 0,
        multiplicity_upper: int = ModelUtil.MULTIPLICITY_UNLIMITED,
        elem_constraint: ModelElemConstraint = ModelElemConstraint.ANY,
        publication_support: bool = True,
        subscription_support: bool = True,
        security: ModelSecurity = ModelSecurity.APPKEY,
        corresponds: Iterable[ModelRelation] = [],
        extends: Iterable[ModelRelation] = [],
        extended_by: Iterable["Model"] = [],
        cid: int = ModelUtil.CID_SIG_MDL,
        metadata_headers: List[str] = [],
        metadata_info: Dict[str, int] = {}
    ):
        self.cid = cid
        self.mid = mid
        self.name = name
        self.multiplicity_lower = multiplicity_lower
        self.multiplicity_upper = multiplicity_upper
        self.elem_constraint = elem_constraint
        self.publication_support = publication_support
        self.subscription_support = subscription_support
        self.security = security
        self.metadata_headers = metadata_headers
        self.metadata_info = metadata_info
        self._corresponds = []
        self._extends = []
        self._extended_by = []
        for mdl_relation in corresponds:
            self.add_corresponds(mdl_relation)
        for mdl_relation in extends:
            self.add_extends(mdl_relation)
        for mdl in extended_by:
            self.add_extends(mdl)

    @property
    def cid(self) -> int:
        return self._cid

    @cid.setter
    def cid(self, value) -> None:
        if not (0 <= value <= 0xFFFF):
            GeneratorLogger.add_marker(ErrorLevel.ERROR,f"Invalid company id: 0x{value:04X}.")
        self._cid = value

    @property
    def mid(self) -> int:
        return self._mid

    @mid.setter
    def mid(self, value) -> None:
        if not (0 <= value <= 0xFFFF):
            GeneratorLogger.add_marker(ErrorLevel.ERROR,f"Invalid model id: 0x{value:04X}.")
        self._mid = value

    @property
    def name(self) -> str:
        return self._name

    @name.setter
    def name(self, value) -> None:
        self._name = value

    @property
    def multiplicity_lower(self) -> int:
        return self._multiplicity_lower

    @multiplicity_lower.setter
    def multiplicity_lower(self, value) -> None:
        if value < 0:
            GeneratorLogger.add_marker(ErrorLevel.ERROR,f"Invalid multiplicity: {value}.")
        self._multiplicity_lower = value

    @property
    def multiplicity_upper(self) -> int:
        return self._multiplicity_upper

    @multiplicity_upper.setter
    def multiplicity_upper(self, value) -> None:
        if value < 0:
            GeneratorLogger.add_marker(ErrorLevel.ERROR,f"Invalid multiplicity: {value}.")
        self._multiplicity_upper = value

    @property
    def elem_constraint(self) -> ModelElemConstraint:
        return self._elem_constraint

    @elem_constraint.setter
    def elem_constraint(self, value) -> None:
        assert isinstance(value, ModelElemConstraint)
        self._elem_constraint = value

    @property
    def publication_support(self) -> bool:
        return self._publication_support

    @publication_support.setter
    def publication_support(self, value) -> None:
        self._publication_support = value

    @property
    def subscription_support(self) -> bool:
        return self._subscription_support

    @subscription_support.setter
    def subscription_support(self, value) -> None:
        self._subscription_support = value

    @property
    def security(self) -> ModelSecurity:
        return self._security

    @security.setter
    def security(self, value) -> None:
        assert isinstance(value, ModelSecurity)
        self._security = value

    @property
    def corresponds(self) -> Iterable[ModelRelation]:
        yield from self._corresponds

    @property
    def extends(self) -> Iterable[ModelRelation]:
        yield from self._extends

    @property
    def extended_by(self) -> Iterable["Model"]:
        yield from self._extended_by

    def has_corresponding_models(self) -> bool:
        return len(self._corresponds) != 0

    def has_extended_models(self) -> bool:
        return len(self._extends) != 0

    def is_extended_by_models(self) -> bool:
        return len(self._extended_by) != 0

    def add_corresponds(self, model_relation: ModelRelation) -> None:
        if model_relation not in self._corresponds:
            self._corresponds.append(model_relation)

    def add_extends(self, model_relation: ModelRelation) -> None:
        if model_relation not in self._extends:
            self._extends.append(model_relation)
            model_relation.model.add_extended_by(self)

    def add_extended_by(self, model: "Model") -> None:
        if model not in self._extended_by:
            self._extended_by.append(model)

    def link_extended_model(self, model_relation: ModelRelation) -> None:
        self.add_extends(model_relation)
        model_relation.model.add_extended_by(self)

    @property
    def vmid(self):
        return VendorModelId(cid=self.cid, mid=self.mid)

    def is_sig_model(self):
        return self.cid == ModelUtil.CID_SIG_MDL

    def is_vendor_model(self):
        return not self.is_sig_model()

    @property
    def macro_name(self):
        return to_c_macro(self.name)

    def __eq__(self, other):
        if isinstance(other, Model):
            return self.vmid == other.vmid
        return NotImplemented

    def __str__(self) -> str:
        return f"{self.name} (cid=0x{self.cid:04X}, mid=0x{self.mid:04X})"


class CorrespondingGroup:
    next_unique_group_id = 0

    @classmethod
    def get_unique_group_id(cls) -> int:
        if cls.next_unique_group_id >= ModelUtil.CORRESPONDING_GROUP_MAX_COUNT:
            GeneratorLogger.add_marker(ErrorLevel.ERROR,
                f"Max corresponding group count is exceeded. "
                f"({ModelUtil.CORRESPONDING_GROUP_MAX_COUNT})"
            )
        group_id = cls.next_unique_group_id
        cls.next_unique_group_id += 1
        return group_id

    def __init__(self, mdl_instances: Iterable["ModelInst"]):
        self.id = self.get_unique_group_id()
        self.id_mdl_inst_dict: Dict[ModelInstId, ModelInst] = {}
        for mdl_inst in mdl_instances:
            self.add_mdl_inst(mdl_inst)

    def add_mdl_inst(self, mdl_inst: "ModelInst") -> None:
        self.id_mdl_inst_dict[mdl_inst.id] = mdl_inst
        mdl_inst.corresponding_group = self

    def has_mdl_inst(self, mdl_inst: "ModelInst") -> bool:
        return mdl_inst.id in self.id_mdl_inst_dict

    def has_mdl_inst_with_id(self, mdl_inst_id: ModelInstId) -> bool:
        return mdl_inst_id in self.id_mdl_inst_dict

    def has_mdl_inst_with_vmid(self, vmid: VendorModelId) -> bool:
        return vmid in (mdl_inst.vmid for mdl_inst in self.id_mdl_inst_dict.values())

    def has_mdl_inst_with_name(self, name: str) -> bool:
        return name in (mdl_inst.name for mdl_inst in self.id_mdl_inst_dict.values())

    def __contains__(self, item) -> bool:
        if isinstance(item, ModelInst):
            return self.has_mdl_inst(item)
        elif isinstance(item, ModelInstId):
            return self.has_mdl_inst_with_id(item)
        elif isinstance(item, VendorModelId):
            return self.has_mdl_inst_with_vmid(item)
        elif isinstance(item, str):
            # If the item is string then it shall be the name of the model instance
            return self.has_mdl_inst_with_name(item)
        else:
            raise TypeError(
                f"The {type(item)} is not supported in __contains__ method "
                f"of {type(self).__class__} class."
            )

    @property
    def mdl_insts(self) -> Iterable["ModelInst"]:
        yield from self.id_mdl_inst_dict.values()


class ModelItemFormat(enum.IntEnum):
    SHORT = 0
    LONG = 1

    def bin(self) -> int:
        return self.value << 1


class ExtendedModelItem:
    ELEM_OFFSET_SHORT_BIT_CNT = 3
    ELEM_OFFSET_LONG_BIT_CNT = 8
    MDL_IDX_SHORT_SHIFT = 3

    def __init__(self, elem_offset: int, mdl_item_index: int):
        ModelUtil.validate_model_item_long_elem_offset(elem_offset)
        ModelUtil.validate_model_item_long_model_index(mdl_item_index)
        self._elem_offset = elem_offset
        self._mdl_item_index = mdl_item_index

    @property
    def elem_offset(self) -> int:
        return self._elem_offset

    @property
    def elem_offset_short_bin(self) -> int:
        ModelUtil.validate_model_item_short_elem_offset(self.elem_offset)
        return ModelUtil.twos_complement(self.elem_offset, self.ELEM_OFFSET_SHORT_BIT_CNT)

    @property
    def elem_offset_long_bin(self) -> int:
        return ModelUtil.twos_complement(self.elem_offset, self.ELEM_OFFSET_LONG_BIT_CNT)

    @property
    def model_index(self) -> int:
        return self._mdl_item_index

    @property
    def model_index_short_bin(self) -> int:
        ModelUtil.validate_model_item_short_model_index(self.model_index)
        return self.model_index << self.MDL_IDX_SHORT_SHIFT

    @property
    def model_item_index_long_bin(self) -> int:
        return self.model_index

    @property
    def model_item_short_bin(self) -> int:
        return self.model_index_short_bin | self.elem_offset_short_bin


class ModelInst:
    def __init__(
        self,
        mid: Union[str, int],
        name: str = "",
        elem: Optional["Element"] = None,
        mdl_index: int = 0,
        corresponding_group: Optional[CorrespondingGroup] = None,
        extends: Iterable["ModelInst"] = [],
        extended_by: Iterable["ModelInst"] = [],
        cid: int = ModelUtil.CID_SIG_MDL,
    ):
        # Perform str to int conversion on company and model ids if it is necessary.
        if isinstance(mid, str):
            mid = int(mid, 0)
        if isinstance(cid, str):
            cid = int(cid, 0)
        # Model name is queried by company and model id so the name of the model
        # instance is ignored unless an unknown vendor model is found in .btmeshconf
        # or .dcd files. Unknown vendor model means that it is not present in
        # ModelCollection loaded from model descriptor json files.
        # A new model is created and added to ModelCollection for unknown vendor
        # models with instance model name found in .btmeshconf or .dcd file.
        # This is important for backward compatibility because customers may
        # have their vendor models in .btmeshconf already.
        if ModelCollection.model_ids_exist(cid=cid, mid=mid):
            mdl = ModelCollection.get_model_by_ids(cid=cid, mid=mid)
        else:
            if cid == ModelUtil.CID_SIG_MDL:
                # All SIG models shall be listed in model descriptor json files
                GeneratorLogger.add_marker(ErrorLevel.ERROR,
                    f"No SIG model exists with 0x{mid:04X} identifier. "
                    f"List and content of loaded model descriptor json files "
                    f"shall be checked."
                )
            else:
                # It is not mandatory to list all vendor models in model descriptor
                # json files. It is necessary only when one models extends or
                # corresponds to another vendor model.
                if not name:
                    name = f"VendorModel_cid_{cid:04X}_mid_{mid:04X}"
                mdl = Model(cid=cid, mid=mid, name=name)
                ModelCollection.add_model(mdl)
        self.mdl = mdl
        self.elem = elem
        self.mdl_index = mdl_index
        self.corresponding_group = corresponding_group
        self._extends = []
        for mdl_inst in extends:
            self.add_extends(mdl_inst)
        self._extended_by = []
        for mdl_inst in extended_by:
            self.add_extends(mdl_inst)

    @property
    def mdl(self) -> Model:
        return self._mdl

    @mdl.setter
    def mdl(self, value) -> None:
        assert isinstance(value, Model)
        self._mdl = value

    @property
    def mid(self) -> int:
        return self.mdl.mid

    @property
    def cid(self) -> int:
        return self.mdl.cid

    @property
    def vmid(self) -> int:
        return self.mdl.vmid

    def is_sig_model(self):
        return self.mdl.is_sig_model()

    def is_vendor_model(self):
        return self.mdl.is_vendor_model()

    @property
    def id(self) -> ModelInstId:
        return ModelInstId(self.elem_index, self.cid, self.mid)

    @property
    def name(self) -> str:
        return self.mdl.name

    @property
    def elem(self) -> Optional["Element"]:
        return self._elem

    @elem.setter
    def elem(self, value) -> None:
        assert value is None or isinstance(value, Element)
        self._elem = value

    @property
    def elem_index(self) -> int:
        if self.elem is None:
            return 0
        else:
            return self.elem.elem_index

    @property
    def mdl_index(self) -> int:
        return self._mdl_index

    @mdl_index.setter
    def mdl_index(self, value) -> None:
        self._mdl_index = value

    @property
    def corresponding_group(self) -> CorrespondingGroup:
        return self._corresponding_group

    @corresponding_group.setter
    def corresponding_group(self, value: Optional[CorrespondingGroup]) -> None:
        assert value is None or isinstance(value, CorrespondingGroup)
        # Warning could be reported when model_inst isn't part of it.
        self._corresponding_group = value

    @property
    def corresponding_present(self) -> bool:
        return self.corresponding_group is not None

    @property
    def extends(self) -> Iterable["ModelInst"]:
        yield from self._extends

    @property
    def extends_count(self) -> int:
        return len(self._extends)

    def add_extends(self, model_inst: "ModelInst") -> None:
        if model_inst not in self._extends:
            self._extends.append(model_inst)

    @property
    def extended_model_items(self) -> Iterable[ExtendedModelItem]:
        return (self.get_extended_model_item(ext_mdl) for ext_mdl in self.extends)

    @property
    def extended_by(self) -> Iterable["ModelInst"]:
        yield from self.extended_by

    def add_extended_by(self, model_inst: "ModelInst") -> None:
        if model_inst not in self._extended_by:
            self._extended_by.append(model_inst)

    def link_extended_model_inst(self, model_inst: "ModelInst") -> None:
        if model_inst is None:
            pass
        else:
            self.add_extends(model_inst)
            model_inst.add_extended_by(self)

    @property
    def model_item_header(self) -> int:
        return self.get_model_item_header()

    def get_model_item_header(self, format: Optional[ModelItemFormat] = None) -> int:
        if format is None:
            if self.model_item_format_short_supported:
                format = ModelItemFormat.SHORT
            else:
                format = ModelItemFormat.LONG
        header = 0x01 if self.corresponding_present else 0x00
        header |= format.bin()
        header |= (self.get_extended_items_count() << 2) & 0xFC
        return header

    @property
    def model_item_format_short_supported(self) -> bool:
        return all(
            (
                self.elem_offset(mdl_inst) >= ModelUtil.MDL_ITEM_SHORT_ELEM_OFFSET_MIN
                and self.elem_offset(mdl_inst)
                <= ModelUtil.MDL_ITEM_SHORT_ELEM_OFFSET_MAX
                and mdl_inst.mdl_index >= ModelUtil.MDL_ITEM_SHORT_MDL_IDX_MIN
                and mdl_inst.mdl_index <= ModelUtil.MDL_ITEM_SHORT_MDL_IDX_MAX
            )
            for mdl_inst in self.extends
        )

    def elem_offset(self, other_mdl: "ModelInst") -> int:
        return other_mdl.elem_index - self.elem_index

    def get_extended_items_count(self) -> int:
        return self.extends_count

    def get_extended_model_item(self, extended_mdl: "ModelInst") -> ExtendedModelItem:
        elem_offset = self.elem_offset(extended_mdl)
        return ExtendedModelItem(
            elem_offset=elem_offset,
            mdl_item_index=extended_mdl.mdl_index,
        )

    def __eq__(self, other):
        if isinstance(other, ModelInst):
            return self.vmid == other.vmid
        return NotImplemented

    def __str__(self) -> str:
        return (
            f"{self.name} (elem_index={self.elem_index}, "
            f"cid=0x{self.cid:04X}, mid=0x{self.mid:04X})"
        )


class ModelCollection:
    MULTIPLICITY_PATTERN = r"\s*(?:\*|\d+|\d+\s*\.\.\s*[\d+\*])\s*"
    ELEM_CONSTRAINT_PATTERN = r"primary|primary_any|any"
    SECURITY_PATTERN = r"devkey|appkey|mixed"
    RELATION_ELEM_CONSTRAINT_PATTERN = (
        r"same|other_larger_elem_index|other_smaller_elem_index"
    )
    id_mdl_dict: Dict[VendorModelId, Model] = {}
    name_mdl_dict: Dict[VendorModelId, Model] = {}

    @classmethod
    def reset(cls):
        cls.id_mdl_dict.clear()
        cls.name_mdl_dict.clear()

    @classmethod
    def get_model_by_ids(cls, mid: int, cid: int = ModelUtil.CID_SIG_MDL):
        vmid = VendorModelId(cid, mid)
        return cls.get_model_by_vmid(vmid)

    @classmethod
    def get_model_by_vmid(cls, vmid: VendorModelId):
        return cls.id_mdl_dict[vmid]

    @classmethod
    def get_model_by_name(cls, name: str):
        return cls.name_mdl_dict[name]

    @classmethod
    def model_ids_exist(cls, mid: int, cid: int = ModelUtil.CID_SIG_MDL):
        vmid = VendorModelId(cid, mid)
        return vmid in cls.id_mdl_dict

    @classmethod
    def model_name_exists(cls, name: str):
        return name in cls.name_mdl_dict

    @classmethod
    def add_model(cls, mdl: Model):
        if mdl.vmid in cls.id_mdl_dict or mdl.name in cls.name_mdl_dict:
            GeneratorLogger.add_marker(ErrorLevel.ERROR,f"The {mdl} is duplicated.")
        cls.id_mdl_dict[mdl.vmid] = mdl
        cls.name_mdl_dict[mdl.name] = mdl

    @classmethod
    def build_model(cls, model_descriptor: ModelDescriptor) -> Model:
        # Low and high multiplicity is calculated from multiplicity string value
        # Examples: 1, *, 0..1, 0..*, 1..*
        raw_multiplicity = model_descriptor.multiplicity
        if not re.match(cls.MULTIPLICITY_PATTERN, raw_multiplicity):
            GeneratorLogger.add_marker(ErrorLevel.ERROR,
                f"Invalid multiplicity: {raw_multiplicity} in {model_descriptor} "
                f"(expected: {cls.MULTIPLICITY_PATTERN})"
            )
        if ".." in raw_multiplicity:
            raw_mul_lower, raw_mul_upper = raw_multiplicity.split("..")
            raw_mul_lower = raw_mul_lower.strip()
            raw_mul_upper = raw_mul_upper.strip()
            if raw_mul_lower == "*":
                multiplicity_lower = 0
            else:
                multiplicity_lower = int(raw_mul_lower, 0)
            if raw_mul_upper == "*":
                multiplicity_upper = ModelUtil.MULTIPLICITY_UNLIMITED
            else:
                multiplicity_upper = int(raw_mul_upper, 0)
        else:
            if raw_multiplicity == "*":
                multiplicity_lower = 0
                multiplicity_upper = ModelUtil.MULTIPLICITY_UNLIMITED
            else:
                multiplicity_lower = int(raw_multiplicity, 0)
                multiplicity_upper = multiplicity_lower
        # Model element constraint is validated and substituted by enum counterpart
        raw_element_constraint = model_descriptor.element_constraint
        if not re.match(cls.ELEM_CONSTRAINT_PATTERN, raw_element_constraint):
            GeneratorLogger.add_marker(ErrorLevel.ERROR,
                f"Invalid element_constraint: {raw_element_constraint} "
                f"in {model_descriptor} (expected: {cls.ELEM_CONSTRAINT_PATTERN})."
            )
        if raw_element_constraint == "primary":
            element_constraint = ModelElemConstraint.PRIMARY
        elif raw_element_constraint == "primary_any":
            element_constraint = ModelElemConstraint.PRIMARY_ANY
        elif raw_element_constraint == "any":
            element_constraint = ModelElemConstraint.ANY
        # Model security is validated and substituted by enum counterpart
        raw_security = model_descriptor.security
        if not re.match(cls.SECURITY_PATTERN, raw_security):
            GeneratorLogger.add_marker(ErrorLevel.ERROR,
                f"Invalid security: {raw_security} in {model_descriptor}. "
                f"(expected: {cls.SECURITY_PATTERN})"
            )
        if raw_security == "devkey":
            security = ModelSecurity.DEVKEY
        elif raw_security == "appkey":
            security = ModelSecurity.APPKEY
        elif raw_security == "mixed":
            security = ModelSecurity.MIXED
        # The extended and corresponding models can't be set because those models
        # might haven't been built yet
        return Model(
            cid=model_descriptor.cid,
            mid=model_descriptor.mid,
            name=model_descriptor.name,
            multiplicity_lower=multiplicity_lower,
            multiplicity_upper=multiplicity_upper,
            elem_constraint=element_constraint,
            publication_support=model_descriptor.publication_support,
            subscription_support=model_descriptor.subscription_support,
            security=security,
            metadata_headers=model_descriptor.metadata_headers,
            metadata_info=model_descriptor.metadata_info
        )

    @classmethod
    def build_model_relation(
        cls,
        mdl_desc: ModelDescriptor,
        mdl_desc_relation: ModelDescriptorRelation,
        mdl_relation_type: ModelRelationType,
    ) -> ModelRelation:
        raw_elem_constraint = mdl_desc_relation.element_constraint
        if not re.match(cls.RELATION_ELEM_CONSTRAINT_PATTERN, raw_elem_constraint):
            GeneratorLogger.add_marker(ErrorLevel.ERROR,
                f"Invalid model relation ({mdl_relation_type.name.lower}) "
                f"element constraint: {raw_elem_constraint} in {mdl_desc} "
                f"(expected: {cls.RELATION_ELEM_CONSTRAINT_PATTERN})."
            )
        if raw_elem_constraint == "same":
            elem_constraint = ModelRelationElemConstraint.SAME
        elif raw_elem_constraint == "other_smaller_elem_index":
            elem_constraint = ModelRelationElemConstraint.OTHER_SMALLER_ELEM_INDEX
        elif raw_elem_constraint == "other_larger_elem_index":
            elem_constraint = ModelRelationElemConstraint.OTHER_LARGER_ELEM_INDEX
        if mdl_desc_relation.model_name:
            # Get MID/CID from name
            mdl = cls.get_model_by_name(mdl_desc_relation.model_name)
            mdl_desc_relation.mid = mdl.mid
            mdl_desc_relation.cid = mdl.cid if mdl.cid else ModelUtil.CID_SIG_MDL
        else:
            # Get name from MID/CID info
            mdl = cls.get_model_by_ids(mdl_desc_relation.mid, mdl_desc_relation.cid)
            mdl_desc_relation.model_name = mdl.name

        if not cls.model_ids_exist(mdl_desc_relation.mid, mdl_desc_relation.cid):
            GeneratorLogger.add_marker(ErrorLevel.ERROR,
                f"Related model ({mdl_relation_type.name.lower}) with "
                f"{mdl_desc_relation.model_name} referenced by {mdl_desc} "
                "name doesn't exist."
            )
        mdl_relation = ModelRelation(mdl, elem_constraint, mdl_relation_type)
        return mdl_relation

    @classmethod
    def link_model(cls, model_descriptor: ModelDescriptor) -> None:
        if not cls.model_name_exists(model_descriptor.name):
            # This is really unexpected because model linking shall be executed
            # after models has been created from the very same model descriptors
            raise KeyError(f"Model ({model_descriptor.name.lower}) doesn't exist.")
        mdl = cls.get_model_by_name(model_descriptor.name)
        for corresponds_relation in model_descriptor.corresponds:
            mdl_relation = cls.build_model_relation(
                model_descriptor, corresponds_relation, ModelRelationType.CORRESPONDS
            )
            mdl.add_corresponds(mdl_relation)
        for extends_relation in model_descriptor.extends:
            mdl_relation = cls.build_model_relation(
                model_descriptor, extends_relation, ModelRelationType.EXTENDS
            )
            mdl.link_extended_model(mdl_relation)

    @classmethod
    def build(cls, model_descriptors: Iterable[ModelDescriptor]) -> None:
        # Phase 1: Build all models from model descriptors
        for mdl_desc in model_descriptors:
            mdl = cls.build_model(mdl_desc)
            cls.add_model(mdl)
        # Phase 2: Link all models (extends, corresponds) based on model descriptors
        for mdl_desc in model_descriptors:
            cls.link_model(mdl_desc)


class Source(object):
    def __init__(self, filename, group):
        self.filename = filename
        self.group = group

    def __eq__(self, other):
        if isinstance(other, Source):
            return (self.filename, self.group) == (other.filename, other.group)
        return NotImplemented

    def __neq__(self, other):
        return not (self == other)


class Element(object):
    def __init__(
        self,
        name: str,
        location: str,
        elem_index: Optional[int] = None,
        group: Optional[str] = None,
        sig_models: Iterable[ModelInst] = [],
        vendor_models: Iterable[ModelInst] = [],
        sources: Iterable[Source] = [],
    ):
        self.name = name
        self.location = int(location, 0)
        self.elem_index = elem_index
        # It is important to create empty lists because vendor model indexes are
        # affected when SIG model is appended to the element so the vendor models
        # are iterated in this case.
        self._sig_models = []
        self._vendor_models = []
        self.sig_models = [ModelInst(**m) for m in sig_models]
        self.vendor_models = self.check_vendor_models(vendor_models)
        self.sources = list(sources)

    def check_vendor_models(self, vendor_models: Iterable[ModelInst]) -> List[ModelInst]:
        vend_mod = [ModelInst(**m) for m in vendor_models]
        for m in vend_mod:
            i = vend_mod.index(m) + 1
            while i < len(vend_mod):
                n = vend_mod[i]
                if m.name == n.name:
                    print(f"\nBtMeshGenerator: In element: '{self.name}' ", end="")
                    print(f"duplicated vendor model name: '{m.name}' \n")
                    # exit('Duplicated vendor model name')
                    return vend_mod
                elif m.mid == n.mid:
                    print(f"\nBtMeshGenerator: In element: '{self.name}' ", end="")
                    print(f"duplicated vendor model ID (mid): {hex(m.mid)}\n")
                    # exit('Duplicated vendor model ID')
                    return vend_mod
                else:
                    i += 1
        return vend_mod

    @property
    def filenames(self) -> List[str]:
        return [source.filename for source in self.sources]

    @property
    def num_s(self) -> int:
        return len(self._sig_models)

    @property
    def num_v(self) -> int:
        return len(self._vendor_models)
    
    @property
    def num_s_metadata(self) -> int:
        return len([m for m in self._sig_models if len(m.mdl.metadata_info) > 0])
    
    @property
    def num_v_metadata(self) -> int:
        return len([m for m in self._vendor_models if len(m.mdl.metadata_info) > 0])

    @property
    def sig_models(self) -> Iterable[ModelInst]:
        yield from self._sig_models

    @sig_models.setter
    def sig_models(self, values) -> None:
        self._sig_models = []
        self.extend_sig_models(values)

    @property
    def vendor_models(self) -> Iterable[ModelInst]:
        yield from self._vendor_models

    @vendor_models.setter
    def vendor_models(self, values) -> None:
        self._vendor_models = []
        self.extend_vendor_models(values)

    @property
    def models(self) -> Iterable[ModelInst]:
        yield from self._sig_models
        yield from self._vendor_models

    @property
    def macros(self) -> List[str]:
        return [
            to_c_macro("_".join([filename, self.name])) for filename in self.filenames
        ]

    @property
    def group_macros(self) -> List[str]:
        # Remove duplicate entries from the list.
        # If a dcd file contributed to the same element more groups with the same
        # group name then entries could be duplicated. The group macro name and
        # value is the same so it can be tolerated.
        filtered_source_pairs = dict.fromkeys(
            (s.filename, s.group) for s in self.sources
        )
        # Generate group macro for each source which has explicit group name
        return [
            to_c_macro("_".join([src_pair[0], "group", src_pair[1], "elem_index"]))
            for src_pair in filtered_source_pairs.keys()
            if src_pair[1]
        ]

    def append_sig_model(self, mdl_inst: ModelInst) -> None:
        mdl_inst.elem = self
        mdl_inst.mdl_index = len(self._sig_models)
        self._sig_models.append(mdl_inst)
        self.update_vendor_model_indexes()

    def append_vendor_model(self, mdl_inst: ModelInst) -> None:
        mdl_inst.elem = self
        mdl_inst.mdl_index = len(self._sig_models) + len(self._vendor_models)
        self._vendor_models.append(mdl_inst)

    def update_vendor_model_indexes(self):
        base_vendor_mdl_idx = len(self._sig_models)
        for local_idx, vendor_mdl_inst in enumerate(self._vendor_models):
            vendor_mdl_inst.mdl_index = base_vendor_mdl_idx + local_idx

    def extend_sig_models(self, mdl_insts: Iterable[ModelInst]) -> None:
        for mdl_inst in mdl_insts:
            mdl_inst.elem = self
            mdl_inst.mdl_index = len(self._sig_models)
            self._sig_models.append(mdl_inst)
        self.update_vendor_model_indexes()

    def remove_sig_model(self, mid: int) -> None:
        for mdl_inst in self.sig_models:
            if mdl_inst.mid == mid:
                self._sig_models.remove(mdl_inst)
                break

    def extend_vendor_models(self, mdl_insts: Iterable[ModelInst]) -> None:
        for mdl_inst in mdl_insts:
            mdl_inst.elem = self
            mdl_inst.mdl_index = len(self._sig_models) + len(self._vendor_models)
            self._vendor_models.append(mdl_inst)
    
    def remove_vendor_model(self, mid: int) -> None:
        for mdl_inst in self.vendor_models:
            if mdl_inst.mid == mid:
                self._vendor_models.remove(mdl_inst)
                break

    def is_mergeable_with(self, other: "Element") -> bool:
        if self.name != other.name:
            return False
        self_sig_models = {m.mid for m in self.sig_models}
        other_sig_models = {m.mid for m in other.sig_models}
        if not self_sig_models.isdisjoint(other_sig_models):
            return False
        self_vendor_models = {m.vmid for m in self.vendor_models}
        other_vendor_models = {m.vmid for m in other.vendor_models}
        if not self_vendor_models.isdisjoint(other_vendor_models):
            return False
        return True

    def merge_with(self, other: "Element") -> None:
        self.extend_sig_models(other.sig_models)
        self.extend_vendor_models(other.vendor_models)
        self.sources.extend(other.sources)



class DCD(object):
    def __init__(self, cid, pid, vid, elements=[], model_descriptors=None):
        self.cid = int(cid, 0)
        self.pid = int(pid, 0)
        self.vid = int(vid, 0)
        self.elements = [Element(elem_index=i, **e) for i, e in enumerate(elements)]
        self.unique_vendor_models = []
        # Use command line argument could be used.
        self.page_1_present = True
        self.corresponding_groups: List[CorrespondingGroup] = []

    @property
    def total_models(self):
        return sum(elem.num_s + elem.num_v for elem in self.elements)

    @property
    def total_elements(self):
        return len(self.elements)

    def collect_v_models(self):
        vendor_models: List[Model] = []
        for elem in self.elements:
            for mdl_inst in elem.vendor_models:
                for mdl in vendor_models:
                    if mdl_inst.mdl.name == mdl.name or mdl_inst.mdl.mid == mdl.mid:
                        break
                else:
                    vendor_models.append(mdl_inst.mdl)
        return vendor_models

    def add_chunk(self, chunk: Iterable[Element]):
        for e in chunk:
            for element in self.elements:
                if element.is_mergeable_with(e):
                    element.merge_with(e)
                    break
            else:
                e.elem_index = len(self.elements)
                self.elements.append(e)

    def validate(self) -> bool:
        # The code below is not running when there is only one element because
        # no cross-checks are necessary.
        # It is not allowed to have the same group name from the same file
        # on different elements because the macro value is ambiguous.
        validation_error_list = []
        if not self.check_model_multiplicity():
            validation_error = (
                f"BtMeshGenerator: Model multiplicity invalid."
            )
            validation_error_list.append(validation_error)
        if not self.check_element_constraints():
            validation_error = (
                f"BtMeshGenerator: Element constraint invalid."
            )
            validation_error_list.append(validation_error)
        for elem_idx, elem in enumerate(self.elements):
            for elem_other_idx in range(elem_idx + 1, len(self.elements)):
                elem_other = self.elements[elem_other_idx]
                for source in elem.sources:
                    if source.group is None:
                        # If group is none then group macro is not generated so
                        # duplication shall not be checked.
                        continue
                    if source in elem_other.sources:
                        # If any source group is None in other element then the
                        # equality check is evaluated to false for sure because
                        # source.group is not None due to previous check.
                        validation_error = (
                            f"BtMeshGenerator: Duplicated group ({source.group}) "
                            f"from same file ({source.filename}.dcd) on different "
                            f"elements ({elem.name}, {elem_other.name})."
                        )
                        validation_error_list.append(validation_error)
        if validation_error_list:
            print("\n".join(validation_error_list))
            for validation_error in validation_error_list:
                GeneratorLogger.add_marker(ErrorLevel.ERROR,validation_error)
            return False
        return True

    def build_models_relations(self) -> None:
        self.build_and_link_corresponding_groups()
        self.link_extended_model_insts()

    def build_and_link_corresponding_groups(self) -> None:
        for e in self.elements:
            for mdl_inst in e.models:
                self.build_and_link_corresponding_group(mdl_inst)

    def build_and_link_corresponding_group(self, mdl_inst: ModelInst) -> None:
        # Check if the model shall have corresponding relationship with another
        # model according to the specification
        if not mdl_inst.mdl.has_corresponding_models():
            return
        # Check if the model instance is already part of an existing corresponding
        # group because it is possible that a previous model instance corresponds
        # relation added this model instance
        if mdl_inst.corresponding_group is not None:
            return
        cg = CorrespondingGroup([mdl_inst])
        self.corresponding_groups.append(cg)
        self.link_corresponding_model_inst(mdl_inst, cg, set())

    def link_corresponding_model_inst(
        self,
        mdl_inst: ModelInst,
        corresponding_group: CorrespondingGroup,
        processed_mdl_inst_ids: Set[ModelInstId],
    ) -> None:
        for corresponding_relation in mdl_inst.mdl.corresponds:
            corresponding_mdl_inst = self.find_related_model_inst(
                mdls=corresponding_relation.model,
                elem_constraint=corresponding_relation.elem_constraint,
                origin_elem_index=mdl_inst.elem_index,
            )
            if corresponding_mdl_inst is None:
                GeneratorLogger.add_marker(ErrorLevel.ERROR,
                    f"No corresponding {corresponding_relation.model} model "
                    f"instance was found for {mdl_inst} model instance."
                )
                break
            if corresponding_mdl_inst.corresponding_group is not None:
                if corresponding_mdl_inst.corresponding_group.id == corresponding_group.id:
                    # If the function is called recursively to discover model instances
                    # belonging to the same corresponding group then the bidirectional
                    # corresponds relationships is detected here.
                    # The processed_mdl_inst_ids could be used as well but this way
                    # it is possible to check interleaved multi-element corresponding
                    # relationship which is not allowed by the specification.
                    continue
                else:
                    # Warning could be reported.
                    pass
            if corresponding_mdl_inst.id not in processed_mdl_inst_ids:
                processed_mdl_inst_ids.add(corresponding_mdl_inst.id)
                # This should not happen if the corresponding model instances of
                # two main models are not interleaved which is not allowed by the
                # mesh specification
                # Warning could be reported.
                if corresponding_mdl_inst.corresponding_group is None:
                    corresponding_group.add_mdl_inst(corresponding_mdl_inst)
                    self.link_corresponding_model_inst(
                        corresponding_mdl_inst,
                        corresponding_group,
                        processed_mdl_inst_ids,
                    )

    def link_extended_model_insts(self) -> None:
        for e in self.elements:
            for mdl_inst in e.models:
                self.link_extended_model_inst(mdl_inst)

    def link_extended_model_inst(self, mdl_inst: ModelInst):
        for extends_relation in mdl_inst.mdl.extends:
            extended_mdl_inst = self.find_related_model_inst(
                mdls=extends_relation.model,
                elem_constraint=extends_relation.elem_constraint,
                origin_elem_index=mdl_inst.elem_index,
            )
            if extended_mdl_inst is None:
                GeneratorLogger.add_marker(ErrorLevel.ERROR,
                    f"No {extends_relation.model} extended model instance "
                    f"was found for {mdl_inst} model instance."
                )
            # Warning could be reported when when mdl_inst is extended already
            # by another mutually exclusive model.
            mdl_inst.link_extended_model_inst(extended_mdl_inst)

    def find_related_model_inst(
        self,
        mdls: Union[Model, Iterable[Model]],
        elem_constraint: ModelRelationElemConstraint,
        origin_elem_index: int,
        mdl_inst_filter: Optional[Callable[[ModelInst], bool]] = None,
    ) -> Optional[ModelInst]:
        assert isinstance(elem_constraint, ModelRelationElemConstraint)
        if isinstance(mdls, Model):
            mdls = [mdls]
        if mdl_inst_filter is None:
            mdl_inst_filter = lambda _: True
        mdl_vmid_set = set((mdl.vmid for mdl in mdls))
        if elem_constraint is ModelRelationElemConstraint.SAME:
            elem_index_start = origin_elem_index
            elem_index_end = origin_elem_index + 1
            elem_index_step = 1
        elif elem_constraint is ModelRelationElemConstraint.OTHER_SMALLER_ELEM_INDEX:
            if origin_elem_index == 0:
                # There is no element before the primary element with zero
                # element index so the start and end is set to the same value
                # to avoid unnecessary iteration.
                # The general formula in the else case would calculate -1 for
                # elem_index_start which means the last element in python
                # slicing so the formula can't be used in this special case
                # because it would iterate over each element.
                elem_index_start = origin_elem_index
                elem_index_end = origin_elem_index
                elem_index_step = 1
            else:
                # The end is exclusive in python slicing so None shall be used to
                # include the primary element as well
                elem_index_start = origin_elem_index - 1
                elem_index_end = None
                elem_index_step = -1
        elif elem_constraint is ModelRelationElemConstraint.OTHER_LARGER_ELEM_INDEX:
            # The end is exclusive in python slicing so length of elements shall
            # be used to include the last element as well
            elem_index_start = origin_elem_index + 1
            elem_index_end = len(self.elements)
            elem_index_step = 1
        for elem in self.elements[elem_index_start:elem_index_end:elem_index_step]:
            for mdl_inst in elem.models:
                if mdl_inst.mdl.vmid in mdl_vmid_set and mdl_inst_filter(mdl_inst):
                    return mdl_inst
        return None

    def remove_sig_model_from_elem(self, elem_idx: int, mid: int) -> None:
        for e in self.elements:
            if e.elem_index == elem_idx:
                e.remove_sig_model(mid)
                break

    def remove_vendor_model_from_elem(self, elem_idx: int, mid: int) -> None:
        for e in self.elements:
            if e.elem_index == elem_idx:
                e.remove_vendor_model(mid)
                break

    def move_sig_model(self, mid: int, elem_from: int, elem_to:int) -> None:
        model: ModelInst
        for e in self.elements:
            if e.elem_index == elem_from:
                for m in e.models:
                    if m.mid == mid:
                        model = m
                        e.remove_sig_model(mid)
                        break
        for e in self.elements:
            if e.elem_index == elem_to:
                e.append_sig_model(model)
                break

    def move_vendor_model(self, mid: int, elem_from: int, elem_to:int) -> None:
        model: ModelInst
        for e in self.elements:
            if e.elem_index == elem_from:
                for m in e.models:
                    if m.mid == mid:
                        model = m
                        e.remove_vendor_model(mid)
                        break
        for e in self.elements:
            if e.elem_index == elem_to:
                e.append_vendor_model(model)
                break
    def check_model_multiplicity(self) -> bool:
        # A model is considered mandatory if its multiplicity is '1' or '1..*'
        mandatory_mdl_list = []
        # A dictionary of all models present on the device and their instance count
        model_cnt_dict = {}
        
        # Check for mandatory models
        for model in ModelCollection.id_mdl_dict.values():
            if model.multiplicity_lower >= 1:
                mandatory_mdl_list.append(model.vmid)
        
        # Check for upper multiplicities
        for element in self.elements:
            for model in element.models:
                if model.vmid not in model_cnt_dict:
                    model_cnt_dict[model.vmid] = 1
                else:
                    model_cnt_dict[model.vmid] += 1

        # Validate that all mandatory models are present
        for vmid, cnt in model_cnt_dict.items():
            # Check mandatory, e.g. 1..
            if vmid in mandatory_mdl_list:
                mandatory_mdl_list.remove(vmid)
            # Check upper limits, i.e. ..1
            if ModelCollection.get_model_by_ids(vmid.mid,vmid.cid).multiplicity_upper < cnt:
                # More models found than allowed
                return False

        return len(mandatory_mdl_list) == 0

    def check_element_constraints(self) -> bool:
        # element_constraint can be the following:
        # "any":          doesn't matter where the element is
        # "primary":      the element shall be supported by a primary element
        # "primary_any":  the element shall be supported by a primary element and any secondary elements
        # Note that multiplicity 0.. can still have one of the above two constraints,
        # in this case these apply only if the element is present.
        primary_any_list = []
        for element in self.elements:
            for model in element.models:
                if model.mdl.elem_constraint == ModelElemConstraint.PRIMARY:
                    if element.elem_index != 0:
                        return False
                elif model.mdl.elem_constraint == ModelElemConstraint.PRIMARY_ANY:
                    if element.elem_index == 0:
                        primary_any_list.append(model.mid)
                    elif model.mid not in primary_any_list:
                        return False
        return True

class MIDEntry(object):
    def __init__(self, mid):
        self.mid = int(mid,0)

class ProfileData(object):
    def __init__(self, name, uuid, ver_maj, ver_min, ver_z, mid_list=[]):
        self.name = name
        self.uuid = int(uuid,0)
        self.ver_maj = int(ver_maj,0)
        self.ver_min = int(ver_min,0)
        self.ver_z = int(ver_z,0)
        self.mid_list = [MIDEntry(**m) for m in mid_list]
        self.offsets = set()
        self.additional_data=[]

    def calculate_offsets(self, elem_list: Iterable[Element]):
        for elem in elem_list:
            for mid in self.mid_list:
                for model in elem.models:
                    if mid.mid == model.mid:
                        self.offsets.add(elem.elem_index)


class Profiles(object):
    def __init__(self, profile_data=[]):
        self.profile_data = [ProfileData(**m) for m in profile_data]

    def add_profile(self, profile: Iterable[ProfileData]):
        for p in profile:
            self.profile_data.append(p)

def dcd_chunk(filename, elements=[]) -> List[Element]:
    # The group information is optional so get dictionary API is used in order
    # to receive None value when the group is not present.
    # This way no KeyError occurs like in case of normal dictionary lookup.
    return [Element(**e, sources=[Source(filename, e.get("group"))]) for e in elements]


def dcdgen(dcd, profiles, metadata, valid):
    dcd.unique_vendor_models = dcd.collect_v_models()
    env = Environment(lstrip_blocks=True, trim_blocks=True, keep_trailing_newline=True)
    env.loader = FileSystemLoader(str(BTMESH_GEN_DIR_PATH))
    return (
        env.get_template("templates/sl_btmesh_dcd.h.template").render(dcd=dcd, profiles=profiles, metadata=metadata, valid=valid ),
        env.get_template("templates/sl_btmesh_dcd.c.template").render(dcd=dcd, profiles=profiles, metadata=metadata, valid=valid ),
    )
    
def dcd_build(
    input_path: Path,
    model_descriptor_paths: Iterable[Path] = [BTMESH_SIG_MDL_DESC_PATH],
) -> tuple[DCD, List[Path]]:
    if input_path.is_dir():
        btmeshconf_file_path = next(
            (p for p in input_path.iterdir() if p.suffix.lower() == ".btmeshconf"), None
        )
        if not btmeshconf_file_path:
            print(f"BtMeshGenerator: No 'btmeshconf' file found in {input_path}.")
            exit(1)
        dcd_file_paths = [
            dcd_file_path
            for dcd_file_path in input_path.iterdir()
            if dcd_file_path.suffix.lower() == ".dcd"
        ]
        profile_paths = [
            profile_path
            for profile_path in input_path.iterdir()
            if profile_path.suffix.lower() == ".profile"
        ]
    else:
        btmeshconf_file_path = input_path
        dcd_file_paths = []
        profile_paths = []

    model_descriptors = []
    with btmeshconf_file_path.open() as f:
        btmeshconf = json.load(f)

    if "vendor_model_descriptors" in btmeshconf:
        vendor_model_descriptor_chunk=btmeshconf["vendor_model_descriptors"]
        model_descriptors.extend(
            ModelDescriptor(**raw_md) for raw_md in vendor_model_descriptor_chunk
        )

    for model_descriptor_path in model_descriptor_paths:
        with model_descriptor_path.open() as f:
            raw_model_descriptors_chunk = json.load(f)
            model_descriptors.extend(
                ModelDescriptor(**raw_md) for raw_md in raw_model_descriptors_chunk
            )
    ModelCollection.build(model_descriptors)

    dcd = DCD(**btmeshconf["composition_data"], model_descriptors=model_descriptors)

    for dcd_file_path in dcd_file_paths:
        with dcd_file_path.open() as f:
            dcd.add_chunk(dcd_chunk(dcd_file_path.stem, json.load(f)))

    return dcd, profile_paths

def dcd_validate(dcd: DCD) -> bool:
    return dcd.validate()

def dcd_build_models_relations(dcd: DCD):
    dcd.build_models_relations()

def generate_profiles(
        output_path: Path,
        profile_paths: List[Path],
        dcd: DCD
):
    metadata = False
    for e in dcd.elements:
        for s in e.sources:
            if s.filename == 'btmesh_lcd_server':
                metadata = True
                break
    profiles = Profiles()
    for profile_path in profile_paths:
        with profile_path.open() as f:
            raw_profile = json.load(f)
            profiles.add_profile(ProfileData(**p) for p in raw_profile)
    for profile in profiles.profile_data:
        profile.calculate_offsets(dcd.elements)

    dcd_valid = GeneratorLogger.result_code == 0
    dcd_h_file, dcd_c_file = dcdgen(dcd, profiles, metadata, dcd_valid)

    dcd_c_path = output_path / "sl_btmesh_dcd.c"
    dcd_h_path = output_path / "sl_btmesh_dcd.h"

    with dcd_h_path.open("w") as f:
        f.write(dcd_h_file)
    with dcd_c_path.open("w") as f:
        f.write(dcd_c_file)

    print(f"BtMeshGenerator: DCD written to {dcd_c_path.absolute()}")

def generate(
    input_path: Path,
    output_path: Path,
    model_descriptor_paths: Iterable[Path] = [BTMESH_SIG_MDL_DESC_PATH],
):
    try:
        dcd, profile_paths = dcd_build(input_path, model_descriptor_paths)
        valid = dcd_validate(dcd)
        if valid:
            dcd_build_models_relations(dcd)

        generate_profiles(output_path, profile_paths, dcd)
    except Exception as err:
        formatted_lines = traceback.format_exc().splitlines()
        GeneratorLogger.add_log(2, ErrorLevel.ERROR, f"An exception occured during generation", repr(err), formatted_lines)

    write_logs(output_path)
    # Print to stdout so the errors appear from command-line 
    if GeneratorLogger.result_code != 0:
        for log in GeneratorLogger.logs:
            print("BtMeshGenerator: " + log.message)
        for marker in GeneratorLogger.validation_markers:
            print("BtMeshGenerator: " + marker.message)

def write_logs(log_path: Path):
    GeneratorLogger.write_logs(log_path)

if __name__ == "__main__":
    import argparse

    cwd = Path.cwd()
    parser = argparse.ArgumentParser(
        description="BLE Mesh Device Composition Data code generator"
    )
    print("BtMeshGenerator:", cwd)
    parser.add_argument("input", nargs="?", default=cwd, type=Path)
    parser.add_argument("output", nargs="?", default=cwd, type=Path)
    args = parser.parse_args()

    generate(args.input, args.output)
