#include "Game/Util/DemoUtil.hpp"
#include "Game/Demo/DemoFunction.hpp"
#pragma auto_inline off
#include "Game/Boss/KoopaJrCastleNerves.hpp"
#include <cmath>
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/PartsModelLodCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Ride/FluffWind.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"


namespace NrvKoopaJrCastle { KoopaJrCastleNrvOpening KoopaJrCastleNrvOpening::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastle { KoopaJrCastleNrvBattle KoopaJrCastleNrvBattle::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastle::KoopaJrCastle(const char* pName)
    : LiveActor(pName), mWind(nullptr), mCannon(nullptr), mFireBar(nullptr), mRingBeam(nullptr), mElectric(nullptr), mDominate(nullptr),
      mBody(nullptr), mHead(nullptr), mCapsule(nullptr), mKoopaJr(nullptr), mBreakModel(nullptr), mStateOpening(nullptr),
      mStateBattle(nullptr), mCameraInfo(nullptr), mFrontVec(0.0f, 0.0f, 0.0f), mStartPos(0.0f, 0.0f, 0.0f), mCameraOffset(0.0f, 0.0f, 0.0f) {
    PSMTXIdentity(mBaseMtx.toMtxPtr());
}

KoopaJrCastle::~KoopaJrCastle() {}

void KoopaJrCastle::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, 0x29, -1, -1, -1);
    MR::initDefaultPos(this, rIter);
    MR::invalidateClipping(this);
    mStartPos = mPosition;
    MR::makeMtxTR(mBaseMtx.toMtxPtr(), this);
    MR::calcFrontVec(&mFrontVec, this);
    MR::initUseStageSwitchReadA(this, rIter);
    MR::initUseStageSwitchReadB(this, rIter);
    MR::useStageSwitchWriteAutoDead(this, rIter);
    MR::setClippingFarMax(this);
    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, nullptr);
    MR::getJMapInfoArg7WithInit(rIter, &mCameraRegId);
    if (mCameraRegId != -1) {
        MR::declareCameraRegisterVec(this, mCameraRegId, &mCameraPos);
    }
    MR::createSceneObj(0x8B);
    reinterpret_cast< KoopaJrCastleAccessor* >(MR::getSceneObjHolder()->getObj(SceneObj_KoopaJrCastleAccessor))->mCastle = this;

    mBody = new KoopaJrCastleBody(this, "\x93y\x91\xe4(Body)");
    mBody->initWithoutIter();
    mHead = new KoopaJrCastleHead(mBody, "\x96""C\x91\xe4(Head)");
    MR::invalidateClipping(mHead);
    mHead->initFixedPosition(MR::getJointMtx(mBody, "HeadPos"), TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    mHead->initWithoutIter();
    mCapsule = new KoopaJrCastleCapsule(mHead, "\x83J\x83v\x83Z\x83\x8b");
    mCapsule->initFixedPosition(MR::getJointMtx(mHead, "Top"), TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    mCapsule->initWithoutIter();
    MR::invalidateClipping(mCapsule);
    mKoopaJr = new PartsModelLodCtrl(mHead, "\x83N\x83""b\x83pJr", "KoopaJr", 0, 0x13, false);
    MR::initShadowFromCSVWithInitShadowVolumeSphere(mKoopaJr, "ShadowKoopaJrCastle");
    MR::validateClipping(mKoopaJr);
    mKoopaJr->initFixedPosition(MR::getJointMtx(mHead, "Apron"), TVec3f(0.0f, 130.0f, 100.0f), TVec3f(0.0f, 0.0f, 0.0f));
    mKoopaJr->initWithoutIter();
    mKoopaJr->initLightCtrl();
    mKoopaJr->mLinkInfo.mZoneID = mLinkInfo.mZoneID;
    mKoopaJr->getLodCtrl()->setDistanceToMiddle(0.0f);
    MR::startBva(mKoopaJr->getLodCtrl()->mMiddleModel, "Cockpit");
    MR::startBva(mKoopaJr->getLodCtrl()->mLowModel, "Cockpit");
    MR::startAction(mKoopaJr, "Wait");
    mBreakModel = new BreakModel(this, "\x83L\x83\x83\x83""b\x83X\x83\x8b\x89\xf3\x82\xea\x83\x82\x83""f\x83\x8b", "KoopaJrCastleBreak", 0, "Break");
    mBreakModel->initWithoutIter();
    mCameraInfo = new ActorCameraInfo(rIter);
    mWind = new KoopaJrCastleWind(this, "\x95\x97\x81i\x89~\x81j");
    MR::setPosition(mWind, mPosition);
    mWind->initWithoutIter();
    mCannon = new KoopaJrCastleCannon(this, "\x96""C\x8c\x82");
    mCannon->initWithoutIter();
    mFireBar = new KoopaJrCastleFireBar("\x83t\x83@\x83""C\x83""A\x83o\x81[");
    mFireBar->setup(mBody, TVec3f(0.0f, 1635.0f, 0.0f), TVec3f(0.0f), "KoopaJrCastleBody");
    mFireBar->initWithoutIter();
    mRingBeam = new KoopaJrCastleRingBeam("\x83\x8a\x83\x93\x83O\x83r\x81[\x83\x80\x8dU\x8c\x82");
    mRingBeam->setup(mBody, TVec3f(0.0f, 1400.0f, 0.0f), TVec3f(0.0f), "KoopaJrCastleBody");
    mRingBeam->initWithoutIter();
    mElectric = new KoopaJrCastleElectric("\x93""d\x8b""C");
    mElectric->initWithoutIter();
    mDominate = new KoopaJrCastleDominate("\x83v\x83\x8c\x83""C\x83\x84\x81[\x8ex\x94z");
    mDominate->initWithoutIter();
    initNerve(&NrvKoopaJrCastle::KoopaJrCastleNrvOpening::sInstance, 2);
    mStateOpening = new KoopaJrCastleStateOpening(this);
    MR::initActorState(this, mStateOpening, &NrvKoopaJrCastle::KoopaJrCastleNrvOpening::sInstance, "PhaseOpening");
    mStateBattle = new KoopaJrCastleStateBattle(this);
    MR::initActorState(this, mStateBattle, &NrvKoopaJrCastle::KoopaJrCastleNrvBattle::sInstance, "PhaseBattle");
    initBinder(375.0f, 375.0f, 0);
    MR::offBind(this);
    MR::declarePowerStar(this);
    MR::declareStarPiece(this, 30);
    MR::tryInitDemoSheetAnim(mBody, rIter, "\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x93o\x8f\xea", "DemoOpeningKoopaJrCastle");
    MtxPtr rootMtx = MR::getJointMtx(mKoopaJr, "Root");
    DemoFunction::registerDemoTalkMessageCtrlDirect(mKoopaJr, MR::createTalkCtrlDirect(mKoopaJr, rIter, "DemoOpeningKoopaJrCastle000", TVec3f(0.0f), rootMtx), "\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x93o\x8f\xea");
    MR::initTalkAnimDemoCast(mKoopaJr, rIter, "\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x93o\x8f\xea", "DemoOpeningKoopaJrCastle");
    MR::initTalkAnimDemoCast(mHead, rIter, "\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x93o\x8f\xea", "DemoOpeningKoopaJrCastle");
    makeActorAppeared();
}

void KoopaJrCastle::makeActorAppeared() {
    MR::invalidateClipping(this);
    setNerve(&NrvKoopaJrCastle::KoopaJrCastleNrvOpening::sInstance);
    LiveActor::makeActorAppeared();
}

void KoopaJrCastle::control() {
    mCameraPos = mCapsule->mPosition;
    TVec3f grav(0.0f);
    MR::calcVecToPlayerHorizon(&grav, mCapsule, 0);
    if (!MR::isNearZero(grav, 0.001f)) {
        MR::normalize(&grav);
        TPos3f mtx;
        PSMTXIdentity(mtx.toMtxPtr());
        MR::makeMtxFrontUp(&mtx, mGravity, grav);
        MR::extractMtxXDir(mtx.toMtxPtr(), &mCameraOffset);
        TVec3f toPlayer(0.0f);
        MR::calcVecToPlayer(&toPlayer, mCapsule);
        f32 d = toPlayer.dot(mGravity) / 2000.0f;
        f32 v;
        if (d < 0.0f) {
            v = 0.0f;
        } else if (d > 1.0f) {
            v = 1.0f;
        } else {
            v = d;
        }
        mCameraOffset = (grav * -0.0f) * v;
    }
    mCameraPos += mCameraOffset;
    if (mStateBattle->isDemo()) {
        mCameraPos += mGravity * 1200.0f;
    } else {
        mCameraPos += mGravity * -100.0f;
    }
    MR::calcMtxFromGravityAndZAxis(&mBaseMtx, this, mFrontVec);
}

// NONMATCHING: vector temporary stack slots and saved registers.
void KoopaJrCastle::exeBattle() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::startStageBGM("MBGM_SMG2_BOSS_06A", false);
    }
    if (MR::updateActorState(this)) {
        TVec3f pos(mPosition - mGravity * 1600.0f);
        MR::requestAppearPowerStar(this, pos);
        TVec3f cam(mStartPos + mGravity * -1000.0f);
        mCameraPos = cam;
        mCapsule->kill();
        mHead->kill();
        mBody->kill();
        kill();
    }
}

void KoopaJrCastle::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrCastle::KoopaJrCastleNrvBattle::sInstance)) {
        mStateBattle->attackSensor(pSender, pReceiver);
    }
}

bool KoopaJrCastle::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrCastle::KoopaJrCastleNrvBattle::sInstance)) {
        return mStateBattle->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    return false;
}

bool KoopaJrCastle::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrCastle::KoopaJrCastleNrvBattle::sInstance)) {
        return mStateBattle->receiveOtherMsg(msg, pSender, pReceiver);
    }
    return false;
}

MtxPtr KoopaJrCastle::getBaseMtx() const {
    return const_cast< TPos3f& >(mBaseMtx).toMtxPtr();
}

KoopaJrCastleAccessor::KoopaJrCastleAccessor() : NameObj("クッパJrキャッスルアクセス"), mCastle(nullptr) {}

KoopaJrCastleAccessor::~KoopaJrCastleAccessor() {}
