// Popo::control is inserted verbatim at the marker, with only its class
// qualifier changed to this fixture. The target type and constructor are real.
#include "Game/Player/YoshiLockOnTarget.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {
int checks;
void check(bool value, const char* message) {
    ++checks;
    if (!value) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
std::vector<int> events;
}
namespace NrvPopo {
struct PopoNrvTouch { static int sInstance; };
int PopoNrvTouch::sInstance;
}
struct ScaleProbe {
    YoshiLockOnTarget* target;
    const TVec3f* expected;
    MtxPtr matrix;
    int updates = 0;
    void updateNerve() {
        events.push_back(4);
        // Inspect pointer object bytes without evaluating a corrupted pointer.
        check(std::memcmp(&target->mMtx, &matrix, sizeof(matrix)) == 0, "matrix pointer bytes overwritten");
        check(std::memcmp(&target->mOffset, expected, sizeof(*expected)) == 0, "scale update saw wrong offset vector");
        ++updates;
    }
};
class PopoFixture {
public:
    YoshiLockOnTarget* mYoshiLockOn;
    TVec3f mLockOnOffset;
    ScaleProbe* mScaleController;
    bool targetable, pointing, touched = false;
    mutable int pointingCalls = 0;
    bool isStarPointerTargetable() const { events.push_back(1); return targetable; }
    void setNerve(const int* nerve) {
        check(nerve == &NrvPopo::PopoNrvTouch::sInstance, "same touch nerve");
        events.push_back(3); touched = true;
    }
    void control();
};
namespace MR {
bool isStarPointerPointing2PAttachTarget(const PopoFixture* actor) {
    events.push_back(2); ++actor->pointingCalls; return actor->pointing;
}
}

// __POPO_CONTROL__

struct GuardedTarget {
    std::uint64_t before = 0x12345678abcdef01ULL;
    YoshiLockOnTarget value;
    std::uint64_t after = 0xfedcba9876543210ULL;
    GuardedTarget(MtxPtr matrix) : value(nullptr, matrix, TVec3f(-1.0f, -2.0f, -3.0f)) {}
};
int main() {
    static_assert(sizeof(void*) == 8 && sizeof(TVec3f) == 12, "actual native64 ABI");
    static_assert(offsetof(YoshiLockOnTarget, mMtx) == 8, "matrix pointer at old raw offset");
    static_assert(offsetof(YoshiLockOnTarget, mOffset) == 16, "named vector native offset");
    static_assert(offsetof(YoshiLockOnTarget, mEligibleThisFrame) == 28, "flag follows whole vector");
    std::printf("Native layout: matrix=%zu offset=%zu eligible=%zu size=%zu\n",
                offsetof(YoshiLockOnTarget, mMtx), offsetof(YoshiLockOnTarget, mOffset),
                offsetof(YoshiLockOnTarget, mEligibleThisFrame), sizeof(YoshiLockOnTarget));
    // Ordinary finite vectors and exact copy payloads (signed zero, NaN, Inf).
    const std::uint32_t samples[][3] = {
        {0x3fa00000, 0xc0200000, 0x42c60000},
        {0x80000000, 0x7fc12345, 0x7f800000},
        {0x7f7fffff, 0xff7fffff, 0x00000001}
    };
    for (int gate = 0; gate < 4; ++gate) {
        for (const auto& sample : samples) {
            Mtx matrix = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}};
            GuardedTarget guarded(matrix);
            guarded.value.mEligibleThisFrame = true;
            TVec3f input;
            std::memcpy(&input, sample, sizeof(input));
            ScaleProbe scale{&guarded.value, &input, matrix};
            PopoFixture actor{&guarded.value, input, &scale, bool(gate & 1), bool(gate & 2)};
            unsigned char expected[sizeof(guarded)];
            std::memcpy(expected, &guarded, sizeof(guarded));
            std::memcpy(expected + offsetof(GuardedTarget, value) + offsetof(YoshiLockOnTarget, mOffset),
                        &input, sizeof(input));
            events.clear(); actor.control();
            check(std::memcmp(&guarded, expected, sizeof(guarded)) == 0, "only intended vector bytes change");
            check(guarded.value.mActor == nullptr && guarded.value.mMtx == matrix, "canonical pointer fields preserved");
            check(guarded.value.mEligibleThisFrame, "eligibility flag preserved");
            check(guarded.before == 0x12345678abcdef01ULL && guarded.after == 0xfedcba9876543210ULL, "bounds preserved");
            check(actor.pointingCalls == ((gate & 1) ? 1 : 0), "existing short circuit retained");
            check(actor.touched == (gate == 3), "existing touch condition retained");
            check(scale.updates == 1, "one scale update on every path");
            std::vector<int> order{1};
            if (gate & 1) order.push_back(2);
            if (gate == 3) order.push_back(3);
            order.push_back(4);
            check(events == order, "gate/nerve/vector-before-update ordering retained");
        }
    }
    std::printf("%d Popo typed lock-on checks passed\n", checks);
}
