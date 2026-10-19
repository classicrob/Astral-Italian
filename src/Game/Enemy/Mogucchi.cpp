#include "Game/Enemy/Mogucchi.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/MogucchiHill.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "math_types.hpp"


namespace NrvMogucchi {
    NEW_NERVE_ONEND(MogucchiNrvStroll, Mogucchi, Stroll, Stroll);
    NEW_NERVE(MogucchiNrvAppearDown, Mogucchi, AppearDown);
    class MogucchiNrvDown : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            reinterpret_cast<Mogucchi*>(spine->mExecutor)->exeDown();
        }
        virtual void executeOnEnd(Spine* spine) const {
            MR::deleteEffect(reinterpret_cast<LiveActor*>(spine->mExecutor), "PointerTouchManual");
        }
        static MogucchiNrvDown sInstance;
    };
    MogucchiNrvDown MogucchiNrvDown::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(MogucchiNrvDive, Mogucchi, Dive);
    NEW_NERVE(MogucchiNrvScatter, Mogucchi, Scatter);
    NEW_NERVE(MogucchiNrvDie, Mogucchi, Die);
};  // namespace NrvMogucchi

namespace {
    inline void resetBodySensorOffset(LiveActor* pActor) {
        MR::setSensorOffset(pActor, "Body", TVec3f(0.0f, 0.0f, 0.0f));
    }
};  // namespace

namespace {
    const Vec sHeadOffset = {40.0f, 0.0f, -8.0f};
    const Vec sBodyOffset = {0.0f, 0.0f, 0.0f};
};  // namespace

Mogucchi::Mogucchi(const char* pName)
    : LiveActor(pName), mHill(), mHole(), mGrounded(), mScatterPropulsionSpeed(), mScatterNormal(0.0f, 0.0f, 1.0f), mUnusedE8(), mStrollSpeed(),
      mMaxStrollSpeed(5.0f), mSpinCounterCooldown(), mIsStoppedByP2() {
    mNewHolePos.identity();
}

Mogucchi::~Mogucchi() {
}

void Mogucchi::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::getJMapInfoArg0NoInit(rIter, &mMaxStrollSpeed);
    MR::initActor(this, rIter, "Mogucchi", false);
    MR::connectToSceneEnemy(this);
    initSensor();
    initEffectKeeper(1, nullptr, false);
    MR::addEffect(this, "PointerTouchManual");
    MR::setEffectHostMtx(this, "PointerTouchManual", MR::getJointMtx(this, "Nose"));
    initSound(4, "Mogucchi", nullptr, TVec3f(0.0f));
    initRailRider(rIter);
    MR::declareStarPiece(this, 3);
    initNerve(GET_NERVE(Mogucchi, MogucchiNrvStroll));
    MR::initUseStageSwitchWriteDead(this, rIter);

    MR::initStarPointerTargetAtJoint(this, "Head", 113.0f, TVec3f(::sHeadOffset));
    createMogucchiHill();
    createHole();
    MR::startAction(this, "Walk");
    MR::startBtp(this, "EyeOpen");
    calcAnim();
    MR::useStageSwitchAwake(this, rIter);
    makeActorAppeared();
}

void Mogucchi::initAfterPlacement() {
    MR::moveCoordToNearestPos(this, mPosition);
    MR::calcGravityVector(this, MR::getRailPos(this), &mRailGravity, nullptr, nullptr);
    updatePosition();
    updateReferenceMtx();
}

void Mogucchi::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    MR::showModel(this);
    setNerve(GET_NERVE(Mogucchi, MogucchiNrvStroll));
    mHole->makeActorAppeared();
    MR::showModel(mHole);
    MR::validateClipping(this);
}

void Mogucchi::makeActorDead() {
    LiveActor::makeActorDead();

    if (!MR::isDead(mHole)) {
        mHole->kill();
    }
}

void Mogucchi::kill() {
    LiveActor::kill();

    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }

    setNerve(GET_NERVE(Mogucchi, MogucchiNrvDie));

    if (!MR::isDead(mHole)) {
        mHole->kill();
    }
}

void Mogucchi::control() {
    if (isNerve(GET_NERVE(Mogucchi, MogucchiNrvScatter)) || isNerve(GET_NERVE(Mogucchi, MogucchiNrvDie))) {
        return;
    }

    updateReferenceMtx();
}

