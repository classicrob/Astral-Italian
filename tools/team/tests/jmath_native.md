# Native JMath checks

Run `python3 tools/team/tests/jmath_native.py` from any directory. Requires a system Clang/GCC C++11 compiler and AddressSanitizer/UndefinedBehaviorSanitizer. Override with `--cxx` or `CXX`. The runner compiles actual repository headers and `src/JSystem/JMath/JMath.cpp` in optimized and sanitized modes. It supplies no SDK replacement functions. Linker dead stripping removes unused Euler/matrix routines and their SDK dependencies. Temporary binaries are removed automatically.

The tests exercise 10,000 randomized vector/Hermite cases, independent double-precision Hermite reference values, endpoint values and derivatives, exact input/output aliases, shortest-path quaternion interpolation, overlapping/unaligned copies, and square-root edge cases. Explicit checks remain enabled; implicit floating-point contraction is disabled so the operation order is reproducible. Explicit `std::fma` calls stay fused.

Contracts:

- `JMAFastSqrt` preserves nonpositive values, signed zero, and NaN. Positive infinity produces NaN because PPC `frsqrte(+infinity)` returns zero before the multiply. Positive finite values use host square root rather than emulating the PPC estimate; bit-identical estimate results are not promised.
- Hermite arguments are `(position,time0,value0,slope0,time1,value1,slope1)`. Slopes are derivatives with respect to position/time. Arithmetic follows the original float/fused operation order. No clamping or equal-time guard is added; extrapolation is allowed and equal times propagate NaN in the tested degenerate case.
- The four Gekko copy functions snapshot 3, 6, 12, or 16 floats before writing. Portable `memmove` supports overlapping sources/destinations and preserves representations.
- Vector operations allow destination equal to either input. Dot reduction is `fma(ax,bx,ay*by)+az*bz`; squared magnitude is `fma(z,z,x*x)+y*y`; squared distance is `fma(dx,dx,dy*dy)+dz*dz`. These preserve paired-single lane order. Arbitrary partial overlap between distinct typed `Vec` objects is outside this contract.
- Scale-add computes `fma(vec1,scale,vec2)` componentwise; lerp computes `fma(b-a,t,a)`. Both permit exact input/output aliases.
- Quaternion lerp chooses the negated endpoint when dot<0, does not clamp, and does not normalize. Its dot reduction is `fma(pz,qz,px*qx)+fma(pw,qw,py*qy)`.

These tests check portable arithmetic and memory behavior. They do not emulate Gekko estimate bits, exception state, denormal flushing, or NaN payload conversion.

Validation on macOS system Clang: optimized and ASan/UBSan runs passed against the installed repository files. MW-preprocessed token streams for JMath.hpp, JMATrigonometric.hpp, and JMath.cpp were also compared with HEAD and found identical.
