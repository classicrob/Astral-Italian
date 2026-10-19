#include "Game/RhythmLib/AudRhythmWrap.hpp"
#include "Game/Util.hpp"
#include "Game/RhythmLib/AudRhythmMeSystem.hpp"

namespace AudRhythmWrap {
    f32 wrapBeatPosition(s32 n, f32 t) {
        s32 k = (s32)t;
        s32 q = k / n;
        return t - (f32)(q * n);
    }
}

void AudRhythmWrap::rhythmProc() {
    AudRhythmMeSystem::getInstance()->rhythmProc();
}

bool AudRhythmWrap::isBeatTiming(s32 beatPeriod, f32 offset) {
    if (getBeatCount() < 2) {
        return false;
    }

    f32 a = getPrevBeatPhase();
    f32 b = getBeatPhase();
    if (a > 1.0f) {
        a = 1.0f;
    }
    if (b > 1.0f) {
        b = 1.0f;
    }

    f32 start = wrapBeatPosition(beatPeriod, a + ((f32)getPrevBeatCount() - 1.0f));
    f32 end = wrapBeatPosition(beatPeriod, b + ((f32)getBeatCount() - 1.0f));
    if (start > end) {
        start -= (f32)beatPeriod;
    }

    if (start <= offset && offset < end) {
        return true;
    }
    return false;
}

u32 AudRhythmWrap::getBeatCount() {
    return AudRhythmMeSystem::getInstance()->mRhythmHolders[AudRhythmMeSystem::getInstance()->mBgmIdx].mInfos[0].mBeatCount;
}

u32 AudRhythmWrap::getPrevBeatCount() {
    return AudRhythmMeSystem::getInstance()->mRhythmHolders[AudRhythmMeSystem::getInstance()->mBgmIdx].mInfos[0].mPrevBeatCount;
}

f32 AudRhythmWrap::getBeatPhase() {
    return AudRhythmMeSystem::getInstance()->mRhythmHolders[AudRhythmMeSystem::getInstance()->mBgmIdx].calcBeatPhase(0);
}

f32 AudRhythmWrap::getPrevBeatPhase() {
    return AudRhythmMeSystem::getInstance()->mRhythmHolders[AudRhythmMeSystem::getInstance()->mBgmIdx].calcPrevBeatPhase(0);
}
