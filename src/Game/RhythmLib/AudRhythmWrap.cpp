#include "Game/RhythmLib/AudRhythmWrap.hpp"
#include "Game/Util.hpp"
#include "Game/RhythmLib/AudRhythmMeSystem.hpp"

namespace AudRhythmWrap {
    f32 fn_8042A680(s32 n, f32 t) {
        s32 k = (s32)t;
        s32 q = k / n;
        return t - (f32)(q * n);
    }
}

void AudRhythmWrap::rhythmProc() {
    AudRhythmMeSystem::getInstance()->rhythmProc();
}

bool AudRhythmWrap::isBeatTiming(s32 beatPeriod, f32 offset) {
    if (fn_8042A840() < 2) {
        return false;
    }

    f32 a = fn_8042A8A0();
    f32 b = fn_8042A880();
    if (a > 1.0f) {
        a = 1.0f;
    }
    if (b > 1.0f) {
        b = 1.0f;
    }

    f32 start = fn_8042A680(beatPeriod, a + ((f32)fn_8042A860() - 1.0f));
    f32 end = fn_8042A680(beatPeriod, b + ((f32)fn_8042A840() - 1.0f));
    if (start > end) {
        start -= (f32)beatPeriod;
    }

    if (start <= offset && offset < end) {
        return true;
    }
    return false;
}

u32 AudRhythmWrap::fn_8042A840() {
    return AudRhythmMeSystem::getInstance()->mRhythmHolders[AudRhythmMeSystem::getInstance()->mBgmIdx].mInfos[0]._38;
}

u32 AudRhythmWrap::fn_8042A860() {
    return AudRhythmMeSystem::getInstance()->mRhythmHolders[AudRhythmMeSystem::getInstance()->mBgmIdx].mInfos[0]._3C;
}

f32 AudRhythmWrap::fn_8042A880() {
    return AudRhythmMeSystem::getInstance()->mRhythmHolders[AudRhythmMeSystem::getInstance()->mBgmIdx].fn_8042A230(0);
}

f32 AudRhythmWrap::fn_8042A8A0() {
    return AudRhythmMeSystem::getInstance()->mRhythmHolders[AudRhythmMeSystem::getInstance()->mBgmIdx].fn_8042A280(0);
}
