#include "Game/Enemy/Terebo.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Player/YoshiFunction.hpp"

namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, MtxPtr, const TVec3f&);
};  // namespace MR


// Unnamed SMG2 helpers (not in this unit).

namespace NrvTerebo {
    NEW_NERVE(TereboNrvAppear, Terebo, Appear);
    NEW_NERVE(TereboNrvWait, Terebo, Wait);
    NEW_NERVE(TereboNrvHide, Terebo, Hide);
    NEW_NERVE(TereboNrvHitReaction, Terebo, HitReaction);
    NEW_NERVE(TereboNrvSpinReaction, Terebo, SpinReaction);
    NEW_NERVE(TereboNrvTongue, Terebo, Tongue);
    NEW_NERVE(TereboNrvDeath, Terebo, Death);

    // Nerve with an empty execute (never entered in SMG2 code, only checked in control).
    class TereboNrvNone : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static TereboNrvNone sInstance;
    };
    TereboNrvNone TereboNrvNone::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(TereboNrvSupportFreeze, Terebo, SupportFreeze);
};  // namespace NrvTerebo

Terebo::Terebo(const char* pName) : LiveActor(pName), mAnimScaleController(nullptr), mBindStarPointer(nullptr), mFrontVec(0.0f, 0.0f, 1.0f), mRailSpeed(5.0f), mIsGroupMoveStop(false) {
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, static_cast< MtxPtr >(nullptr), TVec3f(0.0f));
}

void Terebo::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Terebo", false);
    if (MR::isExistRail(this)) {
        MR::moveCoordAndTransToNearestRailPos(this);
    }
    MR::calcFrontVec(&mFrontVec, this);
    f32 railSpeed = -1.0f;
    MR::getJMapInfoArg0NoInit(rIter, &railSpeed);
    if (railSpeed >= 0.0f) {
        mRailSpeed = railSpeed;
    }
    initNerve(GET_NERVE(Terebo, TereboNrvWait), 1);
    mAnimScaleController = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mAnimScaleController);
    MR::initActorState(this, mBindStarPointer, GET_NERVE(Terebo, TereboNrvSupportFreeze), "SupportFreeze");
    MR::joinToGroupArray(this, rIter, getName(), 0x20);
    MR::useStageSwitchSyncAppear(this, rIter);
    s32 gravityOff = 0;
    MR::getJMapInfoArg0NoInit(rIter, &gravityOff);
    if (gravityOff != 0) {
        MR::onCalcGravity(this);
        return;
    }
    MR::offCalcGravity(this);
    TVec3f dir(0.0f, 1.0f, 0.0f);
    f32 mtx[3][4];
    MR::makeMtxTR(mtx, this);
    MR::extractMtxYDir(mtx, &dir);
    mGravity = -dir;
}

void Terebo::makeActorAppeared() {
    if (MR::isValidSwitchAppear(this)) {
        setNerve(GET_NERVE(Terebo, TereboNrvAppear));
    } else {
        setNerve(GET_NERVE(Terebo, TereboNrvWait));
    }
    s32 currentNo = -1;
    s32 nextNo = -1;
    MR::getRailPointArg0WithInit(this, MR::getCurrentRailPointNo(this), &currentNo);
    MR::getRailPointArg0WithInit(this, MR::getNextRailPointNo(this), &nextNo);
    if (currentNo == 0 && nextNo == 0) {
        setNerve(GET_NERVE(Terebo, TereboNrvHide));
        MR::deleteEffectAll(this);
    }
    LiveActor::makeActorAppeared();
}

void Terebo::control() {
    mAnimScaleController->updateNerve();
    if (!isNerve(GET_NERVE(Terebo, TereboNrvNone)) && !isNerve(GET_NERVE(Terebo, TereboNrvHide)) && !isNerve(GET_NERVE(Terebo, TereboNrvSpinReaction)) && !isNerve(GET_NERVE(Terebo, TereboNrvTongue))) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn);
    }
    if (MR::isPlayerElementModeTeresa()) {
        MR::invalidateHitSensor(this, "Spin");
    } else {
        MR::validateHitSensor(this, "Spin");
    }
    if (isBindStarPointerEnable() && mBindStarPointer->tryStartPointBind()) {
        setNerve(GET_NERVE(Terebo, TereboNrvSupportFreeze));
    }
}

