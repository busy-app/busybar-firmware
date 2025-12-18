"""
CLI helper utilities for BSB Test Automation.

Provides utilities for CLI output parsing and validation.
"""

from __future__ import annotations

import re
from typing import Any

import allure


class CLIOutput:
    """Wrapper around CLI command output with parsing helpers."""

    def __init__(self, output: str, command: str = None):
        self._output = output
        self._command = command
        self._lines: list[str] | None = None

    @property
    def raw(self) -> str:
        """Get raw output string."""
        return self._output

    @property
    def lines(self) -> list[str]:
        """Get output as list of lines (cached)."""
        if self._lines is None:
            self._lines = self._output.strip().split("\n")
        return self._lines

    @property
    def command(self) -> str | None:
        """Get the command that produced this output."""
        return self._command

    def __str__(self) -> str:
        return self._output

    def __contains__(self, item: str) -> bool:
        return item in self._output

    # Parsing methods

    def get_value(self, key: str, separator: str = ":") -> str | None:
        """
        Extract value for a key from key-value formatted output.

        Args:
            key: Key to search for
            separator: Separator between key and value (default ":")

        Returns:
            Value string or None if not found

        Example:
            output = "Version: 1.2.3\\nBuild: 456"
            output.get_value("Version")  # Returns "1.2.3"
        """
        for line in self.lines:
            if key in line and separator in line:
                parts = line.split(separator, 1)
                if len(parts) == 2 and key in parts[0]:
                    return parts[1].strip()
        return None

    def get_int_value(self, key: str, separator: str = ":") -> int | None:
        """Extract integer value for a key."""
        value = self.get_value(key, separator)
        if value is not None:
            try:
                # Handle values like "1234 bytes" -> extract first number
                match = re.search(r"-?\d+", value)
                if match:
                    return int(match.group())
            except ValueError:
                pass
        return None

    def get_float_value(self, key: str, separator: str = ":") -> float | None:
        """Extract float value for a key."""
        value = self.get_value(key, separator)
        if value is not None:
            try:
                match = re.search(r"-?\d+\.?\d*", value)
                if match:
                    return float(match.group())
            except ValueError:
                pass
        return None

    def get_all_values(self, separator: str = ":") -> dict[str, str]:
        """
        Parse all key-value pairs from output.

        Returns:
            Dictionary of key-value pairs
        """
        result = {}
        for line in self.lines:
            if separator in line:
                parts = line.split(separator, 1)
                if len(parts) == 2:
                    key = parts[0].strip()
                    value = parts[1].strip()
                    if key:
                        result[key] = value
        return result

    def find_line(self, pattern: str, regex: bool = False) -> str | None:
        """
        Find first line matching pattern.

        Args:
            pattern: String or regex pattern to match
            regex: Whether pattern is a regex

        Returns:
            Matching line or None
        """
        if regex:
            compiled = re.compile(pattern)
            for line in self.lines:
                if compiled.search(line):
                    return line
        else:
            for line in self.lines:
                if pattern in line:
                    return line
        return None

    def find_lines(self, pattern: str, regex: bool = False) -> list[str]:
        """
        Find all lines matching pattern.

        Args:
            pattern: String or regex pattern to match
            regex: Whether pattern is a regex

        Returns:
            List of matching lines
        """
        results = []
        if regex:
            compiled = re.compile(pattern)
            for line in self.lines:
                if compiled.search(line):
                    results.append(line)
        else:
            for line in self.lines:
                if pattern in line:
                    results.append(line)
        return results

    def extract_regex(self, pattern: str, group: int = 1) -> str | None:
        """
        Extract first regex match from output.

        Args:
            pattern: Regex pattern with capture group
            group: Group number to extract (default 1)

        Returns:
            Matched group string or None
        """
        match = re.search(pattern, self._output)
        if match:
            try:
                return match.group(group)
            except IndexError:
                return match.group(0)
        return None

    def extract_all_regex(self, pattern: str, group: int = 1) -> list[str]:
        """
        Extract all regex matches from output.

        Args:
            pattern: Regex pattern with capture group
            group: Group number to extract (default 1)

        Returns:
            List of matched strings
        """
        results = []
        for match in re.finditer(pattern, self._output):
            try:
                results.append(match.group(group))
            except IndexError:
                results.append(match.group(0))
        return results

    # Validation methods

    def assert_contains(self, *texts: str, case_sensitive: bool = True) -> "CLIOutput":
        """Assert output contains all specified texts."""
        check_output = self._output if case_sensitive else self._output.lower()

        for text in texts:
            check_text = text if case_sensitive else text.lower()
            if check_text not in check_output:
                raise AssertionError(f"Output does not contain '{text}'")
        return self

    def assert_not_contains(
        self, *texts: str, case_sensitive: bool = True
    ) -> "CLIOutput":
        """Assert output does not contain any of the specified texts."""
        check_output = self._output if case_sensitive else self._output.lower()

        for text in texts:
            check_text = text if case_sensitive else text.lower()
            if check_text in check_output:
                raise AssertionError(f"Output contains forbidden text '{text}'")
        return self

    def assert_matches(self, pattern: str) -> "CLIOutput":
        """Assert output matches regex pattern."""
        if not re.search(pattern, self._output):
            raise AssertionError(f"Output does not match pattern '{pattern}'")
        return self

    def assert_has_value(self, key: str, separator: str = ":") -> "CLIOutput":
        """Assert output has a value for the specified key."""
        value = self.get_value(key, separator)
        if value is None:
            raise AssertionError(f"Output does not contain key '{key}'")
        return self

    def assert_value_equals(
        self, key: str, expected: str, separator: str = ":"
    ) -> "CLIOutput":
        """Assert key has expected value."""
        actual = self.get_value(key, separator)
        if actual is None:
            raise AssertionError(f"Output does not contain key '{key}'")
        if actual != expected:
            raise AssertionError(
                f"Key '{key}': expected '{expected}', got '{actual}'"
            )
        return self

    def assert_line_count(
        self,
        min_lines: int = None,
        max_lines: int = None,
        exact_lines: int = None,
    ) -> "CLIOutput":
        """Assert line count constraints."""
        actual = len(self.lines)

        if exact_lines is not None:
            if actual != exact_lines:
                raise AssertionError(
                    f"Expected {exact_lines} lines, got {actual}"
                )
            return self

        if min_lines is not None and actual < min_lines:
            raise AssertionError(
                f"Expected at least {min_lines} lines, got {actual}"
            )

        if max_lines is not None and actual > max_lines:
            raise AssertionError(
                f"Expected at most {max_lines} lines, got {actual}"
            )

        return self

    # Allure integration

    def attach_to_allure(self, name: str = "CLI Output") -> "CLIOutput":
        """Attach output to Allure report."""
        content = self._output
        if self._command:
            content = f"Command: {self._command}\n\n{content}"
        allure.attach(content, name=name, attachment_type=allure.attachment_type.TEXT)
        return self


