#!/usr/bin/env python3
"""Parse JUnit XML and generate PR comment + Job Summary for GitHub Actions."""

import json
import html
import os
import subprocess
import sys
import xml.etree.ElementTree as ET


JUNIT_FAILURE_MESSAGE_LIMIT = 800
FULL_LOG_LINK_KEY = "Full Log"
TRACE_LINK_KEYS = ("U5 Trace", "Si917 Trace")


def parse_junit(xml_path: str) -> dict:
    if not os.path.exists(xml_path):
        return None

    root = ET.parse(xml_path).getroot()

    total = failures = errors = skipped = 0
    time_s = 0.0
    for suite in root.iter("testsuite"):
        total += int(suite.attrib.get("tests", 0))
        failures += int(suite.attrib.get("failures", 0))
        errors += int(suite.attrib.get("errors", 0))
        skipped += int(suite.attrib.get("skipped", 0))
        time_s += float(suite.attrib.get("time", 0))

    failed_tests = []
    for tc in root.iter("testcase"):
        fail = tc.find("failure")
        err = tc.find("error")
        if fail is not None or err is not None:
            node = fail if fail is not None else err
            name = tc.attrib.get("classname", "") + "::" + tc.attrib.get("name", "")
            msg = node.attrib.get("message", "")[:JUNIT_FAILURE_MESSAGE_LIMIT]
            failed_tests.append((name, msg))

    return {
        "total": total,
        "passed": total - failures - errors - skipped,
        "failures": failures,
        "errors": errors,
        "skipped": skipped,
        "failed_count": failures + errors,
        "time_s": time_s,
        "failed_tests": failed_tests,
    }


def load_forced_traces(log_dir: str) -> dict:
    """Index forced-trace artifacts by JUnit-style test id.

    For each test that triggered a forced GDB trace, conftest writes a thin
    sidecar (`<log_dir>/forced_traces/<sanitized_nodeid>.sidecar.json`)
    listing the request_ids and a JUnit-style id we can match against
    JUnit XML output. The actual S3 URLs live in a per-request flag
    (`<log_dir>/forced_traces/<request_id>.flag`) enriched by
    notification_service. We join them here so the PR-comment renderer
    sees `{junit_id: [{processor, s3_urls, trace_status, ...}, ...]}`.
    """
    if not log_dir:
        return {}
    trace_dir = os.path.join(log_dir, "forced_traces")
    if not os.path.isdir(trace_dir):
        return {}

    mapping: dict[str, list] = {}
    for entry in sorted(os.listdir(trace_dir)):
        if not entry.endswith(".sidecar.json"):
            continue
        sidecar_path = os.path.join(trace_dir, entry)
        try:
            with open(sidecar_path) as f:
                records = json.load(f)
        except (OSError, json.JSONDecodeError):
            continue
        if not isinstance(records, list):
            records = [records] if isinstance(records, dict) else []
        for record in records:
            if not isinstance(record, dict):
                continue
            junit_id = record.get("junit_id") or record.get("nodeid")
            if not junit_id:
                continue
            request_id = record.get("request_id")
            flag = _read_forced_trace_flag(trace_dir, request_id) if request_id else None
            mapping.setdefault(junit_id, []).append(
                {
                    "request_id": request_id,
                    "phase": record.get("phase"),
                    "processor": record.get("processor") or (flag or {}).get("processor"),
                    "trace_status": (flag or {}).get("trace_status") or record.get("trace_status"),
                    "s3_urls": (flag or {}).get("s3_urls") or {},
                    "skip_reason": (flag or {}).get("skip_reason"),
                    "reason": record.get("reason") or (flag or {}).get("reason"),
                }
            )
    return mapping


def _read_forced_trace_flag(trace_dir: str, request_id: str) -> dict | None:
    flag_path = os.path.join(trace_dir, f"{request_id}.flag")
    if not os.path.exists(flag_path):
        return None
    try:
        with open(flag_path) as f:
            return json.load(f)
    except (OSError, json.JSONDecodeError):
        return None


def _summary_error(msg: str, limit: int = 120) -> str:
    cleaned = (msg or "").replace("\n", " ").strip()
    if not cleaned:
        return "no error message"
    return cleaned if len(cleaned) <= limit else cleaned[: limit - 1] + "…"


