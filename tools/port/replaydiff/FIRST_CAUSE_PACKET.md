# Offline first-cause packet

`first_cause_packet.py` checks a pinned acceptance receipt and every referenced
artifact before and after analysis. It independently scans the complete trace
pair for each field's first raw difference, retaining the declared exclusions.
The default-tolerance result is explicitly taken from the pinned acceptance,
not relabelled as a newly independent tolerance calculation.

```sh
nice -n 19 python3 tools/port/replaydiff/first_cause_packet.py \
  --root /private/completed-pair --out /private/fresh-packet \
  --acceptance-sha256 ACCEPTANCE_SHA256 \
  --source-repo /path/to/source --original-source ORIGINAL_COMMIT
```

The output contains all columns for frames 880–910, first differences over the
complete traces, state changes within the window, complete input records for
that window, and a compressed byte-offset index for **every** read group in
both the full export and normalized suffix. Zero/error returns retain slot-zero
sample history, as required by the existing headless tape contract. The exact
window records must agree between the full export and suffix after changing
only the frame label. Each index also hashes all bytes preceding the window.
Complete files stay in place, are hash-bound, and remain necessary evidence;
the window is not a standalone replay or equivalent starting state.

Optional source inventory pins both commits, selected source blob hashes and
the complete native/src/include/libs diff. It supplies source evidence, not an
inference about the original executable's compiler flags or runtime FP mode.

Output is private evidence. Do not commit generated input records, trace rows
or paths to game assets. A receipt remains BLOCKED with null qualified counts;
first recorded disagreement does not establish a source defect or its cause.

Run focused controls with:

```sh
python3 -m unittest discover -s tools/port/replaydiff -p test_first_cause_packet.py
```
