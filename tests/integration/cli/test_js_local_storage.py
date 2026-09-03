"""JavaScript localStorage checks for JS Apps."""

import json
from textwrap import dedent

import allure
import pytest


pytestmark = [
    pytest.mark.cli,
    pytest.mark.usefixtures("js_local_storage_clean"),
]


@allure.epic("BSB CLI Testing")
@allure.feature("JavaScript localStorage API")
@allure.story("JS Apps")
class TestJSLocalStorage:
    """Exercise localStorage through independent JS CLI processes."""

    @allure.title("JavaScript localStorage exposes an empty storage contract.")
    def test_empty_storage_contract(self, js_case_runner):
        body = dedent(
            """
                assert(typeof localStorage === "object",
                    "localStorage type=" + typeof localStorage);
                assert(typeof localStorage.length === "number",
                    "length type=" + typeof localStorage.length);
                assert(typeof localStorage.key === "function", "key missing");
                assert(typeof localStorage.getItem === "function",
                    "getItem missing");
                assert(typeof localStorage.setItem === "function",
                    "setItem missing");
                assert(typeof localStorage.removeItem === "function",
                    "removeItem missing");
assert(
    typeof localStorage.clear === "function",
    "clear missing"
);

                const clearResult = localStorage.clear();
                const removeResult = localStorage.removeItem("missing");

                assert(clearResult === undefined,
                    "clear result=" + clearResult);
                assert(removeResult === undefined,
                    "removeItem result=" + removeResult);
                assert(localStorage.length === 0,
                    "length=" + localStorage.length);
                assert(localStorage.getItem("missing") === null,
                    "missing key did not return null");
                assert(localStorage.key(0) === null,
                    "key(0) did not return null");
            """
        ).strip()

        js_case_runner("local_storage_empty", body)

    @allure.title("JavaScript localStorage supports CRUD and key overwrite.")
    def test_crud_and_overwrite(self, js_case_runner):
        body = dedent(
            """
                localStorage.clear();
                const firstResult = localStorage.setItem("status", "busy");
                localStorage.setItem("mode", "focus");

                assert(firstResult === undefined,
                    "setItem result=" + firstResult);
                assert(localStorage.length === 2,
                    "length after insert=" + localStorage.length);
                assert(localStorage.getItem("status") === "busy",
                    "initial status=" + localStorage.getItem("status"));

                localStorage.setItem("status", "available");
                assert(localStorage.length === 2,
                    "overwrite changed length=" + localStorage.length);
                assert(localStorage.getItem("status") === "available",
                    "overwritten status=" + localStorage.getItem("status"));

                localStorage.removeItem("mode");
                assert(localStorage.length === 1,
                    "length after remove=" + localStorage.length);
                assert(localStorage.getItem("mode") === null,
                    "removed key is still present");

                localStorage.removeItem("mode");
                assert(localStorage.length === 1,
                    "repeated remove changed length=" + localStorage.length);
            """
        ).strip()

        js_case_runner("local_storage_crud", body)

    @allure.title(
        "JavaScript localStorage enumerates keys without fixed order."
    )
    def test_key_enumeration(self, js_case_runner):
        body = dedent(
            """
                localStorage.clear();
                localStorage.setItem("alpha", "1");
                localStorage.setItem("beta", "2");
                localStorage.setItem("gamma", "3");

                const keys = [];
                for(let index = 0; index < localStorage.length; index++) {
                    keys.push(localStorage.key(index));
                }

                assert(keys.length === 3, "keys length=" + keys.length);
                assert(keys.indexOf("alpha") !== -1, "alpha is missing");
                assert(keys.indexOf("beta") !== -1, "beta is missing");
                assert(keys.indexOf("gamma") !== -1, "gamma is missing");
                assert(localStorage.key(localStorage.length) === null,
                    "out-of-range key did not return null");
                assert(localStorage.key(-1) === null,
                    "negative key index did not return null");
            """
        ).strip()

        js_case_runner("local_storage_keys", body)

    @allure.title(
        "JavaScript localStorage persists and clears across JS runs."
    )
    def test_persistence_between_runs(self, js_case_runner):
        write_body = dedent(
            """
                localStorage.clear();
                localStorage.setItem("session", "persisted");
                localStorage.setItem("counter", "42");
            """
        ).strip()
        js_case_runner("local_storage_persist_write", write_body)

        read_body = dedent(
            """
                assert(localStorage.length === 2,
                    "persisted length=" + localStorage.length);
                assert(localStorage.getItem("session") === "persisted",
                    "persisted session is missing");
                assert(localStorage.getItem("counter") === "42",
                    "persisted counter is missing");
            """
        ).strip()
        js_case_runner("local_storage_persist_read", read_body)

        js_case_runner("local_storage_persist_clear", "localStorage.clear();")

        verify_clear_body = dedent(
            """
                assert(localStorage.length === 0,
                    "length after persisted clear=" + localStorage.length);
                assert(localStorage.getItem("session") === null,
                    "session survived clear");
                assert(localStorage.getItem("counter") === null,
                    "counter survived clear");
            """
        ).strip()
        js_case_runner("local_storage_persist_verify_clear", verify_clear_body)

    @allure.title("JavaScript localStorage preserves strings and Unicode.")
    def test_string_roundtrip(self, js_case_runner):
        unicode_key = "ключ"
        unicode_value = 'Привет, BUSY Bar! "quotes" \\ slash\nnext line'
        unicode_body = dedent(
            f"""
                localStorage.clear();
                localStorage.setItem("", "");
                localStorage.setItem(
                    {json.dumps(unicode_key)},
                    {json.dumps(unicode_value)}
                );

                assert(localStorage.getItem("") === "",
                    "empty key/value roundtrip failed");
                assert(localStorage.getItem({json.dumps(unicode_key)}) ===
                    {json.dumps(unicode_value)}, "Unicode value mismatch");
                assert(localStorage.length === 2,
                    "Unicode storage length=" + localStorage.length);
            """
        ).strip()
        js_case_runner("local_storage_strings_unicode", unicode_body)

        long_value = "данные-" * 128
        long_body = dedent(
            f"""
                localStorage.clear();
                const expectedLongValue = {json.dumps(long_value)};
                localStorage.setItem("long", expectedLongValue);

                assert(localStorage.getItem("long") === expectedLongValue,
                    "long value mismatch");
                assert(localStorage.length === 1,
                    "long string storage length=" + localStorage.length);
            """
        ).strip()
        js_case_runner("local_storage_strings_long", long_body)

    @allure.title("JavaScript localStorage rejects invalid argument types.")
    def test_invalid_arguments(self, js_case_runner):
        body = dedent(
            """
                localStorage.clear();

                function assertTypeError(callback, operation) {
                    let errorCaught = false;
                    try {
                        callback();
                    } catch(error) {
                        errorCaught = true;
                        assert(error instanceof TypeError,
                            operation + " error=" + error);
                    }
                    assert(errorCaught, operation + " did not throw");
                }

                assertTypeError(function() { localStorage.key(); }, "key missing index");
                assertTypeError(function() { localStorage.setItem("key"); },
                    "setItem missing value");
                assert(localStorage.length === 0,
                    "invalid calls mutated storage");
            """
        ).strip()

        js_case_runner("local_storage_invalid_arguments", body)

    @allure.title("JavaScript localStorage writes its versioned backing file.")
    def test_backing_file_format(
        self,
        js_case_runner,
        storage_api,
        js_local_storage_clean,
    ):
        body = dedent(
            """
                localStorage.clear();
                localStorage.setItem("status", "busy");
                localStorage.setItem("counter", "42");
            """
        ).strip()
        js_case_runner("local_storage_backing_file", body)

        with allure.step("Read the localStorage backing file"):
            response = storage_api.read(js_local_storage_clean)
            assert response.status_code == 200, (
                f"expected backing file HTTP 200, got {response.status_code}: "
                f"{response.text[:200]!r}"
            )
            payload = response.json()

        with allure.step("Verify the versioned JSON representation"):
            assert payload == {
                "format_version": 1,
                "data": {"status": "busy", "counter": "42"},
            }, f"unexpected backing payload: {payload!r}"

    @allure.title("JavaScript localStorage recovers an invalid backing file.")
    @pytest.mark.parametrize(
        ("case_name", "invalid_payload"),
        [
            ("malformed", b"{not-json"),
            (
                "wrong_version",
                json.dumps(
                    {"format_version": 2, "data": {"stale": "value"}}
                ).encode(),
            ),
            ("missing_data", json.dumps({"format_version": 1}).encode()),
            (
                "wrong_data_type",
                json.dumps({"format_version": 1, "data": []}).encode(),
            ),
        ],
    )
    def test_invalid_backing_file_recovers(
        self,
        case_name,
        invalid_payload,
        js_case_runner,
        storage_api,
        js_local_storage_clean,
    ):
        js_case_runner(
            f"local_storage_{case_name}_initialize",
            "localStorage.clear();",
        )

        with allure.step(f"Write {case_name} localStorage backing data"):
            response = storage_api.write(
                js_local_storage_clean,
                invalid_payload,
            )
            assert response.status_code == 200, (
                f"failed to seed {case_name} backing data: "
                f"HTTP {response.status_code}, {response.text[:200]!r}"
            )

        verify_body = dedent(
            """
                assert(localStorage.length === 0,
                    "recovered length=" + localStorage.length);
                assert(localStorage.getItem("stale") === null,
                    "stale value survived recovery");
            """
        ).strip()
        js_case_runner(f"local_storage_{case_name}_recover", verify_body)

        with allure.step(
            "Verify the backing file was replaced with an empty store"
        ):
            response = storage_api.read(js_local_storage_clean)
            assert response.status_code == 200, (
                f"expected recovered file HTTP 200, got {response.status_code}"
            )
            payload = response.json()
            assert payload == {"format_version": 1, "data": {}}, (
                f"unexpected recovered payload: {payload!r}"
            )

    @allure.title("JavaScript localStorage survives repeated mutations.")
    def test_repeated_mutations(self, js_case_runner):
        mutate_body = dedent(
            """
                localStorage.clear();
                for(let index = 0; index < 12; index++) {
                    localStorage.setItem("key-" + index, "value-" + index);
                }
                for(let index = 0; index < 12; index += 2) {
                    localStorage.setItem("key-" + index, "updated-" + index);
                }
                for(let index = 0; index < 12; index += 3) {
                    localStorage.removeItem("key-" + index);
                }
                assert(localStorage.length === 8,
                    "mutated length=" + localStorage.length);
            """
        ).strip()
        js_case_runner("local_storage_repeated_mutate", mutate_body)

        verify_body = dedent(
            """
                assert(localStorage.length === 8,
                    "persisted mutation length=" + localStorage.length);
                for(let index = 0; index < 12; index++) {
                    const actual = localStorage.getItem("key-" + index);
                    if(index % 3 === 0) {
                        assert(actual === null,
                            "removed key survived: key-" + index);
                    } else {
                        const expected = index % 2 === 0 ?
                            "updated-" + index : "value-" + index;
                        assert(actual === expected,
                            "key-" + index + "=" + actual);
                    }
                }
            """
        ).strip()
        js_case_runner("local_storage_repeated_verify", verify_body)

    @allure.title("JavaScript localStorage is isolated by application ID.")
    def test_app_id_isolation(
        self,
        js_case_runner,
        js_local_storage_clean,
        storage_api,
    ):
        other_path = (
            "/ext/apps_data/jsrunner/"
            "other.localstorage.json"
        )
        other_payload = {
            "format_version": 1,
            "data": {
                "owner": "other",
                "only_other": "present",
            },
        }

        storage_api.remove_raw(other_path)
        try:
            with allure.step("Create a second application storage namespace"):
                response = storage_api.write(
                    other_path,
                    json.dumps(other_payload).encode("utf-8"),
                )
                assert response.status_code == 200, (
                    f"failed to seed {other_path}: HTTP "
                    f"{response.status_code}, {response.text[:200]!r}"
                )

            js_case_runner(
                "local_storage_app_id_write",
                dedent(
                    """
                        localStorage.clear();
                        localStorage.setItem("owner", "cli");
                        localStorage.setItem("only_cli", "present");
                    """
                ).strip(),
            )
            js_case_runner(
                "local_storage_app_id_verify",
                dedent(
                    """
                        assert(localStorage.getItem("owner") === "cli",
                            "wrong namespace owner");
                        assert(localStorage.getItem("only_cli") === "present",
                            "CLI value missing");
                        assert(localStorage.getItem("only_other") === null,
                            "other app value leaked into CLI namespace");
                        localStorage.setItem("verified", "true");
                    """
                ).strip(),
            )

            with allure.step("Verify both backing stores remain independent"):
                cli_response = storage_api.read(js_local_storage_clean)
                assert cli_response.status_code == 200, (
                    f"failed to read CLI store: HTTP "
                    f"{cli_response.status_code}, {cli_response.text[:200]!r}"
                )
                cli_payload = cli_response.json()
                assert cli_payload == {
                    "format_version": 1,
                    "data": {
                        "owner": "cli",
                        "only_cli": "present",
                        "verified": "true",
                    },
                }, f"unexpected CLI namespace: {cli_payload!r}"

                other_response = storage_api.read(other_path)
                assert other_response.status_code == 200, (
                    f"failed to read other store: HTTP "
                    f"{other_response.status_code}, "
                    f"{other_response.text[:200]!r}"
                )
                actual_other = other_response.json()
                assert actual_other == other_payload, (
                    f"other namespace was modified: {actual_other!r}"
                )
        finally:
            storage_api.remove_raw(other_path)

    @allure.title(
        "JavaScript localStorage supports maximum-length application IDs."
    )
    @pytest.mark.skip(reason="crash in web server")
    def test_max_length_app_id_backing_file(self, storage_api):
        app_id = "app.busy.localstorage.max.id.000"
        assert len(app_id) == 32, f"invalid boundary app ID: {app_id!r}"

        backing_file = (
            f"/ext/apps_data/jsrunner/{app_id}.localstorage.json"
        )
        expected = {
            "format_version": 1,
            "data": {"boundary": "max-app-id"},
        }

        storage_api.remove_raw(backing_file)
        try:
            with allure.step("Write the maximum-length app ID backing file"):
                response = storage_api.write(
                    backing_file,
                    json.dumps(expected).encode("utf-8"),
                )
                assert response.status_code == 200, (
                    f"failed to write {backing_file}: HTTP "
                    f"{response.status_code}, {response.text[:200]!r}"
                )

            with allure.step("Read the maximum-length app ID backing file"):
                response = storage_api.read(backing_file)
                assert response.status_code == 200, (
                    f"failed to read {backing_file}: HTTP "
                    f"{response.status_code}, {response.text[:200]!r}"
                )
                actual = response.json()
                assert actual == expected, (
                    f"unexpected maximum-length app store: {actual!r}"
                )
        finally:
            storage_api.remove_raw(backing_file)

    @allure.title(
        "JavaScript localStorage stores repeated non-BMP characters."
    )
    def test_repeated_non_bmp_roundtrip(self, js_case_runner):
        value = "🚀" * 16
        body = dedent(
            f"""
                localStorage.clear();
                const expected = {json.dumps(value)};
                localStorage.setItem("emoji", expected);
                assert(localStorage.getItem("emoji") === expected,
                    "non-BMP value mismatch");
            """
        ).strip()

        js_case_runner("local_storage_non_bmp", body)
