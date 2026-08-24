"""Helpers for uploading and running machine-readable JavaScript test cases."""

import json
from textwrap import indent

import allure


RESULT_PREFIX = "JS_TEST_RESULT"


def build_js_case(case_name, body):
    """Wrap a JS assertion body in a machine-readable PASS/FAIL protocol."""
    case_literal = json.dumps(case_name)
    indented_body = indent(body.strip(), "    ")
    return (
        "function assert(condition, message) {\n"
        "    if (!condition) throw new Error(message);\n"
        "}\n\n"
        "async function run() {\n"
        f"{indented_body}\n"
        "}\n\n"
        "run().then(function() {\n"
        f'    console.log("{RESULT_PREFIX}|PASS|" + {case_literal});\n'
        "}).catch(function(error) {\n"
        f'    console.error("{RESULT_PREFIX}|FAIL|" + '
        f'{case_literal} + "|" + error);\n'
        "});"
    )


def run_js_case(cli, storage_api, storage_dir, case_name, body, timeout=25):
    """Upload one JS case, execute it on the device, and assert its result."""
    script_path = f"{storage_dir}/js_{case_name}.js"
    source = build_js_case(case_name, body)

    allure.attach(source, "JavaScript source", allure.attachment_type.TEXT)

    with allure.step(f"Upload JavaScript case {case_name}"):
        response = storage_api.write(script_path, source.encode("utf-8"))
        assert response.status_code == 200, (
            f"failed to upload {script_path}: HTTP {response.status_code}, "
            f"body={response.text[:200]!r}"
        )

    with allure.step(f"Run JavaScript case {case_name}"):
        output = cli.execute_command(
            f"js {script_path}", timeout=timeout, slow_command=True
        )
        allure.attach(
            output,
            "JavaScript CLI output",
            allure.attachment_type.TEXT,
        )

    with allure.step(f"Verify JavaScript case {case_name}"):
        pass_marker = f"{RESULT_PREFIX}|PASS|{case_name}"
        fail_marker = f"{RESULT_PREFIX}|FAIL|{case_name}"
        assert fail_marker not in output, f"JavaScript case failed: {output!r}"
        assert pass_marker in output, (
            f"JavaScript case produced no PASS marker {pass_marker!r}: "
            f"{output!r}"
        )

    return output
