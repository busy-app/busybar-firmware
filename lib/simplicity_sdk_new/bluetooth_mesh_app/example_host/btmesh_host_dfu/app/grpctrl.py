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

import logging
from typing import Dict, Optional, Set, Tuple

import btmesh.util
from btmesh.db import (
    BtmeshDbNodeNewTermEvent,
    BtmeshDbNodeRemovedEvent,
    ElementRef,
    ModelID,
    Node,
)
from btmesh.util import BtmeshRetryParams

from .btmesh import app_btmesh
from .db import BtmeshDfuAppDbClearedEvent, BtmeshDfuAppGroup, app_db

logger = logging.getLogger(__name__)


class BtmeshDfuAppGroupController:
    NETKEY_IDX = 0

    def __init__(self) -> None:
        # One for bind, pub, sub
        # (elem, mdl) --> set(app_group1_name, app_group2_name, ...)
        self.pub_info: Dict[Tuple[ElementRef, ModelID], Set[str]] = {}
        self.sub_info: Dict[Tuple[ElementRef, ModelID], Set[str]] = {}
        self.bind_info: Dict[Tuple[ElementRef, ModelID], Set[str]] = {}

    def _add_pub_info(self, app_group_name: str, elem: ElementRef, mdl: ModelID):
        if (elem, mdl) not in self.pub_info:
            self.pub_info[(elem, mdl)] = set()
        self.pub_info[(elem, mdl)].add(app_group_name)

    def _remove_pub_info(self, app_group_name: str, elem: ElementRef, mdl: ModelID):
        self.pub_info[(elem, mdl)].remove(app_group_name)
        if len(self.pub_info[(elem, mdl)]) == 0:
            del self.pub_info[(elem, mdl)]

    def _add_sub_info(self, app_group_name: str, elem: ElementRef, mdl: ModelID):
        if (elem, mdl) not in self.sub_info:
            self.sub_info[(elem, mdl)] = set()
        self.sub_info[(elem, mdl)].add(app_group_name)

    def _remove_sub_info(self, app_group_name: str, elem: ElementRef, mdl: ModelID):
        self.sub_info[(elem, mdl)].remove(app_group_name)
        if len(self.sub_info[(elem, mdl)]) == 0:
            del self.sub_info[(elem, mdl)]

    def _add_bind_info(self, app_group_name: str, elem: ElementRef, mdl: ModelID):
        if (elem, mdl) not in self.bind_info:
            self.bind_info[(elem, mdl)] = set()
        self.bind_info[(elem, mdl)].add(app_group_name)

    def _remove_bind_info(self, app_group_name: str, elem: ElementRef, mdl: ModelID):
        self.bind_info[(elem, mdl)].remove(app_group_name)
        if len(self.bind_info[(elem, mdl)]) == 0:
            del self.bind_info[(elem, mdl)]

    def _is_appkey_required_on_node(self, appkey_index: int, node: Node):
        for elem in node.elem_refs:
            for bind_elem, bind_mdl in self.bind_info:
                if elem == bind_elem:
                    for app_group_name in self.bind_info[(bind_elem, bind_mdl)]:
                        app_group = app_db.get_app_group_by_name(app_group_name)
                        if app_group.appkey_index == appkey_index:
                            return True
        return False

    def _build_pub_info(self):
        for app_group in app_db.gen_app_groups():
            for pub_elem in app_group.pub_elems:
                for pub_mdl in app_group.gen_pub_elem_mdls(pub_elem):
                    self._add_pub_info(app_group.name, pub_elem, pub_mdl)

    def _build_sub_info(self):
        for app_group in app_db.gen_app_groups():
            for sub_elem in app_group.sub_elems:
                for sub_mdl in app_group.gen_sub_elem_mdls(sub_elem):
                    self._add_sub_info(app_group.name, sub_elem, sub_mdl)

    def _build_bind_info(self):
        for app_group in app_db.gen_app_groups():
            for bind_elem in app_group.bind_elems:
                for bind_mdl in app_group.gen_bind_elem_mdls(bind_elem):
                    self._add_bind_info(app_group.name, bind_elem, bind_mdl)

    def build_info(self):
        self._build_pub_info()
        self._build_sub_info()
        self._build_bind_info()

    def clear_info(self):
        self.pub_info.clear()
        self.sub_info.clear()
        self.bind_info.clear()

    def setup(self):
        self.build_info()
        app_btmesh.core.subscribe("btmesh_levt_app_db_cleared", self.on_app_db_cleared)
        app_btmesh.core.subscribe(
            "btmesh_levt_db_node_removed", self.on_db_node_removed
        )
        app_btmesh.core.subscribe(
            "btmesh_levt_db_node_new_term", self.on_db_node_new_term
        )

    def on_app_db_cleared(self, event: BtmeshDfuAppDbClearedEvent):
        self.clear_info()

    def on_db_node_removed(self, event: BtmeshDbNodeRemovedEvent):
        node = event.node
        self.remove_node(node, local=True)

    def on_db_node_new_term(self, event: BtmeshDbNodeNewTermEvent):
        node = event.node
        if not event.orig_dcd:
            return
        for elem_index in range(event.orig_dcd.elem_count):
            for model_id in event.orig_dcd.elements[elem_index].models:
                if node.has_model(elem_index=elem_index, model_id=model_id):
                    continue
                addr = node.get_elem_addr(elem_index=elem_index)
                # The model is not present on the node anymore in its new term
                # after composition refresh or address refresh (e.g. due to FW
                # update) so the model shall be removed from the app groups.
                self.remove_elem_mdl(
                    addr=addr,
                    mdl=model_id,
                    local=True,
                )

    def remove_node(
        self,
        node: Node,
        local: bool = False,
        retry_params: BtmeshRetryParams = None,
    ):
        for addr in node.elem_addrs:
            self.remove_elem_addr(
                node=node, addr=addr, local=local, retry_params=retry_params
            )

    def remove_elem_addr(
        self,
        addr: int,
        local: bool = False,
        retry_params: BtmeshRetryParams = None,
        node: Optional[Node] = None,
    ):
        btmesh.util.validate_unicast_address(addr)
        if node:
            # It is essential to use the node parameter when the node is removed
            # because the node is no longer part of the database.
            # The performance is better when the node is known because it is not
            # necessary to search for the node in the database.
            target_elem = node.get_elem_ref(elem_addr=addr)
        else:
            target_elem = app_db.btmesh_db.get_elem_ref(addr)
        # Create list of operations for publication, subscription and binding.
        # This is essential because the remove_pub_mdl, remove_sub_mdl and
        # remove_bind_mdl methods modifies the pub_info, sub_info, bind_info
        # dictionaries which needs to be iterated as well.
        # Note: It may result in incorrect behavior when a collection is modified
        #       during the iteration of the same collection.
        pub_mdls = [mdl for elem, mdl in self.pub_info.keys() if elem == target_elem]
        sub_mdls = [mdl for elem, mdl in self.sub_info.keys() if elem == target_elem]
        bind_mdls = [mdl for elem, mdl in self.bind_info.keys() if elem == target_elem]
        for mdl in pub_mdls:
            app_group_names = set(self.pub_info[(target_elem, mdl)])
            for app_group_name in app_group_names:
                # Each model can publish to a single group address only and
                # hence to a single app group.
                # Defensive programming: iterate over the values.
                # Note: The auto_unbind feature is not used because the whole
                #       node is removed so all bindings are removed as well.
                self.remove_pub_mdl(
                    app_group_name=app_group_name,
                    addr=addr,
                    mdl=mdl,
                    auto_unbind=False,
                    local=local,
                    retry_params=retry_params,
                    node=node,
                )
        for mdl in sub_mdls:
            app_group_names = set(self.sub_info[(target_elem, mdl)])
            for app_group_name in app_group_names:
                # Each model can be subscribed to multiple group addresses and
                # hence to multiple app groups.
                # Note: The auto_unbind feature is not used because the whole
                #       node is removed so all bindings are removed as well.
                self.remove_sub_mdl(
                    app_group_name=app_group_name,
                    addr=addr,
                    mdl=mdl,
                    auto_unbind=False,
                    local=local,
                    retry_params=retry_params,
                    node=node,
                )
        for mdl in bind_mdls:
            app_group_names = set(self.bind_info[(target_elem, mdl)])
            for app_group_name in app_group_names:
                # Each model can be bound to multiple application keys and hence
                # to multiple app groups.
                self.remove_bind_mdl(
                    app_group_name=app_group_name,
                    addr=addr,
                    mdl=mdl,
                    local=local,
                    retry_params=retry_params,
                    node=node,
                )

    def remove_elem_mdl(
        self,
        addr: int,
        mdl: ModelID,
        local: bool = False,
        retry_params: BtmeshRetryParams = None,
        node: Optional[Node] = None,
    ):
        btmesh.util.validate_unicast_address(addr)
        if node:
            # It is essential to use the node parameter when the node is removed
            # because the node is no longer part of the database.
            # The performance is better when the node is known because it is not
            # necessary to search for the node in the database.
            elem = node.get_elem_ref(elem_addr=addr)
        else:
            elem = app_db.btmesh_db.get_elem_ref(addr)
        if (elem, mdl) in self.pub_info:
            app_group_names = set(self.pub_info[(elem, mdl)])
            for app_group_name in app_group_names:
                # Each model can publish to a single group address only and
                # hence to a single app group.
                # Defensive programming: iterate over the values.
                # Note: The auto_unbind feature is not used because the element
                #       address and model pair is removed completely from the
                #       app group.
                self.remove_pub_mdl(
                    app_group_name=app_group_name,
                    addr=addr,
                    mdl=mdl,
                    auto_unbind=False,
                    local=local,
                    retry_params=retry_params,
                    node=node,
                )
        if (elem, mdl) in self.sub_info:
            app_group_names = set(self.sub_info[(elem, mdl)])
            for app_group_name in app_group_names:
                # Each model can be subscribed to multiple group addresses and
                # hence to multiple app groups.
                # Note: The auto_unbind feature is not used because the element
                #       address and model pair is removed completely from the
                #       app group.
                self.remove_sub_mdl(
                    app_group_name=app_group_name,
                    addr=addr,
                    mdl=mdl,
                    auto_unbind=False,
                    local=local,
                    retry_params=retry_params,
                    node=node,
                )
        if (elem, mdl) in self.bind_info:
            app_group_names = set(self.bind_info[(elem, mdl)])
            for app_group_name in app_group_names:
                # Each model can be bound to multiple application keys and hence
                # to multiple app groups.
                self.remove_bind_mdl(
                    app_group_name=app_group_name,
                    addr=addr,
                    mdl=mdl,
                    local=local,
                    retry_params=retry_params,
                    node=node,
                )

    def add_app_group(
        self,
        name: str,
        group_addr: int,
        appkey_index: int,
        pub_ttl: int = 5,
        pub_credentials: int = 0,
        pub_period_ms: int = 0,
        pub_retransmit_count: int = 0,
        pub_retransmit_interval_ms: int = 0,
    ):
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

    def rename_app_group(self, current_name, new_name):
        app_db.rename_app_group(current_name=current_name, new_name=new_name)
        # If the rename fails then an exception is raised so this code runs only
        # when the rename was successful in the application database
        for app_group_set in self.pub_info.values():
            if current_name in app_group_set:
                app_group_set.remove(current_name)
                app_group_set.add(new_name)
        for app_group_set in self.sub_info.values():
            if current_name in app_group_set:
                app_group_set.remove(current_name)
                app_group_set.add(new_name)
        for app_group_set in self.bind_info.values():
            if current_name in app_group_set:
                app_group_set.remove(current_name)
                app_group_set.add(new_name)

    def add_pub_mdl(
        self,
        app_group_name: str,
        addr: int,
        mdl: ModelID,
        auto_bind=True,
        retry_params: BtmeshRetryParams = None,
        node: Optional[Node] = None,
    ):
        if node is None:
            node = app_db.btmesh_db.get_node_by_elem_addr(addr)
        elem = node.get_elem_ref(elem_addr=addr)
        app_group = app_db.get_app_group_by_name(app_group_name)
        other_app_group_names = self.pub_info.get((elem, mdl), None)
        if app_group.has_pub_elem(elem) and app_group.has_pub_elem_mdl(elem, mdl):
            raise ValueError(
                f"The {mdl.pretty_name()} model name on 0x{addr:04X} element "
                f"address already publishes to {app_group_name} app group."
            )
        if other_app_group_names:
            other_app_group_name = next(iter(other_app_group_names))
            raise ValueError(
                f"The {mdl.pretty_name()} model name on 0x{addr:04X} element "
                f"address can publish to one app group only. It is already in "
                f"{other_app_group_name} app group."
            )
        if auto_bind and not (
            app_group.has_bind_elem(elem) and app_group.has_bind_elem_mdl(elem, mdl)
        ):
            self.add_bind_mdl(app_group_name=app_group_name, addr=addr, mdl=mdl)
        app_btmesh.conf.set_model_pub(
            node=node,
            elem_index=elem.elem_index,
            model=mdl,
            pub_address=app_group.group_addr,
            appkey_index=app_group.appkey_index,
            credentials=app_group.pub_credentials,
            ttl=app_group.pub_ttl,
            period_ms=app_group.pub_period_ms,
            retransmit_count=app_group.pub_retransmit_count,
            retransmit_interval_ms=app_group.pub_retransmit_interval_ms,
            retry_params=retry_params,
        )
        app_group.add_pub_elem_mdl(elem=elem, mdl=mdl)
        self._add_pub_info(app_group_name=app_group.name, elem=elem, mdl=mdl)

    def remove_pub_mdl(
        self,
        app_group_name: str,
        addr: int,
        mdl: ModelID,
        auto_unbind=True,
        local: bool = False,
        retry_params: BtmeshRetryParams = None,
        node: Optional[Node] = None,
    ):
        if node is None:
            node = app_db.btmesh_db.get_node_by_elem_addr(addr)
        elem = node.get_elem_ref(elem_addr=addr)
        app_group = app_db.get_app_group_by_name(app_group_name)
        if not (app_group.has_pub_elem(elem) and app_group.has_pub_elem_mdl(elem, mdl)):
            raise ValueError(
                f"The {mdl.pretty_name()} model name on 0x{addr:04X} element "
                f"address does not publish to {app_group_name} app group."
            )
        if not local:
            # If local is set then the operation affects the database only and
            # no message is sent to the remote node.
            app_btmesh.conf.set_model_pub(
                node=node,
                elem_index=elem.elem_index,
                model=mdl,
                pub_address=btmesh.util.UNASSIGNED_ADDR,
                appkey_index=0,
                credentials=0,
                ttl=0,
                period_ms=0,
                retransmit_count=0,
                retransmit_interval_ms=0,
                retry_params=retry_params,
            )
            if (
                auto_unbind
                and app_group.has_bind_elem(elem)
                and app_group.has_bind_elem_mdl(elem, mdl)
                and not (
                    app_group.has_sub_elem(elem)
                    and app_group.has_sub_elem_mdl(elem, mdl)
                )
            ):
                self.remove_bind_mdl(
                    app_group_name=app_group_name, addr=addr, mdl=mdl, node=node
                )
        app_group.remove_pub_elem_mdl(elem=elem, mdl=mdl)
        self._remove_pub_info(app_group_name=app_group.name, elem=elem, mdl=mdl)

    def add_sub_mdl(
        self,
        app_group_name: str,
        addr: int,
        mdl: ModelID,
        auto_bind=True,
        retry_params: BtmeshRetryParams = None,
        node: Optional[Node] = None,
    ):
        if node is None:
            node = app_db.btmesh_db.get_node_by_elem_addr(addr)
        elem = node.get_elem_ref(elem_addr=addr)
        app_group = app_db.get_app_group_by_name(app_group_name)
        if app_group.has_sub_elem(elem) and app_group.has_sub_elem_mdl(elem, mdl):
            raise ValueError(
                f"The {mdl.pretty_name()} model name on 0x{addr:04X} element "
                f"address is already subscribed to {app_group_name} app group."
            )
        for other_app_group_name in self.sub_info.get((addr, mdl), set()):
            # The other app group shall exists so no exception can occur here
            other_app_group = app_db.get_app_group_by_name(other_app_group_name)
            if app_group.group_addr == other_app_group.group_addr:
                # The model is subscribed to the same group address due to
                # another app group so no operation shall be performed which
                # is forbidden for subscription because it may lead to unexpected
                # behavior.
                raise ValueError(
                    f"The {mdl.pretty_name()} model name on 0x{addr:04X} element "
                    f"is already subscribed to 0x{other_app_group.group_addr:04X} "
                    f"group address by {other_app_group_name} app group so it "
                    f"can't be added to {app_group_name} app group."
                )
        if auto_bind and not (
            app_group.has_bind_elem(elem) and app_group.has_bind_elem_mdl(elem, mdl)
        ):
            self.add_bind_mdl(app_group_name=app_group_name, addr=addr, mdl=mdl)
        app_btmesh.conf.add_model_sub(
            node=node,
            elem_index=elem.elem_index,
            model=mdl,
            sub_address=app_group.group_addr,
            retry_params=retry_params,
        )
        app_group.add_sub_elem_mdl(elem=elem, mdl=mdl)
        self._add_sub_info(app_group_name=app_group.name, elem=elem, mdl=mdl)

    def remove_sub_mdl(
        self,
        app_group_name: str,
        addr: int,
        mdl: ModelID,
        auto_unbind=True,
        local: bool = False,
        retry_params: BtmeshRetryParams = None,
        node: Optional[Node] = None,
    ):
        if node is None:
            node = app_db.btmesh_db.get_node_by_elem_addr(addr)
        elem = node.get_elem_ref(elem_addr=addr)
        app_group = app_db.get_app_group_by_name(app_group_name)
        if not (app_group.has_sub_elem(elem) and app_group.has_sub_elem_mdl(elem, mdl)):
            raise ValueError(
                f"The {mdl.pretty_name()} model name on 0x{addr:04X} element "
                f"address is not subscribed to {app_group_name} app group."
            )
        if not local:
            # If local is set then the operation affects the database only and
            # no message is sent to the remote node.
            app_btmesh.conf.remove_model_sub(
                node=node,
                elem_index=elem.elem_index,
                model=mdl,
                sub_address=app_group.group_addr,
                retry_params=retry_params,
            )
            if (
                auto_unbind
                and app_group.has_bind_elem(elem)
                and app_group.has_bind_elem_mdl(elem, mdl)
                and not (
                    app_group.has_pub_elem(elem)
                    and app_group.has_pub_elem_mdl(elem, mdl)
                )
            ):
                # If the same model on an element publishes and subscribes to
                # the same app group then appkey shall not be unbound by the
                # auto_unbind feature until both of these are removed.
                self.remove_bind_mdl(
                    app_group_name=app_group_name, addr=addr, mdl=mdl, node=node
                )
        app_group.remove_sub_elem_mdl(elem=elem, mdl=mdl)
        self._remove_sub_info(app_group_name=app_group.name, elem=elem, mdl=mdl)

    def add_bind_mdl(
        self,
        app_group_name: str,
        addr: int,
        mdl: ModelID,
        auto_bind=False,
        retry_params: BtmeshRetryParams = None,
        node: Optional[Node] = None,
    ):
        if node is None:
            node = app_db.btmesh_db.get_node_by_elem_addr(addr)
        elem = node.get_elem_ref(elem_addr=addr)
        app_group = app_db.get_app_group_by_name(app_group_name)
        if app_group.has_bind_elem(elem) and app_group.has_bind_elem_mdl(elem, mdl):
            if auto_bind:
                return
            raise ValueError(
                f"The {mdl.pretty_name()} model name on 0x{addr:04X} element "
                f"address is already bound to {app_group_name} app group."
            )
        for other_app_group_name in self.bind_info.get((elem, mdl), set()):
            # The other app group shall exists so no exception can occur here
            other_app_group = app_db.get_app_group_by_name(other_app_group_name)
            if app_group.appkey_index == other_app_group.appkey_index:
                # The appkey_index is not bound to the model by another app group
                # so no operation shall be performed
                break
        else:
            # The appkey_index is not bound to the model by another app group
            # so the bind operation is necessary
            if not node.has_appkey_index(app_group.appkey_index):
                # The appkey was not uploaded to the node previously so
                # this is the first model which will use it
                app_btmesh.conf.add_appkey(
                    node=node,
                    appkey_index=app_group.appkey_index,
                    netkey_index=self.NETKEY_IDX,
                    retry_params=retry_params,
                )
            app_btmesh.conf.bind_model(
                node=node,
                elem_index=elem.elem_index,
                model=mdl,
                appkey_index=app_group.appkey_index,
                retry_params=retry_params,
            )
            app_group.add_bind_elem_mdl(elem=elem, mdl=mdl)
            self._add_bind_info(app_group_name=app_group.name, elem=elem, mdl=mdl)

    def remove_bind_mdl(
        self,
        app_group_name: str,
        addr: int,
        mdl: ModelID,
        local: bool = False,
        retry_params: BtmeshRetryParams = None,
        node: Optional[Node] = None,
    ):
        if node is None:
            node = app_db.btmesh_db.get_node_by_elem_addr(addr)
        elem = node.get_elem_ref(elem_addr=addr)
        app_group = app_db.get_app_group_by_name(app_group_name)
        if not (
            app_group.has_bind_elem(elem) and app_group.has_bind_elem_mdl(elem, mdl)
        ):
            raise ValueError(
                f"The {mdl.pretty_name()} model name on 0x{addr:04X} element "
                f"address is not bound to {app_group_name} app group."
            )
        if not local:
            for other_app_group_name in self.bind_info.get((elem, mdl), set()):
                # The other app group shall exists so no exception can occur here
                other_app_group = app_db.get_app_group_by_name(other_app_group_name)
                # Iterate over the appkey bindings of other app groups to check
                # if other app groups require the binding to the same app key
                if app_group_name == other_app_group_name:
                    continue
                if app_group.appkey_index == other_app_group.appkey_index:
                    # Other app group specifies the same appkey index for the
                    # model on the element address so the appkey binding shall
                    # be kept on the remote node but it shall be removed from
                    # the app group in the app database.
                    break
            else:
                # The appkey index is bound to the model on the element address
                # only due the app group which removes the bind model so the an
                # unbind operation is necessary on the remote node
                app_btmesh.conf.unbind_model(
                    node=node,
                    elem_index=elem.elem_index,
                    model=mdl,
                    appkey_index=app_group.appkey_index,
                    retry_params=retry_params,
                )
        app_group.remove_bind_elem_mdl(elem=elem, mdl=mdl)
        self._remove_bind_info(app_group_name=app_group.name, elem=elem, mdl=mdl)
        if not local and not self._is_appkey_required_on_node(
            appkey_index=app_group.appkey_index, node=node
        ):
            # The appkey is no longer necessary on the node because no model
            # is bound to it and therefore it can be removed
            app_btmesh.conf.remove_appkey(
                node=node,
                appkey_index=app_group.appkey_index,
                netkey_index=self.NETKEY_IDX,
                retry_params=retry_params,
            )


app_grctrl = BtmeshDfuAppGroupController()
