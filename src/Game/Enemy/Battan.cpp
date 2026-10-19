#pragma auto_inline off
#include "Game/Enemy/Battan.hpp"
#include "Game/Util.hpp"
#include "Game/Util/BattanKingCollisionPress.hpp"
#include "Game/Enemy/BattanKing.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Enemy/SupportTicoStop.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvBattan {
    NEW_NERVE(BattanNrvWait, Battan, Wait);
    class BattanNrvWalk : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast<Battan*>(pSpine->mExecutor)->exeBattanWalk();
        }
        static BattanNrvWalk sInstance;
    };
    BattanNrvWalk BattanNrvWalk::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(BattanNrvTurn, Battan, Turn);
    class BattanNrvTurnEnd : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast<Battan*>(pSpine->mExecutor)->exeBattanTurnEnd();
        }
        static BattanNrvTurnEnd sInstance;
    };
    BattanNrvTurnEnd BattanNrvTurnEnd::sInstance ATTRIBUTE_WEAK;
    class BattanNrvHalfTurn : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast<Battan*>(pSpine->mExecutor)->exeBattanHalfTurn();
        }
        static BattanNrvHalfTurn sInstance;
    };
    BattanNrvHalfTurn BattanNrvHalfTurn::sInstance ATTRIBUTE_WEAK;
    class BattanNrvFind : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast<Battan*>(pSpine->mExecutor)->exeBattanFind();
        }
        static BattanNrvFind sInstance;
    };
    BattanNrvFind BattanNrvFind::sInstance ATTRIBUTE_WEAK;
    class BattanNrvRun : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast<Battan*>(pSpine->mExecutor)->exeBattanRun();
        }
        static BattanNrvRun sInstance;
    };
    BattanNrvRun BattanNrvRun::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(BattanNrvAttackSign, Battan, AttackSign);
    class BattanNrvAttack : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast<Battan*>(pSpine->mExecutor)->exeBattanAttack();
        }
        static BattanNrvAttack sInstance;
    };
    BattanNrvAttack BattanNrvAttack::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(BattanNrvWaitFall, Battan, WaitFall);
    class BattanNrvRiseSign : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast<Battan*>(pSpine->mExecutor)->exeBattanRiseSign();
        }
        static BattanNrvRiseSign sInstance;
    };
    BattanNrvRiseSign BattanNrvRiseSign::sInstance ATTRIBUTE_WEAK;
    class BattanNrvRise : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast<Battan*>(pSpine->mExecutor)->exeBattanRise();
        }
        static BattanNrvRise sInstance;
    };
    BattanNrvRise BattanNrvRise::sInstance ATTRIBUTE_WEAK;
    class BattanNrvDown : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast<Battan*>(pSpine->mExecutor)->exeBattanDown();
        }
        static BattanNrvDown sInstance;
    };
    BattanNrvDown BattanNrvDown::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvBattan

#define GET_BATTAN_NERVE(name) (&NrvBattan::name::sInstance)

// Global table (SMG2 lbl_80728EF8): 14 TVec3 constants filled by __sinit. Index names follow the asm offsets.
static TVec3f sVector0(800.0f, 1300.0f, 1600.0f);
static TVec3f sVector1(600.0f, 1300.0f, 1600.0f);
static TVec3f sVector2(450.0f, 1300.0f, 700.0f);
static TVec3f sVector3(0.0f, -400.0f, 0.0f);
static TVec3f sVector4(0.0f, 525.0f, -100.0f);
static TVec3f sVector5(700.0f, 850.0f, 200.0f);
static TVec3f sVector6(700.0f, 850.0f, 400.0f);
static TVec3f sVector7(50.0f, 0.0f, 0.0f);
static TVec3f sVector8(-50.0f, 0.0f, 0.0f);
static TVec3f sVector9(-148.43800354003906f, 100.0f, 846.6799926757812f);
static TVec3f sVector10(183.593994140625f, 100.0f, 724.6090087890625f);
static TVec3f sVector11(-235.0f, 100.0f, 567.3829956054688f);
static TVec3f sVector12(43.0f, 100.0f, 392.0f);
static TVec3f sVector13(0.0f, 0.0f, 150.0f);

