#include "Game/MapObj/GCaptureTarget.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/GCapture.hpp"
#include "Game/MapObj/MapPartsRailMover.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/types.h>

namespace NrvGCaptureTarget {
    NEW_NERVE(GCaptureTargetNrvTryDemoAppear, GCaptureTarget, TryDemoAppear);
    NEW_NERVE(GCaptureTargetNrvAppear, GCaptureTarget, Appear);
    NEW_NERVE(GCaptureTargetNrvWait, GCaptureTarget, Wait);
    NEW_NERVE(GCaptureTargetNrvPointable, GCaptureTarget, Pointable);
    NEW_NERVE(GCaptureTargetNrvHitPointer, GCaptureTarget, HitPointer);
    NEW_NERVE(GCaptureTargetNrvActive, GCaptureTarget, Active);
};  // namespace NrvGCaptureTarget

GCaptureTarget::GCaptureTarget(const char* pName)
    : LiveActor(pName), mRailMover(), mStarAnimSpeed(1.0f), mCameraInfo(), mReleaseDistance(-1.0f), mPointableRange(2500.0f), mFarAwayColor(true) {
    MR::createGCapture();
}

void GCaptureTarget::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, "GCaptureTarget", false);
    MR::connectToSceneNoSilhouettedMapObjStrongLight(this);
    MR::getJMapInfoArg0WithInit(rIter, &mReleaseDistance);
    MR::getJMapInfoArg1NoInit(rIter, &mPointableRange);
    initEffectKeeper(0, nullptr, false);
    initSound(4, "GCaptureTarget", nullptr, TVec3f(0.0f));
    initNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvWait));
    MR::initShadowVolumeSphere(this, 50.0f);
    MR::setShadowDropLength(this, nullptr, 300.0f);
    MR::onCalcShadowOneTime(this, nullptr);
    MR::calcGravity(this);

    if (MR::isConnectedWithRail(rIter)) {
        initHitSensor(1);
        MR::addHitSensorMapObj(this, "body", 8, 0.0f, TVec3f(0.0f, 0.0f, 0.0f));
        MR::invalidateHitSensor(this, "body");
        initRailRider(rIter);
        mRailMover = new MapPartsRailMover(this);
        mRailMover->init(rIter);
    }

    MR::initStarPointerTarget(this, 200.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::setGroupClipping(this, rIter, 32);

    if (MR::tryRegisterDemoCast(this, rIter)) {
        makeActorDead();
    } else if (MR::useStageSwitchSyncAppear(this, rIter)) {
        MR::joinToGroupArray(this, rIter, "ブルースター出現グループ", 32);
        MR::initActorCamera(this, rIter, &mCameraInfo);
    }

    if (MR::initUseStageSwitchReadB(this, rIter)) {
        MR::listenStageSwitchOnB(this, MR::FunctorV0M<LiveActor*, void (LiveActor::*)()>(this, &LiveActor::kill));
    }

    s32 arg;
    MR::getJMapInfoArg2WithInit(rIter, &arg);
    mFarAwayColor = arg != 0;
    MR::startAction(this, "Wait");
    MR::startBrk(this, "Switch");
    MR::setBrkFrameAndStop(this, 1.0f);
    MR::useStageSwitchAwake(this, rIter);
}

void GCaptureTarget::initAfterPlacement() {
}

void GCaptureTarget::appear() {
    LiveActor::appear();
    MR::invalidateClipping(this);
    MR::deleteEffectAll(this);
    MR::hideModel(this);
    MR::startActionSound(this, "SyGcaptureAppear", -1, -1, -1);
    setNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvAppear));

    if (MR::isExistActorCamera(mCameraInfo)) {
        MR::requestStartDemo(this, "出現", GET_NERVE(GCaptureTarget, GCaptureTargetNrvAppear),
                             GET_NERVE(GCaptureTarget, GCaptureTargetNrvTryDemoAppear));
    }
}

void GCaptureTarget::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    MR::emitEffect(this, "TargetLight");
    setNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvWait));

    if (mRailMover != nullptr) {
        mRailMover->start();
    }
}

void GCaptureTarget::makeActorDead() {
    if (mRailMover != nullptr) {
        mRailMover->end();
    }

    LiveActor::makeActorDead();
    MR::deleteEffectAll(this);
    MR::unrequestGCaptureTarget(this);
}

void GCaptureTarget::startClipped() {
    LiveActor::startClipped();
    MR::deleteEffectAll(this);
}

void GCaptureTarget::endClipped() {
    LiveActor::endClipped();
    emitNerveEffect();
}

void GCaptureTarget::control() {
    if (mRailMover != nullptr) {
        mRailMover->movement();
        MR::setPosition(this, mRailMover->_28);
    }
}

void GCaptureTarget::exeTryDemoAppear() {
}

void GCaptureTarget::exeAppear() {
    if (MR::isFirstStep(this)) {
        if (MR::isExistActorCamera(mCameraInfo)) {
            MR::startActorCameraTargetSelf(this, mCameraInfo, 30);
            MR::callRequestMovementOnAllGroupMember(this);
        }
        MR::setBrkFrameAndStop(this, 1.0f);
    }

    if (MR::isStep(this, 30)) {
        emitNerveEffect();
        MR::showModel(this);
        MR::startAction(this, "Appear");
        MR::startActionSound(this, "SE_OJ_GCAPTURE_APPEAR", -1, -1, -1);
    }

    if (MR::isActionEnd(this)) {
        MR::startAction(this, "Wait");
    }

    if (MR::isGreaterStep(this, 150)) {
        if (MR::isExistActorCamera(mCameraInfo)) {
            MR::endDemoWaitCameraInterpolating(this, "出現");
            MR::endActorCamera(this, mCameraInfo, false, -1);
        }
        setNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvWait));
    }
}

