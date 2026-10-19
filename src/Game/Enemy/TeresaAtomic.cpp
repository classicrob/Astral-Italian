#include "Game/Enemy/TeresaAtomic.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/MapObj/ClipAreaDropHolder.hpp"
#include "Game/Map/SunshadeMapHolder.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"

namespace NrvTeresaAtomic {
    NEW_NERVE(TeresaAtomicNrvAppear, TeresaAtomic, Appear);
    NEW_NERVE(TeresaAtomicNrvWait, TeresaAtomic, Wait);
    NEW_NERVE(TeresaAtomicNrvChase, TeresaAtomic, Chase);
    NEW_NERVE(TeresaAtomicNrvShay, TeresaAtomic, Shay);
    NEW_NERVE(TeresaAtomicNrvAggressive, TeresaAtomic, Aggressive);
    NEW_NERVE(TeresaAtomicNrvLaugh, TeresaAtomic, Laugh);
    NEW_NERVE(TeresaAtomicNrvDead, TeresaAtomic, Dead);
    NEW_NERVE(TeresaAtomicNrvDisappear, TeresaAtomic, Disappear);

    class TeresaAtomicNrvDeadPowerStarGet : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static TeresaAtomicNrvDeadPowerStarGet sInstance;
    };
    TeresaAtomicNrvDeadPowerStarGet TeresaAtomicNrvDeadPowerStarGet::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvTeresaAtomic

#define NRV(x) GET_NERVE(TeresaAtomic, TeresaAtomicNrv##x)

TeresaAtomic::TeresaAtomic(const char* pName) : LiveActor(pName) {
    mClipAreaDrop = nullptr;
    mFrontVec.x = 0.0f;
    mFrontVec.y = 0.0f;
    mFrontVec.z = 1.0f;
    mSpeed = 10.0f;
    mPowerStarIndex = -1;
    mChildParts = nullptr;
    mPartsHost = nullptr;
    mChildCount = 0;
}

namespace {
    const char* sFollowerNames[] = {"Follower1", "Follower2", "Follower3", "Follower4"};
}

void TeresaAtomic::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    if (MR::isConnectedWithRail(rIter)) {
        MR::moveCoordAndTransToStartPos(this);
        MR::fn_800531B0(&mFrontVec, this, 100.0f);
    } else {
        MR::calcFrontVec(&mFrontVec, this);
    }
    MR::getJMapInfoArg0NoInit(rIter, &mSpeed);
    f32 radius = -1.0f;
    MR::getJMapInfoArg1WithInit(rIter, &radius);
    if (radius >= 1.0f) {
        MR::createClipAreaDropHolder();
        mClipAreaDrop = (LiveActor*)(u32)MR::appearClipAreaDrop(mPosition, radius);
    }
    mPowerStarIndex = -1;
    MR::getJMapInfoArg2WithInit(rIter, &mPowerStarIndex);
    if (mPowerStarIndex >= 0) {
        MR::declarePowerStar(this);
    }
    MR::calcGravity(this);
    TVec3f offset = mGravity * -900.0f;
    TVec3f pos = mPosition + offset;
    MR::resetPosition(this, pos);
    mChildCount = 4;
    mChildParts = new PartsModel*[mChildCount];
    mPartsHost = MR::createPartsModelMapObj2(this, "取り巻きテレサ位置", "TeresaAtomicPosition", getBaseMtx());
    MR::startAction(mPartsHost, "Wait");
    const char* const* followerNames = sFollowerNames;
    for (s32 i = 0; i < mChildCount; i++) {
        mChildParts[i] = new PartsModel(mPartsHost, "取り巻きテレサ", nullptr, nullptr, -1, false);
        MR::initActor(mChildParts[i], "Teresa", "TeresaFollower", false);
        mChildParts[i]->initWithoutIter();
        mChildParts[i]->loadFixedPosition(followerNames[i], this);
        MR::setSensorFollowPos(this, followerNames[i], &mChildParts[i]->mPosition);
    }
    initNerve(NRV(Appear), 0);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void TeresaAtomic::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    for (s32 i = 0; i < mChildCount; i++) {
        mChildParts[i]->makeActorAppeared();
    }
}

