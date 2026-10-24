#define JGEOMETRY_VEC3_SET_NOINLINE
#include "Game/Enemy/BattanKing.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/Pattan.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"
#include "Game/Util/CollisionPartsFilter.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/TalkUtil.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Util/BattanKingCollisionPress.hpp"
#include <JSystem/JMath/JMath.hpp>
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    // (actor name, parts/arc name, joint name) of the five collision parts.
    struct CollisionPartsInfo {
        const char* mActorName;
        const char* mPartsName;
        const char* mJointName;
    };

    CollisionPartsInfo sCollisionPartsInfo[5] = {
        {"コリジョン[体　]", "BattanKing", "BattanKingBody"},
        {"コリジョン[左腕]", "BattanKingArmL", "ArmL"},
        {"コリジョン[右腕]", "BattanKingArmR", "ArmR"},
        {"コリジョン[左足]", "BattanKingLegL", "LegL"},
        {"コリジョン[右足]", "BattanKingLegR", "LegR"},
    };
}

namespace {
    inline bool isInsidePushBox(const TVec3f& pos, const TVec3f& range, const f32& depth) {
        if (!(0.5f * -range.x <= pos.x && pos.x < 0.5f * range.x)) return false;
        if (!(0.0f <= pos.y && pos.y < range.y)) return false;
        if (!(0.5f * -depth <= pos.z && pos.z < 0.5f * depth)) return false;
        return true;
    }
}

namespace NrvBattanKing {
    NEW_NERVE(BattanKingNrvWaitSwitch, BattanKing, WaitSwitch);
    NEW_NERVE(BattanKingNrvOpeningDemo, BattanKing, OpeningDemo);
    NEW_NERVE(BattanKingNrvWaitStand, BattanKing, WaitStand);
    NEW_NERVE(BattanKingNrvTurn, BattanKing, Turn);
    NEW_NERVE(BattanKingNrvFindOrSummon, BattanKing, FindOrSummon);
    NEW_NERVE(BattanKingNrvRun, BattanKing, Run);
    NEW_NERVE(BattanKingNrvWaitSummon, BattanKing, WaitSummon);
    NEW_NERVE(BattanKingNrvQuake, BattanKing, Quake);
    NEW_NERVE(BattanKingNrvQuakeAgain, BattanKing, Quake);
    NEW_NERVE(BattanKingNrvAttackJump, BattanKing, AttackJump);
    NEW_NERVE(BattanKingNrvAttackSign, BattanKing, AttackSign);
    NEW_NERVE(BattanKingNrvAttack, BattanKing, Attack);
    NEW_NERVE(BattanKingNrvWaitFall, BattanKing, WaitFall);
    NEW_NERVE(BattanKingNrvRiseSign, BattanKing, RiseSign);
    NEW_NERVE(BattanKingNrvRise, BattanKing, Rise);
    NEW_NERVE(BattanKingNrvDamage, BattanKing, Damage);
    NEW_NERVE(BattanKingNrvDead, BattanKing, Dead);
    NEW_NERVE(BattanKingNrvDownDemo, BattanKing, DownDemo);
};  // namespace NrvBattanKing

// Statically constructed data of the TU (__sinit_\BattanKing_cpp, after the nerves; 0x807288F0..0x80728A44).
namespace BattanKingVec {
    static TVec3f sPlayerDetectSize(2000, 1200, 10000);
    static TVec3f sPlayerAttackSize(800, 1200, 1300);
    static TVec3f sPowerStarOffset(-610.0f, 50.0f, 143.5f);
    static TVec3f sOffset914(-747.0f, -25.0f, 73.2f);
    static TVec3f sOffset920(0.0f, 0.0f, 0.0f);
    static TVec3f sOffset92C(0.0f, 0.0f, 5000.0f);
    static TVec3f sDropPoint938(0.0f, 1150.0f, -200.0f);
    static TVec3f sOrigin944(900.0f, 0.0f, 200.0f);
    static TVec3f sOffset950(400.0f, 0.0f, 0.0f);
    static TVec3f sRange95C(1400.0f, 1750.0f, 400.0f);
    static TVec3f sPushOffset(0.0f, 250.0f, 0.0f);
    static BattanKingVolumeParam sVolumeParam[8] = {
        {TVec3f(1.6f, 1.0f, 2.0f), TVec3f(0.0f, -480.0f, 1500.0f)},
        {TVec3f(0.4f, 1.0f, 0.2f), TVec3f(900.0f, -480.0f, 1550.0f)},
        {TVec3f(0.4f, 1.0f, 0.2f), TVec3f(-900.0f, -480.0f, 1550.0f)},
        {TVec3f(0.5f, 1.0f, 0.5f), TVec3f(1200.0f, -480.0f, 1550.0f)},
        {TVec3f(0.5f, 1.0f, 0.5f), TVec3f(-1200.0f, -480.0f, 1550.0f)},
        {TVec3f(0.5f, 1.0f, 0.8f), TVec3f(430.0f, -480.0f, 350.0f)},
        {TVec3f(0.5f, 1.0f, 0.8f), TVec3f(-430.0f, -480.0f, 350.0f)},
        {TVec3f(13.0f, 1.0f, 13.0f), TVec3f(0.0f, -480.0f, 1500.0f)},
    };
    static TVec3f sVolumeScale(0.08f, 0.0f, 0.08f);
}  // namespace BattanKingVec



