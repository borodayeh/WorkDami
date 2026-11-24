#!/usr/bin/env bash
set -euo pipefail
[ "${MSYSTEM:-}" = "CLANG64" ] || { echo "MSYSTEM must be CLANG64, got: ${MSYSTEM:-unset}" >&2; exit 1; }
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
UPSTREAM="${1:-$ROOT/upstream/workrave}"
BUILD="${2:-$ROOT/build/workrave-qt}"
TOOLCHAIN="$UPSTREAM/cmake/toolchains/msys2.cmake"
[ -f "$TOOLCHAIN" ] || { echo "toolchain missing: $TOOLCHAIN" >&2; exit 1; }
cmake -S "$UPSTREAM" -B "$BUILD" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
  -DWITH_UI=Qt
cmake --build "$BUILD" --config Release
