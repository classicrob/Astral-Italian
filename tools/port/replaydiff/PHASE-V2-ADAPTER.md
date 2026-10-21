# Proposed phase-v2 content-pair adapter

This is a separate proposed content contract, not legacy same-source-frame admission.
`visual_pairs.py`, `visual_readiness.py` and `native_visual_receipt.py` are unchanged.
The adapter reuses file integrity/geometry/image metrics and the existing Dolphin
proposal's ready, actual-input and typed first-next-draw checks. Native GPU and
publication fields are not converted into fictional legacy rendered-source frames.
No app/build/GPU/Game/emulator/worker/debugger or runtime is provided. Checked-PNG
fixture reuse stays disabled. Qualification is always BLOCKED, qualified counts
null; there is no runtime-approval or waiver switch.

## File adapter and independent integrity root

The private manifest declares schema_version2, contract
`post-seam-request/next-observed-draw-v2`, phase_contract_status `proposed`, an explicit
synthetic boolean and pairs with stage/scenario/offset plus native/dolphin descriptors.
Offsets are exactly60/300/600; missing slots stay NOTMEASURED. Each descriptor has a
hash-bound producer receipt and eight artifact references: ready, input, draw, copy,
selection, readback, publication, color. These wrap the existing event records;
`COMMON`/`ROLE_GROUPS` list required fields without a new runtime event framework.

Producer receipts bind role/host/run identity, actual executable file, source manifest
(reviewed semantic base plus actual revision/source files), instrumentation sites/
source files and a build receipt binding those hashes. Hook source files must appear
in that source manifest. Clock facts/explicit unknown and distinct loop/draw/update/
VI/present/dump counts remain per provider. Color needs separate source/binary/role/
geometry-bound validation. PNG extension alone never proves color encoding.

The CLI requires an independent operator pin file plus its out-of-band digest:

```
python3 phase_v2_pairs.py --manifest PRIVATE/pairs.json --out PRIVATE/new-report \
  --operator-pins PRIVATE/operator-pin.json --operator-pins-sha256 REVIEWED_SHA256
```

The pin contains only schema_version1, scope `operator-integrity-pin`, contract and
manifest_sha256. Its separately supplied digest roots the manifest and transitively
verified provider/source/binary/stop/raw-export/closure/census/event/color/publication
artifacts. The tool never generates a pin or accepts a manifest's assertion of it.
Independently review producer/controller evidence before pinning. Self-rebound files
fail against the retained root. An operator can intentionally replace the entire
root: checksums are integrity evidence, not cryptographic producer authentication,
signed execution records, runtime permission or qualification. Fixture tests create
explicitly synthetic roots only. All nested files and roots are rehashed before a
measured result/report is finalized; drift cannot leave a measured row.

## Content and input semantics

Ready artifacts contain dense source observations, independently recorded origin and
CPU-stop witness. The adapter rederives the first ready seam and rejects shifted
claims. Original raw export, closed collector receipt and full0..stop read census
bind the whole input history; dense per-frame ordinals and canonical616 idle suffix
are checked. Suffix validity alone cannot prove prefix preservation. Input-delivery
errors are categorized separately from image metrics and gameplay interpretation.

First-next-draw selection requires complete history, witnessed adjacent-seam bounds,
matching typed entry/exit and actual selected scene. Mode3 copy/writer lineage,
first unchanged-policy native selection or first matching nonduplicate/nonstitched
Dolphin VI selection, resource/staging versions, completion and checked PNG publication
must agree. Native checks same-encoder ordering, complete write coverage, no unknown
foreign-buffer writes/reset/overwrite, validation/map completion and checked no-replace
publication tied to the actual readback. No field is relabelled to requested state.

The616 protocol is600 idle inputs plus16 predeclared idle observation-tail inputs.
Exact CPU stop and all draw/selection history stay within the bound; GPU drain may
finish later only without further emulation. There is no fitting, guessed lag, frame
rebase/deletion, forced seed or later convenient draw. Native latest-copy and Dolphin
VI policies are distinct and do not imply equal scanout time.

## Results and remaining implementation

The JSON report and HTML index reuse existing resolution-tolerant metrics/ranking,
without a new acceptance threshold. Consistent fixtures are PROPOSED_CONTENT_PAIR /
SYNTHETIC_METRIC_ONLY. Future actual artifacts can produce only MEASURED_UNQUALIFIED
proposed diagnostics. Every row/report keeps qualification BLOCKED and null counts;
no captured label or operator pin produces a gameplay PASS or accepted provider.
Requested post-endFrame state stays separate from draw entry/exit metadata; full
drawn gameplay state is explicitly not claimed. Sampled epoch is not authoritative
lifecycle ID. RNG/state, clocks, counts and unknowns remain separate per provider.

The smallest remaining implementation list is:

1. **GAME:** provide reviewed native-gated draw entry/exit sites, and loop-entry only
   if old render flag is needed. No hooks are installed by this delivery.
2. **PORT/native observer:** emit detached typed draw metadata, sampled-epoch authority,
   per-seam input census/original closure/CPU stop and ordered draw-copy-resource-
   selection-readback identity, with checked PNG/color evidence. Lifecycle API is held.
3. **PORT/Dolphin host bridge:** read pinned retail draw/actual-selected-scene sites
   and bind ordered XFB/VI/staging/publication/color receipts. CPU/GPU clock or address
   coincidence is not a bridge. Implementation/build/runtime require separate review.

Then request the small GPU controls and ONE supported-galaxy allocation described in
DOLPHIN-VISUAL-PHASE.md. T2 native captures and old601-row evidence are not phase-v2
pairs and do not solve the missing provider bindings.
