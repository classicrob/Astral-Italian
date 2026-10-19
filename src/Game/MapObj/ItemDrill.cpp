#pragma auto_inline off
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/MapObj/ItemDrill.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/ActorStateDemoAppearance.hpp"
#include "Game/Camera/CameraTargetMtx.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util.hpp"
#include "Game/Util/CollisionPartsFilter.hpp"
#include "Game/MapObj/CollectTicoHolder.hpp"
#include "Game/MapObj/DrillHole.hpp"
#include "JSystem/JGeometry/TVec.hpp"
#include "JSystem/JMath/JMath.hpp"


namespace {
    void makeDrillMtx(TPos3f* pMtx, const TVec3f& rUp, const TVec3f& rFront, const TVec3f& rPosition) {
        if (MR::isSameDirection(rUp, rFront, 0.01f)) {
            MR::makeMtxUpNoSupport(pMtx, rUp);
        } else {
            MR::makeMtxUpFront(pMtx, rUp, rFront);
        }
        pMtx->setTrans(rPosition);
    }
}

namespace NrvItemDrill {
    // Wait and Appear have their own bodies in SMG2 (no exeWait / exeAppear symbols).
    class ItemDrillNrvAppear : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static ItemDrillNrvAppear sInstance ATTRIBUTE_WEAK;
    };
    ItemDrillNrvAppear ItemDrillNrvAppear::sInstance ATTRIBUTE_WEAK;

    class ItemDrillNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static ItemDrillNrvWait sInstance ATTRIBUTE_WEAK;
    };
    ItemDrillNrvWait ItemDrillNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(ItemDrillNrvTake, ItemDrill, Take);
    NEW_NERVE(ItemDrillNrvTaken, ItemDrill, Taken);
    NEW_NERVE(ItemDrillNrvDrillDiveStart, ItemDrill, DrillDiveStart);
    NEW_NERVE(ItemDrillNrvDrillDiveLoop, ItemDrill, DrillDiveLoop);
    NEW_NERVE(ItemDrillNrvDrilling, ItemDrill, Drillling);
    NEW_NERVE(ItemDrillNrvReverseDrilling, ItemDrill, ReverseDrillling);
    NEW_NERVE(ItemDrillNrvDrillDiveEnd, ItemDrill, DrillDiveEnd);
    NEW_NERVE(ItemDrillNrvDrop, ItemDrill, Drop);
    NEW_NERVE(ItemDrillNrvHide, ItemDrill, Hide);
    NEW_NERVE(ItemDrillNrvReappear, ItemDrill, Reappear);
    NEW_NERVE(ItemDrillNrvGroupHide, ItemDrill, GroupHide);
}  // namespace NrvItemDrill

// Local-space offset of the drill tip (constructed in __sinit_ after the nerves).
static TVec3f sDrillTipOffset(0.0f, 14.0f, 2.0f);

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

    mBaseTRMtx.identity();
    mMtxC0.identity();
    mMtxFC.identity();
    mMtx12C.identity();
    mMtx15C.identity();
    mMtx1A8.identity();
    mMtx1D8.identity();

    mTriangle = new Triangle();
}

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
            TVec3f* pDir = &mDirs[i];
            pDir->x = 0.0f;
            pDir->y = -1.0f;
            pDir->z = 0.0f;
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

