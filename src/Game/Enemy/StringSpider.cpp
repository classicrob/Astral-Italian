#pragma auto_inline off
#include "Game/Enemy/StringSpider.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/Util/YoshiLockOnUtil.hpp"
#include "Game/Enemy/SpinHitController.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/mtx.h>
#include <revolution/types.h>

namespace {
    static const f32 hSwing1SeMinSpeed = 2.0f;
    static const f32 hSwing1SeMaxSpeed = 12.0f;
    static const f32 hSwing1SeMaxVolume = 0.0f;
    static const f32 hSwing2SeMinSpeed = 5.0f;
    static const f32 hSwing2SeMaxSpeed = 25.0f;
    static const f32 hSwing2SeMaxVolume = 0.0f;
    static const f32 hCollisionSePlaySpeed = 15.0f;
    static const f32 hCollisionSeMaxVolumeSpeed = 100.0f;
    static const f32 hSpinSeMinVolume = 5.0f;
    static const f32 hSpinSeMinNoteLength = 0.0f;
    static const f32 hSpinSeMaxNoteLength = 1.0f;
    static const f32 hSpinSeLimitMinVolume = 40.0f;
};  // namespace

namespace NrvStringSpider {
    NEW_NERVE_ONEND(HostTypeNrvWait, StringSpider, Wait, Wait);
    NEW_NERVE(HostTypeNrvDownStart, StringSpider, DownStart);
    NEW_NERVE(HostTypeNrvDownEnd, StringSpider, DownEnd);
    NEW_NERVE(HostTypeNrvDown, StringSpider, Down);
    NEW_NERVE(HostTypeNrvUp, StringSpider, Up);
    NEW_NERVE(HostTypeNrvSearch, StringSpider, Search);
    NEW_NERVE(HostTypeNrvAttack, StringSpider, Attack);
    NEW_NERVE(HostTypeNrvHit, StringSpider, Hit);
    NEW_NERVE(HostTypeNrvSpinned, StringSpider, Spinned);
    NEW_NERVE(HostTypeNrvBlow, StringSpider, Blow);
    class HostTypeNrvEaten : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static HostTypeNrvEaten sInstance;
    };
    HostTypeNrvEaten HostTypeNrvEaten::sInstance ATTRIBUTE_WEAK;
    class HostTypeNrvDpdGripped : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            MR::updateActorStateAndNextNerve(static_cast<StringSpider*>(spine->mExecutor), GET_NERVE(StringSpider, HostTypeNrvSearch));
        }
        static HostTypeNrvDpdGripped sInstance;
    };
    HostTypeNrvDpdGripped HostTypeNrvDpdGripped::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvStringSpider

StringSpider::StringSpider(const char* pName)
    : LiveActor(pName), mSpinHitController(nullptr), mScaleController(nullptr), mBindState(nullptr), mString(nullptr), mFixedPos(nullptr),
      mYoshiLockOn(nullptr), mPoseQuat(0, 0, 0, 1), mInitPoseQuat(0, 0, 0, 1), mMaxLength(300.0f), mCurrentLength(300.0f) {
}

void StringSpider::init(const JMapInfoIter& rIter) {
    if (MR::isValidInfo(rIter)) {
        s32 length = -1;
        MR::getJMapInfoArg0NoInit(rIter, &length);
        if (length != -1) {
            f32 value = length;
            mMaxLength = value;
            mCurrentLength = 0.333f * value;
        }
    }
    MR::initActor(this, rIter, "StringSpider", false);
    MR::makeQuatFromRotate(&mPoseQuat, this);
    mInitPoseQuat = mPoseQuat;
    MR::addEffectHitNormal(this, nullptr);
    mSpinHitController = new SpinHitController(this);
    mScaleController = new AnimScaleController(nullptr);
    initNerve(GET_NERVE(StringSpider, HostTypeNrvWait), 1);
    mBindState = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mBindState, GET_NERVE(StringSpider, HostTypeNrvDpdGripped), "SupportFreeze");
    MR::onCalcGravity(this);
    mFixedPos = new FixedPosition((MtxPtr)nullptr, TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, (MtxPtr)nullptr, TVec3f(0.0f));
    mString = new StringSpiderString("ストリングスパイダーの糸");
    MR::resetPosition(mString, mPosition);
    mString->makeActorAppeared();
    makeActorAppeared();
    MR::emitEffect(this, "EyeLight");
}

