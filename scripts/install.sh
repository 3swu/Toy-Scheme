#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${PROJECT_ROOT}/build}"
PREFIX="${PREFIX:-/usr/local}"
USE_SUDO="${USE_SUDO:-0}"

if [[ $# -gt 0 ]]; then
    echo "Usage: $0" >&2
    echo "Env options: PREFIX=/path USE_SUDO=1 BUILD_DIR=/path" >&2
    exit 1
fi

"${SCRIPT_DIR}/build.sh"

if [[ "${USE_SUDO}" == "1" ]]; then
    sudo cmake --install "${BUILD_DIR}" --prefix "${PREFIX}"
else
    cmake --install "${BUILD_DIR}" --prefix "${PREFIX}"
fi

echo "Install complete: ${PREFIX}/bin/Toy-Scheme"
