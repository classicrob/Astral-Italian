#include "Game/Player/MarioYoshiBalloon.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioAnimator.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Player/YoshiLockOnManager.hpp"
#include "Game/Player/YoshiTongueAction.hpp"
#include "Game/Screen/GameSceneLayoutHolder.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/MapObj/BigFanHolder.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

MarioYoshiBalloon::MarioYoshiBalloon(MarioActor* actor) : MarioState(actor, MarioStatus_YoshiBalloon) {
    mAir = 1.0f;
    mRequestedState = 0;
    mState = 0;
    mStateFrame = 0;
    mRequestedAnimation = 0;
    mAnimation = 0;
    mAnimationFrame = 0;
    mFirstJet = true;
    mWind.x = 0.0f;
    mWind.y = 0.0f;
    mWind.z = 0.0f;
}

bool MarioYoshiBalloon::start() {
    getPlayer()->mMovementStates.mLowBits.mBit30 = 0;
    getPlayer()->mMovementStates.mLowBits.mBit31 = 1;
    getPlayer()->mMovementStates.mLowBits.mBit20 = 0;
    MR::getGameSceneLayoutHolder()->changeLifeMeterModeWater();
    mWind.z = 0.0f;
    mWind.y = 0.0f;
    mWind.x = 0.0f;
    mAir = 1.0f;
    mState = 0;
    mRequestedState = 0;
    mStateFrame = 0;
    mRequestedAnimation = 0;
    mAnimation = 0;
    mAnimationFrame = 0;
    static_cast<YoshiLockOnManager*>(mActor->fn_803B9820())->mSuspended = true;
    mFirstJet = true;
    MR::startActionSound(mActor->fn_803B9830()->mRideModelActor, "Swell", -1, -1, -1);
    getPlayer()->_A72 = 0;
    return true;
}

// NONMATCHING: extra saved GPR and stack frame; equivalent comparison materialization.
bool MarioYoshiBalloon::close() {
    MR::getGameSceneLayoutHolder()->changeLifeMeterModeGround();
    mActor->getYoshi()->fn_8040EDE0(nullptr);
    mActor->getYoshi()->fn_8040EFB0();
    setJumpVec(fn_803EF300());
    mActor->fn_803B9810()->fn_8041FE80();
    YoshiLockOnManager* lockOn = static_cast<YoshiLockOnManager*>(mActor->fn_803B9820());
    lockOn->mSuspended = false;
    mActor->getYoshi()->fn_8040CC00();
    mActor->fn_803B9830()->mRideModelActor->kill();
    mActor->mMarioAnim->disableExtraAnimation();
    changeAnimation("落下", "落下");
    MR::startActionSound(mActor->fn_803B9830()->mRideModelActor, "Burst", -1, -1, -1);
    bool empty = mAir <= 0.0f;
    if (empty == false) {
        MR::emitEffect(mActor->getYoshi(), "BalloonDamage");
    }
    return true;
}

bool MarioYoshiBalloon::update() {
    if (!fn_80414580()) {
        return false;
    }
    fn_80414730();
    return true;
}

void MarioYoshiBalloon::fn_80414570() { mAir = 0.0f; }

// NONMATCHING: equivalent comparison materialization and GPR allocation.
bool MarioYoshiBalloon::fn_80414580() {
    if (mRequestedState != mState) {
        mStateFrame = 0;
        mState = mRequestedState;
    }
    switch (mState) {
    case 0:
        if (mStateFrame == 0) {
            changeAnimation("バルーン膨らむ", static_cast<const char*>(nullptr));
            mActor->getYoshi()->fn_8040EF40();
        }
        if (!mActor->getYoshi()->fn_8040F490("バルーン膨らむ")) {
            fn_80415390(1);
            mRequestedState = 1;
        }
        addVelocity(-getAirGravityVec() * 5.0f);
        break;
    case 1:
        bool empty = mAir <= 0.0f;
        if (empty) {
            return false;
        }
        const TVec3f* gravity;
        const TVec3f& velocity = fn_803EF300();
        gravity = &getAirGravityVec();
        TVec3f horizontal;
        JMAVECScaleAdd(*gravity, velocity, horizontal, -gravity->dot(velocity));
        TVec3f vertical;
        JMathInlineVEC::PSVECSubtract(fn_803EF300(), horizontal, vertical);
        fn_80415330();
        fn_80414B90(vertical);
        fn_80414CD0();
        fn_80415030();
        fn_80415150();
        break;
    }
    mStateFrame++;
    return true;
}