void StringSpider::initAfterPlacement() {
    Triangle triangle = Triangle();
    TVec3f v1, upVec, v2;
    MR::calcUpVec(&upVec, this);
    upVec.setLength(200.0f);

    if (MR::getFirstPolyOnLineToMap(&v1, &triangle, mPosition - upVec * 0.5f, upVec)) {
        triangle.getBaseInvMtx()->mult(v1, v2);
        MR::resetPosition(mString, v1);
        mFixedPos->setBaseMtx(triangle.getBaseMtx()->mMtx);
        mFixedPos->setLocalTrans(v2);
    } else {
        mFixedPos = nullptr;
    }
}

void StringSpider::kill() {
    LiveActor::kill();
    MR::emitEffect(this, "Death");
    MR::startActionSound(this, "EmExplodeS");
}

void StringSpider::makeActorDead() {
    LiveActor::makeActorDead();
    mString->kill();
}

void StringSpider::startClipped() {
    LiveActor::startClipped();
    mString->startClipped();
    MR::deleteEffect(this, "EyeLight");
}

void StringSpider::endClipped() {
    LiveActor::endClipped();
    mString->endClipped();
    MR::emitEffect(this, "EyeLight");
}

void StringSpider::doSwingAction(f32 a1) {
    TVec3f root(mString->mPosition);
    root.sub(mPosition);
    MR::normalizeOrZero(&root);
    if (MR::isOppositeDirection(mGravity, root, 1.0f)) {
        TVec3f side;
        mInitPoseQuat.getXDir(side);
        TVec3f vel;
        MR::vecKillElement(mVelocity, root, &vel);
        if (MR::isNearZero(vel)) {
            vel.set(side);
        } else {
            side.scale(side.dot(vel));
            MR::normalizeOrZero(&side);
            vel.set(side);
        }
        TVec3f add(vel);
        add.scale(a1);
        mVelocity.add(add);
    }
}

void StringSpider::tryStartStringPointBind() {
    if (canPointBindSpider() && mBindState->tryStartPointBind()) {
        setNerve(GET_NERVE(StringSpider, HostTypeNrvDpdGripped));
    }
}

void StringSpider::reactStringForce() {
    TVec3f root;
    f32 length;
    if (isOverStringLength(&root, &length)) {
        MR::normalizeOrZero(&root);
        MR::calcReflectionVector(&mVelocity, root, 0.25f, 1.0f);
        mVelocity += root * (length - mCurrentLength) * 0.02f;
        MR::restrictVelocity(this, 100.0f);
    }
}

bool StringSpider::isOverStringLength(TVec3f* a1, f32* a2) {
    TVec3f rootVec = calcToRootVector();
    f32 rootLen = rootVec.length();
    if (a1 != nullptr) {
        a1->set(rootVec);
    }

    if (a2 != nullptr) {
        *a2 = rootLen;
    }

    return mCurrentLength < rootLen;
}

void StringSpider::control() {
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvEaten))) return;
    if (isEatable()) YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (mFixedPos != nullptr) {
        mFixedPos->calc();
        mFixedPos->copyTrans(&mString->mPosition);
    }
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvSpinned))) return;
    mScaleController->updateNerve();
    tryStartStringPointBind();
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    MR::addVelocityToGravity(this, 1.5f);
    MR::attenuateVelocity(this, 0.98f);
    if (!isNerve(GET_NERVE(StringSpider, HostTypeNrvSpinned)) && !isNerve(GET_NERVE(StringSpider, HostTypeNrvBlow))) reactStringForce();
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvSearch)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvAttack))) startSwingLevelSound();
}

TVec3f StringSpider::calcToRootVector() {
    return mString->mPosition - mPosition;
}

void StringSpider::calcPosture(f32 a1, bool a2) {
    TVec3f v1;
    TVec3f rootVec = calcToRootVector();
    TVec3f v3, v9;
    mPoseQuat.getYDir(v1);
    MR::normalizeOrZero(&rootVec);
    if (!MR::isNearZero(rootVec)) {
        mPoseQuat.getZDir(v3);

        if (a2) {
            MR::calcVecToPlayerH(&v9, this, &v1);
            MR::normalizeOrZero(&v9);
            v3.set(v9);

            if (MR::isNearZero(v3)) {
                return;
            }
        }

        MR::blendQuatUpFront(&mPoseQuat, rootVec, v3, a1, 0.03f);
    }
}

void StringSpider::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, "Wait");
    }

    calcPosture(0.25f, false);
    if (MR::isValidSwitchA(this)) {
        if (MR::isOnSwitchA(this)) {
            setNerve(GET_NERVE(StringSpider, HostTypeNrvDownStart));
        }
    } else if (MR::calcDistanceToPlayer(this) < 1400.0f) {
        setNerve(GET_NERVE(StringSpider, HostTypeNrvDownStart));
    }
}

