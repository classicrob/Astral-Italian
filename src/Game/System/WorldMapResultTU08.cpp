#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/Screen/GalaxyConfirmLayout.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/Util/ActorCameraUtil.hpp"

extern "C" {
    void fn_804F6B10(WorldMapResultCommand*);
    void* fn_804F6E90(WorldMapResultCommand*);
    void* fn_804F6FE0(WorldMapResultCommand*);
    void fn_804F8380(WorldMapResultCommand*, const char*);
    void fn_804F8470(WorldMapResultCommand*, const char*, s32);
    void fn_804F7580(WorldMapResultCommand*, const char*);
    void fn_804F82C0(WorldMapResultCommand*, s32, s32);
    bool fn_804F72D0(WorldMapResultCommand*);
    void fn_804E8480(WorldMapResultCommand*, const TVec3f*);
    void fn_804E84D0(WorldMapResultCommand*, const TVec3f*);
    void fn_804E89F0(WorldMapResultCommand*, void*);
    void fn_804E88A0(WorldMapResultCommand*, const TVec3f*, const TVec3f*, s32);
    void fn_804E8780(WorldMapResultCommand*);
    LiveActor* fn_804E8470(WorldMapResultCommand*);
    TVec3f* fn_804E8510(WorldMapResultCommand*);
    s32 fn_8046F870(void*);
    s32 fn_8046F8D0(void*);
    s32 fn_804F7620(WorldMapResultCommand*, f32);
    bool fn_804F6F20(WorldMapResultCommand*);
    bool fn_804F6F60(WorldMapResultCommand*);
    bool fn_804F6FA0(WorldMapResultCommand*);
}

extern "C" {
    extern const char* lbl_807D0550;
    extern const char* lbl_807D0554;
    extern const char* lbl_807D0558;
    extern const char* lbl_807D055C;
    extern const char* lbl_807D0560;
    extern const char* lbl_807D0564;
    extern const char* lbl_807D0568;
    extern const char* lbl_807D056C;
}

extern f32 lbl_807E1D00;
extern f32 lbl_807E1D08;
extern f32 lbl_807E1D0C;
extern s32 lbl_807E1D10;
extern s32 lbl_807E1D14;
extern f32 lbl_807E1D18;
extern s32 lbl_807E1D1C;
extern s32 lbl_807E1D20;
extern f32 lbl_807E1D24;
extern s32 lbl_807E1D28;
extern s32 lbl_807E1D2C;
extern f32 lbl_807E1D30;
extern s32 lbl_807E1D34;
extern s32 lbl_807E1D38;

/// Camera vectors of the galaxy entry (4 entries of 3 vectors each).
struct WorldMapResultEnterGalaxyTable {
    /* 0x00 */ TVec3f mA;
    /* 0x0C */ TVec3f mB;
    /* 0x18 */ TVec3f mC;
    /* 0x24 */ TVec3f mD;
};
extern WorldMapResultEnterGalaxyTable lbl_8072BA38;

namespace MR {
    void setDepthOfFieldBlurDistance(s32, s32);
    void initAnimCamera(const LiveActor*, const ActorCameraInfo*, const char*);
    void startAnimCameraTargetPlayer(const LiveActor*, const ActorCameraInfo*, const char*, s32, bool, f32);
    void createDepthOfFieldBlur();
    void setDepthOfFieldBlurIntensity(f32);
    void closeSystemWipeWhiteFade(s32);
    bool isUseScenarioSelectMusicOneStar();
    void stopStageBGM(u32);
    void startStarPointerModeDemo(void*);
    void setImageEffectControlAuto();
    TVec3f getCamZdir();
    TVec3f getCamYdir();
    bool isLessEqualStep(const LiveActor*, s32);
    f32 calcNerveEaseInRate(const NerveExecutor*, s32);
    f32 calcNerveEaseInOutRate(const NerveExecutor*, s32);
}

/// Galaxy-entry step ("State[ギャラクシーに入る]"): zooms the camera into the galaxy and starts the demo.
WMR_STATE_DECL(WorldMapResultStateEnterGalaxy, virtual void control(); void exeZoomIn(); void exeWait(); void exeDemo(); void exeZoomOut();
               /* 0x10 */ TVec3f mPos; /* 0x1C */ TVec3f* mTargetPos; /* 0x20 */ const char* mCameraName; /* 0x24 */ const char* mAnimName;
               /* 0x28 */ GalaxyConfirmLayout* mConfirmLayout; /* 0x2C */ ActorCameraInfo* mCamInfo; /* 0x30 */ const s32* mBlurNear;
               /* 0x34 */ const s32* mBlurFar; /* 0x38 */ f32 mRate; /* 0x3C */ bool mFlag;);