bool Mogucchi::checkHipDrop() const {
    // !MR::isPlayerHipDropLand() will not match
    if (MR::isPlayerHipDropLand() == false) {
        return false;
    }

    return !(MR::calcDistanceToPlayer(this) > 800.0f);
}

void Mogucchi::exeStroll() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Walk");
        MR::startBtp(this, "EyeOpen");
        MR::startAction(mHole, "Walk");
        mHill->start();
        mUnusedE8 = 0.0f;
        mStrollSpeed = mMaxStrollSpeed;
        mIsStoppedByP2 = false;
        mSpinCounterCooldown = 0;
    }

    MR::startActionSound(this, "EmLvMoghillMove", -1, -1, -1);
    updateStrollSpeed();
    reflectStarPointer2P();
    updateSpinCounterCooldown();
    MR::moveCoord(this, mStrollSpeed);
    MR::calcGravityVector(this, MR::getRailPos(this), &mRailGravity, nullptr, 0);
    updatePosition();
    makeEulerRotation();

    if (Mogucchi::checkHipDrop()) {
        setNerve(GET_NERVE(Mogucchi, MogucchiNrvAppearDown));
    }
}

void Mogucchi::exeAppearDown() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::startAction(this, "SwoonStart");
        MR::startBtp(this, "EyeClose");
        MR::startAction(mHole, "SwoonStart");
        MR::startActionSound(this, "EmMogucchiAppear");
        MR::startActionSound(this, "EvMogucchiSwoon");

        ::resetBodySensorOffset(this);
        MR::setSensorOffset(this, "spin", TVec3f(0.0f, 0.0f, 0.0f));
    }

    MR::startActionSound(this, "EmLvSwoonS");
    MR::setNerveAtBckStopped(this, GET_NERVE(Mogucchi, MogucchiNrvDown));
}

void Mogucchi::exeDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Swoon");
        MR::startBtp(this, "EyeClose");
        MR::startAction(mHole, "Swoon");
        mIsStoppedByP2 = false;
    }
    reflectStarPointer2P();
    MR::startActionSound(this, "EmLvSwoonS");

    if (MR::isGreaterEqualStep(this, 30) && !mIsStoppedByP2) {
        setNerve(GET_NERVE(Mogucchi, MogucchiNrvDive));
    }
}

void Mogucchi::exeDive() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
        MR::startBtp(this, "EyeOpen");
        MR::startAction(mHole, "Hide");
        MR::startActionSound(this, "EmMogucchiHide");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Mogucchi, MogucchiNrvStroll));
    }
}

void Mogucchi::exeScatter() {
    if (MR::isFirstStep(this)) {
        mHill->end();
        MR::startAction(this, "PunchDown");
        MR::startBtp(this, "EyeClose");
        MR::startAction(mHole, "PunchDown");
        MR::startBlowHitSound(this);
        mScatterPropulsionSpeed = 50.0f;
        MR::invalidateClipping(this);
    }

    mScatterNormal.orthogonalize(mRailGravity);
    MR::normalizeOrZero(&mScatterNormal);

    if (!MR::isNearZero(mScatterNormal)) {
        TRot3f mtx;
        mtx.setXDir(mRailGravity.cross(mScatterNormal));
        mtx.setYDir(-mRailGravity);
        mtx.setZDir(-mScatterNormal);
        mtx.getEulerXYZ(mRotation);
        mRotation.mult(180.0f / PI);
    }

    mPosition.add(mRailGravity.scaleInline(-mScatterPropulsionSpeed) + mScatterNormal.multInLine(23.0f));
    mScatterPropulsionSpeed -= 1.2f;

    if (MR::isGreaterEqualStep(this, 15)) {
        setNerve(GET_NERVE(Mogucchi, MogucchiNrvDie));
    }
}

void Mogucchi::exeDie() {
    if (MR::isFirstStep(this)) {
        MR::appearStarPiece(this, mPosition, 3, 10.0f, 40.0f, false);
        MR::startActionSound(this, "EmExplodeS");
        MR::emitEffect(this, "Death");
        MR::hideModel(this);
        MR::hideModel(mHole);
    }

    if (MR::isGreaterEqualStep(this, 60)) {
        return kill();
    }
}

