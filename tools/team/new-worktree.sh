#!/bin/sh
# Create a lane worktree with the game dump and tools linked in, then do a first build.
#   tools/team/new-worktree.sh <lane>     -> ~/MacioGalaxy2-wt/<lane> on branch wt/<lane>
set -e
lane="$1"
[ -n "$lane" ] || { echo "usage: $0 <lane>" >&2; exit 1; }
main="$HOME/MacioGalaxy2"
wt="$HOME/MacioGalaxy2-wt/$lane"
avail_gb=$(df -g / | awk 'NR==2{print $4}')
if [ "$avail_gb" -lt 35 ]; then echo "only ${avail_gb} GB free; floor is 33 GB" >&2; exit 1; fi
git -C "$main" worktree add -b "wt/$lane" "$wt" main
mkdir -p "$wt/orig/SB4E01" "$wt/build"
ln -s "$main/orig/SB4E01/sys" "$wt/orig/SB4E01/sys"
cp -R "$main/build/tools" "$wt/build/tools"
cd "$wt"
python3 configure.py >/dev/null
ninja >/dev/null
touch "$wt/.metadata_never_index" "$wt/build/.metadata_never_index"
echo "ready: $wt"
