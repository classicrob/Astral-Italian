# MarioSideStep: restore natural pool order and data ownership

Ready for root landing; source/split frozen. Exactly one source trial.

Commit contents:

- src/Game/Player/MarioSideStep.cpp: move unchanged close() definition after
  update(), matching actual function order ctor/start/update/close.
- config/SB4E01/splits.txt: only SideStep.data start806DDD30 ->806DDCF0;
  end806DDD78 unchanged.
- Root-selected bounded evidence from this directory. No proof tools, headers,
  signatures, symbols, benign lists or unrelated source/config changes.

The original64B literal prefix lived in auto_07_806DDCA0_data before the SideStep
vtable. Source close-before-update encountered wall-right/wall-left before Body;
target update-before-close encountered Body first. Natural definition order
restores all136B.data including16 canonical ADDR32 words exactly to actual DOL.
Recovered preceding auto80B and all9 CollisionShadow refs are unchanged. Generated
UND lbl_ symbols were split metadata, not demonstrated external linkage/escape.

Measured results (distinct metrics):

| Metric | Before | After |
| --- | ---: | ---: |
| Strict SG functions | 1/4 | 2/4 |
| Strict SG bytes | 64 | 180 (+116 start) |
| SG fuzzy unit | 94.3% | 94.5% |
| Root full-report matched code | baseline | +148 (close) |
| Original two pool rows, unchanged-default rdclass | 0 benign / 2 REAL | 1 benign / 1 REAL |

Default rdclass accepts close__13MarioSideStepFv as a:37 instructions, physical
registers/stack/calls/control flow identical, full pool checks. Proposed row is
side-step/proposed.txt; root/Astra alone may bind/publish it. Update remains REAL:
guard conservatively treats a call result as a possible string alias at
stfs f0,0x744(r3). No return/alias exemption or additional source trial.

Root reports local full nj-j4 green and no full-report unit losses. Worker checks:

- All361 original target instruction words verified against actual DOL, including
  exact relocation/callee destinations and SDA21 RA/base linking from original
  __init_registers. Close37 masked words, ordered calls and scalar bytes exact.
- All13 emitted source functions keep sizes; ctor/start+9 weak bodies raw exact.
  Only7 same-register ADDI words change in close/update, each reaching the same
  complete NUL string. All100 effective code refs retain complete literal/scalar
  payloads and callee identity.
- .sdata2 ordering changes (zero,0.1,-0.866 ->0.1,-0.866,zero); per-use full scalar
  payloads/widths remain exact. It is not a byte-identical scalar section claim.
- Existing31 pool tests and219 full test_rd*.py tests passed. rdpool.py and
  test_rdpool_peruse.py are untouched; no new proof framework implemented.

Evidence: trial1-pool.json (all136B/16ADDR32), final-bindings.json (DOL-bound raw
instructions/source-object-tool hashes), raw-source-preservation.json,
source-effective-refs-preservation.json, preceding-auto-preservation.json,
final-sg.txt, final-rdclass.log, proposed.txt + associated JSON. Original and final
source/objects remain frozen in before/ and after/. Reproducers from checkout root:

```
nice -n 10 env PYTHONDONTWRITEBYTECODE=1 python3 ~/MacioGalaxy2-team/scratch-enemies/sol8/pool-peruse/side-step/verify_source_delta.py
nice -n 10 env PYTHONDONTWRITEBYTECODE=1 python3 ~/MacioGalaxy2-team/scratch-enemies/sol8/pool-peruse/side-step/bind_final.py
```

Original pool-task census is retained in ../actual33.json: every row needs
writable storage (35.data+2.sdata+1.rodata origins). Readonly-only infrastructure
would clear zero rows; this source/ownership repair clears one with strict
defaults. Shared SERVER NOTES updated through host_amemqempmf (term_tmimnmjrx5,
exit0). Root alone commits/remote-lands/rebinds and releases SideStep to Astra.

Root integration suite subsequently passes247 tests. This source landing excludes all proof-tool WIP; the accepted close row uses unchanged default proof dispatch.