void ItemDrill::draw() const {
    if (!MR::isDead(this) && !MR::isClipped(this) && isTakenByPlayer()) {
        MR::drawModelAlphaShadow(this);
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

    if (MR::isEqualSensor(pReceiver, this, "Body") == false) {
        return false;
    }
    return MR::isMsgPlayerTrample(msg);
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

bool ItemDrill::receiveMsgItemGet(HitSensor* pReceiver, HitSensor* pSender) {
    if (MR::isPlayerSwimming()) {
        return false;
    }
    if (MR::isPlayerElementModeYoshi()) {
        return false;
    }
    bool isBusy = false;
    if (MR::isPlayerLifeDecreasedThisFrame() || MR::isPlayerInRush() || MR::isPlayerRecovering()) {
        isBusy = true;
    }
    if (isBusy) {
        return false;
    }
    if (!MR::isEqualSensor(pReceiver, this, "Body")) {
        return false;
    }
    if (!MR::isSensorPlayer(pSender)) {
        return false;
    }
    bool isWaiting = false;
    if (isNerve(GET_NERVE(ItemDrill, ItemDrillNrvWait)) || isNerve(GET_NERVE(ItemDrill, ItemDrillNrvReappear))) {
        isWaiting = true;
    }
    if (!isWaiting) {
        return false;
    }
    mUnk210 = pSender;
    setNerve(GET_NERVE(ItemDrill, ItemDrillNrvTake));
    MR::curePlayerElementMode();
    return true;
}

bool ItemDrill::startDrill() {
    if (!isNerve(GET_NERVE(ItemDrill, ItemDrillNrvTaken))) {
        return false;
    }

    TVec3f gravity;
    TVec3f front;
    TVec3f playerPos(*MR::getPlayerPos());
    mVecF0.set(playerPos);

    if (mDirs == nullptr) {
        if (!MR::calcGravityVectorOrZero(this, *MR::getPlayerPos(), &gravity, nullptr, 0)) {
            gravity.set(*MR::getPlayerGravity());
        }
    } else {
        gravity.set(*mDirs);
    }
    JGeometry::negateInternal(reinterpret_cast< const f32* >(&gravity), reinterpret_cast< f32* >(&gravity));

    MR::getPlayerFrontVec(&front);
    makeDrillMtx(&mMtx12C, gravity, front, playerPos);

    bool isAdjusted = mLimitPos->adjustPositionByCollision(playerPos, nullptr);
    mUnk20C = -1;
    if (isAdjusted) {
        TVec3f scaled(gravity);
        scaled.scale(50.0f);
        TVec3f limitPos(mLimitPos->mPosition);
        JMathInlineVEC::PSVECAdd(&limitPos, &scaled, &limitPos);
        TVec3f diff(limitPos);
        JMathInlineVEC::PSVECSubtract(&playerPos, &diff, &diff);
        if (!MR::getFirstPolyOnLineToMap(nullptr, nullptr, playerPos, diff)) {
            mMtx12C.setTrans(mLimitPos->mPosition);
            mUnk20C = 0;
        }
    }

    if (!checkDrillStart()) {
        return false;
    }

    JMath::gekko_ps_copy12(&mMtxFC, MR::getPlayerBaseMtx());
    MR::enablePlayerSilhouetteDuringRush();
    mDrillingTime = 0;
    mUnk220 = false;
    setNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrillDiveStart));
    return true;
}

