#!/bin/sh
set -eu
exec python3 -I "$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)/symbolicate.py" "$@"
