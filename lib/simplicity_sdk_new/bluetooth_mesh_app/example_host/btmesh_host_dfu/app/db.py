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

import dataclasses
import json
import operator
from datetime import datetime
from pathlib import Path
from typing import ClassVar, Dict, Iterable, Mapping, Optional, Union

import btmesh.util
import btmesh.db
from btmesh.db import BtmeshDatabase, ElementRef, FWID, ModelID, DatabaseVersion
from btmesh.event import LocalEvent
from btmesh.statedict import StateDictObject
from btmesh.util import BtmeshRetryParams

from .btmesh import app_btmesh
from .cfg import app_cfg
from .rnode import RemoteNode


class BtmeshDfuAppDbLoadError(Exception):
    def __init__(self, json_path, *args: object):
        super().__init__(*args)
        self.json_path = json_path


@dataclasses.dataclass
class BtmeshDfuAppDbClearedEvent(LocalEvent):
    name: ClassVar[str] = "btmesh_levt_app_db_cleared"


class BtmeshDfuAppGroup(StateDictObject):
    @classmethod
    def validate_elem(
        cls,
        elems_dict,
        elem: ElementRef,
        elem_name: str,
        shall_exist=None,
    ):
        if shall_exist is not None:
            if shall_exist and (elem not in elems_dict):
                raise ValueError(
                    f"App group {elem_name} element {elem} does not exists."
                )
            elif (not shall_exist) and (elem in elems_dict):
                raise ValueError(
                    f"App group {elem_name} element {elem} already exists."
                )

    @classmethod
    def add_elem(cls, elems_dict, elem: ElementRef, elems_name: str):
        cls.validate_elem(elems_dict, elem, elems_name, shall_exist=False)
        elems_dict[elem] = set()

    @classmethod
    def remove_elem(cls, elems_dict, elem, elems_name):
        cls.validate_elem(elems_dict, elem, elems_name, shall_exist=True)
        return elems_dict.pop(elem)

    @classmethod
    def has_elem_mdl(cls, elems_dict, elem, mdl, elems_name):
        cls.validate_elem(elems_dict, elem, elems_name, shall_exist=True)
        return mdl in elems_dict[elem]

    @classmethod
    def add_elem_mdl(cls, elems_dict, elem, mdl, elems_name):
        cls.validate_elem(elems_dict, elem, elems_name, shall_exist=None)
        if elem not in elems_dict:
            cls.add_elem(elems_dict, elem, elems_name)
        elems_dict[elem].add(mdl)

    @classmethod
    def remove_elem_mdl(cls, elems_dict, elem, mdl, elems_name):
        cls.validate_elem(elems_dict, elem, elems_name, shall_exist=True)
        elems_dict[elem].remove(mdl)
        if not len(elems_dict[elem]):
            del elems_dict[elem]

    @classmethod
    def gen_elem_mdls(cls, elems_dict, elem, elems_name) -> Iterable[ModelID]:
        cls.validate_elem(elems_dict, elem, elems_name, shall_exist=True)
        return (mdl for mdl in elems_dict.get(elem, set()))

    def create_mdl(self, attr_name, raw_mdl):
        if isinstance(raw_mdl, Mapping):
            mdl = ModelID.create_from_dict(raw_mdl)
        elif isinstance(raw_mdl, ModelID):
            mdl = raw_mdl
        else:
            self.raise_construction_error(attr_name, raw_mdl, type_error=True)
        return mdl

    def __init__(
        self,
        name,
        group_addr=0,
        appkey_index=0,
        pub_ttl=5,
        pub_credentials=0,
        pub_period_ms=0,
        pub_retransmit_count=0,
        pub_retransmit_interval_ms=0,
        pub_elems={},
        sub_elems={},
        bind_elems={},
    ) -> None:
        super().__init__()
        self.name = name
        self.group_addr = group_addr
        self.appkey_index = appkey_index
        self.pub_ttl = pub_ttl
        self.pub_credentials = pub_credentials
        self.pub_period_ms = pub_period_ms
        self.pub_retransmit_count = pub_retransmit_count
        self.pub_retransmit_interval_ms = pub_retransmit_interval_ms
        self._pub_elems = {}
        self._sub_elems = {}
        self._bind_elems = {}
        pub_elems = {
            ElementRef.create_from_dict(elem): mdls for elem, mdls in pub_elems.items()
        }
        sub_elems = {
            ElementRef.create_from_dict(elem): mdls for elem, mdls in sub_elems.items()
        }
        bind_elems = {
            ElementRef.create_from_dict(elem): mdls for elem, mdls in bind_elems.items()
        }
        for elem in pub_elems:
            self.add_pub_elem(elem)
            for raw_mdl in pub_elems[elem]:
                mdl = self.create_mdl(f"pub_elems[{elem}] mdl", raw_mdl)
                self.add_pub_elem_mdl(elem, mdl)
        for elem in sub_elems:
            self.add_sub_elem(elem)
            for raw_mdl in sub_elems[elem]:
                mdl = self.create_mdl(f"sub_elems[{elem}] mdl", raw_mdl)
                self.add_sub_elem_mdl(elem, mdl)
        for elem in bind_elems:
            self.add_bind_elem(elem)
            for raw_mdl in bind_elems[elem]:
                mdl = self.create_mdl(f"bind_elems[{elem}] mdl", raw_mdl)
                self.add_bind_elem_mdl(elem, mdl)

    @property
    def name(self):
        return self._name

    @name.setter
    def name(self, value):
        btmesh.util.validate_name(value, "Invalid app group name.")
        self._name = value

    @property
    def group_addr(self):
        return self._group_addr

    @group_addr.setter
    def group_addr(self, value):
        btmesh.util.validate_group_address(value, "Invalid app group address.")
        self._group_addr = btmesh.util.addr_to_int(value)

    @property
    def appkey_index(self):
        return self._appkey_index

    @appkey_index.setter
    def appkey_index(self, value):
        ERROR_MSG = "Invalid app group appkey index."
        btmesh.util.validate_appkey_index(value, ERROR_MSG)
        if app_cfg.network.appkey_cnt <= value:
            raise ValueError(
                f"{ERROR_MSG} Configured appkey count is {app_cfg.network.appkey_cnt} "
                f"but {value} appkey index is set for app group."
            )
        self._appkey_index = value

    @property
    def pub_ttl(self):
        return self._pub_ttl

    @pub_ttl.setter
    def pub_ttl(self, value):
        btmesh.util.validate_ttl(value, "Invalid app group pub TTL.")
        self._pub_ttl = value

    @property
    def pub_credentials(self):
        return self._pub_credentials

    @pub_credentials.setter
    def pub_credentials(self, value):
        btmesh.util.validate_pub_credentials(
            value, "Invalid app group pub credentials."
        )
        self._pub_credentials = value

    @property
    def pub_period_ms(self):
        return self._pub_period_ms

    @pub_period_ms.setter
    def pub_period_ms(self, value):
        btmesh.util.validate_pub_period_ms(value, "Invalid app group pub period.")
        self._pub_period_ms = value

    @property
    def pub_retransmit_count(self):
        return self._pub_retransmit_count

    @pub_retransmit_count.setter
    def pub_retransmit_count(self, value):
        btmesh.util.validate_pub_retransmit_count(
            value, "Invalid app group pub retransmit count."
        )
        self._pub_retransmit_count = value

    @property
    def pub_retransmit_interval_ms(self):
        return self._pub_retransmit_interval_ms

    @pub_retransmit_interval_ms.setter
    def pub_retransmit_interval_ms(self, value):
        btmesh.util.validate_pub_retransmit_interval_ms(
            value, "Invalid app group pub retransmit interval."
        )
        self._pub_retransmit_interval_ms = value

    # Publication element address methods
    @property
    def pub_elems(self):
        return (elem for elem in self._pub_elems)

    def has_pub_elem(self, elem: ElementRef):
        return elem in self._pub_elems

    def add_pub_elem(self, elem: ElementRef):
        self.add_elem(self._pub_elems, elem, "pub")

    def remove_pub_elem(self, elem: ElementRef):
        self.remove_elem(self._pub_elems, elem, "pub")

    def has_pub_elem_mdl(self, elem: ElementRef, mdl):
        return self.has_elem_mdl(self._pub_elems, elem, mdl, "pub")

    def add_pub_elem_mdl(self, elem: ElementRef, mdl):
        self.add_elem_mdl(self._pub_elems, elem, mdl, "pub")

    def remove_pub_elem_mdl(self, elem: ElementRef, mdl):
        self.remove_elem_mdl(self._pub_elems, elem, mdl, "pub")

    def gen_pub_elem_mdls(self, elem: ElementRef) -> Iterable[ModelID]:
        return self.gen_elem_mdls(self._pub_elems, elem, "pub")

    # Subscription element address methods
    @property
    def sub_elems(self):
        return (elem for elem in self._sub_elems)

    def has_sub_elem(self, elem: ElementRef):
        return elem in self._sub_elems

    def add_sub_elem(self, elem: ElementRef):
        self.add_elem(self._sub_elems, elem, "sub")

    def remove_sub_elem(self, elem: ElementRef):
        self.remove_elem(self._sub_elems, elem, "sub")

    def has_sub_elem_mdl(self, elem: ElementRef, mdl):
        return self.has_elem_mdl(self._sub_elems, elem, mdl, "sub")

    def add_sub_elem_mdl(self, elem: ElementRef, mdl):
        self.add_elem_mdl(self._sub_elems, elem, mdl, "sub")

    def remove_sub_elem_mdl(self, elem: ElementRef, mdl):
        self.remove_elem_mdl(self._sub_elems, elem, mdl, "sub")

    def gen_sub_elem_mdls(self, elem: ElementRef) -> Iterable[ModelID]:
        return self.gen_elem_mdls(self._sub_elems, elem, "sub")

    # Binding element address methods
    @property
    def bind_elems(self):
        return (elem for elem in self._bind_elems)

    def has_bind_elem(self, elem: ElementRef):
        return elem in self._bind_elems

    def add_bind_elem(self, elem: ElementRef):
        self.add_elem(self._bind_elems, elem, "bind")

    def remove_bind_elem(self, elem: ElementRef):
        self.remove_elem(self._bind_elems, elem, "bind")

    def has_bind_elem_mdl(self, elem, mdl):
        return self.has_elem_mdl(self._bind_elems, elem, mdl, "bind")

    def add_bind_elem_mdl(self, elem: ElementRef, mdl):
        self.add_elem_mdl(self._bind_elems, elem, mdl, "bind")

    def remove_bind_elem_mdl(self, elem: ElementRef, mdl):
        self.remove_elem_mdl(self._bind_elems, elem, mdl, "bind")

    def gen_bind_elem_mdls(self, elem) -> Iterable[ModelID]:
        return self.gen_elem_mdls(self._bind_elems, elem, "bind")

    # Binding element address methods
    @property
    def elems(self):
        # Remove duplicate values because an address might be in publication,
        # subscription and bind addresses as well
        addrs_set = set(self.pub_elems)
        addrs_set.update(self.sub_elems)
        addrs_set.update(self.bind_elems)
        yield from addrs_set


