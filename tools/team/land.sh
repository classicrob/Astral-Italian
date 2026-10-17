#!/bin/sh
# Land the current worktree branch on main: rebase, full build (hash check), fast-forward main, push.
# Run from your lane worktree root with a clean tree (commit first).
set -e
main="$HOME/MacioGalaxy2"
lock="$HOME/MacioGalaxy2-team/land.lock"
branch=$(git rev-parse --abbrev-ref HEAD)
[ -z "$(git status --porcelain --untracked-files=no)" ] || { echo "commit your changes first" >&2; exit 1; }
for attempt in 1 2 3 4 5; do
  git rebase main
  python3 configure.py >/dev/null
  ninja >/dev/null || { echo "build/hash check failed after rebase; fix before landing" >&2; exit 1; }
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
