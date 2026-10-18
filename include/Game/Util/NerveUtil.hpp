#pragma once

#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"

#include <revolution/types.h>

class Nerve;
class NerveExecutor;

namespace MR {
    f32 calcNerveParabolaValue(const NerveExecutor*, s32);
    /// @brief Determines if the working nerve action's frame of execution is greater than or equal to the given frame value.
    /// @param pExecutor The pointer to the nerve action executor.
    /// @param step The frame value to compare with.
    /// @return `true` if the working frame of execution is greater than or equal to `step`, `false` otherwise.
    bool isGreaterEqualStep(const NerveExecutor* pExecutor, s32 step);
};  // namespace MR

// Merged from Petari (core lane): overloads missing here


namespace MR {
    bool isStep(const NerveExecutor* pExecutor, s32 step) NO_INLINE;
    bool isFirstStep(const NerveExecutor* pExecutor);
    bool isLessStep(const NerveExecutor* pExecutor, s32 step);
    bool isLessEqualStep(const NerveExecutor* pExecutor, s32 step);
    bool isGreaterStep(const NerveExecutor* pExecutor, s32 step);
    bool isIntervalStep(const NerveExecutor* pExecutor, s32 step);
    bool isNewNerve(const NerveExecutor* pExecutor);
    f32 calcNerveRate(const NerveExecutor* pExecutor, s32 stepMax);
    f32 calcNerveEaseInOutRate(const NerveExecutor* pExecutor, s32 stepMax);
    f32 calcNerveEaseInRate(const NerveExecutor* pExecutor, s32 stepMax);
    f32 calcNerveEaseOutRate(const NerveExecutor* pExecutor, s32 stepMax);
    f32 calcNerveValue(const NerveExecutor* pExecutor, s32 stepMax, f32 valueStart, f32 valueEnd);
    f32 calcNerveEaseOutValue(const NerveExecutor* pExecutor, s32 stepMax, f32 valueStart, f32 valueEnd);
    f32 calcNerveEaseInOutValue(const NerveExecutor* pExecutor, s32 stepMax, f32 valueStart, f32 valueEnd);
    f32 calcNerveEaseInOutValue(const NerveExecutor* pExecutor, s32 stepMin, s32 stepMax, f32 valueStart, f32 valueEnd);
    f32 calcNerveEaseOutValue(const NerveExecutor* pExecutor, s32 stepMax, f32 valueStart, f32 valueEnd);  // objects: SMG2 addition
    void setNerveAtStep(NerveExecutor* pExecutor, const Nerve* pNerve, s32 step);
};  // namespace MR

namespace MR {
    f32 calcNerveEaseInOutRate(const NerveExecutor*, s32, s32, s32);
    f32 calcNerveParabolaValue(const NerveExecutor*, s32, f32, f32);
}

namespace MR {
    f32 calcNerveEaseInValue(const NerveExecutor*, s32, f32, f32);
    f32 calcNerveRate(const NerveExecutor*, s32, s32);
}