// Per-part descriptors (SMG2 lbl_80682F54): collision name, model name, joint name.
struct BattanPartsDesc {
    const char* mName;
    const char* mModelName;
    const char* mJointName;
};

static BattanPartsDesc sPartsDesc[5] = {
    {"コリジョン[体　]", "Battan", "Root"},
    {"コリジョン[左腕]", "BattanArmL", "ArmL"},
    {"コリジョン[右腕]", "BattanArmR", "ArmR"},
    {"コリジョン[左足]", "BattanLegL", "LegL"},
    {"コリジョン[右足]", "BattanLegR", "LegR"},
};

// Unnamed SMG2 helpers used by Battan.
// Zero-valued tuning constant the target reads from .sbss2 at run time (defined at the end of the TU).
namespace {
    extern const volatile s32 sRisePushStartStep;
}

// Joint-controller delegate built in fn_8015BA20 (layout from the ctor stores).

// Filter for the rail/ground probe: rejects Battan's own collision parts.
#pragma auto_inline on
inline BattanPartsFilter::BattanPartsFilter(Battan* pHost, bool (Battan::*callback)(const CollisionParts*) const) : CollisionPartsFilterBase() {
    mHost = pHost;
    mCallback = callback;
}

#pragma auto_inline off

bool BattanPartsFilter::isInvalidParts(const CollisionParts* pParts) const {
    return (mHost->*mCallback)(pParts);
}

Battan::Battan(const char* pName) : LiveActor(pName) {
    mTicoStop = nullptr;
    mCollisionParts = nullptr;
    mPressParts = nullptr;
    mBreakModel = nullptr;
    mBreakParts = nullptr;
    mFront.x = 0.0f;
    mFront.y = 0.0f;
    mFront.z = 1.0f;
    mFrontPrev.x = 0.0f;
    mFrontPrev.y = 0.0f;
    mFrontPrev.z = 1.0f;
    s32 walkSpeed = 600;
    s32 runSpeed = 1200;
    mWalkSpeed = walkSpeed;
    mRunSpeed = runSpeed;
    mRailPassNo = -1;
    mIsTurnBack = false;
    mIsDropStarPiece = true;
    mIsPlayerFront = false;
    mIsPlayerSide = false;
}

Battan::~Battan() {
}