namespace BattanKingParam {
    const f32 sRun[4] = {0.8f, 0.5f, 0.95f, 1.8f};
    const f32 sDash[4] = {1.4f, 0.5f, 0.95f, 1.8f};
    const f32 sTurn[4] = {0.0f, 0.5f, 0.95f, 1.8f};
    const f32 sReturn[4] = {0.2f, 0.5f, 0.95f, 1.8f};
    const volatile f32 sAttackTurnRate = 0.0f;
    const volatile s32 sInitialPattanCount = 0;
    const volatile s32 sPushStartStep = 0;
    const volatile s32 sAttackSignShakeFrame = 80;
    const volatile s32 sVolumeDrawPasses = 4;
    const volatile s32 sEarthquakeDrawPasses = 1;
    const GXColor sVolumeColor = {0x30, 0x00, 0x00, 0xFF};
    const GXColor sEarthquakeColor = {0xF6, 0x00, 0x00, 0x4E};
}  // namespace BattanKingParam

void BattanKing::moveAndTurnToPlayer(const f32* params) {
    MR::moveAndTurnToPlayer(this, &mFront, params[0], params[1], params[2], params[3]);
}

// @A580
// Builds the matrix of one volume drawer from the actor's base matrix, a (scale, offset) pair and the shake offset.
// NONMATCHING: register allocation/scheduling only (target keeps &offset in r31 across the copy-ctor call)
static void calcVolumeMtx(MtxPtr pMtx, LiveActor* pActor, const BattanKingVolumeParam* pParam, const TVec3f& rShake) {
    JMath::gekko_ps_copy12(pMtx, pActor->getBaseMtx());
    TVec3f offset(pParam->mOffset);
    offset += rShake;
    MR::addTransMtxLocal(pMtx, offset);
    MR::preScaleMtx(pMtx, pParam->mScale);
}


BattanKing::BattanKing(const char* pName) : LiveActor(pName) {
    mCollisions = NULL;
    mCollisionPresses = NULL;
    mBreakModel = NULL;
    mVolumeDrawers = NULL;
    mShake.x = 0.0f;
    mShake.y = 0.0f;
    mShake.z = 0.0f;
    mVolumeMtxs = NULL;
    mEarthquakeDrawer = NULL;
    mCameraInfo = NULL;
    mTalkStateA = NULL;
    mTalkStateB = NULL;
    mPattanHolder = NULL;
    mFront.x = 0.0f;
    mFront.y = 0.0f;
    mFront.z = 1.0f;
    mPhase = 0;
    mAttackType = -1;
    mUnk1B0.x = 0.0f;
    mUnk1B0.y = 0.0f;
    mUnk1B0.z = 0.0f;
    mUnk1BC.x = 0.0f;
    mUnk1BC.y = 0.0f;
    mUnk1BC.z = 0.0f;
    mArg7 = -1;
    mFlagA = 0;
    mFlagB = 0;
    mFlagC = 0;
    mMtxFx.identity();
    mMtxBreak.identity();
    mMtxFollow.identity();
}

