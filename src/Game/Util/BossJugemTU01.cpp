#pragma auto_inline off
#include "Game/Boss/BossJugem.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Util.hpp"

// Dash points the BossJugem rushes between ("ジュゲム突進点１/２").
static const char* sRushPointNames[2] = {"\203\127\203\205\203\121\203\200\223\313\220\151\223\137\202\120", "\203\127\203\205\203\121\203\200\223\313\220\151\223\137\202\121"};

// Constants of the two volume drawers of the thunder.
static TVec3f sBoltScale(0.65f, 0.65f, 0.65f);
static TVec3f sBoltOffset(0.0f, -700.0f, 0.0f);
static TVec3f sBoltDrawScale(0.09f, 0.09f, 0.09f);
static TVec3f sRangeScale(0.0f, 0.0f, 0.0f);
static TVec3f sRangeOffset(0.0f, -1000.0f, 400.0f);
static TVec3f sRangeDrawScale(0.09f, 0.09f, 0.09f);

namespace NrvBossJugemStateDash {
    NEW_NERVE(BossJugemStateDashNrvDashSign, BossJugemStateDash, DashSign);
    NEW_NERVE(BossJugemStateDashNrvDashCharge, BossJugemStateDash, DashCharge);
    NEW_NERVE(BossJugemStateDashNrvDash, BossJugemStateDash, Dash);
    NEW_NERVE(BossJugemStateDashNrvDashEnd, BossJugemStateDash, DashEnd);
};  // namespace NrvBossJugemStateDash

namespace NrvBossJugemThunder {
    class BossJugemThunderNrvCharge : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            BossJugemThunder* pActor = reinterpret_cast< BossJugemThunder* >(pSpine->mExecutor);
            if (MR::isFirstStep(pActor)) {
                pActor->mRangeDrawer->setScaleUpRate(40);
            }
        }
        static BossJugemThunderNrvCharge sInstance;
    };
    BossJugemThunderNrvCharge BossJugemThunderNrvCharge::sInstance ATTRIBUTE_WEAK;
    class BossJugemThunderNrvThunder : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            BossJugemThunder* pActor = reinterpret_cast< BossJugemThunder* >(pSpine->mExecutor);
            if (MR::isFirstStep(pActor)) {
                pActor->mBoltDrawer->setScaleUpRate(40);
            }
        }
        static BossJugemThunderNrvThunder sInstance;
    };
    BossJugemThunderNrvThunder BossJugemThunderNrvThunder::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(BossJugemThunderNrvDisappear, BossJugemThunder, Disappear);
};  // namespace NrvBossJugemThunder

BossJugemStateDash::BossJugemStateDash(BossJugem* pHost)
    : ActorStateBase< BossJugem >("\203\173\203\130\203\127\203\205\203\121\203\200\223\144\214\202\203\137\203\142\203\126\203\205", pHost), mThunder(nullptr), mDashDir(0.0f, 0.0f, 0.0f),
      mStartPos(0.0f, 0.0f, 0.0f) {
}

void BossJugemStateDash::init() {
    mThunder = new BossJugemThunder(mHost, "\227\213\203\202\203\146\203\213");
    mThunder->initWithoutIter();
    mThunder->makeActorDead();
    initNerve(&NrvBossJugemStateDash::BossJugemStateDashNrvDashSign::sInstance, 0);
}

void BossJugemStateDash::appear() {
    mIsDead = false;
    setNerve(&NrvBossJugemStateDash::BossJugemStateDashNrvDashSign::sInstance);
}

void BossJugemStateDash::kill() {
    mIsDead = true;
    mThunder->makeActorDead();
}

// NONMATCHING: target loads the string address before mHost (argument load scheduling only) (FP order/grouping checked: no FP instructions).
bool BossJugemStateDash::isInvincible() const {
    return !MR::isAnyAnimPlaying(mHost, "DashEnd");
}

void BossJugemStateDash::exeDashSign() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(mHost, "ThunderCharge", -1, -1, -1);
        mHost->startActionAll("DashSignMove");
        BossJugem* pHost = mHost;
        f32 coord = MR::getRailCoord(pHost);
        if (0.5f * MR::getRailTotalLength(pHost) > coord) {
            MR::setRailDirectionToStart(mHost);
        } else {
            MR::setRailDirectionToEnd(mHost);
        }
        mStartPos = mHost->mPosition;
    }
    mHost->turnToPlayer(0.06f);
    BossJugem* pHost = mHost;
    MR::moveCoordAndFollowTrans(pHost, MR::calcRailSpeedEaseAtEnds(pHost, 30.0f, 0.1f, 300.0f));
    if (MR::isRailReachedGoal(mHost)) {
        setNerve(&NrvBossJugemStateDash::BossJugemStateDashNrvDashCharge::sInstance);
    }
}

