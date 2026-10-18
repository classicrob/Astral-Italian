#pragma auto_inline off
#include "Game/MapObj/ItemDrill.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/ActorStateDemoAppearance.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util.hpp"
#include "JSystem/JGeometry/TVec.hpp"
#include "JSystem/JMath/JMath.hpp"

class CameraTargetMtx {
public:
    CameraTargetMtx(const char*);
    u8 _[0x94];
};

namespace Collision {
    void addToGlobal(CollisionParts*);
    void removeToGlobal(CollisionParts*);
};  // namespace Collision

namespace MR {
    void* createCollisionPartsFilterDelegator(bool (*)(const CollisionParts*));
    f32 calcDistanceToPlayer(const TVec3f&);
    bool isPlayerInRange(f32, const TVec3f&);
};  // namespace MR

void createDrillHoleHolderSceneObj();

extern "C" void fn_800222A0(const LiveActor*);
extern "C" CollisionParts* fn_80038FB0(const TVec3f*, const TVec3f*);
extern "C" bool fn_80050380();
extern "C" void fn_8004FF70();
extern "C" void fn_802B5800(TMtx34f*, const TVec3f*, const TVec3f*, TVec3f*);

namespace JGeometry {
    void negateInternal(const f32*, f32*);
};  // namespace JGeometry

extern "C" bool fn_802B8500(const CollisionParts*);

namespace NrvItemDrill {
    NEW_NERVE(ItemDrillNrvDrillDiveStart, ItemDrill, DrillDiveStart);
    NEW_NERVE(ItemDrillNrvDrillDiveLoop, ItemDrill, DrillDiveLoop);
    NEW_NERVE(ItemDrillNrvDrilling, ItemDrill, Drillling);
    NEW_NERVE(ItemDrillNrvReverseDrilling, ItemDrill, ReverseDrillling);
    NEW_NERVE(ItemDrillNrvDrillDiveEnd, ItemDrill, DrillDiveEnd);
    NEW_NERVE(ItemDrillNrvDrop, ItemDrill, Drop);
    NEW_NERVE(ItemDrillNrvHide, ItemDrill, Hide);
    NEW_NERVE(ItemDrillNrvReappear, ItemDrill, Reappear);
    NEW_NERVE(ItemDrillNrvGroupHide, ItemDrill, GroupHide);
    NEW_NERVE(ItemDrillNrvTake, ItemDrill, Take);
    NEW_NERVE(ItemDrillNrvTaken, ItemDrill, Taken);

    // Wait and Appear have their own bodies in SMG2 (no exeWait / exeAppear symbols).
    class ItemDrillNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static ItemDrillNrvWait sInstance ATTRIBUTE_WEAK;
    };
    ItemDrillNrvWait ItemDrillNrvWait::sInstance ATTRIBUTE_WEAK;

    class ItemDrillNrvAppear : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static ItemDrillNrvAppear sInstance ATTRIBUTE_WEAK;
    };
    ItemDrillNrvAppear ItemDrillNrvAppear::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvItemDrill

// NONMATCHING: member class types (TPos3f/TVec3f) get default-ctor calls here that the target does not make;
// needs POD-typed members in the header.
ItemDrill::ItemDrill(const char* pName) : LiveActor(pName) {
    mVecF0.x = 0.0f;
    mVecF0.y = 0.0f;
    mVecF0.z = 0.0f;
    mCameraInfo = nullptr;
    mIsArg0 = false;
    mIsArg2 = false;
    mTriangle = nullptr;
    mCameraTargetMtx = nullptr;
    mEffectTrans.x = 0.0f;
    mEffectTrans.y = 0.0f;
    mEffectTrans.z = 0.0f;
    mLimitPos = nullptr;
    mUnk20C = 0;
    mUnk210 = nullptr;
    mDrillDepth = 300.0f;
    mUnk218 = 0;
    mDrillingTime = 0;
    mUnk220 = false;
    mDirs = nullptr;

    mMtxC0.identity();
    mMtxFC.identity();
    mMtx12C.identity();
    mMtx15C.identity();
    mMtx1A8.identity();
    mMtx1D8.identity();

    mTriangle = new Triangle();
}

