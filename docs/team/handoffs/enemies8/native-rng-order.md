# Native RNG argument order

KirairaChain::control and both random-wind paths in Flag::updateFlag now evaluate
their RNG draws in separate full expressions. The DOL draws Z, Y, X for Kiraira
and Z, X for each Flag vector; the vector still receives X, Y, Z. Bounds, draw
counts, surrounding draws, timer guards and loop placement stay the same.

C++ does not specify the relative evaluation order of function arguments.
Apple clang 16 on arm64 evaluates the old extracted sites left to right at both
`-O0` and `-O2`, assigning different draws to the axes. The named temporaries
restore the DOL assignment at both optimization levels. The witness uses the
actual extracted source/control blocks with a small stateful RNG mock, and checks
counts, bounds, axes and skipped paths. It is not a native whole-game test.

The Wii sequence is preserved. Kiraira changes four FMR register-home words.
Flag inlines two complete 16-byte vector constructors into the same three STFS
operations; all 50 ordered arithmetic/compare words remain unchanged. Other
emitted bodies and allocated data/references remain unchanged in both units.

The parent lead approved exactly these two native exceptions, including their
**full-report exact-code cost of 2,240 bytes**:

| Function | Target bytes | Before | After |
| --- | ---: | ---: | ---: |
| `control__12KirairaChainFv` | 260 | 100% | 99.69231% |
| `updateFlag__4FlagFv` | 1,980 | 100% | 98.422226% |

Single-object SG numbers differ: Flag already had literal-relocation residue,
so its SG strict-anchor count did not fall. That does not remove its 1,980-byte
full-report cost. No other function loss is authorized.

Frozen evidence: `~/MacioGalaxy2-team/scratch-enemies/sol8/arg-order-rng/`, including
`REPORT.md`, `temp-map.md`, `preservation.json`, `exception-handoff.json`, and
`native/run.sh`. The root's exact per-function guard and full reports are in
`scratch-enemies/sol8/action-native-gate/`. The server native ledger is
`scratch-astra/native-exceptions.md`. The default classifier is run after landing;
native approval alone is not a whole-function proof or a benign exemption.
