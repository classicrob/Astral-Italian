#include "Game/MapObj/ExterminationSwitch.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

namespace NameObjFactory {
    LiveActor* initChildObj(const JMapInfoIter&, int);
};  // namespace NameObjFactory

extern "C" const char lbl_806A0480[];
extern "C" const char lbl_806A04A4[];
extern "C" const char lbl_806A04B4[];
extern "C" const char lbl_806A04D0[];

extern "C" void fn_8005A2A0(s32);
extern "C" void fn_8005A2E0();
extern "C" void fn_8005A310(s32);
extern "C" void fn_80024F50();

namespace NrvExterminationSwitch {
    NEW_NERVE(ExterminationSwitchNrvWait, ExterminationSwitch, Wait);
    NEW_NERVE(ExterminationSwitchNrvCheck, ExterminationSwitch, Check);
    NEW_NERVE(ExterminationSwitchNrvAppear, ExterminationSwitch, Appear);
    NEW_NERVE(ExterminationSwitchNrvOn, ExterminationSwitch, On);
};  // namespace NrvExterminationSwitch

ExterminationSwitch::ExterminationSwitch(const char* pName) : LiveActor(pName) {
    mCheckerType = 0;
    mStep = 0;
    mIsShowCount = false;
    if (MR::isEqualString(pName, lbl_806A0480)) {
        mCheckerType = 1;
    }
}

ExterminationSwitch::~ExterminationSwitch() {}

void ExterminationSwitch::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    s32 childNum = MR::getChildObjNum(rIter);
    mGroup = new LiveActorGroup(lbl_806A04A4, childNum);
    for (s32 i = 0; i < childNum; i++) {
        LiveActor* pChild = NameObjFactory::initChildObj(rIter, i);
        mGroup->registerActor(pChild);
    }

    MR::invalidateClipping(this);
    MR::useStageSwitchAwake(this, rIter);

    s32 powerStarId = -1;
    MR::getJMapInfoArg0NoInit(rIter, &powerStarId);
    MR::getJMapInfoArg1NoInit(rIter, &mStep);

    if (mCheckerType == 1) {
        mIsShowCount = true;
        mStep = 0x69;
        bool arg2 = false;
        MR::getJMapInfoArg2NoInit(rIter, &arg2);
        if (arg2) {
            mIsShowCount = false;
        }
        MR::declarePowerStar(this, powerStarId);
    } else if (mCheckerType == 0) {
        MR::initUseStageSwitchWriteA(this, rIter);
    }

    if (mIsShowCount) {
        fn_8005A2A0(mGroup->getObjNum());
        fn_8005A2E0();
    }

    initSound(2, lbl_806A04B4, nullptr, TVec3f(0.0f));
    initNerve(&NrvExterminationSwitch::ExterminationSwitchNrvWait::sInstance, 0);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void ExterminationSwitch::exeWait() {
    if (MR::isFirstStep(this)) {
        for (s32 i = 0; i < mGroup->getObjNum(); i++) {
            MR::isDead(mGroup->getActor(i));
        }
    }
    setNerve(&NrvExterminationSwitch::ExterminationSwitchNrvCheck::sInstance);
}

void ExterminationSwitch::exeCheck() {
    if (MR::isStep(this, 60)) {
        for (s32 i = 0; i < mGroup->getObjNum(); i++) {
            MR::isDead(mGroup->getActor(i));
        }
    }

    s32 aliveNum = 0;
    for (s32 i = 0; i < mGroup->getObjNum(); i++) {
        if (!MR::isDead(mGroup->getActor(i))) {
            aliveNum++;
        }
    }

    if (mIsShowCount) {
        fn_8005A310(mGroup->getObjNum() - aliveNum);
    }

    if (aliveNum > 0) {
        return;
    }

    if (mCheckerType == 1) {
        if (!MR::isPlayerDead()) {
            MR::startActionSound(this, lbl_806A04D0, -1, -1, -1);
        }
        fn_80024F50();
        setNerve(&NrvExterminationSwitch::ExterminationSwitchNrvAppear::sInstance);
    } else if (mCheckerType == 0) {
        setNerve(&NrvExterminationSwitch::ExterminationSwitchNrvOn::sInstance);
    }
}

void ExterminationSwitch::exeAppear() {
    if (MR::isGreaterEqualStep(this, mStep)) {
        MR::requestAppearPowerStar(this);
        kill();
    }
}

void ExterminationSwitch::exeOn() {
    if (MR::isValidSwitchA(this)) {
        if (MR::isGreaterEqualStep(this, mStep)) {
            MR::onSwitchA(this);
            kill();
        }
    }
}
