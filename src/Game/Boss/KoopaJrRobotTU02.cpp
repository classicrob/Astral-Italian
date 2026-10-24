#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/MapObj/StarPieceGroup.hpp"
#include "Game/MapObj/StarPiece.hpp"
#include "Game/Enemy/HomingKiller.hpp"
#include "Game/Util.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/LiveActor/Nerve.hpp"
#ifdef GALAXY_NATIVE
#include <petari/game_compat.hpp>
#else
#include <functional.hpp>
#endif

template < typename It, typename Pmf >
static Pmf forEachCall(It it, It end, Pmf pmf) {
    for (; it != end; ++it) {
        ((*it)->*pmf)();
    }
    return pmf;
}

template < typename It, typename Fn >
static inline void forEachCallInline(It it, It end, Fn fn) {
    It last = end;
    for (It first = it; first != last; ++first) {
        fn(*first);
    }
}

template < typename It, typename Fn >
static Fn forEachFn(It it, It end, Fn fn) {
    for (; it != end; ++it) {
        fn(*it);
    }
    return fn;
}

namespace NrvKoopaJrRobotSeq {
    class KoopaJrRobotSeqNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast< KoopaJrRobotSeq* >(pSpine->mExecutor)->exeA();
        }
        static KoopaJrRobotSeqNrvA sInstance;
    };
    class KoopaJrRobotSeqNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast< KoopaJrRobotSeq* >(pSpine->mExecutor)->exeA();
        }
        static KoopaJrRobotSeqNrvB sInstance;
    };
    class KoopaJrRobotSeqNrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast< KoopaJrRobotSeq* >(pSpine->mExecutor)->exeA();
        }
        static KoopaJrRobotSeqNrvC sInstance;
    };
    class KoopaJrRobotSeqNrvD : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast< KoopaJrRobotSeq* >(pSpine->mExecutor)->exeB();
        }
        static KoopaJrRobotSeqNrvD sInstance;
    };
    class KoopaJrRobotSeqNrvE : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast< KoopaJrRobotSeq* >(pSpine->mExecutor)->exeC();
        }
        static KoopaJrRobotSeqNrvE sInstance;
    };
    class KoopaJrRobotSeqNrvF : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrRobotSeq* pSeq = reinterpret_cast< KoopaJrRobotSeq* >(pSpine->mExecutor);
            if (MR::isFirstStep(pSeq)) {
                pSeq->startRearLauncherShots();
                pSeq->setNerve(&KoopaJrRobotSeqNrvC::sInstance);
            }
        }
        static KoopaJrRobotSeqNrvF sInstance;
    };
    class KoopaJrRobotSeqNrvG : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static KoopaJrRobotSeqNrvG sInstance;
    };
    KoopaJrRobotSeqNrvA KoopaJrRobotSeqNrvA::sInstance ATTRIBUTE_WEAK;
    KoopaJrRobotSeqNrvB KoopaJrRobotSeqNrvB::sInstance ATTRIBUTE_WEAK;
    KoopaJrRobotSeqNrvC KoopaJrRobotSeqNrvC::sInstance ATTRIBUTE_WEAK;
    KoopaJrRobotSeqNrvD KoopaJrRobotSeqNrvD::sInstance ATTRIBUTE_WEAK;
    KoopaJrRobotSeqNrvE KoopaJrRobotSeqNrvE::sInstance ATTRIBUTE_WEAK;
    KoopaJrRobotSeqNrvF KoopaJrRobotSeqNrvF::sInstance ATTRIBUTE_WEAK;
    KoopaJrRobotSeqNrvG KoopaJrRobotSeqNrvG::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvKoopaJrRobotSeq

HomingKiller* KoopaJrRobot::allocKiller(LiveActor* pOwner) {
    return mSeq->mPool->allocate(pOwner);
}

bool KoopaJrRobot::releaseKiller(LiveActor* pOwner, HomingKiller* pKiller) {
    return mSeq->mPool->release(pOwner, pKiller);
}

bool KoopaJrRobot::isKillerShotEnd(HitSensor* pSensor) {
    return mSeq->mPool->isShotEnd(pSensor);
}

void KoopaJrRobot::attackShootableKillers() {
    mSeq->attackShootableKillers();
}

bool KoopaJrRobot::isArmSensor(HitSensor* pSensor) {
    const char* names[6] = {"ElbowL", "UpperArmL", "ForeArmL", "ElbowR", "UpperArmR", "ForeArmR"};
    for (u32 i = 0; i < 6; i++) {
        if (MR::isEqualSensor(pSensor, this, names[i])) {
            return true;
        }
    }
    return false;
}

bool KoopaJrRobot::isWeakPointSensor(HitSensor* pSensor) {
    for (s32 i = 0; i < 4; i++) {
        if (MR::isEqualSensor(pSensor, this, KoopaJrRobotPart::getSensorName(i))) {
            return true;
        }
    }
    return false;
}

