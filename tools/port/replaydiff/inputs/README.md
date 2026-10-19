# Filtered input scripts

`galaxy-route.json` is the current exploratory SB4E01 navigation script, schema
v1 documented in the parent README. It is not yet a qualified galaxy fixture.
Its frames index the caller-filtered GameSystem loop seam, not VI or movie polls.
Only deliberate controller buttons, normalized pointer/stick values and timing
belong here. No game assets, saves, RAM, screenshots or recordings belong here.

Run through `dolphin_collect.py --input-script`; use the resulting successful
`inputs.jsonl` as the native harness input. This preserves the actual ordered
KPAD calls rather than inventing a per-frame read count. The native sample
encoder is shared with the injector. Freeze the script before canonical repeats.