// NONMATCHING: initial state loads and comparison operand order.
void MarioYoshiBalloon::fn_80414730() {
    if (mAnimation != mRequestedAnimation) {
        mAnimation = mRequestedAnimation;
        mAnimationFrame = 0;
    }
    if (mAnimation != 0) {
        MR::setStageBGMStateBit(0x200);
    }
    s32 tone = 1000.0f * mAir;
    switch (mAnimation) {
    case 1:
        if (mAnimationFrame == 0) {
            mActor->fn_803B9830()->mRideModelActor->appear();
            mActor->mMarioAnim->enableExtraAnimation();
            mActor->mMarioAnim->changeExtraAnimation("YoshiBalloonMove");
            MR::startAction(mActor->fn_803B9830()->mRideModelActor, "YoshiBalloonStart");
            MR::startActionSound(mActor->fn_803B9830()->mRideModelActor, "JetStartV", -1, -1, -1);
        }
        if (mAnimationFrame == 1) {
            mActor->getYoshi()->fn_8040CC20();
        }
        if (MR::isActionEnd(mActor->fn_803B9830()->mRideModelActor)) {
            if (checkLvlA()) fn_80415390(3);
            else fn_80415390(2);
        }
        MR::startActionSound(mActor->fn_803B9830()->mRideModelActor, "YoshiBalloonStart", -1, -1, -1);
        MR::startActionSound(mActor->fn_803B9830()->mRideModelActor, "YoshiBalloonJetTone", tone, -1, -1);
        break;
    case 2:
        if (mAnimationFrame == 0) {
            MR::startAction(mActor->fn_803B9830()->mRideModelActor, "YoshiBalloonMove");
        }
        if (mFirstJet) {
            MR::startActionSound(mActor->fn_803B9830()->mRideModelActor, "YoshiBalloonStart", -1, -1, -1);
        } else {
            MR::startActionSound(mActor->fn_803B9830()->mRideModelActor, "YoshiBalloonReStart", -1, -1, -1);
        }
        MR::startActionSound(mActor->fn_803B9830()->mRideModelActor, "YoshiBalloonJetTone", tone, -1, -1);
        if (checkLvlA()) fn_80415390(3);
        else fn_80415390(2);
        break;
    case 3:
        if (mAnimationFrame == 0) {
            MR::startAction(mActor->fn_803B9830()->mRideModelActor, "YoshiBalloonBrake");
            MR::startActionSound(mActor->fn_803B9830()->mRideModelActor, "YoshiBalloonBrakeTRG", -1, -1, -1);
            mFirstJet = false;
        }
        MR::startActionSound(mActor->fn_803B9830()->mRideModelActor, "YoshiBalloonBrakeLV", -1, -1, -1);
        if (checkLvlA()) fn_80415390(3);
        else fn_80415390(2);
        break;
    case 4:
        if (mAnimationFrame == 0) {
            MR::startAction(mActor->fn_803B9830()->mRideModelActor, "YoshiBalloonBound");
            mFirstJet = false;
        }
        MR::fn_80008510(mActor->fn_803B9830()->mRideModelActor, "YoshiBalloonReStart", 1);
        MR::startActionSound(mActor->fn_803B9830()->mRideModelActor, "YoshiBalloonReStart", -1, -1, -1);
        MR::startActionSound(mActor->fn_803B9830()->mRideModelActor, "YoshiBalloonJetTone", tone, -1, -1);
        if (MR::isActionEnd(mActor->fn_803B9830()->mRideModelActor)) {
            if (checkLvlA()) fn_80415390(3);
            else fn_80415390(2);
        }
        break;
    case 5:
        if (mAnimationFrame == 0) {
            MR::startAction(mActor->fn_803B9830()->mRideModelActor, "YoshiBalloonEat");
            mFirstJet = false;
        }
        if (MR::isActionEnd(mActor->fn_803B9830()->mRideModelActor)) {
            if (checkLvlA()) fn_80415390(3);
            else fn_80415390(2);
        }
        break;
    }
    mAnimationFrame++;
}

// NONMATCHING: temporary vector stack slots.
void MarioYoshiBalloon::fn_80414B90(const TVec3f& vertical) {
    TVec3f lift;
    if (checkLvlA()) {
        lift = -getAirGravityVec() * 0.7f;
        mAir -= 0.0015f;
    } else {
        lift = -getAirGravityVec() * 7.0f;
        mAir -= 0.003f;
    }
    addVelocity(vertical * 0.95f + lift * 0.05f);
}