def parse_cli_output(output: str, command: str = None) -> CLIOutput:
    """
    Create a CLIOutput wrapper for parsing.

    Args:
        output: Raw CLI output string
        command: Optional command that produced the output

    Returns:
        CLIOutput instance
    """
    return CLIOutput(output, command)


def parse_table_output(
    output: str,
    header_row: int = 0,
    separator: str = None,
) -> list[dict[str, str]]:
    """
    Parse tabular CLI output into list of dictionaries.

    Args:
        output: CLI output with tabular data
        header_row: Row number containing headers (0-indexed)
        separator: Column separator (None for whitespace)

    Returns:
        List of dictionaries with column headers as keys
    """
    lines = output.strip().split("\n")
    if len(lines) <= header_row:
        return []

    header_line = lines[header_row]
    if separator:
        headers = [h.strip() for h in header_line.split(separator)]
    else:
        headers = header_line.split()

    results = []
    for line in lines[header_row + 1 :]:
        if not line.strip():
            continue

        if separator:
            values = [v.strip() for v in line.split(separator)]
        else:
            values = line.split()

        # Pad values if fewer than headers
        while len(values) < len(headers):
            values.append("")

        row = dict(zip(headers, values[: len(headers)]))
        results.append(row)

    return results


def extract_section(
    output: str,
    start_marker: str,
    end_marker: str = None,
    include_markers: bool = False,
) -> str | None:
    """
    Extract a section of output between markers.

    Args:
        output: CLI output
        start_marker: Text marking section start
        end_marker: Text marking section end (None for end of output)
        include_markers: Whether to include markers in result

    Returns:
        Section text or None if start marker not found
    """
    start_idx = output.find(start_marker)
    if start_idx == -1:
        return None

    if not include_markers:
        start_idx += len(start_marker)

    if end_marker:
        end_idx = output.find(end_marker, start_idx)
        if end_idx == -1:
            section = output[start_idx:]
        else:
            if include_markers:
                end_idx += len(end_marker)
            section = output[start_idx:end_idx]
    else:
        section = output[start_idx:]

    return section.strip()
