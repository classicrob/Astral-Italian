#include "Game/Boss/DodoryuDemo.hpp"
#include "Game/Util.hpp"
#include "Game/Boss/Dodoryu.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    static const s32 sOpeningHideFrame = 60;
    // static const s32 sOpeningAppearFrame = _;
    // static const s32 sOpeningAppearRumbleStep = _;
    // static const s32 sOpeningHideRumbleStep = _;
    static const s32 sOpeningAppearSmileSoundStep = 55;
    // static const s32 sAppearRumbleStep = _;
    // static const s32 sDownRumbleStep = _;
    static const s32 sDieSoundStep = 242;
};  // namespace

namespace {
    NEW_NERVE(DodoryuDemoOpeningNrvOpeningDemoWait, DodoryuDemoOpening, DemoWait);
    NEW_NERVE(DodoryuDemoOpeningNrvOpeningUnderground, DodoryuDemoOpening, Underground);
    NEW_NERVE(DodoryuDemoOpeningNrvOpeningUndergroundHide, DodoryuDemoOpening, UndergroundHide);
    NEW_NERVE(DodoryuDemoOpeningNrvOpeningAppear, DodoryuDemoOpening, Appear);
    NEW_NERVE(DodoryuDemoOpeningNrvOpeningWalk, DodoryuDemoOpening, Walk);
    NEW_NERVE(DodoryuDemoOpeningNrvOpeningHideReady, DodoryuDemoOpening, HideReady);
    NEW_NERVE(DodoryuDemoOpeningNrvOpeningHide, DodoryuDemoOpening, Hide);
    NEW_NERVE(DodoryuDemoAppearNrvAppearDemoWait, DodoryuDemoAppear, DemoWait);
    NEW_NERVE(DodoryuDemoAppearNrvAppearPlay, DodoryuDemoAppear, Play);
    NEW_NERVE(DodoryuDemoAngryNrvAngryDemoWait, DodoryuDemoAngry, DemoWait);
    NEW_NERVE(DodoryuDemoAngryNrvAngryPlay, DodoryuDemoAngry, Play);
    NEW_NERVE(DodoryuDemoDownNrvDownDemoWait, DodoryuDemoDown, DemoWait);
    NEW_NERVE(DodoryuDemoDownNrvDownPlay, DodoryuDemoDown, Play);
};  // namespace

namespace {
    void resetCastDisposition(Dodoryu* pDodoryu) {
        TPos3f mtx;

        pDodoryu->killAllHillPieces();
        MR::findNamePos("ドドリュウ再セット", mtx.toMtxPtr());
        pDodoryu->setMtx(mtx);
        pDodoryu->snapToGround();

        MR::findNamePos("マリオ再セット", mtx.toMtxPtr());
        MR::setPlayerBaseMtx(mtx.toMtxPtr());
        MR::setPlayerStateWait();
    }
};  // namespace

namespace {
    const char* sOpeningDemoName = "OpeningDemo";
    const char* sAppearDemoName = "AppearDemo";
    const char* sAngryDemoName = "AngryDemo";
    const char* sDownDemoName = "DownDemo";
}

DodoryuDemoOpening::DodoryuDemoOpening(Dodoryu* pHost, const char* pName)
    : DodoryuStateBase(pHost, pName), mActorCameraInfo(), _9C(false) {
    initWithoutIter();
}

void DodoryuDemoOpening::init(const JMapInfoIter& rIter) {
    MR::invalidateClipping(this);
    initNerve(GET_NERVE_ANON(DodoryuDemoOpeningNrvOpeningUnderground));
    MR::initAnimCamera(mHost, &mActorCameraInfo, sOpeningDemoName);
    makeActorAppeared();
}

void DodoryuDemoOpening::control() {
    if (tryFinish()) {
        return;
    }
}

void DodoryuDemoOpening::start() {
    setNerve(GET_NERVE_ANON(DodoryuDemoOpeningNrvOpeningDemoWait));
    _9C = false;
}

void DodoryuDemoOpening::end() {
    MR::endAnimCamera(mHost, &mActorCameraInfo, sOpeningDemoName, -1, true);
    if (!MR::isDemoEndWaitCameraInterpolating()) {
        MR::endDemo(mHost, sOpeningDemoName);
    }

    TPos3f playerResetMtx;
    mHost->getPlayerResetMtx(&playerResetMtx, 0);
    mHost->mRumbleEnabled = false;

    MR::setPlayerBaseMtx(playerResetMtx.toMtxPtr());
    MR::setPlayerStateWait();
    MR::showPlayer();
}