bool KoopaJrRobotFunction::isPlayerInRushAir() {
    bool ret = false;
    if (MR::isPlayerInRush() && !MR::isOnGroundPlayer()) {
        ret = true;
    }
    return ret;
}

bool KoopaJrRobot::isPlayerNearBack(f32 angle) {
    bool inAir = false;
    if (MR::isPlayerInRush() && !MR::isOnGroundPlayer()) {
        inAir = true;
    }
    if (inAir) {
        return false;
    }
    TVec3f player(*MR::getPlayerCenterPos());
    TVec3f dir(player);
    dir.sub(mPosition);
    MR::vecKillElement(dir, mGravity, &dir);
    MR::normalizeOrZero(&dir);
    TVec3f back;
    back.negate(mZAxis);
    return MR::diffAngleAbs(back, dir) * 57.29578f < angle;
}

bool KoopaJrRobot::isAllBodyPartsBroken() {
    bool ret = false;
    if (mParts[0]->isBroken() && mParts[1]->isBroken() && mParts[2]->isBroken()) {
        ret = true;
    }
    return ret;
}

LiveActor* KoopaJrRobot::getKillerOwner(HomingKiller* pKiller) {
    return mSeq->mPool->getOwner(pKiller);
}

void KoopaJrRobotFunction::attachBodyToAllRoot(KoopaJrRobot* pHost) {
    KoopaJrRobotPod* pBody = pHost->mBody;
    pBody->setMtx(MR::getJointMtx(pHost, "AllRoot"));
}

void KoopaJrRobotFunction::attachBodyToHead(KoopaJrRobot* pHost) {
    KoopaJrRobotPod* pBody = pHost->mBody;
    pBody->setMtx(MR::getJointMtx(pHost, "Head"));
}

void KoopaJrRobotFunction::showStarPieces(KoopaJrRobot* pHost) {
    StarPieceGroup* pGroup = pHost->mStarPieceGroup;
    for (s32 i = 0; i < pGroup->mNumPieces; i++) {
        LiveActor* pPiece = pGroup->mPieces[i];
        MR::showModelAndEffects(pPiece);
        NameObjFunction::requestMovementOn(pPiece);
    }
}

void KoopaJrRobotFunction::hideStarPieces(KoopaJrRobot* pHost) {
    StarPieceGroup* pGroup = pHost->mStarPieceGroup;
    for (s32 i = 0; i < pGroup->mNumPieces; i++) {
        LiveActor* pPiece = pGroup->mPieces[i];
        MR::hideModelAndEffects(pPiece);
        NameObjFunction::requestMovementOff(pPiece);
    }
}

LiveActor* KoopaJrRobotFunction::createHand(KoopaJrRobot* pHost, const char* pName) {
    CollisionObj* pObj = new CollisionObj(pHost, "クッパJrロボットコリジョンオブジェ", pName,
                                         pHost->getSensor(pName), MR::getJointMtx(pHost, pName), -1);
    pObj->initWithoutIter();
    return pObj;
}

KoopaJrRobotPart* KoopaJrRobotFunction::createPart(KoopaJrRobot* pHost, s32 id) {
    return KoopaJrRobotPart::create(pHost, id);
}

struct KoopaJrRobotBeamInfo {
    const char* mName;
    const char* mJoint;
    Vec mOffset;
    f32 mSpeed;
    s32 mLife;
    f32 mA;
    f32 mB;
};

static const KoopaJrRobotBeamInfo sBeamInfo[3] = {
    {"\x83r\x81[\x83\x80L", "HandL", {750.0f, 0.0f, -700.0f}, 20.0f, 130, 600.0f, 3500.0f},
    {"\x83r\x81[\x83\x80R", "HandR", {750.0f, 0.0f, 700.0f}, 20.0f, 130, 600.0f, 3500.0f},
    {"\x83r\x81[\x83\x80""BODY", "AllRoot", {0.0f, 400.0f, 0.0f}, 20.0f, 180, 2200.0f, 5500.0f},
};

// NONMATCHING: two loads (lwz life / lfs speed) scheduled in swapped order
LiveActor* KoopaJrRobotFunction::createCollision(KoopaJrRobot* pHost, const JMapInfoIter& rIter, s32 index) {
    const KoopaJrRobotBeamInfo* pInfo = &sBeamInfo[index];
    KoopaJrRobotBeam* pBeam = new KoopaJrRobotBeam(pInfo->mName, pHost, MR::getJointMtx(pHost, pInfo->mJoint), TVec3f(pInfo->mOffset), pInfo->mLife, pInfo->mSpeed,
                                                    pInfo->mA, pInfo->mB);
    pBeam->init(rIter);
    return pBeam;
}