ItemDrill::~ItemDrill() {
}

void ItemDrill::draw() const {
    if (!MR::isDead(this) && !MR::isClipped(this) && isTakenByPlayer()) {
        fn_800222A0(this);
    }
}

#pragma dont_inline on
void ItemDrill::calcAndSetBaseMtx() {
    if (isNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrop))) {
        mBaseTRMtx.setTrans(mPosition);
    }
    MR::setBaseTRMtx(this, mBaseTRMtx);
}
#pragma dont_inline off

bool ItemDrill::isTakenByPlayer() const {
    bool result = false;
    if (isNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrillDiveLoop)) || isNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrilling)) ||
        isNerve(GET_NERVE(ItemDrill, ItemDrillNrvReverseDrilling)) || isNerve(GET_NERVE(ItemDrill, ItemDrillNrvTaken)) ||
        isNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrillDiveStart)) || isNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrillDiveEnd))) {
        result = true;
    }
    return result;
}

void ItemDrill::incDrillingTimeCheckKillPlayer() {
    mDrillingTime++;
    if (mDrillingTime > 600 || MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        MR::forceKillPlayerByAbyss();
    }
}

void ItemDrill::endCamera() {
    if (mCameraInfo != nullptr) {
        if (MR::isActiveActorCamera(this, mCameraInfo)) {
            MR::endActorCamera(this, mCameraInfo, false, -1);
        }
    }
}

void ItemDrill::endFireDamage() {
    MR::endBindAndPlayerFireDamage(this);
    endCamera();
    if (!isNerve(GET_NERVE(ItemDrill, ItemDrillNrvHide))) {
        setNerve(GET_NERVE(ItemDrill, ItemDrillNrvTaken));
    }
}

void ItemDrill::emitInvalidEffect() {
    MtxPtr pMtx = MR::getJointMtx(this, "DrillPod");
    TMtx34f mtx;
    JMath::gekko_ps_copy12(&mtx, pMtx);
    TVec3f local;
    local.x = 0.0f;
    local.y = -100.0f;
    local.z = 0.0f;
    mtx.mult(local, mEffectTrans);
    MR::emitEffect(this, "Invalid");
}

void ItemDrill::emitReflectEffect() {
    MtxPtr pMtx = MR::getJointMtx(this, "DrillPod");
    TMtx34f mtx;
    JMath::gekko_ps_copy12(&mtx, pMtx);
    TVec3f local;
    local.x = 0.0f;
    local.y = -100.0f;
    local.z = 0.0f;
    mtx.mult(local, mEffectTrans);
    MR::emitEffect(this, "Reflect");
}

#pragma dont_inline on
void ItemDrill::cancelDrill(bool isEffect) {
    if (isEffect) {
        emitInvalidEffect();
        MR::startActionSound(this, "Cancel", -1, -1, -1);
    }
    MR::startLevelSoundPlayer("DrillCancel", -1, -1);
    f32 up[3];
    mBaseTRMtx.getYDir(*reinterpret_cast< TVec3f* >(up));
    MR::endBindAndPlayerForceWeakGravityJumpInputOff(this, *reinterpret_cast< TVec3f* >(up));
    endCamera();
    setNerve(GET_NERVE(ItemDrill, ItemDrillNrvTaken));
}
#pragma dont_inline off

// NONMATCHING: getPlayerPos() / &mDirs[14] evaluation order differs (target computes the pointer before the call).
void ItemDrill::exeTaken() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    MR::makeMtxTR(mBaseTRMtx, mPosition, mRotation);

    if (mDirs != nullptr) {
        for (s32 i = 0; i < 14; i++) {
            mDirs[i].set(mDirs[i + 1]);
        }
        TVec3f* pLast = &mDirs[14];
        if (!MR::calcGravityVectorOrZero(this, *MR::getPlayerPos(), pLast, nullptr, 0)) {
            mDirs[14].set(*MR::getPlayerGravity());
        }
    }
}

