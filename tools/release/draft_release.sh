#!/bin/bash
# Creates, or respins, a DRAFT GitHub Release (visible only to repository writers) with the assets make_release.sh produced.
# Run again with a new build under the same version to replace a draft's assets and notes; published releases are refused.
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
# Tag the exact source commit the app was built from (a draft's tag is created only when it is published).
commit=$(python3 -I -c 'import json,sys; print(json.load(open(sys.argv[1]))["source_commit"])' "$prefix.manifest.json")
if gh release view "v$version" --repo classicrob/Astral-Italian >/dev/null 2>&1; then
    # Respin: only a still-unpublished draft may be replaced; a published release is never touched.
    [ "$(gh release view "v$version" --repo classicrob/Astral-Italian --json isDraft --jq .isDraft)" = true ] \
        || { echo "v$version is already published; choose a new version" >&2; exit 1; }
    for name in $(gh release view "v$version" --repo classicrob/Astral-Italian --json assets --jq '.assets[].name'); do
        gh release delete-asset "v$version" "$name" --repo classicrob/Astral-Italian --yes
    done
    gh release edit "v$version" --repo classicrob/Astral-Italian --draft --prerelease --target "$commit" \
        --title "Astral Italian $version (tester build)" --notes-file "$prefix.notes.md"
    gh release upload "v$version" "$prefix.zip" "$prefix.dSYM.zip" "$prefix.sha256" --repo classicrob/Astral-Italian
else
    gh release create "v$version" --repo classicrob/Astral-Italian --draft --prerelease --target "$commit" \
        --title "Astral Italian $version (tester build)" --notes-file "$prefix.notes.md" \
        "$prefix.zip" "$prefix.dSYM.zip" "$prefix.sha256"
fi
gh release view "v$version" --repo classicrob/Astral-Italian --json url,isDraft --jq '"\(.url) draft=\(.isDraft)"'
echo "Draft ready. Review it at https://github.com/classicrob/Astral-Italian/releases and publish by hand."
