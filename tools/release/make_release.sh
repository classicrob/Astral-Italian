#!/bin/bash
# Build the tester release package from a FROZEN Petari.app: a self-contained "Astral Italian.app" in a zip, the
# separate dSYM, checksums and a manifest. Never signs with a Developer ID and never publishes anything:
# see sign_notarize.sh and draft_release.sh.
#
#   tools/release/make_release.sh --app ~/MacioGalaxy2-play/build5.2.1/Petari.app --dsym <Petari.app.dSYM> \
#       --version 0.2.0 --commit <40-hex source commit> --min-macos 14.0 [--lib-root DIR] [--out DIR]
#
# --min-macos   the deployment target the frozen build was compiled for. The script FAILS if the executable or
#               any bundled library records a higher minimum (rebuild with CMAKE_OSX_DEPLOYMENT_TARGET instead).
# --lib-root    resolve /opt/homebrew/... libraries below DIR (an archived runtime); default: the real /opt/homebrew.
# --out         default ~/MacioGalaxy2-team/scratch-release/out
set -eu
here=$(cd "$(dirname "$0")" && pwd)
repo=$(cd "$here/../.." && pwd)

app="" dsym="" version="" commit="" minmacos="" libroot="" out="$HOME/MacioGalaxy2-team/scratch-release/out"
while [ $# -gt 0 ]; do
    case "$1" in
    --app) app=$2; shift 2 ;;
    --dsym) dsym=$2; shift 2 ;;
    --version) version=$2; shift 2 ;;
    --commit) commit=$2; shift 2 ;;
    --min-macos) minmacos=$2; shift 2 ;;
    --lib-root) libroot=$2; shift 2 ;;
    --out) out=$2; shift 2 ;;
    *) echo "make_release: unknown option $1" >&2; exit 2 ;;
    esac
done
for v in app dsym version commit minmacos; do
    eval "[ -n \"\$$v\" ]" || { echo "make_release: --${v//_/-} is required (see the header of this script)" >&2; exit 2; }
done
case "$version" in *[!0-9A-Za-z.+-]*|"") echo "make_release: bad version $version" >&2; exit 2 ;; esac
case "$commit" in *[!0-9a-f]*|"") echo "make_release: --commit must be lowercase hex" >&2; exit 2 ;; esac
[ -x "$app/Contents/MacOS/Petari" ] || { echo "make_release: no executable at $app/Contents/MacOS/Petari" >&2; exit 1; }
[ -f "$app/../FROZEN.txt" ] || [ "${ALLOW_UNFROZEN:-0}" = 1 ] || {
    echo "make_release: $app has no sibling FROZEN.txt; releases are built from frozen builds only" >&2; exit 1; }
[ -d "$dsym" ] || { echo "make_release: dSYM $dsym not found" >&2; exit 1; }
avail=$(df -g "$HOME" | awk 'NR==2{print $4}')
[ "$avail" -ge 20 ] || { echo "make_release: only ${avail} GB free" >&2; exit 1; }

# The dSYM must belong to this exact executable.
exe_uuid=$(dwarfdump --uuid "$app/Contents/MacOS/Petari" | awk '{print $2}' | head -n1)
dsym_uuid=$(dwarfdump --uuid "$dsym" | awk '{print $2}' | head -n1)
[ -n "$exe_uuid" ] && [ "$exe_uuid" = "$dsym_uuid" ] || {
    echo "make_release: dSYM UUID ($dsym_uuid) does not match the executable ($exe_uuid)" >&2; exit 1; }

name="AstralItalian-$version"
stage="$out/stage-$version"
folder="$stage/Astral Italian $version"
mkdir -p "$out"
if [ -e "$stage" ]; then
    case "$stage" in "$out"/stage-*) rm -rf "$stage" ;; *) echo "make_release: refusing to clear $stage" >&2; exit 1 ;; esac
fi
mkdir -p "$folder"
trap 'case "$stage" in "$out"/stage-*) rm -rf "$stage" ;; esac' EXIT
ditto "$app" "$folder/Astral Italian.app"
bundle="$folder/Astral Italian.app"
plist="$bundle/Contents/Info.plist"

