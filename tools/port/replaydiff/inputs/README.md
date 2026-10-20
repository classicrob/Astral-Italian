# Filtered input scripts

`galaxy-route.json` is the current exploratory SB4E01 navigation script, schema
v1 documented in the parent README. It is not yet a qualified galaxy fixture.
Its frames index the caller-filtered GameSystem loop seam, not VI or movie polls.
Only deliberate controller buttons, normalized pointer/stick values, acceleration and timing
belong here. No game assets, saves, RAM, screenshots or recordings belong here.

Run through `dolphin_collect.py --input-script`; use the resulting successful
`inputs.jsonl` as the native harness input. This preserves the actual ordered
KPAD calls rather than inventing a per-frame read count. The native sample
encoder is shared with the injector. Freeze the script before canonical repeats.

`stage-start.json` is the proposed 300-loop common gameplay suffix: idle0..89,
walk90..119 (stick0.3,0), idle120..149, jump A150..161, idle162..219,
core acceleration pulse220..244, idle245..299. It has no pointer. This is
controller intent, not a qualified capture. `acc` is the raw filtered KPAD
three-axis acceleration in g; baseline[0,-1,0], pulse[3,-1,0]. A25-frame
pulse avoids a second separated distance-swing edge in the game's20-sample
history under one read/update per loop; actual jump/spin execution still
requires playable-stage verification. Native/Dolphin prefixes remain explicit
and independently timed. Suffix frame0 is the first input loop AFTER sync
row0; append it at source_frame+1, retaining the idle row0 input.
