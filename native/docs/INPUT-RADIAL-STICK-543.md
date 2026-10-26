# Radial stick encoding (5.4.3)

This is a native gameplay-feel fix, separate from the input fixture lifecycle
repair. Server stale-fixtures-543-r2 exposes Alt+W+D length 0.616026 instead of
half length. No SDK code or SDK clamp mode changes.

## Source evidence

On main 7fdb1c38, native/input/remote_model.cpp::stickVector normalizes WASD
then applies Settings::walkStickScale (default .5). For Alt+W+D the desired vector
is (.353553, .353553). The old nextReport encodes each axis independently using
rawStickAxis: round(15 + 56 * abs(axis)), giving raw (35,35). Its header and README
explicitly describe an inverse of the cross clamp.

The actual SDK in src/RVL_SDK/kpad/KPAD.c initializes kp_stick_clamp_cross=FALSE
(line 25). read_kpad_stick (1115) selects clamp_stick_circle (1062-1082). No other
source sets that flag. The circular map takes RAW RADIUS, not each raw axis,
through the 15..71 range. WPadStick::update copies the decoded values unchanged.
Thus (hypot(35,35)-15)/56 = .6160263336, matching the server failure. Straight
half stick is (0,43), which decodes to .5 and masks the mismatch on axes. Full
keyboard diagonal saturates to unit length and also masks it.

The intended half-length contract is documented in native/include/petari/input.hpp
(Settings::walkStickScale) and native/input/README.md. Merely accepting .616026
would retain the direction-dependent walking speed error.

## Encoding and compatibility

For finite desired vector v with length L > 0, let d=v/L and
R=15+56*min(L,1). Encode each component of R*d to its nearest integer raw value.
The circular SDK map then gives the intended length and direction within the
integer quantization limits below. Neutral stays raw (0,0).

The explicitly preserved legacy byte cases are:

- ALL axis vectors (one component zero), including neutral, cardinal full stick,
  and partial/Walk axis values;
- full-scale 45-degree keyboard directions: raw (+/-55,+/-55), as before.

Those cases already decode with the intended direction and magnitude (subject
to axis quantization); the full diagonal's larger raw radius simply saturates.
Every other vector uses the radial inverse. This changes ANALOG/gamepad input
at non-cardinal angles as well as partial keyboard diagonals: for example, the
old full 30-degree vector encoded (63,43), biasing direction to ~34.315 degrees;
the radial encoding (61,36) is ~30.55 degrees. PORT explicitly approved correcting
that analog direction bias instead of preserving those incorrect old bytes.

New native recordings therefore contain different KPAD values for affected
vectors. Existing tapes replay recorded raw SDK reads and are unaffected. Input
sample timing, replay bypass, buttons, accelerometers, pointer and motion steering
are unchanged. The raw replay code is not edited.

## Quantization proof and tests

Rounding each ideal raw coordinate changes it by at most 1/2, so vector error is
at most e=sqrt(2)/2 raw units. The SDK radial length map has slope 1/56 with clamps
at 0 and 1, giving decoded length error <= e/56 = sqrt(2)/112 = .0126269068.
For nonzero decoded output, the disk of radius e about the ideal raw vector
subtends angle at most asin(e/R). At full scale R=71 this is
asin(sqrt(2)/142), about .571 degrees. Radial clamping preserves direction.
A tiny target may quantize inside the neutral disk: its length is still checked
against the same bound; a neutral output has no direction to compare. Preserved
axis cases have zero angular error and <= .5/56 length error; preserved full
keyboard diagonals have zero angular error and saturated unit length, so they
also meet the output bounds despite using their old raw radius.

The original half-diagonal fixture's .01 length tolerance was not attainable:
(30,30) decodes to .4897572656; (31,31) decodes to .5150110792. The test now uses
the proved sqrt(2)/112 limit, keeping its equal-axis check. This is an integer
representability bound, not an allowance chosen to accept the old .616026 bug.

native_input_stick calls the ACTUAL SDK clamp_stick_circle, linked from the
production SDK object. It checks axis/diagonal/arbitrary-angle/full-scale vectors,
all quadrants, sign symmetry, byte-preserved cases, neutral and small lengths.
It checks actual RemoteModel keyboard and analog producer reports, and asserts
the SDK cross-clamp flag remains FALSE. Float-roundoff allowance is separate from
the quantization bound. No per-Rig reset, game, user cache or device is involved.

Validation before server handoff: a source-formula float32 calculation passed
3240 angle/length vectors and 4002 axis-byte cases (not native execution). Native
SDK-linked and full registered execution are pending PORT's server run. The full
native_input executable also needs the separately approved fixture-lifecycle
branch; this production branch is based directly on main, not on that branch.
