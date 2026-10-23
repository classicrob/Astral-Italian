#!/bin/bash
set -euo pipefail
# Usage: package.sh BUILD_DIR [DIST_DIR] [LIBUSB_PREFIX]
build=${1:?Usage: package.sh BUILD_DIR [DIST_DIR] [LIBUSB_PREFIX]}
dist=${2:-"$HOME/MacioGalaxy2-team/scratch-pad/pad/dist"}
usb=${3:-$(cat "$build/padprobe-libusb-prefix.txt")}
here=$(cd -- "$(dirname -- "$0")" && pwd)
[[ -x "$build/padprobe" && -f "$usb/lib/libusb-1.0.0.dylib" ]]
mkdir -p "$dist"
dist=$(cd -- "$dist" && pwd)
# Use a private staging directory; never remove another lane's package.
stage=$(mktemp -d "$dist/.padprobe-stage.XXXXXX")
trap 'rm -rf -- "$stage"' EXIT
mkdir -p "$stage/PadProbe/bin" "$stage/PadProbe/Frameworks"
cp "$build/padprobe" "$stage/PadProbe/bin/padprobe"
cp "$usb/lib/libusb-1.0.0.dylib" "$stage/PadProbe/Frameworks/libusb-1.0.0.dylib"
install_name_tool -id '@rpath/libusb-1.0.0.dylib' "$stage/PadProbe/Frameworks/libusb-1.0.0.dylib"
# Plain-name dlopen in the supplied scratch SDL resolves beside this alias via
# LC_RPATH. It needs no Homebrew installation and no DYLD environment variables.
ln -s libusb-1.0.0.dylib "$stage/PadProbe/Frameworks/libusb-1.0.dylib"
cp "$here/../../PADPROBE.md" "$stage/PadProbe/README.txt"
mkdir -p "$stage/PadProbe/licenses"
cp "$here/../../licenses/libusb-COPYING" "$stage/PadProbe/licenses/libusb-COPYING"
cp "$here/../../licenses/THIRD_PARTY_NOTICES.txt" "$stage/PadProbe/licenses/THIRD_PARTY_NOTICES.txt"
cp "$build/padprobe-SDL-LICENSE.txt" "$stage/PadProbe/licenses/SDL-LICENSE.txt"
codesign --force --sign - "$stage/PadProbe/Frameworks/libusb-1.0.0.dylib"
codesign --force --sign - "$stage/PadProbe/bin/padprobe"
codesign --verify --strict "$stage/PadProbe/Frameworks/libusb-1.0.0.dylib"
codesign --verify --strict "$stage/PadProbe/bin/padprobe"
if [[ -e "$dist/PadProbe" || -e "$dist/PadProbe.zip" ]]; then
    echo "Refusing to overwrite an existing PadProbe package in $dist; choose another DIST_DIR" >&2
    exit 1
fi
mv "$stage/PadProbe" "$dist/PadProbe"
ditto -c -k --keepParent "$dist/PadProbe" "$dist/PadProbe.zip"
LC_ALL=C shasum -a 256 "$dist/PadProbe.zip"
du -h "$dist/PadProbe.zip"
