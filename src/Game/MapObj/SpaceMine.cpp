#include "Game/MapObj/SpaceMine.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/BenefitItemOneUp.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/Util.hpp"
#include "Game/Util/YoshiLockOnUtil.hpp"

namespace {
    const f32 sRotateSpeedNormal = 1.0f;
    const f32 sRotateSpeedFast = 1.0f;
    // sDeadTime
    // sAppearValidDistance
    const f32 sSensorRadius = 100.0f;
    const f32 sDistanceNear = 1500.0f;
    const f32 sDistanceFar = 1450.0f;
    const s32 sAppearEffectTime = 120;
};  // namespace

namespace NrvSpaceMine {
    NEW_NERVE(HostTypeWait, SpaceMine, Wait);
    NEW_NERVE(HostTypeAppear, SpaceMine, Appear);
    NEW_NERVE(HostTypeReaction, SpaceMine, Reaction);

    class HostTypePause : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            SpaceMine* actor = reinterpret_cast<SpaceMine*>(pSpine->mExecutor);
            actor->exePause();
        }

        virtual void executeOnEnd(Spine* pSpine) const {
            SpaceMine* actor = reinterpret_cast<SpaceMine*>(pSpine->mExecutor);
            actor->getSensor("Body")->setType(0x40);
            actor->resumeMapPartsFunctions();
        }

        static HostTypePause sInstance;
    };
    HostTypePause HostTypePause::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvSpaceMine

SpaceMine::SpaceMine(const char* pName)
    : MapObjActor(pName), mRotateSpeed(::sRotateSpeedNormal), mClippingRange(gZeroVec), mShadowType(-1), mItemType(-1), mIsPlayerNear() {
    mBenefitItem = nullptr;
    mLockOnTarget = nullptr;
    mAnimScaleCtrl = nullptr;
    mStarPointerState = nullptr;
    mKinokoMtx.identity();
}

void SpaceMine::init(const JMapInfoIter& rIter) {
    MapObjActor::init(rIter);
    MapObjActorInitInfo info;
    info.setupHioNode("\x92\x6e\x8c\x60\x83\x49\x83\x75\x83\x57\x83\x46");
    info.setupDefaultPos();
    info.setupConnectToScene();
    info.setupEffect(nullptr, 4);
    info.setupSound(2, nullptr);
    info.setupNerve(GET_NERVE(SpaceMine, HostTypeWait));
    info.setupRailMover();
    info.setupHitSensor();
    TVec3f sensorOffs;
    sensorOffs.x = 0.0f;
    sensorOffs.y = 0.0f;
    sensorOffs.z = 0.0f;
    info.setupHitSensorParam(8, ::sSensorRadius, sensorOffs);
    info.setupGroupClipping(0x10);
    info.setupState(1);
    MR::getJMapInfoArg0NoInit(rIter, &mShadowType);

    bool isValid = isShadowValid(mShadowType);

    if (isValid) {
        info.setupShadow(nullptr);
    }

    s32 arg1 = -1;
    MR::getJMapInfoArg1NoInit(rIter, &arg1);

    if (arg1 != -1) {
        info.setupBinder(::sSensorRadius, 0.0f);
    }

    MR::getJMapInfoArg2NoInit(rIter, &mItemType);
    MR::declareCoin(this, 1);
    MR::declareStarPiece(this, 3);

    if (mItemType == 2) {
        mBenefitItem = MR::createKinokoOneUp();
        mBenefitItem->_DE = 2;
    }

    initialize(rIter, info);

    TVec3f zero;
    zero.x = 0.0f;
    zero.y = 0.0f;
    zero.z = 0.0f;
    MR::initStarPointerTarget(this, 110.0f, zero);
    mLockOnTarget = MR::createYoshiLockOnTarget(this, static_cast<MtxPtr>(nullptr), TVec3f(0.0f));
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mStarPointerState = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mStarPointerState, GET_NERVE(SpaceMine, HostTypePause), nullptr);

    if (isCalcShadowAlways()) {
        MR::onCalcShadow(this, nullptr);
        MR::onCalcShadowDropGravity(this, nullptr);
    }

    MR::makeMtxTR(mKinokoMtx.toMtxPtr(), this);
}

bool SpaceMine::isCalcShadowAlways() const {
    return mShadowType == 1 || (isShadowValid(mShadowType) && MR::isExistRail(this));
}

void SpaceMine::kill() {
    MR::emitEffect(this, "Explosion");
    MR::startActionSound(this, "OjSpacemineExplosion");
    MR::tryRumblePad(this, "中", 0);
    MapObjActor::kill();
}

void SpaceMine::initAfterPlacement() {
    if (MR::isExistRail(this)) {
        MR::initAndSetRailClipping(&mClippingRange, this, ::sSensorRadius, 500.0f);
        return;
    }

    bool isShadow = isShadowValid(mShadowType);

    if (isShadow) {
        MR::setClippingRangeIncludeShadow(this, &mClippingRange, ::sSensorRadius);
    }
}

