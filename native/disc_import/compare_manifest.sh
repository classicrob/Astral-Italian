#!/bin/sh
# Compare only paths in the reference manifest; never modifies either tree.
set -eu
LC_ALL=C
export LC_ALL
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <extracted-root> <sha256-manifest>" >&2
    exit 2
fi
root=$1
manifest=$(cd "$(dirname "$2")" && pwd)/$(basename "$2")
cd "$root"
# Reject absolute paths and parent traversal before letting shasum open paths.
python3 - "$manifest" <<'PY'
import pathlib, re, sys
for line in pathlib.Path(sys.argv[1]).read_text().splitlines():
    if not re.fullmatch(r'[0-9a-fA-F]{64}  .+', line):
        sys.exit('Invalid SHA-256 manifest line')
    path = pathlib.PurePosixPath(line[66:])
    if path.is_absolute() or '..' in path.parts or path.parts[0] not in ('files', 'sys'):
        sys.exit('Unsafe path in manifest')
PY
shasum -a 256 -c "$manifest"