void GCaptureTarget::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        emitNerveEffect();

        f32 frame;
        if (mFarAwayColor) {
            frame = 0.0f;
        } else {
            frame = 1.0f;
        }

        MR::setBrkFrameAndStop(this, frame);
    }

    mStarAnimSpeed -= 0.05f;
    if (mStarAnimSpeed < 0.0f) {
        mStarAnimSpeed = 0.0f;
    }
    MR::setBckRate(this, mStarAnimSpeed);

    if (MR::isNearPlayer(this, mPointableRange)) {
        setNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvPointable));
    }
}

void GCaptureTarget::exePointable() {
    if (MR::isFirstStep(this)) {
        emitNerveEffect();
        MR::setBrkFrameAndStop(this, 1.0f);
    }

    if (!MR::isJudgedToClipFrustum(mPosition, 50.0f)) {
        MR::noticeInTouchableRange();
    }

    mStarAnimSpeed += 0.05f;
    if (mStarAnimSpeed > 0.5f) {
        mStarAnimSpeed = 0.5f;
    }
    MR::setBckRate(this, mStarAnimSpeed);

    if (MR::isStarPointerPointing(this, 0, true, "弱") && MR::requestGCaptureTarget(this)) {
        MR::invalidateClipping(this);
        setNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvHitPointer));
    } else if (!MR::isNearPlayer(this, mPointableRange)) {
        setNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvWait));
    }
}

void GCaptureTarget::exeHitPointer() {
    if (MR::isFirstStep(this)) {
        MR::setBrkFrameAndStop(this, 1.0f);
    }

    if (!MR::isRequestedGCaptureTarget(this)) {
        setNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvWait));
    } else {
        MR::requestStarPointerModeBlueStarReady(this);
        if (MR::isFirstStep(this)) {
            emitNerveEffect();
        }

        if (MR::isStep(this, 3)) {
            MR::startCSSound("CS_STAR_POWER", nullptr, 0, 0, 1.0f);
        }

        MR::noticeInTouchableRange();
        MR::startActionSound(this, "SE_OJ_LV_MAGIC_PNT_G_POINT", -1, -1, -1);

        mStarAnimSpeed += 0.2f;
        if (mStarAnimSpeed > 1.0f) {
            mStarAnimSpeed = 1.0f;
        }
        MR::setBckRate(this, mStarAnimSpeed);

        bool isNear = MR::isNearPlayer(this, mPointableRange);
        if (MR::isStarPointerPointing(this, 0, true, "弱") && isNear) {
            MR::requestGCaptureTarget(this);
        }

        if (!isNear) {
            MR::unrequestGCaptureTarget(this);
        }
    }
}

void GCaptureTarget::exeActive() {
    if (MR::isFirstStep(this)) {
        emitNerveEffect();
        MR::startAction(this, "React");
        MR::startActionSound(this, "SE_OJ_MAGIC_PNT_G_ON", -1, -1, -1);
        MR::setBrkFrameAndStop(this, 1.0f);
    }

    MR::noticeInTouchableRange();
    MR::startActionSound(this, "SE_OJ_LV_MAGIC_PNT_G_PULL", MR::calcDistanceToPlayer(this), -1, -1);
    MR::startActionSound(this, "SE_OJ_LV_MAGIC_PNT_G_POINT", -1, -1, -1);
}

void GCaptureTarget::decidedTarget() {
    if (!isNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvActive))) {
        setNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvActive));
    }
}

void GCaptureTarget::releasedTarget() {
    if (!isNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvWait))) {
        MR::startAction(this, "Wait");
        setNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvWait));
    }
}

void GCaptureTarget::emitNerveEffect() {
    if (isNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvAppear))) {
        MR::emitEffect(this, "TargetLight");
        MR::deleteEffect(this, "Touch");
        MR::deleteEffect(this, "Active");
    } else if (isNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvWait))) {
        MR::deleteEffect(this, "TargetLight");
        MR::deleteEffect(this, "Touch");
        MR::deleteEffect(this, "Active");
    } else if (isNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvPointable))) {
        MR::emitEffect(this, "TargetLight");
        MR::emitEffect(this, "TouchAble");
        MR::deleteEffect(this, "Touch");
        MR::deleteEffect(this, "Active");
    } else if (isNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvHitPointer))) {
        MR::emitEffect(this, "TargetLight");
        MR::emitEffect(this, "Touch");
        MR::deleteEffect(this, "Active");
    } else if (isNerve(GET_NERVE(GCaptureTarget, GCaptureTargetNrvActive))) {
        MR::emitEffect(this, "TargetLight");
        MR::deleteEffect(this, "Touch");
        MR::emitEffect(this, "Active");
    }
}

bool GCaptureTarget::isReleaseForce() const {
    return MR::isDead(this);
}

void GCaptureTarget::getTargetPosition(TVec3f* pPos) {
    pPos->x = mPosition.x;
    pPos->y = mPosition.y;
    pPos->z = mPosition.z;
}

f32 GCaptureTarget::releaseDistance() const {
    return mReleaseDistance;
}

f32 GCaptureTarget::getPointableRange() const {
    return mPointableRange;
}
