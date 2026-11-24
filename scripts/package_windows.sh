#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${1:-$ROOT/build/workrave-qt}"
ARTIFACT="${2:-$ROOT/artifacts/maksa-test-fixture}"
UPSTREAM="${3:-$ROOT/upstream/workrave}"
mkdir -p "$ARTIFACT/maksa"

EXE="$(find "$BUILD" -type f -name workrave.exe -print -quit)"
[ -n "$EXE" ] || { echo "workrave.exe not found under $BUILD" >&2; exit 1; }
cp "$EXE" "$ARTIFACT/workrave.exe"
cp "$ROOT/core/data/test_exercises.tsv" "$ARTIFACT/maksa/test_exercises.tsv"
cp "$UPSTREAM/COPYING" "$ARTIFACT/COPYING-Workrave.txt"
cp "$ROOT/THIRD_PARTY.md" "$ARTIFACT/THIRD_PARTY.md"
cat > "$ARTIFACT/README-TEST.txt" <<'TXT'
MAKSA / TEST-FIXTURE BUILD — NOT RELEASE CONTENT
================================================

This archive is an unverified Windows test artifact produced by GitHub Actions.

Contents:
  workrave.exe                 patched Workrave with the Maksa test HUD
  maksa/test_exercises.tsv     explicit TEST_FIXTURE records only
  COPYING-Workrave.txt         upstream GPL-3.0 licence
  THIRD_PARTY.md               attribution and design-reference notices

Run: extract the archive and start workrave.exe. The HUD shows a test record,
the step number and the session state. Pause / Resume / Skip are wired to the
same in-memory session. No installer and no code signing are included.

NOT verified by this artifact: visual notch behaviour, multi-monitor behaviour,
persistent settings on a real machine, audio, animations, real exercise data.
TXT

DEPLOY="$(command -v windeployqt6 || command -v windeployqt-qt6 || { echo "${MINGW_PREFIX:-}/bin/windeployqt6"; })"
if [ -x "$DEPLOY" ]; then
  "$DEPLOY" --dir "$ARTIFACT" --qmldir "$UPSTREAM/ui/app/toolkits/qt/qml" "$ARTIFACT/workrave.exe"
else
  echo "windeployqt not found; packaging may be incomplete" >&2
  exit 1
fi

[ -s "$ARTIFACT/workrave.exe" ] || { echo "empty artifact" >&2; exit 1; }
echo "test artifact prepared: $ARTIFACT"
