#include "Game/Enemy/Battan.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Enemy/SupportTicoStop.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvBattan {
    NEW_NERVE(BattanNrvWait, Battan, Wait);
    NEW_NERVE(BattanNrvWalk, Battan, Walk);
    NEW_NERVE(BattanNrvTurn, Battan, Turn);
    NEW_NERVE(BattanNrvTurnEnd, Battan, TurnEnd);
    NEW_NERVE(BattanNrvHalfTurn, Battan, HalfTurn);
    NEW_NERVE(BattanNrvFind, Battan, Find);
    NEW_NERVE(BattanNrvRun, Battan, Run);
    NEW_NERVE(BattanNrvAttackSign, Battan, AttackSign);
    NEW_NERVE(BattanNrvAttack, Battan, Attack);
    NEW_NERVE(BattanNrvWaitFall, Battan, WaitFall);
    NEW_NERVE(BattanNrvRiseSign, Battan, RiseSign);
    NEW_NERVE(BattanNrvRise, Battan, Rise);
    NEW_NERVE(BattanNrvDown, Battan, Down);
};  // namespace NrvBattan

#define GET_BATTAN_NERVE(name) (&NrvBattan::name::sInstance)

// Global table (SMG2 lbl_80728EF8): 14 TVec3 constants filled by __sinit. Index names follow the asm offsets.
static TVec3f sTable[14] = {
    TVec3f(800.0f, 1300.0f, 1600.0f),   // [0]  0x00
    TVec3f(600.0f, 1300.0f, 1600.0f),   // [1]  0x0C (also lbl_80728F04)
    TVec3f(450.0f, 1300.0f, 700.0f),    // [2]  0x18 (also lbl_80728F10)
    TVec3f(0.0f, -400.0f, 0.0f),        // [3]  0x24 (also lbl_80728F1C)
    TVec3f(0.0f, 525.0f, -100.0f),      // [4]  0x30 (also lbl_80728F28)
    TVec3f(700.0f, 850.0f, 200.0f),     // [5]  0x3C (also lbl_80728F34)
    TVec3f(700.0f, 850.0f, 400.0f),     // [6]  0x48 (also lbl_80728F40)
    TVec3f(50.0f, 0.0f, 0.0f),          // [7]  0x54 (also lbl_80728F4C)
    TVec3f(-50.0f, 0.0f, 0.0f),         // [8]  0x60 (also lbl_80728F58)
    TVec3f(-148.438f, 100.0f, 846.68f), // [9]  0x6C
    TVec3f(183.594f, 100.0f, 724.609f), // [10] 0x78
    TVec3f(-235.0f, 100.0f, 567.383f),  // [11] 0x84
    TVec3f(43.0f, 100.0f, 392.0f),      // [12] 0x90
    TVec3f(0.0f, 0.0f, 150.0f),         // [13] 0x9C
};

// Per-part descriptors (SMG2 lbl_80682F54): collision name, model name, joint name.
struct BattanPartsDesc {
    const char* mName;
    const char* mModelName;
    const char* mJointName;
};

static const BattanPartsDesc sPartsDesc[5] = {
    {"コリジョン[体　]", "Battan", "Root"},
    {"コリジョン[左腕]", "BattanArmL", "ArmL"},
    {"コリジョン[右腕]", "BattanArmR", "ArmR"},
    {"コリジョン[左足]", "BattanLegL", "LegL"},
    {"コリジョン[右足]", "BattanLegR", "LegR"},
};

// Unnamed SMG2 helpers used by Battan.
extern "C" LiveActor* fn_8008D920(LiveActor*, LiveActor*, const char*, const char*, s32, MtxPtr, s32);
extern "C" LiveActor* fn_8008DB20(LiveActor*, LiveActor*, const char*, const char*, s32, MtxPtr, s32);
extern "C" bool fn_8008DDC0(const TVec3f*, f32, f32);
extern "C" bool fn_8008E060(const TPos3f*, const TVec3f*);
extern "C" void fn_8000F880(const TVec3f*);
extern "C" void fn_800515E0();
extern "C" s32 lbl_807E3020;
extern "C" bool fn_800531B0(TVec3f*, LiveActor*, f32);

// Joint-controller delegate built in fn_8015BA20 (layout from the ctor stores).
struct BattanJointDelegate {
    JointController* mController;  // 0x00
    TVec3f mPos;                   // 0x04
    TVec3f mDir;                   // 0x10 (default 0,1,0)
    u32 _1C[3];                    // 0x1C..0x27 (callback data, zeroed)
};

