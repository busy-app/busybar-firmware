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


def build_crash_comment(flag_path: str) -> str | None:
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
    trace_status = data.get("trace_status", "unknown")
    crash_line = (data.get("crash_line") or "")[:3000]
    s3_urls = data.get("s3_urls")
    runner_name = os.environ.get("RUNNER_NAME", "unknown")

    lines = []
    lines.append(f"### 🚨 Crash Detected ({processor})")
    lines.append("")
    lines.append(f"**Trace status:** {trace_status} | **Runner:** {runner_name}")
    lines.append("")
    lines.append("<details>")
    lines.append("<summary>Crash log</summary>")
    lines.append("")
    lines.append("```")
    lines.append(crash_line)
    lines.append("```")
    lines.append("")
    lines.append("</details>")
    lines.append("")
    lines.append("**Traces and logs:**")

    if s3_urls:
        for key, url in s3_urls.items():
            lines.append(f"- [{key}]({url})")
    else:
        lines.append("URLs not available")

    return "\n".join(lines)


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

    # Test results (only when JUNIT_XML is provided)
    if junit_xml:
        results = parse_junit(junit_xml)

        # Job Summary
        summary = build_job_summary(results, allure_url, firmware_url, branch)
        if summary_file:
            with open(summary_file, "a") as f:
                f.write(summary + "\n")

        # PR Comment
        if pr_number:
            comment = build_pr_comment(results, allure_url)
            post_or_update_pr_comment(comment, "### Integration Tests:")

    # Crash PR Comment
    crash_flag = os.environ.get("CRASH_FLAG", "")
    if pr_number and crash_flag:
        crash_comment = build_crash_comment(crash_flag)
        if crash_comment:
            post_or_update_pr_comment(crash_comment, "### 🚨 Crash Detected")


if __name__ == "__main__":
    main()