void StringSpider::exeDownStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "DownStart");
    }

    calcPosture(0.25f, false);
    MR::calcDistanceToPlayer(this);

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(StringSpider, HostTypeNrvDown));
    }
}

void StringSpider::exeDownEnd() {
    if (MR::isFirstStep(this)) MR::startAction(this, "DownEnd");
    calcPosture(0.25f, false);
    doSwingAction(0.2f);
    if (MR::isActionEnd(this)) setNerve(GET_NERVE(StringSpider, HostTypeNrvSearch));
}

void StringSpider::exeDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Down");
    }

    MR::startActionSound(this, "EmLvStrspiderDown");
    calcPosture(0.25f, true);
    extendString();
    f32 v1;
    bool isOver = isOverStringLength(nullptr, &v1);
    if ((mMaxLength - 230.0f) < v1) {
        MR::tryStartBck(this, "PreDownEnd");
    }

    doSwingAction(0.2f);
    if (mCurrentLength == mMaxLength && isOver) {
        setNerve(GET_NERVE(StringSpider, HostTypeNrvDownEnd));
    }
}

void StringSpider::exeUp() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Up");
    }

    MR::startActionSound(this, "EmLvStrspiderUp");
    calcPosture(0.25f, true);

    if (MR::calcDistanceToPlayer(this) < 1400.0f) {
        setNerve(GET_NERVE(StringSpider, HostTypeNrvDownStart));
    } else {
        shortenString(5.0f);
        if (mCurrentLength == (0.333f * mMaxLength)) {
            setNerve(GET_NERVE(StringSpider, HostTypeNrvWait));
        }
    }
}

void StringSpider::exeSearch() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Wait");
    calcPosture(0.25f, true);
    if (MR::isBindedWallOfMap(this) || MR::isBindedGround(this)) shortenString(40.0f);
    else extendString();
    doSwingAction(0.2f);
    f32 distance = MR::calcDistanceToPlayer(this);
    if ((!MR::isValidSwitchA(this) || !MR::isOnSwitchA(this)) && 1600.0f < distance) setNerve(GET_NERVE(StringSpider, HostTypeNrvUp));
}

void StringSpider::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hit");
        MR::startActionSound(this, "EvStrspiderHit");
    }
    calcPosture(0.25f, true);
    extendString();
    if (MR::isActionEnd(this)) setNerve(GET_NERVE(StringSpider, HostTypeNrvSearch));
}

void StringSpider::exeHit() {

    if (MR::isFirstStep(this)) {
        MR::startBlowHitSound(this);
        MR::startAction(this, "Spin");
    }

    f32 nerveRate = MR::calcNerveRate(this, 480);
    f32 nerveRateMinus = 1.0f - nerveRate;
    f32 liner1 = MR::getLinerValueFromMinMax(nerveRateMinus, ::hSpinSeMinNoteLength, ::hSpinSeMaxNoteLength, ::hSpinSeLimitMinVolume, 100.0f);
    f32 liner2 = MR::getLinerValueFromMinMax(nerveRate, ::hSpinSeMinNoteLength, ::hSpinSeMaxNoteLength, ::hSpinSeMinVolume, 24.0f);
    if (liner1 >= 51.0f) {
        MR::startActionSound(this, "EmLvStrspiderSpin", liner1, liner2, -1);
    }

    TQuat4f v14;
    v14.setRotate(TVec3f(0, 1, 0), (-0.4f * nerveRateMinus));
    PSQUATMultiply(mPoseQuat, v14, mPoseQuat);
    calcPosture(0.25f, false);

    if (MR::isGreaterStep(this, 480)) {
        setNerve(GET_NERVE(StringSpider, HostTypeNrvSearch));
    }
}

void StringSpider::exeSpinned() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Press");
        MR::startActionSound(this, "EmStompedS");
        MR::startActionSound(this, "EvStrspiderDead");
        MR::zeroVelocity(this);
    }
    if (MR::isActionEnd(this)) {
        MR::emitItemStomp(this);
        kill();
    }
}

void StringSpider::exeBlow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Blow");
        MR::startActionSound(this, "EvStrspiderDead");
        MR::startBlowHitSound(this);
        mString->kill();
    }

    if (mSpinHitController->execute(this)) MR::emitItemHit(this);
}

void StringSpider::extendString() {
    mCurrentLength += 100.0f;
    mCurrentLength = MR::clamp(mCurrentLength, 0.333f * mMaxLength, mMaxLength);
}

void StringSpider::shortenString(f32 a1) {
    mCurrentLength -= a1;
    mCurrentLength = MR::clamp(mCurrentLength, 0.333f * mMaxLength, mMaxLength);
}

