#!/bin/sh
# Prove that native-hook edits leave the Wii build unchanged: build every object without the edits, then
# with them, and compare each object file byte for byte. The DOL hash alone is not enough because
# NonMatching units link the original asm, not our objects.
# Usage (run from the worktree root):
#   tools/team/hookproof.sh [outdir]            uncommitted edits in src/include/libs vs HEAD
#   tools/team/hookproof.sh outdir BASE [HEAD]  committed edits: BASE..HEAD (worktree must be clean there)
# Every input file of each phase is touched before its build, and the script FAILS unless each phase
# recompiled every object that depends on a changed file (it checks that the touched files' objects are newer
# than the phase start), so a stale build can never pass as a proof.
set -u
out=${1:-build/hookproof}
base=${2:-}
head=${3:-HEAD}
rm -rf "$out"; mkdir -p "$out/saved"
if [ -n "$base" ]; then
  git diff --name-only --diff-filter=M "$base" "$head" -- src include libs > "$out/modified.txt"
  git diff --name-only --diff-filter=A "$base" "$head" -- src include libs > "$out/added.txt"
  while read -r f; do mkdir -p "$out/saved/$(dirname "$f")"; git show "$head:$f" > "$out/saved/$f"; done \
    < "$out/modified.txt"
  while read -r f; do mkdir -p "$out/saved/$(dirname "$f")"; git show "$head:$f" > "$out/saved/$f"; done \
    < "$out/added.txt"
  before() { while read -r f; do git show "$base:$f" > "$f"; done < "$out/modified.txt"; }
else
  git diff --name-only -- src include libs > "$out/modified.txt"
  git ls-files --others --exclude-standard -- src include libs > "$out/added.txt"
  cat "$out/modified.txt" "$out/added.txt" | while read -r f; do
    mkdir -p "$out/saved/$(dirname "$f")"; cp "$f" "$out/saved/$f"
  done
  before() { git checkout -- $(cat "$out/modified.txt"); }
fi
[ -s "$out/modified.txt" ] || [ -s "$out/added.txt" ] || { echo "no changed files" >&2; exit 2; }
hash_objs() {
  find build/SB4E01/src -name '*.o' -print0 | xargs -0 shasum | sed 's|  build/SB4E01/| |' | sort -k2 > "$1"
}
touch_inputs() { cat "$out/modified.txt" "$out/added.txt" | while read -r f; do [ -f "$f" ] && touch "$f"; done; }
fresh_count() {  # objects newer than the phase start marker
  find build/SB4E01/src -name '*.o' -newer "$1" | wc -l | tr -d ' '
}
direct_objs() {  # objects of the changed .c/.cpp files (each must be rebuilt in each phase)
  cat "$out/modified.txt" "$out/added.txt" | grep -E '\.(c|cpp)$' | sed -E 's/\.(c|cpp)$/.o/' | while read -r f; do
    [ -f "build/SB4E01/$f" ] && echo "build/SB4E01/$f"; done
}
check_direct() {  # every changed source's object must be newer than the phase marker
  stale=0
  for o in $(direct_objs); do [ "$o" -nt "$1" ] || { echo "STALE $o" >> "$out/stale.txt"; stale=$((stale + 1)); }; done
  echo "$stale"
}

# Before: without the edits.
before
while read -r f; do rm -f "$f"; done < "$out/added.txt"
touch_inputs
sleep 1; touch "$out/mark-before"; sleep 1
nice -n 10 sh tools/team/nj.sh -k 0 > "$out/build-before.log" 2>&1; rc_b=$?
hash_objs "$out/before.sha"
fb=$(fresh_count "$out/mark-before"); sb=$(check_direct "$out/mark-before")
# After: with the edits.
(cd "$out/saved" && find . -type f) | while read -r f; do mkdir -p "$(dirname "$f")"; cp "$out/saved/$f" "$f"; done
touch_inputs
sleep 1; touch "$out/mark-after"; sleep 1
nice -n 10 sh tools/team/nj.sh -k 0 > "$out/build-after.log" 2>&1; rc_a=$?
hash_objs "$out/after.sha"
fa=$(fresh_count "$out/mark-after"); sa=$(check_direct "$out/mark-after")
join -1 2 -2 2 "$out/before.sha" "$out/after.sha" | awk '$2 != $3 {print $1}' > "$out/changed-objects.txt"
nb=$(wc -l < "$out/before.sha" | tr -d ' '); na=$(wc -l < "$out/after.sha" | tr -d ' ')
nd=$(wc -l < "$out/changed-objects.txt" | tr -d ' ')
echo "before rc=$rc_b rebuilt=$fb stale-direct=$sb; after rc=$rc_a rebuilt=$fa stale-direct=$sa"
echo "objects: $nb before, $na after, $nd differ"
if [ $rc_b -ne 0 ] || [ $rc_a -ne 0 ] || [ "$sb" -ne 0 ] || [ "$sa" -ne 0 ] || [ "$fa" -eq 0 ] || [ "$nb" -ne "$na" ] \
   || [ "$nd" -ne 0 ]; then
  echo "PROOF FAILED (see $out/build-*.log, stale.txt, changed-objects.txt)"; exit 1
fi
echo "PROOF OK"
