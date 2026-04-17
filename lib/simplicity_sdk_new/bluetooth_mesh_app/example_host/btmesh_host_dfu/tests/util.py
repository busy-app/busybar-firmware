"""Utility functions for tests."""

from typing import Optional, Iterable

from app.db import app_db, BtmeshDfuAppGroup
from btmesh.db import Node, DCD, DCDElement, ModelID
from btmesh.mdl import NamedModelID

# ---------------------------------------------------------------------------
# Model lists – light-related models per BT Mesh Model Spec
# ---------------------------------------------------------------------------

NODE_REQUIRED_PRIM_ELEM_MDLS = (
    ModelID(NamedModelID.CONFIGURATION_SERVER),
    ModelID(NamedModelID.HEALTH_SERVER),
)

# Light Lightness node – Main element (elem 0)
LIGHT_MAIN_ELEM_MDLS = (
    ModelID(NamedModelID.GENERIC_ONOFF_SERVER),
    ModelID(NamedModelID.GENERIC_LEVEL_SERVER),
    ModelID(NamedModelID.GENERIC_DEFAULT_TRANSITION_TIME_SERVER),
    ModelID(NamedModelID.GENERIC_POWER_ONOFF_SERVER),
    ModelID(NamedModelID.GENERIC_POWER_ONOFF_SETUP_SERVER),
    ModelID(NamedModelID.LIGHT_LIGHTNESS_SERVER),
    ModelID(NamedModelID.LIGHT_LIGHTNESS_SETUP_SERVER),
)

# Light CTL node – Main element (elem 0)
LIGHT_CTL_MAIN_ELEM_MDLS = LIGHT_MAIN_ELEM_MDLS + (
    ModelID(NamedModelID.LIGHT_CTL_SERVER),
    ModelID(NamedModelID.LIGHT_CTL_SETUP_SERVER),
)

# Light CTL node – Temperature element (elem 1)
LIGHT_CTL_TEMP_ELEM_MDLS = (
    ModelID(NamedModelID.GENERIC_LEVEL_SERVER),
    ModelID(NamedModelID.LIGHT_CTL_TEMPERATURE_SERVER),
)


# ---------------------------------------------------------------------------
# Database helpers
# ---------------------------------------------------------------------------

def create_node(
    uuid_hex: str,
    prim_addr: int,
    elem_count: int,
    name: str,
    appkey_indexes: Optional[Iterable[int]] = (0, 1, 2),
    dcd: Optional[DCD] = None,
) -> Node:
    """Create a minimal Node for testing."""
    # devkey is arbitrary for these tests; 16 zero-bytes are valid.
    devkey = bytes(16)
    node = Node(
        uuid=uuid_hex,
        devkey=devkey,
        prim_addr=prim_addr,
        elem_count=elem_count,
        name=name,
        appkey_indexes=appkey_indexes,
        dcd=dcd,
    )
    app_db.btmesh_db.add_node(node)
    return node


def create_onoff_light_node(
    uuid_hex: str,
    prim_addr: int,
    elem_count: int,
    name: str,
    appkey_indexes: Optional[Iterable[int]] = (0, 1, 2),
):
    """Create a minimal OnOff Light node with required models per BT Mesh Model Spec."""
    elem0 = DCDElement(
        idx=0,
        loc=0x0000,
        models=[
            *NODE_REQUIRED_PRIM_ELEM_MDLS,
            ModelID(NamedModelID.GENERIC_ONOFF_SERVER),
        ],
    )
    dcd = DCD(
        cid=0x02FF,
        pid=0xACDC,
        vid=0x0042,
        crpl=0,
        relay=True,
        proxy=True,
        friend=True,
        lpn=False,
        elements=[elem0],
    )
    return create_node(
        uuid_hex=uuid_hex,
        prim_addr=prim_addr,
        elem_count=elem_count,
        name=name,
        appkey_indexes=appkey_indexes,
        dcd=dcd,
    )


