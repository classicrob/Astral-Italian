# ActionSoundInfo default initialization ABI

The constructor at `802205A0` forwards its already typed `const ActionSoundData*`
parameter to the default initializer at `80220730`. The DOL explicitly copies
incoming `r5` to argument `r4` before the call. The source previously discarded
that constructor parameter and declared the provider without a parameter.

The owning header, constructor call, provider definition and symbol identity now
use `setDefault(const ActionSoundData*)`. The initializer does not use the formal:
its complete 112-byte body overwrites incoming `r4` and `r5` before reading either.
This is an ABI/source-shape inference from explicit argument staging, rather than
a claim that the old omission caused a runtime behavior defect or that the exact
historical C++ declaration was recovered.

The sole provider call is at `802205C8`. The sole constructor call is at
`80220390`, inside the ActionSoundData constructor, and passes its own typed
`this` pointer. A conventional complete DOL branch/address-word census found no
other direct caller or static vtable/PMF reference. It does not exclude arbitrary
computed code addresses.

After the single source form, all 98 constructor words equal the target, with the
provider relocation explicitly rebound to its corrected identity. The provider
remains byte-identical; all five other functions retain their full bodies,
addresses and effective references. Data, string order, scalar widths and
allocated storage outside the constructor are unchanged. No layout, pointer
width, inline policy, compiler option or FP expression changes.

Frozen evidence is in `~/MacioGalaxy2-team/scratch-enemies/sol8/action-sound-info/`:
`REPORT.md`, `final-evidence.json`, `final-audit.py`, the candidate patch, original
DOL bodies and complete source/target objects. The root performs target
regeneration and the full-build/report guard after applying the map correction.
