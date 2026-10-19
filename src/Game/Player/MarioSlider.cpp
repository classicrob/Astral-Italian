#include "Game/Player/MarioSlider.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/KariKariDirector.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Util/MathUtil.hpp"





MarioSlider::MarioSlider(MarioActor* pActor) : MarioState(pActor, MarioStatus_Slider) {
    _14.x = 0.0f;
    _14.y = 0.0f;
    _14.z = 0.0f;
    _20.x = 0.0f;
    _20.y = 0.0f;
    _20.z = 0.0f;
    _2C.x = 0.0f;
    _2C.y = 0.0f;
    _2C.z = 0.0f;
    _38 = 0.0f;
    _3C = 0.0f;
    _40 = 0;
    mPushCooldown = 0;
    mSlideEffectActive = false;
}

void MarioSlider::calcGroundAccel() {
    TVec3f* val = &getPlayer()->_368;
    TVec3f v6 = val->cross(*getPlayer()->getGravityVec());
    MR::normalizeOrZero(&v6);
    _2C.cross(v6, *val);
    MR::normalizeOrZero(&_2C);
}

bool MarioSlider::postureCtrl(MtxPtr) {
    Mtx rotMtx;
    TVec3f& frontVec = getFrontVec();
    PSMTXRotAxisRad(rotMtx, &frontVec, 0.2f * (MR::pi() * _3C / 2.0f));

    TVec3f newHeadVec;
    PSMTXMultVec(rotMtx, &getPlayer()->_368, &newHeadVec);

    MarioConst* pConst = mActor->mConst;
    Mario* pPlayer = getPlayer();
    MR::vecBlendSphere(&getPlayer()->_1FC, pPlayer->_1FC, newHeadVec, pConst->getTable()->mSliderHeadRotateRatio);
    return false;
}

void MarioSlider::calcWallHit() {
    if (((getPlayer()->mMovementStates.mLowWord >> 5) & 1U)) {
        TVec3f stack_50(_14);
        f32 sideDot = MR::vecKillElement(stack_50, *getPlayer()->mSideWallTriangle->getNormal(0), &stack_50);

        if (sideDot < 0.0f) {
            if (-sideDot < 10.0f) {
                sideDot = -10.0f;
            }

            _14 = stack_50 - *getPlayer()->mSideWallTriangle->getNormal(0) * sideDot;
        }
    }

    if (((getPlayer()->mMovementStates.mLowWord >> 23) & 1U)) {
        TVec3f stack_44(_14);
        f32 frontDot = MR::vecKillElement(stack_44, *getPlayer()->mFrontWallTriangle->getNormal(0), &stack_44);

        if (frontDot < 0.0f) {
            if (-frontDot < 10.0f) {
                frontDot = -10.0f;
            }

            f32 frontScale = 0.5f * frontDot;
            _14 = stack_44 - *getPlayer()->mFrontWallTriangle->getNormal(0) * frontScale;

            TVec3f stack_38(_14);
            MR::normalize(&stack_38);
            getPlayer()->setFrontVecKeepUp(stack_38);
        }
    }
}



bool MarioSlider::start() {
    _14 = getPlayer()->_16C;
    MR::vecKillElement(_14, getPlayer()->_368, &_14);
    _20 = getFrontVec();
    getPlayer()->cancelSquatMode();
    changeAnimation("スライダー尻", "スライダー尻");
    _38 = 0.0f;
    _3C = 0.0f;
    _40 = 10;
    mPushCooldown = 0;
    mSlideEffectActive = false;
    return true;
}

