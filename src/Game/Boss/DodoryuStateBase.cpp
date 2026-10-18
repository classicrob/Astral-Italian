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

void DodoryuDemo_FORCE_MATCH_SDATA2() {
    (void)10.0f;
}

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

DodoryuDemoOpening::DodoryuDemoOpening(Dodoryu* pHost, const char* pName)
    : DodoryuStateBase(pHost, pName), mActorCameraInfo(), _9C(false) {
    initWithoutIter();
}

void DodoryuDemoOpening::init(const JMapInfoIter& rIter) {
    MR::invalidateClipping(this);
    initNerve(GET_NERVE_ANON(DodoryuDemoOpeningNrvOpeningUnderground));
    MR::initAnimCamera(mHost, &mActorCameraInfo, "OpeningDemo");
    makeActorAppeared();
}

void DodoryuDemoOpening::control() {
    if (tryFinish()) {
        return;
    }
}

void DodoryuDemoOpening::start() {
    setNerve(GET_NERVE_ANON(DodoryuDemoOpeningNrvOpeningDemoWait));
}

void DodoryuDemoOpening::end() {
    MR::endAnimCamera(mHost, &mActorCameraInfo, "OpeningDemo", -1, true);
    MR::endDemo(mHost, "ドドリュウ開始デモ");

    TPos3f playerResetMtx;
    mHost->getPlayerResetMtx(&playerResetMtx, 0);
    mHost->_150 = false;

    MR::setPlayerBaseMtx(playerResetMtx.toMtxPtr());
    MR::setPlayerStateWait();
    MR::showPlayer();
}

void DodoryuDemoOpening::exeDemoWait() {
    if (!MR::tryStartDemo(mHost, "ドドリュウ開始デモ")) {
        return;
    }

    MR::invalidateShadowAll(mHost);
    MR::startSystemSE("SE_DM_MEET_BOSS");
    MR::startStageBGM("BGM_PINCH_3", false);
    MR::startAnimCameraTargetOther(mHost, &mActorCameraInfo, "OpeningDemo", CameraTargetArg(mHost->_148), 0, false, 1.0f);
    mHost->pauseOff();
    MR::hidePlayer();
    MR::startAction(mHost, "EffectWalk");
    mHost->startLeadHillBck("EffectWalk");
    MR::startBva(mHost, "Normal");
    MR::requestMovementOnImageEffect();
    MR::setImageEffectControlAuto();
    mHost->shiftMoveStateRail(10.0f);
    mHost->killAllHillPieces();
    mHost->stopHillForce();
    MR::moveCoordToStartPos(mHost);
    mHost->resetRabbit();
    setNerve(GET_NERVE_ANON(DodoryuDemoOpeningNrvOpeningUnderground));
}

void DodoryuDemoOpening::exeUnderground() {
    if (MR::isFirstStep(this)) {
        mHost->startHill();

        Dodoryu* pHost = mHost;
        pHost->_150 = true;
        pHost->_154 = 0;
    }

    MR::startActionSound(mHost, "BmLvDodoryuMoveUShallow");
    turnToRabbit();

    if (MR::isGreaterEqualStep(this, ::sOpeningHideFrame)) {
        setNerve(GET_NERVE_ANON(DodoryuDemoOpeningNrvOpeningUndergroundHide));
    }
}

void DodoryuDemoOpening::exeUndergroundHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "EffectWalkHide");
        mHost->startLeadHillBck("EffectWalkHide");
    }

    turnToRabbit();

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

    MR::startActionSound(mHost, "BmLvDodoryuMoveGround");

    if (MR::isStep(this, ::sOpeningAppearSmileSoundStep)) {
        MR::startActionSound(mHost, "BvDodoryuFaintSmile");
    }

    turnToRabbit();

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

    MR::startActionSound(mHost, "BmLvDodoryuMoveGround");
    turnToRabbit();

    if (MR::isGreaterEqualStep(this, 60)) {
        mHost->_150 = false;

        setNerve(GET_NERVE_ANON(DodoryuDemoOpeningNrvOpeningHideReady));
    }
}

void DodoryuDemoOpening::exeHideReady() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Hide");
        mHost->startLeadHillBck("Hide");
    }

    MR::startActionSound(mHost, "BmLvDodoryuMoveGround");
    turnToRabbit();

    if (MR::isActionEnd(mHost)) {
        MR::startAction(mHost, "EffectWalkAppear");
        setNerve(GET_NERVE_ANON(DodoryuDemoOpeningNrvOpeningHide));
    }
}

void DodoryuDemoOpening::exeHide() {
    turnToRabbit();

    if (MR::isBckOneTimeAndStopped(mHost)) {
        MR::startAction(mHost, "EffectWalk");

        Dodoryu* pHost = mHost;
        pHost->_150 = true;
        pHost->_154 = 0;
        mHost->startLeadHillBck("EffectWalk");
    }
}