void BattanKing::init(const JMapInfoIter& rIter) {
    PattanHolder* pHolder;
    const char* pPattanName;
    MR::initActor(this, rIter, "BattanKing", false);

    mCollisions = new BattanKingCollision*[5];
    mCollisionPresses = new BattanKingCollisionPress*[5];
    for (s32 i = 0; i < 5; i++) {
        mCollisions[i] = new BattanKingCollision(this, sCollisionPartsInfo[i].mActorName, sCollisionPartsInfo[i].mPartsName, NULL, MR::getJointMtx(this, sCollisionPartsInfo[i].mJointName), 0x1E);
        mCollisions[i]->initWithoutIter();

        mCollisionPresses[i] = new BattanKingCollisionPress(this, sCollisionPartsInfo[i].mActorName, sCollisionPartsInfo[i].mPartsName, NULL, MR::getJointMtx(this, sCollisionPartsInfo[i].mJointName), 0x1E);
        mCollisionPresses[i]->initWithoutIter();
    }

    mVolumeMtxs = new TPos3f[7];

    mVolumeDrawers = new VolumeDrawer*[7];
    mVolumeDrawers[0] = MR::createVolumeDrawer("Body", "VolumeBox", mVolumeMtxs[0]);
    mVolumeDrawers[1] = MR::createVolumeDrawer("ArmL", "VolumeBox", mVolumeMtxs[1]);
    mVolumeDrawers[2] = MR::createVolumeDrawer("ArmR", "VolumeBox", mVolumeMtxs[2]);
    mVolumeDrawers[3] = MR::createVolumeDrawer("HandL", "VolumeCylinder", mVolumeMtxs[3]);
    mVolumeDrawers[4] = MR::createVolumeDrawer("HandR", "VolumeCylinder", mVolumeMtxs[4]);
    mVolumeDrawers[5] = MR::createVolumeDrawer("LegL", "VolumeBox", mVolumeMtxs[5]);
    mVolumeDrawers[6] = MR::createVolumeDrawer("LegR", "VolumeBox", mVolumeMtxs[6]);
    mEarthquakeDrawer = MR::createVolumeDrawer("地震範囲描画", "VolumeBox", mEarthquakeMtx);

    pHolder = new PattanHolder("パタン保持", 10);
    mPattanHolder = pHolder;
    pPattanName = "パタン";
    for (s32 i = 0; i < pHolder->mObjNumMax; i++) {
        Pattan* pPattan = new Pattan(pPattanName);
        pPattan->initWithoutIter();
        pHolder->registerActor(pPattan);
    }
    mPattanHolder->makeAllDead();

    mBreakModel = new BreakModel(this, "バッタンキング(Break)", "BattanKingBreak", getBaseMtx(), "Break");
    mBreakModel->initWithoutIter();

    MR::getJMapInfoArg7WithInit(rIter, &mArg7);
    if (mArg7 != -1) {
        MR::declareCameraRegisterVec(this, mArg7, &mUnk1B0);
        MR::declareCameraRegisterVec(this, mArg7 + 1, &mUnk1BC);
    }

    mCameraInfo = new ActorCameraInfo(rIter);

    CollisionPartsFilterDelegator< BattanKing >::DelegateFilter partsFilter = &BattanKing::isInvalidCollisionParts;
    MR::setBinderCollisionPartsFilter(this, new CollisionPartsFilterDelegator< BattanKing >(this, partsFilter));

    MR::tryRegisterNamePosLinkObj(this, rIter, "バッタンキング基準位置");
    MR::tryRegisterNamePosLinkObj(this, rIter, "マリオ位置");
    MR::findLinkNamePos(this, "バッタンキング基準位置", mMtxFx);
    MR::setEffectHostMtx(this, "AttackShockWave", mMtxFx);
    MR::setEffectHostMtx(this, "QuakeShockWave", mMtxFx);
    MR::setEffectHostMtx(this, "AttackSign", mMtxFx);
    MR::setEffectHostMtx(this, "QuakeSign", mMtxFx);

    MR::calcFrontVec(&mFront, this);
    mUnk180 = mPosition;
    mUnk18C = mFront;
    MR::declarePowerStar(this);
    MR::makeMtxTR(mMtxFollow, this);

    BaseMatrixFollowValidateDelegator<BattanKing>::Func validate = &BattanKing::isValidCollisionState;
    MR::addBaseMatrixFollowTarget(this, rIter, &mMtxFollow, new BaseMatrixFollowValidateDelegator<BattanKing>(this, validate));

    initNerve(&NrvBattanKing::BattanKingNrvWaitStand::sInstance, 2);

    mTalkStateA = new BattanKingTalkState(this, rIter, "バッタンキング登場", "DemoBattanKingOpening", "BattanKing000");
    mTalkStateB = new BattanKingTalkState(this, rIter, "バッタンキングダウン", "DemoBattanKingDown", "BattanKing001");
    mTalkStateB->mFlag18 = 1;

    for (s32 i = 0; i < 5; i++) {
        MR::registerDemoCast(mCollisions[i], "バッタンキング登場", rIter);
        MR::registerDemoCast(mCollisions[i], "バッタンキングダウン", rIter);
        MR::registerDemoCast(mCollisionPresses[i], "バッタンキング登場", rIter);
        MR::registerDemoCast(mCollisionPresses[i], "バッタンキングダウン", rIter);
    }
    MR::registerDemoCast(mBreakModel, "バッタンキングダウン", rIter);

    MR::initActorState(this, mTalkStateA, &NrvBattanKing::BattanKingNrvOpeningDemo::sInstance, NULL);
    MR::initActorState(this, mTalkStateB, &NrvBattanKing::BattanKingNrvDownDemo::sInstance, NULL);

    makeActorAppeared();
    MR::validateClipping(this);
}

// @ADA0
void BattanKing::makeActorAppeared() {
    setNerve(&NrvBattanKing::BattanKingNrvWaitSwitch::sInstance);
    LiveActor::makeActorAppeared();
    resetVolumes();
    showCollisionPartsNormal();
    updateJointVisibility();
}

void BattanKing::makeActorDead() {
    LiveActor::makeActorDead();
    for (s32 i = 0; i < 7; i++) {
        mVolumeDrawers[i]->makeActorDead();
    }
    mEarthquakeDrawer->makeActorDead();
    mPattanHolder->makeAllDead();
}

