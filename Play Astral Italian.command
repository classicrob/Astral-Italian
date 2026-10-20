#!/bin/sh
# Double-click to play Astral Italian (native Super Mario Galaxy 2). See PLAY.md.
# Without arguments: the menu (galaxy, Odyssey movement on/off, audio on/off).
# With arguments they go to native/play/play.sh, e.g.  ./"Play Astral Italian.command" fluffy-bluff --odyssey
here=$(cd "$(dirname "$0")" && pwd)
if [ $# -gt 0 ]; then
    exec sh "$here/native/play/play.sh" "$@"
fi
exec sh "$here/native/play/Play Astral Italian.command"
