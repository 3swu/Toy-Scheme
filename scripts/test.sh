#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${PROJECT_ROOT}/build}"
BIN_PATH="${BUILD_DIR}/Toy-Scheme"
CASES_DIR="${PROJECT_ROOT}/tests/cases"
EXPECTED_DIR="${PROJECT_ROOT}/tests/expected"
ARTIFACT_DIR="${ARTIFACT_DIR:-${PROJECT_ROOT}/test-artifacts}"
SKIP_BUILD=0

if [[ "${1:-}" == "--skip-build" ]]; then
    SKIP_BUILD=1
    shift
fi

if [[ $# -gt 0 ]]; then
    echo "Usage: $0 [--skip-build]" >&2
    exit 1
fi

if [[ "${SKIP_BUILD}" == "0" ]]; then
    "${SCRIPT_DIR}/build.sh"
fi

if [[ ! -x "${BIN_PATH}" ]]; then
    echo "Binary not found: ${BIN_PATH}" >&2
    exit 1
fi

rm -rf "${ARTIFACT_DIR}"
mkdir -p "${ARTIFACT_DIR}/raw" "${ARTIFACT_DIR}/filtered" "${ARTIFACT_DIR}/diff"

shopt -s nullglob
case_files=("${CASES_DIR}"/*.scm)

if [[ ${#case_files[@]} -eq 0 ]]; then
    echo "No test cases found in ${CASES_DIR}" >&2
    exit 1
fi

total=0
failed=0

for case_file in "${case_files[@]}"; do
    total=$((total + 1))
    case_name="$(basename "${case_file}" .scm)"
    expected_file="${EXPECTED_DIR}/${case_name}.txt"
    raw_file="${ARTIFACT_DIR}/raw/${case_name}.raw.txt"
    output_file="${ARTIFACT_DIR}/filtered/${case_name}.out.txt"
    diff_file="${ARTIFACT_DIR}/diff/${case_name}.diff.txt"

    if [[ ! -f "${expected_file}" ]]; then
        echo "[FAIL] ${case_name}: missing expected file ${expected_file}"
        failed=$((failed + 1))
        continue
    fi

    printf '' | "${BIN_PATH}" -f "${case_file}" > "${raw_file}" 2>&1 || true

    awk '
        $0 == "Welcome to Toy-Scheme" {next}
        $0 == "Press Ctrl-C to exit" {next}
        $0 ~ /^> / {next}
        $0 == ">" {next}
        $0 ~ /^[[:space:]]*$/ {next}
        {print}
    ' "${raw_file}" > "${output_file}"

    if diff -u "${expected_file}" "${output_file}" > "${diff_file}"; then
        rm -f "${diff_file}"
        echo "[PASS] ${case_name}"
    else
        failed=$((failed + 1))
        echo "[FAIL] ${case_name} (diff: ${diff_file})"
    fi
done

echo "Total: ${total}, Passed: $((total - failed)), Failed: ${failed}"

if [[ "${failed}" -gt 0 ]]; then
    exit 1
fi
