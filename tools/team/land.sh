#!/bin/sh
# Land the current worktree branch on main: take the landing lock FIRST, then rebase, full build (hash check),
# fast-forward main, push, release. Holding the lock through the build means main cannot move underneath you,
# so there are no rebuild/retry loops; other landers queue and each lands after exactly one build.
# Files whose content is unchanged by the rebase get their old mtimes back, so ninja only rebuilds what changed.
# Run from your lane worktree root with a clean tree (commit first).
set -e
main="$HOME/MacioGalaxy2"
lock="$HOME/MacioGalaxy2-team/land.lock"
branch=$(git rev-parse --abbrev-ref HEAD)
[ -z "$(git status --porcelain --untracked-files=no)" ] || { echo "commit your changes first" >&2; exit 1; }

# A caller (e.g. safe_land.py) may already hold the lock for its whole apply+build+guard+land sequence:
# it sets LAND_LOCK_HELD=<pid written in land.lock/pid>, and then land.sh neither acquires nor releases the lock.
held_by_caller=""
if [ -n "$LAND_LOCK_HELD" ] && [ "$(cat "$lock/pid" 2>/dev/null)" = "$LAND_LOCK_HELD" ]; then
  held_by_caller=1
else
  waited=0
  until mkdir "$lock" 2>/dev/null; do
    holder=$(cat "$lock/pid" 2>/dev/null)
    if [ -n "$holder" ] && ! kill -0 "$holder" 2>/dev/null; then rm -rf "$lock"; continue; fi
    [ $waited -eq 0 ] && echo "waiting for landing lock (held by $(cat "$lock/who" 2>/dev/null))" >&2
    waited=1
    sleep 5
  done
  echo $$ > "$lock/pid"
  echo "$branch" > "$lock/who"
fi
stamps=$(mktemp)
if [ -n "$held_by_caller" ]; then trap 'rm -f "$stamps"' EXIT INT TERM HUP
else trap 'rm -rf "$lock"; rm -f "$stamps"' EXIT INT TERM HUP; fi

git diff --name-only main...HEAD | while read -r f; do
  [ -f "$f" ] && echo "$(git hash-object "$f") $(stat -f %m "$f") $f" >> "$stamps"
done
git rebase main
while read -r h m f; do
  [ -f "$f" ] && [ "$(git hash-object "$f")" = "$h" ] && touch -t "$(date -r "$m" +%Y%m%d%H%M.%S)" "$f"
done < "$stamps"
tools/team/nj.sh >/dev/null || { echo "build/hash check failed after rebase; fix before landing" >&2; exit 1; }
git -C "$main" merge --ff-only -q "$branch"
# cache the report of the exact main we just built, so lanes can use it as a regression baseline
# (safe_land.py: skip the separate "build plain main" step when ~/MacioGalaxy2-team/main-report/<main sha>.json exists)
mkdir -p "$HOME/MacioGalaxy2-team/main-report"
cp build/SB4E01/report.json "$HOME/MacioGalaxy2-team/main-report/$(git rev-parse HEAD).json" 2>/dev/null || true
ls -t "$HOME/MacioGalaxy2-team/main-report"/*.json 2>/dev/null | tail -n +31 | xargs rm -f 2>/dev/null || true
git -C "$main" push -q origin main || echo "push failed (landed locally)" >&2
echo "landed $branch at $(git rev-parse --short HEAD)"