def _blockquoted(msg: str) -> str:
    text = (msg or "").strip() or "no error message"
    return "\n".join(
        f"> {html.escape(line)}" if line else ">"
        for line in text.splitlines()
    )


def _trace_links(traces: list) -> list:
    seen: set[str] = set()
    lines: list[str] = []
    for trace in traces or []:
        for key in TRACE_LINK_KEYS:
            url = (trace.get("s3_urls") or {}).get(key)
            if not url or url in seen:
                continue
            seen.add(url)
            lines.append(f"- [{key}]({url})")
    return lines


def _first_full_log_url(traces_by_id: dict) -> str:
    for traces in traces_by_id.values():
        for trace in traces or []:
            url = (trace.get("s3_urls") or {}).get(FULL_LOG_LINK_KEY)
            if url:
                return url
    return ""


def _report_links(allure_url: str, full_log_url: str = "") -> str:
    links = []
    if allure_url:
        links.append(f"[Allure Report]({allure_url})")
    if full_log_url:
        links.append(f"[{FULL_LOG_LINK_KEY}]({full_log_url})")
    return " | ".join(links)


def _failed_test_block(name: str, msg: str, traces: list) -> str:
    lines = [
        "<details>",
        (
            f"<summary><code>{html.escape(name)}</code> — FAILED — "
            f"{html.escape(_summary_error(msg))}</summary>"
        ),
        "",
        _blockquoted(msg),
        "",
    ]
    link_lines = _trace_links(traces)
    if link_lines:
        lines.extend(link_lines)
        lines.append("")
    elif traces:
        # We have forced-trace records but no S3 URLs (upload disabled or
        # still in flight when the workflow ran). Surface this so the
        # reader does not assume traces were never attempted.
        status = (traces[0].get("trace_status") or "unknown").strip()
        skip_reason = traces[0].get("skip_reason")
        if skip_reason:
            lines.append(f"_Trace skipped: {skip_reason}_")
        else:
            lines.append(f"_Trace captured (status: {status}), no S3 links available._")
        lines.append("")
    lines.append("</details>")
    return "\n".join(lines)


def load_pytest_reruns(path: str) -> list[str]:
    if not path or not os.path.exists(path):
        return []

    reruns: list[str] = []
    seen: set[str] = set()
    try:
        with open(path, errors="replace") as f:
            for line in f:
                if not line.startswith("RERUN "):
                    continue
                name = line.removeprefix("RERUN ").strip()
                if not name or name in seen:
                    continue
                seen.add(name)
                reruns.append(name)
    except OSError:
        return []
    return reruns


def build_pr_comment(
    results: dict | None,
    allure_url: str,
    log_dir: str = "",
    rerun_failures: list[str] | None = None,
) -> str:
    lines = []
    rerun_failures = rerun_failures or []
    recovered = bool(results and results["failed_count"] == 0 and rerun_failures)
    failed_tests = (results or {}).get("failed_tests") or []
    traces_by_id = load_forced_traces(log_dir) if failed_tests else {}
    report_links = _report_links(allure_url, _first_full_log_url(traces_by_id))

    if results is None:
        lines.append("### Integration Tests: results file not found")
        if report_links:
            lines.append(report_links)
        return "\n".join(lines)

    r = results
    if recovered:
        lines.append(
            f"### Integration Tests: passed after retry "
            f"({len(rerun_failures)} failed initially / {r['passed']} passed / {r['total']} total)"
        )
    elif r["failed_count"] == 0:
        lines.append(f"### Integration Tests: all {r['passed']} passed")
    else:
        lines.append(
            f"### Integration Tests: {r['failed_count']} failed / "
            f"{r['passed']} passed / {r['total']} total"
        )

    if report_links:
        lines.append(report_links)

    if recovered:
        lines.append("")
        lines.append("Recovered after retry:")
        for name in rerun_failures[:30]:
            lines.append(f"- <code>{html.escape(name)}</code>")
        if len(rerun_failures) > 30:
            lines.append(f"_…and {len(rerun_failures) - 30} more retried tests._")
    elif failed_tests:
        lines.append("")
        for name, msg in failed_tests[:30]:
            lines.append(_failed_test_block(name, msg, traces_by_id.get(name, [])))
            lines.append("")
        if len(failed_tests) > 30:
            lines.append(f"_…and {len(failed_tests) - 30} more failed tests._")

    return "\n".join(lines).rstrip() + "\n"


