"""
ESL Key Library
"""

# Copyright 2023 Silicon Laboratories Inc. www.silabs.com
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

from ctypes import c_ubyte, c_uint16, byref, memmove, sizeof
import ctypes
from ap_constants import (
    BROADCAST_ADDRESS,
    PA_SUBEVENT_MAX,
    EAD_KEY_MATERIAL_SIZE,
    AES_KEY_SIZE,
    EAD_IV_SIZE,
    ESL_ID_MASK,
    ESL_GROUP_MASK,
)
import esl_lib
import esl_key_lib_wrapper as eklw


class ESLRecord:
    """ESL Key Library database record"""

    def __init__(
        self,
        address: esl_lib.Address,
        ap_address: esl_lib.Address = None,
        ltk: bytes = None,
        apk: bytes = None,
        rk: bytes = None,
        esl_id: int = None,
        group: int = None,
    ):
        if isinstance(address, esl_lib.Address):
            self._address = address
        else:
            self.address = address  # BT address of the ESL
        if isinstance(ap_address, esl_lib.Address):
            self._ap_address = ap_address
        else:
            self.ap_address = ap_address  # BT address of the bonded AP
        self.ltk = ltk  # long term key for bonding
        self.apk = apk  # access point sync AES-128 key
        self.rk = rk  # ESL response AES-128 key
        self.esl_id = esl_id  # ESL ID
        self.group = group  # Group ID

    @property
    def address(self) -> esl_lib.Address:
        """ESL BT address getter"""
        return self._address

    @address.setter
    def address(self, address: bytes):
        """ESL BT address setter with validation"""
        self._address = esl_lib.Address(address)

    @property
    def ap_address(self) -> esl_lib.Address:
        """Bonded AP BT address getter"""
        return self._ap_address

    @ap_address.setter
    def ap_address(self, address: bytes):
        """Bonded AP BT address setter with validation"""
        self._ap_address = esl_lib.Address(address) if address is not None else None

    @property
    def esl_id(self) -> int:
        """ESL ID getter"""
        return self._esl_id

    @esl_id.setter
    def esl_id(self, id: int):
        """ESL ID setter with range validation"""
        if id is None or id in range(0, BROADCAST_ADDRESS):
            self._esl_id = id
        else:
            raise ValueError(f"Cannot set ESL ID {id}, it's out of range!")

    @property
    def group(self) -> int:
        """ESL Group ID getter"""
        return self._group

    @group.setter
    def group(self, group: int):
        """ESL Group ID setter with range validation"""
        if group is None or group in range(0, PA_SUBEVENT_MAX):
            self._group = group
        else:
            raise ValueError(f"Cannot set Group ID {group}, it's out of range!")

    @property
    def esl_address(self) -> int:
        """ESL Address getter"""
        esl_address = None
        if None not in (self.esl_id, self.group):
            esl_address = self.esl_id | (self.group << 8)
        return esl_address

    @property
    def ltk(self) -> bytes:
        """Long Term Key getter"""
        return self._ltk

    @ltk.setter
    def ltk(self, key: bytes):
        """Long Term Key setter with length check"""
        if key is None or len(key) == AES_KEY_SIZE:
            self._ltk = key
        else:
            raise ValueError(
                f"Invalid AES-128 key length of {len(key)} bytes for Long Term Key can't be set!"
            )

    @property
    def apk(self) -> bytes:
        """AP Sync Key getter"""
        return self._apk

    @apk.setter
    def apk(self, key: bytes):
        """AP Sync key setter with length check"""
        if key is None or len(key) == EAD_KEY_MATERIAL_SIZE:
            self._apk = key
        else:
            raise ValueError(
                f"Invalid length of {len(key)} bytes for AP Sync Key Material can't be set!"
            )

    @property
    def rk(self) -> bytes:
        return self._rk

    @rk.setter
    def rk(self, key: bytes):
        """Response Key setter with length check"""
        if key is None or len(key) == EAD_KEY_MATERIAL_SIZE:
            self._rk = key
        else:
            raise ValueError(
                f"Invalid length of {len(key)} bytes for Response Key Material can't be set!"
            )

    def __repr__(self) -> str:
        """Class representation"""
        s3 = "key"
        keys = [self._ltk, self._apk, self._rk]
        if self._esl_id is None or self._group is None:
            s1 = "not yet configured"
        else:
            s1 = f"configured as ID {self._esl_id} in group {self._group}"
        if all(prop is None for prop in keys):
            s2 = "all"
            s3 += "s"
            s4 = "are missing"
        else:
            s4 = "set"
            keys_count = len([x for x in keys if x is not None])
            if keys_count == 1:
                s2 = "only "
                s3 += " is"
            else:
                s2 = ""
                s3 += "s are"
        if self.ap_address is None:
            s5 = "AP identity unset"
        else:
            s5 = f"AP identity is set to {self.ap_address} address"
        return f"ESL at {self.address} address {s1}, {s2}{', '.join(k for ix, k in enumerate(['ltk', 'apk', 'rk']) if keys[ix] is not None)} {s3} {s4}, {s5}"


