#!/bin/sh
# Land the current worktree branch on main: take the landing lock FIRST, then rebase, full build (hash check),
# fast-forward main, push, release. Holding the lock through the build means main cannot move underneath you,
# so there are no rebuild/retry loops; other landers queue and each lands after exactly one build.
# Files whose content is unchanged by the rebase get their old mtimes back, so ninja only rebuilds what changed.
# Run from your lane worktree root with a clean tree (commit first).
set -e
main="${LAND_MAIN:-$HOME/MacioGalaxy2}"
lock="$HOME/MacioGalaxy2-team/land.lock"
branch=$(git rev-parse --abbrev-ref HEAD)
[ -z "$(git status --porcelain --untracked-files=no)" ] || { echo "commit your changes first" >&2; exit 1; }

# A caller (e.g. safe_land.py) may already hold the lock for its whole apply+build+guard+land sequence:
# it sets LAND_LOCK_HELD=<pid written in land.lock/pid>, and then land.sh neither acquires nor releases the lock.
held_by_caller=""
if [ -n "$LAND_LOCK_HELD" ] && [ "$(cat "$lock/pid" 2>/dev/null)" = "$LAND_LOCK_HELD" ]; then
  held_by_caller=1
else
  python3 tools/team/landlock.py acquire "$branch" >/dev/null   # fair FIFO queue; holds the lock for this shell ($$)
fi
stamps=$(mktemp)
if [ -n "$held_by_caller" ]; then trap 'rm -f "$stamps"' EXIT INT TERM HUP
else trap 'rm -rf "$lock"; rm -f "$stamps"' EXIT INT TERM HUP; fi

git diff --name-only main...HEAD | while read -r f; do
  if [ -f "$f" ]; then
    echo "$(git hash-object "$f") $(stat -f %m "$f") $f" >> "$stamps"
  fi
done
if ! git rebase main; then
  git rebase --abort
  echo "REBASE CONFLICT against main: nothing landed. Rebase your branch onto main yourself (resolve), then land again." >&2
  exit 1
fi
while read -r h m f; do
  if [ -f "$f" ] && [ "$(git hash-object "$f")" = "$h" ]; then
    touch -t "$(date -r "$m" +%Y%m%d%H%M.%S)" "$f"
  fi
done < "$stamps"
# Never trust the worktree's mtimes for the branch's OWN changes (2026-10-19: a cp -p restore left hooked
# sources older than their objects, so ninja skipped them and a broken landing passed): touch every file this
# branch changes so it is really recompiled. Files only changed by main keep the restored mtimes above.
git diff --name-only main...HEAD -- src include libs | while read -r f; do [ -f "$f" ] && touch "$f"; done
for script in native/play/*.sh native/play/*.command "Play Astral Italian.command"; do
  [ -f "$script" ] || continue
  sh -n "$script" || { echo "launcher syntax check failed: $script" >&2; exit 1; }
done
python3 tools/team/classdup.py || { echo "duplicate class definitions (ODR): fix before landing" >&2; exit 1; }
gate_mode=$(python3 - <<'PYGATE'
import subprocess


def docs_only(path):
    if path.startswith(("src/", "include/", "libs/", "build/")):
        return False
    if path in ("Play Astral Italian.command", "native/play/Play Astral Italian.command"):
        return True
    parts = path.split("/")
    return path.endswith(".md") or (len(parts) == 3 and parts[:2] == ["native", "play"] and path.endswith(".sh"))


paths = subprocess.check_output(["git", "diff", "--no-renames", "--name-only", "-z", "main...HEAD"]).split(b"\0")
print("docs" if all(docs_only(path.decode("utf-8", errors="strict")) for path in paths if path) else "full")
PYGATE
) || exit 1
if [ "$gate_mode" = docs ]; then
  echo "launcher/docs-only change: syntax and class checks passed; Wii build unchanged"
else
  tools/team/nj.sh -j "${TEAM_BUILD_JOBS:-6}" >/dev/null || { echo "build/hash check failed after rebase; fix before landing" >&2; exit 1; }
  # crossed-dispatch oracle: every vtable slot / nerve binding of the freshly built objects against the original DOL;
  # new CROSSED/WRONG/CONFLICT rows (or a stale allow-list line after a fix) block the landing
  python3 tools/team/dispatchcheck.py --quiet --allow tools/team/dispatchcheck-allow.txt --strict \
    || { echo "dispatch oracle failed: a binding differs from the original DOL (see rows above); update tools/team/dispatchcheck-allow.txt only for reviewed rows" >&2; exit 1; }
fi
git -C "$main" merge --ff-only -q "$branch"
# cache the report of the exact main we just built, so lanes can use it as a regression baseline
# (safe_land.py: skip the separate "build plain main" step when ~/MacioGalaxy2-team/main-report/<main sha>.json exists)
if [ "$gate_mode" = full ]; then
mkdir -p "$HOME/MacioGalaxy2-team/main-report"
cp build/SB4E01/report.json "$HOME/MacioGalaxy2-team/main-report/$(git rev-parse HEAD).json" 2>/dev/null || true
ls -t "$HOME/MacioGalaxy2-team/main-report"/*.json 2>/dev/null | tail -n +9 | xargs rm -f 2>/dev/null || true
fi
git -C "$main" push -q origin main || echo "push failed (landed locally)" >&2
echo "landed $branch at $(git rev-parse --short HEAD)"