void TeresaAtomic::makeActorDead() {
    LiveActor::makeActorDead();
    for (s32 i = 0; i < mChildCount; i++) {
        mChildParts[i]->makeActorDead();
    }
}

void TeresaAtomic::appear() {
    LiveActor::appear();
    setNerve(NRV(Appear));
}

void TeresaAtomic::kill() {
    LiveActor::kill();
    if (mPowerStarIndex >= 0) {
        MR::requestAppearPowerStar(this);
    }
}

void TeresaAtomic::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        if (isEnableAttack() && MR::sendMsgEnemyAttackStrongToDir(pReceiver, pSender, mFrontVec)) {
            setNerve(NRV(Laugh));
            return;
        }
        if (isEnablePush()) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool TeresaAtomic::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return isEnableStarPieceAttack();
    }
    if (MR::isMsgStarPieceReflect(msg) && isEnableStarPieceAttack()) {
        return true;
    }
    return false;
}

void TeresaAtomic::control() {
    if (mClipAreaDrop != nullptr) {
        MR::setPosition(mClipAreaDrop, mPosition);
    }
    if (MR::isPowerStarGetDemoActive() && !isNerve(NRV(DeadPowerStarGet))) {
        setNerve(NRV(DeadPowerStarGet));
        MR::zeroVelocity(this);
    }
}

void TeresaAtomic::calcAndSetBaseMtx() {
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFrontVec);
}

bool TeresaAtomic::isPlayerFacingTeresa() {
    TVec3f front;
    MR::getPlayerFrontVec(&front);
    TVec3f* pPlayerPos = MR::getPlayerPos();
    TVec3f diff = this->mPosition - *pPlayerPos;
    return MR::diffAngleAbs(front, diff) < 0.87266463f;
}

bool TeresaAtomic::isEnableAttack() const {
    if (isNerve(NRV(Appear)) || isNerve(NRV(Dead)) ||
        isNerve(NRV(Disappear))) {
        return false;
    }
    return true;
}

bool TeresaAtomic::isEnablePush() const {
    if (isNerve(NRV(Appear)) || isNerve(NRV(Dead)) ||
        isNerve(NRV(Disappear))) {
        return false;
    }
    return true;
}

bool TeresaAtomic::isEnableStarPieceAttack() const {
    if (isNerve(NRV(Wait)) || isNerve(NRV(Chase)) ||
        isNerve(NRV(Shay)) || isNerve(NRV(Aggressive)) ||
        isNerve(NRV(Laugh))) {
        return true;
    }
    return false;
}

bool TeresaAtomic::tryBecomeShy() {
    bool isShay = false;
    if (!MR::isPlayerSquat()) {
        if (this->isPlayerFacingTeresa() && MR::isNearPlayer(this, 3500.0f)) {
            isShay = true;
        }
    }
    if (isShay) {
        this->setNerve(NRV(Shay));
        return true;
    }
    return false;
}

bool TeresaAtomic::tryDisappear() {
    if (!MR::isExistRail(this)) {
        return false;
    }
    f32 railCoord = MR::calcNearestRailCoord(this, *MR::getPlayerPos());
    if (MR::getRailCoord(this) > railCoord) {
        setNerve(NRV(Disappear));
        return true;
    }
    return false;
}

bool TeresaAtomic::tryDead() {
    if (MR::isInAreaObj("GlaringLightArea", mPosition) || !MR::isInShadeFromTheSun(mPosition, 10000.0f)) {
        setNerve(NRV(Dead));
        return true;
    }
    return false;
}

void TeresaAtomic::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::showModelIfHidden(this);
        MR::startAction(this, "Appear");
        for (s32 i = 0; i < mChildCount; i++) {
            MR::showModelIfHidden(mChildParts[i]);
            MR::startAction(mChildParts[i], "Appear");
        }
    }
    if (MR::isActionEnd(this)) {
        setNerve(NRV(Chase));
    }
}

