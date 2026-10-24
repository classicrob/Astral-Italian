#!/bin/bash
# Full M1 5.4 campaign. All runs are sequential, private, no AX, nice 0.
# Usage: m1gates54.sh LABEL [--only promotion,cold,...] [--quiet-seconds 900]
set -eu
D=$(cd "$(dirname "$0")" && pwd)
export PYTHONDONTWRITEBYTECODE=1
exec python3 -B "$D/campaign54m.py" "$@"