// NONMATCHING: the target addresses the volume params off one .bss base register (section anchor) and loads the
// frame/draw-pass params (80/4/1) from .sdata2 where ours folds them to immediates; calls, values and branches are identical
void BattanKing::control() {
    if (!MR::isDead(this)) {
        updateShockPos();
    } else {
        mUnk1B0.set< f32 >(mUnk180);
        JMAVECScaleAdd(&mGravity, &mUnk1B0, &mUnk1B0, -300.0f);
    }

    if (isNerve(&NrvBattanKing::BattanKingNrvAttackSign::sInstance)) {
        f32 rate = MR::calcNerveEaseOutRate(this, BattanKingParam::sAttackSignShakeFrame);
        f32 halfTurn = 180.0f;
        mShake.z = 5.0f * JMath::sSinCosTable.sinDegree(halfTurn * 30.0f * rate);
    } else {
        mShake.y = 0.0f;
    }

    calcVolumeMtx(mVolumeMtxs[0], this, &BattanKingVec::sVolumeParam[0], mShake);
    calcVolumeMtx(mVolumeMtxs[1], this, &BattanKingVec::sVolumeParam[1], mShake);
    calcVolumeMtx(mVolumeMtxs[2], this, &BattanKingVec::sVolumeParam[2], mShake);
    calcVolumeMtx(mVolumeMtxs[3], this, &BattanKingVec::sVolumeParam[3], mShake);
    calcVolumeMtx(mVolumeMtxs[4], this, &BattanKingVec::sVolumeParam[4], mShake);
    calcVolumeMtx(mVolumeMtxs[5], this, &BattanKingVec::sVolumeParam[5], mShake);
    calcVolumeMtx(mVolumeMtxs[6], this, &BattanKingVec::sVolumeParam[6], mShake);

    for (s32 i = 0; i < 7; i++) {
        mVolumeDrawers[i]->setColor(Color8(BattanKingParam::sVolumeColor));
        mVolumeDrawers[i]->setDrawPasses(BattanKingParam::sVolumeDrawPasses);
        mVolumeDrawers[i]->setScale(BattanKingVec::sVolumeScale);
    }

    calcVolumeMtx(mEarthquakeMtx, this, &BattanKingVec::sVolumeParam[7], mShake);
    mEarthquakeDrawer->setColor(Color8(BattanKingParam::sEarthquakeColor));
    mEarthquakeDrawer->setDrawPasses(BattanKingParam::sEarthquakeDrawPasses);
    mEarthquakeDrawer->setScale(BattanKingVec::sVolumeScale);

    if (mAttackType > 0) {
        mAttackType--;
        if (MR::isOnGroundPlayer() && !isAnyCollisionOnActor() && !MR::isPlayerDead()) {
            MR::noticePlayerDashChance();
        }

        for (s32 i = 0; i < mPattanHolder->mObjNum; i++) {
            Pattan* pPattan = static_cast< Pattan* >(mPattanHolder->getActor(i));
            if (!MR::isDead(pPattan)) {
                pPattan->tryPattanJump();
            }
        }
    }
}

void BattanKing::exeWaitStand() {
    if (MR::isFirstStep(this)) {
        mFlagA = 0;
        MR::startAction(this, "WaitStand");
        MR::zeroVelocity(this);
        MR::offBind(this);
        showCollisionPartsNormal();
    }
    if (mPhase == 2) MR::tryEmitEffect(this, "Weaken");
    if (MR::isGreaterEqualStep(this, 30)) {
        showCollisionPartsNormal();
        MR::onBind(this);
        if (MR::isPlayerInCollisionBox((const TPos3f*)getBaseMtx(), &BattanKingVec::sPlayerDetectSize))
            setNerve(&NrvBattanKing::BattanKingNrvFindOrSummon::sInstance);
        else setNerve(&NrvBattanKing::BattanKingNrvTurn::sInstance);
    }
}

void BattanKing::exeTurn() {
    if (MR::isFirstStep(this)) {
        if (MR::isPlayerLeftSide(this)) MR::startAction(this, "TurnLeft");
        else MR::startAction(this, "TurnRight");
        updateCollisionPartsFootState();
    }
    if (mPhase == 2) MR::tryEmitEffect(this, "Weaken");
    if (BattanKingCollisionUtil::isFacingOutsideDemoBounds(this)) {
        TVec3f target;
        MR::tryFindLinkNamePos(this, "バッタンキング基準位置", &target, 0);
        MR::moveAndTurnToTarget(this, &mFront, target, BattanKingParam::sReturn[0], BattanKingParam::sReturn[1], BattanKingParam::sReturn[2], BattanKingParam::sReturn[3]);
    } else moveAndTurnToPlayer(BattanKingParam::sTurn);
    if ((MR::isGreaterStep(this, 15) && MR::isPlayerInCollisionBox((const TPos3f*)getBaseMtx(), &BattanKingVec::sPlayerDetectSize)) || MR::isGreaterStep(this, 120))
        setNerve(&NrvBattanKing::BattanKingNrvFindOrSummon::sInstance);
}

void BattanKing::exeFindOrSummon() {
    if (MR::isFirstStep(this)) {
        if (mPhase == 0 && BattanKingParam::sInitialPattanCount == 0) {
            MR::startAction(this, "Find"); updateCollisionPartsFootState();
        } else { MR::startAction(this, "Summon"); updateCollisionPartsFootState(); }
        MR::invalidateClipping(this); MR::zeroVelocity(this);
    }
    if (mPhase == 2) MR::tryEmitEffect(this, "Weaken");
    if (MR::isStep(this, 50)) spawnPattans();
    if (MR::isActionEnd(this)) {
        showCollisionPartsNormal();
        if (mPhase == 0 && BattanKingParam::sInitialPattanCount == 0) setNerve(&NrvBattanKing::BattanKingNrvRun::sInstance);
        else setNerve(&NrvBattanKing::BattanKingNrvWaitSummon::sInstance);
    }
}

void BattanKing::exeRun() {
    if (MR::isFirstStep(this)) {
        if (mPhase == 2) MR::startAction(this, "Dash"); else MR::startAction(this, "Run");
        MR::invalidateClipping(this); updateCollisionPartsFootState();
    }
    if (mPhase == 2) MR::tryEmitEffect(this, "Weaken");
    if (MR::isGreaterStep(this, 60) && MR::isPlayerInCollisionBox((const TPos3f*)getBaseMtx(), &BattanKingVec::sPlayerAttackSize)) {
        MR::zeroVelocity(this); setNerve(&NrvBattanKing::BattanKingNrvAttackJump::sInstance); return;
    }
    if (MR::isGreaterStep(this, 180) || BattanKingCollisionUtil::isFacingOutsideDemoBounds(this)) {
        showCollisionPartsNormal(); MR::zeroVelocity(this); setNerve(&NrvBattanKing::BattanKingNrvAttackJump::sInstance); return;
    }
    if (mPhase == 2) moveAndTurnToPlayer(BattanKingParam::sDash);
    else moveAndTurnToPlayer(BattanKingParam::sRun);
}