// Filter for the rail/ground probe: rejects Battan's own collision parts.
BattanPartsFilter::BattanPartsFilter(Battan* pHost) : CollisionPartsFilterBase() {
    mHost = pHost;
    _8 = 0;
    _C = -1;
    _10 = nullptr;
}

bool BattanPartsFilter::isInvalidParts(const CollisionParts* pParts) const {
    return mHost->isBattanPartsSensor(pParts);
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
    mWalkSpeed = 600.0f;
    mRunSpeed = 1200.0f;
    mRailPassNo = -1;
    mIsTurnBack = false;
    mIsDropStarPiece = true;
    mIsPlayerFront = false;
    mIsPlayerSide = false;
}

Battan::~Battan() {
}

void Battan::init(const JMapInfoIter& rIter) {
    MR::isValidInfo(rIter);
    MR::initActor(this, rIter, "Battan", nullptr, false);

    mCollisionParts = new LiveActor*[5];
    mPressParts = new LiveActor*[5];
    for (s32 i = 0; i < 5; i++) {
        const BattanPartsDesc& rDesc = sPartsDesc[i];
        if (i == 0 || i == 3 || i == 4) {
            LiveActor* pPress = static_cast<LiveActor*>(::operator new(0x98));
            if (pPress != nullptr) {
                pPress = fn_8008D920(pPress, this, rDesc.mName, rDesc.mModelName, 0, MR::getJointMtx(this, rDesc.mJointName), 0x1E);
            }
            mPressParts[i] = pPress;
            mPressParts[i]->initWithoutIter();
        } else {
            mPressParts[i] = nullptr;
        }

        LiveActor* pCollision = static_cast<LiveActor*>(::operator new(0x98));
        if (pCollision != nullptr) {
            pCollision = fn_8008DB20(pCollision, this, rDesc.mName, rDesc.mModelName, 0, MR::getJointMtx(this, rDesc.mJointName), 0x1E);
        }
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

    BattanPartsFilter* pFilter = new BattanPartsFilter(this);
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
    s32 flag = 0;
    MR::getJMapInfoArg2WithInit(rIter, &flag);
    mIsDropStarPiece = (flag == 0);

    initNerve(GET_BATTAN_NERVE(BattanNrvWait), 0);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void Battan::makeActorAppeared() {
    setNerve(GET_BATTAN_NERVE(BattanNrvWait));
    LiveActor::makeActorAppeared();
    mRailPassNo = -1;
    setPartsStateStand();
    mIsPlayerFront = false;
    mIsPlayerSide = false;
}

void Battan::control() {
    mTicoStop->updateNerve();
}

void Battan::calcAndSetBaseMtx() {
    fn_8000F880(&mFront);
}

void Battan::calcAnim() {
    LiveActor::calcAnim();
    if (MR::isDead(this) || MR::isNoCalcAnim(this)) {
        return;
    }
    if (MR::isClipped(this)) {
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
void Battan::setPartsStateStand() {
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
}

void Battan::setPartsStateAttack() {
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
}

void Battan::setPartsStateAllAppear() {
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
}

void Battan::setPartsStateHalf() {
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
}

void Battan::killPartsAll() {
    for (s32 i = 0; i < 5; i++) {
        mCollisionParts[i]->kill();
        if (mPressParts[i] != nullptr) {
            mPressParts[i]->kill();
        }
    }
}

void Battan::onCollisionPartsAll(LiveActor* pActor) {
        for (s32 i = 0; i < 5; i++) {
        MR::onUpdateCollisionParts(mCollisionParts[i]);
        if (mPressParts[i] != nullptr) {
            MR::onUpdateCollisionParts(mPressParts[i]);
        }
    }
}

void Battan::offCollisionPartsAll(LiveActor* pActor) {
        for (s32 i = 0; i < 5; i++) {
        MR::offUpdateCollisionParts(mCollisionParts[i]);
        if (mPressParts[i] != nullptr) {
            MR::offUpdateCollisionParts(mPressParts[i]);
        }
    }
}

// True when pParts belongs to one of this Battan's body parts.
bool Battan::isBattanPartsSensor(const CollisionParts* pParts) const {
    for (s32 i = 0; i < 5; i++) {
        if (static_cast<const void*>(mCollisionParts[i]) == static_cast<const void*>(pParts)) {
            return true;
        }
        if (mPressParts[i] != nullptr && static_cast<const void*>(mPressParts[i]) == static_cast<const void*>(pParts)) {
            return true;
        }
    }
    return false;
}

// Nerves in which an incoming attack is treated as a hit (fn_8015AB40).
static bool isAttackingNerve(LiveActor* pActor) {
    if (pActor->isNerve(GET_BATTAN_NERVE(BattanNrvWaitFall)) || pActor->isNerve(GET_BATTAN_NERVE(BattanNrvRiseSign))) {
        return true;
    }
    if (pActor->isNerve(GET_BATTAN_NERVE(BattanNrvRise)) && MR::isLessStep(pActor, 0x1E)) {
        return true;
    }
    return false;
}

void Battan::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "HandL") || MR::isEqualSensor(pSender, this, "HandR")) {
        if ((MR::isSensorPlayerOrYoshiOrRide(pReceiver) || MR::isSensorEnemy(pReceiver)) && !MR::sendMsgPush(pReceiver, pSender)) {
        }
    } else if (MR::isEqualSensor(pSender, this, "Push")) {
        if (isNerve(GET_BATTAN_NERVE(BattanNrvAttack)) && MR::isGreaterEqualStep(this, 0x2A) && MR::isLessStep(this, 0x33)) {
            TVec3f dir(0.0f);
            if (calcPlayerHitSide(&dir, &sTable[6])) {
                if (mIsPlayerFront && MR::isGreaterEqualStep(this, 0x32)) {
                    fn_800515E0();
                }
                TVec3f push(dir);
                push.scale(1.0f);
                MR::pushPlayer(push);
            }
        } else if (isNerve(GET_BATTAN_NERVE(BattanNrvRise)) && MR::isGreaterEqualStep(this, lbl_807E3020) && MR::isLessStep(this, 0x6E)) {
            TVec3f dir(0.0f);
            if (calcPlayerHitSide(&dir, &sTable[5])) {
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
        MR::getJointMtx(this, "Root");
        if (fn_8008DDC0(&sTable[4], 200.0f, 400.0f)) {
            return tryAttackDown(this);
        }
        return false;
    }
    return false;
}

// Hip-drop / attack kill check (fn_8015ABD0). Returns true when the Battan was knocked down.
bool Battan::tryAttackDown(LiveActor* pActor) {
    if (isAttackingNerve(pActor)) {
        MR::tryHitReaction(pActor, "ヒップドロップ");
        static_cast<Battan*>(pActor)->mTicoStop->tryTouch();
        pActor->setNerve(GET_BATTAN_NERVE(BattanNrvDown));
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

// Steps along the rail and applies gravity-aligned velocity (fn_8015ACE0).
bool Battan::moveRailStep(f32 speed) {
    mRailPassNo = MR::moveCoordAndCheckPassPointNo(this, speed);
    if (mRailPassNo == -1) {
        TVec3f tmp;
        MR::vecKillElement(MR::getRailDirection(this), mGravity, &tmp);
        MR::normalize(&mFront, tmp);
    }

    MR::killVelocityHorizon(this);

    TVec3f sp8 = MR::getRailPos(this) - mPosition;
    TVec3f sp14;
    MR::vecKillElement(sp8, mGravity, &sp14);

    f32 rate = MR::normalize(sp14.length() / 150.0f, 0.0f, 1.0f);
    if (rate > 0.01f && mFront.dot(sp14) > 0.0f) {
        MR::addVelocity(this, sp14);
    }

    f32 vmag = mVelocity.length();
    if (vmag > 30.0f) {
        mVelocity.scale(30.0f / vmag);
    }
    MR::addVelocityToGravity(this, 0.5f);
    if (MR::isBindedGround(this)) {
        MR::killVelocityVertical(this);
    }
    return true;
}

// True when the rail is at its edge: loop rails never end, others end on pass point -1 or at the goal.
bool Battan::isRailEndReached() const {
    if (MR::isLoopRail(this)) {
        return (mRailPassNo + 1) != 0;
    }
    return mRailPassNo != -1 || MR::isRailReachedGoal(this);
}

// Repositions the four break parts around the body (fn_8015AEE0).
void Battan::updateLegs() {
    mBreakModel->appear();
    for (s32 i = 0; i < 4; i++) {
        TVec3f pos(0.0f);
        if (calcLegPos(&pos, i)) {
            TVec3f rot(0.0f);
            TPos3f mtx;
            MR::makeMtxFrontUpPos(&mtx, mFront, TVec3f(0.0f) - mGravity, mPosition);
            MR::rotateMtxLocalYDegree(mtx.toMtxPtr(), MR::getRandomDegree());
                MR::setPosition(mBreakParts[i], pos);
            MR::setRotation(mBreakParts[i], rot);
            mBreakParts[i]->appear();
        }
    }
}

// Ray-casts a break-part position from the body's base matrix (fn_8015B130). NONMATCHING: mtx setup approximated.
bool Battan::calcLegPos(TVec3f* pPos, s32 index) {
    TVec3f offs(0.0f);
    switch (index) {
    case 0:
        offs = sTable[9];
        break;
    case 1:
        offs = sTable[10];
        break;
    case 2:
        offs = sTable[11];
        break;
    case 3:
        offs = sTable[12];
        break;
    }

    TPos3f mtx;
    MR::addTransMtxLocal(mtx.toMtxPtr(), offs);
    TVec3f origin;
    MR::extractMtxTrans(mtx.toMtxPtr(), &origin);
    TVec3f dir(mGravity);
    dir.scale(300.0f);
    if (MR::getFirstPolyOnLineToMap(pPos, nullptr, origin, dir)) {
        MR::addTransMtxLocal(mtx.toMtxPtr(), sTable[13]);
        MR::extractMtxTrans(mtx.toMtxPtr(), &origin);
        TVec3f dir2(mGravity);
        dir2.scale(300.0f);
        if (MR::getFirstPolyOnLineToMap(nullptr, nullptr, origin, dir2)) {
            return true;
        }
    }
    return false;
}

// Checks whether the player is inside the hit box in the body's local space (fn_8015B2D0). NONMATCHING: range test approximated.
bool Battan::calcPlayerHitSide(TVec3f* pDir, const TVec3f* pRange) {
    bool prevFront = mIsPlayerFront;

    TPos3f mtx(MR::getJointMtx(this, "BattanBody"));
    TVec3f trans;
    MR::extractMtxTrans(mtx.toMtxPtr(), &trans);
    TVec3f d = *MR::getPlayerCenterPos() - trans;
    TVec3f local(d.x * mtx.mMtx[0][0] + d.y * mtx.mMtx[1][0] + d.z * mtx.mMtx[2][0],
                 d.x * mtx.mMtx[0][1] + d.y * mtx.mMtx[1][1] + d.z * mtx.mMtx[2][1],
                 d.x * mtx.mMtx[0][2] + d.y * mtx.mMtx[1][2] + d.z * mtx.mMtx[2][2]);

    bool inRange = (-0.5f * pRange->x < local.x && local.x < 0.5f * pRange->x) &&
                   (0.0f <= local.y && local.y < pRange->y) && (-0.5f * pRange->z < local.z && local.z < 0.5f * pRange->z);

    mIsPlayerFront = local.z > 0.0f;
    if (!inRange) {
        return false;
    }

    mIsPlayerSide = true;
    MR::extractMtxZDir(MR::getJointMtx(this, "BattanBody"), pDir);
    if (mIsPlayerFront) {
        pDir->scale(0.5f * pRange->z - local.z);
    } else {
        pDir->scale(-0.5f * pRange->z - local.z);
    }

    if (mIsPlayerFront != prevFront) {
        mIsPlayerFront = prevFront;
        return true;
    }
    return local.z < 0.5f * pRange->z;
}

void Battan::updateHitSensor(HitSensor* pSensor) {
    TPos3f mtx;
    if (MR::isEqualSensor(pSensor, this, "HandL")) {
        mtx = TPos3f(MR::getJointMtx(this, "HandL"));
        MR::addTransMtxLocal(mtx.toMtxPtr(), sTable[7]);
    } else if (MR::isEqualSensor(pSensor, this, "HandR")) {
        mtx = TPos3f(MR::getJointMtx(this, "HandR"));
        MR::addTransMtxLocal(mtx.toMtxPtr(), sTable[8]);
    } else {
        return;
    }

    TVec3f pos;
    MR::extractMtxTrans(mtx.toMtxPtr(), &pos);
    *MR::getSensorPos2(pSensor) = pos;

    TVec3f diff = pos - mPosition;
    TVec3f side(0.0f);
    MR::vecKillElement(diff, mGravity, &side);
    f32 len = side.length();
    if (len <= 50.0f) {
        if (!MR::normalizeOrZero(&side, side)) {
            side = mFront;
        }
        TVec3f up = TVec3f(0.0f) - mGravity;
        TVec3f off(side);
        off.scale(100.0f);
        *MR::getSensorPos2(pSensor) = *MR::getSensorPos2(pSensor) + off;
    }
}

void Battan::exeWait() {
    if (MR::isFirstStep(this)) {
        setPartsStateStand();
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

void Battan::exeWalk() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Walk");
        MR::onBind(this);
        MR::vecKillElement(MR::getRailDirection(this), mGravity, &mFront);
        MR::normalize(&mFront, mFront);
        MR::validateClipping(this);
    }
    if (!mTicoStop->tryAttachTouch()) {
        TPos3f mtx;
        TVec3f up = TVec3f(0.0f) - mGravity;
        MR::makeMtxFrontUpPos(&mtx, mFront, up, mPosition);
        MR::addTransMtxLocal(mtx.toMtxPtr(), sTable[3]);
        if (fn_8008E060(&mtx, &sTable[1])) {
            MR::zeroVelocity(this);
            setNerve(GET_BATTAN_NERVE(BattanNrvFind));
            return;
        }
        moveRailStep(0.01f * mWalkSpeed);
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

void Battan::exeTurnEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "TurnEnd");
        MR::offBind(this);
        MR::vecKillElement(MR::getRailDirection(this), mGravity, &mFront);
        MR::normalize(&mFront, mFront);
        MR::resetPosition(this);
    }
    setNerve(GET_BATTAN_NERVE(BattanNrvWalk));
}

void Battan::exeHalfTurn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HalfTurn");
        mIsTurnBack = false;
        MR::validateClipping(this);
        MR::zeroVelocity(this);
        mFrontPrev = mFront;
    }
    if (!mTicoStop->tryAttachTouch()) {
        TVec3f target(0.0f, 0.0f, 1.0f);
        fn_800531B0(&target, this, 0.1f);
        if (!MR::isRailGoingToEnd(this)) {
            target = -target;
        }
        TVec3f killed;
        MR::vecKillElement(target, mGravity, &killed);
        MR::normalize(&target, killed);
        MR::vecBlendSphere(&mFront, mFrontPrev, target, MR::calcNerveRate(this, 0x3C));
        if (MR::isGreaterStep(this, 0x3C)) {
            setNerve(GET_BATTAN_NERVE(BattanNrvWalk));
        }
    }
}

void Battan::exeFind() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Find");
        MR::invalidateClipping(this);
        MR::offBind(this);
    }
    if (!mTicoStop->tryAttachTouch() && MR::isActionEnd(this)) {
        TPos3f mtx;
        TVec3f up = TVec3f(0.0f) - mGravity;
        MR::makeMtxFrontUpPos(&mtx, mFront, up, mPosition);
        MR::addTransMtxLocal(mtx.toMtxPtr(), sTable[3]);
        if (fn_8008E060(&mtx, &sTable[2])) {
            setNerve(GET_BATTAN_NERVE(BattanNrvAttackSign));
            return;
        }
        setNerve(GET_BATTAN_NERVE(BattanNrvRun));
    }
}