void StringSpider::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mPoseQuat);
    MR::updateBaseScale(this, mScaleController);
    MR::calcJointWorldPos(&mString->mEndPos, TVec3f(-80.0f, 0.0f, 0.0f), this, "Hip");
    mString->calcAndSetBaseMtx();
}

void StringSpider::calcAnim() {
    LiveActor::calcAnim();
    mString->calcAnim();
}

void StringSpider::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvEaten))) {
        if (MR::isSensorPlayer(pReceiver)) {
            MR::setScale(this, TVec3f(1.0f));
            MR::setVelocitySeparateHV(this, pReceiver, pSender, 30.0f, 0.0f);
            setNerve(GET_NERVE(StringSpider, HostTypeNrvHit));
        }
    } else {
        TVec3f direction;
        MR::calcSensorDirectionNormalize(&direction, pReceiver, pSender);
        if (MR::isSensorPlayer(pReceiver)) {
            if (MR::isPlayerHipDropFalling() || MR::isPlayerHipDropLand()) return;
            if (MR::isPlayerExistUp(this, MR::getSensorRadius(this, "Body"), 0.25f)) {
                MR::sendMsgPush(pReceiver, pSender);
                return;
            }
            if (!isDamaging() || !MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) MR::sendMsgPush(pReceiver, pSender);
        } else if ((MR::isSensorEnemy(pReceiver) || MR::isSensorYoshi(pReceiver)) && MR::sendMsgPush(pReceiver, pSender)) {
            MR::calcReflectionVector(&mVelocity, direction, 0.25f, 1.0f);
            f32 speed = mVelocity.length();
            if (speed >= 3.0f) MR::startActionSound(this, "EmStrspiderCollision", MR::getLinerValueFromMinMax(speed, 0.0f, 15.0f, 0.0f, 100.0f), -1);
        }
    }
}

bool StringSpider::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvEaten))) return false;
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvSpinned)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvBlow))) return false;
    if (MR::isMsgToEnemyAttackTrample(msg)) return false;
    if (MR::isMsgToEnemyAttackBlow(msg)) {
        TVec3f v(*MR::getSensorPos(pReceiver));
        v.sub(*MR::getSensorPos(pSender));
        v.setLength(MR::getSensorHost(pSender)->mVelocity.length());
        mVelocity.add(v);
        return true;
    }
    if (MR::isMsgExplosionAttack(msg)) {
        mSpinHitController->start(this, *MR::getSensorPos(pSender), *MR::getSensorPos(pReceiver));
        setNerve(GET_NERVE(StringSpider, HostTypeNrvBlow));
        return true;
    }
    return false;
}

bool StringSpider::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvEaten))) return false;
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvSpinned)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvBlow))) return false;
    if (MR::isSensorPlayer(pSender)) return false;
    TVec3f v(*MR::getSensorPos(pReceiver));
    v.sub(*MR::getSensorPos(pSender));
    f32 length = v.length();
    f32 senderRadius = MR::getSensorRadius(pSender);
    f32 receiverRadius = MR::getSensorRadius(pReceiver);
    v.setLength(0.5f * ((receiverRadius + senderRadius) - length));
    mVelocity.add(v);
    return true;
}

bool StringSpider::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvEaten))) return false;
    if (MR::isMsgStarPieceAttack(msg)) {
        MR::setVelocitySeparateHV(this, pSender, pReceiver, 30.0f, 0.0f);
        setNerve(GET_NERVE(StringSpider, HostTypeNrvHit));
        return true;
    }
    if (MR::isMsgLockOnStarPieceShoot(msg)) return canLockOnSpiderWithStarPiece();
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvSpinned)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvBlow))) return false;
    if (MR::isMsgPlayerHipDrop(msg) || MR::isMsgPlayerTrample(msg)) {
        TVec3f v(*MR::getSensorPos(pSender));
        v.sub(*MR::getSensorPos(pReceiver));
        MR::vecKillElement(v, mGravity, &v);
        MR::normalizeOrZero(&v);
        if (!MR::isNearZero(v)) {
            v.scale(10.0f);
            TVec3f gravity(mGravity);
            gravity.scale(13.0f);
            v.sub(gravity);
            mVelocity.add(v);
        }
        setNerve(GET_NERVE(StringSpider, HostTypeNrvSpinned));
        return true;
    }
    if (MR::isMsgInvincibleAttack(msg) || MR::isMsgYoshiProjectileOrEat(msg)) {
        mSpinHitController->start(this, *MR::getSensorPos(pSender), *MR::getSensorPos(pReceiver));
        setNerve(GET_NERVE(StringSpider, HostTypeNrvBlow));
        return true;
    }
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && canSpinHitSpider()) {
        MR::setVelocitySeparateHV(this, pSender, pReceiver, 30.0f, 0.0f);
        setNerve(GET_NERVE(StringSpider, HostTypeNrvHit));
        return true;
    }
    return false;
}

