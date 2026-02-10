#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${PROJECT_ROOT}/build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
PARALLEL_JOBS="${PARALLEL_JOBS:-}"

if [[ "${1:-}" == "--clean" ]]; then
    rm -rf "${BUILD_DIR}"
    shift
fi

if [[ $# -gt 0 ]]; then
    echo "Usage: $0 [--clean]" >&2
    exit 1
fi

cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
if [[ -n "${PARALLEL_JOBS}" ]]; then
    cmake --build "${BUILD_DIR}" --parallel "${PARALLEL_JOBS}"
else
    cmake --build "${BUILD_DIR}" --parallel
fi

ln -sfn "${BUILD_DIR}/Toy-Scheme" "${PROJECT_ROOT}/Toy-Scheme"

echo "Build complete: ${BUILD_DIR}/Toy-Scheme"
echo "Synced executable: ${PROJECT_ROOT}/Toy-Scheme"