void Mogucchi::endStroll() {
    MR::deleteEffect(this, "PointerTouchManual");
}

void Mogucchi::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver) && (MR::isEqualSensor(pSender, this, "Body") || MR::isEqualSensor(pSender, this, "head"))) {
        bool isStroll = false;
        if (isNerve(GET_NERVE(Mogucchi, MogucchiNrvStroll))) {
            if (!mIsStoppedByP2) {
                isStroll = true;
            }
        }

        if (!isStroll || !MR::isOnGroundPlayer()) {
            MR::sendMsgPush(pReceiver, pSender);
            return;
        }

        TVec3f attackDir;
        calcAttackDir(&attackDir, *MR::getSensorPos(pSender), *MR::getSensorPos(pReceiver));

        if (MR::sendMsgEnemyAttackStrongToDir(pReceiver, pSender, attackDir)) {
            MR::shakeCameraNormal();
            return;
        }

        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool Mogucchi::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "spin")) {
        return receiveAttackBySpinSensor(msg, pSender, pReceiver);
    }

    return receiveAttackByBodySensor(msg, pSender, pReceiver);
}

void Mogucchi::initSensor() {
    LiveActor::initHitSensor(3);
    MR::addHitSensorAtJointEnemy(this, "head", "Head", 32, 83.0f, TVec3f(::sHeadOffset));
    MR::addHitSensorAtJointEnemy(this, "Body", "Spine", 32, 120.0f, TVec3f(::sBodyOffset));
    MR::addHitSensorEnemy(this, "spin", 16, 180.0f, TVec3f(0.0f, 0.0f, 0.0f));
}

void Mogucchi::createMogucchiHill() {
    s32 railLength = MR::getRailTotalLength(this) / 150.0f;
    if (railLength > 20) {
        railLength = 20;
    }

    // "Mogucchi hill"
    mHill = new MogucchiHill(this, railLength, "モグッチ塚");
    mHill->initWithoutIter();
    if (railLength > 5) {
        mHill->setAppearNum(mHill->mPieceCount - 5);
    }

    MogucchiHill* hill = mHill;
    hill->mAppearDistance = 80.0f;
    hill->mReserveDistance = 100.0f;
}

void Mogucchi::updateStrollSpeed() {
    if (!(mStrollSpeed < mMaxStrollSpeed)) {
        return;
    }

    mStrollSpeed += 0.1f;
    if (mStrollSpeed > mMaxStrollSpeed) {
        mStrollSpeed = mMaxStrollSpeed;
    }
}

void Mogucchi::reflectStarPointer2P() {
    // "Weak"
    if (MR::isStarPointerPointing2PAttachTarget(this)) {
        if (!mIsStoppedByP2) {
            mIsStoppedByP2 = true;
            MR::startDPDHitSound();
            MR::emitEffectWithScale(this, "PointerTouchManual", 1.4f, -1);
        }

        mStrollSpeed = 0.0f;
        MR::startDPDFreezeLevelSound(this);
        return;
    }

    if (!mIsStoppedByP2) {
        return;
    }

    mIsStoppedByP2 = false;
    MR::deleteEffect(this, "PointerTouchManual");
}

void Mogucchi::updatePosition() {
    Triangle triangle = Triangle();

    TVec3f v1;
    mGrounded = MR::getFirstPolyOnLineToMap(&v1, &triangle, MR::getRailPos(this), mRailGravity.scaleInline(1000.0f));

    if (mGrounded) {
        MR::setPosition(this, v1);
    } else {
        MR::setPosition(this, MR::getRailPos(this));
    }
}

void Mogucchi::createHole() {
    // "Mogucchi hole"
    mHole = new ModelObj("モグッチ穴", "MogucchiHole", mNewHolePos, MR::DrawBufferType_MapObjStrongLight, -2, -2, false);
    mHole->initWithoutIter();
}

void Mogucchi::calcAttackDir(TVec3f* pDir, const TVec3f& rSenderPos, const TVec3f& rReceiverPos) const {
    pDir->sub(rReceiverPos, rSenderPos);
    pDir->orthogonalize(mRailGravity);
    MR::normalizeOrZero(pDir);

    if (MR::isNearZero(*pDir)) {
        pDir->set< f32 >(getBaseMtx()[0][1], getBaseMtx()[1][1], getBaseMtx()[2][1]);
    }

    pDir->add(-mRailGravity);
    MR::normalize(pDir);
}