void Terebo::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
        MR::validateClipping(this);
        MR::showModel(this);
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Terebo, TereboNrvWait));
    }
    updateOnRail();
}

void Terebo::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        MR::validateClipping(this);
        MR::showModel(this);
    }
    updateOnRail();
}

void Terebo::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
        MR::validateClipping(this);
        MR::hideModel(this);
    }
    updateOnRail();
}

void Terebo::exeHitReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HitReaction");
        MR::validateClipping(this);
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Terebo, TereboNrvWait));
    }
    updateOnRail();
}

// NONMATCHING: last fmuls ((1-t*t)*90) has its operands swapped; CW normalizes commutative operands.
void Terebo::exeSpinReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SpinReaction");
        MR::validateClipping(this);
    }
    f32 rate = MR::calcNerveRate(this, (s32)MR::getBckFrameMax(this));
    f32 t = 4.0f * rate * (rate - 1.0f);
    f32 k = 1.0f - t * t;
    MR::setShadowVolumeSphereRadius(this, nullptr, k * 90.0f);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Terebo, TereboNrvWait));
    }
    updateOnRail();
}

// NONMATCHING: last fmuls ((1-t*t)*90) has its operands swapped; CW normalizes commutative operands.
void Terebo::exeTongue() {
    if (MR::isFirstStep(this)) {
        MR::startBrk(this, "SpinReaction");
        MR::validateClipping(this);
        MR::tryHitReaction(this, "\x83\x88\x83\x62\x83\x56\x81\x5B\x90\xE3\x94\xBD\x89\x9E");
    }
    f32 rate = MR::calcNerveRate(this, (s32)MR::getBrkFrameMax(this));
    f32 t = 4.0f * rate * (rate - 1.0f);
    f32 k = 1.0f - t * t;
    MR::setShadowVolumeSphereRadius(this, nullptr, k * 90.0f);
    if (MR::isGreaterStep(this, MR::getBckFrameMax(this, "SpinReaction"))) {
        setNerve(GET_NERVE(Terebo, TereboNrvWait));
    }
    updateOnRail();
}

void Terebo::exeSupportFreeze() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::sendMsgGroupMoveStopToMember(this, getSensor("Body"), "Body");
    }
    if (MR::updateActorStateAndNextNerve(this, GET_NERVE(Terebo, TereboNrvWait))) {
        MR::sendMsgGroupMoveStartToMember(this, getSensor("Body"), "Body");
    }
}

void Terebo::exeDeath() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HitReaction");
        MR::validateClipping(this);
    }
    if (MR::isActionEnd(this)) {
        kill();
    }
}

void Terebo::calcAndSetBaseMtx() {
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFrontVec);
    MR::calcFrontVec(&mFrontVec, this);
    MR::updateBaseScale(this, mAnimScaleController);
}

void Terebo::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "Spin")) {
        return;
    }
    if (MR::isSensorPlayer(pReceiver)) {
        if (isHitEnable()) {
            if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                tryHitReaction();
            }
        } else if (!isNerve(GET_NERVE(Terebo, TereboNrvHide))) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    } else if (MR::isSensorYoshi(pReceiver) && !isNerve(GET_NERVE(Terebo, TereboNrvHide))) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool Terebo::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Spin")) {
        if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
            return trySpinReaction();
        }
        return false;
    }
    if (MR::isMsgStarPieceAttack(msg)) {
        tryStarPieceAttack();
        return false;
    }
    if (MR::isMsgSearchlightAttack(msg) || MR::isMsgFireBallAttack(msg) || MR::isMsgInvincibleAttack(msg)) {
        return tryDeath();
    }
    return false;
}