bool ItemDrill::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgRockAttack(msg)) {
        MR::sendMsgPush(pSender, pReceiver);
        return false;
    }

    if (MR::isMsgStarPieceReflect(msg)) {
        if (MR::isEqualSensor(pReceiver, this, "Body")) {
            return true;
        }
    }

    if (!MR::isPlayerElementModeYoshi()) {
        return false;
    }

    if (MR::isEqualSensor(pReceiver, this, "Body")) {
        return MR::isMsgPlayerTrample(msg);
    }
    return false;
}

bool ItemDrill::receiveMsgItemGet(HitSensor* pReceiver, HitSensor* pSender) {
    if (MR::isPlayerSwimming()) {
        return false;
    }
    if (MR::isPlayerElementModeYoshi()) {
        return false;
    }
    if (fn_80050380() || MR::isPlayerInRush() || MR::isPlayerRecovering()) {
        return false;
    }
    if (!MR::isEqualSensor(pReceiver, this, "Body")) {
        return false;
    }
    if (!MR::isSensorPlayer(pSender)) {
        return false;
    }
    if (!isNerve(GET_NERVE(ItemDrill, ItemDrillNrvWait)) && !isNerve(GET_NERVE(ItemDrill, ItemDrillNrvReappear))) {
        return false;
    }
    mUnk210 = pSender;
    setNerve(GET_NERVE(ItemDrill, ItemDrillNrvTake));
    MR::curePlayerElementMode();
    return true;
}

#pragma dont_inline on
bool ItemDrill::startDrill() {
    if (!isNerve(GET_NERVE(ItemDrill, ItemDrillNrvTaken))) {
        return false;
    }

    TVec3f playerPos(*MR::getPlayerPos());
    mVecF0.set(playerPos);

    TVec3f gravity;
    if (mDirs == nullptr) {
        if (!MR::calcGravityVectorOrZero(this, *MR::getPlayerPos(), &gravity, nullptr, 0)) {
            gravity.set(*MR::getPlayerGravity());
        }
    } else {
        gravity.set(*mDirs);
    }
    JGeometry::negateInternal(reinterpret_cast< const f32* >(&gravity), reinterpret_cast< f32* >(&gravity));

    TVec3f front;
    MR::getPlayerFrontVec(&front);
    fn_802B5800(&mMtx12C, &gravity, &front, &playerPos);

    mUnk20C = -1;
    if (mLimitPos->adjustPositionByCollision(playerPos, nullptr)) {
        TVec3f scaled(gravity);
        scaled.scale(50.0f);
        TVec3f limitPos(mLimitPos->mPosition);
        PSVECAdd(&limitPos, &scaled, &limitPos);
        TVec3f diff(limitPos);
        PSVECSubtract(&playerPos, &diff, &diff);
        if (!MR::getFirstPolyOnLineToMap(nullptr, nullptr, playerPos, diff)) {
            mMtx12C.setTrans(mLimitPos->mPosition);
            mUnk20C = 0;
        }
    }

    if (!checkDrillStart()) {
        return false;
    }

    JMath::gekko_ps_copy12(&mMtxFC, MR::getPlayerBaseMtx());
    fn_8004FF70();
    mDrillingTime = 0;
    mUnk220 = false;
    setNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrillDiveStart));
    return true;
}
#pragma dont_inline off

bool ItemDrill::checkAimDirection(HitSensor* pFrom, HitSensor* pTo) {
    TVec3f up;
    mMtx12C.getYDir(up);
    JGeometry::negateInternal(reinterpret_cast< const f32* >(&up), reinterpret_cast< f32* >(&up));

    TVec3f dir;
    const TVec3f* pFromPos = MR::getSensorPos(pFrom);
    PSVECSubtract(MR::getSensorPos(pTo), pFromPos, &dir);
    if (MR::normalizeOrZero(&dir)) {
        return false;
    }

    f32 angle = 60.0f;
    if (angle < 0.0f) {
        angle = -angle;
    }
    s32 index = static_cast< s32 >(45.511112f * angle) & 0x3FFF;
    f32 cosVal = *reinterpret_cast< const f32* >(reinterpret_cast< const u8* >(&JMath::sSinCosTable) + index * 8 + 4);
    return cosVal < up.dot(dir);
}