// NONMATCHING: register allocation, descriptor/string pool placement, and instruction scheduling.
void Battan::init(const JMapInfoIter& rIter) {
    if (MR::isValidInfo(rIter)) {
        MR::isConnectedWithRail(rIter);
    }
    MR::initActor(this, rIter, "Battan", false);

    mCollisionParts = new LiveActor*[5];
    mPressParts = new LiveActor*[5];
    for (s32 i = 0; i < 5; i++) {
        const BattanPartsDesc& rDesc = sPartsDesc[i];
        if (i == 0 || i == 3 || i == 4) {
            LiveActor* pPress = static_cast<LiveActor*>(::operator new(0x98));
            if (pPress != nullptr) {
                pPress = MR::fn_8008D920(pPress, this, rDesc.mName, rDesc.mModelName, 0, MR::getJointMtx(this, rDesc.mJointName), 0x1E);
            }
            mPressParts[i] = pPress;
            mPressParts[i]->initWithoutIter();
        } else {
            mPressParts[i] = nullptr;
        }

        // the SMG2 ctor takes a BattanKing*, but Battan passes itself
        LiveActor* pCollision = new BattanKingCollision(reinterpret_cast<BattanKing*>(this), rDesc.mName, rDesc.mModelName, nullptr, MR::getJointMtx(this, rDesc.mJointName), 0x1E);
        mCollisionParts[i] = pCollision;
        mCollisionParts[i]->initWithoutIter();
    }

    MR::moveCoordAndTransToNearestRailPos(this);
    MR::calcFrontVec(&mFront, this);

    TVec3f sp1C(mFront);
    MR::calcMovingDirectionAlongRail(this, &sp1C, mPosition, 800.0f, false, nullptr);
    if (mFront.dot(sp1C) < 0.0f) {
        MR::reverseRailDirection(this);
    }

    mTicoStop = new SupportTicoStop(this, nullptr);

    mBreakModel = new BreakModel(this, "バッタン壊れモデル", "BattanBreak", getBaseMtx(), "Break");
    mBreakModel->initWithoutIter();

    mBreakParts = new LiveActor*[4];
    for (s32 i = 0; i < 4; i++) {
        mBreakParts[i] = new LiveActor("バッタン壊れモデル残るもの");
        MR::initActor(mBreakParts[i], "BattanBreakStay", false);
        mBreakParts[i]->initWithoutIter();
        mBreakParts[i]->makeActorDead();
    }

    bool (Battan::*callback)(const CollisionParts*) const = &Battan::isBattanOwnCollisionParts;
    BattanPartsFilter* pFilter = new BattanPartsFilter(this, callback);
    MR::setBinderCollisionPartsFilter(this, pFilter);
    MR::declareStarPiece(this, 7);

    f32 val = -1.0f;
    MR::getJMapInfoArg0NoInit(rIter, &val);
    if (val >= 0.0f) {
        mWalkSpeed = val;
    }
    MR::getJMapInfoArg1NoInit(rIter, &val);
    if (val >= 0.0f) {
        mRunSpeed = val;
    }
    bool flag = false;
    MR::getJMapInfoArg2WithInit(rIter, &flag);
    mIsDropStarPiece = (flag == 0);

    initNerve(GET_BATTAN_NERVE(BattanNrvWait), 0);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void Battan::makeActorAppeared() {
    setNerve(GET_BATTAN_NERVE(BattanNrvWait));
    LiveActor::makeActorAppeared();
    mRailPassNo = -1;
    setBattanStandingParts();
    mIsPlayerFront = false;
    mIsPlayerSide = false;
}

void Battan::control() {
    mTicoStop->updateNerve();
}

void Battan::calcAndSetBaseMtx() {
    MR::fn_8000F880(this, &mFront);
}

void Battan::calcAnim() {
    LiveActor::calcAnim();
    if (MR::isDead(this) || MR::isNoCalcAnim(this) || MR::isClipped(this)) {
        return;
    }
    for (s32 i = 0; i < 5; i++) {
        mCollisionParts[i]->calcAnim();
        if (mPressParts[i] != nullptr) {
            mPressParts[i]->calcAnim();
        }
    }
}

// Sets the body/press parts to the standing pose (body 0,3,4 visible; press all hidden).
void Battan::setBattanStandingParts() {
    mCollisionParts[0]->appear();
    mCollisionParts[1]->kill();
    mCollisionParts[2]->kill();
    mCollisionParts[3]->appear();
    mCollisionParts[4]->appear();
    for (s32 i = 0; i < 5; i++) {
        if (mPressParts[i] != nullptr) {
            mPressParts[i]->kill();
        }
    }
    MR::validateHitSensor(this, "HandL");
    MR::validateHitSensor(this, "HandR");
}

void Battan::setBattanPressingParts() {
    for (s32 i = 0; i < 5; i++) {
        if (mPressParts[i] != nullptr) {
            mPressParts[i]->appear();
            mCollisionParts[i]->kill();
        } else {
            mCollisionParts[i]->appear();
        }
    }
    mCollisionParts[1]->kill();
    mCollisionParts[2]->kill();
    MR::validateHitSensor(this, "HandL");
    MR::validateHitSensor(this, "HandR");
}

void Battan::setBattanFallenParts() {
    mCollisionParts[0]->appear();
    mCollisionParts[1]->appear();
    mCollisionParts[2]->appear();
    mCollisionParts[3]->appear();
    mCollisionParts[4]->appear();
    for (s32 i = 0; i < 5; i++) {
        if (mPressParts[i] != nullptr) {
            mPressParts[i]->kill();
        }
    }
    MR::invalidateHitSensor(this, "HandL");
    MR::invalidateHitSensor(this, "HandR");
}

void Battan::setBattanRisingParts() {
    for (s32 i = 0; i < 5; i++) {
        if (mPressParts[i] != nullptr) {
            if (i == 0) {
                mPressParts[i]->kill();
                mCollisionParts[i]->appear();
            } else {
                mPressParts[i]->appear();
                mCollisionParts[i]->kill();
            }
        } else {
            mCollisionParts[i]->appear();
        }
    }
    MR::invalidateHitSensor(this, "HandL");
    MR::invalidateHitSensor(this, "HandR");
}

void Battan::killPartsAll() {
    for (s32 i = 0; i < 5; i++) {
        mCollisionParts[i]->kill();
        if (mPressParts[i] != nullptr) {
            mPressParts[i]->kill();
        }
    }
}

void Battan::onCollisionPartsAll() {
        for (s32 i = 0; i < 5; i++) {
        MR::onUpdateCollisionParts(mCollisionParts[i]);
        if (mPressParts[i] != nullptr) {
            MR::onUpdateCollisionParts(mPressParts[i]);
        }
    }
}

void Battan::offCollisionPartsAll() {
        for (s32 i = 0; i < 5; i++) {
        MR::offUpdateCollisionParts(mCollisionParts[i]);
        if (mPressParts[i] != nullptr) {
            MR::offUpdateCollisionParts(mPressParts[i]);
        }
    }
}

// True when pParts belongs to one of this Battan's body parts.
bool Battan::isBattanOwnCollisionParts(const CollisionParts* pParts) const {
    LiveActor* host = pParts->mHitSensor->mHost;
    for (s32 i = 0; i < 5; i++) {
        if (mCollisionParts[i] == host) {
            return true;
        }
        if (mPressParts[i] != nullptr && mPressParts[i] == host) {
            return true;
        }
    }
    return false;
}

// Nerves in which an incoming attack is treated as a hit (isBattanVulnerable).
bool Battan::isBattanVulnerable() const {
    bool result = false;
    if (isNerve(GET_BATTAN_NERVE(BattanNrvWaitFall)) || isNerve(GET_BATTAN_NERVE(BattanNrvRiseSign)) ||
        (isNerve(GET_BATTAN_NERVE(BattanNrvRise)) && MR::isLessStep(this, 30))) {
        result = true;
    }
    return result;
}

// NONMATCHING: temporary vector stack allocation.
void Battan::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "HandL") || MR::isEqualSensor(pSender, this, "HandR")) {
        if (MR::isSensorPlayerOrYoshiOrRide(pReceiver) || MR::isSensorEnemy(pReceiver)) {
            if (MR::sendMsgPush(pReceiver, pSender)) {
                return;
            } else {
                return;
            }
        }
        return;
    } else if (MR::isEqualSensor(pSender, this, "Push")) {
        if (isNerve(GET_BATTAN_NERVE(BattanNrvAttack)) && MR::isGreaterEqualStep(this, 0x2A) && MR::isLessStep(this, 0x33)) {
            TVec3f dir(0.0f);
            if (calcBattanPlayerPushDirection(&dir, &sVector6)) {
                if (mIsPlayerFront && MR::isGreaterEqualStep(this, 0x32)) {
                    MR::fn_800515E0();
                }
                TVec3f push(dir);
                push.scale(1.0f);
                MR::pushPlayer(push);
            }
        } else if (isNerve(GET_BATTAN_NERVE(BattanNrvRise)) && MR::isGreaterEqualStep(this, sRisePushStartStep) && MR::isLessStep(this, 0x6E)) {
            TVec3f dir(0.0f);
            if (calcBattanPlayerPushDirection(&dir, &sVector5)) {
                TVec3f push(dir);
                push.scale(1.0f);
                MR::pushPlayer(push);
            }
        } else {
            mIsPlayerFront = false;
            mIsPlayerSide = false;
        }
    }
}

