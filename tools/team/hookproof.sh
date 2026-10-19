#!/bin/sh
# Prove that source edits leave every Wii object unchanged. Each phase is a CLEAN object build (all
# build/SB4E01/src objects deleted, then the full default build), so headers, .inc files and all their
# dependents are recompiled by construction; the two complete object lists (paths and hashes) must be
# identical. The DOL hash alone is not enough: NonMatching units link the original asm, not our objects.
# A perturbed control file shows the comparison sees real rebuilt objects.
# Usage (from the worktree root; nothing staged; no other writers in this worktree while it runs):
#   tools/team/hookproof.sh [outdir]            uncommitted edits in src/include/libs vs HEAD
#   tools/team/hookproof.sh outdir BASE [HEAD]  committed edits BASE..HEAD (worktree clean at HEAD)
set -u
out=${1:-build/hookproof}
base=${2:-}
head=${3:-HEAD}
case "$out" in src*|include*|libs*|/) echo "bad outdir" >&2; exit 2;; esac
[ -z "$(git diff --cached --name-only)" ] || { echo "staged changes: unstage or commit first" >&2; exit 2; }
rm -rf "$out"; mkdir -p "$out/saved"
if [ -n "$base" ]; then
  [ -z "$(git status --porcelain --untracked-files=no -- src include libs)" ] || { echo "worktree not clean" >&2; exit 2; }
  base=$(git rev-parse "$base") && head=$(git rev-parse "$head") || exit 2
  git diff --name-status --no-renames "$base" "$head" -- src include libs > "$out/status.txt"
  grep -vE '^[MAD]	' "$out/status.txt" && { echo "unsupported change types" >&2; exit 2; }
  awk -F'\t' '$1=="M"{print $2}' "$out/status.txt" > "$out/modified.txt"
  awk -F'\t' '$1=="A"{print $2}' "$out/status.txt" > "$out/added.txt"
  awk -F'\t' '$1=="D"{print $2}' "$out/status.txt" > "$out/deleted.txt"
  phase_before() {
    while read -r f; do git show "$base:$f" > "$f" || exit 3; done < "$out/modified.txt"
    while read -r f; do mkdir -p "$(dirname "$f")"; git show "$base:$f" > "$f" || exit 3; done < "$out/deleted.txt"
    while read -r f; do rm -f "$f"; done < "$out/added.txt"
  }
  phase_after() {
    git checkout "$head" -- src include libs || exit 3
    while read -r f; do rm -f "$f"; done < "$out/deleted.txt"
  }
else
  git diff --name-status --no-renames HEAD -- src include libs > "$out/status.txt"
  grep -vE '^[MD]	' "$out/status.txt" && { echo "unsupported change types" >&2; exit 2; }
  awk -F'\t' '$1=="M"{print $2}' "$out/status.txt" > "$out/modified.txt"
  awk -F'\t' '$1=="D"{print $2}' "$out/status.txt" > "$out/deleted.txt"
  git ls-files --others --exclude-standard -- src include libs > "$out/added.txt"
  cat "$out/modified.txt" "$out/added.txt" | while read -r f; do
    mkdir -p "$out/saved/$(dirname "$f")"; cp "$f" "$out/saved/$f" || exit 3
  done
  phase_before() {
    [ -s "$out/modified.txt" ] && { git checkout HEAD -- $(cat "$out/modified.txt") || exit 3; }
    [ -s "$out/deleted.txt" ] && { git checkout HEAD -- $(cat "$out/deleted.txt") || exit 3; }
    while read -r f; do rm -f "$f"; done < "$out/added.txt"
  }
  phase_after() {
    (cd "$out/saved" && find . -type f) | while read -r f; do mkdir -p "$(dirname "$f")"; cp "$out/saved/$f" "$f"; done
    while read -r f; do rm -f "$f"; done < "$out/deleted.txt"
  }
  # Restore the proposed tree whatever happens.
  trap 'phase_after' EXIT INT TERM HUP
fi
n=$(cat "$out/modified.txt" "$out/added.txt" "$out/deleted.txt" | grep -c .)
[ "$n" -gt 0 ] || { echo "no changed files" >&2; exit 2; }

clean_build() {  # $1 = phase name
  find build/SB4E01/src -name '*.o' -delete
  nice -n 10 sh tools/team/nj.sh -j 4 -k 0 > "$out/build-$1.log" 2>&1; rc=$?
  find build/SB4E01/src -name '*.o' -print0 | xargs -0 shasum | sed 's|  build/SB4E01/| |' | sort -k2 > "$out/$1.full.sha"
  python3 tools/team/objhash.py build/SB4E01/src > "$out/$1.sha" || echo 99
  echo "$rc"
}
phase_before; rc_b=$(clean_build before)
phase_after;  rc_a=$(clean_build after)
nb=$(grep -c . "$out/before.sha"); na=$(grep -c . "$out/after.sha")
# Compare code/data/relocations/symbols (tools/team/objhash.py skips the DWARF line/debug sections, which change
# whenever source lines move); full-file differences are reported for information.
diff "$out/before.sha" "$out/after.sha" > "$out/object-diff.txt"; nd=$(grep -c '^[<>]' "$out/object-diff.txt")
nf=$(diff "$out/before.full.sha" "$out/after.full.sha" | grep -c '^<')

# Control: perturb one edited .cpp so that its object MUST change, then restore it.
ctl=$(grep -E '\.cpp$' "$out/modified.txt" "$out/added.txt" | head -1 | cut -d: -f2); ctl_ok=0
if [ -n "$ctl" ]; then
  ctlo="build/SB4E01/${ctl%.cpp}.o"; h0=$(shasum "$ctlo" | cut -d' ' -f1)
  cp "$ctl" "$out/control.bak"; printf '\nextern const int hookproof_control_marker = 0x1234;\n' >> "$ctl"
  nice -n 10 ninja -j 4 "$ctlo" > "$out/build-control.log" 2>&1
  h1=$(shasum "$ctlo" | cut -d' ' -f1)
  cp "$out/control.bak" "$ctl"; touch "$ctl"
  nice -n 10 ninja -j 4 "$ctlo" >> "$out/build-control.log" 2>&1
  h2=$(shasum "$ctlo" | cut -d' ' -f1)
  [ "$h1" != "$h0" ] && [ "$h2" = "$h0" ] && ctl_ok=1
  echo "control $ctl: perturbed object changed=$([ "$h1" != "$h0" ] && echo yes || echo NO), restored identical=$([ "$h2" = "$h0" ] && echo yes || echo NO)"
fi
echo "files: $n changed; clean builds: before rc=$rc_b ($nb objects), after rc=$rc_a ($na objects); code-section lines differing: $nd (objects differing only in debug info: $nf)"
if [ "$rc_b" -ne 0 ] || [ "$rc_a" -ne 0 ] || [ "$nb" -lt 2000 ] || [ "$nb" -ne "$na" ] || [ "$nd" -ne 0 ] \
   || [ -z "$ctl" ] || [ "$ctl_ok" -ne 1 ]; then
  echo "PROOF FAILED (see $out/build-*.log, object-diff.txt)"; exit 1
fi
echo "PROOF OK"
