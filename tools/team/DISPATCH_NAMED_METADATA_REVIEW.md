# Disabled three-stage named-key metadata observer

No actual observation, signal/control run, admission change or operator value
was obtained. The disabled plan fb78e1e3 is source preparation only. Existing
6700 gate, eight-key launcher context, importer and disabled full827 are unchanged.

## Exact observation boundary and privacy

Three stage records ONLY: `launcherentry.json` (after fixed source/plan/floor/lock
checks and fresh directory creation, before deliberate environment setup),
`parentaftereightkeysetup.json` (immediately after the existing eight-key mapping
is installed), and `childentry.json` (selected once in child main, before plan
validation; after interpreter and listed standard-library imports). They observe
Python's mapping, not a direct native-environ enumeration or an OS launch seam.
The exact selected key is `__CF_USER_TEXT_ENCODING`; absence is distinct from
empty. Strict UTF-8 hex, byte length and SHA256, <=512 bytes/no NUL; overflow or
unrepresentable input FAILS, never substitutes/normalizes. No other ambient key,
value or value hash is captured. Requested base eight values are fixed plan
inputs, not ambient observations. No values are printed to chat or stdout by
this runner. Source/model tests use SYNTHETIC values only.

Each stage includes exact stage/plan SHA/source byte pins/Python pin, actual
PID/PGID/SID/UID, and actual parent PID. Private root mode700; EXCL temporary600
followed by rename, no stale final replacement. Each stage record <=16384 bytes,
three stage names maximum; duplicate JSON keys and bool schema/identity counters
reject. Source/input pins are checked in parent and child. Child record parser
checks size/type/mode/UID, exact envelope, named value representation/hash/length,
plan/source/Python/parent joins. Interrupted partial `.tmp` files are preserved.

Invocation, journal, child log, budget/cleanup and completion are separate control
metadata, not additional named-value observations; the whole root is limited to
64MiB sampled growth. Final completion uses a 64KiB reserve under that same cap
and floor. No additional observer or universal framework is introduced.

## One metadata child; inherited reviewed lifecycle

`MetadataJob` inherits existing LiveBudgetJob TERM1/KILL3, wait/reap-before-group
probe, four-second cleanup bound, EPERM uncertainty, journal and no-signals when
identity is unverified. It reuses the existing finite scan check. The narrow
identity adapter extracts the original four identity fields from the third stage
record rather than creating a fourth identity record. It checks exact registered
PID, parent PID, current UID, kernel getpgid/getsid and immutable record bytes.
Contradiction/failed observation is sticky and cannot later gain admission.

The spawn prefix is one pinned Python + this metadata helper + exact invocation
path/SHA + actual parent PID. No executable payload argument, tool target, ack
file or target exec exists. Popen preserves current env=None inheritance solely
for metadata observation of the same boundary, not arbitrary command admission.
Single-thread mask-block/spawn/register/child-mask-restore order is unchanged.
Normal wait/reap is journaled before strict group probe. Retirement clears the
fixture path only after strict absence; subsequent final scans cannot mistake a
retired record for an unregistered child. Child self-expires10s and otherwise
waits1s after publishing (no ack), then returns0. Failure executes no added
signals/probes; unknown ownership remains UNKNOWN and natural expiry is observed
as time passage only, never an absence PASS. No retries or alternative signals.

## Limits, allocation and remaining gates

Disabled plan fixes 30s total /10s sampled work +4s cleanup /64MiB /50GiB floor /
nice19 /nonblocking native.lock /fresh absent root. Finite scans retain their
sampled syscall-latency limitation, not a universal real-time OS quota. Existing
failures/roots remain untouched. Exactly one child is permitted in source; its
argv and every source/Python byte are pinned and rechecked before spawn.

Exact enabled-only plan/root allocation, L independent source disposition,
actual nonblocking lock/no-overlap/floor/source rechecks, actual runtime cleanup
and value/materialization/stability evidence all remain OPEN. No enabled copy is
created. This helper is NOT a generic compiler-runner qualification and does NOT
supply a ninth-key operator value or admission. The same key at multiple stages
need not imply a stable value or augmentation mechanism.

Six no-process/tmpfile/AST models PASS. They cover selected-only privacy/value
format; private EXCL/stale record retention; schema/parent/source/hash/size
rejections; duplicate JSON rejection; missing/contradicted identity no signals;
verified kernel join + immutable record failure; disabled-before-lock and no
child payload/ack. They do not execute main/child/Popen, send signals, repeat v7,
read an actual named value, or prove actual process lifetime.