bool Battan::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDropFloor(msg)) {
        if (MR::isPlayerInCollisionCylinder(MR::getJointMtx(this, "Root"), &sVector4, 200.0f, 400.0f)) {
            return tryBattanKnockDown();
        }
        return false;
    }
    return false;
}

// Hip-drop / attack kill check (tryBattanKnockDown). Returns true when the Battan was knocked down.
bool Battan::tryBattanKnockDown() {
    if (isBattanVulnerable()) {
        MR::tryHitReaction(this, "ヒップドロップ");
        mTicoStop->tryTouch();
        setNerve(GET_BATTAN_NERVE(BattanNrvDown));
        return true;
    }
    return false;
}

// Rail end handling: loop rails keep going, otherwise turn around (fn_8015AC40).
bool Battan::tryRailEndTurn() {
    if (MR::isLoopRail(this)) {
        MR::zeroVelocity(this);
        setNerve(GET_BATTAN_NERVE(BattanNrvHalfTurn));
        return true;
    }
    if (MR::isRailReachedGoal(this)) {
        MR::reverseRailDirection(this);
        setNerve(GET_BATTAN_NERVE(BattanNrvTurn));
        return true;
    }
    MR::zeroVelocity(this);
    setNerve(GET_BATTAN_NERVE(BattanNrvHalfTurn));
    return true;
}