KoopaJrRobotBinderActor* KoopaJrRobotFunction::createFollower(LiveActor* pHost, s32 index) {
    const char* names[5] = {"ForeArmL", "ForeArmR", "ElbowL", "ElbowR", "HeadCover"};
    KoopaJrRobotBinderActor* pActor = new KoopaJrRobotBinderActor(pHost->getSensor(names[index]));
    pActor->initWithoutIter();
    return pActor;
}

KoopaJrRobotPod* KoopaJrRobotFunction::createBody(KoopaJrRobot* pHost, MtxPtr pMtx) {
    KoopaJrRobotPod* pPod = new KoopaJrRobotPod(pHost, pMtx);
    pPod->initWithoutIter();
    return pPod;
}

PartsModel* KoopaJrRobotFunction::createHeadModel(KoopaJrRobotPod* pPod) {
    PartsModel* pModel = MR::createPartsModelNpc(pPod, "\x83N\x83""b\x83pJr", "KoopaJr", MR::getJointMtx(pPod, "KoopaJrPos"));
    MR::registerDemoSimpleCastAll(pModel);
    MR::initShadowFromCSVWithInitShadowVolumeSphere(pModel, "Shadow");
    pModel->kill();
    return pModel;
}

void KoopaJrRobotFunction::hideAll(KoopaJrRobot* pHost) {
    MR::hideModelAndOnCalcAnim(pHost);
    MR::hideModelAndOnCalcAnim(pHost->mParts[0]->mModel);
    MR::hideModelAndOnCalcAnim(pHost->mParts[1]->mModel);
    MR::hideModelAndOnCalcAnim(pHost->mParts[2]->mModel);
    LiveActor** pChildren = pHost->mChildren.mArr;
    forEachFn(pChildren, pChildren + pHost->mChildCount, MR::hideModelAndOnCalcAnim);
}

void KoopaJrRobotFunction::showAll(KoopaJrRobot* pHost) {
    MR::showModel(pHost);
    MR::showModel(pHost->mParts[0]->mModel);
    MR::showModel(pHost->mParts[1]->mModel);
    MR::showModel(pHost->mParts[2]->mModel);
    LiveActor** pChildren = pHost->mChildren.mArr;
    forEachFn(pChildren, pChildren + pHost->mChildCount, MR::showModel);
}

void KoopaJrRobotFunction::offDrawEffect(KoopaJrRobot* pHost) {
    MR::offDrawEffect(pHost->mParts[0]->mModel);
    MR::offDrawEffect(pHost->mParts[1]->mModel);
    MR::offDrawEffect(pHost->mParts[2]->mModel);
}

void KoopaJrRobotFunction::onDrawEffect(KoopaJrRobot* pHost) {
    MR::onDrawEffect(pHost->mParts[0]->mModel);
    MR::onDrawEffect(pHost->mParts[1]->mModel);
    MR::onDrawEffect(pHost->mParts[2]->mModel);
}

KoopaJrRobotSeq::KoopaJrRobotSeq(KoopaJrRobot* pHost) : NerveExecutor("\x83L\x83\x89\x81[\x90\xa7\x8c\xe4"), mHost(pHost), mPool(nullptr), mMagnumLauncher(nullptr), mLaunchersDisabled(false), mLauncherSide(false) {}

// NONMATCHING: regalloc only (begin pointer r26 vs p r23 permuted); string-pool offsets differ
void KoopaJrRobotSeq::initLaunchers() {
    mPool = new KoopaJrRobotKillerPool(mHost, 48);
    mMagnumLauncher = new KoopaJrRobotKillerLauncher(mHost, 1, new FixedPosition(mHost, "MagnumKillerPoint", TVec3f(0.0f), TVec3f(0.0f, 90.0f, 0.0f)));
    mMagnumLauncher->initWithoutIter();
    mMagnumLauncher->makeActorDead();
    KoopaJrRobotKillerLauncher** pBegin = mLaunchers;
    for (KoopaJrRobotKillerLauncher** p = pBegin; p != mLaunchers + 8; p++) {
        const char* pName = nullptr;
        switch (p - pBegin) {
        case 0:
            pName = "KillerPointB";
            break;
        case 1:
            pName = "KillerPointA";
            break;
        case 2:
            pName = "KillerPointC";
            break;
        case 3:
            pName = "KillerPointD";
            break;
        case 4:
            pName = "KillerPointE";
            break;
        case 5:
            pName = "KillerPointF";
            break;
        case 6:
            pName = "KillerPointG";
            break;
        case 7:
            pName = "KillerPointH";
            break;
        }
        *p = new KoopaJrRobotKillerLauncher(mHost, 0, new FixedPosition(mHost, pName, TVec3f(0.0f), TVec3f(0.0f, 90.0f, 0.0f)));
        (*p)->initWithoutIter();
        (*p)->makeActorDead();
    }
    initNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvB::sInstance, 0);
}

