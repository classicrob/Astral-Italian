#include "Game/GameAudio/AudEffectDirector.hpp"
#include "Game/AudioLib/AudEffector.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

struct AudEffectData {
    /* 0x00 */ f32 mFxSend;
    /* 0x04 */ s32 mCutoff;
    /* 0x08 */ f32 mVolume;
    /* 0x0C */ u32 mFadeTime;
    /* 0x10 */ s32 mSpecialFx;
    /* 0x14 */ s32 _14;
};

namespace {
    const AudEffectData cAudEffectDataList[] = {
        {0.0f, 127, 1.0f, 90, -1, 0}, {1.0f, 127, 1.0f, 90, -1, 0}, {0.0f, 10, 1.0f, 90, -1, 0}, {1.0f, 10, 1.0f, 90, 0, 0}, {1.0f, 80, 1.0f, 90, -1, 0},
#ifdef GALAXY_NATIVE
        // SB4E01 lbl_806488D0 has 7 entries (0xA8 bytes); types 5 and 6 are used (AudSceneMgr requests 6).
        // PLACEHOLDER(port): native-only until the Wii table carries them too; resolve when the owner completes the table for both builds.
        {0.7f, 127, 1.0f, 90, -1, 0}, {0.54f, 127, 1.0f, 90, -1, 0},
#endif
    };
};  // namespace

AudEffectDirector::AudEffectDirector() : NameObj("オーディオエフェクトディレクター"), mEffectType(0), mAppliedEffectType(0), mPriority(0), mSavedEffectType(0) {
}

void AudEffectDirector::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, MR::MovementType_AudEffectDirector, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);
    setAudioEffectParam(mEffectType);
}

void AudEffectDirector::movement() {
    if (mAppliedEffectType != mEffectType) {
        setAudioEffectParam(mEffectType);
        mAppliedEffectType = mEffectType;
    }

    mPriority = 0;
}

void AudEffectDirector::setEffectType(s32 a1, s32 a2, s32 saveType) {
    if (a2 < 0) {
        a2 = 0;
    }

    if (a2 > 127) {
        a2 = 127;
    }

    if (a2 >= mPriority) {
        mEffectType = a1;
        mPriority = a2;
    }
    if (saveType) {
        mSavedEffectType = a1;
    }
}

void AudEffectDirector::restoreEffectType() {
    setEffectType(mSavedEffectType, 0, 0);
}

void AudEffectDirector::setAudioEffectParam(s32 type) {
    AudEffector* pAudEffector = AudWrap::getSystem()->getAudEffector();

    pAudEffector->setFxSend(::cAudEffectDataList[type].mFxSend);
    pAudEffector->setCutoff(::cAudEffectDataList[type].mCutoff);
    pAudEffector->setSpecialFx(::cAudEffectDataList[type].mSpecialFx);
}