bool SpaceMine::appearItem() {
    if (mItemType == 0) {
        MR::appearStarPiece(this, mPosition, 3, 10.0f, 40.0f, false);
        return true;
    }

    if (mItemType == 1) {
        MR::appearCoinPop(this, mPosition, 1);
        return true;
    }

    if (mItemType == 2) {
        MR::appearKinokoOneUpPop(mBenefitItem, mKinokoMtx.toMtxPtr());
        return true;
    }

    return false;
}

void SpaceMine::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::sendMsgEnemyAttackExplosion(pReceiver, pSender)) {
        kill();
        if (!MR::isSensorPlayer(pReceiver)) {
            appearItem();
        }
    } else {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool SpaceMine::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgInvincibleAttack(msg)) {
        appearItem();
        kill();
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        appearItem();
        kill();
        return true;
    }

    if (MR::isMsgSupportTicoSpin(msg) && !MR::isDead(this)) {
        appearItem();
        kill();
        return true;
    }

    if (MR::isMsgYoshiProjectileOrEat(msg) && !MR::isDead(this)) {
        appearItem();
        kill();
        return true;
    }

    return false;
}

bool SpaceMine::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg) && !MR::isDead(this)) {
        appearItem();
        kill();
        return true;
    }

    return false;
}

// NONMATCHING: the normalize test compiles to cror/bne/b in the target (ours: beq), and the target frame is 0x10 bytes larger
// with the tongue-hit temporaries in different slots; the register assignment of pSender/this is swapped as a result.
bool SpaceMine::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (isNerve(GET_NERVE(SpaceMine, HostTypeAppear))) {
            return false;
        }

        HitSensor* pBody = getSensor("Body");
        const TVec3f* pBodyPos = MR::getSensorPos(pBody);
        TVec3f dir(*MR::getSensorPos(pSender));
        dir -= *pBodyPos;
        f32 radius = MR::getSensorRadius(pBody);

        f32 lenSq = dir.squared();
        if (lenSq <= JGeometry::TUtil< f32 >::epsilon()) {
        } else {
            f32 k = JGeometry::TUtil< f32 >::inv_sqrt(lenSq) * radius;
            dir.x *= k;
            dir.y *= k;
            dir.z *= k;
        }

        TVec3f hitPos(*MR::getSensorPos(pBody));
        hitPos += dir;
        MR::emitEffectHit(this, hitPos, nullptr);

        if (isNerve(GET_NERVE(SpaceMine, HostTypeWait))) {
            setNerve(GET_NERVE(SpaceMine, HostTypeReaction));
        }
    }

    return false;
}

void SpaceMine::exeWait() {
    if (isCalcShadowAlways()) {
        MR::setClippingRangeIncludeShadow(this, &mClippingRange, ::sSensorRadius);
    }

    f32 playerDist = MR::calcDistanceToPlayer(this);
    if (mIsPlayerNear) {
        if (::sDistanceFar < playerDist) {
            mIsPlayerNear = 0;
            mRotateSpeed = ::sRotateSpeedNormal;
        }
    } else if (playerDist < ::sDistanceNear) {
        mIsPlayerNear = 1;
        mRotateSpeed = ::sRotateSpeedFast;
    }

    if (MR::isBinded(this)) {
        if (MR::isBindedGround(this)) {
            MR::sendMsgEnemyAttackExplosionToBindedSensor(this, MR::getGroundSensor(this));
        }

        if (MR::isBindedWall(this)) {
            MR::sendMsgEnemyAttackExplosionToBindedSensor(this, MR::getWallSensor(this));
        }

        if (MR::isBindedRoof(this)) {
            MR::sendMsgEnemyAttackExplosionToBindedSensor(this, MR::getRoofSensor(this));
        }

        MR::offBind(this);
        kill();
    } else {
        mRotation.y += mRotateSpeed;
        mRotation.y = MR::repeatDegree(mRotation.y);
        YoshiFunction::tryYoshiLick(mLockOnTarget);
        if (mStarPointerState->tryStartPointBind()) {
            setNerve(GET_NERVE(SpaceMine, HostTypePause));
        }
    }
}

void SpaceMine::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "Appear");
        MR::setClippingRangeIncludeShadow(this, &mClippingRange, ::sSensorRadius);
    }

    if (MR::isStep(this, ::sAppearEffectTime)) {
        MR::startAction(this, "Appear");
        MR::showModel(this);
    }

    if (MR::isGreaterStep(this, ::sAppearEffectTime)) {
        if (MR::isActionEnd(this)) {
            setNerve(GET_NERVE(SpaceMine, HostTypeWait));
        }
    }
}

void SpaceMine::exeReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Reaction");
        MR::startActionSoundPlayer("TongueHitWall", -1, -1);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(SpaceMine, HostTypeWait));
    }
}

void SpaceMine::exePause() {
    if (MR::isFirstStep(this)) {
        pauseMapPartsFunctions();
        getSensor("Body")->setType(0x18);
    }

    YoshiFunction::tryYoshiLick(mLockOnTarget);
    MR::updateActorStateAndNextNerve(this, GET_NERVE(SpaceMine, HostTypeWait));
}

SpaceMine::~SpaceMine() {
}