void Battan::exeRun() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Run");
        MR::invalidateClipping(this);
    }
    if (!mTicoStop->tryAttachTouch()) {
        TPos3f mtx;
        TVec3f up = TVec3f(0.0f) - mGravity;
        MR::makeMtxFrontUpPos(&mtx, mFront, up, mPosition);
        MR::addTransMtxLocal(mtx.toMtxPtr(), sTable[3]);
        if (fn_8008E060(&mtx, &sTable[0])) {
            TPos3f mtx2;
            MR::makeMtxFrontUpPos(&mtx2, mFront, up, mPosition);
            MR::addTransMtxLocal(mtx2.toMtxPtr(), sTable[3]);
            if (fn_8008E060(&mtx2, &sTable[2])) {
                MR::zeroVelocity(this);
                setNerve(GET_BATTAN_NERVE(BattanNrvAttackSign));
                return;
            }
            moveRailStep(0.01f * mRunSpeed);
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

static bool isTicoStopTouchByte14(const SupportTicoStop* pTico) {
    return *reinterpret_cast<const u8*>(reinterpret_cast<const char*>(pTico) + 0x14) != 0;
}

void Battan::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        MR::invalidateClipping(this);
    }
    if (MR::isLessStep(this, 0x2E) && mTicoStop->tryAttachTouch()) {
        if (isTicoStopTouchByte14(mTicoStop)) {
            setPartsStateAllAppear();
        }
    } else {
        if (mTicoStop->isTouching() && MR::isGreaterStep(this, 0x23)) {
            setPartsStateAttack();
        }
        if (MR::isStep(this, 0x23)) {
            setPartsStateAttack();
        }
        if (MR::isActionEnd(this)) {
            setPartsStateAllAppear();
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

void Battan::exeRiseSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "RiseSign");
        MR::invalidateClipping(this);
        offCollisionPartsAll(this);
    }
    if (!mTicoStop->tryAttachTouch() && MR::isGreaterEqualStep(this, 0x3C)) {
        onCollisionPartsAll(this);
        setPartsStateStand();
        setNerve(GET_BATTAN_NERVE(BattanNrvRise));
    }
}

