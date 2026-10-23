#!/bin/bash
# Creates a DRAFT GitHub Release (visible only to repository writers) with the assets make_release.sh produced.
# It never publishes: a person reviews the draft on GitHub and presses Publish.
#
#   tools/release/draft_release.sh out/AstralItalian-0.2.0   (prefix without extension)
set -eu
prefix=${1:?usage: draft_release.sh PATH/AstralItalian-X.Y.Z}
version=$(basename "$prefix"); version=${version#AstralItalian-}
for f in "$prefix.zip" "$prefix.dSYM.zip" "$prefix.sha256" "$prefix.notes.md"; do
    [ -f "$f" ] || { echo "missing $f" >&2; exit 1; }
done
python3 -I "$(dirname "$0")/scan_bundle.py" "$prefix.zip"
gh release create "v$version" --repo classicrob/Astral-Italian --draft --prerelease \
    --title "Astral Italian $version (tester build)" --notes-file "$prefix.notes.md" \
    "$prefix.zip" "$prefix.dSYM.zip" "$prefix.sha256"
echo "Draft created. Review it at https://github.com/classicrob/Astral-Italian/releases and publish by hand."