bool Terebo::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isDead(this)) {
        return false;
    }
    if (MR::isMsgGroupMoveStop(msg)) {
        if (!isNerve(GET_NERVE(Terebo, TereboNrvSupportFreeze))) {
            mIsGroupMoveStop = true;
        }
        return true;
    }
    if (MR::isMsgGroupMoveStart(msg)) {
        mIsGroupMoveStop = false;
        return true;
    }
    if (MR::isMsgYoshiStartTongue(msg)) {
        tryTongue();
        return false;
    }
    if (msg == 0x4D /* ACTMES 77 */) {
        tryHitReaction();
        return false;
    }
    return false;
}

bool Terebo::isHitEnable() const {
    bool ret;
    bool notHidden = !MR::isHiddenModel(this);
    if (notHidden) {
        ret = false;
        if (isNerve(GET_NERVE(Terebo, TereboNrvAppear)) || isNerve(GET_NERVE(Terebo, TereboNrvWait))) {
            ret = true;
        }
    } else {
        ret = false;
    }
    return ret;
}

bool Terebo::isSpinReactionEnable() const {
    bool ret;
    bool notHidden = !MR::isHiddenModel(this);
    if (notHidden) {
        ret = false;
        if (isNerve(GET_NERVE(Terebo, TereboNrvAppear)) || isNerve(GET_NERVE(Terebo, TereboNrvWait)) || isNerve(GET_NERVE(Terebo, TereboNrvHitReaction))) {
            ret = true;
        }
    } else {
        ret = false;
    }
    return ret;
}

bool Terebo::isDeathEnable() const {
    bool ret;
    bool notHidden = !MR::isHiddenModel(this);
    if (notHidden) {
        ret = false;
        if (isNerve(GET_NERVE(Terebo, TereboNrvAppear)) || isNerve(GET_NERVE(Terebo, TereboNrvWait)) || isNerve(GET_NERVE(Terebo, TereboNrvHitReaction))) {
            ret = true;
        }
    } else {
        ret = false;
    }
    return ret;
}

bool Terebo::isBindStarPointerEnable() const {
    bool notHidden = !MR::isHiddenModel(this);
    if (notHidden) {
        if (isNerve(GET_NERVE(Terebo, TereboNrvAppear)) || isNerve(GET_NERVE(Terebo, TereboNrvWait)) || isNerve(GET_NERVE(Terebo, TereboNrvHitReaction))) {
            return true;
        }
        return false;
    }
    return false;
}

bool Terebo::tryDeath() {
    if (isDeathEnable()) {
        setNerve(GET_NERVE(Terebo, TereboNrvDeath));
        return true;
    }
    return false;
}

bool Terebo::tryHitReaction() {
    if (isHitEnable() && !isNerve(GET_NERVE(Terebo, TereboNrvSupportFreeze))) {
        setNerve(GET_NERVE(Terebo, TereboNrvHitReaction));
        return true;
    }
    return false;
}

bool Terebo::trySpinReaction() {
    if (isSpinReactionEnable() && !isNerve(GET_NERVE(Terebo, TereboNrvSpinReaction)) && !isNerve(GET_NERVE(Terebo, TereboNrvSupportFreeze))) {
        setNerve(GET_NERVE(Terebo, TereboNrvSpinReaction));
        return true;
    }
    return false;
}

bool Terebo::tryStarPieceAttack() {
    if (isSpinReactionEnable()) {
        setNerve(GET_NERVE(Terebo, TereboNrvSpinReaction));
        return true;
    }
    if (isNerve(GET_NERVE(Terebo, TereboNrvSupportFreeze))) {
        MR::sendMsgGroupMoveStartToMember(this, getSensor("Body"), "Body");
        setNerve(GET_NERVE(Terebo, TereboNrvSpinReaction));
        return true;
    }
    return false;
}

bool Terebo::tryTongue() {
    if (isHitEnable() && !isNerve(GET_NERVE(Terebo, TereboNrvTongue)) && !isNerve(GET_NERVE(Terebo, TereboNrvSupportFreeze))) {
        setNerve(GET_NERVE(Terebo, TereboNrvTongue));
        return true;
    }
    return false;
}

