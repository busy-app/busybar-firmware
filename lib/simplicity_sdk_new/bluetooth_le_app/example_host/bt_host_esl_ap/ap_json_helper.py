"""
ESL AP Network configuration file helper class.
"""

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
import json
from jsonschema import validate, exceptions
from itertools import chain
from esl_lib import Address
from ap_constants import PA_SUBEVENT_MAX, BROADCAST_ADDRESS
import ap_logger
import esl_lib
import esl_tag
import esl_tag_db
import esl_key_lib


class JSONHelper:
    def __new__(cls):
        if not hasattr(cls, "instance"):
            cls.instance = super(JSONHelper, cls).__new__(cls)
        return cls.instance

    def __init__(self) -> None:
        self._esl_group_schema = {
            "$schema": "https://json-schema.org/draft/2020-12/schema",
            "id": "ESL_group_schema",
            "title": "ESL grouping JSON schema",
            "description": "The schema allows for the pre-configuration of an ESL network by grouping the BT addresses of ESLs and defining the ESL ID within each group.",
            "type": "object",
            "maxProperties": 128,
            "patternProperties": {
                r"^(group_([0-9]|[1-9][0-9]|1[01][0-9]|12[0-7]))$": {
                    "type": "object",
                    "maxProperties": 255,
                    "uniqueItems": True,
                    "patternProperties": {
                        r"^(id_(0|[1]?\d\d?|2[0-4]\d|25[0-4]))$": {
                            "type": "string",
                            "pattern": r"^([a-fA-F0-9]{2}:){5}[a-fA-F0-9]{2}$",
                            "uniqueItems": True,
                        }
                    },
                    "additionalProperties": False,
                }
            },
            "additionalProperties": False,
        }

    @property
    def _log(self):
        return ap_logger.getLogger("JSN")

    def _check_config_duplicates(self, data: dict) -> dict:
        """Search for duplicates in the configuration, return empty dictionary on failure"""
        allitems = {}
        index_key = 0
        for key in data:
            idata = data[key]
            for k2 in idata:
                allitems.setdefault("group_" + str(index_key) + "_" + k2, idata[k2])
            index_key += 1
            if d := self._has_duplicates(data[key]):
                self._log.error(f"Duplicated BT address found for: {d} in {key}!")
                data = {}
        if d := self._has_duplicates(allitems):
            self._log.error(f"Following duplicates found overall: {d}")
            data = {}
        return data

    def _has_duplicates(self, d: dict) -> set:
        """Checks for duplicated values in a dictionary passed as an argument"""
        if not isinstance(d, dict):
            raise ValueError(f"{d} is not a dictionary ({type(d)})!")
        rev_dict = {}
        for key, value in d.items():
            rev_dict.setdefault(value, set()).add(key)

        result = set(
            chain.from_iterable(
                values for key, values in rev_dict.items() if len(values) > 1
            )
        )
        return result

    def export_network_config(
        self,
        tags: list[esl_tag.Tag,],
        file: str,
    ) -> dict:
        """Convert a list of configured ESLs into network config and save it into file on success"""
        config = {}
        for tag in sorted(tags):
            if tag.esl_address is not None:
                config = self.extend_network_config(
                    config, tag.group_id, tag.esl_id, tag.ble_address
                )
        if len(config) != 0:
            self.save_network_config(file, config)
        return config

    def import_network_config(
        self,
        lib: esl_lib,
        key_db: esl_key_lib.Lib,
        tag_db: esl_tag_db.TagDB,
        file: str,
    ) -> dict:
        """Import a list of ESLs with pre-defined configuration into the ESL database"""
        config = self.load_network_config(file)
        for group_key in config:
            esl_data = config[group_key]
            group = int(group_key.partition('group_')[2])
            for esl_key in esl_data:
                id = int(esl_key.partition('id_')[2])
                byte_data = bytearray.fromhex(esl_data[esl_key].translate({ord(':'): None}))
                byte_data.reverse()
                address = Address(bytes(byte_data))
                esl_record = esl_key_lib.ESLRecord(address, esl_id=id, group=group)
                tag = tag_db.add(lib, address)
                tag.esl_address = esl_record.esl_address
                try:
                    key_db.add_esl(esl_record)
                except Exception as e:
                    self._log.debug(e)

        return config

    def extend_network_config(
        self, config: dict, group: int, esl_id: int, address: Address
    ) -> dict:
        """Try to extend the configuration according to the given parameters, but return with the original data if it fails"""
        try:
            if not isinstance(group, int) or group not in range(0, PA_SUBEVENT_MAX):
                raise ValueError(
                    f"Group must be an integer in range (0,{PA_SUBEVENT_MAX-1})!"
                )
            if not isinstance(esl_id, int) or esl_id not in range(0, BROADCAST_ADDRESS):
                raise ValueError(
                    f"ESL ID must be an integer in range (0,{BROADCAST_ADDRESS-1})!"
                )
            if not isinstance(address, Address):
                raise ValueError("BT Address shall be in esl_lib::Address format!")

            extended_config = json.loads(json.dumps(config))  # input validation!
            # Allows epmty dictionary to be extended
            if (
                extended_config is None
                or len(extended_config) == 0
                and extended_config != config
            ):
                raise ValueError("Input config seems already invalid!")
            update_key = str(f"group_{group}")
            update_value = {f"id_{esl_id}": repr(address)}
            extended_config[update_key] |= update_value
        except KeyError:
            extended_config.setdefault(update_key, update_value)
        except ValueError as e:
            self._log.error(e)
            return config
        if len(self._check_config_duplicates(extended_config)) != 0:
            return extended_config
        else:
            self._log.error(f"Cannot add a duplicate address: {repr(address)}!")
            return config

    def load_network_config(self, file: str):
        """Import an ESL network configuration from a properly formatted JSON file"""
        data = {}
        self._log.debug("Try importing ESL network configuration from file %s.", file)
        try:
            with open(file, "r") as f:
                try:
                    data = dict(json.load(f))
                except json.decoder.JSONDecodeError as e:
                    self._log.error(e)
            try:
                if len(data) == 0:
                    raise exceptions.ValidationError(
                        f"Imported JSON file {file} is empty - the source file may be corrupted."
                        " Try looking at previously reported errors if there are any!"
                    )
                validate(instance=data, schema=self._esl_group_schema)
            except exceptions.ValidationError as ex:
                self._log.error(
                    "Configuration import failed with the following exception: %s", ex
                )
            else:
                self._log.debug("JSON schema test PASSED on input file %s!", file)
        except Exception as e:
            self._log.error(f"Could not open the file {file} due to exception {e}")

        return self._check_config_duplicates(data)

    def save_network_config(self, file: str, data: dict):
        """Export an ESL network configuration from a properly formatted JSON file."""
        try:
            if len(data) == 0:
                raise exceptions.ValidationError(
                    f"JSON file {file} would be empty, abort saving!"
                )
            validate(instance=data, schema=self._esl_group_schema)
        except exceptions.ValidationError as ex:
            self._log.error(ex)
            return
        else:
            self._log.debug(
                "JSON schema test PASSED on output data, saving to file %s.", file
            )
        with open(file, "w") as f:
            try:
                for key in data:
                    data[key] = dict(sorted(data[key].items()))
                dict(sorted(data.items()))
            except:
                self._log.error(
                    "Can't save network config: invalid output data passed as argument!"
                )
                return
            json.dump(data, f, sort_keys=False, indent=2)
            self._log.debug("ESL network config file %s saved successfully!", file)