bool StringSpider::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerKick(msg)) {
        if (canKickHitSpider()) {
            mSpinHitController->start(this, *MR::getSensorPos(pSender), *MR::getSensorPos(pReceiver));
            setNerve(GET_NERVE(StringSpider, HostTypeNrvBlow));
            return true;
        }
        return false;
    }
    if (isEatable() && MR::isMsgYoshiStartTongue(msg)) {
        MR::tryHitReactionAtPos(this, "舌引っ張り", &mPosition);
        MR::zeroVelocity(this);
        MR::addVelocityMoveToTargetHorizon(this, *MR::getPlayerPos(), 30.0f);
        setNerve(GET_NERVE(StringSpider, HostTypeNrvHit));
        return false;
    }
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvEaten))) {
        if (MR::isMsgYoshiLoopTongue(msg)) return false;
        if (MR::isMsgYoshiReleaseEaten(msg) || MR::isMsgYoshiCancelEaten(msg)) {
            MR::setScale(this, TVec3f(1.0f));
            MR::setVelocitySeparateHV(this, pSender, pReceiver, 30.0f, 0.0f);
            setNerve(GET_NERVE(StringSpider, HostTypeNrvHit));
            return false;
        }
        return false;
    }
    return false;
}

void StringSpider::startSwingLevelSound() {
    f32 mag = mVelocity.length();
    s32 liner1 = 100.0f * MR::getLinerValueFromMinMax(mag, ::hSwing1SeMinSpeed, ::hSwing1SeMaxSpeed, ::hSwing1SeMaxVolume, 1.0f);
    s32 liner2 = 100.0f * MR::getLinerValueFromMinMax(mag, ::hSwing2SeMinSpeed, ::hSwing2SeMaxSpeed, ::hSwing2SeMaxVolume, 1.0f);
    MR::startActionSound(this, "EmLvStrspiderSwing1", liner1);
    MR::startActionSound(this, "EmLvStrspiderSwing2", liner2);
}

bool StringSpider::canLockOnSpiderWithStarPiece() const {
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvHit)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvSpinned)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvBlow))) return false;
    return true;
}

bool StringSpider::isEatable() const {
    bool result = false;
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvWait)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvDownStart)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvDownEnd)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvUp)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvSearch)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvHit))) result = true;
    return result;
}

bool StringSpider::isDamaging() const {
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvEaten)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvDpdGripped)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvHit)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvBlow)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvSpinned))) return false;
    return true;
}

bool StringSpider::canSpinHitSpider() const {
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvHit))) return MR::isGreaterStep(this, 45);
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvBlow)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvSpinned))) return false;
    return true;
}

bool StringSpider::canKickHitSpider() const {
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvHit)) && MR::isGreaterStep(this, 5)) return true;
    return false;
}

bool StringSpider::canPointBindSpider() const {
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvSpinned)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvBlow)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvEaten)) || isNerve(GET_NERVE(StringSpider, HostTypeNrvDpdGripped))) return false;
    if (isNerve(GET_NERVE(StringSpider, HostTypeNrvHit))) return MR::isGreaterStep(this, 20);
    return true;
}

void StringSpider::endWait() {
    MR::invalidateClipping(this);
}

StringSpiderString::StringSpiderString(const char* pName) : LiveActor(pName), mRotation(0, 0, 0, 1), mEndPos(0, 0, 0) {
    MR::initActor(this, "StringSpiderString", false);
    MR::connectToScene(this, MR::MovementType_None, MR::CalcAnimType_MapObjDecoration, MR::DrawBufferType_Enemy, MR::DrawType_None);
    MR::invalidateClipping(this);
}

void StringSpiderString::calcAndSetBaseMtx() {
    TVec3f v10(mScale);
    TVec3f v9(mPosition);
    v9.sub(mEndPos);
    MR::separateScalarAndDirection(&v10.y, &v9, v9);
    v10.y *= 0.063f;

    if (MR::isOppositeDirection(TVec3f(0, 1, 0), v9)) {
        mRotation.setRotate(TVec3f(1, 0, 0), MR::pi());
    } else {
        mRotation.setRotate(TVec3f(0, 1, 0), v9);
    }

    TPos3f v12;
    v12.makeQuat(mRotation);
    v12.setTrans(mPosition);
    MR::setBaseTRMtx(this, v12);
    MR::setBaseScale(this, v10);
}