void Battan::exeRise() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Rise");
        MR::invalidateClipping(this);
    }
    if (MR::isGreaterEqualStep(this, 0x3C) && mTicoStop->tryAttachTouch()) {
        if (isTicoStopTouchByte14(mTicoStop)) {
            setPartsStateStand();
        }
    } else {
        if (mTicoStop->isTouching() && MR::isGreaterStep(this, 0xC)) {
            setPartsStateHalf();
        }
        if (MR::isStep(this, 0xC)) {
            setPartsStateHalf();
        }
        if (MR::isActionEnd(this)) {
            setNerve(GET_BATTAN_NERVE(BattanNrvWait));
        }
    }
}

void Battan::exeDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Down");
        MR::invalidateClipping(this);
    }
    if (MR::isStep(this, 3)) {
        killPartsAll();
    }
    if (MR::isGreaterEqualStep(this, 3) || MR::isOnPlayer(this) || MR::isHiddenModel(this)) {
        if (!MR::isHiddenModel(this)) {
            updateLegs();
            if (mIsDropStarPiece) {
                TVec3f off(mFront);
                off.scale(400.0f);
                TVec3f pos = mPosition + off;
                MR::appearStarPiece(this, pos, 7, 10.0f, 40.0f, false);
            }
            MR::hideModel(this);
            killPartsAll();
            return;
        }
        if (MR::isDead(mBreakModel)) {
            kill();
        }
    }
}

