#include "workgroup_protocol.hpp"
#include <cassert>
#include <cstdio>
#include <initializer_list>
using namespace PetariNative::Platform::AudioWorkgroup;
int main() {
    assert(Protocol::effectiveComputation(500,2000)==1000);
    assert(Protocol::effectiveComputation(1500,2000)==1500);
    Protocol::Cycle c;
    Origin a{{0x1000, 1}, 1, 1000000000};
    assert(c.begin(a, 1, 7, 32000, 1000000000, 0x2000, 0x3000));
    assert(c.deadline == 1014000000 && c.subframes * 80 == 560);
    assert(!c.finish()); // An AI registration cannot end pending DSP work.
    for (unsigned n = 0; n < 6; ++n) {
        assert(c.completeSubframe() && !c.ready() && !c.finish());
        assert(!c.begin(a, 2, 7, 32000, 1000000000, 0x2000, 0x3000)); // no overlap
    }
    assert(c.completeSubframe() && c.ready() && c.finish());
    assert(!c.completeSubframe() && !c.finish());
    assert(c.begin(a, 2, 10, 48000, 1000000000, 0x4000, 0x5000));
    assert(c.deadline == 1013333333 && c.subframes * 80 == 800);
    assert(c.cancel() && !c.cancel()); // pause/exit mid-frame; not completed
    for (auto o : {Origin{}, Origin{{0x1000,0},1,1}, Origin{{0x1000,1},0,1}, Origin{{0x1000,1},1,0}})
        assert(!c.begin(o, 3, 7, 32000, 1000000000, 0x2000, 0x3000));
    assert(!c.begin(a, 3, 0, 32000, 1000000000, 0x2000, 0x3000));
    assert(!c.begin(a, 3, 256, 32000, 1000000000, 0x2000, 0x3000));
    assert(!c.begin(a, 3, 7, 44100, 1000000000, 0x2000, 0x3000));
    assert(!c.begin(a, 3, 7, 32000, 0, 0x2000, 0x3000));
    // UINT64_MAX ticks/second does not itself overflow the deadline: seven
    // 32 kHz subframes have a budget of floor(ticks * 7 / 500). Compute an
    // independent expected result by quotient/remainder, without the wide
    // intermediate used by Cycle::begin. The old rejection assertion mistook
    // an overflowing 64-bit intermediate for an overflowing final deadline.
    constexpr auto maxClockBudget = (UINT64_MAX / 500) * 7 +
                                    ((UINT64_MAX % 500) * 7) / 500;
    assert(c.begin(a, 3, 7, 32000, UINT64_MAX, 0x2000, 0x3000));
    assert(c.deadline == a.queuedMach + maxClockBudget);
    assert(c.cancel());
    auto lastRepresentable = a;
    lastRepresentable.queuedMach = UINT64_MAX - maxClockBudget;
    assert(c.begin(lastRepresentable, 3, 7, 32000, UINT64_MAX, 0x2000, 0x3000));
    assert(c.deadline == UINT64_MAX && c.cancel());
    ++lastRepresentable.queuedMach;
    assert(!c.begin(lastRepresentable, 3, 7, 32000, UINT64_MAX, 0x2000, 0x3000));
    assert(!c.active); // A true deadline overflow must not activate a cycle.
    assert(!c.begin(a, 3, 7, 32000, 1, 0x2000, 0x3000)); // budget rounds to zero
    auto overflow = a; overflow.queuedMach = UINT64_MAX - 10;
    assert(!c.begin(overflow, 3, 7, 32000, 1000000000, 0x2000, 0x3000));
    assert(!c.begin(a, 3, 7, 32000, 1000000000, 0x2000, 0x2000));
    Protocol::Membership m;
    assert(!m.adopt(1,1,1,0) && !m.adopt(1,1,0,22) && !m.joined);
    assert(m.adopt(1,1,0,0) && !m.adopt(1,2,0,0));
    assert(!m.leave(2) && m.joined && m.leave(1) && !m.leave(1));
    assert(m.adopt(2,2,0,0) && m.epoch == 2 && m.leave(2));
    assert(!(Identity{0x1000,1} == Identity{0x1000,2})); // address reuse is not identity
    std::puts("audio workgroup production ledger/ownership PASS");
}