void BossJugemStateDash::exeDashCharge() {
    if (MR::isFirstStep(this)) {
        mStartPos = mHost->mPosition;
        mHost->startActionAll("DashSignMove");
    }
    s32 endStep = 120 - MR::getBckFrameMax(mHost, "DashSign");
    if (MR::isStep(this, endStep)) {
        mHost->startActionAll("DashSign");
    }
    if (MR::isStep(this, endStep + 20)) {
        mThunder->appear();
        MR::invalidateShadow(mHost, nullptr);
        MR::invalidateShadow(mHost->mCloud, nullptr);
    }
    TVec3f rushPos(0.0f);
    calcRushTarget(&rushPos, &mDashDir, mStartPos);
    f32 rate = MR::calcNerveEaseInOutRate(this, 120);
    TVec3f pos = mStartPos + (rushPos - mStartPos) * rate;
    f32 lift = MR::calcNerveEaseOutValue(this, 30, 0.0f, 800.0f);
    BossJugem* pHost = mHost;
    MR::setPosition(pHost, pos - pHost->mGravity * lift);
    TVec3f dir = rushPos - mStartPos;
    MR::vecKillElement(dir, mHost->mGravity, &dir);
    MR::normalize(&dir);
    TVec3f blend(dir);
    MR::vecBlendSphere(&blend, dir, mDashDir, MR::calcNerveRate(this, endStep));
    MR::vecBlendSphere(&mHost->mFrontVec, mHost->mFrontVec, blend, 0.06f);
    MR::normalize(&mHost->mFrontVec);
    if (MR::isGreaterEqualStep(this, 120)) {
        setNerve(&NrvBossJugemStateDash::BossJugemStateDashNrvDash::sInstance);
    }
}

void BossJugemStateDash::exeDash() {
    if (MR::isFirstStep(this)) {
        mHost->startActionAll("Dash");
        mStartPos = mHost->mPosition;
        mThunder->setNerve(&NrvBossJugemThunder::BossJugemThunderNrvThunder::sInstance);
        MR::validateHitSensor(mHost, "ThunderTop");
        MR::validateHitSensor(mHost, "ThunderMiddle");
        MR::validateHitSensor(mHost, "ThunderBottom");
    }
    f32 rate = MR::calcNerveEaseInOutRate(this, 210);
    TVec3f* pPos = &mHost->mPosition;
    MR::vecBlend(pPos, mStartPos, mStartPos + mDashDir * 3000.0f, rate);
    TVec3f hitPos(0.0f);
    BossJugem* pHost = mHost;
    TVec3f probeA = pHost->mPosition + pHost->mFrontVec * 200.0f;
    pHost = mHost;
    TVec3f probeB = pHost->mPosition + pHost->mFrontVec * 0.0f;
    if (MR::getFirstPolyOnLineToMap(&hitPos, nullptr, probeA, mHost->mGravity * 800.0f * 1.5f) ||
        MR::getFirstPolyOnLineToMap(&hitPos, nullptr, probeB, mHost->mGravity * 800.0f * 1.5f)) {
        MR::tryEmitEffect(mHost->mCloud, "DashGroundSpark");
    } else {
        MR::tryDeleteEffect(mHost->mCloud, "DashGroundSpark");
    }
    if (MR::isGreaterEqualStep(this, 195)) {
        mThunder->setNerve(&NrvBossJugemThunder::BossJugemThunderNrvDisappear::sInstance);
    }
    if (MR::isGreaterEqualStep(this, 210)) {
        setNerve(&NrvBossJugemStateDash::BossJugemStateDashNrvDashEnd::sInstance);
        MR::invalidateHitSensor(mHost, "ThunderTop");
        MR::invalidateHitSensor(mHost, "ThunderMiddle");
        MR::invalidateHitSensor(mHost, "ThunderBottom");
    }
}