void DodoryuDemoOpening::exeDemoWait() {
    if (!MR::tryStartDemo(mHost, sOpeningDemoName)) {
        return;
    }

    MR::invalidateShadowAll(mHost);
    MR::startActionSound(mHost, "DmMeetBoss");
    MR::startAnimCameraTargetOther(mHost, &mActorCameraInfo, sOpeningDemoName, CameraTargetArg(mHost->mInitialCameraTarget), 0, false, 1.0f);
    mHost->pauseOff();
    MR::hidePlayer();
    MR::startAction(mHost, "EffectWalk");
    mHost->startLeadHillBck("EffectWalk");
    MR::startBva(mHost, "Normal");
    MR::requestMovementOnImageEffect();
    mHost->shiftMoveStateRail(10.0f);
    mHost->killAllHillPieces();
    mHost->stopHillForce();
    MR::moveCoordToStartPos(mHost);
    setNerve(GET_NERVE_ANON(DodoryuDemoOpeningNrvOpeningUnderground));
}

void DodoryuDemoOpening::exeUnderground() {
    if (MR::isFirstStep(this)) {
        mHost->startHill();

        Dodoryu* pHost = mHost;
        pHost->mRumbleEnabled = true;
        pHost->mRumbleStep = 0;
    }

    if (MR::tryEndDemo(mHost, sOpeningDemoName, false)) {
        mHost->nextState();
        _9C = true;
        return;
    }

    MR::startActionSound(mHost, "BmLvDodoryuMoveUShallow");

    if (MR::isGreaterEqualStep(this, ::sOpeningHideFrame)) {
        setNerve(GET_NERVE_ANON(DodoryuDemoOpeningNrvOpeningUndergroundHide));
    }
}

void DodoryuDemoOpening::exeUndergroundHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "EffectWalkHide");
        mHost->startLeadHillBck("EffectWalkHide");
    }

    if (MR::tryEndDemo(mHost, sOpeningDemoName, false)) {
        mHost->nextState();
        _9C = true;
        return;
    }

    if (MR::isActionEnd(mHost)) {
        setNerve(GET_NERVE_ANON(DodoryuDemoOpeningNrvOpeningAppear));
    }
}

void DodoryuDemoOpening::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "OpeningAppear");
        MR::startBtk(mHost, "OpeningAppear");
        mHost->startLeadHillBck("Appear");
        MR::startActionSound(mHost, "BvDodoryuAppear1");
        MR::startActionSound(mHost, "BmDodoryuAppearHead");
    }

    if (MR::tryEndDemo(mHost, sOpeningDemoName, false)) {
        mHost->nextState();
        _9C = true;
        return;
    }

    MR::startActionSound(mHost, "BmLvDodoryuMoveGround");

    if (MR::isStep(this, ::sOpeningAppearSmileSoundStep)) {
        MR::startActionSound(mHost, "BvDodoryuFaintSmile");
    }


    if (MR::isActionEnd(mHost)) {
        setNerve(GET_NERVE_ANON(DodoryuDemoOpeningNrvOpeningWalk));
    }
}

void DodoryuDemoOpening::exeWalk() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "OpeningWalk");
        MR::startBtk(mHost, "OpeningWalk");
        mHost->startLeadHillBck("Walk");
    }

    if (MR::tryEndDemo(mHost, sOpeningDemoName, false)) {
        mHost->nextState();
        _9C = true;
        return;
    }

    MR::startActionSound(mHost, "BmLvDodoryuMoveGround");

    if (MR::isGreaterEqualStep(this, 60)) {
        mHost->mRumbleEnabled = false;

        setNerve(GET_NERVE_ANON(DodoryuDemoOpeningNrvOpeningHideReady));
    }
}

void DodoryuDemoOpening::exeHideReady() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Hide");
        mHost->startLeadHillBck("Hide");
    }

    if (MR::tryEndDemo(mHost, sOpeningDemoName, false)) {
        mHost->nextState();
        _9C = true;
        return;
    }

    MR::startActionSound(mHost, "BmLvDodoryuMoveGround");

    if (MR::isActionEnd(mHost)) {
        MR::startAction(mHost, "EffectWalkAppear");
        setNerve(GET_NERVE_ANON(DodoryuDemoOpeningNrvOpeningHide));
    }
}