void ItemDrill::endFireDamage() {
    MR::endBindAndPlayerFireDamage(this);
    endCamera();
    if (!isNerve(GET_NERVE(ItemDrill, ItemDrillNrvHide))) {
        setNerve(GET_NERVE(ItemDrill, ItemDrillNrvTaken));
    }
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


bool ItemDrill::checkDrillStart() {
    TVec3f start;
    mMtx12C.mult(TVec3f(0.0f, 100.0f, 0.0f), start);

    TVec3f down;
    mMtx12C.getYDir(down);
    down.setLength(-5000.0f);

    TVec3f hit;
    if (MR::getFirstPolyOnLineToMap(&hit, mTriangle, start, down)) {
        TVec3f normal;
        normal.set(*mTriangle->getFaceNormal());
        TVec3f zdir;
        mMtx12C.getZDir(zdir);
        makeDrillMtx(&mMtx1A8, normal, zdir, hit);

        if (mIsArg0) {
            JMath::gekko_ps_copy12(&mMtx15C, &mMtx1A8);
            TVec3f trans;
            mMtx12C.getTrans(trans);
            JMath::gekko_ps_copy12(&mMtx12C, &mMtx1A8);
            mMtx12C.setTrans(trans);
        } else {
            JMath::gekko_ps_copy12(&mMtx15C, &mMtx12C);
            mMtx15C.setTrans(hit);
        }
        return true;
    }
    return false;
}

bool ItemDrill::tryAlignToTriangle(Triangle* pTri, const TVec3f& rDir) {
    f32 limit = 30.0f * static_cast< f32 >(getNerveStep());
    f32 dist = (mDrillDepth >= limit) ? limit : mDrillDepth;

    TVec3f start;
    start.setLength(rDir, 60.0f);
    JMathInlineVEC::PSVECAdd(&start, &mPosition, &start);

    TVec3f back;
    back.setLength(rDir, -dist);

    TVec3f down;
    down.setLength(rDir, mDrillDepth);

    TVec3f hit;
    if (MR::getFirstPolyOnLineToMap(&hit, pTri, start, back)) {
        if (!MR::getFirstPolyOnLineToMap(nullptr, nullptr, hit, down)) {
            TVec3f normal;
            normal.set(*pTri->getFaceNormal());
            TVec3f zdir;
            mMtx12C.getZDir(zdir);
            makeDrillMtx(&mMtx1D8, normal, zdir, hit);
            JMath::gekko_ps_copy12(&mMtx15C, &mMtx12C);
            mMtx15C.setTrans(hit);
            return true;
        }
    }
    return false;
}

bool ItemDrill::isOtherTriangleOnLine(Triangle* pTri, const TVec3f& rDir) {
    TVec3f end;
    end.setLength(rDir, 60.0f);
    if (MR::getFirstPolyOnLineToMap(nullptr, pTri, mPosition, end)) {
        return pTri != mTriangle;
    }
    return false;
}

bool ItemDrill::updateDrillMove(const TVec3f& rMove) {
    TVec3f dir;
    TVec3f camPos;
    TVec3f center;

    bool result = false;
    s32 counter = mUnk20C;
    if (counter >= 0) {
        s32 next = counter + 1;
        mUnk20C = next;
        s32 maxCount = 30;
        f32 blend = static_cast< f32 >(next) / static_cast< f32 >(maxCount);
        f32 t;
        if (blend < 0.0f) {
            t = 0.0f;
        } else if (blend > 1.0f) {
            t = 1.0f;
        } else {
            t = blend;
        }

        JMathInlineVEC::PSVECAdd(&mVecF0, &rMove, &mVecF0);

        bool moved;
        if (MR::normalizeOrZero(&dir, rMove) == false) {
            TVec3f pos1(mPosition);
            JMathInlineVEC::PSVECAdd(&pos1, &rMove, &pos1);
            moved = mLimitPos->adjustPositionByCollision(pos1, &dir);
        } else {
            TVec3f pos2(mPosition);
            JMathInlineVEC::PSVECAdd(&pos2, &rMove, &pos2);
            moved = mLimitPos->adjustPositionByCollision(pos2, nullptr);
        }

        if (moved) {
            JMAVECLerp(&mVecF0, &mLimitPos->mPosition, &mPosition, t);
        } else if (isNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrillDiveLoop))) {
            mMtx12C.setTrans(mVecF0);
            mUnk20C = -1;
            if (checkDrillStart()) {
                mUnk218 = 0;
                JMathInlineVEC::PSVECAdd(&mPosition, &rMove, &mPosition);
            } else {
                result = true;
            }
        } else {
            mUnk20C = -1;
            JMathInlineVEC::PSVECAdd(&mPosition, &rMove, &mPosition);
        }
    } else {
        JMathInlineVEC::PSVECAdd(&mPosition, &rMove, &mPosition);
    }

    mBaseTRMtx.setTrans(mPosition);
    JMath::gekko_ps_copy12(&mMtxFC, MR::getJointMtx(this, "MarioPosition"));

    if (MR::isActiveActorCamera(this, mCameraInfo)) {
        mCameraTargetMtx->mMatrix.getTrans(camPos);
        center.set(*MR::getPlayerCenterPos());
        if (40.0f < PSVECDistance(&camPos, &center)) {
            TVec3f diff(center);
            JMathInlineVEC::PSVECSubtract(&diff, &camPos, &diff);
            MR::normalize(&center, diff);
            JMAVECScaleAdd(&center, &camPos, &center, 40.0f);
        }
        mCameraTargetMtx->mMatrix.setTrans(center);
    }
    return result;
}

