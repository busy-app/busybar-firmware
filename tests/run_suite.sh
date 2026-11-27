#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"

filter="${MARKER_FILTER-not regression}"
filter="$(printf '%s' "$filter" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')"
case "$filter" in
  \"*\") filter="${filter#\"}"; filter="${filter%\"}" ;;
esac
if [[ "${filter,,}" == 'all' ]]; then
  filter=''
fi

retries="${PYTEST_FAILED_TEST_RETRIES-1}"
if [[ ! "$retries" =~ ^[0-9]+$ ]]; then
  echo 'PYTEST_FAILED_TEST_RETRIES must be a nonnegative integer.' >&2
  exit 2
fi

pytest_args=(. -v --durations=25 --alluredir=allure-results --clean-alluredir --junitxml=test-results.xml)
if [[ -n "$filter" ]]; then
  pytest_args+=(-m "$filter")
  echo "Running pytest with marker filter: $filter"
else
  echo 'Running pytest without marker filter (full suite)'
fi
if [[ "$retries" -gt 0 ]]; then
  pytest_args+=(--reruns "$retries" --reruns-delay 1 -rR)
fi

test_command=(python3 -m pytest "${pytest_args[@]}")
if [[ "$filter" == 'not regression' ]]; then
  # Interrupt early enough for pytest to write partial reports before the hard limit.
  test_command=(timeout --foreground --signal=INT --kill-after=15s 585s "${test_command[@]}")
fi

exec "${test_command[@]}"
