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

// Shared string blob of the unit (names are read from its offsets, as the SMG2 code does).
extern "C" char lbl_80692B88[];

// Unnamed SMG2 helpers, defined elsewhere in the unit or in other TUs.
extern "C" void fn_800531B0(TVec3f*, LiveActor*, f32);

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
        virtual void execute(Spine* pSpine) const {
            TeresaAtomic* actor = reinterpret_cast< TeresaAtomic* >(pSpine->mExecutor);
            if (MR::isFirstStep(actor)) {
                MR::tryStartActionNotPlaying(actor, "Wait");
            }
            actor->kill();
        }
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

void TeresaAtomic::init(const JMapInfoIter& rIter) {
    char* pBase = lbl_80692B88;
    MR::initActor(this, rIter, false);
    if (MR::isConnectedWithRail(rIter)) {
        MR::moveCoordAndTransToStartPos(this);
        fn_800531B0(&mFrontVec, this, 100.0f);
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
    mPartsHost = MR::createPartsModelMapObj2(this, pBase + 0x40, pBase + 0x54, getBaseMtx());
    MR::startAction(mPartsHost, pBase + 0x6C);
    const char* const* followerNames = reinterpret_cast<const char* const*>(pBase + 0x30);
    for (s32 i = 0; i < mChildCount; i++) {
        mChildParts[i] = new PartsModel(mPartsHost, pBase + 0x74, nullptr, nullptr, -1, false);
        MR::initActor(mChildParts[i], pBase + 0x84, pBase + 0x8C, false);
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

extern "C" bool fn_8020F940(TeresaAtomic* pActor) {
    TVec3f front;
    MR::getPlayerFrontVec(&front);
    TVec3f* pPlayerPos = MR::getPlayerPos();
    TVec3f diff = pActor->mPosition - *pPlayerPos;
    return MR::diffAngleAbs(front, diff) < 0.8726646f;
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

extern "C" bool fn_8020FB50(TeresaAtomic* pActor) {
    bool isShay = false;
    if (!MR::isPlayerSquat()) {
        if (fn_8020F940(pActor) && MR::isNearPlayer(pActor, 3500.0f)) {
            isShay = true;
        }
    }
    if (isShay) {
        pActor->setNerve(NRV(Shay));
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
    if (!fn_8020FB50(this) && !tryDead()) {
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
        fn_800531B0(&front, this, 100.0f);
        MR::turnDirectionDegree(this, &mFrontVec, front, 4.0f);
        MR::moveCoordAndFollowTrans(this, mSpeed);
        TVec3f offset = mGravity * -900.0f;
        TVec3f pos = mPosition + offset;
        MR::setPosition(this, pos);
    } else {
        TVec3f vel = mFrontVec * mSpeed;
        MR::setVelocity(this, vel);
    }
    if (!fn_8020FB50(this) && !tryDead()) {
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
        if (fn_8020F940(this) && MR::isNearPlayer(this, 3500.0f)) {
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
    if (!fn_8020FB50(this)) {
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