def build_job_summary(results: dict | None, allure_url: str, firmware_url: str, branch: str) -> str:
    lines = []
    commit = os.environ.get("GITHUB_SHA", "")[:7]

    lines.append("# Integration Tests")
    lines.append(f"**Branch:** {branch} | **Commit:** `{commit}`")
    if firmware_url:
        lines.append(f"**Firmware:** {firmware_url}")
    lines.append("")

    if results is None:
        lines.append("Test results file not found")
    else:
        r = results
        icon = "✅" if r["failed_count"] == 0 else "❌"
        lines.append(
            f"{icon} **{r['passed']}** passed | **{r['failed_count']}** failed | "
            f"**{r['skipped']}** skipped | **{r['total']}** total | {r['time_s']:.0f}s"
        )

    lines.append("")
    lines.append("## Links")
    if allure_url:
        lines.append(f"- [Allure Report]({allure_url})")

    return "\n".join(lines)


def get_first_failed_test(xml_path: str) -> str | None:
    """Return classname::name of the first failed/errored test from JUnit XML."""
    if not xml_path or not os.path.exists(xml_path):
        return None
    try:
        root = ET.parse(xml_path).getroot()
    except Exception:
        return None
    for tc in root.iter("testcase"):
        if tc.find("failure") is not None or tc.find("error") is not None:
            classname = tc.attrib.get("classname", "")
            name = tc.attrib.get("name", "")
            return f"{classname}::{name}" if classname else name
    return None


def _is_trace_ok_from_results(proc_data: dict) -> bool:
    return proc_data.get("exit_code") == 0 and (proc_data.get("size") or 0) > 0


def _is_trace_ok_legacy(data: dict, key: str, log_dir: str) -> bool:
    """Fallback for crash flags written by old serial_logger (no `trace_results`)."""
    rel = data.get(key)
    if not rel:
        return False
    full = os.path.join(log_dir, rel) if log_dir else rel
    try:
        return os.path.exists(full) and os.path.getsize(full) > 0
    except OSError:
        return False


def build_crash_comment(flag_path: str, junit_xml: str | None = None) -> str | None:
    if not flag_path or not os.path.exists(flag_path):
        return None

    try:
        with open(flag_path) as f:
            data = json.load(f)
    except Exception:
        return None

    if not data:
        return None

    processor = data.get("processor", "unknown")
    crash_line = (data.get("crash_line") or "")[:3000]
    s3_urls = data.get("s3_urls") or {}
    runner_name = os.environ.get("RUNNER_NAME", "unknown")
    run_id = os.environ.get("GITHUB_RUN_ID", "")
    sha_full = os.environ.get("GITHUB_SHA", "")
    sha_short = sha_full[:7] if sha_full else ""
    repo = os.environ.get("GITHUB_REPOSITORY", "")
    log_dir = os.path.dirname(flag_path) if flag_path else ""

    trace_results = data.get("trace_results") or {}
    if trace_results:
        u5 = trace_results.get("u5") or {}
        si917 = trace_results.get("si917") or {}
        u5_exit = u5.get("exit_code")
        si917_exit = si917.get("exit_code")
        u5_ok = _is_trace_ok_from_results(u5)
        si917_ok = _is_trace_ok_from_results(si917)
    else:
        u5_exit = None
        si917_exit = None
        u5_ok = _is_trace_ok_legacy(data, "trace_file_u5", log_dir)
        si917_ok = _is_trace_ok_legacy(data, "trace_file_si917", log_dir)

    both_ok = u5_ok and si917_ok

    lines = [f"### 🚨 Crash Detected ({processor})", ""]

    meta_parts = []
    if run_id:
        if repo:
            meta_parts.append(
                f"**Run:** [#{run_id}](https://github.com/{repo}/actions/runs/{run_id})"
            )
        else:
            meta_parts.append(f"**Run:** #{run_id}")
    if sha_short:
        meta_parts.append(f"**SHA:** `{sha_short}`")
    meta_parts.append(f"**Runner:** {runner_name}")
    failed_test = get_first_failed_test(junit_xml) if junit_xml else None
    if failed_test:
        meta_parts.append(f"**Test:** `{failed_test}`")
    if meta_parts:
        lines.append(" | ".join(meta_parts))
        lines.append("")

    lines.extend(
        [
            "<details>",
            "<summary>Crash log</summary>",
            "",
            "```",
            crash_line,
            "```",
            "",
            "</details>",
            "",
        ]
    )

    lines.append("**Stack Traces:**")
    for proc_label, ok, exit_code, url_key in (
        ("U5 Trace", u5_ok, u5_exit, "U5 Trace"),
        ("Si917 Trace", si917_ok, si917_exit, "Si917 Trace"),
    ):
        url = s3_urls.get(url_key)
        if ok:
            ec = exit_code if exit_code is not None else 0
            label = f"{proc_label} (exit {ec})"
            line = f"- ✅ [{label}]({url})" if url else f"- ✅ {label}"
        elif exit_code is None:
            line = f"- ⚠️ {proc_label} (not collected)"
        else:
            line = f"- ❌ {proc_label} (exit {exit_code})"
        lines.append(line)
    lines.append("")

    full_log_url = s3_urls.get(FULL_LOG_LINK_KEY)
    if full_log_url:
        lines.append("**Serial Logs:**")
        lines.append(f"- [{FULL_LOG_LINK_KEY}]({full_log_url})")
        lines.append("")

    if not both_ok:
        lines.append("> ⚠️ Stack trace collection partially failed — see exit codes above")

    return "\n".join(lines).rstrip() + "\n"


