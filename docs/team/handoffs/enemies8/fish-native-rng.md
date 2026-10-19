# Fish constructor native RNG order

The Fish constructor now stages all five draws in separate full expressions:
offset Z, Y, X, approach distance, then search time. Moving only the first three
to the body would incorrectly put distance and time first, so all five move
together. Bounds, result types, base-constructor registration and final member
values are preserved.

The initializer list gives the RNG-dependent fields safe zeros. This emits five
additional zero stores and moves the front/step/escape initializers before the
draws. These are real changes in store timing, rather than byte-identical code.
The complete 304-byte actual RNG closure touches its separate global state and
private frames, with no Fish-field read, callback or registry traversal. None of
the remaining member initializers observes the moved fields. The construction
relation excludes concurrent observation of the unfinished non-atomic object;
early base registration remains in the unchanged prefix.

Apple clang at `-O0` and `-O2`, using the extracted actual constructor with a
stateful RNG harness, matches all final fields against the DOL order across
4,101 seeds. The old expression assigns the first and third draws to the wrong
axes. The harness explicitly records the moved early stores; it is not a native
whole-game test. The integer draw can round to 10, which both versions preserve.

The single candidate grows the constructor from 264 to 276 bytes. All 33 other
retained emitted bodies/references and all 15 single-object strict anchors are
preserved. A source-only 16-byte vector constructor retires naturally. Scalar
pool placement changes, with every consumed typed payload preserved. There is
no new helper, layout, API, pragma or compiler setting.

The parent approved an exception for exactly `__ct__4FishFPCcP9FishGroupib`, with
an expected 264-byte full-report exact-code cost. The root's full-report guard
must confirm that cost and reject any unrelated change. MacioGalaxy's Fish
source still has the original argument expression; there is no existing native
ordering hook to transfer.

Frozen evidence: `scratch-enemies/sol8/fish-rng-design/` under the shared team
directory, particularly `body-alternative/REVIEW.md`,
`native-trial-6b7e21d9/REPORT.md`, `preservation.json`, and
`native-trial-6b7e21d9/native-harness/REPORT.md`. Native approval does not itself
certify a whole-function benign exemption.

Final local guard on exact `59c6b180`: full build and original-DOL hash pass;
all 3,480 units are compared. Only this constructor changes score, from 100%
to 66.77273%, costing exactly 264 matched-code bytes. FishGroup retains 22/23
code functions, 3,116/3,380 bytes. No other score or target inventory changes.
The older SDK12 `580d944b` cache was explicitly discarded after intervening
Player/Rotate source changes; the guard uses the actual commit-keyed baseline.