void Mogucchi::makeEulerRotation() {
    TPos3f mtx;
    MR::makeMtxUpFront(&mtx, -mRailGravity, MR::getRailDirection(this));
    mtx.getEulerXYZ(mRotation);
    mRotation.mult(180.0f / PI);
}

void Mogucchi::calcScatterVec(const TVec3f& rStart, const TVec3f& rEnd) {
    mScatterNormal.sub(rEnd, rStart);
    mScatterNormal.orthogonalize(mRailGravity);
    MR::normalizeOrZero(&mScatterNormal);
}

bool Mogucchi::receiveAttackBySpinSensor(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        return false;
    }

    if (isNerve(GET_NERVE(Mogucchi, MogucchiNrvStroll))) {
        if (mSpinCounterCooldown == 0) {
            MR::startActionSound(this, "EmMogucchiRefrect", -1, -1, -1);
            MR::sendMsgEnemyAttackCounterSpin(pSender, pReceiver);
            mSpinCounterCooldown = 30;
            return true;
        }

        return false;
    }

    bool isDown =
        isNerve(GET_NERVE(Mogucchi, MogucchiNrvDown)) || isNerve(GET_NERVE(Mogucchi, MogucchiNrvAppearDown)) || isNerve(GET_NERVE(Mogucchi, MogucchiNrvDive));

    if (!isDown) {
        return false;
    }

    if (isNerve(GET_NERVE(Mogucchi, MogucchiNrvAppearDown)) && MR::isLessStep(this, 10)) {
        return false;
    }

    MR::stopScene(8);
    MR::tryRumblePadMiddle(this, WPAD_CHAN0);
    calcScatterVec(*MR::getSensorPos(pSender), *MR::getSensorPos(pReceiver));
    setNerve(GET_NERVE(Mogucchi, MogucchiNrvScatter));

    return true;
}

bool Mogucchi::receiveAttackByBodySensor(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        return false;
    }

    if (!isNerve(GET_NERVE(Mogucchi, MogucchiNrvScatter)) && !isNerve(GET_NERVE(Mogucchi, MogucchiNrvDie)) && MR::isMsgStarPieceAttack(msg)) {
        setNerve(GET_NERVE(Mogucchi, MogucchiNrvAppearDown));
        return true;
    }

    if (!isNerve(GET_NERVE(Mogucchi, MogucchiNrvScatter)) && !isNerve(GET_NERVE(Mogucchi, MogucchiNrvDie)) && MR::isMsgPlayerHitProjectiles(msg)) {
        MR::stopScene(8);
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        calcScatterVec(*MR::getSensorPos(pSender), *MR::getSensorPos(pReceiver));
        setNerve(GET_NERVE(Mogucchi, MogucchiNrvScatter));
        return true;
    }

    bool isDown =
        isNerve(GET_NERVE(Mogucchi, MogucchiNrvDown)) || isNerve(GET_NERVE(Mogucchi, MogucchiNrvAppearDown)) || isNerve(GET_NERVE(Mogucchi, MogucchiNrvDive));

    if (!isDown) {
        if (MR::isMsgPlayerTrample(msg)) {
            MR::startActionSound(this, "EmMogucchiRefrect");
            return true;
        }

        return false;
    }

    if (isNerve(GET_NERVE(Mogucchi, MogucchiNrvAppearDown)) && MR::isLessStep(this, 10)) {
        return false;
    }

    if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
        MR::startAction(this, "Bounce");
        MR::startBtp(this, "EyeOpen");
        return true;
    }

    if (MR::isMsgPlayerHitAll(msg)) {
        MR::stopScene(8);
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        calcScatterVec(*MR::getSensorPos(pSender), *MR::getSensorPos(pReceiver));
        setNerve(GET_NERVE(Mogucchi, MogucchiNrvScatter));
        return true;
    }

    return false;
}

void Mogucchi::updateReferenceMtx() {
    TVec3f v1 = mRotation * (PI / 180.0f);

    mNewHolePos.setRT(v1, mPosition);

    MR::setPosition(mHole, mPosition);
}

void Mogucchi::updateSpinCounterCooldown() {
    if (--mSpinCounterCooldown <= 0) {
        mSpinCounterCooldown = 0;
    }
}
