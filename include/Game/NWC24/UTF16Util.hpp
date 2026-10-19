#pragma once

#include <revolution.h>
#ifdef GALAXY_NATIVE
#include <cstddef>
#else
#include <size_t.h>
#endif

namespace MR {
    /// @brief Scans up to 1024 UTF-16 code units for a terminator.
    /// @param pStr The pointer to the null-terminated wide string to be examined.
    /// @return The length, or zero if no terminator is found within that limit.
    size_t strlenUTF16(const u16* pStr);
};  // namespace MR