// Steps along the rail and applies gravity-aligned velocity (moveBattanOnRail).
void Battan::moveBattanOnRail(f32 speed) {
    mRailPassNo = MR::moveCoordAndCheckPassPointNo(this, speed);
    if (mRailPassNo == -1) {
        MR::vecKillElement(MR::getRailDirection(this), mGravity, &mFront);
        MR::normalize(&mFront);
    }
    MR::killVelocityHorizon(this);
    TVec3f horizontal(0.0f);
    TVec3f delta = MR::getRailPos(this) - mPosition;
    MR::vecKillElement(delta, mGravity, &horizontal);
    f32 value = PSVECMag(&horizontal) / 150.0f;
    f32 rate = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
    if (rate > 0.01f && mFront.dot(horizontal) > 0.0f) {
        MR::addVelocity(this, horizontal);
    }
    f32 speedMagnitude = PSVECMag(&mVelocity);
    f32 limit = 30.0f;
    if (limit < speedMagnitude) {
        mVelocity.setLength(limit);
    }
    MR::addVelocityToGravity(this, 0.5f);
    if (MR::isBindedGround(this)) {
        MR::killVelocityVertical(this);
    }
}

// True when the rail is at its edge: loop rails never end, others end on pass point -1 or at the goal.
bool Battan::isRailEndReached() const {
    if (MR::isLoopRail(this)) {
        return (mRailPassNo + 1) != 0;
    }
    return mRailPassNo != -1 || MR::isRailReachedGoal(this);
}

// Repositions the four break parts around the body (appearBattanBreakParts).
// NONMATCHING: Euler degree-conversion floating-point register allocation.
void Battan::appearBattanBreakParts() {
    mBreakModel->appear();
    for (s32 i = 0; i < 4; i++) {
        TVec3f pos(0.0f);
        if (calcBattanBreakPartPosition(&pos, i)) {
            TVec3f rot(0.0f);
            TPos3f mtx;
            mtx.identity();
            PSMTXCopy(getBaseMtx(), mtx);
            MR::rotateMtxLocalYDegree(mtx, MR::getRandomDegree());
            mtx.getEuler(rot);
            rot.x = 57.29578f * rot.x;
            rot.y = 57.29578f * rot.y;
            rot.z = 57.29578f * rot.z;
            MR::setPosition(mBreakParts[i], pos);
            MR::setRotation(mBreakParts[i], rot);
            mBreakParts[i]->appear();
        }
    }
}