void BattanKing::exeWaitSummon() {
    if (MR::isFirstStep(this)) MR::startAction(this, "WaitSummon");
    if (mPhase == 2) MR::tryEmitEffect(this, "Weaken");
    if (MR::isGreaterStep(this, 111)) setNerve(&NrvBattanKing::BattanKingNrvQuake::sInstance);
}

void BattanKing::exeQuake() {
    if (MR::isFirstStep(this)) { MR::startAction(this, "Quake"); mEarthquakeDrawer->setScaleUpRate(20); }
    if (mPhase == 2) MR::tryEmitEffect(this, "Weaken");
    if (MR::isStep(this, 57)) updateCollisionPartsFootState();
    if (MR::isStep(this, 77)) mAttackType = 20;
    if (MR::isGreaterStep(this, 15)) moveAndTurnToPlayer(BattanKingParam::sTurn);
    if (MR::isActionEnd(this)) {
        showCollisionPartsNormal(); mEarthquakeDrawer->setScaleDownRate(20);
        if (isNerve(&NrvBattanKing::BattanKingNrvQuakeAgain::sInstance)) setNerve(&NrvBattanKing::BattanKingNrvRun::sInstance);
        else if (mPhase == 2) setNerve(&NrvBattanKing::BattanKingNrvQuakeAgain::sInstance);
        else setNerve(&NrvBattanKing::BattanKingNrvRun::sInstance);
    }
}

void BattanKing::exeAttackJump() {
    if (MR::isFirstStep(this)) MR::startAction(this, "AttackJump");
    if (mPhase == 2) MR::tryEmitEffect(this, "Weaken");
    if (MR::isActionEnd(this)) setNerve(&NrvBattanKing::BattanKingNrvAttackSign::sInstance);
}

void BattanKing::exeAttackSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSign"); MR::invalidateClipping(this); MR::offBind(this); MR::zeroVelocity(this);
        for (s32 i=0; i<7; ++i) mVolumeDrawers[i]->setScaleUpRate(20);
        mEarthquakeDrawer->setScaleUpRate(20);
    }
    if (mPhase == 2) MR::tryEmitEffect(this, "Weaken");
    if (MR::isGreaterEqualStep(this, 80)) setNerve(&NrvBattanKing::BattanKingNrvAttack::sInstance);
}

void BattanKing::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack"); MR::invalidateClipping(this); showCollisionPartsSwapped(); mFlagB=0; mFlagC=0;
    }
    if (MR::isLessStep(this, 20)) {
        TVec3f toPlayer(0.0f); MR::calcVecToPlayerH(&toPlayer, this, 0);
        MR::vecBlendSphere(&mFront, mFront, toPlayer, BattanKingParam::sAttackTurnRate); MR::normalize(&mFront);
    } else MR::zeroVelocity(this);
    if (MR::isStep(this, 61)) {
        for (s32 i=0; i<7; ++i) mVolumeDrawers[i]->setScaleDownRate(20);
        mEarthquakeDrawer->setScaleDownRate(20); updateAttackAnim(20);
    }
    if (MR::isLessStep(this, 61)) { if (mPhase == 2) MR::tryEmitEffect(this, "Weaken"); }
    else MR::tryDeleteEffect(this, "Weaken");
    if (MR::isActionEnd(this)) setNerve(&NrvBattanKing::BattanKingNrvWaitFall::sInstance);
}

void BattanKing::exeWaitFall() {
    if (MR::isFirstStep(this)) { MR::startAction(this, "WaitFall"); MR::invalidateClipping(this); showCollisionPartsNormal(); }
    s32 duration=150; if (mPhase == 0) duration=180;
    if (MR::isGreaterEqualStep(this,duration)) setNerve(&NrvBattanKing::BattanKingNrvRiseSign::sInstance);
}

void BattanKing::exeRiseSign() {
    if (MR::isFirstStep(this)) { MR::startAction(this, "RiseSign"); MR::invalidateClipping(this); }
    if (MR::isGreaterEqualStep(this,120)) setNerve(&NrvBattanKing::BattanKingNrvRise::sInstance);
}

void BattanKing::exeRise() {
    if (MR::isFirstStep(this)) { MR::startAction(this, "Rise"); MR::invalidateClipping(this); updateCollisionPartsFootState(); mFlagB=0; mFlagC=0; }
    if (mPhase == 2) MR::tryEmitEffect(this, "Weaken");
    if (MR::isActionEnd(this)) { showCollisionPartsNormal(); setNerve(&NrvBattanKing::BattanKingNrvWaitStand::sInstance); MR::onBind(this); }
}

