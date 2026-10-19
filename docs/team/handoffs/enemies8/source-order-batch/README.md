# Restore source order and data ownership

One natural source-order trial per unit restores original pools without changing
calls, constants, fields, branches, class layouts, or the default checker.

| Unit | Strict SG before → after | Report matched-code gain |
| --- | ---: | ---: |
| MorphItemObjNeo | 29/54 → 35/54 | 2,960 B |
| MarioDamage | 21/40 → 21/40 | 1,300 B |
| MarioHang | 7/23 → 7/23 | 136 B |
| YoshiHolder | 38/59 → 40/59 | 280 B |
| Total | | 4,676 B |

The full ninja/original-DOL hash check passes; every other unit's matched code
is nondecreasing against the base report. Strict and relocation-masked report
bytes are different metrics. No unit is marked `Matching`.

Morph's unchanged getter moves before the resource dispatcher; its automatic
pointer array is const, preserving the 17-instruction stack-copy getter. All
71 emitted functions and effective references survive, with 55 ADDI changes
reaching the same complete strings. The 56-byte read-only initializer/double
region and its ten ADDR32 entries match the original DOL. New ownership covers
that region, the 976-byte base/nerve data region and four-byte END literal. Two
existing derived-class weak vtables remain separate source-only emissions;
owned-data comparison excludes them and explicitly maps the nine switch-case
PCs over the existing four-byte tryGetItem setup difference. This is not a
whole-function tryGetItem proof. No scalar-pool extent is claimed.

Damage removes its source-only forcing switch and orders existing includes to
emit the six vtables in original order. All 57 surviving function bodies,
342 calls and 39 scalar loads retain their identities. All 872 data bytes,
96 ADDR32 entries and 80 read-only bytes match. Its scalar section still differs
in layout. The separate marker in Mario.cpp is left with the Player owner.

Hang moves unchanged tryClimb before close; its 224-byte data region and all
16 pointers match. The preceding 152-byte pool is recovered, preserving both
neighboring units' full bytes and pointers. All 39 emitted functions and 298
references survive, including the thirteen previously sourced tail helpers.

Holder moves unchanged fn_8041BB00 after the constructor and fn_8041B700. All
70 emitted functions, 145 references, 38 strict and 50 report-masked anchors
survive; its complete 80-byte string pool matches. Flutter is still requested
when the countdown was already zero.

Unchanged default rdclass clears FireRun::close, FireDance::close, and Hang's
fn_803EDE40. Morph's four open rows remain REAL at conservative pool/return
provenance guards; Damage start/impactEffect and Hang close/update remain REAL.
Existing Damage call-audit differences are only save/restore helper choices.
No proof-mode experiment or proof-list waiver is included in this landing.