// Ray-casts a break-part position from the body base matrix.
bool Battan::calcBattanBreakPartPosition(TVec3f* pPos, s32 index) {
    TPos3f mtx;
    mtx.set(getBaseMtx());
    TVec3f offs(0.0f);
    switch (index) {
    case 0:
        offs = sVector9;
        break;
    case 1:
        offs = sVector10;
        break;
    case 2:
        offs = sVector11;
        break;
    case 3:
        offs = sVector12;
        break;
    }

    MR::addTransMtxLocal(mtx.toMtxPtr(), offs);
    TVec3f origin(mPosition);
    MR::extractMtxTrans(mtx.toMtxPtr(), &origin);
    TVec3f dir(mGravity);
    dir.scale(300.0f);
    if (MR::getFirstPolyOnLineToMap(pPos, nullptr, origin, dir)) {
        MR::addTransMtxLocal(mtx.toMtxPtr(), sVector13);
        MR::extractMtxTrans(mtx.toMtxPtr(), &origin);
        TVec3f dir2(mGravity);
        dir2.scale(300.0f);
        if (MR::getFirstPolyOnLineToMap(nullptr, nullptr, origin, dir2)) {
            return true;
        }
    }
    return false;
}

// NONMATCHING: floating-point register allocation in range comparisons.
bool Battan::calcBattanPlayerPushDirection(TVec3f* pDir, const TVec3f* pRange) {
    bool prevFront = mIsPlayerFront;
    bool prevSide = mIsPlayerSide;
    TPos3f inverse;
    TPos3f mtx;
    PSMTXCopy(MR::getJointMtx(this, "BattanBody"), mtx);
    PSMTXInverse(mtx, inverse);
    TVec3f local(0.0f);
    PSMTXMultVec(inverse, MR::getPlayerCenterPos(), &local);
    f32 half = 0.5f;
    f32 z = local.z;
    bool inRange;
    if (!(half * -pRange->x <= local.x && local.x < half * pRange->x)) {
        inRange = false;
    } else if (!(0.0f <= local.y && local.y < pRange->y)) {
        inRange = false;
    } else {
        f32 depth = 500.0f;
        if (!(half * -depth <= z && z < half * depth)) {
            inRange = false;
        } else {
            inRange = true;
        }
    }
    bool front = z >= 0.0f;
    mIsPlayerFront = front;
    if (!inRange) {
        return false;
    }
    mIsPlayerSide = true;
    MR::extractMtxZDir(MR::getJointMtx(this, "BattanBody"), pDir);
    f32 edge = prevFront ? 0.5f * pRange->z : 0.5f * -pRange->z;
    pDir->scale(edge - z);
    if (prevSide && front != prevFront) {
        mIsPlayerFront = prevFront;
        return true;
    }
    if (z < 0.5f * pRange->z) {
        return true;
    }
    return false;
}

// NONMATCHING: temporary vector stack allocation.
void Battan::updateHitSensor(HitSensor* pSensor) {
    TPos3f mtx;
    PSMTXIdentity(mtx);
    if (MR::isEqualSensor(pSensor, this, "HandL")) {
        PSMTXCopy(MR::getJointMtx(this, "HandL"), mtx);
        MR::addTransMtxLocal(mtx.toMtxPtr(), sVector7);
    } else if (MR::isEqualSensor(pSensor, this, "HandR")) {
        PSMTXCopy(MR::getJointMtx(this, "HandR"), mtx);
        MR::addTransMtxLocal(mtx.toMtxPtr(), sVector8);
    } else {
        return;
    }

    TVec3f pos;
    MR::extractMtxTrans(mtx.toMtxPtr(), &pos);
    TVec3f* sensorPos = MR::getSensorPos2(pSensor);
    sensorPos->x = pos.x;
    sensorPos->y = pos.y;
    sensorPos->z = pos.z;

    TVec3f diff = pos - *MR::getPlayerPos();
    TVec3f side(0.0f);
    MR::vecKillElement(diff, mGravity, &side);
    f32 len = PSVECMag(&side);
    if (!(len > 50.0f)) {
        if (MR::normalizeOrZero(&side)) {
            side = mFront;
        }
        TVec3f neg = -side;
        TVec3f off(neg);
        off.scale(100.0f);
        MR::getSensorPos2(pSensor)->add(off);
    }
}

void Battan::exeWait() {
    if (MR::isFirstStep(this)) {
        setBattanStandingParts();
        MR::startAction(this, "WaitStand");
        MR::validateClipping(this);
        MR::offBind(this);
        mRailPassNo = -1;
    }
    if (!mTicoStop->tryAttachTouch() && (!MR::isValidSwitchA(this) || MR::isOnSwitchA(this)) && MR::isGreaterEqualStep(this, 0x1E)) {
        if (mIsTurnBack) {
            tryRailEndTurn();
            return;
        }
        setNerve(GET_BATTAN_NERVE(BattanNrvWalk));
    }
}

