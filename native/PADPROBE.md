# PadProbe: five-minute hardware checklist (macOS arm64)

**The physical Switch 2 Pro Controller has NOT yet been tested.** The probe's
no-pad receipts prove software execution and packaging only. Close the game,
Steam, and other controller tools first. Use a USB **data** cable. Unplug the
pad and disconnect other gamepads. No Homebrew, SDK, or environment variables
are needed on the desk machine.

1. **Unzip in Terminal (about 20 seconds).** In the directory containing the zip:

   ```sh
   unzip PadProbe.zip
   ```

   If downloaded/copied through a browser, remove quarantine once:

   ```sh
   xattr -dr com.apple.quarantine PadProbe
   ```

   scp/rsync copies need no quarantine command.

2. **Probe (about one minute).** Keep the pad unplugged when starting:

   ```sh
   PadProbe/bin/padprobe
   ```

   Plug it in at **“Plug the pad in now”**. Press only the named control, then
   release it; center sticks between directions. A/B/X/Y mean Nintendo labels:
   A=right/EAST, B=bottom/SOUTH, X=top/NORTH, Y=left/WEST. Each prompt times out
   automatically after at most six seconds; no Enter is required. Follow the
   still/shake prompts and note whether you feel the half-second rumble pulse.
   The total run is limited to 60 seconds, including insertion time. If rows
   were skipped or you need more time, repeat with:

   ```sh
   PadProbe/bin/padprobe -t 240
   ```

   The file `padprobe-report.txt` is written in your current directory (path
   printed at the end; under 30 KB). Copy it before rerunning, which overwrites
   it. A diagnostic FAIL still exits 0; exit 2 means invalid arguments or a
   report-file error. A complete probe ends early once all steps finish.

