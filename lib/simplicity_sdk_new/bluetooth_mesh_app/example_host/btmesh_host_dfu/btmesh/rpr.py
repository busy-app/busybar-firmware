# Copyright 2024 Silicon Laboratories Inc. www.silabs.com
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
from typing import Callable, Optional, NamedTuple

from bgapi.bglib import CommandFailedError
from bgapix.slstatus import SlStatus

from . import util
from .conf import Configurator
from .core import BtmeshComponent, BtmeshCore
from .db import DCD, Node
from .errors import BtmeshError, BtmeshErrorCode
from .util import BtmeshIntEnum, BtmeshRetryParams

logger = logging.getLogger(__name__)


class NPPIOperation(BtmeshIntEnum):
    DEVICE_KEY_UPDATE = 0x0  # Update Device Key
    ADDRESS_UPDATE = 0x1  # Update device address. Note that also DCD will be updated if updated DCD is available
    DCD_UPDATE = 0x2  # Update device DCD
    REMOTE_PROVISIONING = 0xFF  # Provision a remote device.
    UNKNOWN_VALUE = util.ENUM_UNKNOWN_VALUE


class RPRClientLinkState(BtmeshIntEnum):
    IDLE = 0x0
    OPENING = 0x1
    ACTIVE = 0x2
    OUTBOUND_PACKET_TRANSFER = 0x3
    CLOSING = 0x4
    UNKNOWN_VALUE = util.ENUM_UNKNOWN_VALUE


class RPRServerStatus(BtmeshIntEnum):
    SUCCESS = 0x0
    SCAN_CANNOT_START = 0x1
    INVALID_STATE = 0x2
    LIMITED_RESOURCES = 0x3
    LINK_CANNOT_OPEN = 0x4
    LINK_OPEN_FAILED = 0x5
    LINK_CLOSED_BY_DEVICE = 0x6
    LINK_CLOSED_BY_SERVER = 0x7
    LINK_CLOSED_BY_CLIENT = 0x8
    LINK_CLOSED_AS_CANNOT_RECEIVE_PDU = 0x9
    LINK_CLOSED_AS_CANNOT_SEND_PDU = 0xA
    LINK_CLOSED_AS_CANNOT_DELIVER_PDU_REPORT = 0xB
    UNKNOWN_VALUE = util.ENUM_UNKNOWN_VALUE


class DCDPair(NamedTuple):
    page_0: DCD
    page_128: Optional[DCD]