pb() { /usr/libexec/PlistBuddy -c "Set :$1 $2" "$plist" 2>/dev/null || /usr/libexec/PlistBuddy -c "Add :$1 string $2" "$plist"; }
builddate=$(date -u +%Y-%m-%d)
pb CFBundleShortVersionString "$version"
pb CFBundleVersion "$version"
pb CFBundleLongVersionString "$version ($commit)"
pb CFBundleGetInfoString "Astral Italian $version: Super Mario Galaxy 2 native macOS port (fan project, no game data)"
pb NSHumanReadableCopyright "Fan project; contains no Nintendo game data. You need your own copy of the game."
pb LSMinimumSystemVersion "$minmacos"
/usr/libexec/PlistBuddy -c "Add :LSApplicationCategoryType string public.app-category.games" "$plist" 2>/dev/null || true
/usr/libexec/PlistBuddy -c "Add :NSHighResolutionCapable bool true" "$plist" 2>/dev/null || true
/usr/libexec/PlistBuddy -c "Add :AstralSourceCommit string $commit" "$plist"
/usr/libexec/PlistBuddy -c "Add :AstralBuildDate string $builddate" "$plist"
/usr/libexec/PlistBuddy -c "Add :AstralExecutableUUID string $exe_uuid" "$plist"
/usr/libexec/PlistBuddy -c "Delete :CFBundleIconFile" "$plist" 2>/dev/null || true
if [ -f "$repo/tools/release/AppIcon.icns" ]; then
    mkdir -p "$bundle/Contents/Resources"
    cp "$repo/tools/release/AppIcon.icns" "$bundle/Contents/Resources/AppIcon.icns"
    pb CFBundleIconFile "AppIcon"
fi
cp "$repo/LICENSE" "$bundle/Contents/Resources/LICENSE.txt"
cp "$repo/tools/release/THIRD-PARTY-NOTICES.txt" "$bundle/Contents/Resources/THIRD-PARTY-NOTICES.txt"

if [ -n "$libroot" ]; then
    python3 -I "$here/bundle_libs.py" "$bundle" --lib-root "$libroot"
else
    python3 -I "$here/bundle_libs.py" "$bundle"
fi

# Ad-hoc signature (Apple Silicon needs a valid one to run at all). A Developer ID signature comes from sign_notarize.sh.
codesign --force --sign - --identifier dev.petari.Petari "$bundle"
codesign --verify --strict "$bundle"

# Gate: no game data, no outside libraries, no minos above the target.
python3 -I "$here/scan_bundle.py" "$bundle" --min-macos "$minmacos"

sed -e "s/__VERSION__/$version/g" -e "s/__COMMIT__/${commit:0:8}/g" -e "s/__MINMACOS__/$minmacos/g" \
    "$here/payload/READ-ME-FIRST.txt" > "$folder/READ ME FIRST.txt"
cp "$here/payload/Collect Bug Report.command" "$folder/Collect Bug Report.command"
chmod +x "$folder/Collect Bug Report.command"

zip="$out/$name.zip"
rm -f "$zip"
(cd "$stage" && ditto -c -k --keepParent "Astral Italian $version" "$zip")
python3 -I "$here/scan_bundle.py" "$zip" --min-macos "$minmacos"

dsymzip="$out/$name.dSYM.zip"
rm -f "$dsymzip"
ditto -c -k --keepParent "$dsym" "$dsymzip"

(cd "$out" && shasum -a 256 "$name.zip" "$name.dSYM.zip" > "$name.sha256")
sha_zip=$(awk -v f="$name.zip" '$2==f{print $1}' "$out/$name.sha256")
libs=$(ls "$bundle/Contents/Frameworks" 2>/dev/null | wc -l | tr -d ' ')
cat > "$out/$name.manifest.json" <<EOF
{
  "version": "$version",
  "source_commit": "$commit",
  "build_date": "$builddate",
  "min_macos": "$minmacos",
  "executable_uuid": "$exe_uuid",
  "bundled_libraries": $libs,
  "frozen_app": "$app",
  "zip": "$name.zip",
  "zip_sha256": "$sha_zip",
  "zip_bytes": $(stat -f %z "$zip"),
  "dsym": "$name.dSYM.zip"
}
EOF
sed -e "s/__VERSION__/$version/g" -e "s/__COMMIT__/$commit/g" -e "s/__MINMACOS__/$minmacos/g" -e "s/__SHA256__/$sha_zip/g" \
    "$here/RELEASE-NOTES.template.md" > "$out/$name.notes.md"
echo "make_release: done"
ls -la "$out/$name".* | awk '{print $5, $9}'