namespace NrvWorldMapResultStateEnterGalaxy {
    WMR_NERVE_DECL(WorldMapResultStateEnterGalaxyZoomIn);
    WMR_NERVE_DECL(WorldMapResultStateEnterGalaxyWait);
    WMR_NERVE_DECL(WorldMapResultStateEnterGalaxyDemo);
    WMR_NERVE_DECL(WorldMapResultStateEnterGalaxyZoomOut);
};  // namespace NrvWorldMapResultStateEnterGalaxy

WorldMapResultStateEnterGalaxy::WorldMapResultStateEnterGalaxy(WorldMapResultCommand* pHost) : ActorStateBase< WorldMapResultCommand >("State[ギャラクシーに入る]", pHost) {
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
    mTargetPos = NULL;
    mCameraName = NULL;
    mAnimName = NULL;
    mConfirmLayout = NULL;
    mBlurNear = NULL;
    mBlurFar = NULL;
    mRate = 0.0f;
    mFlag = false;
}

void WorldMapResultStateEnterGalaxy::init() {
    fn_804F8380(mHost, lbl_807D0560);
    fn_804F8380(mHost, lbl_807D0564);
    fn_804F8380(mHost, lbl_807D0568);
    fn_804F8380(mHost, lbl_807D056C);
    mConfirmLayout = new GalaxyConfirmLayout();
    mConfirmLayout->initWithoutIter();
    initNerve(GET_NERVE(WorldMapResultStateEnterGalaxy, WorldMapResultStateEnterGalaxyZoomIn), 0);
    mCamInfo = new ActorCameraInfo(-1, 0);
    MR::initAnimCamera(MR::getPlayerDemoActor(), mCamInfo, lbl_807D0550);
    MR::initAnimCamera(MR::getPlayerDemoActor(), mCamInfo, lbl_807D0554);
    MR::initAnimCamera(MR::getPlayerDemoActor(), mCamInfo, lbl_807D0558);
    MR::initAnimCamera(MR::getPlayerDemoActor(), mCamInfo, lbl_807D055C);
    MR::createDepthOfFieldBlur();
    mIsDead = true;
}

void WorldMapResultStateEnterGalaxy::appear() {
    mIsDead = false;
    if (fn_804F6F20(mHost)) {
        mTargetPos = &lbl_8072BA38.mB;
        mCameraName = lbl_807D0564;
        mAnimName = lbl_807D0554;
        mBlurNear = &lbl_807E1D1C;
        mBlurFar = &lbl_807E1D20;
        mRate = lbl_807E1D18;
    } else if (fn_804F6F60(mHost)) {
        mTargetPos = &lbl_8072BA38.mC;
        mCameraName = lbl_807D0568;
        mAnimName = lbl_807D0558;
        mBlurNear = &lbl_807E1D28;
        mBlurFar = &lbl_807E1D2C;
        mRate = lbl_807E1D24;
    } else if (fn_804F6FA0(mHost)) {
        mTargetPos = &lbl_8072BA38.mD;
        mCameraName = lbl_807D056C;
        mAnimName = lbl_807D055C;
        mBlurNear = &lbl_807E1D34;
        mBlurFar = &lbl_807E1D38;
        mRate = lbl_807E1D30;
    } else {
        mTargetPos = &lbl_8072BA38.mA;
        mCameraName = lbl_807D0560;
        mAnimName = lbl_807D0550;
        mBlurNear = &lbl_807E1D10;
        mBlurFar = &lbl_807E1D14;
        mRate = lbl_807E1D0C;
    }
    mFlag = fn_804F72D0(mHost);
    fn_804F8470(mHost, mCameraName, -1);
    fn_804F7580(mHost, "Wait");
    mPos = *fn_804E8510(mHost);
    setNerve(GET_NERVE(WorldMapResultStateEnterGalaxy, WorldMapResultStateEnterGalaxyZoomIn));
}

void WorldMapResultStateEnterGalaxy::kill() {
    mIsDead = true;
}

void WorldMapResultStateEnterGalaxy::control() {
    fn_804F6B10(mHost);
    MR::setDepthOfFieldBlurIntensity(lbl_807E1D08);
    MR::setDepthOfFieldBlurDistance(*mBlurNear, *mBlurFar);
}

WorldMapResultStateEnterGalaxy::~WorldMapResultStateEnterGalaxy() {
}