s32 ItemDrill::calcDiveStep() {
    TVec3f from;
    mMtx12C.getTrans(from);
    TVec3f to;
    mMtx15C.getTrans(to);
    s32 steps = static_cast< s32 >(PSVECDistance(&from, &to) / 30.0f) + 1;

    TVec3f delta(to);
    JMathInlineVEC::PSVECSubtract(&delta, &from, &delta);
    TVec3f step(delta);
    step.scale(1.0f / static_cast< f32 >(steps));

    if (updateDrillMove(step)) {
        return -1;
    }
    return steps;
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

void ItemDrill::alignDrillMtx(const TPos3f& rMtx, const TVec3f& rDir) {
    TPos3f tmp;
    TPos3f copy;
    TVec3f up;
    TVec3f neg;
    TVec3f a;
    TVec3f b;

    rMtx.getYDir(up);
    JGeometry::negateInternal(reinterpret_cast< const f32* >(&rDir), reinterpret_cast< f32* >(&neg));
    a.set(up);
    PSVECMag(&a);
    PSVECNormalize(&a, &a);
    b.set(neg);
    PSVECMag(&b);
    PSVECNormalize(&b, &b);
    JMAVECScaleAdd(&b, &up, &up, -2.0f * a.dot(b));
    PSVECMag(&up);
    PSVECNormalize(&up, &up);

    JMath::gekko_ps_copy12(&tmp, &rMtx);
    MR::turnMtxToYDirRate(&tmp, up, 1.0f);
    copy = tmp;
    reflectDrillAt(&copy);
}

void ItemDrill::reflectDrillAt(TMtx34f* pMtx) {
    JMath::gekko_ps_copy12(&mMtx12C, pMtx);
    TVec3f pos;
    f32 depth = 350.0f;
    TVec3f offset;
    offset.x = 0.0f;
    offset.y = -depth;
    offset.z = 0.0f;
    mMtx12C.mult(offset, pos);
    mMtx12C.setTrans(pos);

    TPos3f copy = mMtx12C;
    JMath::gekko_ps_copy12(&mBaseTRMtx, &copy);
    mBaseTRMtx.getTrans(mPosition);

    MR::startBckPlayer("DrillDiveLoop");
    MR::startAction(this, "DrillDiveLoop");
    MR::tryRumblePadStrong(this, 0);
    MR::startActionSound(this, "BmTspiderBound", -1, -1, -1);
    if (mCameraInfo != nullptr && !MR::isActiveActorCamera(this, mCameraInfo)) {
        MR::startActorCameraTargetMtx(this, mCameraInfo, mCameraTargetMtx, -1);
    }
    mUnk220 = true;
    mUnk20C = -1;
    setNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrilling));
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
    if (isNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrillDiveLoop)) || isNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrilling)) ||
        isNerve(GET_NERVE(ItemDrill, ItemDrillNrvReverseDrilling))) {
        isDrilling = true;
    }
    if (isDrilling) {
        return checkAimDirection(pSender, pReceiver);
    }
    return false;
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


bool ItemDrill::checkAimDirection(HitSensor* pFrom, HitSensor* pTo) {
    TVec3f up;
    mMtx12C.getYDir(up);
    JGeometry::negateInternal(reinterpret_cast< const f32* >(&up), reinterpret_cast< f32* >(&up));

    TVec3f dir;
    const TVec3f* pFromPos = MR::getSensorPos(pFrom);
    JMathInlineVEC::PSVECSubtract(MR::getSensorPos(pTo), pFromPos, &dir);
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

void ItemDrill::exeTake() {
    if (MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        setNerve(GET_NERVE(ItemDrill, ItemDrillNrvHide));
    }

    if (!MR::sendMsgTaken(mUnk210, getSensor("Body"))) {
        MR::startAction(this, "ItemWait");
        setNerve(GET_NERVE(ItemDrill, ItemDrillNrvWait));
        return;
    }

    MR::tryRumblePadMiddle(this, 0);
    MR::startActionSound(this, "Get", -1, -1, -1);
    MR::sendMsgGroupHideToMember(this, getSensor("Body"), "Body");

    if (mDirs != nullptr) {
        TVec3f gravity(0.0f, -1.0f, 0.0f);
        if (!MR::calcGravityVectorOrZero(this, *MR::getPlayerPos(), &gravity, nullptr, 0)) {
            gravity.set(*MR::getPlayerGravity());
        }
        for (s32 i = 0; i < 15; i++) {
            mDirs[i].set(gravity);
        }
    }

    setNerve(GET_NERVE(ItemDrill, ItemDrillNrvTaken));
}

// NONMATCHING: saved register holding the player-gravity destination differs.
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
        TVec3f* pPlayerPos = MR::getPlayerPos();
        if (!MR::calcGravityVectorOrZero(this, *pPlayerPos, pLast, nullptr, 0)) {
            pLast->set(*MR::getPlayerGravity());
        }
    }
}