#ifdef __MWERKS__
#pragma push
#pragma auto_inline off
#endif
void BattanKing::exeDamage() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Damage"); updateCollisionPartsFootState();
        PSMTXCopy(MR::getJointMtx(this, "Root"), mMtxBreak.mMtx);
        TVec3f offset(BattanKingVec::sDropPoint938); f32 back = 300.0f; offset.z += -back;
        MR::addTransMtxLocal(mMtxBreak.mMtx,offset);
        TVec3f position; MR::extractMtxTrans(mMtxBreak.mMtx,&position); MR::emitItemBossDamageAtPos(this,&position);
    }
    if (MR::isActionEnd(this)) { showCollisionPartsNormal(); setNerve(&NrvBattanKing::BattanKingNrvWaitStand::sInstance); }
}

#ifdef __MWERKS__
#pragma pop
#endif
void BattanKing::exeWaitSwitch() {
    if (MR::isFirstStep(this)) MR::hideModel(this);
    if (MR::isOnSwitchA(this)) { MR::invalidateClipping(this); setNerve(&NrvBattanKing::BattanKingNrvOpeningDemo::sInstance); }
}

void BattanKing::exeOpeningDemo() {
    if (MR::isFirstStep(this)) { MR::stopStageBgmForDemo(); MR::invalidateShadowAll(this); MR::setPosition(this,mUnk180); }
    if (MR::updateActorStateAndNextNerve(this,&NrvBattanKing::BattanKingNrvFindOrSummon::sInstance)) {
        mPosition=mUnk180; mFront=mUnk18C;
        MR::startStageBGM("MBGM_SMG2_BOSS_08A",false);
        MR::startAction(this,"Find"); MR::startBckNoInterpole(this,"Find");
    }
}

void BattanKing::exeDead() {
    if (MR::isFirstStep(this)) { mPattanHolder->makeAllDead(); MR::startAction(this,"Damage"); }
    if (MR::isGreaterStep(this,30)) setNerve(&NrvBattanKing::BattanKingNrvDownDemo::sInstance);
}

#ifdef __MWERKS__
#pragma push
#pragma dont_inline on
#endif
void BattanKing::exeDownDemo() {
    if (MR::isFirstStep(this)) { MR::stopStageBGM(10); showCollisionPartsNormal(); }
    if (MR::updateActorState(this)) {
        MR::tryHitReactionDead(this);
        TMtx34f mtx; PSMTXCopy(mMtxFx.mMtx,mtx.mMtx); MR::addTransMtxLocal(mtx.mMtx,BattanKingVec::sPowerStarOffset);
        TVec3f position(0.0f); MR::extractMtxTrans(mtx.mMtx,&position); MR::requestAppearPowerStar(this,position);
        mUnk1B0.set<f32>(mUnk180); JMAVECScaleAdd(&mGravity,&mUnk1B0,&mUnk1B0,-300.0f); kill();
    }
}

#ifdef __MWERKS__
#pragma pop
#endif

void BattanKing::calcAndSetBaseMtx() {
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFront);
    PSMTXCopy(getBaseMtx(), mMtxFollow);
    MR::addTransMtxLocal(mMtxFollow, BattanKingVec::sOffset920);
    MR::calcFrontVec(&mFront, this);
}

// NONMATCHING: regalloc
void BattanKing::calcAnim() {
    LiveActor::calcAnim();
    if (MR::isDead(this) || MR::isNoCalcAnim(this) || MR::isClipped(this)) {
        return;
    }
    for (s32 i = 0; i < 5; i++) {
        mCollisions[i]->calcAnim();
        mCollisionPresses[i]->calcAnim();
    }
}

// NONMATCHING: regalloc
void BattanKing::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "Push")) {
        if (isNerve(&NrvBattanKing::BattanKingNrvAttack::sInstance) && MR::isGreaterEqualStep(this, 0x28) &&
            MR::isLessStep(this, 0x3F)) {
            TVec3f push(0.0f);
            if (calcPlayerPush(&push)) {
                MR::pushPlayer(push * 1.0f);
            }
        } else if (isNerve(&NrvBattanKing::BattanKingNrvRise::sInstance) && MR::isGreaterEqualStep(this, BattanKingParam::sPushStartStep) &&
                   MR::isLessStep(this, 0x6E)) {
            TVec3f push(0.0f);
            if (calcPlayerPush(&push)) {
                MR::pushPlayer(push * 1.0f);
            }
        } else {
            mFlagB = 0;
            mFlagC = 0;
        }
    } else if (MR::isSensorPlayer(pReceiver)) {
        if (isNerve(&NrvBattanKing::BattanKingNrvAttack::sInstance)) {
            if (MR::isEqualSensor(pSender, this, "HandL") || MR::isEqualSensor(pSender, this, "HandR")) {
                MR::sendMsgPush(pReceiver, pSender);
                return;
            }
        } else {
            bool isWalking = false;
            if (isNerve(&NrvBattanKing::BattanKingNrvTurn::sInstance) || isNerve(&NrvBattanKing::BattanKingNrvRun::sInstance)) isWalking = true;
            if (isWalking) {
                if (MR::isEqualSensor(pSender, this, "LegL") || MR::isEqualSensor(pSender, this, "LegR") ||
                    MR::isEqualSensor(pSender, this, "FootL") || MR::isEqualSensor(pSender, this, "FootR")) {
                    MR::sendMsgPush(pReceiver, pSender);
                    return;
                }
            }
        }
    } else {
        bool isWalking = false;
        if (isNerve(&NrvBattanKing::BattanKingNrvTurn::sInstance) || isNerve(&NrvBattanKing::BattanKingNrvRun::sInstance)) isWalking = true;
        if (isWalking && MR::isSensorEnemy(pReceiver)) {
            if (MR::sendArbitraryMsg(0xF4, pReceiver, pSender)) return;
        }
    }
}