bool ItemDrill::checkAttackFromEnemy(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pSender, this, "Attack")) {
        return false;
    }
    if (!MR::isSensorEnemy(pReceiver)) {
        return false;
    }
    if (isNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrillDiveEnd))) {
        return true;
    }
    bool isDrilling = false;
    if (isNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrillDiveLoop)) || isNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrilling))) {
        isDrilling = true;
    }
    if (isNerve(GET_NERVE(ItemDrill, ItemDrillNrvReverseDrilling))) {
        isDrilling = true;
    }
    if (!isDrilling) {
        return false;
    }
    return checkAimDirection(pSender, pReceiver);
}

bool ItemDrill::checkAttackFromDiver(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pSender, this, "Attack")) {
        return false;
    }
    if (!isNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrillDiveLoop))) {
        return false;
    }
    return checkAimDirection(pSender, pReceiver);
}

void ItemDrill::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (checkAttackFromEnemy(pSender, pReceiver)) {
        if (MR::sendMsgDrillAttack(pReceiver, pSender)) {
            MR::emitEffectHit(this, *MR::getSensorPos(pReceiver), nullptr);
            return;
        }
    }

    if (checkAttackFromDiver(pSender, pReceiver)) {
        if (MR::sendMsgRushEnd(pReceiver, pSender)) {
            cancelDrill(true);
            return;
        }
    }

    bool isDrilling = false;
    if (isNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrillDiveLoop)) || isNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrilling)) ||
        isNerve(GET_NERVE(ItemDrill, ItemDrillNrvReverseDrilling))) {
        isDrilling = true;
    }

    if (isDrilling || isNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrillDiveEnd))) {
        if (MR::isEqualSensor(pSender, this, "Attack") || MR::isEqualSensor(pSender, this, "Player")) {
            MR::tryGetItem(pSender, pReceiver);
            return;
        }
    }

    bool isWaiting = false;
    if (isNerve(GET_NERVE(ItemDrill, ItemDrillNrvWait)) || isNerve(GET_NERVE(ItemDrill, ItemDrillNrvReappear))) {
        isWaiting = true;
    }
    if (!isWaiting) {
        return;
    }
    if (!MR::isSensorPlayerOrRide(pReceiver)) {
        return;
    }

    if (MR::isCurrentRushRockMario() || MR::isPlayerElementModeYoshi()) {
        MR::sendMsgPush(pReceiver, pSender);
    } else if (MR::isPlayerCarryAny() && MR::getPlayerBodySensor() == pReceiver) {
        MR::startActionSound(this, "Disappear", -1, -1, -1);
        setNerve(GET_NERVE(ItemDrill, ItemDrillNrvHide));
    }
}

bool ItemDrill::isOtherTriangleOnLine(Triangle* pTri, const TVec3f& rDir) {
    TVec3f end;
    end.setLength(rDir, 60.0f);
    if (MR::getFirstPolyOnLineToMap(nullptr, pTri, mPosition, end)) {
        return mTriangle != pTri;
    }
    return false;
}

