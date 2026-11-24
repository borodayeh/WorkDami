#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
UPSTREAM="${1:-$ROOT/upstream/workrave}"
URL="$(awk -F': ' '$1=="repository"{print $2}' "$ROOT/upstream/workrave.txt")"
SHA="$(awk -F': ' '$1=="commit"{print $2}' "$ROOT/upstream/workrave.txt")"
[ -n "$URL" ] && [ -n "$SHA" ] || { echo "bad upstream pin file" >&2; exit 1; }
mkdir -p "$UPSTREAM"
git init -q "$UPSTREAM"
git -C "$UPSTREAM" fetch --depth 1 "$URL" "$SHA"
git -C "$UPSTREAM" checkout -q FETCH_HEAD
ACTUAL="$(git -C "$UPSTREAM" rev-parse HEAD)"
[ "$ACTUAL" = "$SHA" ] || { echo "pin mismatch: $ACTUAL != $SHA" >&2; exit 1; }
echo "upstream pinned at $ACTUAL"