class RemoteProvisioningClient(BtmeshComponent):
    NETKEY_IDX = 0

    def __init__(
        self,
        core: BtmeshCore,
        conf: Configurator,
    ):
        super().__init__(core)
        self.conf = conf

    def init(self) -> None:
        # Initialize remote provisioning server
        try:
            self.lib.btmesh.remote_provisioning_client.init()
        except CommandFailedError as e:
            if e.errorcode != SlStatus.INVALID_STATE:
                raise

    def address_refresh(
        self,
        server_addr: int,
        elem_idx: int,
        timeout_sec: int,
        uuid: bytes,
        new_prim_addr: int,
        sync_dcd: bool = True,
        retry_params: Optional[BtmeshRetryParams] = None,
    ) -> None:
        dcd_p0, dcd_p128 = self._get_dcd_pages(
            server_addr, sync=sync_dcd, retry_params=retry_params
        )
        self._open_link(
            server_addr, elem_idx, timeout_sec, NPPIOperation.ADDRESS_UPDATE, b""
        )
        self._handle_update(
            uuid=uuid,
            server_addr=server_addr,
            new_prim_addr=new_prim_addr,
            max_time=timeout_sec,
            new_dcd=dcd_p128,
            orig_dcd=dcd_p0,
        )

    def comp_refresh(
        self,
        server_addr: int,
        elem_idx: int,
        timeout_sec: int,
        uuid: bytes,
        sync_dcd: bool = True,
        retry_params: Optional[BtmeshRetryParams] = None,
    ) -> None:
        dcd_p0, dcd_p128 = self._get_dcd_pages(
            server_addr, sync=sync_dcd, retry_params=retry_params
        )
        self._open_link(
            server_addr, elem_idx, timeout_sec, NPPIOperation.DCD_UPDATE, b""
        )
        self._handle_update(
            uuid=uuid,
            server_addr=server_addr,
            new_prim_addr=0,
            max_time=timeout_sec,
            new_dcd=dcd_p128,
            orig_dcd=dcd_p0,
        )

    def get_dcd_pages(
        self,
        node: Node,
        sync: bool = True,
        retry_params: Optional[BtmeshRetryParams] = None,
    ) -> DCDPair:
        dcd_p0 = self.conf.get_dcd_cached(
            node,
            page=util.DCD_PAGE_0,
            update_db=True,
            retry_params=retry_params,
        )
        if sync:
            dcd_p128 = self.conf.get_dcd(
                node,
                page=util.DCD_PAGE_128,
                update_db=True,
                retry_params=retry_params,
            )
        else:
            dcd_p128 = self.conf.get_dcd_cached(
                node,
                page=util.DCD_PAGE_128,
                update_db=True,
                retry_params=retry_params,
            )
        return DCDPair(dcd_p0, dcd_p128)

    def _get_dcd_pages(
        self,
        server_addr: int,
        sync: bool = True,
        retry_params: Optional[BtmeshRetryParams] = None,
    ) -> DCDPair:
        node = self.db.get_node_by_elem_addr(server_addr)
        return self.get_dcd_pages(
            node=node,
            sync=sync,
            retry_params=retry_params,
        )

    def _open_link(
        self, server_addr, elem_idx, timeout_sec, nppi: NPPIOperation, uuid: bytes
    ):
        logger.debug(
            f"Opening remote provisioning link for {uuid.hex()} "
            f"(server address {hex(server_addr)}, timeout {timeout_sec}, "
            f"NPPI: {nppi.to_pretty_name()})"
        )
        self.lib.btmesh.remote_provisioning_client.open_link(
            self.NETKEY_IDX, server_addr, elem_idx, timeout_sec, nppi, uuid
        )
        event = self.lib.wait_event(
            "btmesh_evt_remote_provisioning_client_link_status",
            max_time=timeout_sec,
        )
        if event.result == SlStatus.TIMEOUT:
            raise BtmeshError(
                BtmeshErrorCode.RPR_LINK_OPEN_FAILED,
                f"Remote provisioning link open for {hex(server_addr)} failed with timeout.",
            )
        elif event.result != SlStatus.OK or event.status != RPRServerStatus.SUCCESS:
            state = RPRClientLinkState.from_int(event.state)
            status = RPRServerStatus.from_int(event.status)
            raise BtmeshError(
                BtmeshErrorCode.RPR_LINK_OPEN_FAILED,
                f"Remote provisioning link open for {hex(server_addr)} failed "
                f"with status {hex(event.status)}: {status.pretty_name}. "
                f"Link state: {state.pretty_name}.",
                event=event,
            )
        if event.state != RPRClientLinkState.ACTIVE:
            raise BtmeshError(
                BtmeshErrorCode.RPR_LINK_OPEN_FAILED,
                f"Remote provisioning link open for {hex(event.server)} failed: "
                f"Link state is not ACTIVE ({event.state})",
                event=event,
            )

    def _handle_update(
        self,
        uuid: bytes,
        server_addr: int,
        new_prim_addr: int,
        max_time: int = 20,
        new_dcd: Optional[DCD] = None,
        orig_dcd: Optional[DCD] = None,
    ):
        logger.info(
            f"Starting remote provisioning for device {uuid.hex()} "
            f"(server address {hex(server_addr)}, timeout {max_time})"
        )

        self.lib.btmesh.prov.create_provisioning_session(
            self.NETKEY_IDX, uuid, max_time
        )
        self.lib.btmesh.prov.provision_remote_device(uuid, server_addr)
        event = self.lib.wait_event(
            [
                "btmesh_evt_prov_provisioning_failed",
                "btmesh_evt_prov_provisioning_suspended",
            ],
            max_time=max_time,
        )

        if event == "btmesh_evt_prov_provisioning_failed":
            raise BtmeshError(
                BtmeshErrorCode.PROVISIONING_FAILED,
                f"Remote provisioning {uuid.hex()} device failed "
                f"with {util.prov_failure_reason_str(event.reason)}.",
                event=event,
            )
        elif event == "btmesh_evt_prov_provisioning_suspended":
            if new_prim_addr != 0:
                # Address update - this step is the only difference from DCD update
                logger.debug(
                    f"Sending address update: {hex(server_addr)} -> {hex(new_prim_addr)})"
                )
                self.lib.btmesh.prov.set_device_address(uuid, new_prim_addr)

            # Common steps for both DCD and address update
            self.lib.btmesh.prov.continue_provisioning(uuid)
            event = self.lib.wait_event(
                [
                    "btmesh_evt_prov_device_provisioned",
                    "btmesh_evt_prov_provisioning_failed",
                ],
                max_time=max_time,
            )
            if event == "btmesh_evt_prov_provisioning_failed":
                raise BtmeshError(
                    BtmeshErrorCode.PROVISIONING_FAILED,
                    f"Remote provisioning {(uuid.hex())} device failed "
                    f"with {util.prov_failure_reason_str(event.reason)}.",
                    event=event,
                )
            elif event == "btmesh_evt_prov_device_provisioned":
                _uuid: bytes = event.uuid
                logger.info(
                    "Address refresh completed successfully. "
                    f"UUID: {_uuid.hex()}, Primary address: {hex(event.address)}."
                )
                rsp = self.lib.btmesh.prov.get_ddb_entry(event.uuid)
                logger.debug(
                    f"get_ddb_entry: uuid: {_uuid.hex()}, netkey_idx: "
                    f"{hex(rsp.netkey_index)}, address: {hex(rsp.address)}, "
                    f"elements: {rsp.elements}"
                )
                node = self.db.get_node_by_uuid(event.uuid)
                node.devkey = rsp.device_key
                self.db.start_new_term(
                    node=node,
                    new_prim_addr=rsp.address,
                    new_elem_count=rsp.elements,
                    new_dcd=new_dcd,
                    orig_dcd=orig_dcd,
                )