def create_light_node(
    uuid_hex: str,
    prim_addr: int,
    elem_count: int,
    name: str,
    appkey_indexes: Optional[Iterable[int]] = (0, 1, 2),
):
    """Create a Light Lightness node with all required models per BT Mesh Model Spec."""
    elem0 = DCDElement(
        idx=0,
        loc=0x0000,
        models=[
            *NODE_REQUIRED_PRIM_ELEM_MDLS,
            *LIGHT_MAIN_ELEM_MDLS,
        ],
    )
    dcd = DCD(
        cid=0x02FF,
        pid=0xACDC,
        vid=0x0042,
        crpl=0,
        relay=True,
        proxy=True,
        friend=True,
        lpn=False,
        elements=[elem0],
    )
    return create_node(
        uuid_hex=uuid_hex,
        prim_addr=prim_addr,
        elem_count=elem_count,
        name=name,
        appkey_indexes=appkey_indexes,
        dcd=dcd,
    )


def create_light_ctl_node(
    uuid_hex: str,
    prim_addr: int,
    elem_count: int,
    name: str,
    appkey_indexes: Optional[Iterable[int]] = (0, 1, 2),
):
    """Create a Light CTL node with all required models per BT Mesh Model Spec.

    Light CTL (Color Temperature and Lightness) Server extends Light Lightness Server
    and requires two elements:
    - Element 0: All Light Lightness models plus CTL models
    - Element 1: Temperature control models
    """
    elem0 = DCDElement(
        idx=0,
        loc=0x0000,
        models=[
            *NODE_REQUIRED_PRIM_ELEM_MDLS,
            *LIGHT_CTL_MAIN_ELEM_MDLS,
        ],
    )
    elem1 = DCDElement(
        idx=1,
        loc=0x0000,
        models=[
            *LIGHT_CTL_TEMP_ELEM_MDLS,
        ],
    )
    dcd = DCD(
        cid=0x02FF,
        pid=0xACDC,
        vid=0x0042,
        crpl=0,
        relay=True,
        proxy=True,
        friend=True,
        lpn=False,
        elements=[elem0, elem1],
    )
    return create_node(
        uuid_hex=uuid_hex,
        prim_addr=prim_addr,
        elem_count=elem_count,
        name=name,
        appkey_indexes=appkey_indexes,
        dcd=dcd,
    )

def create_app_group(
    name: str,
    group_addr: int,
    appkey_index: int = 0,
    pub_ttl: int = 5,
    pub_credentials: int = 0,
    pub_period_ms: int = 0,
    pub_retransmit_count: int = 0,
    pub_retransmit_interval_ms: int = 0,
) -> BtmeshDfuAppGroup:
    """Create a minimal BtmeshDfuAppGroup for testing."""
    app_group = BtmeshDfuAppGroup(
        name=name,
        group_addr=group_addr,
        appkey_index=appkey_index,
        pub_ttl=pub_ttl,
        pub_credentials=pub_credentials,
        pub_period_ms=pub_period_ms,
        pub_retransmit_count=pub_retransmit_count,
        pub_retransmit_interval_ms=pub_retransmit_interval_ms,
    )
    app_db.add_app_group(app_group)
    return app_group

def add_subs(
    app_group: BtmeshDfuAppGroup,
    node: Node,
    elem_index: int,
    mdls: Iterable[ModelID],
    auto_bind: bool = True,
    strict: bool = True,
):
    elem = node.get_elem_ref(elem_index=elem_index)
    for mdl in mdls:
        if strict and not node.has_model(elem_index=elem_index, model_id=mdl):
            raise ValueError(
                f"Node {node.name} element {elem_index} doesn't have model {mdl}."
            )
        if auto_bind:
            app_group.add_bind_elem_mdl(elem, mdl)
        app_group.add_sub_elem_mdl(elem, mdl)