class BtmeshDfuAppDatabase(StateDictObject):
    VERSION = DatabaseVersion(2, 0)

    def __init__(
        self,
        btmesh_db=None,
        app_groups={},
        fwid_metadata_cache={},
        version: Optional[Union[DatabaseVersion, Dict]] = None,
    ):
        if version is None:
            self.version = DatabaseVersion(1, 0)
        else:
            self.version = DatabaseVersion.create_from_dict(version)
        if isinstance(btmesh_db, BtmeshDatabase):
            self.btmesh_db = btmesh_db
        elif isinstance(btmesh_db, Mapping):
            self.btmesh_db = BtmeshDatabase.create_from_dict(btmesh_db)
        else:
            self.btmesh_db = BtmeshDatabase()
        self.app_groups: Dict[str, BtmeshDfuAppGroup] = {}
        for group in app_groups.values():
            if isinstance(group, Mapping):
                self.add_app_group(BtmeshDfuAppGroup.create_from_dict(group))
            elif isinstance(group, BtmeshDfuAppGroup):
                self.add_app_group(group)
            else:
                self.raise_construction_error("group", group, type_error=True)
        self.fwid_metadata_cache: Dict[bytes, bytes] = {}
        for raw_fwid, raw_metadata in fwid_metadata_cache.items():
            fwid_bytes = StateDictObject.to_bytes(raw_fwid)
            metadata = StateDictObject.to_bytes(raw_metadata)
            self.fwid_metadata_cache[fwid_bytes] = metadata

    def clear(self):
        self.version = self.VERSION
        self.btmesh_db.clear()
        self.app_groups.clear()
        self.fwid_metadata_cache.clear()
        db_clr_event = BtmeshDfuAppDbClearedEvent()
        app_btmesh.core.emit_local_event(db_clr_event)

    def save(self, path=None):
        if path is None:
            json_path = app_cfg.persistence.path
        else:
            # Create path object from parameter to accept string as well.
            # Path class handles Path object parameter in its constructor
            # gracefully so it is not necessary to check for type.
            json_path = Path(path)
        with json_path.open("w") as json_file:
            json_dict = self.to_dict()
            json_text = json.dumps(json_dict, indent=4)
            json_file.write(json_text)

    def load(self, path=None):
        if path is None:
            json_path = app_cfg.persistence.path
        else:
            # Create path object from parameter to accept string as well.
            # Path class handles Path object parameter in its constructor
            # gracefully so it is not necessary to check for type.
            json_path = Path(path)
        if json_path.exists():
            with json_path.open() as json_file:
                json_text = json_file.read()
                try:
                    json_dict = json.loads(json_text)
                    db_changed = BtmeshDfuAppDatabaseMigration.migrate(
                        json_dict, self.VERSION, json_path
                    )
                    if db_changed:
                        self.save()
                    self.from_dict(json_dict)
                except (json.JSONDecodeError, ValueError, TypeError) as e:
                    if app_cfg.persistence.default_on_failed_load:
                        if app_cfg.persistence.backup_on_failed_load:
                            p = json_path
                            date_str = f"_{datetime.now():%Y%m%d_%H%M%S}"
                            temp_folder_path = p.parent / "temp"
                            temp_folder_path.mkdir(exist_ok=True)
                            backup_path = temp_folder_path / Path(
                                p.stem + date_str + p.suffix + ".bak"
                            )
                            with backup_path.open("w") as backup_file:
                                backup_file.write(json_text)
                        self.clear()
                        self.save()
                    raise BtmeshDfuAppDbLoadError(
                        json_path,
                        f"Failed to load {json_path} persistent data. "
                        f"JSON load error reason: {e}",
                    ) from e
        else:
            self.clear()

    def add_app_group(self, app_group):
        if app_group.name in self.app_groups:
            raise ValueError(f'App group name "{app_group.name}" already exists.')
        self.app_groups[app_group.name] = app_group

    def remove_app_group(self, name):
        return self.app_groups.pop(name)

    def has_app_group(self, name):
        return name in self.app_groups

    def rename_app_group(self, current_name, new_name):
        if current_name not in self.app_groups:
            raise ValueError(f'App group with "{current_name}" does not exist.')
        if new_name in self.app_groups:
            raise ValueError(f'App group name "{new_name}" already exists.')
        # It is necessary to remove the app_group on rename because the key
        # in the dictionary is the name of the app_group
        app_group = self.remove_app_group(current_name)
        app_group.name = new_name
        self.add_app_group(app_group)

    def gen_app_groups(self):
        return (ag for ag in self.app_groups.values())

    def gen_app_group_names(self):
        return (ag for ag in self.app_groups.keys())

    def get_app_group_list(self, groupfilter=None, order_property=None, reverse=False):
        identity = lambda n: n
        groupfilter = groupfilter if groupfilter else identity
        app_group_gen = self.gen_app_groups()
        if order_property:
            key = operator.attrgetter(order_property)
            return sorted(app_group_gen, key=key, reverse=reverse)
        else:
            return list(app_group_gen)

    def get_app_group_by_name(self, name) -> BtmeshDfuAppGroup:
        if name not in self.app_groups:
            raise ValueError(f'App group with "{name}" does not exist.')
        return self.app_groups[name]

    def gen_app_group_nodes(self, app_group_name):
        app_group = self.get_app_group_by_name(app_group_name)
        node_dict = dict.fromkeys(
            (self.btmesh_db.get_node_by_elem_ref(elem) for elem in app_group.elems)
        )
        yield from node_dict.keys()

    def get_app_group_node_list(
        self, app_group_name, nodefilter=None, order_property=None, reverse=False
    ):
        identity = lambda n: n
        nodefilter = nodefilter if nodefilter else identity
        node_gen = self.gen_app_group_nodes(app_group_name)
        if order_property:
            key = operator.attrgetter(order_property)
            return sorted(node_gen, key=key, reverse=reverse)
        else:
            return list(node_gen)

    def get_remote_node_by_name(self, name, retry_params: BtmeshRetryParams = None):
        node = self.btmesh_db.get_node_by_name(name)
        return RemoteNode(btmesh=app_btmesh, node=node, retry_params=retry_params)

    def get_remote_node_by_uuid(self, uuid, retry_params: BtmeshRetryParams = None):
        node = self.btmesh_db.get_node_by_uuid(uuid)
        return RemoteNode(btmesh=app_btmesh, node=node, retry_params=retry_params)

    def get_remote_node_by_prim_addr(
        self, prim_addr, retry_params: BtmeshRetryParams = None
    ):
        node = self.btmesh_db.get_node_by_prim_addr(prim_addr)
        return RemoteNode(btmesh=app_btmesh, node=node, retry_params=retry_params)

    def get_remote_node_by_elem_addr(
        self, elem_addr, retry_params: BtmeshRetryParams = None
    ):
        node = self.btmesh_db.get_node_by_elem_addr(elem_addr)
        return RemoteNode(btmesh=app_btmesh, node=node, retry_params=retry_params)

    def get_metadata_by_fwid(
        self, fwid: FWID, default: Optional[bytes] = None
    ) -> Optional[bytes]:
        return self.fwid_metadata_cache.get(fwid.to_bytes(), default)

    def add_fwid_metadata_pair(self, fwid: FWID, metadata: bytes) -> None:
        self.fwid_metadata_cache[fwid.to_bytes()] = metadata

    def clear_fwid_metadata_cache(self) -> None:
        self.fwid_metadata_cache.clear()


