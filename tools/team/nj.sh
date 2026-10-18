#!/bin/sh
# Locked build for a worktree shared by several agents: configure.py + ninja under build/.build.lock
# (the same lock sg.py takes before re-running configure.py). Recovers a stale lock (dead pid) and a
# corrupted .ninja_deps ("manifest still dirty" loop). Usage: tools/team/nj.sh [ninja args...]
lock=build/.build.lock
mkdir -p build
while ! mkdir "$lock" 2>/dev/null; do
  pid=$(cat "$lock/pid" 2>/dev/null)
  if [ -n "$pid" ] && ! kill -0 "$pid" 2>/dev/null; then rm -rf "$lock"; continue; fi
  sleep 1
done
echo $$ > "$lock/pid"
trap 'rm -rf "$lock"' EXIT INT TERM
python3 configure.py >/dev/null || exit 1
out=$(ninja "$@" 2>&1); rc=$?
if [ $rc -ne 0 ] && printf '%s\n' "$out" | grep -q "still dirty"; then
  rm -f .ninja_deps .ninja_log
  out=$(ninja "$@" 2>&1); rc=$?
fi
[ $rc -eq 0 ] || printf '%s\n' "$out" | tail -40
exit $rc
