# Copyright 2025 Silicon Laboratories Inc. www.silabs.com
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

from typing import Iterable, Optional

import btmesh.util
from btmesh.db import Node
from btmesh.util import BtmeshRetryParams

from .btmesh import app_btmesh
from .cfg import app_cfg
from .db import app_db
from .ui import app_ui


class BtmeshDfuAppTermManager:
    def get_next_free_addr(self) -> int:
        # Get the last used address
        all_nodes = app_db.btmesh_db.get_node_list()
        highest_used_elem_addr = max(node.get_last_elem_addr() for node in all_nodes)
        # New address is greater by 1 than the current highest element address
        next_available_addr = highest_used_elem_addr + 1
        return next_available_addr

    def address_refresh(
        self,
        node: Node,
        new_prim_addr: Optional[int],
        timeout_sec: Optional[int],
        sync_dcd: bool = True,
        retry_params: Optional[BtmeshRetryParams] = None,
    ):
        if new_prim_addr is None:
            new_prim_addr = self.get_next_free_addr()
        if timeout_sec is None:
            timeout_sec = app_cfg.prov.refresh_timeout_s_default
        app_ui.info(
            f"Address Refresh procedure started for {node.name} from "
            f"{hex(node.prim_addr)} to {hex(new_prim_addr)}."
        )
        app_btmesh.rpr_clt.address_refresh(
            server_addr=node.prim_addr,
            elem_idx=app_cfg.prov.rpr_elem_index,
            timeout_sec=timeout_sec,
            uuid=node.uuid,
            new_prim_addr=new_prim_addr,
            sync_dcd=sync_dcd,
            retry_params=retry_params,
        )
        app_ui.info("Address Refresh procedure completed successfully.")

    def comp_refresh(
        self,
        node: Node,
        timeout_sec: int,
        sync_dcd: bool = True,
        retry_params: Optional[BtmeshRetryParams] = None,
    ):
        if timeout_sec is None:
            timeout_sec = app_cfg.prov.refresh_timeout_s_default
        app_ui.info(
            f"Composition Refresh procedure started for {node.name} "
            f"({hex(node.prim_addr)})."
        )
        app_btmesh.rpr_clt.comp_refresh(
            server_addr=node.prim_addr,
            elem_idx=app_cfg.prov.rpr_elem_index,
            timeout_sec=timeout_sec,
            uuid=node.uuid,
            sync_dcd=sync_dcd,
            retry_params=retry_params,
        )
        app_ui.info("Composition Refresh procedure completed successfully.")

    def start_new_term(
        self,
        node: Node,
        timeout_sec: Optional[int] = None,
        new_prim_addr: Optional[int] = None,
        sync_dcd: bool = True,
        retry_params: Optional[BtmeshRetryParams] = None,
    ):
        dcd_p0, dcd_p128 = app_btmesh.rpr_clt.get_dcd_pages(
            node=node, sync=sync_dcd, retry_params=retry_params
        )
        if dcd_p0.elem_count < dcd_p128.elem_count:
            self.address_refresh(
                node=node,
                new_prim_addr=new_prim_addr,
                timeout_sec=timeout_sec,
                sync_dcd=sync_dcd,
                retry_params=retry_params,
            )
        else:
            self.comp_refresh(
                node=node,
                timeout_sec=timeout_sec,
                sync_dcd=sync_dcd,
                retry_params=retry_params,
            )

    def start_new_terms(
        self,
        nodes: Iterable[Node],
        timeout_sec: Optional[int] = None,
        sync_dcd: bool = True,
        retry_params: Optional[BtmeshRetryParams] = None,
    ):
        for node in nodes:
            try:
                self.start_new_term(
                    node=node,
                    timeout_sec=timeout_sec,
                    sync_dcd=sync_dcd,
                    retry_params=retry_params,
                )
            except Exception as e:
                app_ui.error(
                    f"Failed to start new term for {node.name} "
                    f"({hex(node.prim_addr)}): {e}"
                )
        app_ui.info("All nodes have been processed.")


app_term = BtmeshDfuAppTermManager()