class BtmeshDfuAppDatabaseMigration:
    def create_migration_backup(
        cls, db: Dict, date: datetime, db_path: Optional[Path] = None
    ):
        if app_cfg.persistence.backup_on_migration:
            if db_path is None:
                db_path = app_cfg.persistence.path
            temp_folder_path = db_path.parent / "temp"
            temp_folder_path.mkdir(exist_ok=True)
            migration_folder_path = temp_folder_path / "migration"
            migration_folder_path.mkdir(exist_ok=True)
            date_str = f"{date:%Y%m%d_%H%M%S}"
            major_version = db["version"]["major"]
            backup_file_name = (
                f"{db_path.stem}_{date_str}_v{major_version}{db_path.suffix}.bak"
            )
            backup_path = migration_folder_path / backup_file_name
            with backup_path.open("w") as backup_file:
                json_text = json.dumps(db, indent=4)
                backup_file.write(json_text)

    @classmethod
    def migrate(
        cls, db: Dict, target_version: DatabaseVersion, db_path: Optional[Path] = None
    ) -> bool:
        db_changed = False
        date = datetime.now()
        if "version" not in db:
            db["version"] = {"major": 1, "minor": 0}
        orig_version = DatabaseVersion.create_from_dict(db["version"])
        cls.create_migration_backup(cls, db, date, db_path)
        migrations = btmesh.db.collect_migrations(cls)
        for major in range(orig_version.major + 1, target_version.major + 1):
            if major in migrations:
                migrations[major](db)
                db["version"]["major"] = major
                cls.create_migration_backup(cls, db, date, db_path)
                db_changed = True
        return db_changed

    @classmethod
    def db_v1_get_elem_ref_from_addr(cls, db: Dict, elem_addr: str) -> str:
        elem_addr = int(elem_addr, 10)
        for node in db["btmesh_db"]["nodes"]:
            if node["prim_addr"] <= elem_addr < node["prim_addr"] + node["elem_count"]:
                uuid = node["uuid"]
                elem_idx = elem_addr - node["prim_addr"]
                elem_ref = f"{uuid}[{elem_idx}]"
                return elem_ref

    @classmethod
    def migrate_to_v2(cls, db: Dict) -> Dict:
        for _, app_group in db["app_groups"].items():
            pub_elems = {}
            sub_elems = {}
            bind_elems = {}
            for elem_addr, mdls in app_group["pub_addrs"].items():
                elem_ref = cls.db_v1_get_elem_ref_from_addr(db, elem_addr)
                pub_elems[elem_ref] = mdls
            for elem_addr, mdls in app_group["sub_addrs"].items():
                elem_ref = cls.db_v1_get_elem_ref_from_addr(db, elem_addr)
                sub_elems[elem_ref] = mdls
            for elem_addr, mdls in app_group["bind_addrs"].items():
                elem_ref = cls.db_v1_get_elem_ref_from_addr(db, elem_addr)
                bind_elems[elem_ref] = mdls
            app_group["pub_elems"] = pub_elems
            app_group["sub_elems"] = sub_elems
            app_group["bind_elems"] = bind_elems
            del app_group["pub_addrs"]
            del app_group["sub_addrs"]
            del app_group["bind_addrs"]
        if "version" not in db:
            db["version"] = {}
        db["version"]["major"] = 2
        db["version"]["minor"] = 0
        return db


app_db = BtmeshDfuAppDatabase()
