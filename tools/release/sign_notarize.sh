#!/bin/bash
# NOT RUN by the release process until the project has an Apple Developer ID (a paid Apple Developer account).
# Signs the bundle inside a make_release.sh zip with a Developer ID Application certificate and the hardened
# runtime, submits it to Apple's notary service, staples the ticket and rebuilds the zip.
#
#   SIGN_IDENTITY="Developer ID Application: Name (TEAMID)" NOTARY_PROFILE=astral-notary \
#       tools/release/sign_notarize.sh out/AstralItalian-0.2.0.zip
#
# One-time setup of NOTARY_PROFILE (stores credentials in your keychain, never in the repo):
#   xcrun notarytool store-credentials astral-notary --apple-id you@example.com --team-id TEAMID
set -eu
here=$(cd "$(dirname "$0")" && pwd)
zip=${1:?usage: sign_notarize.sh PATH/AstralItalian-X.Y.Z.zip}
: "${SIGN_IDENTITY:?set SIGN_IDENTITY to the Developer ID Application certificate name}"
: "${NOTARY_PROFILE:?set NOTARY_PROFILE to a notarytool keychain profile}"
work=$(dirname "$zip")/notarize-work
case "$work" in */notarize-work) ;; *) exit 1 ;; esac
[ ! -e "$work" ] || rm -rf "$work"
mkdir -p "$work"
ditto -x -k "$zip" "$work"
app=$(find "$work" -maxdepth 2 -name "Astral Italian.app" -type d | head -n1)
[ -n "$app" ] || { echo "no Astral Italian.app inside $zip" >&2; exit 1; }
# Innermost first: libraries, then the executable, then the bundle.
for lib in "$app"/Contents/Frameworks/*.dylib; do
    codesign --force --timestamp --options runtime --sign "$SIGN_IDENTITY" "$lib"
done
codesign --force --timestamp --options runtime --entitlements "$here/entitlements.plist" --sign "$SIGN_IDENTITY" "$app"
codesign --verify --deep --strict --verbose=2 "$app"
submit="$work/submit.zip"
ditto -c -k --keepParent "$app" "$submit"
xcrun notarytool submit "$submit" --keychain-profile "$NOTARY_PROFILE" --wait
xcrun stapler staple "$app"
xcrun stapler validate "$app"
spctl --assess --type execute --verbose=2 "$app"
folder=$(dirname "$app")
signed="${zip%.zip}-notarized.zip"
(cd "$(dirname "$folder")" && ditto -c -k --keepParent "$(basename "$folder")" "$signed")
shasum -a 256 "$signed" | tee "${signed%.zip}.sha256"
rm -rf "$work"
echo "notarized package: $signed"
