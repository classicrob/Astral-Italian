# MarioHang: restore tryClimb literal encounter order

Source frozen after one natural form. Root owns split/configure/fullbuild/git.

Worker source change: move unchanged tryClimb definition/comment before close.
Actual target orders tryClimb before close. This encounters the SlipUpPrep name
at pool44, before End60/EndSlope70/Basic84/Fall8C, exactly as original.
Parent applied only .data start806DD5E8 ->806DD550,end806DD630 unchanged.

Complete224B.data, including152B literal prefix and72B vtable, equals actual
DOL after all16 ADDR32 relocations resolve canonically. Original prefix was
auto_07_806DD550_data, exactly152B with no sibling payload. Full neighbors pinned:
preceding MarioHairAndHatModel448B/32ADDR32 endingD550; following MarioHolder
56B/9ADDR32 beginningD630. Both reconstruct exactly to actual DOL.

Preservation checks:

- Source text is exactly original with unchanged tryClimb block moved. No
  header/signature/symbol/forcing/helper/native behavior changes. Scoped CA0/D70
  dont_inline and all Hang13 tail helper ownership remain unchanged.
- All39 emitted source functions/sizes retained. Only8 same-register ADDI words
  change, each resolving to the same full NUL string. All298 effective code
  references retain complete object payloads and callee identities.
- All23 target functions/1213 original instruction words verified against actual
  DOL, including real calls, raw conditional branches, HA/LO and SDA21 bases.
  EDE40's34 masked words/ordered calls/full scalar payloads now exactly match.
- Current masked-word+calls+scalar anchors16 ->17, all prior16 preserved.
  Historical Hang64's13 anchors are context only, never current proof waivers.

Final regenerated-target SG: strict7/23,768/4852B unchanged; all7 prior strict
anchors retained, fuzzy98.5% ->98.6%. Root batch nj-j4 passed. Final rebuilt
source retains exact trial1 ALLOC bytes/refs/39function identities; source text
still frozen. Current target's1213 actual-DOL words and object/source/tool hashes
bound in target-bindings.json. No second source form needed.

Final normal unchanged-default rdclass: original0benign/3REAL ->1benign/2REAL.
fn_803EDE40__9MarioHangFv exits as a34instructions/136B. close/update remain REAL
at conservative unknown returned/partial pool pointer offset. No checker
exceptions, extra source forms, or register polishing. Proposed row files are
owner review input; no canonical benign writes.

Bounded evidence: trial1-data.json, trial1-preservation.json,
neighbor-witness.json, masked-anchor-preservation.json, target-bindings.json,
before/sg.txt, trial1-sg.txt, final-sg.txt, final-rdclass.log, proposed.txt(+JSON).
Inputs saved before edits in before/; frozen candidate in after/. Reproduce:

```
nice -n 10 env PYTHONDONTWRITEBYTECODE=1 python3 ~/MacioGalaxy2-team/scratch-enemies/sol8/hang/verify_delta.py
nice -n 10 env PYTHONDONTWRITEBYTECODE=1 python3 ~/MacioGalaxy2-team/scratch-enemies/sol8/hang/bind_target.py
```

Ownership ledger read from server originals scratch-astra/sol-hang38/{REPORT.md,
mappings.json},hang64/REPORT.md via host_amemqempmf. Root alone publishes current
bindings, commits/lands/revalidates all-unit losses, and releases unit to Astra.
Shared SERVER NOTES append completed through host_amemqempmf terminal
term_4dp58tc7kv,exit0. ActionTable remains read-only until Hang mainSHA/release.