// NONMATCHING: float load/multiply instruction scheduling.
void Battan::exeBattanWalk() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Walk");
        MR::onBind(this);
        MR::vecKillElement(MR::getRailDirection(this), mGravity, &mFront);
        MR::normalize(&mFront);
        MR::validateClipping(this);
    }
    if (!mTicoStop->tryAttachTouch()) {
        TPos3f mtx;
        TVec3f up = -mGravity;
        MR::makeMtxFrontUpPos(&mtx, mFront, up, mPosition);
        MR::addTransMtxLocal(mtx.toMtxPtr(), sVector3);
        if (MR::isPlayerInCollisionBox(&mtx, &sVector1)) {
            MR::zeroVelocity(this);
            setNerve(GET_BATTAN_NERVE(BattanNrvFind));
            return;
        }
        moveBattanOnRail(mWalkSpeed * 0.01f);
        if (isRailEndReached()) {
            tryRailEndTurn();
        }
    }
}

void Battan::exeTurn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Turn");
        MR::offBind(this);
        mIsTurnBack = false;
        MR::validateClipping(this);
        MR::zeroVelocity(this);
    }
    if (!mTicoStop->tryAttachTouch() && MR::isActionEnd(this)) {
        setNerve(GET_BATTAN_NERVE(BattanNrvTurnEnd));
    }
}

void Battan::exeBattanTurnEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "TurnEnd");
        MR::offBind(this);
        MR::vecKillElement(MR::getRailDirection(this), mGravity, &mFront);
        MR::normalize(&mFront);
        MR::resetPosition(this);
    }
    setNerve(GET_BATTAN_NERVE(BattanNrvWalk));
}

void Battan::exeBattanHalfTurn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HalfTurn");
        MR::offBind(this);
        mIsTurnBack = false;
        MR::validateClipping(this);
        MR::zeroVelocity(this);
        mFrontPrev = mFront;
    }
    if (!mTicoStop->tryAttachTouch()) {
        TVec3f target(0.0f, 0.0f, 1.0f);
        MR::fn_800531B0(&target, this, 0.1f);
        if (!MR::isRailGoingToEnd(this)) {
            target = -target;
        }
        MR::vecKillElement(target, mGravity, &target);
        MR::normalize(&target);
        MR::vecBlendSphere(&mFront, mFrontPrev, target, MR::calcNerveRate(this, 0x3C));
        if (MR::isGreaterStep(this, 0x3C)) {
            setNerve(GET_BATTAN_NERVE(BattanNrvWalk));
        }
    }
}

void Battan::exeBattanFind() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Find");
        MR::invalidateClipping(this);
    }
    if (!mTicoStop->tryAttachTouch() && MR::isActionEnd(this)) {
        TPos3f mtx;
        TVec3f up = -mGravity;
        MR::makeMtxFrontUpPos(&mtx, mFront, up, mPosition);
        MR::addTransMtxLocal(mtx.toMtxPtr(), sVector3);
        if (MR::isPlayerInCollisionBox(&mtx, &sVector2)) {
            setNerve(GET_BATTAN_NERVE(BattanNrvAttackSign));
            return;
        }
        setNerve(GET_BATTAN_NERVE(BattanNrvRun));
    }
}