def post_or_update_pr_comment(comment_body: str, marker: str):
    pr_number = os.environ.get("PR_NUMBER", "")
    repo = os.environ.get("GITHUB_REPOSITORY", "")
    if not pr_number or not repo:
        return

    # Find existing comment
    try:
        result = subprocess.run(
            [
                "gh", "api", f"repos/{repo}/issues/{pr_number}/comments",
                "--jq", f'[.[] | select(.body | startswith("{marker}"))][0].id',
            ],
            capture_output=True, text=True, timeout=30,
        )
        existing_id = result.stdout.strip()
    except Exception:
        existing_id = ""

    try:
        if existing_id and existing_id != "null":
            subprocess.run(
                ["gh", "api", f"repos/{repo}/issues/comments/{existing_id}",
                 "-X", "PATCH", "-f", f"body={comment_body}"],
                timeout=30,
            )
        else:
            subprocess.run(
                ["gh", "pr", "comment", pr_number, "--body", comment_body],
                timeout=30,
            )
    except Exception as e:
        print(f"Warning: failed to post PR comment: {e}", file=sys.stderr)


def main():
    junit_xml = os.environ.get("JUNIT_XML", "")
    allure_url = os.environ.get("ALLURE_URL", "")
    firmware_url = os.environ.get("FIRMWARE_URL", "")
    branch = os.environ.get("BRANCH_NAME", "")
    summary_file = os.environ.get("GITHUB_STEP_SUMMARY", "")
    pr_number = os.environ.get("PR_NUMBER", "")
    crash_flag = os.environ.get("CRASH_FLAG", "")
    log_dir = os.environ.get("LOG_DIR", "")

    # Test results step (skipped on the crash-only invocation, where JUNIT_XML
    # is provided solely to extract the failed test name for the crash comment)
    if junit_xml and not crash_flag:
        results = parse_junit(junit_xml)
        rerun_failures = load_pytest_reruns(os.environ.get("PYTEST_OUTPUT", ""))

        summary = build_job_summary(results, allure_url, firmware_url, branch)
        if summary_file:
            with open(summary_file, "a") as f:
                f.write(summary + "\n")

        if pr_number:
            comment = build_pr_comment(results, allure_url, log_dir, rerun_failures)
            post_or_update_pr_comment(comment, "### Integration Tests:")

    if pr_number and crash_flag:
        crash_comment = build_crash_comment(crash_flag, junit_xml or None)
        if crash_comment:
            post_or_update_pr_comment(crash_comment, "### 🚨 Crash Detected")


if __name__ == "__main__":
    main()