3. **Game (about two minutes).** After the probe exits, launch the lead's actual
   game build. From its repository root, the usual command is:

   ```sh
   build/macos-gx/native/app/Petari.app/Contents/MacOS/Petari \
     --disc build/game-data/SB4E01 2>&1 | tee pad-game-log.txt
   ```

   Use the supplied frozen app/disc paths if different. Look for `[pad] using`
   with controller name, layout, gyro/accelerometer, and rumble capabilities.
   Open **F1 → Controls**: the controller column should appear. In a stage,
   check A jump/confirm, B jump in play/back in menus, X/Y/R spin, and the right
   stick Star Pointer with Odyssey off. Enable gyro (`Gyro=on` in controls.txt,
   with the game closed, or Odyssey layout's auto mode), move the pad to aim,
   and press R3 to recenter. Check ZL crouch, ZR shoot, and L camera recenter.
   With **Sol B's mapping included in that game build**, check GL/GR/C against
   the current **Switch Pro Controller** table in `native/CONTROLS.md` (and the
   Controls page's bindings); record the actual actions. If those rows are
   absent, record **SKIP: mapping not integrated** instead of guessing actions.

4. **Reconnect and return evidence (about one minute).** Unplug/replug during
   the game: no held movement/buttons should stick; the pad should reconnect.
   Trigger a gameplay rumble and note felt/not felt. Send **padprobe-report.txt**,
   **pad-game-log.txt**, the game build/commit, USB or Bluetooth, macOS version,
   and brief pass/fail notes for step 3, reconnect, and physical rumble. The
   report includes USB serial/device metadata; send it to the team privately.

## Appendix: interpreting the report

Expected examples (illustrative, **not hardware results**):

```text
PASS USB interface 1 claim=LIBUSB_SUCCESS (0)
FAIL USB interface 0 claim=LIBUSB_ERROR_ACCESS (-3) (HID claim failure is expected on macOS)
PASS IOHIDDeviceOpen 057e:2069 = 0x00000000
SDL log category=...: Added HIDAPI device ... driver = Switch 2 ...
PASS control C: expected input arrived; last=MISC2 id=21 value=0
FAIL control GL (left grip): TIMEOUT; UNEXPECTED SDL BUTTON ...
VERDICT: PARTIAL: inspect FAIL layers and unexpected control IDs
```

Interface 0's libusb claim failure alone is expected: macOS owns HID input.
Interface 1's vendor bulk IN+OUT claim and IOHID open should succeed. The probe
never detaches a kernel driver, changes configuration, or sends USB transfers;
it releases every successful claim immediately. SDL subsequently performs its
own handshake. Thus a successful bulk claim proves access, not handshake
success. Each Nintendo device/configuration/interface/endpoint is listed;
report-descriptor offsets and bytes describe the independent IOHID layout.

The summary prefers USB/HID product `057e:2069` over other Nintendo products.
Test with just one controller: multiple identical controllers can give mixed
layer results. Any other SDL gamepad can run the general input/sensor/rumble
tests; Switch 2 USB and C/GL/GR qualification is SKIP. Capture expects MISC1,
C MISC2, GL LEFT_PADDLE1, GR RIGHT_PADDLE1. An unexpected SDL button is a
mapping failure even when another expected event also arrives. An axis must
cross 50% after returning below 25%. Keep other buttons released.

| Failed layer | Likely cause / next check |
| --- | --- |
| libusb load | Missing/wrong-architecture dylib, signature, quarantine, or incomplete zip; inspect each dlopen path. |
| USB enumerate | No Nintendo USB device: cable, port, power, Bluetooth-only connection, or libusb enumeration error. |
| USB open | Another process owns the device or macOS denied access; use the exact libusb error. |
| bulk interface claim | Vendor bulk interface busy/inaccessible; interface 0 HID failure is expected. |
| IOHID open | No HID service, access denied, exclusive owner, or unexpected descriptor; inspect IOKit return code/descriptor. |
| SDL driver attached | USB handshake/IOHID routing/driver hint issue; inspect captured INPUT `driver =` and `couldn't open` lines. |
| SDL mapping | Joystick exists without gamepad mapping, or prompted physical control arrived at a different ID. |
| gamepad event flow | Open succeeded but input reports did not arrive; check selected device, connection, and driver logs. |
| mapping of extras | Report bits or SDL mapping differ; return observed IDs for C/GL/GR/Capture. Home/Capture may be intercepted by macOS. |
| sensors | Unsupported/disabled sensors, no samples, or no increased variation during shake; compare advertised/observed rates and repeat. |
| rumble | SDL rejected the call; PASS only means accepted, so also report whether the pulse was felt. |

`NO_PAD` means no controller evidence arrived by the deadline; expected on a
machine without a pad. `*_FAILED` identifies the earliest actionable layer
when no gamepad opens. `EVENTS_FAILED` means a gamepad opened without guided
physical input. `PARTIAL` means at least one measured layer failed;
`INCOMPLETE` means a required test was skipped (usually increase `-t`).
`OTHER_GAMEPAD` qualifies only generic event flow. `PASS` requires every
measured layer and guided control to pass; still confirm the game and felt
rumble. Detail output has per-phase limits with a visible truncation notice;
input-test and summary space is reserved. SDL logs have a 4.5 KB allowance.
If capped, repeat with one controller attached.

## Appendix: building and packaging on the M1

Standalone, without configuring GX or game assets:

```sh
cmake -S native/tools/padprobe -B /tmp/padprobe-build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/padprobe-build --parallel 4
ctest --test-dir /tmp/padprobe-build --output-on-failure
native/tools/padprobe/package.sh /tmp/padprobe-build
python3 native/tests/padprobe_tests.py --binary /tmp/padprobe-build/padprobe \
  --receipts native/tests/receipts/padprobe_nopad_20261022
```

Default dependencies come from `native/cmake/PadDependencies.cmake`, including
the app's pinned SDL 3.4.10, Aurora patches, ordered `0001`/`0002` patches, and
libusb 1.0.30. `dependency-paths.tsv` records their hashes. Scratch-only override:
set **both** `-DPADPROBE_SDL_DIR="$HOME/MacioGalaxy2-team/scratch-pad/pad"` and
`-DPADPROBE_LIBUSB_DIR="$HOME/MacioGalaxy2-team/scratch-pad/pad/prefix"`.
For that development build use `DYLD_FALLBACK_LIBRARY_PATH` pointing at the
scratch prefix's lib directory. The final zip is always tested without it.

`package.sh BUILD_DIR [DIST_DIR] [LIBUSB_PREFIX]` produces `PadProbe/` and
`PadProbe.zip` under `~/MacioGalaxy2-team/scratch-pad/pad/dist/` by default,
refusing to overwrite existing output. It copies the selected build's libusb
prefix, sets install name `@rpath/libusb-1.0.0.dylib`, and ad-hoc signs/validates
both arm64 binaries. SDL patch 0001 finds `../Frameworks` relative to
`PadProbe/bin/padprobe`; the probe's own dlopen independently tries
`@executable_path/Frameworks`, `@executable_path/../Frameworks`, and plain names.
An executable rpath and plain-name symlink also support the older scratch SDL
loader. Licenses are included. No environment variables are required.

For the attached non-pad robustness check, add `--all-usb` to the test script;
it lists all devices and explicitly reports SKIP if no non-Nintendo USB device
is attached. Remove your scratch build once the receipts and zip are verified.