def main():
    """Simple unit test for JSONHelper class"""
    data = {
        "group_0": {"id_0": "aa:bb:cc:dd:ee:ff", "id_3": "aa:bb:cc:dd:fe:dc"},
        "group_3": {"id_1": "aa:bb:cc:dd:ff:ee"},
    }
    data2 = {}
    data3 = {}
    wrongfile = "./data.json"
    file = "./data_test.json"
    file2 = "./data_test2.json"

    ap_logger.setLogLevel(ap_logger.LEVELS["DEBUG"])
    jason = JSONHelper()
    # Save a (hopefully) proper file
    jason.save_network_config(file, data)
    # Try to load a file with either incorrectly formatted data or duplicates - either way, we expect it to fail gently.
    data2 = jason.load_network_config(wrongfile)
    # Load the previously saved data
    data2 = jason.load_network_config(file)

    # Try extend the previosly loaded config
    esl_id = 254
    group = 3
    address = Address(b"ef1436")

    # Expect success:
    data2 = jason.extend_network_config(data2, group, esl_id, address)
    # Expected error due to an attempt to add a duplicate address to another group:
    data2 = jason.extend_network_config(data2, group + 1, esl_id, address)
    # Expect success:
    data2 = jason.extend_network_config(data2, group, esl_id - 1, Address(b"ef1437"))
    # Expect success:
    jason.save_network_config(file2, data2)
    address = Address(b"ff1436")
    # Expect success:
    data2 = jason.extend_network_config(data2, 25, 243, address)
    # Expected error due to an attempt to add a duplicate address to another group:
    data2 = jason.extend_network_config(data2, 42, 34, address)
    jason.save_network_config(file2, data2)
    # Check extend on an empty dictionary, it shall work
    data3 = jason.extend_network_config(data3, group, esl_id, address)


if __name__ == "__main__":
    main()
