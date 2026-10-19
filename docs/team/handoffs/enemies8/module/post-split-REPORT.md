# Post-root-split MarioModule binding — PASSED / FROZEN

READONLY after root added.data[806DD6BC,806DD880). No SG, compiler, ninja,
configuration, source/header/split changes or proof changes. Root finalnj is
separate; no root fullbuild/hash outcome claimed here.

Current target.o owns452B.data. Resolve every one of its91ADDR32s against the
pinned current symbol map: full452B == actual originalDOL == saved frozen source
448B +4zero alignment. All91source and target reference offsets/actual words
match. This validates the actual post-split target, not only an earlier source
projection. Full binary images and per-cell evidence are retained.

Target.text4944B, including all padding, equals actualDOL after restoring ALL
154explicit relocations:121REL24,25SDA21,4HA,4LO. The r2/r13 bases come directly
from originalDOL __init_registers LIS/ORI words; every reconstructed instruction
and the complete image compare exactly. Raw target.text also equals the earlier
pre-split target.text. No new equivalence/proof rule or skipped branch/call.

Neighbor ownership preserved:
- P_PlanetMap prefix12B[D6B0,D6BC) exactly unchanged and not claimed by Module.
- Module claims only452B[D6BC,D880).
- Move24B[D880,D898) exactly unchanged and outside Module; current sourceMove
  complete20B +4zero alignment/all3ADDR32 also equals originalDOL.
- Preceding Messenger full48B[D680,D6B0), including all target relocations,
  resolves exactly to originalDOL. All complete neighbor bytes are retained.

Historical pre-split target SHA (trial1/target.o; no.data section):
fa8a159926a4df1c8edbc4337b93ab044558f7c2e1c764dad15d3fbe8b212221.
Current post-split target SHA (post-split/target.o; owns.data452B):
12f59a1ba12721b912a932989bf43744b0343a6b3e7e09cd86898ef526682c70.

Frozen CPP and source ELF remain unchanged:
CPP ffd304b592ce0dfa91b7525bb2cee933ac790efc1e59aa6cda14e29d765fe447.
Source ELF4b218d63137c67bcbb77fa6cf73b7c7328c09caf5b55d6f0c4b757a54054f3fa.
All input hashes were identical before/after capture. bindings.json separates
both phases. Symbols/splits and current target.o snapshots are local to this
evidence directory; validation.json pins DOL and every input.

Prior normal SG89/98 and zero saved row exits are historical prior checks;
parent also reports final post-split SG89/98. This validation did not rerun SG
or row audit and makes no code-gain claim. Root owns final report/fullbuild/git.