void Terebo::updateOnRail() {
    if (mIsGroupMoveStop) {
        return;
    }
    if (!moveOnRailAndCheckNextPoint()) {
        return;
    }
    s32 currentNo = -1;
    s32 nextNo = -1;
    MR::getRailPointArg0WithInit(this, MR::getCurrentRailPointNo(this), &currentNo);
    MR::getRailPointArg0WithInit(this, MR::getNextRailPointNo(this), &nextNo);
    if (currentNo != 0 || nextNo != 0) {
        if (isNerve(GET_NERVE(Terebo, TereboNrvHide))) {
            setNerve(GET_NERVE(Terebo, TereboNrvWait));
        }
    } else if (!isNerve(GET_NERVE(Terebo, TereboNrvHide))) {
        setNerve(GET_NERVE(Terebo, TereboNrvHide));
        calcAndSetBaseMtx();
    }
}

// NONMATCHING: one argument-setup instruction (addi r3/r4 order) for the projection copy is swapped.
bool Terebo::moveOnRailAndCheckNextPoint() {
    f32 speed = mRailSpeed;
    s32 prevNo = MR::getNextRailPointNo(this);
    MR::moveCoordAndFollowTransEaseInOutAndReverse(this, speed, 0.2f * speed, 100.0f);
    s32 nextNo = MR::getNextRailPointNo(this);
    TVec3f dir(MR::getRailDirection(this));
    if (!MR::isSameDirection(dir, mGravity, 0.01f)) {
        f32 d = dir.dot(mGravity);
        TVec3f proj(mGravity);
        proj.scale(d);
        dir -= proj;
        MR::normalizeOrZero(&dir);
        MR::turnVecToVecDegree(&mFrontVec, mFrontVec, dir, 3.5f, mGravity);
    }
    return prevNo != nextNo;
}

TereboGroup::TereboGroup(const char* pName) : LiveActor(pName) {}

// NONMATCHING: target keeps `x - 0.0f` and `0.0f + x` as fsubs/fadds with a loaded 0.0 (CW folds them away here).
namespace {
    extern const f32 sTereboGroupZero;
}

void TereboGroup::init(const JMapInfoIter& rIter) {
    initRailRider(rIter);
    MR::setGroupClipping(this, rIter, 0x10);
    s32 clippingGroupID = -1;
    MR::getJMapInfoClippingGroupID(rIter, &clippingGroupID);
    s32 count = 0;
    MR::getJMapInfoArg6NoInit(rIter, &count);
    f32 spacing = 0.0f;
    MR::getJMapInfoArg7NoInit(rIter, &spacing);
    MR::moveCoordAndTransToNearestRailPos(this);
    const f32 zero = sTereboGroupZero;
    for (s32 i = 0; i < count; i++) {
        Terebo* pTerebo = new Terebo("\x83\x65\x83\x8C\x83\x7B\x81\x5B\x81\x69\x83\x4F\x83\x8B\x81\x5B\x83\x76\x8E\x77\x92\xE8\x81\x6A");
        TVec3f pos(0.0f);
        f32 offset;
        if (spacing != zero) {
            offset = spacing * (f32)i;
        } else {
            offset = ((f32)i / (f32)count) * MR::getRailTotalLength(this);
        }
        f32 total = MR::getRailTotalLength(this);
        f32 coord = offset + MR::getRailCoord(this);
        coord -= zero;
        f32 railCoord = zero + (f32)fmod(total + coord, total);
        MR::calcRailPosAtCoord(&pos, this, railCoord);
        pTerebo->init(rIter);
        MR::setPosition(pTerebo, pos);
        MR::setRailCoord(pTerebo, railCoord);
        MR::useStageSwitchSyncAppear(pTerebo, rIter);
    }
    makeActorDead();
}

Terebo::~Terebo() {}

TereboGroup::~TereboGroup() {}

namespace {
    const f32 sTereboGroupZero = 0.0f;
}
