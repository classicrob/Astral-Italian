#include "Game/MapObj/CloudStep.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapPartsRailMover.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SpringValue.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

// PLACEHOLDER(objects): unnamed target symbol fn_80068E40, declared extern "C" so the reloc name matches; resolve when fn_80068E40 gets its real name in symbols.txt
extern "C" void fn_80068E40(SpringValue*, f32);

namespace NrvCloudStep {
    class CloudStepNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }

        static CloudStepNrvWait sInstance;
    };
    CloudStepNrvWait CloudStepNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(CloudStepNrvAppear, CloudStep, Appear);
    NEW_NERVE(CloudStepNrvDisappear, CloudStep, Disappear);
};  // namespace NrvCloudStep

CloudStep::CloudStep(const char* pName) : LiveActor(pName) {
    mRailMover = nullptr;
    mClipPos.zero();
    mBounceTimer = 0;
    mIsHeavy = false;
    mSpring = new SpringValue(0.0f, 0.0f, 0.03f, 0.92f, 0.0f);
}

void CloudStep::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    const char* pName = nullptr;
    MR::getObjectName(&pName, rIter);
    if (!MR::isEqualString(pName, "CloudStep")) {
        mIsHeavy = true;
    }
    if (MR::isConnectedWithRail(rIter)) {
        mRailMover = new MapPartsRailMover(this);
        mRailMover->init(rIter);
        mRailMover->initAfterPlacement();
    }
    MR::onCalcGravity(this);
    MR::initMapPartsClipping(this, rIter, &mClipPos, false);
    initNerve(&NrvCloudStep::CloudStepNrvAppear::sInstance, 0);
    MR::useStageSwitchAwake(this, rIter);
    if (MR::initUseStageSwitchReadB(this, rIter)) {
        setNerve(&NrvCloudStep::CloudStepNrvWait::sInstance);
    }
    MR::listenStageSwitchOnOffB(this, MR::Functor(this, &CloudStep::startAppear), MR::Functor(this, &CloudStep::startDisappear));
    MR::useStageSwitchSyncAppear(this, rIter);
    MR::startAction(this, "Wait");
}

void CloudStep::control() {
    if (mBounceTimer > 0) {
        mBounceTimer--;
        mSpring->update();
        fn_80068E40(mSpring, 60.0f);
    }
}

// NONMATCHING: base scale math (frsqrte path) simplified, ~45%.
void CloudStep::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    TPos3f mtx;
    mtx = *reinterpret_cast< const TPos3f* >(getBaseMtx());
    TVec3f offset(mGravity);
    offset *= mSpring->mSpringValue;
    MR::addTransMtx(mtx.mMtx, offset);
    MR::setBaseTRMtx(this, mtx);
    f32 k = mIsHeavy ? 0.25f : 0.12f;
    f32 s = 1.0f - k * (mSpring->mSpringValue / 60.0f);
    if (s < 0.05f) {
        s = 0.05f;
    }
    f32 r = 1.0f / s;
    MR::setBaseScale(this, TVec3f(r, s, r));
}

bool CloudStep::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDropFloor(msg)) {
        setNerve(&NrvCloudStep::CloudStepNrvDisappear::sInstance);
        return true;
    }
    return false;
}

bool CloudStep::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isPlayerElementModeBee()) {
        return false;
    }
    if (MR::isPlayerElementModeCloud()) {
        if (MR::isMsgFloorTouch(msg)) {
            mSpring->mVelocity += mIsHeavy ? 8.0f : 5.0f;
            mBounceTimer = 10;
            return false;
        }
        if (MR::isMsgWallTouch(msg) || MR::isMsgCeilTouch(msg)) {
            setNerve(&NrvCloudStep::CloudStepNrvDisappear::sInstance);
            MR::invalidateCollisionParts(this);
        }
    }
    return false;
}

void CloudStep::exeAppear() {
    if (mRailMover != nullptr) {
        mRailMover->movement();
    }
    MR::setPosition(this, mRailMover->_28);
}

void CloudStep::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Disappear");
        MR::invalidateClipping(this);
        MR::invalidateCollisionParts(this);
    }
    if (!mIsHeavy) {
        if (MR::isStep(this, 20)) {
            MR::invalidateShadow(this, nullptr);
        }
        if (MR::isStep(this, 79)) {
            MR::validateShadow(this, nullptr);
        }
    }
    if (MR::isActionEnd(this)) {
        MR::validateCollisionParts(this);
        MR::validateClipping(this);
        MR::startAction(this, "Wait");
        if (MR::isValidSwitchB(this) && !MR::isOnSwitchB(this)) {
            setNerve(&NrvCloudStep::CloudStepNrvWait::sInstance);
        } else {
            setNerve(&NrvCloudStep::CloudStepNrvAppear::sInstance);
        }
    }
}

void CloudStep::startAppear() {
    if (isNerve(&NrvCloudStep::CloudStepNrvWait::sInstance)) {
        setNerve(&NrvCloudStep::CloudStepNrvAppear::sInstance);
    }
}

void CloudStep::startDisappear() {
    if (isNerve(&NrvCloudStep::CloudStepNrvAppear::sInstance)) {
        setNerve(&NrvCloudStep::CloudStepNrvWait::sInstance);
    }
}

CloudStep::~CloudStep() {
}