// NONMATCHING: temporary stack slots, GPR allocation and repeated integer-abs CSE.
bool MarioSlider::update() {
    if (!getPlayer()->checkSliderMode()) {
        if (((getPlayer()->mMovementStates.mLowWord >> 30) & 1U) != 0) {
            if (isSlipPolygon(getGroundPolygon())) {
                Mario* player = getPlayer();
                player->_8F8 = _14;
                player->_8F0 = 10.0f;
                return false;
            }
        }

        _40--;
        if (_40 == 0) {
            getPlayer()->tryDrop();
            getPlayer()->mJumpVec = mActor->getLastMove();
            return false;
        }
    } else {
        _40 = 10;
    }

    calcGroundAccel();
    if (mPushCooldown > 0) --mPushCooldown;

    if (checkTrgA()) {
        getPlayer()->mWalkSpeed = (_14.length() / mActor->mConst->getTable()->mJumpFrontSpeed);
        getPlayer()->tryJump();
        return false;
    } else {
        const TVec3f& front=getFrontVec();
        TVec3f cam=MR::getCamZdir();
        f32 stickY,stickX;
        if (cam.dot(front)>=0.0f) {stickY=-getStickY();stickX=getStickX();}
        else {stickY=getStickY();stickX=-getStickX();}
        _38 = (stickY * (1.0f - mActor->getConst().getTable()->mSliderBrakeIne)) + (_38 * mActor->mConst->getTable()->mSliderBrakeIne);
        MarioConstTable* tbl = mActor->mConst->getTable();
        _3C = (stickX * (1.0f - tbl->mSliderWeightIne)) + (_3C * tbl->mSliderWeightIne);
        f32 v18 = _14.length();
        v18 -= mActor->mConst->getTable()->mSliderBrakePow;

        if (v18 > 20.0f) {
            v18 = (v18 * (1.0f - (0.1f * _38)));
        }

        TVec3f v48(_14);

        if (MR::isNearZero(v48)) {
            v48 = _2C * mActor->mConst->getTable()->mSliderSlopePow;
        }

        if (MR::isNearZero(v48)) {
            return false;
        } else {
            MR::normalize(&v48);
            TVec3f v47(getPlayer()->_368);
            TVec3f v46 = v48.cross(v47);
            MR::vecKillElement(v46, getPlayer()->_368, &v46);
            MR::normalize(&v46);
            _14.setLength(v18);
            _14 += _2C * mActor->mConst->getTable()->mSliderSlopePow;

            _14 += v46 * _3C * mActor->mConst->getTable()->mSliderWeightPow;
            MR::vecKillElement(_14, getPlayer()->_368, &_14);
            if (_14.length() > mActor->mConst->getTable()->mSliderMaxSpeed) {
                _14.setLength(mActor->mConst->getTable()->mSliderMaxSpeed);
            }

            addVelocity(_14);

            TVec3f v45(-getPlayer()->_368 * 15.0f);

            MR::vecKillElement(v45, getFrontVec(), &v45);
            addVelocity(v45);

            TVec3f v44(getPlayer()->mFrontVec);
            TVec3f v43;
            MR::vecKillElement(_14, *getPlayer()->getGravityVec(), &v43);

            if (!MR::isNearZero(v43)) {
                MR::normalize(&v43);
                getPlayer()->setFrontVecKeepUp(v43, mActor->mConst->getTable()->mSliderFrontTurnRatio);
            }

            calcWallHit();
            playSound("坂滑り");
            s32 soundLevel=static_cast<s32>(1000.0f*_3C);
            if (MR::abs(soundLevel)>50) MR::startActionSound(mActor,"SlipSandCurving",MR::abs(soundLevel),-1,-1);
        }
    }

    if (getPlayer()->fn_803B2730()) {
        if (!mSlideEffectActive) { playEffect("砂スライダー煙");mSlideEffectActive=true; }
    } else stopSlideEffect();
    return true;
}

bool MarioSlider::close() {
    if (getPlayer()->isSwimming()) {
        stopAnimation("スライダー尻");
    } else if (((getPlayer()->mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        stopAnimation("スライダー尻", "落下");
    } else {
        stopAnimation("スライダー尻", "基本");
    }

    stopSlideEffect();
    return true;
}

void clampSliderStick(f32* value) {
    f32 result;
    if (*value < -1.0f) result=-1.0f;
    else if (*value > 1.0f) result=1.0f;
    else result=*value;
    *value=result;
}
bool MarioSlider::pushSensor(HitSensor* sender,HitSensor* receiver) {
    const TVec3f& senderPos=*MR::getSensorPos(sender);
    const TVec3f& receiverPos=*MR::getSensorPos(receiver);
    TVec3f push=receiverPos-senderPos;
    if (mPushCooldown>0 && push.dot(_14)>0.0f) return false;
    _14+=push;mPushCooldown=2;return true;
}
void MarioSlider::stopSlideEffect() {
    if (mSlideEffectActive) {stopEffect("砂スライダー煙");mSlideEffectActive=false;}
}
