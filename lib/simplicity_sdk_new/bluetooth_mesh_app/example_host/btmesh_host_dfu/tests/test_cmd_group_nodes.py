"""Tests for BtmeshCmd.add_group_nodes_args and process_group_nodes_args."""

from typing import Dict, Optional, Tuple

import pytest

from app.cmd.cmd import BtmeshCmd
from app.util.argparsex import ArgumentErrorExt, ArgumentParserExt
from btmesh.db import Node
from . import util


class DummyBtmeshCmd(BtmeshCmd):
    """Minimal concrete BtmeshCmd used to exercise add/process_group_nodes_args."""

    @property
    def parser(self) -> ArgumentParserExt:
        return self.dummy_parser

    @property
    def current_parser(self) -> Optional[ArgumentParserExt]:
        return getattr(self, "_current_parser", self.parser)

    def create_parser(self, subparsers) -> ArgumentParserExt:
        self.dummy_parser = subparsers.add_parser(
            "dummy",
            prog="dummy",
            help="Dummy command for testing.",
            exit_on_error_ext=False,
        )
        self._subparsers = self.dummy_parser.add_subparsers(
            dest="dummy_subcmd_name",
            title="Subcommands",
            required=True,
        )
        self.subparser_dict = dict(
            (
                self.create_dummy_group_nodes_full_subparser(self._subparsers),
                self.create_dummy_group_nodes_full_strict_subparser(
                    self._subparsers
                ),
                self.create_dummy_group_nodes_only_subparser(self._subparsers),
                self.create_dummy_group_nodes_grpaddr_strict_subparser(
                    self._subparsers
                ),
            )
        )
        return self.dummy_parser

    def create_dummy_group_nodes_full_subparser(
        self, subparsers
    ) -> Tuple[str, ArgumentParserExt]:
        SUBPARSER_NAME = "group_nodes_full"
        self.dummy_group_nodes_full_parser: ArgumentParserExt = subparsers.add_parser(
            SUBPARSER_NAME,
            help="dummy group_nodes_full subcommand for testing.",
            exit_on_error_ext=False,
        )
        self.dummy_group_nodes_full_parser.set_defaults(
            dummy_subcmd=self.dummy_group_nodes_full_cmd
        )
        self.add_group_nodes_args(
            self.dummy_group_nodes_full_parser,
            add_elem_arg=True,
            add_elem_addrs_arg=True,
            add_group_addr_arg=True,
            elem_default=0,
        )
        return SUBPARSER_NAME, self.dummy_group_nodes_full_parser

    def __call__(self, arg) -> bool:
        self.pargs = self.dummy_parser.parse_args(arg.split())
        self._current_parser = self.subparser_dict.get(
            self.pargs.dummy_subcmd_name, self.parser
        )
        self._current_parser = self.parser
        return False

    def dummy_group_nodes_full_cmd(self, pargs):
        """Dummy command function for the "group_nodes_full" subparser."""
        pass

    # -- group_nodes_full_strict: elem + addrs + group_addr, elem_default=None --

    def create_dummy_group_nodes_full_strict_subparser(
        self, subparsers
    ) -> Tuple[str, ArgumentParserExt]:
        SUBPARSER_NAME = "group_nodes_full_strict"
        self.dummy_group_nodes_full_strict_parser: ArgumentParserExt = (
            subparsers.add_parser(
                SUBPARSER_NAME,
                help="dummy group_nodes_full_strict subcommand for testing.",
                exit_on_error_ext=False,
            )
        )
        self.dummy_group_nodes_full_strict_parser.set_defaults(
            dummy_subcmd=self.dummy_group_nodes_full_strict_cmd
        )
        self.add_group_nodes_args(
            self.dummy_group_nodes_full_strict_parser,
            add_elem_arg=True,
            add_elem_addrs_arg=True,
            add_group_addr_arg=True,
            elem_default=None,
        )
        return SUBPARSER_NAME, self.dummy_group_nodes_full_strict_parser

    def dummy_group_nodes_full_strict_cmd(self, pargs):
        """Dummy command function for the "group_nodes_full_strict" subparser."""
        pass

    # -- group_nodes_only: no elem, no addrs, no group_addr, elem_default=0 --

    def create_dummy_group_nodes_only_subparser(
        self, subparsers
    ) -> Tuple[str, ArgumentParserExt]:
        SUBPARSER_NAME = "group_nodes_only"
        self.dummy_group_nodes_only_parser: ArgumentParserExt = (
            subparsers.add_parser(
                SUBPARSER_NAME,
                help="dummy group_nodes_only subcommand for testing.",
                exit_on_error_ext=False,
            )
        )
        self.dummy_group_nodes_only_parser.set_defaults(
            dummy_subcmd=self.dummy_group_nodes_only_cmd
        )
        self.add_group_nodes_args(
            self.dummy_group_nodes_only_parser,
            add_elem_arg=False,
            add_elem_addrs_arg=False,
            add_group_addr_arg=False,
            elem_default=0,
        )
        return SUBPARSER_NAME, self.dummy_group_nodes_only_parser

    def dummy_group_nodes_only_cmd(self, pargs):
        """Dummy command function for the "group_nodes_only" subparser."""
        pass

    # -- group_nodes_grpaddr_strict: no elem, no addrs, group_addr, elem_default=None --

    def create_dummy_group_nodes_grpaddr_strict_subparser(
        self, subparsers
    ) -> Tuple[str, ArgumentParserExt]:
        SUBPARSER_NAME = "group_nodes_grpaddr_strict"
        self.dummy_group_nodes_grpaddr_strict_parser: ArgumentParserExt = (
            subparsers.add_parser(
                SUBPARSER_NAME,
                help="dummy group_nodes_grpaddr_strict subcommand for testing.",
                exit_on_error_ext=False,
            )
        )
        self.dummy_group_nodes_grpaddr_strict_parser.set_defaults(
            dummy_subcmd=self.dummy_group_nodes_grpaddr_strict_cmd
        )
        self.add_group_nodes_args(
            self.dummy_group_nodes_grpaddr_strict_parser,
            add_elem_arg=False,
            add_elem_addrs_arg=False,
            add_group_addr_arg=True,
            elem_default=None,
        )
        return SUBPARSER_NAME, self.dummy_group_nodes_grpaddr_strict_parser

    def dummy_group_nodes_grpaddr_strict_cmd(self, pargs):
        """Dummy command function for the "group_nodes_grpaddr_strict" subparser."""
        pass


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------


