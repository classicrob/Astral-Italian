#pragma once

namespace PetariNative {
namespace PPC {
// Unrounded Gekko estimates; retain the binary64 result until the consuming operation rounds.
double frsqrte(double value) noexcept;
double fres(double value) noexcept;

// Gekko operand precision and round-to-nearest-even single precision, without FPSCR side effects.
double force25Bit(double value) noexcept;
float forceSingle(double value, bool nonIeee = false) noexcept;
}  // namespace PPC
}  // namespace PetariNative