// Functor thunk stored in the joint delegate (fn_8015B640). NONMATCHING: __ptmf_scall is not expressible here.
extern "C" void fn_8015B640(void* pFunctor) {
    typedef void (*Thunk)(void*);
    Thunk pFn = *reinterpret_cast<Thunk*>(reinterpret_cast<char*>(pFunctor) + 4);
    if (pFn != nullptr) {
        pFn(pFunctor);
    }
}

// Builds the joint-controller delegate for the body joint (fn_8015BA20). NONMATCHING: controller vtable is a shared template.
extern "C" BattanJointDelegate* fn_8015BA20(BattanJointDelegate* pDelegate, JointController* pJoint, LiveActor* pActor) {
    pDelegate->mPos.x = 0.0f;
    pDelegate->mPos.y = 0.0f;
    pDelegate->mPos.z = 0.0f;
    pDelegate->mDir.x = 0.0f;
    pDelegate->mDir.y = 1.0f;
    pDelegate->mDir.z = 0.0f;
    pDelegate->_1C[0] = 0;
    pDelegate->_1C[1] = 0;
    pDelegate->_1C[2] = 0;

    JointController* pCtrl = new JointController();
    pDelegate->mController = pCtrl;
    MR::setJointControllerParam(pCtrl, pActor, "BattanBody");
    (void)pJoint;
    return pDelegate;
}

// Sets the delegate direction from a normalized vector, keeping the old one if the input is zero (fn_8015BB30).
extern "C" void fn_8015BB30(BattanJointDelegate* pDelegate, const TVec3f* pDir) {
    TVec3f old(pDelegate->mDir);
    if (MR::normalizeOrZero(&pDelegate->mDir, *pDir)) {
        pDelegate->mDir = old;
    }
}

// Turns the joint matrix toward the delegate direction and places its translation (fn_8015BB90).
extern "C" s32 fn_8015BB90(BattanJointDelegate* pDelegate, TPos3f* pMtx, TVec3f* pOut) {
    (void)pOut;
    MR::turnMtxToXDirRate(pMtx, pDelegate->mDir, 1.0f);
    pMtx->setTrans(pDelegate->mPos);
    return 1;
}
