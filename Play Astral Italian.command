#!/bin/sh
# Double-click to play Astral Italian (native Super Mario Galaxy 2). See PLAY.md.
# Default frozen build: 5.4.2; 5.4.1, 5.3.2 and 5.2.1 with Cappy stay available in the menu (5.4 and 5.1 too).
# Without arguments: the menu (galaxy, Odyssey movement on/off, audio on/off).
# With arguments they go to native/play/play.sh, e.g.  ./"Play Astral Italian.command" fluffy-bluff --odyssey
here=$(cd "$(dirname "$0")" && pwd)
if [ $# -gt 0 ]; then
    exec sh "$here/native/play/play.sh" "$@"
fi
exec sh "$here/native/play/Play Astral Italian.command"
