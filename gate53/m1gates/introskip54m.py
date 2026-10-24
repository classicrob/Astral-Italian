#!/usr/bin/env python3
"""Preserve intro-skip evidence; frozen54 cannot inject the reference pre-play host keys."""
import json,re,sys
from common import HERE,ROOT,prep,run,write,trim
app,meta=prep();root=ROOT/'runs'/(sys.argv[1] if len(sys.argv)>1 else 'intro-skip')
r=run(app,root/'none',stage='RedBlueExGalaxy',copied=True,seconds=75,bound=True)
t=(root/'none/app.log').read_text(errors='replace')
ready=[line for line in t.splitlines() if line.startswith('[play ready]')]
result={'passed':False,'checks':{'no_press_play_ready':bool(ready),'skip_earlier_than_no_press':False},'no_press_play_ready':ready,'process':r,'frozen_build':meta,'nice':0,'blocked':'Reference introskip53.py uses CGEvent Space every 0.4s before play-ready. PETARI_INPUT_SCRIPT starts only after play-ready. No host input used. A canonical intro-skip tape and candidate full-session replay are required.'}
write(root/'RESULT.json',result);trim(root/'none');print(json.dumps(result));sys.exit(1)