bool DodoryuDemoOpening::tryFinish() {
    if (!isNerve(GET_NERVE_ANON(DodoryuDemoOpeningNrvOpeningDemoWait)) && MR::isAnimCameraEnd(mHost, &mActorCameraInfo, "OpeningDemo")) {
        mHost->displayRabbitMessage();
        mHost->nextState();

        return true;
    }

    return false;
}

void DodoryuDemoOpening::turnToRabbit() {
    TVec3f rabbitDir;
    mHost->calcRabbitDir(&rabbitDir);
    TPos3f mtx(mHost->mBaseMtx);
    TVec3f up;
    mtx.getZDir(up);
    TVec3f front;
    mtx.getTrans(front);
    TPos3f rotation;
    rotation.setRotate(up, rabbitDir);
    mtx.concat(rotation, mtx);
    mtx.setTrans(front);
    mHost->setMtx(mtx);
}

DodoryuDemoAppear::DodoryuDemoAppear(Dodoryu* pHost, const char* pName) : DodoryuStateBase(pHost, pName), mActorCameraInfo() {
    initWithoutIter();
}

void DodoryuDemoAppear::init(const JMapInfoIter& rIter) {
    MR::invalidateClipping(this);
    initNerve(GET_NERVE_ANON(DodoryuDemoAppearNrvAppearPlay));
    MR::initAnimCamera(mHost, &mActorCameraInfo, "AppearDemo");
    makeActorAppeared();
}

void DodoryuDemoAppear::start() {
    setNerve(GET_NERVE_ANON(DodoryuDemoAppearNrvAppearDemoWait));
}

void DodoryuDemoAppear::end() {
    MR::endAnimCamera(mHost, &mActorCameraInfo, "AppearDemo", -1, true);
    MR::endDemo(mHost, "ドドリュウ出現デモ");
    MR::showPlayer();
}

void DodoryuDemoAppear::exeDemoWait() {
    if (!MR::tryStartDemo(mHost, "ドドリュウ出現デモ")) {
        return;
    }

    MR::startAnimCameraTargetSelf(mHost, &mActorCameraInfo, "AppearDemo", 0, false, 1.0f);
    MR::stopStageBGM(60);
    MR::invalidateShadowAll(mHost);
    mHost->leaveRabbit();
    MR::requestMovementOnImageEffect();
    MR::setImageEffectControlAuto();
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

    if (MR::isActionEnd(mHost)) {
        MR::startBtp(mHost, "Normal");
        mHost->nextState();
        MR::startBossBGM(MR::BossBgmID_DodoryuA);

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
    MR::initAnimCamera(mHost, &mActorCameraInfo, "AngryDemo");
    makeActorAppeared();
}

void DodoryuDemoAngry::start() {
    setNerve(GET_NERVE_ANON(DodoryuDemoAngryNrvAngryDemoWait));
}

void DodoryuDemoAngry::end() {
    MR::endAnimCamera(mHost, &mActorCameraInfo, "AngryDemo", -1, true);
    MR::endDemo(mHost, "ドドリュウ怒りデモ");
    MR::showPlayer();
}

void DodoryuDemoAngry::exeDemoWait() {
    if (!MR::tryStartDemo(mHost, "ドドリュウ怒りデモ")) {
        return;
    }

    MR::invalidateShadowAll(mHost);
    ::resetCastDisposition(mHost);
    MR::requestMovementOnImageEffect();
    MR::setImageEffectControlAuto();
    MR::startAnimCameraTargetSelf(mHost, &mActorCameraInfo, "AngryDemo", 0, false, 1.0f);
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

    if (MR::isActionEnd(mHost)) {
        mHost->nextState();
        MR::startBossBGM(MR::BossBgmID_DodoryuB);

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
    MR::endAnimCamera(mHost, &mActorCameraInfo, "DownDemo", -1, true);
    MR::endDemo(mHost, "ドドリュウ死亡デモ");
    MR::showPlayer();
}

void DodoryuDemoDown::exeDemoWait() {
    if (!MR::tryStartDemo(mHost, "ドドリュウ死亡デモ")) {
        return;
    }

    MR::startAnimCameraTargetSelf(mHost, &mActorCameraInfo, "DownDemo", 0, false, 1.0f);
    MR::invalidateShadowAll(mHost);
    MR::requestMovementOnImageEffect();
    MR::setImageEffectControlAuto();
    ::resetCastDisposition(mHost);
    setNerve(GET_NERVE_ANON(DodoryuDemoDownNrvDownPlay));
}

void DodoryuDemoDown::exePlay() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "DownDemo");
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

    if (MR::isAnimCameraEnd(mHost, &mActorCameraInfo, "DownDemo")) {
        mHost->nextState();
    }
}

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