// NONMATCHING: ptmf by-value arg copy: target keeps a second 12-byte PTMF copy on the stack (frame +16)
void KoopaJrRobotSeq::appearLaunchers() {
    mLaunchersDisabled = false;
    mMagnumLauncher->appear();
    void (KoopaJrRobotKillerLauncher::*pmf)() = &KoopaJrRobotKillerLauncher::appear;
    forEachCall(mLaunchers, mLaunchers + 8, pmf);
}

// NONMATCHING: same ptmf arg-copy stack layout as appearLaunchers
void KoopaJrRobotSeq::killLaunchers() {
    mLaunchersDisabled = true;
    mMagnumLauncher->kill();
    mPool->killUnused();
    void (KoopaJrRobotKillerLauncher::*pmf)() = &KoopaJrRobotKillerLauncher::kill;
    forEachCall(mLaunchers, mLaunchers + 8, pmf);
}

// NONMATCHING: same ptmf arg-copy stack layout as appearLaunchers
void KoopaJrRobotSeq::updateLauncherTransforms() {
    if (!mLaunchersDisabled) {
        mMagnumLauncher->updateTransform();
        void (KoopaJrRobotKillerLauncher::*pmf)() = &KoopaJrRobotKillerLauncher::updateTransform;
        forEachCall(mLaunchers, mLaunchers + 8, pmf);
    }
}

void KoopaJrRobotSeq::movementLaunchers() {
    if (!mLaunchersDisabled) {
        forEachCallInline(mLaunchers, mLaunchers + 8, std::mem_fun(&LiveActor::movement));
    }
}

void KoopaJrRobotSeq::movementKillerPool() {
    if (!mLaunchersDisabled) {
        mPool->movementAll();
    }
}

void KoopaJrRobotSeq::startMagnumShot() {
    mMagnumLauncher->startShot();
}

void KoopaJrRobotSeq::startRearLauncherShots() {
    for (s32 i = 2; i < 8; i++) {
        mLaunchers[i]->startShot();
    }
}

// NONMATCHING: regalloc only (loop vars r28..r31 permuted)
void KoopaJrRobotSeq::attackShootableKillers() {
    HitSensor* pSensor;
    KoopaJrRobotKillerLauncher** p;
    if (!MR::isDead(mMagnumLauncher) && mMagnumLauncher->isShootable()) {
        pSensor = mHost->getSensor("Body");
        MR::sendMsgEnemyAttackExplosion(mMagnumLauncher->getKiller()->getSensor("Body"), pSensor);
    }
    for (p = mLaunchers; p != mLaunchers + 8; p++) {
        if (!MR::isDead(*p) && (*p)->isShootable()) {
            pSensor = mHost->getSensor("Body");
            MR::sendMsgEnemyAttackExplosion((*p)->getKiller()->getSensor("Body"), pSensor);
        }
    }
}

void KoopaJrRobotSeq::startFastFrontLauncherShots() {
    setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvD::sInstance);
}

void KoopaJrRobotSeq::startRearLauncherSequence() {
    setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvF::sInstance);
}

void KoopaJrRobotSeq::startSlowFrontLauncherShots() {
    setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvE::sInstance);
}

void KoopaJrRobotSeq::stopLauncherSequence() {
    setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvG::sInstance);
}

void KoopaJrRobotSeq::exeA() {
    if (isNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvA::sInstance)) {
        if (MR::isGreaterStep(this, 360)) {
            setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvD::sInstance);
        }
    } else if (isNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvB::sInstance)) {
        if (MR::isGreaterStep(this, 360)) {
            setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvE::sInstance);
        }
    } else if (isNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvC::sInstance) && MR::isGreaterStep(this, 300)) {
        setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvF::sInstance);
    }
}

void KoopaJrRobotSeq::exeB() {
    if (MR::isFirstStep(this)) {
        mLaunchers[!mLauncherSide]->startShot();
    }
    if (MR::isStep(this, 70)) {
        mLaunchers[mLauncherSide != 0]->startShot();
        mLauncherSide = !mLauncherSide;
        setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvB::sInstance);
    }
}

void KoopaJrRobotSeq::exeC() {
    if (MR::isFirstStep(this)) {
        mLaunchers[!mLauncherSide]->startShot();
    }
    if (MR::isStep(this, 120)) {
        mLaunchers[mLauncherSide != 0]->startShot();
        mLauncherSide = !mLauncherSide;
        setNerve(&NrvKoopaJrRobotSeq::KoopaJrRobotSeqNrvB::sInstance);
    }
}

KoopaJrRobotSeq::~KoopaJrRobotSeq() {}
