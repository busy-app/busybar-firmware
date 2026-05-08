#!/usr/bin/env python3
"""Parse JUnit XML and generate PR comment + Job Summary for GitHub Actions."""

import json
import os
import subprocess
import sys
import xml.etree.ElementTree as ET


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
            msg = node.attrib.get("message", "")[:200]
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


def build_pr_comment(results: dict | None, allure_url: str) -> str:
    lines = []

    if results is None:
        lines.append("### Integration Tests: results file not found")
        if allure_url:
            lines.append(f"[Allure Report]({allure_url})")
        return "\n".join(lines)

    r = results
    if r["failed_count"] == 0:
        lines.append(f"### Integration Tests: all {r['passed']} passed")
    else:
        lines.append(
            f"### Integration Tests: {r['failed_count']} failed / "
            f"{r['passed']} passed / {r['total']} total"
        )

    if allure_url:
        lines.append(f"[Allure Report]({allure_url})")

    if r["failed_tests"]:
        lines.append("")
        lines.append("<details>")
        lines.append(f"<summary>{len(r['failed_tests'])} failed tests</summary>")
        lines.append("")
        lines.append("| Test | Error |")
        lines.append("|------|-------|")
        for name, msg in r["failed_tests"][:30]:
            safe_msg = msg.replace("|", "\\|").replace("\n", " ")
            lines.append(f"| `{name}` | {safe_msg} |")
        if len(r["failed_tests"]) > 30:
            lines.append(f"| ... | +{len(r['failed_tests']) - 30} more |")
        lines.append("")
        lines.append("</details>")

    return "\n".join(lines)


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

    log_links = []
    for key in ("Full Log", "Trimmed Crash Log"):
        if key in s3_urls:
            log_links.append(f"- [{key}]({s3_urls[key]})")
    if log_links:
        lines.append("**Serial Logs:**")
        lines.extend(log_links)
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

    # Test results step (skipped on the crash-only invocation, where JUNIT_XML
    # is provided solely to extract the failed test name for the crash comment)
    if junit_xml and not crash_flag:
        results = parse_junit(junit_xml)

        summary = build_job_summary(results, allure_url, firmware_url, branch)
        if summary_file:
            with open(summary_file, "a") as f:
                f.write(summary + "\n")

        if pr_number:
            comment = build_pr_comment(results, allure_url)
            post_or_update_pr_comment(comment, "### Integration Tests:")

    if pr_number and crash_flag:
        crash_comment = build_crash_comment(crash_flag, junit_xml or None)
        if crash_comment:
            post_or_update_pr_comment(crash_comment, "### 🚨 Crash Detected")


if __name__ == "__main__":
    main()
