"""JavaScript input-listener integration coverage."""

from textwrap import dedent

import allure
import pytest

from utils.js_test_runner import INPUT_READY_MARKER, run_js_input_case


pytestmark = pytest.mark.cli


@allure.epic("BSB CLI Testing")
@allure.feature("JavaScript input API")
@allure.story("JS Apps")
class TestJSInput:
    @allure.title("JavaScript listen validates its input-listener contract.")
    def test_listen_contract_and_validation(self, js_case_runner):
        body = dedent(
            """
                assert(typeof listen === "function", "listen is unavailable");

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

                assertTypeError(function() { listen(); },
                    "missing arguments");
                assertTypeError(function() { listen(7, function() {}); },
                    "non-string event type");
                assertTypeError(function() {
                    listen("unsupported", function() {});
                }, "unknown event type");
                assertTypeError(function() { listen("input", 7); },
                    "non-function handler");

                const unbind = listen("input", function() {});
                assert(typeof unbind === "function",
                    "listen did not return an unbind function");
                assertTypeError(function() {
                    listen("input", function() {});
                }, "handler override");

                unbind();
                const rebound = listen("input", function() {});
                assert(typeof rebound === "function",
                    "listen could not register after unbind");
                rebound();
            """
        ).strip()

        js_case_runner("input_contract", body)

    @allure.title("JavaScript listen maps buttons and encoder events.")
    def test_event_mapping(
        self,
        persistent_cli_connection,
        fresh_cli_connection,
        storage_api,
        storage_dir,
    ):
        body = dedent(
            f"""
                let unbind;
                const events = await new Promise(function(resolve) {{
                    const captured = [];
                    unbind = listen("input", function(event) {{
                        captured.push(event);
                        console.log("JS_INPUT_EVENT|" +
                            JSON.stringify(event));
                        if(captured.length === 6) {{
                            resolve(captured);
                        }}
                    }});
                    console.log("{INPUT_READY_MARKER}");
                }});

                assert(events[0].key === "encoder" &&
                    events[0].action === "clockwise" &&
                    events[0].delta === 1,
                    "clockwise event=" + JSON.stringify(events[0]));
                assert(events[1].key === "encoder" &&
                    events[1].action === "counterclockwise" &&
                    events[1].delta === -1,
                    "counterclockwise event=" + JSON.stringify(events[1]));
                assert(events[2].key === "ok" &&
                    events[2].action === "press",
                    "OK press event=" + JSON.stringify(events[2]));
                assert(events[3].key === "ok" &&
                    events[3].action === "release",
                    "OK release event=" + JSON.stringify(events[3]));
                assert(events[4].key === "start" &&
                    events[4].action === "press",
                    "Start event=" + JSON.stringify(events[4]));
                assert(events[5].key === "back" &&
                    events[5].action === "release",
                    "Back event=" + JSON.stringify(events[5]));
                assert(!("delta" in events[2]),
                    "button event unexpectedly has delta");

                setTimeout(unbind, 0);
            """
        ).strip()
        input_events = [
            ("InputKeyUp", "InputTypePress"),
            ("InputKeyDown", "InputTypePress"),
            ("InputKeyOk", "InputTypePress"),
            ("InputKeyOk", "InputTypeRelease"),
            ("InputKeyStart", "InputTypePress"),
            ("InputKeyBack", "InputTypeRelease"),
        ]

        run_js_input_case(
            persistent_cli_connection,
            fresh_cli_connection,
            storage_api,
            storage_dir,
            "input_mapping",
            body,
            input_events,
        )

    @allure.title("JavaScript listen ignores unsupported input events.")
    def test_unsupported_events_are_filtered(
        self,
        persistent_cli_connection,
        fresh_cli_connection,
        storage_api,
        storage_dir,
    ):
        body = dedent(
            f"""
                let unbind;
                const events = await new Promise(function(resolve) {{
                    const captured = [];
                    unbind = listen("input", function(event) {{
                        captured.push(event);
                        resolve(captured);
                    }});
                    console.log("{INPUT_READY_MARKER}");
                }});

                assert(events.length === 1,
                    "unexpected callback count=" + events.length);
                assert(events[0].key === "ok" &&
                    events[0].action === "press",
                    "accepted event=" + JSON.stringify(events[0]));

                setTimeout(unbind, 0);
            """
        ).strip()
        input_events = [
            ("InputKeyOk", "InputTypeShort"),
            ("InputKeyStart", "InputTypeLong"),
            ("InputKeyBack", "InputTypeRepeat"),
            ("InputKeyUp", "InputTypeRelease"),
            ("InputKeyCustom", "InputTypePress"),
            ("InputKeyOk", "InputTypePress"),
        ]

        run_js_input_case(
            persistent_cli_connection,
            fresh_cli_connection,
            storage_api,
            storage_dir,
            "input_filtering",
            body,
            input_events,
        )

    @allure.title("JavaScript unbind stops subsequent input callbacks.")
    def test_unbind_stops_callbacks(
        self,
        persistent_cli_connection,
        fresh_cli_connection,
        storage_api,
        storage_dir,
    ):
        body = dedent(
            f"""
                const events = await new Promise(function(resolve) {{
                    const captured = [];
                    const unbind = listen("input", function(event) {{
                        captured.push(event);
                        setTimeout(function() {{
                            resolve(captured);
                        }}, 1000);
                        unbind();
                    }});
                    console.log("{INPUT_READY_MARKER}");
                }});

                assert(events.length === 1,
                    "callback ran after unbind: " +
                    JSON.stringify(events));
                assert(events[0].key === "ok",
                    "first event=" + JSON.stringify(events[0]));
            """
        ).strip()
        input_events = [
            ("InputKeyOk", "InputTypePress"),
            ("InputKeyStart", "InputTypePress"),
        ]

        run_js_input_case(
            persistent_cli_connection,
            fresh_cli_connection,
            storage_api,
            storage_dir,
            "input_unbind",
            body,
            input_events,
        )

    @allure.title("JavaScript input listener survives a handler exception.")
    def test_handler_exception_does_not_stop_listener(
        self,
        persistent_cli_connection,
        fresh_cli_connection,
        storage_api,
        storage_dir,
    ):
        body = dedent(
            f"""
                let callbackCount = 0;
                let unbind;
                const finalEvent = await new Promise(function(resolve) {{
                    unbind = listen("input", function(event) {{
                        callbackCount++;
                        if(callbackCount === 1) {{
                            throw new Error("expected handler failure");
                        }}
                        resolve(event);
                    }});
                    console.log("{INPUT_READY_MARKER}");
                }});

                assert(callbackCount === 2,
                    "callback count=" + callbackCount);
                assert(finalEvent.key === "start" &&
                    finalEvent.action === "press",
                    "final event=" + JSON.stringify(finalEvent));

                setTimeout(unbind, 0);
            """
        ).strip()
        input_events = [
            ("InputKeyOk", "InputTypePress"),
            ("InputKeyStart", "InputTypePress"),
        ]

        run_js_input_case(
            persistent_cli_connection,
            fresh_cli_connection,
            storage_api,
            storage_dir,
            "input_handler_exception",
            body,
            input_events,
        )