void DodoryuDemoOpening::exeHide() {
    if (MR::tryEndDemo(mHost, sOpeningDemoName, false)) {
        mHost->nextState();
        _9C = true;
        return;
    }

    if (MR::isBckOneTimeAndStopped(mHost)) {
        MR::startAction(mHost, "EffectWalk");

        Dodoryu* pHost = mHost;
        pHost->mRumbleEnabled = true;
        pHost->mRumbleStep = 0;
        mHost->startLeadHillBck("EffectWalk");
    }
}

bool DodoryuDemoOpening::tryFinish() {
    if (_9C) {
        return true;
    }
    if (MR::isDemoEndWaitCameraInterpolating() && !MR::isDemoEndWaitCinemaFrameStopped()) {
        return false;
    }
    if (!isNerve(GET_NERVE_ANON(DodoryuDemoOpeningNrvOpeningDemoWait)) && MR::isAnimCameraEnd(mHost, &mActorCameraInfo, "OpeningDemo")) {
        mHost->nextState();
        return true;
    }
    return false;
}

DodoryuDemoAppear::DodoryuDemoAppear(Dodoryu* pHost, const char* pName) : DodoryuStateBase(pHost, pName), mActorCameraInfo() {
    initWithoutIter();
}

void DodoryuDemoAppear::init(const JMapInfoIter& rIter) {
    MR::invalidateClipping(this);
    initNerve(GET_NERVE_ANON(DodoryuDemoAppearNrvAppearPlay));
    MR::initAnimCamera(mHost, &mActorCameraInfo, sAppearDemoName);
    makeActorAppeared();
}

void DodoryuDemoAppear::start() {
    setNerve(GET_NERVE_ANON(DodoryuDemoAppearNrvAppearDemoWait));
}

void DodoryuDemoAppear::end() {
    MR::showPlayer();
}

void DodoryuDemoAppear::exeDemoWait() {
    if (!MR::tryStartDemo(mHost, sAppearDemoName)) {
        return;
    }

    MR::startAnimCameraTargetSelf(mHost, &mActorCameraInfo, sAppearDemoName, 0, false, 1.0f);
    MR::stopStageBGMImmediately();
    MR::stopStageBGM(60);
    MR::invalidateShadowAll(mHost);
    MR::requestMovementOnImageEffect();
    ::resetCastDisposition(mHost);
    setNerve(GET_NERVE_ANON(DodoryuDemoAppearNrvAppearPlay));
}

void DodoryuDemoAppear::exePlay() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "AppearDemo");
        MR::startBtp(mHost, "AppearDemo");
        MR::startBtk(mHost, "AppearDemo");
        MR::startBrk(mHost, "Normal");
        MR::startBva(mHost, "Normal");
        MR::hidePlayer();
    }

    if (MR::tryEndDemoAndAnimCamera(mHost, &mActorCameraInfo, sAppearDemoName, 0)) {
        MR::startBtp(mHost, "Normal");
        mHost->nextState();
        MR::startBossBGM(5);

        Dodoryu* pHost = mHost;
        TVec3f jointPos;
        MR::copyJointPos(mHost, "Nose", &jointPos);
        MR::appearStarPiece(pHost, jointPos, 8, 20.0f, 40.0f, false);

        MR::startActionSound(mHost, "OjStarPieceBurst");
    }
}

DodoryuDemoAngry::DodoryuDemoAngry(Dodoryu* pHost, const char* pName) : DodoryuStateBase(pHost, pName), mActorCameraInfo() {
    initWithoutIter();
}

void DodoryuDemoAngry::init(const JMapInfoIter& rIter) {
    MR::invalidateClipping(this);
    initNerve(GET_NERVE_ANON(DodoryuDemoAngryNrvAngryPlay));
    MR::initAnimCamera(mHost, &mActorCameraInfo, sAngryDemoName);
    makeActorAppeared();
}

void DodoryuDemoAngry::start() {
    setNerve(GET_NERVE_ANON(DodoryuDemoAngryNrvAngryDemoWait));
}

void DodoryuDemoAngry::end() {
    MR::showPlayer();
}