void TeresaAtomic::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        for (s32 i = 0; i < mChildCount; i++) {
            MR::startAction(mChildParts[i], "Wait");
        }
    }
    if (!MR::isPlayerDamaging() && MR::isGreaterStep(this, 60)) {
        setNerve(NRV(Chase));
        return;
    }
    if (!this->tryBecomeShy() && !tryDead()) {
        tryDisappear();
    }
}

void TeresaAtomic::exeChase() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Chase");
        for (s32 i = 0; i < mChildCount; i++) {
            MR::startAction(mChildParts[i], "Chase");
        }
        MR::setBckRate(mPartsHost, 1.0f);
    }
    if (MR::isExistRail(this)) {
        TVec3f front;
        MR::fn_800531B0(&front, this, 100.0f);
        MR::turnDirectionDegree(this, &mFrontVec, front, 4.0f);
        MR::moveCoordAndFollowTrans(this, mSpeed);
        TVec3f offset = mGravity * -900.0f;
        TVec3f pos = mPosition + offset;
        MR::setPosition(this, pos);
    } else {
        TVec3f vel = mFrontVec * mSpeed;
        MR::setVelocity(this, vel);
    }
    if (!this->tryBecomeShy() && !tryDead()) {
        tryDisappear();
    }
}

void TeresaAtomic::exeShay() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shay");
        for (s32 i = 0; i < mChildCount; i++) {
            MR::startAction(mChildParts[i], "Shay");
        }
        MR::zeroVelocity(this);
        MR::stopBck(mPartsHost);
    }
    bool isShay = false;
    if (!MR::isPlayerSquat()) {
        if (this->isPlayerFacingTeresa() && MR::isNearPlayer(this, 3500.0f)) {
            isShay = true;
        }
    }
    if (!isShay) {
        setNerve(NRV(Aggressive));
        return;
    }
    if (!tryDead()) {
        tryDisappear();
    }
}

void TeresaAtomic::exeAggressive() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Aggressive");
        for (s32 i = 0; i < mChildCount; i++) {
            MR::startAction(mChildParts[i], "Aggressive");
        }
        MR::zeroVelocity(this);
    }
    if (MR::isActionEnd(this)) {
        setNerve(NRV(Chase));
        return;
    }
    if (!this->tryBecomeShy()) {
        tryDead();
    }
}

void TeresaAtomic::exeLaugh() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Laugh");
        for (s32 i = 0; i < mChildCount; i++) {
            MR::startAction(mChildParts[i], "Laugh");
        }
        MR::zeroVelocity(this);
    }
    if (MR::isActionEnd(this)) {
        setNerve(NRV(Wait));
        return;
    }
    tryDead();
}

void TeresaAtomic::exeDead() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Dead");
        kill();
    }
    if (MR::isActionEnd(this)) {
        kill();
    }
}

void TeresaAtomic::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Disappear");
        for (s32 i = 0; i < mChildCount; i++) {
            MR::startAction(mChildParts[i], "Disappear");
        }
    }
    if (MR::isActionEnd(this)) {
        if (!MR::isHiddenModel(this)) {
            MR::tryHitReaction(this, "\x8f\xc1\x96\xc5");
            MR::hideModel(this);
        }
    }
    for (s32 i = 0; i < mChildCount; i++) {
        if (MR::isActionEnd(mChildParts[i])) {
            MR::hideModelIfShown(mChildParts[i]);
        }
    }
    if (MR::isExistRail(this)) {
        f32 railCoord = MR::calcNearestRailCoord(this, *MR::getPlayerPos());
        if (MR::getRailCoord(this) < railCoord - 1000.0f) {
            setNerve(NRV(Appear));
        }
    }
}


void NrvTeresaAtomic::TeresaAtomicNrvDeadPowerStarGet::execute(Spine* pSpine) const {
    TeresaAtomic* actor = static_cast<TeresaAtomic*>(pSpine->mExecutor);
    if (MR::isFirstStep(actor)) {
        MR::tryStartActionNotPlaying(actor, "Wait");
        actor->kill();
    }
}
