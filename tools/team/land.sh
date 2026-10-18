#!/bin/sh
# Land the current worktree branch on main: rebase, full build (hash check), fast-forward main, push.
# Run from your lane worktree root with a clean tree (commit first).
# Rebuild avoidance: after a rebase, files whose content is unchanged get their old mtimes back (so ninja doesn't
# rebuild them), and on a retry the build is skipped entirely if main only moved in non-build files (docs etc.).
set -e
main="$HOME/MacioGalaxy2"
lock="$HOME/MacioGalaxy2-team/land.lock"
branch=$(git rev-parse --abbrev-ref HEAD)
[ -z "$(git status --porcelain --untracked-files=no)" ] || { echo "commit your changes first" >&2; exit 1; }
stamps=$(mktemp)
trap 'rm -f "$stamps"' EXIT
built_main=""
for attempt in 1 2 3 4 5; do
  # remember content hash + mtime of every file this branch changes
  : > "$stamps"
  git diff --name-only main...HEAD | while read -r f; do
    [ -f "$f" ] && echo "$(git hash-object "$f") $(stat -f %m "$f") $f" >> "$stamps"
  done
  git rebase main
  while read -r h m f; do
    [ -f "$f" ] && [ "$(git hash-object "$f")" = "$h" ] && touch -t "$(date -r "$m" +%Y%m%d%H%M.%S)" "$f"
  done < "$stamps"
  cur_main=$(git rev-parse main)
  if [ -n "$built_main" ] && [ -z "$(git diff --name-only "$built_main" "$cur_main" -- src include libs config configure.py tools/project.py)" ]; then
    echo "main moved only in non-build files; skipping rebuild" >&2
  else
    tools/team/nj.sh >/dev/null || { echo "build/hash check failed after rebase; fix before landing" >&2; exit 1; }
  fi
  built_main=$cur_main
  until mkdir "$lock" 2>/dev/null; do sleep 2; done
  if git -C "$main" merge --ff-only -q "$branch" 2>/dev/null; then
    rmdir "$lock"
    git -C "$main" push -q origin main || echo "push failed (landed locally)" >&2
    echo "landed $branch at $(git rev-parse --short HEAD)"
    exit 0
  fi
  rmdir "$lock"
  echo "main moved; retrying ($attempt)" >&2
done
exit 1