bool BattanKing::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgPlayerHipDropFloor(msg)) {
        if (MR::isPlayerInCollisionCylinder(MR::getJointMtx(this, "Root"), &BattanKingVec::sDropPoint938, 450.0f, 300.0f)) {
            mAttackType = 0;
            return tryAdvanceAttack();
        }
        return false;
    }
    return false;
}

// NONMATCHING: regalloc
bool BattanKing::receiveMsgEnemyAttack(u32, HitSensor*, HitSensor* pReceiver) {
    if (!MR::isSensorAutoRush(pReceiver)) {
        return false;
    } else {
        return MR::isPlayerElementModeInvincible();
    }
}

bool BattanKing::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return false;
}

void BattanKing::resetVolumes() {
    mPhase = 0;
    for (s32 i = 0; i < 7; i++) {
        mVolumeDrawers[i]->makeActorAppeared();
        mVolumeDrawers[i]->setScaleDownRate(0);
    }
    mEarthquakeDrawer->makeActorAppeared();
    mEarthquakeDrawer->setScaleDownRate(0);
    mAttackType = 0;
    mTalkStateA->kill();
    mTalkStateB->kill();
    hideCollisionPartsAll();
}

bool BattanKing::isAttackPhaseStart() const {
    bool result = false;
    if (mFlagA == 0 && (isNerve(&NrvBattanKing::BattanKingNrvWaitFall::sInstance) || isNerve(&NrvBattanKing::BattanKingNrvRiseSign::sInstance) ||
        (isNerve(&NrvBattanKing::BattanKingNrvRise::sInstance) && MR::isLessStep(this, 0x28)))) {
        result = true;
    }
    return result;
}

bool BattanKing::tryAdvanceAttack() {
    if (isAttackPhaseStart()) {
        s32 phase = mPhase;
        mFlagA = 1;
        mPhase = phase + 1;
        updateJointVisibility();
        if (mPhase < 3) {
            setNerve(&NrvBattanKing::BattanKingNrvDamage::sInstance);
            MR::tryHitReaction(this, "ヒップドロップ");
            return true;
        }
        MR::tryDeleteEffect(this, "Weaken");
        setNerve(&NrvBattanKing::BattanKingNrvDead::sInstance);
        MR::tryHitReaction(this, "ヒップドロップ（最後）");
        return true;
    }
    return false;
}

// NONMATCHING: regalloc
void BattanKing::spawnPattans() {
    const TVec3f* gravity;
    s32 count = 0;
    if (mPhase == 0) count = BattanKingParam::sInitialPattanCount;
    if (mPhase == 1) count = 3;
    if (mPhase == 2) count = 5;
    gravity = &mGravity;
    for (s32 i = 0; i < count; i++) {
        PattanHolder* holder = mPattanHolder;
        LiveActor* actor = holder->getDeadActor() ? holder->getDeadActor() : nullptr;
        if (actor == nullptr) break;
        f32 angle = (360.0f * (f32)i) / (f32)count;
        TVec3f offset(BattanKingVec::sOrigin944);
        MR::rotateVecDegree(&offset, offset, *gravity, angle);
        TMtx34f mtx;
        PSMTXCopy(getBaseMtx(), mtx);
        MR::addTransMtxLocal(mtx, offset);
        MR::extractMtxTrans(mtx, &actor->mPosition);
        actor->mGravity = *gravity;
        TVec3f front(mFront);
        MR::calcVecToPlayerH(&front, actor, nullptr);
        appearPattanByJump(actor, &front);
    }
}

void BattanKing::updateJointVisibility() {
    MR::hideJoint(this, "Back1");
    MR::hideJoint(this, "Back2");
    MR::hideJoint(this, "Back3");
    if (mPhase == 0) {
        MR::showJoint(this, "Back1");
    }
    if (mPhase == 1) {
        MR::showJoint(this, "Back2");
    }
    if (mPhase == 2) {
        MR::showJoint(this, "Back3");
    }
    if (mPhase == 3) {
        MR::showJoint(this, "Back3");
    }
}

void BattanKing::updateShockPos() {
    TVec3f pos(0.0f);
    TVec3f origin;
    origin = mPosition;
    TVec3f dir;
    MR::calcVecToPlayerH(&dir, this, nullptr);
    pos.set<f32>(origin);
    JMAVECScaleAdd(mGravity, pos, pos, -1000.0f);
    JMAVECScaleAdd(dir, pos, pos, -100.0f);
    mUnk1B0 = pos;
    TMtx34f mtx;
    PSMTXCopy(getBaseMtx(), mtx);
    MR::addTransMtxLocal(mtx, BattanKingVec::sOffset92C);
    f32 z = mtx[2][3];
    f32 y = mtx[1][3];
    f32 x = mtx[0][3];
    mUnk1BC.set<f32>(x, y, z);
}

void BattanKing::showCollisionPartsNormal() {
    for (s32 i = 0; i < 5; i++) {
        mCollisions[i]->appear();
        mCollisionPresses[i]->kill();
    }
}

void BattanKing::showCollisionPartsSwapped() {
    for (s32 i = 0; i < 5; i++) {
        mCollisions[i]->kill();
        mCollisionPresses[i]->appear();
    }
}

