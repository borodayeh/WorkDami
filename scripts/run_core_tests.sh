#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${1:-$ROOT/build/core}"
CXX_BIN="${CXX:-clang++}"
mkdir -p "$BUILD"
SRC=(
  "$ROOT/core/exercise_catalog.cc"
  "$ROOT/core/routine_planner.cc"
  "$ROOT/core/exercise_session.cc"
  "$ROOT/core/settings.cc"
  "$ROOT/core/exercise_view_adapter.cc"
  "$ROOT/core/coordinator.cc"
)
"$CXX_BIN" -std=c++17 -Wall -Wextra -pedantic -I"$ROOT/core" \
  "${SRC[@]}" "$ROOT/core/tests/run_tests.cc" -o "$BUILD/maksa_core_tests.exe"
"$CXX_BIN" -std=c++17 -Wall -Wextra -pedantic -I"$ROOT/core" \
  "${SRC[@]}" "$ROOT/core/tests/run_stage2_tests.cc" -o "$BUILD/maksa_stage2_tests.exe"
"$BUILD/maksa_core_tests.exe" "$ROOT/core/data/test_exercises.tsv"
"$BUILD/maksa_stage2_tests.exe" "$ROOT/core/data/test_exercises.tsv"
