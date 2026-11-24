#!/usr/bin/env bash
# Compiles and runs the pure C++ core with g++ only. No Qt/CMake required.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/core/build"
mkdir -p "$BUILD"

COMMON=(
  -std=c++17 -Wall -Wextra -pedantic
  -I"$ROOT/core"
  "$ROOT/core/exercise_catalog.cc"
  "$ROOT/core/routine_planner.cc"
  "$ROOT/core/exercise_session.cc"
  "$ROOT/core/settings.cc"
  "$ROOT/core/exercise_view_adapter.cc"
  "$ROOT/core/coordinator.cc"
)

echo "== build stage 1 tests =="
g++ "${COMMON[@]}" "$ROOT/core/tests/run_tests.cc" -o "$BUILD/maksa_core_tests"

echo "== build stage 2 tests =="
g++ "${COMMON[@]}" "$ROOT/core/tests/run_stage2_tests.cc" -o "$BUILD/maksa_stage2_tests"

echo "== build demos =="
g++ "${COMMON[@]}" "$ROOT/core/tools/plan_demo.cc" -o "$BUILD/maksa_plan_demo"
g++ "${COMMON[@]}" "$ROOT/core/tools/e2e_demo.cc" -o "$BUILD/maksa_e2e_demo"

echo "== run stage 1 tests =="
"$BUILD/maksa_core_tests" "$ROOT/core/data/test_exercises.tsv"

echo "== run stage 2 tests =="
"$BUILD/maksa_stage2_tests" "$ROOT/core/data/test_exercises.tsv"

echo "== run plan demo =="
"$BUILD/maksa_plan_demo" "$ROOT/core/data/test_exercises.tsv"

echo "== run end-to-end (non-GUI) demo =="
"$BUILD/maksa_e2e_demo" "$ROOT/core/data/test_exercises.tsv" "$BUILD/maksa_settings.txt"