class Error(Exception):
    def __init__(self, status: int):
        super().__init__(f"ESL key lib status {status:#x}")


class DBRecordError(Exception):
    def __init__(self, status: int, record: ESLRecord):
        super().__init__(f"ESL key lib status {status:#x} on record {record}")


class Lib:
    """ESL Key Library"""

    def __init__(self, file_name="bonding.db"):
        self.key_db_handle = eklw.db_handle_p()
        self.file_name = file_name
        status = eklw.esl_key_lib_init_database(self.file_name, byref(self.key_db_handle))
        if status != eklw.SL_STATUS_OK:
            raise Error(status)

    def _free_key_db_handle(self, handle):
        """Explicitly free the memory allocated for a Key DB handle copy."""
        if handle:
            status = eklw.esl_key_lib_free_threadsafe_handle(handle)
            if status != eklw.SL_STATUS_OK:
                raise Error(status)            
            del handle

    def _copy_key_db_handle(self):
        """Create a threadsafe shallow copy of the self.key_db_handle content."""
        if not self.key_db_handle:
            raise ValueError("The ESL key library is not initialized.")
        
        copy_handle = eklw.db_handle_p()
        status = eklw.esl_key_lib_split_threadsafe_handle(self.key_db_handle, byref(copy_handle))
        if status != eklw.SL_STATUS_OK:
            raise Error(status)
   
        # Return the handle and a cleanup function for ease of use
        return copy_handle, lambda: self._free_key_db_handle(copy_handle)

    def find_ltk(self, address: esl_lib.Address) -> bytes:
        """Search for Long Term Key entry for the given address"""
        key_db_handle, cleanup = self._copy_key_db_handle()
        key_record = eklw.db_record_p()
        ble_address = eklw.bd_addr()
        ble_address.addr = (c_ubyte * 6).from_buffer_copy(address.addr)
        try:
            status = eklw.esl_key_lib_get_record_by_ble_address(
                key_db_handle, byref(ble_address), byref(key_record)
            )
            if status == eklw.SL_STATUS_OK:
                ltk_key_out = eklw.aes_key_128()
                status = eklw.esl_key_lib_get_long_term_key(
                    key_record, byref(ltk_key_out)
                )
            if status == eklw.SL_STATUS_OK:
                return bytes(ltk_key_out.data)
        finally:
            eklw.esl_key_lib_free_record(key_record)
            cleanup()  # Explicitly free the copied handle
        return None

    def find_esl(self, address: esl_lib.Address) -> ESLRecord:
        """Search for ESL database record for the given address"""
        key_db_handle, cleanup = self._copy_key_db_handle()
        db_record = eklw.db_record_p()
        ble_address = eklw.bd_addr()
        ble_address.addr = (c_ubyte * 6).from_buffer_copy(address.addr)
        status = eklw.esl_key_lib_get_record_by_ble_address(
            key_db_handle, byref(ble_address), byref(db_record)
        )
        if status == eklw.SL_STATUS_OK:
            esl = ESLRecord(address)
            try:
                ltk_key_out = eklw.aes_key_128()
                status = eklw.esl_key_lib_get_long_term_key(
                    db_record, byref(ltk_key_out)
                )
                if status == eklw.SL_STATUS_OK:
                    esl.ltk = bytes(ltk_key_out.data)
                response_key_out = eklw.struct_sl_bt_ead_key_material_s()
                status = eklw.esl_key_lib_get_response_key_material(
                    db_record, byref(response_key_out)
                )
                if status == eklw.SL_STATUS_OK:
                    esl.rk = bytes(response_key_out)
                esl_address_out = eklw.esl_address_t()
                status = eklw.esl_key_lib_get_esl_address(
                    db_record, byref(esl_address_out)
                )
                if status == eklw.SL_STATUS_OK:
                    esl.esl_id = esl_address_out.value & ESL_ID_MASK
                    esl.group = (esl_address_out.value & ESL_GROUP_MASK) >> 8
                ap_address = eklw.bd_addr()
                status = eklw.esl_key_lib_get_bind_address(
                    key_db_handle, db_record, byref(ap_address)
                )
                if status == eklw.SL_STATUS_OK:
                    esl.ap_address = bytes(ap_address.addr)
                return esl
            finally:
                eklw.esl_key_lib_free_record(db_record)
        cleanup()
        return None

    def add_esl(self, esl_record: ESLRecord):
        """Add new ESL database entry for the given address"""
        key_db_handle, cleanup = self._copy_key_db_handle()
        ltk_key = eklw.aes_key_128()
        esl_db_record = eklw.db_record_p()
        ble_address = eklw.bd_addr()
        ble_address.addr = (c_ubyte * 6).from_buffer_copy(esl_record.address.addr)
        if esl_record.ltk is None:  # searching for an existing record
            status = eklw.esl_key_lib_get_record_by_ble_address(
                key_db_handle, byref(ble_address), byref(esl_db_record)
            )
            if status not in [eklw.SL_STATUS_OK, eklw.SL_STATUS_NOT_FOUND]:
                cleanup()
                msg = esl_lib.get_sl_status_str(status)
                raise RuntimeError(
                    f"ESL key library error while querying {esl_record.address}: "
                    f" sc={status} ({msg})"
                )
            record_type = eklw.esl_key_lib_record_type_t()
            status = eklw.esl_key_lib_get_record_type(esl_db_record, byref(record_type))
            if (
                status != eklw.SL_STATUS_OK
                or record_type.value != eklw.ESL_KEY_LIB_TAG_RECORD
            ):
                eklw.esl_key_lib_free_record(esl_db_record)
                cleanup()
                raise ValueError(
                    f"The {esl_record.address} address does not match any ESL in the database!"
                )
            # Get LTK just in case - it's a mandatory field anyway
            status = eklw.esl_key_lib_get_long_term_key(esl_db_record, byref(ltk_key))
            if status == eklw.SL_STATUS_OK:
                esl_record.ltk = bytes(ltk_key.data)
        else:
            ltk_key.data = (c_ubyte * AES_KEY_SIZE).from_buffer_copy(esl_record.ltk)
            eklw.esl_key_lib_alloc_record(
                eklw.ESL_KEY_LIB_TAG_RECORD, byref(esl_db_record)
            )
            eklw.esl_key_lib_set_long_term_key(byref(ltk_key), esl_db_record)
            eklw.esl_key_lib_set_ble_address(byref(ble_address), esl_db_record)
        if esl_record.esl_address is not None:
            esl_address = eklw.esl_address_t()
            esl_address = c_uint16(esl_record.esl_address)
            eklw.esl_key_lib_set_esl_address(esl_address, esl_db_record)

        if esl_record.rk is not None:
            response_key = eklw.struct_sl_bt_ead_key_material_s()
            response_key.key = (c_ubyte * AES_KEY_SIZE).from_buffer_copy(
                esl_record.rk[0:AES_KEY_SIZE]
            )
            response_key.iv = (c_ubyte * EAD_IV_SIZE).from_buffer_copy(
                esl_record.rk[AES_KEY_SIZE:EAD_KEY_MATERIAL_SIZE]
            )
            eklw.esl_key_lib_set_response_key_material(
                byref(response_key), esl_db_record
            )

        if esl_record.ap_address is None:
            status = eklw.esl_key_lib_store_record(key_db_handle, esl_db_record)
        else:
            ap_db_record = eklw.db_record_p()
            ap_address = eklw.bd_addr()
            ap_address.addr = (c_ubyte * 6).from_buffer_copy(esl_record.ap_address.addr)

            if esl_record.apk is None:
                status = eklw.esl_key_lib_get_record_by_ble_address(
                    key_db_handle, byref(ap_address), byref(ap_db_record)
                )
                if status != eklw.SL_STATUS_OK:
                    cleanup()
                    raise ValueError(
                        f"The {esl_record.ap_address} address can't be found in the database!"
                    )
                record_type = eklw.esl_key_lib_record_type_t()
                status = eklw.esl_key_lib_get_record_type(
                    byref(esl_db_record), byref(record_type)
                )
                if (
                    status != eklw.SL_STATUS_OK
                    or record_type.value != eklw.ESL_KEY_LIB_AP_RECORD
                ):
                    eklw.esl_key_lib_free_record(ap_db_record)
                    cleanup()
                    raise ValueError(
                        f"The {esl_record.ap_address} address does not match any AP in the database!"
                    )
            else:
                eklw.esl_key_lib_alloc_record(
                    eklw.ESL_KEY_LIB_AP_RECORD, byref(ap_db_record)
                )
                eklw.esl_key_lib_set_ble_address(byref(ap_address), ap_db_record)
                ap_key = eklw.struct_sl_bt_ead_key_material_s()
                ap_key.key = (c_ubyte * AES_KEY_SIZE).from_buffer_copy(
                    esl_record.apk[0:AES_KEY_SIZE]
                )
                ap_key.iv = (c_ubyte * EAD_IV_SIZE).from_buffer_copy(
                    esl_record.apk[AES_KEY_SIZE:EAD_KEY_MATERIAL_SIZE]
                )
                eklw.esl_key_lib_set_ap_key_material(byref(ap_key), ap_db_record)
                status = eklw.esl_key_lib_store_record(key_db_handle, ap_db_record)
                if status == eklw.SL_STATUS_OK:
                    status = eklw.esl_key_lib_store_record_and_bind(
                        key_db_handle, esl_db_record, byref(ap_address)
                    )
            eklw.esl_key_lib_free_record(ap_db_record)
        eklw.esl_key_lib_free_record(esl_db_record)
        cleanup()
        if status != eklw.SL_STATUS_OK:
            raise DBRecordError(status, esl_record)

    def delete_node(self, address: esl_lib.Address):
        """Delete the ESL database entry with the specified address"""
        key_db_handle, cleanup = self._copy_key_db_handle()
        ble_address = eklw.bd_addr()
        ble_address.addr = (c_ubyte * 6).from_buffer_copy(address.addr)
        status = eklw.esl_key_lib_delete_record_by_ble_address(
            key_db_handle, byref(ble_address)
        )
        cleanup()
        if status != eklw.SL_STATUS_OK:
            raise Error(status)

    def delete_ltk(self, address: esl_lib.Address, ap_address: esl_lib.Address = None):
        """Delete the LTK key of an ESL database entry with the specified address"""
        key_db_handle, cleanup = self._copy_key_db_handle()
        esl_record = eklw.db_record_p()
        ble_address = eklw.bd_addr()
        ble_address.addr = (c_ubyte * 6).from_buffer_copy(address.addr)
        try:
            status = eklw.esl_key_lib_get_record_by_ble_address(
                key_db_handle, byref(ble_address), byref(esl_record)
            )
            if status == eklw.SL_STATUS_OK:
                ap_bd_addr = eklw.bd_addr()
                status = eklw.esl_key_lib_get_bind_address(
                    key_db_handle, esl_record, byref(ap_bd_addr)
                )
                if status == eklw.SL_STATUS_OK and ap_address is not None and esl_lib.Address(bytes(ap_bd_addr.addr)) != ap_address:
                    # Do not delete LTK keys that belongs to another AP!
                    return

                ltk_key = eklw.aes_key_128()
                ltk_key.data = (c_ubyte * AES_KEY_SIZE).from_buffer_copy(bytes(AES_KEY_SIZE))
                status = eklw.esl_key_lib_set_long_term_key(
                    byref(ltk_key), esl_record
                )
                if status == eklw.SL_STATUS_OK:
                    status = eklw.esl_key_lib_store_record(key_db_handle, esl_record)
                if status != eklw.SL_STATUS_OK:
                    raise Error(status)
        finally:
            eklw.esl_key_lib_free_record(esl_record)
            cleanup()