// NONMATCHING: float load/multiply instruction scheduling.
void Battan::exeBattanRun() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Run");
        MR::invalidateClipping(this);
    }
    if (!mTicoStop->tryAttachTouch()) {
        TPos3f mtx;
        TVec3f up = -mGravity;
        MR::makeMtxFrontUpPos(&mtx, mFront, up, mPosition);
        MR::addTransMtxLocal(mtx.toMtxPtr(), sVector3);
        if (MR::isPlayerInCollisionBox(&mtx, &sVector0)) {
            TPos3f mtx2;
            MR::makeMtxFrontUpPos(&mtx2, mFront, -mGravity, mPosition);
            MR::addTransMtxLocal(mtx2.toMtxPtr(), sVector3);
            if (MR::isPlayerInCollisionBox(&mtx2, &sVector2)) {
                MR::zeroVelocity(this);
                setNerve(GET_BATTAN_NERVE(BattanNrvAttackSign));
                return;
            }
            moveBattanOnRail(mRunSpeed * 0.01f);
            if (isRailEndReached()) {
                mIsTurnBack = true;
                MR::zeroVelocity(this);
                setNerve(GET_BATTAN_NERVE(BattanNrvAttackSign));
            }
        } else {
            MR::zeroVelocity(this);
            setNerve(GET_BATTAN_NERVE(BattanNrvWait));
        }
    }
}

void Battan::exeAttackSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSign");
        MR::invalidateClipping(this);
        MR::offBind(this);
    }
    if (!mTicoStop->tryAttachTouch() && MR::isGreaterEqualStep(this, 0x50)) {
        setNerve(GET_BATTAN_NERVE(BattanNrvAttack));
    }
}

void Battan::exeBattanAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        MR::invalidateClipping(this);
    }
    if (MR::isLessStep(this, 0x2E) && mTicoStop->tryAttachTouch()) {
        if (mTicoStop->mIsTouchStarted) {
            setBattanFallenParts();
        }
    } else {
        if (mTicoStop->mIsTouching && MR::isGreaterStep(this, 0x23)) {
            setBattanPressingParts();
        }
        if (MR::isStep(this, 0x23)) {
            setBattanPressingParts();
        }
        if (MR::isActionEnd(this)) {
            setBattanFallenParts();
            setNerve(GET_BATTAN_NERVE(BattanNrvWaitFall));
        }
    }
}

void Battan::exeWaitFall() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "WaitFall");
        MR::invalidateClipping(this);
    }
    if (!mTicoStop->tryAttachTouch() && MR::isGreaterEqualStep(this, 0xB4)) {
        setNerve(GET_BATTAN_NERVE(BattanNrvRiseSign));
    }
}

void Battan::exeBattanRiseSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "RiseSign");
        offCollisionPartsAll();
        MR::invalidateClipping(this);
    }
    if (!mTicoStop->tryAttachTouch() && MR::isGreaterEqualStep(this, 0x3C)) {
        onCollisionPartsAll();
        setBattanStandingParts();
        setNerve(GET_BATTAN_NERVE(BattanNrvRise));
    }
}

void Battan::exeBattanRise() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Rise");
        MR::invalidateClipping(this);
    }
    if (MR::isGreaterStep(this, 0x3C) && mTicoStop->tryAttachTouch()) {
        if (mTicoStop->mIsTouchStarted) {
            setBattanStandingParts();
        }
    } else {
        if (mTicoStop->mIsTouching && MR::isGreaterStep(this, 0xC)) {
            setBattanRisingParts();
        }
        if (MR::isStep(this, 0xC)) {
            setBattanRisingParts();
        }
        if (MR::isActionEnd(this)) {
            setNerve(GET_BATTAN_NERVE(BattanNrvWait));
        }
    }
}

// NONMATCHING: temporary vector stack allocation.
void Battan::exeBattanDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Down");
        MR::invalidateClipping(this);
    }
    if (MR::isStep(this, 3)) {
        killPartsAll();
    }
    if (MR::isGreaterEqualStep(this, 3) || MR::isOnPlayer(this) || MR::isHiddenModel(this)) {
        if (!MR::isHiddenModel(this)) {
            appearBattanBreakParts();
            if (mIsDropStarPiece) {
                TVec3f off(mFront);
                off.scale(400.0f);
                TVec3f pos = mPosition + off;
                MR::appearStarPiece(this, pos, 7, 10.0f, 40.0f, false);
            }
            MR::tryHitReactionDead(this);
            MR::hideModel(this);
            killPartsAll();
            return;
        }
        if (MR::isDead(mBreakModel)) {
            kill();
        }
    }
}

namespace {
    extern const volatile s32 sRisePushStartStep = 0;
}
