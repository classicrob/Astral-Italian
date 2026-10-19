#!/bin/sh
# Prove that uncommitted native-hook edits leave the Wii build unchanged: build every object with the
# edits reverted, then with them, and compare each object file byte for byte. The DOL hash alone is
# not enough because NonMatching units link the original asm, not our objects.
# Usage: tools/team/hookproof.sh [outdir]   (run from the worktree root, with the hook edits uncommitted)
set -u
out=${1:-build/hookproof}
rm -rf "$out"; mkdir -p "$out/saved"
git diff --name-only -- src include libs > "$out/modified.txt"
git ls-files --others --exclude-standard -- src include libs > "$out/added.txt"
cat "$out/modified.txt" "$out/added.txt" | while read -r f; do
  mkdir -p "$out/saved/$(dirname "$f")"; cp -p "$f" "$out/saved/$f"
done
hash_objs() {
  find build/SB4E01/src -name '*.o' -print0 | xargs -0 shasum | sed 's|  build/SB4E01/| |' | sort -k2 > "$1"
}
# Before: the committed sources.
git checkout -- $(cat "$out/modified.txt")
while read -r f; do rm -f "$f"; done < "$out/added.txt"
nice -n 10 sh tools/team/nj.sh -k 0 > "$out/build-before.log" 2>&1; echo "before rc=$?" > "$out/rc.txt"
hash_objs "$out/before.sha"
# After: the hooked sources.
(cd "$out/saved" && find . -type f) | while read -r f; do cp -p "$out/saved/$f" "$f"; done
nice -n 10 sh tools/team/nj.sh -k 0 > "$out/build-after.log" 2>&1; echo "after rc=$?" >> "$out/rc.txt"
hash_objs "$out/after.sha"
join -1 2 -2 2 "$out/before.sha" "$out/after.sha" | awk '$2 != $3 {print $1}' > "$out/changed-objects.txt"
cat "$out/rc.txt"
echo "objects: $(wc -l < "$out/before.sha") before, $(wc -l < "$out/after.sha") after, $(wc -l < "$out/changed-objects.txt") differ"
