#!/bin/sh
# Dispatch mode has its own fail-closed contract; the ordinary path is unchanged.
if [ "${DISPATCH_PROOF:-}" = 1 ]; then
  exec python3 tools/team/dispatchlanding.py hook "$@"
fi
# Prove that source edits leave every Wii object unchanged. Each phase is a CLEAN object build (all
# build/SB4E01/src objects deleted, then the full default build), so headers, .inc files and all their
# dependents are recompiled by construction; the two complete object lists (paths and hashes) must be
# identical. The DOL hash alone is not enough: NonMatching units link the original asm, not our objects.
# A perturbed control file shows the comparison sees real rebuilt objects.
# Usage (from the worktree root; nothing staged; no other writers in this worktree while it runs):
#   tools/team/hookproof.sh [outdir]            uncommitted edits in src/include/libs vs HEAD
#   tools/team/hookproof.sh outdir BASE [HEAD]  committed edits BASE..HEAD (worktree clean at HEAD)
# For a batch without an edited .cpp, explicitly select a tracked dependent .cpp:
#   HOOKPROOF_CONTROL_SOURCE=src/JSystem/JKernel/JKRHeap.cpp tools/team/hookproof.sh [outdir]
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
fi
ctl=; control_active=0
restore_control() {
  [ "$control_active" -eq 1 ] || return 0
  cp "$out/control.bak" "$ctl" && touch "$ctl" || { echo "control source restore failed: $ctl" >&2; return 3; }
  control_active=0
}
control_hash() {
  [ -s "$ctlo" ] || return 1
  digest=$(shasum "$ctlo") || return 1
  printf '%s\n' "${digest%% *}"
}
cleanup() {
  proof_rc=$?
  trap - EXIT
  trap '' INT TERM HUP
  restore_control || proof_rc=3
  [ -n "$base" ] || phase_after
  exit "$proof_rc"
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
trap 'exit 129' HUP
n=$(cat "$out/modified.txt" "$out/added.txt" "$out/deleted.txt" | grep -c .)
[ "$n" -gt 0 ] || { echo "no changed files" >&2; exit 2; }

ctl=$(cat "$out/modified.txt" "$out/added.txt" | grep -E '\.cpp$' | head -1)
if [ -n "${HOOKPROOF_CONTROL_SOURCE:-}" ]; then
  [ -z "$ctl" ] || { echo "control override is only for batches without an edited .cpp" >&2; exit 2; }
  ctl=$(python3 - "$HOOKPROOF_CONTROL_SOURCE" "$out" <<'PY'
import json
from pathlib import Path
import re
import subprocess
import sys

def fail(reason):
    sys.exit("invalid control override: " + reason)

root = Path.cwd().resolve()
source = Path(sys.argv[1])
if not source.is_absolute():
    source = root / source
try:
    relative = source.relative_to(root)
except ValueError:
    fail("source must be inside this worktree")
if not relative.parts or relative.parts[0] != "src" or relative.suffix != ".cpp" or ".." in relative.parts:
    fail("source must be a canonical src/*.cpp path")
if not source.is_file() or any(p.is_symlink() for p in (source, *source.parents)):
    fail("source must be a real regular file without symlink aliases")
tracked = subprocess.run(["git", "ls-files", "--error-unmatch", "--", str(relative)], capture_output=True, text=True)
if tracked.returncode or tracked.stdout.splitlines() != [str(relative)]:
    fail("source must be tracked")
obj = Path("build/SB4E01") / relative.with_suffix(".o")
if obj.is_symlink() or not obj.is_file() or not obj.stat().st_size:
    fail("control object must already exist and be nonempty")
out = Path(sys.argv[2])
result = subprocess.run(["ninja", "-t", "deps", str(obj)], capture_output=True, text=True)
(out / "control-deps.txt").write_text(result.stdout)
lines = result.stdout.rstrip().splitlines()
header = re.fullmatch(re.escape(str(obj)) + r": #deps (\d+), deps mtime \d+ \(VALID\)", lines[0]) if lines else None
if result.returncode or not header:
    fail("Ninja must report a VALID dependency record for the control object")
if len(lines) - 1 != int(header[1]) or any(not line.startswith("    ") or not line[4:] for line in lines[1:]):
    fail("malformed Ninja dependency record")
deps = {Path(line[4:]).resolve() for line in lines[1:]}
if source.resolve() not in deps:
    fail("dependency record does not name the control source")
changed = {Path(f).resolve() for name in ("modified", "added", "deleted")
           for f in (out / (name + ".txt")).read_text().splitlines() if f}
matches = sorted(str(p.relative_to(root)) for p in deps & changed)
if not matches:
    fail("control dependency closure does not intersect changed inputs")
(out / "control-dependency-match.json").write_text(json.dumps(matches, indent=2) + "\n")
print(relative)
PY
  ) || exit 2
fi
[ -n "$ctl" ] || { echo "no edited .cpp: set HOOKPROOF_CONTROL_SOURCE to a tracked dependent source" >&2; exit 2; }
printf '%s\n' "$ctl" > "$out/control-source.txt"

clean_build() {  # $1 = phase name
  find build/SB4E01/src -name '*.o' -delete
  nice -n 10 sh tools/team/nj.sh -j "${HOOKPROOF_JOBS:-4}" -k 0 > "$out/build-$1.log" 2>&1; rc=$?
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

# Control: perturb an edited .cpp or an explicitly validated dependent, then restore it.
ctl_ok=0
if [ -n "$ctl" ]; then
  ctlo="build/SB4E01/${ctl%.cpp}.o"; h0=$(control_hash) || h0=
  cp "$ctl" "$out/control.bak" || exit 3
  control_active=1
  printf '\nextern const int hookproof_control_marker = 0x1234;\n' >> "$ctl" || exit 3
  nice -n 10 ninja -j "${HOOKPROOF_JOBS:-4}" "$ctlo" > "$out/build-control.log" 2>&1; rc_c=$?
  h1=$(control_hash) || h1=
  restore_control || exit 3
  nice -n 10 ninja -j "${HOOKPROOF_JOBS:-4}" "$ctlo" >> "$out/build-control.log" 2>&1; rc_r=$?
  h2=$(control_hash) || h2=
  [ "$rc_c" -eq 0 ] && [ "$rc_r" -eq 0 ] && [ -n "$h0" ] && [ -n "$h1" ] && [ -n "$h2" ] \
    && [ "$h1" != "$h0" ] && [ "$h2" = "$h0" ] && ctl_ok=1
  echo "control $ctl: perturbed object changed=$([ "$h1" != "$h0" ] && echo yes || echo NO), restored identical=$([ "$h2" = "$h0" ] && echo yes || echo NO) (compile rc=$rc_c, restore rc=$rc_r)"
fi
echo "files: $n changed; clean builds: before rc=$rc_b ($nb objects), after rc=$rc_a ($na objects); code-section lines differing: $nd (objects differing only in debug info: $nf)"
if [ "$rc_b" -ne 0 ] || [ "$rc_a" -ne 0 ] || [ "$nb" -lt 2000 ] || [ "$nb" -ne "$na" ] || [ "$nd" -ne 0 ] \
   || [ -z "$ctl" ] || [ "$ctl_ok" -ne 1 ]; then
  echo "PROOF FAILED (see $out/build-*.log, object-diff.txt)"; exit 1
fi
echo "PROOF OK"