@pytest.fixture()
def dummy_cmd() -> DummyBtmeshCmd:
    """Build a DummyBtmeshCmd with a fully initialized parser tree."""
    cmd = DummyBtmeshCmd()
    # Replicate what the application does: create a top-level parser, then
    # pass its subparsers to the command.
    root_parser = ArgumentParserExt(exit_on_error_ext=False)
    root_subparsers = root_parser.add_subparsers(dest="command")
    cmd.create_parser(root_subparsers)
    return cmd


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------


class TestProcessGroupNodesFullArgs:
    """Tests for add_group_nodes_args / process_group_nodes_args.
    The process_group_nodes_args should be called from the command handler after
    parsing the command line arguments with the subcommand parser in real use.
    This guarantees that the current parser is set properly which affects the
    exception text by including the subcommand name in the error messages.
    It would make tests more complex and less readable if process_group_nodes_args
    wasn't called directly from the test case because it would be harder to pass
    arguments and assert on the result.
    Therefore, the test cases call the dummy command with the test arguments
    which parse the arguments and produces pargs for process_group_nodes_args,
    and then the test cases call process_group_nodes_args with the pargs to get
    the result and assert on it. The dummy command doesn't reset the current
    parser to the top-level parser after parsing, so the parse error messages
    include the subcommand name as expected after the command execution as well.
    """

    def test_single_node(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Selecting a single node by name with -n and -e returns the expected
        node list and computed element addresses."""
        # -- Arrange --
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]

        # Parse the command line "group_nodes_full -n light0 -e 0"
        dummy_cmd("group_nodes_full -n light0 -e 0")

        # -- Act --
        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        # -- Assert --
        # group_addr should be the unassigned address (0) because --group was
        # not used.
        assert group_addr == 0

        # Exactly the single node should be returned.
        assert len(nodes) == 1
        assert nodes[0] is light0

        # elem_addrs should contain the unicast address computed from
        # prim_addr + elem_index.
        expected_elem_addr = light0.prim_addr + 0
        assert elem_addrs == [expected_elem_addr]

    def test_single_node_elem1(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Selecting a CTL node with elem index 1 returns the temperature
        element address (prim_addr + 1)."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        ctl0 = nw["ctl0"]

        dummy_cmd("group_nodes_full -n ctl0 -e 1")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 1
        assert nodes[0] is ctl0
        assert elem_addrs == [ctl0.prim_addr + 1]

    def test_multiple_nodes(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Selecting multiple nodes by name with -n returns all of them
        with correctly computed element addresses."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]
        ctl0 = nw["ctl0"]

        dummy_cmd("group_nodes_full -n light0 ctl0 -e 0")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 2
        assert light0 in nodes
        assert ctl0 in nodes
        assert light0.prim_addr + 0 in elem_addrs
        assert ctl0.prim_addr + 0 in elem_addrs

    def test_all_nodes_glob(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using a glob pattern '*' with -n returns all nodes in the db."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1

        dummy_cmd("group_nodes_full -n * -e 0")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 3
        node_names = {n.name for n in nodes}
        assert node_names == {"light0", "ctl0", "ctl1"}
        assert len(elem_addrs) == 3

    def test_glob_pattern(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using a glob pattern 'ctl*' with -n returns only matching nodes."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        ctl0 = nw["ctl0"]
        ctl1 = nw["ctl1"]

        dummy_cmd("group_nodes_full -n ctl* -e 0")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 2
        assert ctl0 in nodes
        assert ctl1 in nodes
        assert ctl0.prim_addr + 0 in elem_addrs
        assert ctl1.prim_addr + 0 in elem_addrs

    def test_default_elem(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """When -e is omitted the elem_default=0 from add_group_nodes_args
        is used, so elem_addrs equals prim_addr + 0 for each node."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        ctl1 = nw["ctl1"]

        # No -e argument; parser defaults to elem_default=0
        dummy_cmd("group_nodes_full -n ctl1")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 1
        assert nodes[0] is ctl1
        assert elem_addrs == [ctl1.prim_addr + 0]

    def test_nodes_order_name(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Nodes returned with nodes_order_property='name' are ordered
        alphabetically by name."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1

        dummy_cmd("group_nodes_full -n * -e 0")

        _, nodes, _ = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        node_names = [n.name for n in nodes]
        assert node_names == sorted(node_names)

    def test_nodes_reverse_order(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Setting nodes_reverse=True reverses the name-based ordering."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1

        dummy_cmd("group_nodes_full -n * -e 0")

        _, nodes, _ = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
            nodes_reverse=True,
        )

        node_names = [n.name for n in nodes]
        assert node_names == sorted(node_names, reverse=True)

    def test_nodes_filter(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Passing a nodes_filter narrows result to nodes matching predicate."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]

        dummy_cmd("group_nodes_full -n * -e 0")

        # Filter to single-element nodes only
        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_filter=lambda n: n.elem_count == 1,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 1
        assert nodes[0] is light0
        assert elem_addrs == [light0.prim_addr + 0]

    # -- --group path -------------------------------------------------------

    def test_group_grp0(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -G grp0 returns grp0's group_addr and the nodes belonging
        to grp0 (light0, ctl0), with elem_addrs computed from elem default."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]
        ctl0 = nw["ctl0"]
        grp0 = nw["grp0"]

        dummy_cmd("group_nodes_full -G grp0")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == grp0.group_addr
        assert len(nodes) == 2
        node_names = {n.name for n in nodes}
        assert node_names == {"light0", "ctl0"}
        # Default elem is 0 so elem_addrs come from prim_addr + 0
        assert light0.prim_addr in elem_addrs
        assert ctl0.prim_addr in elem_addrs

    def test_group_grp1(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -G grp1 returns grp1's group_addr and nodes (light0, ctl1)."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]
        ctl1 = nw["ctl1"]
        grp1 = nw["grp1"]

        dummy_cmd("group_nodes_full -G grp1")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == grp1.group_addr
        assert len(nodes) == 2
        node_names = {n.name for n in nodes}
        assert node_names == {"light0", "ctl1"}
        assert light0.prim_addr in elem_addrs
        assert ctl1.prim_addr in elem_addrs

    def test_group_with_explicit_elem(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -G grp0 -e 0 selects group nodes and computes element
        addresses from elem index 0."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        grp0 = nw["grp0"]

        dummy_cmd("group_nodes_full -G grp0 -e 0")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == grp0.group_addr
        assert len(nodes) == 2
        expected_addrs = [n.prim_addr + 0 for n in nodes]
        assert elem_addrs == expected_addrs

    def test_group_order_name(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Nodes returned from --group are ordered by nodes_order_property."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1

        dummy_cmd("group_nodes_full -G grp0")

        _, nodes, _ = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        node_names = [n.name for n in nodes]
        assert node_names == sorted(node_names)

    def test_group_reverse_order(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Nodes returned from --group with nodes_reverse=True are reversed."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1

        dummy_cmd("group_nodes_full -G grp0")

        _, nodes, _ = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
            nodes_reverse=True,
        )

        node_names = [n.name for n in nodes]
        assert node_names == sorted(node_names, reverse=True)

    def test_group_with_group_addr_zero(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -G grp0 --group-addr 0 overrides the group address to 0
        (unicast addressing) while still selecting nodes from grp0."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]
        ctl0 = nw["ctl0"]
        grp0 = nw["grp0"]

        dummy_cmd("group_nodes_full -G grp0 --group-addr 0")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        # group_addr should be 0 (overridden), not grp0.group_addr
        assert group_addr == 0
        assert group_addr != grp0.group_addr

        # Nodes should still be from grp0 (light0 and ctl0)
        assert len(nodes) == 2
        node_names = {n.name for n in nodes}
        assert node_names == {"light0", "ctl0"}

        # elem_addrs computed with default elem (0)
        assert light0.prim_addr in elem_addrs
        assert ctl0.prim_addr in elem_addrs

    # -- --addrs path -------------------------------------------------------

    def test_addrs_single(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using --addrs with a single element address returns the correct
        node and element address list."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]

        dummy_cmd(f"group_nodes_full --addrs {light0.prim_addr}")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 1
        assert nodes[0] is light0
        assert elem_addrs == [light0.prim_addr]

    def test_addrs_multiple(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using --addrs with multiple element addresses returns matching
        nodes and element addresses."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]
        ctl0 = nw["ctl0"]
        ctl0_elem1_addr = ctl0.prim_addr + 1

        dummy_cmd(f"group_nodes_full --addrs {light0.prim_addr} {ctl0_elem1_addr}")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert light0 in nodes
        assert ctl0 in nodes
        assert light0.prim_addr in elem_addrs
        assert ctl0_elem1_addr in elem_addrs

    def test_addrs_filter(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using elem_addrs_filter narrows returned element addresses."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]
        ctl0 = nw["ctl0"]

        dummy_cmd(f"group_nodes_full --addrs {light0.prim_addr} {ctl0.prim_addr}")

        # Filter to keep only light0's primary address
        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
            elem_addrs_filter=lambda addr: addr == light0.prim_addr,
        )

        assert group_addr == 0
        assert elem_addrs == [light0.prim_addr]

    def test_addrs_hex(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using --addrs with hex element addresses works correctly."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        ctl1 = nw["ctl1"]

        dummy_cmd(f"group_nodes_full --addrs {hex(ctl1.prim_addr)}")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 1
        assert nodes[0] is ctl1
        assert elem_addrs == [ctl1.prim_addr]

    def test_addrs_node_elem_format(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using --addrs with node_name[elem_index] format works correctly."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]
        ctl0 = nw["ctl0"]

        dummy_cmd("group_nodes_full --addrs light0[0] ctl0[1]")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 2
        assert light0 in nodes
        assert ctl0 in nodes
        assert light0.prim_addr + 0 in elem_addrs
        assert ctl0.prim_addr + 1 in elem_addrs

    def test_addrs_glob_elem_format(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using --addrs with glob pattern and elem_index (ctl*[1]) selects
        element 1 of all matching CTL nodes."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        ctl0 = nw["ctl0"]
        ctl1 = nw["ctl1"]

        dummy_cmd("group_nodes_full --addrs ctl*[1]")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 2
        assert ctl0 in nodes
        assert ctl1 in nodes
        assert ctl0.prim_addr + 1 in elem_addrs
        assert ctl1.prim_addr + 1 in elem_addrs

    # -- --group-addr with --nodes ------------------------------------------

    def test_nodes_with_group_addr(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -n with --group-addr returns the specified group address
        instead of the unassigned address."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]

        dummy_cmd(f"group_nodes_full -n light0 -e 0 --group-addr 0xC100")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0xC100
        assert len(nodes) == 1
        assert nodes[0] is light0
        assert elem_addrs == [light0.prim_addr + 0]

    # -- elem_addrs correctness per node ------------------------------------

    def test_elem_addrs_match_nodes(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """When multiple nodes are selected each elem_addr in the returned
        list corresponds to prim_addr + elem_index for the respective node,
        in the same order as the nodes list."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1

        dummy_cmd("group_nodes_full -n * -e 0")

        _, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        expected = [n.prim_addr + 0 for n in nodes]
        assert elem_addrs == expected

    def test_node_by_address(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Selecting a node by its primary address with -n works correctly."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        ctl0 = nw["ctl0"]

        dummy_cmd(f"group_nodes_full -n {hex(ctl0.prim_addr)} -e 0")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 1
        assert nodes[0] is ctl0
        assert elem_addrs == [ctl0.prim_addr + 0]

    # -- Non-existent node name with --nodes --------------------------------

    def test_nodes_nonexistent_name(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -n with a node name that doesn't exist raises
        ArgumentErrorExt (converted from BtmeshDfuAppParseSpecError)."""
        dummy_cmd("group_nodes_full -n nonexistent -e 0")

        with pytest.raises(ArgumentErrorExt):
            dummy_cmd.process_group_nodes_args(
                dummy_cmd.pargs,
                nodes_order_property="name",
            )

    # -- Glob matching no nodes with --nodes --------------------------------

    def test_nodes_glob_no_match(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -n with a glob pattern that matches no nodes raises
        ArgumentErrorExt (converted from BtmeshDfuAppParseSpecError)."""
        dummy_cmd("group_nodes_full -n xyz* -e 0")

        with pytest.raises(ArgumentErrorExt):
            dummy_cmd.process_group_nodes_args(
                dummy_cmd.pargs,
                nodes_order_property="name",
            )

    # -- Non-existent group name with --group -------------------------------

    def test_group_nonexistent_name(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -G with a group name that doesn't exist raises
        ArgumentErrorExt (converted from BtmeshDfuAppParseSpecError)."""
        dummy_cmd("group_nodes_full -G nonexistent")

        with pytest.raises(ArgumentErrorExt):
            dummy_cmd.process_group_nodes_args(
                dummy_cmd.pargs,
                nodes_order_property="name",
            )

    # -- Non-existent address with --addrs ----------------------------------

    def test_addrs_nonexistent_address(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using --addrs with a unicast address that doesn't belong to any
        node raises ArgumentErrorExt (converted from BtmeshDfuAppParseSpecError)."""
        dummy_cmd("group_nodes_full --addrs 0x7FFF")

        with pytest.raises(ArgumentErrorExt):
            dummy_cmd.process_group_nodes_args(
                dummy_cmd.pargs,
                nodes_order_property="name",
            )

    # -- Non-existent node in node_name[elem] with --addrs ------------------

    def test_addrs_node_elem_nonexistent_node(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using --addrs with node_name[elem] format where the node name
        doesn't exist raises ArgumentErrorExt (converted from BtmeshDfuAppParseSpecError)."""
        dummy_cmd("group_nodes_full --addrs nonexistent[0]")

        with pytest.raises(ArgumentErrorExt):
            dummy_cmd.process_group_nodes_args(
                dummy_cmd.pargs,
                nodes_order_property="name",
            )

    # -- Mutually exclusive --nodes and --group at parse time ---------------

    def test_nodes_and_group_mutually_exclusive(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Passing both -n and -G raises ArgumentErrorExt at parse time
        because they are mutually exclusive."""
        with pytest.raises(ArgumentErrorExt):
            dummy_cmd("group_nodes_full -n light0 -G grp0")

    def test_nodes_and_addrs_mutually_exclusive(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Passing both -n and --addrs raises ArgumentErrorExt at parse time
        because they are mutually exclusive."""
        with pytest.raises(ArgumentErrorExt):
            dummy_cmd("group_nodes_full -n light0 --addrs 1")

    def test_group_and_addrs_mutually_exclusive(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Passing both -G and --addrs raises ArgumentErrorExt at parse time
        because they are mutually exclusive."""
        with pytest.raises(ArgumentErrorExt):
            dummy_cmd("group_nodes_full -G grp0 --addrs 1")

    # -- Missing required target argument -----------------------------------

    def test_no_target_argument(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Omitting all of -n, -G, and --addrs raises ArgumentErrorExt
        because one of them is required."""
        with pytest.raises(ArgumentErrorExt):
            dummy_cmd("group_nodes_full -e 0")

    # -- --group with non-zero --group-addr ---------------------------------

    def test_group_with_nonzero_group_addr(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -G with a group that has a non-zero group address and
        also providing --group-addr with a non-zero value raises
        ArgumentErrorExt."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        grp0 = nw["grp0"]

        # grp0 has group_addr=0xC000 (non-zero)
        assert grp0.group_addr != 0

        dummy_cmd("group_nodes_full -G grp0 --group-addr 0xC200")

        with pytest.raises(ArgumentErrorExt):
            dummy_cmd.process_group_nodes_args(
                dummy_cmd.pargs,
                nodes_order_property="name",
            )


class TestProcessGroupNodesFullStrictArgs:
    """Tests for add_group_nodes_args / process_group_nodes_args
    with elem_default=None (group_nodes_full_strict subparser).

    The key difference from group_nodes_full is that --elem has no default
    value, making it mandatory when --nodes or --group is used.
    Filtering and ordering tests are omitted because they are already
    covered by TestProcessGroupNodesFullArgs.
    """

    # -- Positive tests: explicit --elem with --nodes/--group works ---------

    def test_nodes_with_explicit_elem(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Selecting a node by name with -n and explicit -e returns expected
        results when elem_default=None."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]

        dummy_cmd("group_nodes_full_strict -n light0 -e 0")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 1
        assert nodes[0] is light0
        assert elem_addrs == [light0.prim_addr + 0]

    def test_nodes_with_explicit_elem1(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Selecting a CTL node with explicit elem index 1 returns the
        temperature element address (prim_addr + 1) when elem_default=None."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        ctl0 = nw["ctl0"]

        dummy_cmd("group_nodes_full_strict -n ctl0 -e 1")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 1
        assert nodes[0] is ctl0
        assert elem_addrs == [ctl0.prim_addr + 1]

    def test_group_with_explicit_elem(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -G with explicit -e returns group nodes and element
        addresses when elem_default=None."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        grp0 = nw["grp0"]
        light0 = nw["light0"]
        ctl0 = nw["ctl0"]

        dummy_cmd("group_nodes_full_strict -G grp0 -e 0")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == grp0.group_addr
        assert len(nodes) == 2
        node_names = {n.name for n in nodes}
        assert node_names == {"light0", "ctl0"}
        assert light0.prim_addr in elem_addrs
        assert ctl0.prim_addr in elem_addrs

    def test_addrs_bypasses_elem_requirement(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using --addrs works without -e even when elem_default=None,
        because --addrs provides element addresses directly."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]

        dummy_cmd(f"group_nodes_full_strict --addrs {light0.prim_addr}")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 1
        assert nodes[0] is light0
        assert elem_addrs == [light0.prim_addr]

    def test_addrs_node_elem_format_bypasses_elem_requirement(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using --addrs with node[elem] format works without -e even when
        elem_default=None."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        ctl0 = nw["ctl0"]

        dummy_cmd("group_nodes_full_strict --addrs ctl0[1]")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 1
        assert nodes[0] is ctl0
        assert elem_addrs == [ctl0.prim_addr + 1]

    def test_nodes_with_group_addr(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -n with --group-addr and explicit -e returns the specified
        group address and computed element addresses."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]

        dummy_cmd("group_nodes_full_strict -n light0 -e 0 --group-addr 0xC100")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0xC100
        assert len(nodes) == 1
        assert nodes[0] is light0
        assert elem_addrs == [light0.prim_addr + 0]

    # -- Negative tests: missing --elem when required -----------------------

    def test_nodes_without_elem_fails(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -n without -e raises ArgumentErrorExt when elem_default=None
        because --elem is mandatory with --nodes."""
        dummy_cmd("group_nodes_full_strict -n light0")

        with pytest.raises(ArgumentErrorExt):
            dummy_cmd.process_group_nodes_args(
                dummy_cmd.pargs,
                nodes_order_property="name",
            )

    def test_group_without_elem_fails(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -G without -e raises ArgumentErrorExt when elem_default=None
        because --elem is mandatory with --group."""
        dummy_cmd("group_nodes_full_strict -G grp0")

        with pytest.raises(ArgumentErrorExt):
            dummy_cmd.process_group_nodes_args(
                dummy_cmd.pargs,
                nodes_order_property="name",
            )

    def test_no_target_argument(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Omitting all of -n, -G, and --addrs raises ArgumentErrorExt
        because one of them is required."""
        with pytest.raises(ArgumentErrorExt):
            dummy_cmd("group_nodes_full_strict -e 0")

    def test_nodes_and_group_mutually_exclusive(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Passing both -n and -G raises ArgumentErrorExt at parse time
        because they are mutually exclusive."""
        with pytest.raises(ArgumentErrorExt):
            dummy_cmd("group_nodes_full_strict -n light0 -G grp0 -e 0")

    def test_nodes_and_addrs_mutually_exclusive(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Passing both -n and --addrs raises ArgumentErrorExt at parse time
        because they are mutually exclusive."""
        with pytest.raises(ArgumentErrorExt):
            dummy_cmd("group_nodes_full_strict -n light0 --addrs 1")

    def test_group_with_nonzero_group_addr(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -G with a non-zero group address and --group-addr with a
        non-zero value raises ArgumentErrorExt."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        grp0 = nw["grp0"]
        assert grp0.group_addr != 0

        dummy_cmd("group_nodes_full_strict -G grp0 -e 0 --group-addr 0xC200")

        with pytest.raises(ArgumentErrorExt):
            dummy_cmd.process_group_nodes_args(
                dummy_cmd.pargs,
                nodes_order_property="name",
            )


class TestProcessGroupNodesOnlyArgs:
    """Tests for add_group_nodes_args / process_group_nodes_args
    with the group_nodes_only subparser configuration.

    Key differences from group_nodes_full:
    - add_elem_arg=False: No --elem/-e argument
    - add_elem_addrs_arg=False: No --addrs argument
    - add_group_addr_arg=False: No --group-addr/-g argument
    - elem_addrs is always None in the result
    Filtering and ordering tests are omitted because they are already
    covered by TestProcessGroupNodesFullArgs.
    """

    # -- Positive tests: basic --nodes and --group work ---------------------

    def test_single_node(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Selecting a single node by name with -n returns the node and
        elem_addrs=None (no --elem support)."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]

        dummy_cmd("group_nodes_only -n light0")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 1
        assert nodes[0] is light0
        assert elem_addrs is None

    def test_multiple_nodes(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Selecting multiple nodes with -n returns nodes and
        elem_addrs=None."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]
        ctl0 = nw["ctl0"]

        dummy_cmd("group_nodes_only -n light0 ctl0")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 2
        assert light0 in nodes
        assert ctl0 in nodes
        assert elem_addrs is None

    def test_all_nodes_glob(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using glob '*' with -n returns all nodes and elem_addrs=None."""
        dummy_cmd("group_nodes_only -n *")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 3
        node_names = {n.name for n in nodes}
        assert node_names == {"light0", "ctl0", "ctl1"}
        assert elem_addrs is None

    def test_group_grp0(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -G grp0 returns group nodes with group_addr and
        elem_addrs=None."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        grp0 = nw["grp0"]

        dummy_cmd("group_nodes_only -G grp0")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == grp0.group_addr
        assert len(nodes) == 2
        node_names = {n.name for n in nodes}
        assert node_names == {"light0", "ctl0"}
        assert elem_addrs is None

    def test_group_grp1(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -G grp1 returns group nodes with group_addr and
        elem_addrs=None."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        grp1 = nw["grp1"]

        dummy_cmd("group_nodes_only -G grp1")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == grp1.group_addr
        assert len(nodes) == 2
        node_names = {n.name for n in nodes}
        assert node_names == {"light0", "ctl1"}
        assert elem_addrs is None

    # -- Negative tests: unsupported arguments raise parse errors -----------

    def test_elem_arg_not_recognized(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -e when add_elem_arg=False raises ArgumentErrorExt at
        parse time because the argument is not recognized."""
        with pytest.raises(ArgumentErrorExt):
            dummy_cmd("group_nodes_only -n light0 -e 0")

    def test_addrs_arg_not_recognized(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using --addrs when add_elem_addrs_arg=False raises
        ArgumentErrorExt at parse time because the argument is not
        recognized."""
        with pytest.raises(ArgumentErrorExt):
            dummy_cmd("group_nodes_only --addrs light0[0]")

    def test_group_addr_arg_not_recognized(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using --group-addr when add_group_addr_arg=False raises
        ArgumentErrorExt at parse time because the argument is not
        recognized."""
        with pytest.raises(ArgumentErrorExt):
            dummy_cmd("group_nodes_only -n light0 --group-addr 0xC100")

    def test_no_target_argument(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Omitting both -n and -G raises ArgumentErrorExt because one
        of them is required."""
        with pytest.raises(ArgumentErrorExt):
            dummy_cmd("group_nodes_only")

    def test_nodes_and_group_mutually_exclusive(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Passing both -n and -G raises ArgumentErrorExt at parse time
        because they are mutually exclusive."""
        with pytest.raises(ArgumentErrorExt):
            dummy_cmd("group_nodes_only -n light0 -G grp0")


class TestProcessGroupNodesGrpaddrStrictArgs:
    """Tests for add_group_nodes_args / process_group_nodes_args
    with the group_nodes_grpaddr_strict subparser configuration.

    Key differences from group_nodes_full:
    - add_elem_arg=False: No --elem/-e argument
    - add_elem_addrs_arg=False: No --addrs argument
    - add_group_addr_arg=True: --group-addr/-g is available
    - elem_default=None
    - elem_addrs is always None in the result
    Filtering and ordering tests are omitted because they are already
    covered by TestProcessGroupNodesFullArgs.
    """

    # -- Positive tests: --nodes, --group, --group-addr work ----------------

    def test_single_node(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Selecting a single node by name with -n returns the node and
        elem_addrs=None."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]

        dummy_cmd("group_nodes_grpaddr_strict -n light0")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 1
        assert nodes[0] is light0
        assert elem_addrs is None

    def test_multiple_nodes(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Selecting multiple nodes with -n returns nodes and
        elem_addrs=None."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]
        ctl0 = nw["ctl0"]

        dummy_cmd("group_nodes_grpaddr_strict -n light0 ctl0")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0
        assert len(nodes) == 2
        assert light0 in nodes
        assert ctl0 in nodes
        assert elem_addrs is None

    def test_group_grp0(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -G grp0 returns group nodes with group_addr and
        elem_addrs=None."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        grp0 = nw["grp0"]

        dummy_cmd("group_nodes_grpaddr_strict -G grp0")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == grp0.group_addr
        assert len(nodes) == 2
        node_names = {n.name for n in nodes}
        assert node_names == {"light0", "ctl0"}
        assert elem_addrs is None

    def test_nodes_with_group_addr(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -n with --group-addr returns specified group address and
        elem_addrs=None."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]

        dummy_cmd("group_nodes_grpaddr_strict -n light0 --group-addr 0xC100")

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0xC100
        assert len(nodes) == 1
        assert nodes[0] is light0
        assert elem_addrs is None

    def test_nodes_with_group_addr_multiple_nodes(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -n with multiple nodes and --group-addr returns specified
        group address, all nodes and elem_addrs=None."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        light0 = nw["light0"]
        ctl0 = nw["ctl0"]

        dummy_cmd(
            "group_nodes_grpaddr_strict -n light0 ctl0 --group-addr 0xC100"
        )

        group_addr, nodes, elem_addrs = dummy_cmd.process_group_nodes_args(
            dummy_cmd.pargs,
            nodes_order_property="name",
        )

        assert group_addr == 0xC100
        assert len(nodes) == 2
        assert light0 in nodes
        assert ctl0 in nodes
        assert elem_addrs is None

    # -- Negative tests: unsupported arguments and conflicts ----------------

    def test_elem_arg_not_recognized(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -e when add_elem_arg=False raises ArgumentErrorExt at
        parse time because the argument is not recognized."""
        with pytest.raises(ArgumentErrorExt):
            dummy_cmd("group_nodes_grpaddr_strict -n light0 -e 0")

    def test_addrs_arg_not_recognized(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using --addrs when add_elem_addrs_arg=False raises
        ArgumentErrorExt at parse time because the argument is not
        recognized."""
        with pytest.raises(ArgumentErrorExt):
            dummy_cmd("group_nodes_grpaddr_strict --addrs light0[0]")

    def test_group_with_nonzero_group_addr(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Using -G with a non-zero group address and --group-addr with a
        non-zero value raises ArgumentErrorExt."""
        nw = nw_gr0_light0_ctl0_grp1_light0_ctl1
        grp0 = nw["grp0"]
        assert grp0.group_addr != 0

        dummy_cmd("group_nodes_grpaddr_strict -G grp0 --group-addr 0xC200")

        with pytest.raises(ArgumentErrorExt):
            dummy_cmd.process_group_nodes_args(
                dummy_cmd.pargs,
                nodes_order_property="name",
            )

    def test_no_target_argument(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Omitting both -n and -G (providing only --group-addr) raises
        ArgumentErrorExt because one of -n or -G is required."""
        with pytest.raises(ArgumentErrorExt):
            dummy_cmd("group_nodes_grpaddr_strict --group-addr 0xC100")

    def test_nodes_and_group_mutually_exclusive(
        self,
        dummy_cmd: DummyBtmeshCmd,
        nw_gr0_light0_ctl0_grp1_light0_ctl1: Dict[str, Node],
    ):
        """Passing both -n and -G raises ArgumentErrorExt at parse time
        because they are mutually exclusive."""
        with pytest.raises(ArgumentErrorExt):
            dummy_cmd("group_nodes_grpaddr_strict -n light0 -G grp0")
