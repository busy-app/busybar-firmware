"""Helpers for uploading and running machine-readable JavaScript test cases."""

import json
import time
from textwrap import indent

import allure


RESULT_PREFIX = "JS_TEST_RESULT"
INPUT_READY_MARKER = "JS_INPUT_READY"


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


def _upload_js_case(storage_api, script_path, source, case_name):
    allure.attach(source, "JavaScript source", allure.attachment_type.TEXT)

    with allure.step(f"Upload JavaScript case {case_name}"):
        response = storage_api.write(script_path, source.encode("utf-8"))
        assert response.status_code == 200, (
            f"failed to upload {script_path}: HTTP {response.status_code}, "
            f"body={response.text[:200]!r}"
        )


def _verify_js_case_output(case_name, output):
    with allure.step(f"Verify JavaScript case {case_name}"):
        pass_marker = f"{RESULT_PREFIX}|PASS|{case_name}"
        fail_marker = f"{RESULT_PREFIX}|FAIL|{case_name}"
        assert fail_marker not in output, f"JavaScript case failed: {output!r}"
        assert pass_marker in output, (
            f"JavaScript case produced no PASS marker {pass_marker!r}: "
            f"{output!r}"
        )


def run_js_case(cli, storage_api, storage_dir, case_name, body, timeout=25):
    """Upload one JS case, execute it on the device, and assert its result."""
    script_path = f"{storage_dir}/js_{case_name}.js"
    source = build_js_case(case_name, body)

    _upload_js_case(storage_api, script_path, source, case_name)

    with allure.step(f"Run JavaScript case {case_name}"):
        output = cli.execute_command(
            f"js {script_path}", timeout=timeout, slow_command=True
        )
        allure.attach(
            output,
            "JavaScript CLI output",
            allure.attachment_type.TEXT,
        )
    _verify_js_case_output(case_name, output)

    return output


def _read_until_marker(cli, chunks, marker, timeout):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            chunk = cli.tn.read_very_eager().decode("utf-8", "ignore")
        except EOFError:
            break
        if chunk:
            chunks.append(chunk)
            if marker in "".join(chunks):
                return
        time.sleep(0.05)

    output = "".join(chunks)
    raise AssertionError(
        f"JavaScript input case did not emit {marker!r}: {output!r}"
    )


def run_js_input_case(
    cli,
    input_cli,
    storage_api,
    storage_dir,
    case_name,
    body,
    input_events,
    timeout=10,
):
    """Run a JS listener case and inject raw input events from another CLI."""
    script_path = f"{storage_dir}/js_{case_name}.js"
    source = build_js_case(case_name, body)
    chunks = []
    prompt_received = False

    _upload_js_case(storage_api, script_path, source, case_name)

    try:
        with allure.step("Start the JavaScript listener"):
            try:
                cli.tn.read_very_eager()
            except EOFError:
                pass
            cli.tn.write(f"js {script_path}\r\n".encode("utf-8"))
            _read_until_marker(
                cli,
                chunks,
                INPUT_READY_MARKER,
                timeout,
            )

        with allure.step("Inject input events"):
            for key, event_type in input_events:
                command = f"input send {key} {event_type}"
                output = input_cli.execute_command(command)
                assert "Usage: input" not in output, (
                    f"input injection failed for {command!r}: {output!r}"
                )

        with allure.step("Wait for the listener case to finish"):
            tail = cli.tn.read_until(b">: ", timeout=timeout)
            chunks.append(tail.decode("utf-8", "ignore"))
            prompt_received = b">: " in tail
    finally:
        if not prompt_received:
            input_cli.execute_command("js -k")
            tail = cli.tn.read_until(b">: ", timeout=timeout)
            chunks.append(tail.decode("utf-8", "ignore"))

    output = "".join(chunks)
    allure.attach(
        output,
        "JavaScript CLI output",
        allure.attachment_type.TEXT,
    )
    assert prompt_received, (
        "JavaScript input case did not return to the CLI prompt: "
        f"{output!r}"
    )
    _verify_js_case_output(case_name, output)

    return output
