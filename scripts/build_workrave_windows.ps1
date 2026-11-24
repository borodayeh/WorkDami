# SUPERSEDED — do not use as the build source of truth.
#
# The previous version mixed PowerShell with `pacman` and machine-specific
# absolute paths, so it could not represent a GitHub Actions runner reliably.
# Use the MSYS2 CLANG64 bash scripts in this directory instead:
#
#   bash scripts/fetch_upstream.sh
#   bash scripts/msys2_build.sh upstream/workrave build/workrave-qt
#   bash scripts/run_core_tests.sh build/core
#   bash scripts/apply_integration.sh upstream/workrave
#   bash scripts/msys2_build.sh upstream/workrave build/workrave-patched
#   bash scripts/package_windows.sh build/workrave-patched artifacts/maksa-test-fixture
#
# This wrapper exists only for local PowerShell users who already have MSYS2;
# it intentionally performs no install and no absolute-path assumptions.

$msys2 = "C:\msys64\usr\bin\bash.exe"
if (-not (Test-Path $msys2)) { throw "MSYS2 bash not found: $msys2" }
& $msys2 -lc "cd '$PSScriptRoot/..' && bash scripts/msys2_build.sh upstream/workrave build/workrave-qt"
