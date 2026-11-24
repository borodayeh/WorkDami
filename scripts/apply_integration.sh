#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
UPSTREAM="${1:-$ROOT/upstream/workrave}"
PATCH="$ROOT/patches/workrave/0001-maksa-integration.patch"
PIN_FILE="$ROOT/upstream/workrave.txt"

[ -f "$UPSTREAM/CMakeLists.txt" ] || { echo "upstream missing" >&2; exit 1; }
[ -f "$PATCH" ] || { echo "patch missing: $PATCH" >&2; exit 1; }
[ -f "$PIN_FILE" ] || { echo "upstream pin missing" >&2; exit 1; }
EXPECTED="$(awk -F': ' '$1=="commit"{gsub(/\r$/, "", $2); print $2}' "$PIN_FILE")"
[ -n "$EXPECTED" ] || { echo "bad upstream pin file" >&2; exit 1; }

# Record the real destination state before touching anything. This log is kept
# as a CI artifact even when the patch step fails.
{
  echo "=== upstream state before patch ==="
  echo "expected_commit=$EXPECTED"
  echo "HEAD=$(git -C "$UPSTREAM" rev-parse HEAD)"
  echo "--- status --short ---"
  git -C "$UPSTREAM" status --short
  echo "--- tracked changes (must be empty) ---"
  git -C "$UPSTREAM" status --porcelain
  echo "--- core.autocrlf ---"
  git -C "$UPSTREAM" config --show-origin --get core.autocrlf || true
  echo "--- core.eol ---"
  git -C "$UPSTREAM" config --show-origin --get core.eol || true
  echo "--- ls-files --eol (patch targets) ---"
  git -C "$UPSTREAM" ls-files --eol -- \
    CMakeLists.txt \
    ui/app/CMakeLists.txt \
    ui/app/toolkits/qt/Toolkit.hh \
    ui/app/toolkits/qt/Toolkit.cc \
    ui/app/toolkits/qt/qml/sanctuary.qrc
  echo "--- patch file ---"
  echo "path=$PATCH"
  sha256sum "$PATCH"
  echo "CR_bytes=$(tr -cd '\r' < "$PATCH" | wc -c)"
  echo "--- patch first 5 lines (escaped) ---"
  sed -n '1,5l' "$PATCH"
} 2>&1

# Fail loudly instead of guessing: wrong commit, a dirty checkout, or a patch
# that was already applied are all real errors, not whitespace problems.
ACTUAL="$(git -C "$UPSTREAM" rev-parse HEAD)"
[ "$ACTUAL" = "$EXPECTED" ] || {
  echo "upstream pin mismatch: $ACTUAL != $EXPECTED" >&2
  exit 1
}
if [ -n "$(git -C "$UPSTREAM" status --porcelain)" ]; then
  echo "upstream checkout is not clean before patch; refusing to guess" >&2
  exit 1
fi
if git -C "$UPSTREAM" apply --reverse --check "$PATCH" >/dev/null 2>&1; then
  echo "patch is already applied to this checkout; refusing to apply twice" >&2
  exit 1
fi

echo "--- git apply --check ---"
git -C "$UPSTREAM" apply --check "$PATCH"

# Only now bring in the untracked source directories created by the patch.
rm -rf "$UPSTREAM/maksa"
mkdir -p "$UPSTREAM/maksa"
cp -r "$ROOT/core" "$UPSTREAM/maksa/core"
cp -r "$ROOT/integration" "$UPSTREAM/maksa/integration"

echo "--- git apply ---"
git -C "$UPSTREAM" apply "$PATCH"
echo "integration patch applied (UNVERIFIED until build succeeds)"
