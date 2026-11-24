#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
UPSTREAM="${1:-$ROOT/upstream/workrave}"
[ -f "$UPSTREAM/CMakeLists.txt" ] || { echo "upstream missing" >&2; exit 1; }
rm -rf "$UPSTREAM/maksa"
mkdir -p "$UPSTREAM/maksa"
cp -r "$ROOT/core" "$UPSTREAM/maksa/core"
cp -r "$ROOT/integration" "$UPSTREAM/maksa/integration"
git -C "$UPSTREAM" apply --whitespace=nowarn "$ROOT/patches/workrave/0001-maksa-integration.patch"
echo "integration patch applied (UNVERIFIED until build succeeds)"