void BattanKing::updateCollisionPartsFootState() {
    showCollisionPartsNormal();
    mCollisions[3]->kill();
    mCollisions[4]->kill();
    mCollisionPresses[3]->appear();
    mCollisionPresses[4]->appear();
}

void BattanKing::hideCollisionPartsAll() {
    for (s32 i = 0; i < 5; i++) {
        mCollisions[i]->kill();
        mCollisionPresses[i]->kill();
    }
}

bool BattanKing::isAnyCollisionOnActor() {
    for (s32 i = 0; i < 5; i++) {
        if (MR::isOnPlayer(mCollisions[i])) {
            return true;
        }
        if (MR::isOnPlayer(mCollisionPresses[i])) {
            return true;
        }
    }
    return MR::isOnPlayer(this);
}

void BattanKing::restoreDemoPose() {
    MR::setPosition(this, mUnk180);
    mFront = mUnk18C;
    calcAndSetBaseMtx();
}

void BattanKing::updateDemoPlayerBaseMtx() {
    TMtx34f mtx;
    PSMTXCopy(mMtxFx, mtx);
    MR::addTransMtxLocal(mtx, BattanKingVec::sOffset914);
    MR::rotateMtxLocalYDegree(mtx, 40.0f);
    MR::setPlayerBaseMtx(mtx);
}

bool BattanKing::isValidCollisionState(s32 id) const {
    if (id == 0) {
        if ((isNerve(&NrvBattanKing::BattanKingNrvAttack::sInstance) && MR::isGreaterEqualStep(this, 0x3D)) ||
            isNerve(&NrvBattanKing::BattanKingNrvWaitFall::sInstance) || isNerve(&NrvBattanKing::BattanKingNrvRiseSign::sInstance) ||
            isNerve(&NrvBattanKing::BattanKingNrvDamage::sInstance)) {
            return true;
        }
        return false;
    } else if (id == 1) {
        if (!isNerve(&NrvBattanKing::BattanKingNrvWaitSwitch::sInstance) && !isNerve(&NrvBattanKing::BattanKingNrvOpeningDemo::sInstance) &&
            !isNerve(&NrvBattanKing::BattanKingNrvDead::sInstance) && !isNerve(&NrvBattanKing::BattanKingNrvDownDemo::sInstance)) {
            return true;
        }
        return false;
    } else if (id == 2) {
        if ((!isNerve(&NrvBattanKing::BattanKingNrvAttack::sInstance) || !MR::isGreaterEqualStep(this, 0x3D)) &&
            !isNerve(&NrvBattanKing::BattanKingNrvWaitFall::sInstance) && !isNerve(&NrvBattanKing::BattanKingNrvRiseSign::sInstance) &&
            !isNerve(&NrvBattanKing::BattanKingNrvDamage::sInstance) && !isNerve(&NrvBattanKing::BattanKingNrvWaitSwitch::sInstance) &&
            !isNerve(&NrvBattanKing::BattanKingNrvOpeningDemo::sInstance) && !isNerve(&NrvBattanKing::BattanKingNrvDead::sInstance) &&
            !isNerve(&NrvBattanKing::BattanKingNrvDownDemo::sInstance)) {
            return true;
        }
        return false;

    }
    return false;
}

bool BattanKing::isInvalidCollisionParts(const CollisionParts* pParts) {
    LiveActor* pHost = pParts->mHitSensor->mHost;
    for (s32 i = 0; i < 5; i++) {
        if (mCollisions[i] == pHost) return true;
        if (mCollisionPresses[i] == pHost) return true;
    }
    return false;
}

void BattanKing::updateAttackAnim(s32 phase) {
    mAttackType = phase;
    if (MR::isOnGroundPlayer() && !isAnyCollisionOnActor()) {
        MR::tryHitReaction(this, "攻撃・プレイヤー地上");
    } else {
        MR::tryHitReaction(this, "攻撃・プレイヤー空中");
    }
}

// NONMATCHING: regalloc
bool BattanKing::calcPlayerPush(TVec3f* pPush) {
    u8 wasFront = mFlagB;
    u8 wasInside = mFlagC;
    TMtx34f inverse;
    TMtx34f mtx;
    PSMTXCopy(MR::getJointMtx(this, "BattanKingBody"), mtx);
    MR::addTransMtxLocal(mtx, BattanKingVec::sPushOffset);
    PSMTXInverse(mtx, inverse);
    TVec3f local(0.0f);
    PSMTXMultVec(inverse, MR::getPlayerCenterPos(), local);
    f32 z = local.z;
    bool inside = isInsidePushBox(local, BattanKingVec::sRange95C, 600.0f);
    u8 isFront = z >= 0.0f;
    mFlagB = isFront;
    if (!inside) return false;
    mFlagC = 1;
    MR::extractMtxZDir(MR::getJointMtx(this, "BattanKingBody"), pPush);
    f32 edge;
    if (wasFront) edge = 0.5f * BattanKingVec::sRange95C.z;
    else edge = 0.5f * -BattanKingVec::sRange95C.z;
    *pPush *= edge - z;
    if (wasInside && isFront != wasFront) {
        mFlagB = wasFront;
        return true;
    }
    if (z < 0.5f * BattanKingVec::sRange95C.z) return true;
    return false;
}