void ItemDrill::exeDrillDiveStart() {
    if (MR::isFirstStep(this)) {
        TPos3f a = mMtx12C;
        a.setTrans(mVecF0);
        TPos3f b = a;
        JMath::gekko_ps_copy12(&mBaseTRMtx, &b);
        mBaseTRMtx.getTrans(mPosition);

        MR::startBckPlayer("DrillDiveStart");
        MR::startAction(this, "DrillDiveStart");
        MR::startLevelSoundPlayer("DrillStart", -1, -1);
        MR::setSensorRadius(this, "Player", 250.0f);
    }

    JMath::gekko_ps_copy12(&mMtxFC, MR::getJointMtx(this, "MarioPosition"));
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrillDiveLoop));
    }
}

void ItemDrill::exeDrillDiveLoop() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("DrillDiveLoop");
        MR::startAction(this, "DrillDiveLoop");
        mUnk218 = 0;
    }

    s32 steps = calcDiveStep();
    if (steps < 0) {
        cancelDrill(false);
        return;
    }

    incDrillingTimeCheckKillPlayer();
    s32 count = mUnk218;
    mUnk218 = count + 1;
    if (steps <= count) {
        if (MR::isGroundCodeDamageFire(mTriangle)) {
            endFireDamage();
            return;
        }
        if (MR::isGroundCodeNoDig(mTriangle)) {
            cancelDrill(true);
            return;
        }

        HitSensor* pBody = getSensor("Body");
        if (MR::sendMsgDrillAttack(MR::getSensor(mTriangle), pBody)) {
            TVec3f up;
            mBaseTRMtx.getYDir(up);
            MR::endBindAndPlayerForceWeakGravityJumpInputOff(this, up);
            endCamera();
            setNerve(GET_NERVE(ItemDrill, ItemDrillNrvTaken));
            return;
        }

        if (mCameraInfo != nullptr) {
            MR::startActorCameraTargetMtx(this, mCameraInfo, mCameraTargetMtx, -1);
            JMath::gekko_ps_copy12(&mCameraTargetMtx->mMatrix, &MR::getCameraInvViewMtx());
            TPos3f* pCamPos = &mCameraTargetMtx->mMatrix;
            pCamPos->setTrans(*MR::getPlayerCenterPos());
        }

        if (!MR::isInPlaneModeCube(mPosition)) {
            appearDrillHoleFromArea1(&mMtx1A8);
        }

        MR::emitEffect(this, "Dive");
        JMath::gekko_ps_copy12(&mMtx12C, &mMtx15C);
        mDrillingTime = 0;
        MR::startActionSound(this, "DigStart", -1, -1, -1);
        MR::startLevelSoundPlayer("DrillDigStart", -1, -1);
        setNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrilling));
    }
}

void ItemDrill::exeDrillling() {
    Triangle hitTri;
    TPos3f sA;
    TPos3f sB;
    TPos3f sE;
    TMtx34f sD;
    TMtx34f sC;
    TVec3f dir;

    mMtx12C.getYDir(dir);
    f32 speed = 30.0f;
    dir.scale(-speed);
    updateDrillMove(dir);
    MR::startActionSound(this, "Digging", -1, -1, -1);
    incDrillingTimeCheckKillPlayer();

    if (tryAlignToTriangle(&hitTri, dir)) {
        if (!mUnk220 && MR::isGroundCodeNoDig(&hitTri)) {
            bool wasDone = mUnk220;
            sA = mMtx12C;
            emitReflectEffect();
            sB = mMtx15C;
            JMath::gekko_ps_copy12(&sC, &sB);
            MR::rotateMtxLocalZ(sC, PI);
            MR::stopSceneForDefaultHit(4);
            sD = sC;
            reflectDrillAt(&sD);
            if (!wasDone) {
                JMath::gekko_ps_copy12(&mMtx15C, &sA);
                MR::rotateMtxLocalZ(mMtx15C, PI);
                setNerve(GET_NERVE(ItemDrill, ItemDrillNrvReverseDrilling));
            }
        } else {
            setNerve(GET_NERVE(ItemDrill, ItemDrillNrvDrillDiveEnd));
        }
    } else if (mIsArg2) {
        if (isOtherTriangleOnLine(&hitTri, dir) && MR::isGroundCodeNoDig(&hitTri)) {
            emitReflectEffect();
            sE = mBaseTRMtx;
            alignDrillMtx(sE, *hitTri.getFaceNormal());
        }
    }
}


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

