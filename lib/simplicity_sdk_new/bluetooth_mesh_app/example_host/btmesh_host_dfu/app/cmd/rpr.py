from typing import Optional

from ..util.argparsex import ArgumentParserExt
from .cmd import BtmeshCmd
from ..btmesh import app_btmesh
from ..cfg import app_cfg
from ..db import app_db
from ..ui import app_ui
from ..term import app_term


class BtmeshAddressRefreshCmd(BtmeshCmd):
    @property
    def parser(self) -> ArgumentParserExt:
        return self.ar_parser

    @property
    def current_parser(self) -> Optional[ArgumentParserExt]:
        return self.parser

    def create_parser(self, subparsers) -> ArgumentParserExt:
        self.ar_parser: ArgumentParserExt = subparsers.add_parser(
            "ar",
            help="Start address refresh procedures.",
            description=("Start address refresh procedures."),
            exit_on_error_ext=False,
        )
        self.ar_parser.add_argument(
            "-a",
            "--address",
            type=str,
            help=(
                "The FIRST new primary element address of the nodes. "
                "If unspecified, the script will use the first available "
                "address that is higher than the current known maximum. "
                "If specified and several nodes are selected for address "
                "refresh, the new addresses will be consecutive. Address "
                "collision checking is not performed, the user has to "
                "make sure that there will be none. "
                "If address refresh fails for a node, its calculated "
                "new address will be used for the next node. "
                "Accepts integers, binary (0b), octal (0o) or hexadecimal "
                "(0x) strings. "
            ),
        )
        self.ar_parser.add_argument(
            "-t",
            "--timeout",
            type=int,
            default=app_cfg.prov.refresh_timeout_s_default,
            help=("Timeout for address refresh procedure, in seconds."),
        )
        self.add_group_nodes_args(
            self.ar_parser,
            add_elem_arg=False,
            add_elem_addrs_arg=False,
            add_group_addr_arg=False,
            elem_default=0,
            group_help=(
                f"Specifies the app group which contains the nodes where the "
                f"Address Refresh procedure shall be initiated. "
                f"The nodes in the group are refreshed one by one. "
            ),
            nodes_help=(
                f"List of nodes where the Address Refresh procedure shall be initiated."
            ),
        )

    def __call__(self, arg) -> bool:
        pargs = self.parser.parse_args(arg.split())
        return self._address_refresh(pargs)

    def _address_refresh(self, pargs) -> bool:
        _, nodes, _ = self.process_group_nodes_args(
            pargs,
            nodes_order_property="name",
            group_order_property="name",
        )
        new_prim_addr = str(pargs.address) if pargs.address else None
        if new_prim_addr:
            new_prim_addr = int(new_prim_addr, 0)
        else:
            new_prim_addr = app_term.get_next_free_addr()
        for node in nodes:
            app_term.address_refresh(
                node=node,
                new_prim_addr=new_prim_addr,
                timeout_sec=pargs.timeout
            )
            # For the next nodes, use consecutive addresses
            # If the address refresh failed, don't change new_addr
            new_node = app_db.btmesh_db.get_node_by_prim_addr(new_prim_addr)
            last_addr = new_node.get_last_elem_addr()
            new_prim_addr = last_addr + 1
        return False


class BtmeshCompRefreshCmd(BtmeshCmd):
    @property
    def parser(self) -> ArgumentParserExt:
        return self.cr_parser

    @property
    def current_parser(self) -> Optional[ArgumentParserExt]:
        return self.parser

    def create_parser(self, subparsers) -> ArgumentParserExt:
        self.cr_parser: ArgumentParserExt = subparsers.add_parser(
            "cr",
            help="Start composition refresh procedures.",
            description=(
                "Start composition refresh procedures. "
                "NOTE: If the node's Composition Data changed in such a way "
                "that the node's element count changed, the node needs to be "
                "refreshed by the Address Refresh procedure, which will "
                "automatically trigger a Composition Refresh as well."
            ),
            exit_on_error_ext=False,
        )
        self.cr_parser.add_argument(
            "-t",
            "--timeout",
            type=int,
            default=app_cfg.prov.refresh_timeout_s_default,
            help="Timeout for composition refresh procedure, in seconds.",
        )
        self.add_group_nodes_args(
            self.cr_parser,
            add_elem_arg=False,
            add_elem_addrs_arg=False,
            add_group_addr_arg=False,
            elem_default=0,
            group_help=(
                f"Specifies the app group which contains the nodes where the "
                f"Composition Refresh procedure shall be initiated. "
                f"The nodes in the group are refreshed one by one. "
            ),
            nodes_help=(
                f"List of nodes where the Composition Refresh procedure shall be initiated."
            ),
        )

    def __call__(self, arg) -> bool:
        pargs = self.parser.parse_args(arg.split())

        return self._comp_refresh(pargs)

    def _comp_refresh(self, pargs) -> bool:
        _, nodes, _ = self.process_group_nodes_args(
            pargs,
            nodes_order_property="name",
            group_order_property="name",
        )
        for node in nodes:
            app_term.comp_refresh(node=node, timeout_sec=pargs.timeout)
        return False


ar_cmd = BtmeshAddressRefreshCmd()
cr_cmd = BtmeshCompRefreshCmd()
