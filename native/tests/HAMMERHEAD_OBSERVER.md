# HammerHead observer null regression (U023)

Run `python3 native/tests/hammerhead_observer_tests.py clang++`.
CTest name: `native_hammerhead_observer`.

The native observation hook requested `getSensor("body")`, but SMG2 registers
`"Body"` at the Head joint in a six-entry keeper. Case-sensitive hash/string
lookup returns null for lowercase. This is a native hook naming error, not a
Wii garbage return or a reason to substitute a zero position. The correction
uses the registered name, inside the existing GALAXY_NATIVE block.

Target SB4E01 bindings:

- `control` 8018C140..8018C148 loads the rumbler pointer at actual Wii +BC and
  tail-calls 80068000. No observer sensor lookup exists in the Wii body.
- `initSensor` 8018D370..8018D4D0 registers six sensors. First name is the complete
  NUL-terminated `Body` object at 80687A6C, joint `Head` at 80687954.
- `LiveActor::getSensor` 802379E0..802379F8 reads typed keeper (+5C on Wii),
  returns null if absent, otherwise tail-calls the keeper lookup at 802360F0.
- The six-entry lookup computes the case-sensitive hash (8002BD00), checks hash
  and strcmp, then returns the sensor; exhausting the list returns null.

The test compiles the real HitSensor, HitSensorInfo, HitSensorKeeper and HashUtil
sources. Canonical headers define their native64 layout. Production initSensor,
LiveActor getter and control bodies are extracted verbatim, changing only the
receiver's class qualifier to an actor fixture. Scene joint registration,
facing/chance, collision-group hookup and rumbler are explicit test seams.
The real native publishActor wrapper and typed lookup remain in the test.
This is not a full initialized actor or gameplay smoke.

Checks retain the Head joint/offset, animated nonzero position, facing, state,
ready/hostile flags, and one rumbler update both with observation off and on.
An old-lowercase-key control compiles successfully and must fail at the actual
null HitSensor access before the observation-off publication guard can run.
No fallback, null guard, new game logic or class layout change is introduced.