void BossJugemStateDash::exeDashEnd() {
    if (MR::isFirstStep(this)) {
        mStartPos = mHost->mPosition;
        MR::validateShadow(mHost, nullptr);
        MR::validateShadow(mHost->mCloud, nullptr);
    }
    if (MR::isStep(this, 90 - MR::getBckFrameMax(mHost, "DashEnd"))) {
        mHost->startActionAll("DashEnd");
    }
    mHost->turnToPlayer(0.06f);
    TVec3f nearest;
    f32 coord = MR::calcNearestRailPos(&nearest, mHost, mStartPos);
    TVec3f* pPos = &mHost->mPosition;
    MR::vecBlendSphere(pPos, mStartPos, nearest, MR::calcNerveEaseInOutRate(this, 90));
    if (MR::isBckOneTimeAndStopped(mHost)) {
        mHost->startActionAll("WaitNormal");
    }
    if (MR::isGreaterStep(this, 90)) {
        MR::setRailCoord(mHost, coord);
        kill();
    }
}

// NONMATCHING: target saves f30/f31 with stfd+psq_st (frame 0x90) where ours uses plain stfd (frame 0x80); body is otherwise identical
f32 BossJugemStateDash::calcRushTarget(TVec3f* pTarget, TVec3f* pDir, const TVec3f& rFrom) {
    TVec3f points[2];
    for (s32 i = 0; i < 2; i++) {
        MR::findNamePos(sRushPointNames[i], &points[i], nullptr);
    }
    TVec3f diffA = points[0] - rFrom;
    f32 distA = PSVECMag(&diffA);
    TVec3f diffB = points[1] - rFrom;
    f32 distB = PSVECMag(&diffB);
    f32 dist = distB;
    if (distA > distB) {
        *pTarget = points[1];
        *pDir = points[0] - points[1];
    } else {
        *pTarget = points[0];
        *pDir = points[1] - points[0];
        dist = distA;
    }
    MR::normalizeOrZero(pDir);
    return dist;
}

BossJugemThunder::BossJugemThunder(BossJugem* pHost, const char* pName)
    : LiveActor(pName), mHost(pHost), mRangeDrawer(nullptr), mBoltDrawer(nullptr), _FC(0.0f, 0.0f, 0.0f), _108(false) {
    mBoltMtx.identity();
    mRangeMtx.identity();
}

void BossJugemThunder::init(const JMapInfoIter& rIter) {
    MR::connectToSceneEnemyMovement(this);
    initNerve(&NrvBossJugemThunder::BossJugemThunderNrvCharge::sInstance, 0);
    mBoltDrawer = MR::createVolumeDrawer("\223\144\214\202\225\140\211\346", "VolumeCylinder", mBoltMtx.toMtxPtr());
    mRangeDrawer = MR::createVolumeDrawer("\223\313\220\151\224\315\210\315\225\140\211\346", "VolumeCylinder", mRangeMtx.toMtxPtr());
    MR::invalidateClipping(this);
    makeActorDead();
}

void BossJugemThunder::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    mBoltDrawer->makeActorAppeared();
    mBoltDrawer->setScaleDownRate(0);
    mRangeDrawer->makeActorAppeared();
    mRangeDrawer->setScaleDownRate(0);
    MR::hideModel(this);
    setNerve(&NrvBossJugemThunder::BossJugemThunderNrvCharge::sInstance);
}

void BossJugemThunder::makeActorDead() {
    LiveActor::makeActorDead();
    mBoltDrawer->makeActorDead();
    mRangeDrawer->makeActorDead();
}

void BossJugemThunder::control() {
    mBoltMtx.set(mHost->getBaseMtx());
    MR::addTransMtxLocal(mBoltMtx.toMtxPtr(), sBoltOffset);
    MR::preScaleMtx(mBoltMtx.toMtxPtr(), sBoltScale);
    GXColor boltColor = {0xDF, 0xDF, 0x00, 0xEB};
    mBoltDrawer->setColor(Color8(boltColor));
    mBoltDrawer->setDrawPasses(1);
    mBoltDrawer->setScale(sBoltDrawScale);
    mRangeMtx.set(mHost->getBaseMtx());
    MR::addTransMtxLocal(mRangeMtx.toMtxPtr(), sRangeOffset);
    MR::preScaleMtx(mRangeMtx.toMtxPtr(), sRangeScale);
    GXColor rangeColor = {0x8C, 0x28, 0x00, 0x87};
    mRangeDrawer->setColor(Color8(rangeColor));
    mRangeDrawer->setDrawPasses(4);
    mRangeDrawer->setScale(sRangeDrawScale);
}

void BossJugemThunder::exeDisappear() {
    if (MR::isFirstStep(this)) {
        mBoltDrawer->setScaleDownRate(40);
        mRangeDrawer->setScaleDownRate(40);
    }
    if (MR::isGreaterStep(this, 40)) {
        kill();
    }
}
