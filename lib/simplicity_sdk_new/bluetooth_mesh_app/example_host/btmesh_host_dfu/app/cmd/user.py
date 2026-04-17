# Copyright 2022 Silicon Laboratories Inc. www.silabs.com
#
# SPDX-License-Identifier: Zlib
#
# The licensor of this software is Silicon Laboratories Inc.
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

from typing import Dict, Iterable, Optional, Set, Tuple

from btmesh.db import ModelID
from btmesh.util import BtmeshRetryParams

from ..cfg import BtmeshDfuAppMdlProfile, app_cfg
from ..db import app_db
from ..grpctrl import app_grctrl
from ..ui import app_ui
from ..util.argparsex import ArgumentParserExt
from .cmd import BtmeshCmd


class BtmeshUserCmd(BtmeshCmd):
    @property
    def parser(self) -> ArgumentParserExt:
        return self.user_parser

    @property
    def current_parser(self) -> Optional[ArgumentParserExt]:
        return self.parser

    def create_parser(self, subparsers) -> ArgumentParserExt:
        self.user_parser = subparsers.add_parser(
            "user",
            help="User command for prototyping and testing.",
            description=(
                "The user command is just a skeleton with common arguments to "
                "select nodes and timeout values for the command. "
                "The command could be implemented by the user to prototype a "
                "new feature or test existing features of this application."
            ),
            exit_on_error_ext=False,
        )
        self.add_group_nodes_args(
            self.user_parser,
            add_elem_arg=True,
            add_elem_addrs_arg=True,
            add_group_addr_arg=True,
        )
        self.add_mdls_arg(self.user_parser)
        self.add_btmesh_multicast_basic_retry_args(self.user_parser)

    def __call__(self, arg) -> bool:
        pargs = self.parser.parse_args(arg.split())
        group_addr, nodes, elem_addrs = self.process_group_nodes_args(
            pargs,
            nodes_order_property="name",
            group_order_property="name",
        )
        mdls = self.process_mdls_arg(pargs)
        retry_params_default = app_cfg.common.btmesh_multicast_retry_params_default
        retry_params = self.process_btmesh_multicast_retry_params(
            pargs, retry_params_default
        )
        app_ui.info(f"Implement user command in {__file__}.")


user_cmd = BtmeshUserCmd()