void DodoryuDemoAngry::exeDemoWait() {
    if (!MR::tryStartDemo(mHost, sAngryDemoName)) {
        return;
    }

    MR::invalidateShadowAll(mHost);
    ::resetCastDisposition(mHost);
    MR::requestMovementOnImageEffect();
    MR::startAnimCameraTargetSelf(mHost, &mActorCameraInfo, sAngryDemoName, 0, false, 1.0f);
    setNerve(GET_NERVE_ANON(DodoryuDemoAngryNrvAngryPlay));
}

void DodoryuDemoAngry::exePlay() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "AngryDemo");
        MR::startBrk(mHost, "AngryStart");
        MR::startBva(mHost, "Angry");
        MR::stopStageBGM(60);
        MR::hidePlayer();
    }

    if (MR::isStep(this, 120)) {
        MR::startCenterScreenBlur(60, 15.0f, 80, 5, 30);
    }

    if (MR::tryEndDemoAndAnimCamera(mHost, &mActorCameraInfo, sAngryDemoName, 0)) {
        mHost->nextState();
        MR::startBossBGM(6);

        Dodoryu* pHost = mHost;
        TVec3f jointPos;
        MR::copyJointPos(mHost, "Nose", &jointPos);
        MR::appearStarPiece(pHost, jointPos, 16, 20.0f, 40.0f, false);

        MR::startActionSound(mHost, "OjStarPieceBurst");
    }
}

DodoryuDemoDown::DodoryuDemoDown(Dodoryu* pHost, const char* pName) : DodoryuStateBase(pHost, pName), mActorCameraInfo() {
    initWithoutIter();
}

void DodoryuDemoDown::init(const JMapInfoIter& rIter) {
    MR::invalidateClipping(this);
    initNerve(GET_NERVE_ANON(DodoryuDemoDownNrvDownPlay));
    MR::initAnimCamera(mHost, &mActorCameraInfo, "DownDemo");
    makeActorAppeared();
}

void DodoryuDemoDown::start() {
    setNerve(GET_NERVE_ANON(DodoryuDemoDownNrvDownDemoWait));
}

void DodoryuDemoDown::end() {
    MR::showPlayer();
}

void DodoryuDemoDown::exeDemoWait() {
    if (!MR::tryStartDemo(mHost, sDownDemoName)) {
        return;
    }

    MR::startAnimCameraTargetSelf(mHost, &mActorCameraInfo, "DownDemo", 0, false, 1.0f);
    MR::invalidateShadowAll(mHost);
    MR::requestMovementOnImageEffect();
    ::resetCastDisposition(mHost);
    setNerve(GET_NERVE_ANON(DodoryuDemoDownNrvDownPlay));
}

void DodoryuDemoDown::exePlay() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, sDownDemoName);
        MR::startBrk(mHost, "Normal");
        MR::startBva(mHost, "Cry");
        MR::stopStageBGM(30);
        MR::hidePlayer();
    }

    if (MR::isStep(this, ::sDieSoundStep)) {
        MR::startActionSound(mHost, "BmDodoryuDie");
    }

    if (MR::isActionEnd(mHost)) {
        MR::hideModel(mHost);
    }

    Dodoryu* host = mHost;
    if (MR::tryEndDemo(host, sDownDemoName, MR::isAnimCameraEnd(host, &mActorCameraInfo, sDownDemoName))) {
        mHost->nextState();
    }
}

#ifndef GALAXY_NATIVE  // native: DodoryuStateBase2.cpp (DOL split owner) defines these; misplaced copies
void DodoryuStateBase::calcAttackDir(TVec3f* pVec, const TVec3f& rVec1, const TVec3f& rVec2) {
    pVec->sub(rVec2, rVec1);

    pVec->orthogonalize(mHost->mGravity);
    MR::normalizeOrZero(pVec);

    if (MR::isNearZero(*pVec)) {
        mHost->mBaseMtx.getXDir(*pVec);
    }

    TVec3f yDir;
    mHost->mBaseMtx.getYDir(yDir);
    pVec->add(yDir);
    MR::normalize(pVec);
}

bool DodoryuStateBase::calcVerticalizedDir(TVec3f* pVec, const TVec3f& rVec) {
    // TODO: Possibly an inline of `DodoryuUtil::calcVerticalizedDir`.
    TVec3f yDir;
    mHost->mBaseMtx.getYDir(yDir);

    pVec->killElement(rVec, yDir);

    if (MR::isNearZero(*pVec)) {
        return false;
    }

    MR::normalize(pVec);

    return true;
}
#endif
