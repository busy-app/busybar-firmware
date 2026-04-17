import pytest

from . import util

from app.db import app_db  # noqa: E402
from app.grpctrl import app_grctrl  # noqa: E402


@pytest.fixture(autouse=True)
def clear_app_db():
    """Clear the application database before each test."""
    # The group control setup builds the internal cache and subscribes to db
    # events to handle database modifications like db clear, node removed or
    # new node term to update its cache accordingly.
    # The cache build is a NOP in each test case because the db is cleared before
    # each test, however the subscription to db events is necessary, especially
    # for database clear to avoid stale cache issues (cache is cleared on db clear).
    app_grctrl.setup()
    yield
    app_db.clear()

@pytest.fixture
def nw_gr0_light0_ctl0_grp1_light0_ctl1():
    """Create a test network with 3 nodes in two groups:
    - Node "light0" is created and its models are subscribed to both "grp0" and "grp1"
    - Node "ctl0" is created and its models are subscribed to group "grp0"
    - Node "ctl1" is created and its models are subscribed to group "grp1"
    """

    # -- Nodes ---------------------------------------------------------------
    light0 = util.create_light_node(
        uuid_hex="aa" * 16,
        prim_addr=0x0001,
        elem_count=1,
        name="light0",
    )
    ctl0 = util.create_light_ctl_node(
        uuid_hex="bb" * 16,
        prim_addr=0x0002,
        elem_count=2,
        name="ctl0",
    )
    ctl1 = util.create_light_ctl_node(
        uuid_hex="cc" * 16,
        prim_addr=0x0004,
        elem_count=2,
        name="ctl1",
    )

    # -- Groups --------------------------------------------------------------
    grp0 = util.create_app_group(name="grp0", group_addr=0xC000, appkey_index=0)
    grp1 = util.create_app_group(name="grp1", group_addr=0xC001, appkey_index=0)

    # -- grp0: light0, ctl0 -------------------------------------------------
    util.add_subs(grp0, light0, 0, util.LIGHT_MAIN_ELEM_MDLS)
    util.add_subs(grp0, ctl0, 0, util.LIGHT_CTL_MAIN_ELEM_MDLS)
    util.add_subs(grp0, ctl0, 1, util.LIGHT_CTL_TEMP_ELEM_MDLS)

    # -- grp1: light0, ctl1 -------------------------------------------------
    util.add_subs(grp1, light0, 0, util.LIGHT_MAIN_ELEM_MDLS)
    util.add_subs(grp1, ctl1, 0, util.LIGHT_CTL_MAIN_ELEM_MDLS)
    util.add_subs(grp1, ctl1, 1, util.LIGHT_CTL_TEMP_ELEM_MDLS)

    app_grctrl.build_info()

    return {
        light0.name: light0,
        ctl0.name: ctl0,
        ctl1.name: ctl1,
        grp0.name: grp0,
        grp1.name: grp1,
    }