# U025: capture coordinates with Gekko word arithmetic

Native image capture explicitly evaluates the operations in the original DOL
at `8047697C..804769B8`: height getter, row division, low-word product and
division for y, then width getter, quotient/product/subtraction for the column,
low-word product and division for x, followed by the unchanged texture call.
The Wii implementation stays verbatim under the other preprocessor branch.

`petari/ppc_integer.hpp` defines the signed destination words of the needed
DIVW/MULLW/SUBF operations. [Dolphin's Gekko interpreter](https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/Core/PowerPC/Interpreter/Interpreter_Integer.cpp#L461)
returns -1 for an exceptional signed divide with a negative numerator, and 0
otherwise; this includes division by zero and INT_MIN/-1. Products and
subtractions keep the low 32 bits. This is a Gekko/Dolphin result model, not a
claim that every PowerPC implementation specifies the same exceptional result.
XER/CR are not modeled; this caller uses the result words only.

Consequently the observed zero-divisor/zero-tile case captures at (0,0), but
a negative tile with a zero divisor and positive dimensions yields (-1,-1).
The fix does not replace all exceptional inputs with zero or skip the texture
capture. It also defines signed product wrap instead of relying on native C++
overflow, preserving all argument roles and getter order.

Run `python3 native/tests/image_capture_native_test.py`. It extracts the actual
production adapter, using recording doubles only for framebuffer getters and
the final JUTTexture call. Its 31,325 ASan/UBSan checks cover normal/signed/zero
dimensions, tile/divisor boundaries, exceptional division, low-word wrap,
forwarded arguments, getter order and 10,000 deterministic arithmetic pairs.
The integer oracle uses independent 64-bit operations. The preserved original
branch compiled natively reproduces divide-by-zero as a negative control.

Fresh isolated ImageEffectLocalUtil Wii objects compare equal with current
`tools/team/objsame.py`; the complete native TU passes the compile audit.
No renderer/gameplay or complete game build is claimed by these tests.