bool ItemDrill::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgItemGet(msg)) {
        if (MR::isPlayerHanging()) {
            return false;
        }
        return receiveMsgItemGet(pReceiver, pSender);
    }

    if (MR::isMsgDamageOrRushDrop(msg) || MR::isMsgWaterDrop(msg)) {
        if (isNerve(GET_NERVE(ItemDrill, ItemDrillNrvTaken))) {
            setNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrop));
            return true;
        }
        return false;
    }

    if (MR::isMsgRushCancel(msg)) {
        TVec3f up;
        mBaseTRMtx.getYDir(up);
        MR::endBindAndPlayerForceWeakGravityJumpInputOff(this, up);
        endCamera();
        setNerve(GET_NERVE(ItemDrill, ItemDrillNrvTaken));
        return true;
    }

    if (MR::isMsgItemRushBegin(msg)) {
        return startDrill();
    }

    if (MR::isMsgReflectV(msg)) {
        if (isNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrillDiveEnd))) {
            emitInvalidEffect();
            TMtx34f tmpL = mMtx15C;
            TMtx34f m1;
            JMath::gekko_ps_copy12(&m1, &tmpL);
            MR::rotateMtxLocalZ(m1, PI);
            MR::stopSceneForDefaultHit(4);
            TMtx34f m2 = m1;
            reflectDrillAt(&m2);
            return true;
        }
    }

    if (MR::isMsgGroupHide(msg)) {
        if (!MR::isEqualSensor(pSender, this, "Body")) {
            setNerve(GET_NERVE(ItemDrill, ItemDrillNrvGroupHide));
        }
    }

    if (MR::isMsgGroupShow(msg)) {
        if (!MR::isEqualSensor(pSender, this, "Body")) {
            if (isNerve(GET_NERVE(ItemDrill, ItemDrillNrvGroupHide))) {
                setNerve(GET_NERVE(ItemDrill, ItemDrillNrvHide));
            }
        }
    }

    if (MR::isMsgUpdateBaseMtx(msg)) {
        MR::setPlayerBaseMtx(mMtxFC);
        return true;
    }

    return false;
}

void ItemDrill::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
        MR::hideModelIfShown(this);
        MR::offBind(this);
        mVelocity.zero();
    }

    if (MR::isGreaterStep(this, 120)) {
        if (!MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
            setNerve(GET_NERVE(ItemDrill, ItemDrillNrvReappear));
        }
    }
}

void ItemDrill::exeGroupHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
        if (!MR::isHiddenModel(this)) {
            MR::emitEffect(this, "HideDrill");
        }
        MR::hideModelIfShown(this);
        mVelocity.zero();
        MR::invalidateHitSensors(this);
    }
}

#pragma dont_inline on
void ItemDrill::exeReappear() {
    if (MR::isFirstStep(this)) {
        MR::showModelIfHidden(this);
        MR::startAction(this, "Appear");
        MR::emitEffect(this, "Replace");
        MR::startActionSound(this, "Replace", -1, -1, -1);

        TMtx34f tmp = mMtxC0;
        JMath::gekko_ps_copy12(&mBaseTRMtx, &tmp);
        mBaseTRMtx.getTrans(mPosition);
    }

    if (MR::isActionEnd(this)) {
        MR::startAction(this, "ItemWait");
        setNerve(GET_NERVE(ItemDrill, ItemDrillNrvWait));
    }
}
#pragma dont_inline off

void ItemDrill::exeReverseDrillling() {
    if (MR::isFirstStep(this)) {
        JMath::gekko_ps_copy12(&mMtx1D8, &mMtx1A8);
        mUnk20C = -1;
    }

    if (MR::isGreaterEqualStep(this, calcDiveStep())) {
        setNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrillDiveEnd));
    }

    MR::startActionSound(this, "Digging", -1, -1, -1);
}