void ItemDrill::exeDrillDiveEnd() {
    if (MR::isFirstStep(this)) {
        TPos3f sX = mMtx15C;
        MR::rotateMtxLocalZ(sX, PI);
        TPos3f sY = sX;
        JMath::gekko_ps_copy12(&mBaseTRMtx, &sY);
        mBaseTRMtx.getTrans(mPosition);

        MR::startBckPlayer("DrillDiveEnd");
        MR::startAction(this, "DrillDiveEnd");
        MR::startLevelSoundPlayer("DrillEnd", -1, -1);
        MR::emitEffect(this, "Appear");
        MR::setSensorRadius(this, "Player", 150.0f);
        endCamera();
        if (!MR::isInPlaneModeCube(mPosition)) {
            appearDrillHoleFromArea2(&mMtx1D8);
        }
    }

    JMath::gekko_ps_copy12(&mMtxFC, MR::getJointMtx(this, "MarioPosition"));
    if (MR::isStep(this, 30)) {
        MR::invalidateHitSensor(this, "Attack");
    }

    if (MR::isActionEnd(this)) {
        MR::launchRescuedStrayTico(*&mMtx1D8);
        TVec3f up;
        mBaseTRMtx.getYDir(up);
        MR::endBindAndPlayerForceWeakGravityJumpInputOff(this, up);
        endCamera();
        setNerve(GET_NERVE(ItemDrill, ItemDrillNrvTaken));
    }
}

void ItemDrill::exeDrop() {
    TVec3f front;
    TVec3f negGrav;
    TVec3f negFront;

    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Drop");
        if (!MR::isPowerStarGetDemoActive()) {
            MR::startLevelSoundPlayer("MorphEnd", -1, -1);
        }
        MR::onBind(this);
        MR::getPlayerFrontVec(&front);
        mBaseTRMtx.identity();
        JGeometry::negateInternal(reinterpret_cast< const f32* >(&front), reinterpret_cast< f32* >(&negFront));
        JGeometry::negateInternal(reinterpret_cast< const f32* >(&mGravity), reinterpret_cast< f32* >(&negGrav));
        if (MR::isSameDirection(negGrav, negFront, 0.01f)) {
            MR::makeMtxUpNoSupport(&mBaseTRMtx, negGrav);
        } else {
            MR::makeMtxUpFront(&mBaseTRMtx, negGrav, negFront);
        }
        mBaseTRMtx.mult(sDrillTipOffset, mVelocity);
    }

    TVec3f g(mGravity);
    g.scale(0.8f);
    JMathInlineVEC::PSVECAdd(&mVelocity, &g, &mVelocity);

    static const f32 sReboundParam[3] = {0.5f, 0.0f, 0.5f};
    MR::reboundVelocityFromCollision(this, sReboundParam[0], sReboundParam[1], sReboundParam[2]);
    if (MR::isActionEnd(this)) {
        MR::sendMsgGroupShowToMember(this, getSensor("Body"), "Body");
        setNerve(GET_NERVE(ItemDrill, ItemDrillNrvHide));
    }
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

