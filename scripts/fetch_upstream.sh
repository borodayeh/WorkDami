#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
UPSTREAM="${1:-$ROOT/upstream/workrave}"
URL="$(awk -F': ' '$1=="repository"{gsub(/\r$/, "", $2); print $2}' "$ROOT/upstream/workrave.txt")"
SHA="$(awk -F': ' '$1=="commit"{gsub(/\r$/, "", $2); print $2}' "$ROOT/upstream/workrave.txt")"
[ -n "$URL" ] && [ -n "$SHA" ] || { echo "bad upstream pin file" >&2; exit 1; }
mkdir -p "$UPSTREAM"
git init -q "$UPSTREAM"
# Pin line endings before the first checkout. MSYS2 git and Git for Windows can
# disagree on core.autocrlf; a CRLF patch against an LF source makes git apply
# fail even though the patch content is correct. Do not "fix" this after checkout.
git -C "$UPSTREAM" config core.autocrlf false
git -C "$UPSTREAM" config core.eol lf
git -C "$UPSTREAM" fetch --depth 1 "$URL" "$SHA"
git -C "$UPSTREAM" checkout -q FETCH_HEAD
ACTUAL="$(git -C "$UPSTREAM" rev-parse HEAD)"
[ "$ACTUAL" = "$SHA" ] || { echo "pin mismatch: $ACTUAL != $SHA" >&2; exit 1; }
echo "upstream pinned at $ACTUAL"