void WorldMapResultStateEnterGalaxy::exeZoomIn() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(fn_804E8470(mHost), "ZoomInGalaxy", -1, -1, -1);
    }
    if (mFlag) {
        fn_804E89F0(mHost, fn_804F6E90(mHost));
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 30);
    TVec3f sum = mPos + *mTargetPos;
    TVec3f scaled = sum * rate;
    TVec3f rest = mPos * (lbl_807E1D00 - rate);
    TVec3f merged = rest + scaled;
    fn_804E8480(mHost, &merged);
    f32 ease = MR::calcNerveEaseInRate(this, 30);
    TVec3f uniform((lbl_807E1D00 - mRate) + ((lbl_807E1D00 - ease) * mRate));
    fn_804E84D0(mHost, &uniform);
    if (MR::isStep(this, 16)) {
        MR::turnOnDepthOfField(false);
    }
    if (MR::isGreaterEqualStep(this, 30)) {
        setNerve(GET_NERVE(WorldMapResultStateEnterGalaxy, WorldMapResultStateEnterGalaxyWait));
    }
}

void WorldMapResultStateEnterGalaxy::exeWait() {
    if (MR::isFirstStep(this)) {
        mConfirmLayout->appear();
    }
    fn_804E89F0(mHost, fn_804F6E90(mHost));
    if (fn_8046F870(mConfirmLayout) != 0 && fn_8046F8D0(mConfirmLayout) != 0) {
        setNerve(GET_NERVE(WorldMapResultStateEnterGalaxy, WorldMapResultStateEnterGalaxyDemo));
        return;
    }
    if (fn_8046F870(mConfirmLayout) != 0 && fn_8046F8D0(mConfirmLayout) == 0) {
        setNerve(GET_NERVE(WorldMapResultStateEnterGalaxy, WorldMapResultStateEnterGalaxyZoomOut));
    }
}

void WorldMapResultStateEnterGalaxy::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::showPlayer();
        fn_804E8780(mHost);
        MR::startAction(MR::getPlayerDemoActor(), mAnimName);
        fn_804F7580(mHost, lbl_807D0550);
        MR::startAnimCameraTargetPlayer(MR::getPlayerDemoActor(), mCamInfo, mAnimName, -1, false, 1.0f);
        TVec3f camZ = MR::getCamZdir();
        TVec3f negCamZ(-camZ.x, -camZ.y, -camZ.z);
        s32 unk = fn_804F7620(mHost, -camZ.x);
        TVec3f camY = MR::getCamYdir();
        fn_804E88A0(mHost, &negCamZ, &camY, unk);
    }
    MR::isLessEqualStep(this, 170);
    if (MR::isStep(this, 170)) {
        MR::closeSystemWipeWhiteFade(-1);
        if (!MR::isUseScenarioSelectMusicOneStar()) {
            MR::stopStageBGM(0x50);
        }
    }
    if (MR::isBckStoppedPlayer()) {
        setNerve(GET_NERVE(WorldMapResultStateEnterGalaxy, WorldMapResultStateEnterGalaxyZoomOut));
        fn_804F6FE0(mHost);
        MR::startStarPointerModeDemo(mHost);
    }
}

void WorldMapResultStateEnterGalaxy::exeZoomOut() {
    if (MR::isFirstStep(this)) {
        MR::setImageEffectControlAuto();
        fn_804F82C0(mHost, -1, -1);
        MR::startActionSound(fn_804E8470(mHost), "ZoomOutGalaxy", -1, -1, -1);
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 30);
    TVec3f scaled = mPos * rate;
    TVec3f sum = mPos + *mTargetPos;
    TVec3f rest = sum * (lbl_807E1D00 - rate);
    TVec3f merged = rest + scaled;
    fn_804E8480(mHost, &merged);
    TVec3f uniform((lbl_807E1D00 - mRate) + (rate * mRate));
    fn_804E84D0(mHost, &uniform);
    fn_804E89F0(mHost, fn_804F6E90(mHost));
    if (MR::isGreaterEqualStep(this, 30)) {
        MR::showPlayer();
        kill();
    }
}

namespace NrvWorldMapResultStateEnterGalaxy {
    WMR_NERVE_DEF(WorldMapResultStateEnterGalaxyZoomOut, WorldMapResultStateEnterGalaxy, ZoomOut);
    WMR_NERVE_DEF(WorldMapResultStateEnterGalaxyDemo, WorldMapResultStateEnterGalaxy, Demo);
    WMR_NERVE_DEF(WorldMapResultStateEnterGalaxyWait, WorldMapResultStateEnterGalaxy, Wait);
    WMR_NERVE_DEF(WorldMapResultStateEnterGalaxyZoomIn, WorldMapResultStateEnterGalaxy, ZoomIn);
};  // namespace NrvWorldMapResultStateEnterGalaxy