ItemDrill::~ItemDrill() {
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

bool ItemDrill::isTargetCollisionParts(const CollisionParts* pParts) {
    return pParts->mKeeperIndex != 3;
}

// ItemDrillMovementLimitPos

ItemDrillMovementLimitPos::ItemDrillMovementLimitPos(const char* pName) : LiveActor(pName) {
    mCollisionParts = nullptr;
    mCollisionPartsFilter = nullptr;
}

void ItemDrillMovementLimitPos::init(const JMapInfoIter&) {
    mCollisionPartsFilter = static_cast<CollisionPartsFilterBase*>(MR::createCollisionPartsFilterDelegator(ItemDrill::isTargetCollisionParts));
    makeActorDead();
}

void ItemDrillMovementLimitPos::initAfterPlacement() {
    MR::calcGravity(this);
    mCollisionParts = MR::findMoveLimitCollision(mPosition, mGravity);
}

bool ItemDrillMovementLimitPos::adjustPositionByCollision(const TVec3f& rPos, const TVec3f* pDir) {
    if (mCollisionParts == nullptr) {
        return false;
    }

    TVec3f* pPos;
    bool isMoved = false;
    MR::setPosition(this, rPos);
    Collision::addToGlobal(mCollisionParts);

    TVec3f pushed;
    pPos = &mPosition;
    for (s32 i = 0; i < 4; i++) {
        if (!checkMapStrikePushback(&pushed, mPosition, pDir)) {
            break;
        }
        JMathInlineVEC::PSVECAdd(pPos, &pushed, pPos);
        isMoved = true;
    }

    Collision::removeToGlobal(mCollisionParts);
    return isMoved;
}

void ItemDrillMovementLimitPos::control() {
}

// NONMATCHING: vector negation is outlined and temporary registers differ.
bool ItemDrillMovementLimitPos::checkMapStrikePushback(TVec3f* pOut, const TVec3f& rPos, const TVec3f* pDir) {
    u32 count = Collision::checkStrikeBallToMapWithThickness(rPos, 130.0f, 140.0f,
                                                             mCollisionPartsFilter, nullptr);
    if (count == 0) {
        return false;
    }

    TVec3f maxV(0, 0, 0);
    TVec3f minV(0, 0, 0);
    for (u32 i = 0; i < count; i++) {
        const HitInfo* pInfo = Collision::getStrikeInfoMap(i);
        TVec3f n(*pInfo->mParentTriangle.getNormal(0));
        if (pDir != nullptr) {
            f32 dot = pDir->dot(n);
            f32 angle = 60.0f;
            if (angle < 0.0f) {
                angle = -angle;
            }
            s32 index = static_cast< s32 >(45.511112f * angle);
            f32 cosVal = *reinterpret_cast< const f32* >(reinterpret_cast< const u8* >(&JMath::sSinCosTable) + ((index * 8) & 0x1FFF8) + 4);
            if (dot < -cosVal || cosVal < dot) {
                continue;
            }
        }

        f32 k = pInfo->mDistance;
        k *= 1.1f;
        f32 vx = n.x * k;
        if (maxV.x < vx) {
            maxV.x = vx;
        } else if (vx < minV.x) {
            minV.x = vx;
        }
        f32 vy = n.y * k;
        if (maxV.y < vy) {
            maxV.y = vy;
        } else if (vy < minV.y) {
            minV.y = vy;
        }
        f32 vz = n.z * k;
        if (maxV.z < vz) {
            maxV.z = vz;
        } else if (vz < minV.z) {
            minV.z = vz;
        }
    }

    pOut->x = maxV.x;
    pOut->y = maxV.y;
    pOut->z = maxV.z;
    JMathInlineVEC::PSVECAdd(pOut, &minV, pOut);
    if (MR::isNearZero(*pOut, 0.001f)) {
        return false;
    }

    const HitInfo* pInfo0 = Collision::getStrikeInfoMap(0);
    TVec3f hit;
    TVec3f newPos;
    TVec3f n0(*pInfo0->mParentTriangle.getNormal(0));
    JMathInlineVEC::PSVECAdd(&rPos, pOut, &newPos);
    TVec3f negN;
    negN.negate(n0);
    if (MR::checkHitSemilinePlane(&hit, newPos, negN, *pInfo0->mParentTriangle.getPos(0), n0)) {
        if (!(PSVECDistance(&newPos, &hit) < 100.0f)) {
            return true;
        }
    }

    return false;
}

ItemDrillMovementLimitPos::~ItemDrillMovementLimitPos() {
}

bool MR::isPlayerInRange(f32 dist, const TVec3f& rPos) {
    return calcDistanceToPlayer(rPos) <= dist;
}