// NONMATCHING: string literals are separate pooled labels (target uses offsets from one lbl_806A3328 string block);
// mDirs init loop differs only in register allocation.
void ItemDrill::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    MR::getJMapInfoArg0NoInit(rIter, &mIsArg0);
    MR::getJMapInfoArg1NoInit(rIter, &mDrillDepth);
    MR::getJMapInfoArg2NoInit(rIter, &mIsArg2);

    JMath::gekko_ps_copy12(&mMtxC0, getBaseMtx());
    JMath::gekko_ps_copy12(&mBaseTRMtx, &mMtxC0);

    MR::addEffectHitNormal(this, nullptr);
    MR::setEffectHostSRT(this, "Invalid", &mEffectTrans, nullptr, nullptr);
    MR::setEffectHostSRT(this, "Reflect", &mEffectTrans, nullptr, nullptr);

    MR::initActorCamera(this, rIter, &mCameraInfo);

    mCameraTargetMtx = new CameraTargetMtx("カメラターゲットダミー");

    createDrillHoleHolderSceneObj();

    mLimitPos = new ItemDrillMovementLimitPos("アイテムドリル移動制限座標");
    MR::copyTransRotate(this, mLimitPos);
    mLimitPos->initWithoutIter();

    MR::joinToGroupArray(this, rIter, "アイテムドリルグループ", 2);

    bool isInitDirs = false;
    MR::getJMapInfoArg3NoInit(rIter, &isInitDirs);
    if (isInitDirs) {
        mDirs = new TVec3f[15];
        for (s32 i = 0; i < 15; i++) {
            mDirs[i].x = 0.0f;
            mDirs[i].y = -1.0f;
            mDirs[i].z = 0.0f;
        }
    }

    initNerve(GET_NERVE(ItemDrill, ItemDrillNrvWait), 1);

    ActorStateDemoAppearance* pAppear = new ActorStateDemoAppearance(this, rIter, "AppearFirst");
    MR::initActorState(this, pAppear, GET_NERVE(ItemDrill, ItemDrillNrvAppear), "AppearFirst");
    MR::startAction(this, "ItemWait");
    setNerve(GET_NERVE(ItemDrill, ItemDrillNrvWait));

    if (MR::useStageSwitchAwake(this, rIter)) {
        makeActorDead();
    } else if (MR::useStageSwitchSyncAppear(this, rIter)) {
        setNerve(GET_NERVE(ItemDrill, ItemDrillNrvAppear));
    }

    MR::registerMarioFaceShipNpc(this, rIter);
}

// ItemDrillMovementLimitPos

ItemDrillMovementLimitPos::ItemDrillMovementLimitPos(const char* pName) : LiveActor(pName) {
    mCollisionParts = nullptr;
    mCollisionPartsFilter = nullptr;
}

ItemDrillMovementLimitPos::~ItemDrillMovementLimitPos() {
}

void ItemDrillMovementLimitPos::init(const JMapInfoIter&) {
    mCollisionPartsFilter = MR::createCollisionPartsFilterDelegator(fn_802B8500);
    makeActorDead();
}

void ItemDrillMovementLimitPos::initAfterPlacement() {
    MR::calcGravity(this);
    mCollisionParts = fn_80038FB0(&mPosition, &mGravity);
}

void ItemDrillMovementLimitPos::control() {
}

bool ItemDrillMovementLimitPos::adjustPositionByCollision(const TVec3f& rPos, const TVec3f* pDir) {
    if (mCollisionParts == nullptr) {
        return false;
    }

    bool isMoved = false;
    MR::setPosition(this, rPos);
    Collision::addToGlobal(mCollisionParts);

    TVec3f pushed;
    for (s32 i = 0; i < 4; i++) {
        if (!checkMapStrikePushback(&pushed, mPosition, pDir)) {
            break;
        }
        PSVECAdd(&mPosition, &pushed, &mPosition);
        isMoved = true;
    }

    Collision::removeToGlobal(mCollisionParts);
    return isMoved;
}

// CollisionParts has no full definition in the repo yet; field at 0xE0.
extern "C" bool fn_802B8500(const CollisionParts* pParts) {
    return *reinterpret_cast< const s32* >(reinterpret_cast< const u8* >(pParts) + 0xE0) != 3;
}

bool MR::isPlayerInRange(f32 dist, const TVec3f& rPos) {
    return calcDistanceToPlayer(rPos) <= dist;
}

namespace NrvItemDrill {
    void ItemDrillNrvWait::execute(Spine* pSpine) const {
        ItemDrill* actor = reinterpret_cast< ItemDrill* >(pSpine->mExecutor);
        if (MR::isInWater(actor, TVec3f(0.0f, 0.0f, 0.0f))) {
            actor->setNerve(GET_NERVE(ItemDrill, ItemDrillNrvHide));
        }
    }

    void ItemDrillNrvAppear::execute(Spine* pSpine) const {
        ItemDrill* actor = reinterpret_cast< ItemDrill* >(pSpine->mExecutor);
        if (MR::updateActorState(actor)) {
            MR::startAction(actor, "ItemWait");
            actor->setNerve(GET_NERVE(ItemDrill, ItemDrillNrvWait));
        }
    }
}  // namespace NrvItemDrill
