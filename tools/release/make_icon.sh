#!/bin/sh
# Regenerates AppIcon.icns from make_icon.swift (an original generic star, not Nintendo artwork). macOS tools only.
set -eu
here=$(cd "$(dirname "$0")" && pwd)
work=$(mktemp -d "${TMPDIR:-/tmp}/astral-icon.XXXXXX")
trap 'rm -rf "$work"' EXIT
png="$work/icon.png"
swift "$here/make_icon.swift" "$png"
set="$work/AppIcon.iconset"
mkdir "$set"
for size in 16 32 128 256 512; do
    sips -z $size $size "$png" --out "$set/icon_${size}x${size}.png" >/dev/null
    double=$((size * 2))
    sips -z $double $double "$png" --out "$set/icon_${size}x${size}@2x.png" >/dev/null
done
iconutil -c icns "$set" -o "$here/AppIcon.icns"
echo "wrote $here/AppIcon.icns"