// NONMATCHING: temporary stack slots, GPR allocation and multiply-result register.
void MarioYoshiBalloon::fn_80414CD0() {
    const TVec3f* gravity;
    const TVec3f& pad = getWorldPadDir();
    gravity = &getAirGravityVec();
    TVec3f desired;
    JMAVECScaleAdd(*gravity, pad, desired, -gravity->dot(pad));
    MR::normalizeOrZero(&desired);
    desired.scale(8.0f);
    TVec3f velocity(fn_803EF300());
    const TVec3f& gravity2 = getAirGravityVec();
    TVec3f horizontal;
    JMAVECScaleAdd(gravity2, velocity, horizontal, -gravity2.dot(velocity));
    TVec3f newVelocity(horizontal * 0.95f + desired * 0.05f);
    addVelocity(newVelocity);
    getPlayer()->setHeadVec(-getAirGravityVec());
    getPlayer()->fixFrontVecFromUpSide();
    TVec3f current;
    TVec3f opposite(-desired);
    const TVec3f& gravity3 = getAirGravityVec();
    TVec3f facing;
    JMAVECScaleAdd(gravity3, opposite, facing, -gravity3.dot(opposite));
    const TVec3f* gravity4;
    const TVec3f& front = getFrontVec();
    gravity4 = &getAirGravityVec();
    JMAVECScaleAdd(*gravity4, front, current, -gravity4->dot(front));
    if (MR::normalizeOrZero(&current)) {
        if (!MR::normalizeOrZero(&facing)) {
            getPlayer()->setFrontVecKeepUp(facing);
        }
    } else {
        if (MR::normalizeOrZero(&facing)) {
            facing = current;
        }
        TVec3f cross;
        PSVECCrossProduct(current, facing, cross);
        f32 angle = current.angle(facing);
        if (cross.dot(getAirGravityVec()) < 0.0f) angle = -angle;
        if (checkLvlA()) angle *= 0.022f;
        else angle *= 0.03f;
        TQuat4f turn;
        turn.setRotate(getAirGravityVec(), angle);
        turn.transform(current);
        getPlayer()->setFrontVecKeepUp(current);
    }
    TVec3f adjusted(getFrontVec());
    TVec3f axis(getPlayer()->mSideVec);
    MR::normalizeOrZero(&velocity);
    f32 alignment = adjusted.dot(-velocity);
    if (alignment < 0.0f) alignment = 0.0f;
    f32 speed = PSVECMag(newVelocity);
    speed *= 0.125f;
    if (speed > 1.0f) speed = 1.0f;
    TQuat4f tilt;
    tilt.setRotate(axis, speed * (MR::toRadian(-20.0f) * alignment));
    tilt.transform(adjusted);
    getPlayer()->setFrontVecKeepSide(adjusted);
}

void MarioYoshiBalloon::fn_80415030() {
    TVec3f wind;
    f32 strength;
    BigFanFunction::calcWindInfo(&wind, mActor->_240, &strength);
    strength = MR::clamp(strength, 0.0f, 10.0f);
    wind.scale(strength);
    mWind.x *= 0.94f;
    mWind.y *= 0.94f;
    mWind.z *= 0.94f;
    mWind += wind * 0.15f;
    addVelocity(wind * 0.5f);
    if (PSVECMag(mWind) > 0.2f) {
        getPlayer()->mDrawStates.mBits.mBit0 = 1;
    }
}

// NONMATCHING: temporary vector stack slots.
void MarioYoshiBalloon::fn_80415150() {
    TVec3f position = getTrans() - getAirGravityVec() * 80.0f;
    TVec3f normal;
    if (fn_804153C0(&normal, position, 110.0f, 100.0f)) {
        TVec3f horizontal;
        TVec3f velocity(getVelocity());
        JMAVECScaleAdd(normal, velocity, horizontal, -normal.dot(velocity));
        f32 scale = ((180.0f * normal.angle(getAirGravityVec())) / 3.1415927f - 100.0f) / -100.0f;
        if (scale < 0.0f) scale = 0.0f;
        TVec3f rebound = velocity - horizontal;
        rebound.scale(scale);
        f32 minimum = 8.0f + 12.0f * scale;
        if (PSVECMag(rebound) < minimum) {
            rebound.set(-normal * minimum);
        }
        velocity = horizontal - rebound;
        clearVelocity();
        addVelocity(velocity);
        fn_80415390(4);
    }
}

// NONMATCHING: zero byte store uses a saved GPR instead of r0.
void MarioYoshiBalloon::fn_80415330() {
    if (getPlayer()->_A72) {
        fn_80415390(5);
        Mario* player = getPlayer();
        player->_A72 = 0;
    }
}

void MarioYoshiBalloon::fn_80415390(s32 animation) {
    if ((mAnimation != 0 || animation == 1) && (mAnimation != 1 || animation != 4)) {
        mRequestedAnimation = animation;
    }
}

bool MarioYoshiBalloon::fn_804153C0(TVec3f* normal, const TVec3f& position, f32 radius, f32 angle) {
    s32 count = Collision::checkStrikeBallToMap(position, radius, nullptr, nullptr);
    if (count == 0) return false;
    bool found = false;
    TVec3f minimum(0.0f);
    TVec3f maximum(0.0f);
    for (u32 i = 0; i < count; i++) {
        TVec3f hitNormal(*Collision::getStrikeInfoMap(i)->mParentTriangle.getNormal(0));
        f32 magnitude = angle;
        if (angle < 0.0f) magnitude = -angle;
        f32 threshold = JMath::sSinCosTable.cos_((u16)(s32)(45.511112f * magnitude));
        if (hitNormal.dot(getAirGravityVec()) >= threshold) {
            if (minimum.x > hitNormal.x) minimum.x = hitNormal.x;
            if (minimum.y > hitNormal.y) minimum.y = hitNormal.y;
            if (minimum.z > hitNormal.z) minimum.z = hitNormal.z;
            if (maximum.x < hitNormal.x) maximum.x = hitNormal.x;
            if (maximum.y < hitNormal.y) maximum.y = hitNormal.y;
            if (maximum.y < hitNormal.z) maximum.z = hitNormal.z;
            found = true;
        }
    }
    if (!found) return false;
    normal->set(minimum + maximum);
    return !MR::normalizeOrZero(normal);
}
