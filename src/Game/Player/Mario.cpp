#include "Game/Player/Mario.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Enemy/KarikariDirector.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Player/MarioAbyssDamage.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioAnimator.hpp"
#include "Game/Player/MarioBlown.hpp"
#include "Game/Player/MarioBump.hpp"
#include "Game/Player/MarioClimb.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Player/MarioCrush.hpp"
#include "Game/Player/MarioDamage.hpp"
#include "Game/Player/MarioDarkDamage.hpp"
#include "Game/Player/MarioFaint.hpp"
#include "Game/Player/MarioFireDamage.hpp"
#include "Game/Player/MarioFireDance.hpp"
#include "Game/Player/MarioFireRun.hpp"
#include "Game/Player/MarioFlip.hpp"
#include "Game/Player/MarioFlow.hpp"
#include "Game/Player/MarioFoo.hpp"
#include "Game/Player/MarioFpView.hpp"
#include "Game/Player/MarioFreeze.hpp"
#include "Game/Player/MarioFrontStep.hpp"
#include "Game/Player/MarioHang.hpp"
#include "Game/Player/MarioMagic.hpp"
#include "Game/Player/MarioMapCode.hpp"
#include "Game/Player/MarioMove.hpp"
#include "Game/Player/MarioParalyze.hpp"
#include "Game/Player/MarioRabbit.hpp"
#include "Game/Player/MarioRecovery.hpp"
#include "Game/Player/MarioSideStep.hpp"
#include "Game/Player/MarioSkate.hpp"
#include "Game/Player/MarioSlider.hpp"
#include "Game/Player/MarioStep.hpp"
#include "Game/Player/MarioStick.hpp"
#include "Game/Player/MarioStun.hpp"
#include "Game/Player/MarioSukekiyo.hpp"
#include "Game/Player/MarioSwim.hpp"
#include "Game/Player/MarioTalk.hpp"
#include "Game/Player/MarioTeresa.hpp"
#include "Game/Player/MarioWait.hpp"
#include "Game/Player/MarioWall.hpp"
#include "Game/Player/MarioWarp.hpp"
#include "Game/Util.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "math_types.hpp"
#include "Game/Player/MarioShadow.hpp"
#include "Game/Player/MarioState.hpp"
#include "Game/Player/FireMarioBall.hpp"
#include <revolution/types.h>
#include "Game/Util/MapUtil.hpp"
#include "Game/AreaObj/CubeCamera.hpp"
#include "Game/Map/CollisionCode.hpp"
#include "Game/Map/SunshadeMapHolder.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include <cstring>
#include <revolution/mtx.h>
#include "Game/Player/MarioAccess.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"
#include "Game/Player/MarioEffect.hpp"
#include "Game/Effect/MultiEmitter.hpp"
#include "Game/Effect/ParticleEmitter.hpp"
#include "Game/LiveActor/EffectKeeper.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <JSystem/JParticle/JPAEmitter.hpp>
#include <JSystem/JParticle/JPAMath.hpp>
#include <cstdio>
#include "Game/Player/MarioModule.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/MapObj/DashRing.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DirectDraw.hpp"
#include "Game/Util/DirectDrawUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include <JSystem/JUtility/JUTTexture.hpp>
#include "Game/Util/GamePadUtil.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JMath/JMATrigonometric.hpp>
#include "Game/AudioLib/AudAnmSoundObject.hpp"
#include "Game/AudioLib/AudSoundId.hpp"
#include "Game/System/ResourceHolder.hpp"
#include <JSystem/JAudio2/JAISound.hpp>
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/AreaObj/WaterArea.hpp"
#include "Game/Player/MarineSnow.hpp"
#include "Game/Screen/GameSceneLayoutHolder.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Animation/XanimeResource.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/MapObj/BigFanHolder.hpp"
#include "Game/Player/MarioParts.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JMath/JMath.hpp>
#include "Game/AreaObj/WarpCube.hpp"
#include "Game/MapObj/WarpPod.hpp"
#include "Game/Util/ActorMovementUtil.hpp"

// ---- Mario.cpp ----

void Mario_FORCE_MATCH_SDATA2() {
    (void)0.00100000005f;
    (void)0.800000012f;
    (void)-0.514999986f;
    (void)200.0f;
    (void)28000.0f;
    (void)2499.0f;
    (void)0.104719758f;
    (void)0.707000017f;
    (void)300.0f;
    (void)1.42857146f;
    (void)0.300000012f;
    (void)0.100000001f;
    (void)150.0f;
    (void)40.0f;
    (void)160.0f;
    (void)80.0f;
    (void)90.0f;
    (void)5.0f;
    (void)1.49599659f;
    (void)170.0f;
    (void)0.785398185f;
    (void)0.99000001f;
    (void)30.0f;
    (void)15.0f;
    (void)45.0f;
    (void)60.0f;
    (void)290.0f;
    (void)260.0f;
    (void)1.04719758f;
    (void)0.200000003f;
    (void)0.0245436933f;
    (void)0.0174532924f;
    (void)0.00999999978f;
    (void)32.0f;
    (void)0.125f;
    (void)100.0f;
    (void)35.0f;
    (void)50.0f;
    (void)0.00390625f;
    (void)120.0f;
    (void)0.0199999996f;
    (void)1.5f;
    (void)2607.59448f;
    (void)-2607.59448f;
    (void)-150.0f;
}


Mario::Mario(MarioActor* pActor) : MarioModule(pActor) {
    initMember();
    mVelocity.zero();
    _13C.zero();
    _148.zero();
    _154.zero();
    mFrontVec.set(1.0f, 0.0f, 0.0f);
    _220 = mFrontVec;
    _43C = mFrontVec;
    _214.set(1.0f, 0.0f, 0.0f);
    _22C.set(1.0f, 0.0f, 0.0f);
    _3E4.set(1.0f, 0.0f, 0.0f);
    _2B4 = 0.0f;
    mSideVec.set(0.0f, 0.0f, 1.0f);
    _290 = mSideVec;
    mAirGravityVec.set(0.0f, -1.0f, 0.0f);
    _1E4.zero();
    bool nearZero = MR::isNearZero(mAirGravityVec);
    mHeadVec = -mAirGravityVec;
    _1FC = mHeadVec;
    _938 = mHeadVec;
    _944 = mFrontVec;
    _950 = mSideVec;
    _60D = false;
    mTargetWalkSpeedIndex = 0;
    mWalkSpeed = 0.0f;
    _2D0 = 0.0f;
    mJumpVec.zero();
    _2E0.zero();
    _2EC.zero();
    _42C = 0;
    _2F8.zero();
    _328.zero();
    _334.zero();
    mVerticalSpeed = 0.0f;
    _430 = 0;
    _3BC = 0;
    _3BE = 0;
    _3C0 = 0;
    _3CE = 0;
    _3CC = 0;
    _3CA = 0;
    _3D0 = 0;
    _3D4 = 0;
    _424 = 0;
    _434 = 0;
    _436 = 0;
    _428 = 0;
    _3D2 = 0;
    _404 = 0;
    _406 = 0;
    _408 = 0;
    _40A = 0;
    _426 = 0;
    _418 = 0;
    _41A = 0;
    _774 = 0;
    _3C2 = 0;
    _3C4 = 0;
    _3C6 = 0;
    _3C8 = 0;
    _7D0 = 0x78;
    _524 = 0.0f;
    _528 = 0.0f;
    _52C = 0.0f;
    _530 = 0.0f;
    _534 = 0;
    _538 = 0.0f;
    _53C = 0.0f;
    _544 = 0;
    mYAngleOffset = 0.0f;
    _3F8 = 0;
    _3FA = 0;
    _3FC = 0;
    _3FE = 0;
    _400 = 0;
    _402 = 0;
    mPrevAnimFrame = 0.0f;
    mIceAnimFoot = 0;
    mSinkTimer = 0;
    mPoisonTimer = 0;
    _71E = 0;
    _414 = 0;
    mMorphResetTimer = 0;
    _41C = 0;
    _41E = 0;
    _410 = 0;
    _420 = 0;
    _43A = 0;
    _3F4 = 0.0f;
    _3F0 = 1.0f;
    _58.zero();
    _12C = 0;
    _76C = 0;
    _770 = 0.0f;
    _738 = 0.0f;
    _73C.set(0.0f, 1.0f, 0.0f);
    _368.set(0.0f, 1.0f, 0.0f);
    _374 = -_368;
    _380 = _368;
    _38C = -_368;
    _398.set(0.0f, 1.0f, 0.0f);
    _344.set(1.0f, 0.0f, 0.0f);
    _74C = 0.0f;
    _750 = 0;
    _754 = 0;
    _40C = 0;
    _40E = 0;
    _412 = 0;
    _558 = 0;
    _2A8 = -_368;
    _29C = _368;
    _548 = _128 = _124 = 0.0f;
    _54C = mHeadVec;
    PSMTXIdentity(_C4);
    PSMTXIdentity(_64);
    PSMTXIdentity(_94);
    clearSlope();
    _910.zero();
    _91C.zero();
    _928.zero();
    _934 = 0.8f;
    _3C = -0.515f;
    mMovementStates_LOW_WORD = 0;
    mMovementStates_HIGH_WORD = 0;
    _10_LOW_WORD = 0;
    _10_HIGH_WORD = 0;
    mDrawStates_WORD = 0;
    _1C_WORD = 0;
    _20_LOW_WORD = 0;
    _20_HIGH_WORD = 0;
    _28 = 0;

    _458 = MR::createTriangleFilterDelegator(this, &Mario::isIgnoreTriangle);

    _45C = new Triangle();
    _460 = new Triangle();
    mGroundPolygon = new Triangle();
    _468 = new Triangle();
    _46C = new Triangle();

    _4C8 = new Triangle();
    mFrontWallTriangle = new Triangle();
    mBackWallTriangle = new Triangle();
    mSideWallTriangle = new Triangle();
    _4D8 = new Triangle();
    _4DC = new Triangle();

    _470 = new Triangle();
    _474 = new Triangle();
    mTmpPolygon = new Triangle();

    _7E0 = new Triangle();

    _820 = new Triangle();

    _47C = new Triangle();
    _480 = new Triangle();
    _484 = new Triangle();

    for (u32 i = 0; i < ARRAY_SIZE(_57C); i++) {
        _57C[i] = new Triangle();
    }

    _578 = 0;

    _4E8.zero();
    _4F4.zero();

    _7D4.zero();
    _8A4.zero();

    _8C8 = new Triangle();

    _8E8 = new Triangle();

    _8CC[0] = new Triangle();
    _8CC[1] = new Triangle();

    _564 = -1;
    _568 = 0;
    _56C = 0;
    _570 = 0;

    for (u32 i = 0; i < ARRAY_SIZE(_A8C); i++) {
        _A8C[i].zero();
    }

    _4A4.zero();
    _4B0.zero();

    _25C.zero();
    _268.zero();

    _350.zero();
    _35C.zero();

    _184.zero();

    _1A8.zero();

    _190.zero();
    _19C.zero();

    _4E4 = 0.0f;
    _4E0 = 0.0f;

    _72C = 200.0f;

    _730 = nullptr;

    _8D8 = nullptr;
    _8D4 = nullptr;

    _A38 = 0;
    _A68 = 0;

    _5FC = 0;

    mWorldPadDir.zero();

    _38 = 0.0f;
    mStickPos.z = 0.0f;
    mStickPos.y = 0.0f;
    mStickPos.x = 0.0f;

    _3D8.zero();

    _244.zero();
    _238.zero();
    _250.zero();

    clear2DStick();

    _6AC = 0;
    _6AD = 0;
    _6B0.zero();
    _6BC.zero();

    mSide2D.set(1.0f, 0.0f, 0.0f);
    mUp2D.set(0.0f, 1.0f, 0.0f);
    mNormal2D.set(0.0f, 0.0f, 1.0f);
    _6A0 = mNormal2D;

    initSound();

    _97C = 0;

    mFlow = new MarioFlow(mActor);
    mWall = new MarioWall(mActor);
    mDamage = new MarioDamage(mActor);
    mFaint = new MarioFaint(mActor);
    mBlown = new MarioBlown(mActor);
    mHang = new MarioHang(mActor);
    mSwim = new MarioSwim(mActor);
    mSlider = new MarioSlider(mActor);
    mFireDamage = new MarioFireDamage(mActor);
    mFireRun = new MarioFireRun(mActor);
    mFireDance = new MarioFireDance(mActor);
    mAbyssDamage = new MarioAbyssDamage(mActor);
    mDarkDamage = new MarioDarkDamage(mActor);
    mStep = new MarioStep(mActor);
    mBump = new MarioBump(mActor);
    mParalyze = new MarioParalyze(mActor);
    mStun = new MarioStun(mActor);
    mCrush = new MarioCrush(mActor);
    mFreeze = new MarioFreeze(mActor);
    mMagic = new MarioMagic(mActor);
    mFpView = new MarioFpView(mActor);
    mRecovery = new MarioRecovery(mActor);
    mFlip = new MarioFlip(mActor);
    mSideStep = new MarioSideStep(mActor);
    mFrontStep = new MarioFrontStep(mActor);
    mStick = new MarioStick(mActor);
    mRabbit = new MarioRabbit(mActor);
    mSukekiyo = new MarioSukekiyo(mActor);
    mBury = new MarioBury(mActor);
    mWait = new MarioWait(mActor);
    mClimb = new MarioClimb(mActor);
    mSkate = new MarioSkate(mActor);
    mFoo = new MarioFoo(mActor);
    mWarp = new MarioWarp(mActor);
    mTeresa = new MarioTeresa(mActor);
    mTalk = new MarioTalk(mActor);
    mMove = new MarioMove(mActor);

    initTask();
    _95C = new FloorCode();
    _960 = 0;
    _962 = 0;
    for (int i = 0; i < ARRAY_SIZE(_964); i++) {
        _964[i] = _960;
    }

    _96A = 0;
    _574 = 0;
    _898 = false;
}


void Mario::updateAndClearStrideParameter() {
    _43C = mFrontVec;
    _16C = mVelocity;
    mPosition = mActor->mPosition;
    mVelocityAfter.zero();
    mVelocity.zero();
    _20_LOW_WORD = mMovementStates_LOW_WORD;
    _20_HIGH_WORD = mMovementStates_HIGH_WORD;
    _28 = _10_LOW_WORD;
    _2C = _10_HIGH_WORD;
    mPrevDrawStates = mDrawStates;
    _34 = _1C_WORD;
    mDrawStates_WORD = 0;
    _1C_WORD = 0;
    mMovementStates._28 = 0;
    if (mMovementStates._1) {
        _10._1B = 0;
        mWall->_1C = 0;
    }

    if (_10._25) {
        _1C._1 = 1;
    }

    if (_10._26) {
        _1C._2 = 1;
    }

    if (_10._24) {
        _1C._0 = 1;
    }

    _10._25 = 0;
    _10._26 = 0;
}


void Mario::checkKeyLock() {
    if (!mMovementStates.jumping) {
        mMovementStates._22 = false;
    }

    if (mActor->_37C < 0x4b) {
        mMovementStates._22 = true;
    }

    if (mActor->isEnableNerveChange() == false) {
        mMovementStates._22 = true;
    }

    if (isSwimming()) {
        mMovementStates._22 = false;
    }

    if (MR::testSubPadButtonC(WPAD_CHAN0)) {
        mDrawStates._7 = true;
    }
}


// Regswap
void Mario::updateTimers() {
    if (mMovementStates._1) {
        _3CE++;
    }

    if (_414 != 0) {
        _414--;
    }

    if (_434 != 0) {
        _434--;
    }

    if (mHang->mHangTimer != 0) {
        mHang->mHangTimer--;
    }
}


/*
void Mario::setFrontVecKeepUp(const TVec3f &rV)
{
    TVec3f stack_38(mFrontVec);
    if (!MR::isNearZero(rV)) {
        setFrontVec(rV);
    }

    TVec3f stack_2C = mHeadVec.cross(mFrontVec);

    if (MR::isNearZero(stack_2C)) {
        TVec3f stack_20;
        MR::vecBlendSphere(stack_38, rV, &stack_20, 0.5f);
        stack_2C.cross(mHeadVec, stack_20);
        setFrontVec(stack_20);
        if (MR::isNearZero(stack_2C)) {
            setFrontVecKeepSide(rV);
            return;
        }
    }

    MR::normalize(&stack_2C);
    mSideVec = stack_2C;
    MR::normalize(&mSideVec);
    setFrontVec(mSideVec.cross(mHeadVec));
    _22C = mFrontVec;
    _328 = mFrontVec % _328.length();
    _344 = mSideVec;
}

*/

void Mario::updateMorphResetTimer() {
    u16 timer = mMorphResetTimer;
    if (timer == 0) {
        return;
    }

    timer--;
    mMorphResetTimer = timer;
    if (timer != 0) {
        return;
    }

    mActor->setPlayerMode(0, true);
    return;
}


void Mario::doExtraServices() {
    TVec3f stack_2c;
    TVec3f stack_20;
    if (MR::diffAngleAbs(_43C, mFrontVec) < mActor->mConst->getTable()->mFrontAngleFixMargin) {
        setFrontVec(_43C);
    }

    bool notBee = !isPlayerModeBee();
    if (notBee) {
        bool notTeresa = !isPlayerModeTeresa();
        if (notTeresa && mMovementStates.jumping && _3BC > 600 && !MR::isNearZero(*getGravityVec())) {
            TVec3f vec(*getGravityVec() * MR::vecKillElement(mJumpVec, *getGravityVec(), &stack_2c));
            mJumpVec = vec;
        }
    }

    bool notFoo = !isPlayerModeFoo();
    if (notFoo) {
        bool notTeresa = !isPlayerModeTeresa();
        if (notTeresa && mMovementStates.jumping && _3BC > 300) {
            TVec3f stack_8(mPosition - *getLastSafetyTrans(nullptr));

            if (MR::isNearZero(mAirGravityVec)) {
            }

            if (MR::vecKillElement(stack_8, mAirGravityVec, &stack_20) > 28000.0f) {
                if (mVerticalSpeed > 2499.0f || mActor->isInZeroGravitySpot()) {
                    mActor->forceKill(0);
                }
            }
        }
    }

    if (getCurrentStatus() != 0 || mActor->_3C0 != 0 || mActor->_EA4) {
        _10._C = 0;
    }
}


bool Mario::isEnableCheckGround() {
    return !isStatusActive(MarioStatus_Swim);
}


void Mario::setGroundNorm(const TVec3f& rVec) {
    if (MR::isNearZero(mAirGravityVec)) {
    }

    if (MR::diffAngleAbs(-rVec, mAirGravityVec) > 1.4959966f) {
    } else {
        _368 = rVec;
        _374 = -rVec;
    }
}


void Mario::checkForceGrounding() {
    if ((mMovementStates._37 || _10._15) && mMovementStates._1 && MR::diffAngleAbs(_368, *_45C->getNormal(0)) < MR::toRadian(45.0f)) {
        mDrawStates._0 = true;
    }

    if (mMovementStates._8 || mMovementStates._19 || mMovementStates._1A) {
        return;
    }

    // does not match if this if statement is an early return
    if (!mMovementStates._15 && !mMovementStates._23 && !mMovementStates.debugMode && mMovementStates._1) {
        if (!mDrawStates._0) {
            TVec3f vec;

            f32 dot = getGravityVec()->dot(-_368);

            if (dot > 0.99f) {
                vec = *getGravityVec();
            } else {
                vec = -_368;
            }

            f32 dot2 = MR::vecKillElement(mVelocity, vec, &mVelocity);

            dot *= (mShadowPos - mPosition).dot(vec);

            if (MR::isNearZero(dot)) {
                return;
            }

            if (!(MR::abs(dot) < 30.0f)) {
                return;
            }

            if (mMovementStates._8) {
                dot2 = dot;
            }

            if (dot2 > dot) {
                dot2 = dot;
            }

            mVelocity += vec * dot2;

            return;
        }

        TVec3f vec2;
        f32 dot3 = MR::vecKillElement(mVelocity, *getGravityVec(), &vec2);
        f32 f4 = vec2.length();

        if (mVerticalSpeed && dot3 > mVerticalSpeed) {
            dot3 = mVerticalSpeed;
        }

        if (!mDrawStates._4) {
            f4 = MR::abs(dot3) > f4 ? 0.0f : MR::sqrt(f4 * f4 - dot3 * dot3);
        }

        MR::normalizeOrZero(&vec2);

        mVelocity = (vec2 * f4) + *getGravityVec() * dot3;
    }
}


void Mario::fixHeadFrontVecByGravity() {
    TVec3f _368Start(_368);

    if (MR::diffAngleAbs(mActor->mCamDirZ, mActor->_FA8) >= MR::toRadian(45.0f) && isStickOn()) {
        _60D = true;
        mDrawStates._D = true;

        _10._A = true;
        _40E = 0;
    }

    if (_3D0 != 0 || _3D2 != 0) {
        mDrawStates._D = false;
    }

    if (!_60D && mActor->_370) {
        mDrawStates._D = false;
    }

    if (isStatusActive(MarioStatus_Bump) || isStatusActive(MarioStatus_Climb)) {
        _60D = true;
    }

    TVec3f headVecCopy(mHeadVec);

    TVec3f minusGravDir(-*getGravityVec());
    MR::normalize(&minusGravDir);

    f32 rotateHeadVecSpeedByGravity;

    if (minusGravDir.dot(mHeadVec) < 0.0f) {
        if (mActor->_334 != 0) {
            f32 lerpFactor = (static_cast< f32 >(mActor->_334) / 15.0f);
            rotateHeadVecSpeedByGravity = (lerpFactor * mActor->getConst().getTable()->mRotateHeadVecSpeedByGravityM) +
                                          ((1.0f - lerpFactor) * mActor->getConst().getTable()->mRotateHeadVecSpeedByGravityL);
        } else {
            rotateHeadVecSpeedByGravity = mActor->getConst().getTable()->mRotateHeadVecSpeedByGravityL;
        }
    } else if (minusGravDir.dot(mHeadVec) < 0.99f) {
        rotateHeadVecSpeedByGravity = mActor->getConst().getTable()->mRotateHeadVecSpeedByGravityM;
    } else {
        rotateHeadVecSpeedByGravity = mActor->getConst().getTable()->mRotateHeadVecSpeedByGravityS;
    }

    if (_60D && mTargetWalkSpeedIndex != 0 && (mMovementStates._1 || !mMovementStates.jumping)) {
        if (mDrawStates._D) {
            MR::isNearZero(mAirGravityVec);

            TVec3f vec(mAirGravityVec);
            if (MR::normalizeOrZero(&vec)) {
                return;
            }

            vec = -vec;

            TVec3f cross;
            cross.cross(_290, vec);

            if (MR::normalizeOrZero(&cross)) {
                return;
            }

            TVec3f cross2;
            cross2.cross(vec, cross);

            if (MR::normalizeOrZero(&cross2)) {
                return;
            }

            _290 = cross2;
            mSideVec = _290;

            MR::normalize(&mSideVec);

            if (_410 < 30) {
                _410++;
            }
        }

        TVec3f newHeadVec;

        if (!MR::vecBlendSphere(mHeadVec, minusGravDir, &newHeadVec, rotateHeadVecSpeedByGravity)) {
            MR::vecRotAxis(mHeadVec, minusGravDir, mSideVec, &newHeadVec, rotateHeadVecSpeedByGravity);
        }

        mHeadVec = newHeadVec;
        MR::normalize(&mHeadVec);

        fixFrontVecFromUpSide();

        _29C = _368Start;

        _290 = mSideVec;
        return;
    }

    bool b1 = true;

    TVec3f cross3;
    cross3.cross(mHeadVec, minusGravDir);

    if (MR::isNearZero(cross3)) {
        if (MR::abs(minusGravDir.dot(mSideVec)) < MR::abs(minusGravDir.dot(mFrontVec))) {
            b1 = false;
        }
    } else if (MR::abs(cross3.dot(mFrontVec)) < MR::abs(cross3.dot(mSideVec))) {
        b1 = false;
    }

    if (b1) {
        TVec3f newHeadVec;

        if (!MR::vecBlendSphere(mHeadVec, minusGravDir, &newHeadVec, rotateHeadVecSpeedByGravity)) {
            MR::vecRotAxis(mHeadVec, minusGravDir, mFrontVec, &newHeadVec, rotateHeadVecSpeedByGravity);
        }

        mHeadVec = newHeadVec;
        MR::normalize(&mHeadVec);

        setFrontVecKeepUp(mFrontVec);

    } else {
        TVec3f newHeadVec;

        if (!MR::vecBlendSphere(mHeadVec, minusGravDir, &newHeadVec, rotateHeadVecSpeedByGravity)) {
            MR::vecRotAxis(mHeadVec, minusGravDir, mSideVec, &newHeadVec, rotateHeadVecSpeedByGravity);
        }

        mHeadVec = newHeadVec;
        MR::normalize(&mHeadVec);

        fixFrontVecFromUpSide();
    }

    if (MR::diffAngleAbs(_43C, mFrontVec) < mActor->getConst().getTable()->mFrontAngleFixMargin) {
        setFrontVec(_43C);
    }

    if (MR::diffAngleAbs(mHeadVec, headVecCopy) < mActor->getConst().getTable()->mHeadAngleFixMargin) {
        mHeadVec = headVecCopy;
        MR::normalize(&mHeadVec);
    }

    bool needsFix = _60D ? true : !mActor->_370;
    if (needsFix && mDrawStates._D && !MR::isNearZero(mHeadVec - _290) && !MR::isNearZero(mHeadVec + _290)) {
        mSideVec = _290;
        MR::normalize(&mSideVec);

        f32 angle = marioAcos(_368Start.dot(_29C));

        TVec3f cross4;
        cross4.cross(_29C, _368Start);
        MR::normalizeOrZero(&cross4);

        if (MR::isNearZero(cross4) || MR::isNearZero(_29C + _368Start) || MR::isNearZero(_29C - _368Start)) {
            _60D = true;
        } else {
            TMtx34f mtx;
            PSMTXRotAxisRad(mtx, cross4, angle);
            PSMTXMultVec(mtx, mSideVec, mSideVec);
            _29C = _368Start;
            _290 = mSideVec;
            _60D = false;
        }

        fixFrontVecFromUpSide();
        return;
    }

    TVec3f cross5;
    cross5.cross(_368Start, _22C);
    MR::normalizeOrZero(&cross5);

    if (!MR::isNearZero(cross5)) {
        _290 = cross5;
    } else {
        _290 = mSideVec;
    }

    _60D = false;
}


// Nearly matches
void Mario::createMtxDir(MtxPtr pMtx, const TVec3f& rFront, const TVec3f& rUp, const TVec3f& rSide) {
    TVec3f side;
    TVec3f front;
    TVec3f up;
    side = rSide;
    front = rFront;
    up = rUp;
    bool isNormal = !isPlayerModeTeresa();
    if (isNormal) {
        if (mMovementStates._37 || _10._15) {
            f32 sideAngleDiff = MR::diffAngleAbs(side, _950);
            f32 frontAngleDiff = MR::diffAngleAbs(front, _944);
            f32 upAngleDiff = MR::diffAngleAbs(up, _938);
            if (sideAngleDiff < mActor->mConst->getTable()->mVibrationAbsorbAngleS) {
                side = _950;
            } else {
                _950 = side;
            }

            if (frontAngleDiff < mActor->mConst->getTable()->mVibrationAbsorbAngleF) {
                front = _944;
            } else {
                _944 = front;
            }

            if (upAngleDiff < mActor->mConst->getTable()->mVibrationAbsorbAngleH) {
                up = _938;
            } else {
                _938 = up;
            }
        }
    }

    pMtx[0][0] = side.x;
    pMtx[1][0] = side.y;
    pMtx[2][0] = side.z;
    pMtx[0][1] = up.x;
    pMtx[1][1] = up.y;
    pMtx[2][1] = up.z;
    pMtx[0][2] = front.x;
    pMtx[1][2] = front.y;
    pMtx[2][2] = front.z;
}


bool Mario::isNonFixHeadVec() const {
    if (isStatusActive(MarioStatus_Swim)) {
        return true;
    }

    if (isStatusActive(MarioStatus_Wall)) {
        return true;
    }

    if (isStatusActive(MarioStatus_SideStep)) {
        return true;
    }

    if (isStatusActive(MarioStatus_Foo)) {
        return true;
    }

    if (isStatusActive(MarioStatus_Stick)) {
        return true;
    }

    if (mMovementStates._3E) {
        return true;
    }

    if (mActor->_EA5) {
        return true;
    }

    if (isStatusActive(MarioStatus_Talk)) {
        return false;
    }

    return mActor->_EA4;
}


void Mario::createDirectionMtx(MtxPtr pMtx) {
    TVec3f side;
    TVec3f up;
    TVec3f front;
    up = _1FC;
    side = mSideVec;
    front = mFrontVec;

    if (mDrawStates._11) {
        MR::normalize(&up);
        front.cross(side, up);
        MR::normalizeOrZero(&front);

        if (MR::isNearZero(front)) {
            front = mFrontVec;
        }

        side.cross(up, front);
        MR::normalizeOrZero(&side);

        if (MR::isNearZero(side)) {
            side = mSideVec;
        }
    } else {
        MR::normalizeOrZero(&up);
        side.cross(up, mFrontVec);
        MR::normalizeOrZero(&side);
        if (MR::isNearZero(side)) {
            side = mSideVec;
        }

        front.cross(side, up);
        MR::normalizeOrZero(&front);
        if (MR::isNearZero(front)) {
            front = mFrontVec;
        }
    }

    createMtxDir(pMtx, front, up, side);
    MR::setMtxTrans(pMtx, 0.0f, 0.0f, 0.0f);
}


bool Mario::createCorrectionMtx(MtxPtr pMtx, TVec3f* pOut) {
    pOut->zero();

    if (isNonFixHeadVec()) {
        PSMTXIdentity(pMtx);
        return false;
    }

    if (MR::isNearZero(_74C) && _754 == 0) {
        PSMTXIdentity(pMtx);
        _750 = 0;
        _74C = 0.0f;
    } else {
        if (_750 == 0 && _754 == 0) {
            TMtx34f tempMtx;
            TVec3f newFrontVec;
            PSMTXRotAxisRad(tempMtx, mHeadVec, _74C);
            PSMTXMultVec(tempMtx, mFrontVec, newFrontVec);
            setFrontVecKeepUp(newFrontVec);
            _74C = 0.0f;
        }

        TVec3f vec2(0.0f, 1.0f, 0.0f);

        if (mMovementStates._37 && !isStatusActive(MarioStatus_Talk)) {
            bool b1 = false;
            f32 dot = mFrontVec.dot(getCamDirX());

            if (_1FC.dot(getCamDirY()) < 0.0f) {
                dot = -dot;
            }

            if (dot < 0.0f && _74C < 0.0f) {
                b1 = true;
            } else if (dot > 0.0f && _74C > 0.0f) {
                b1 = true;
            }

            if (b1) {
                PSMTXRotAxisRad(pMtx, vec2, -_74C);
            } else {
                PSMTXRotAxisRad(pMtx, vec2, _74C);
            }
        } else {
            PSMTXRotAxisRad(pMtx, vec2, _74C);
        }

        if (_754 != 0) {
            _74C += (MR::pi() - _74C) / static_cast< f32 >(_754);
            _754--;
            _750 = 0;
        }

        if (_750 != 0) {
            _74C = _74C * static_cast< f32 >(_750 - 1) / static_cast< f32 >(_750);
            _750--;
            if (_750 == 0) {
                setFrontVecKeepUp(mFrontVec);
            }
        }
    }

    return true;
}


void Mario::slopeTiltHead(TVec3f* pVec) {
    if (mMovementStates._1) {
        if (_8F0 > 0.0f && !mMovementStates._23) {
            f32 blendFactor = _8F0 / 5.0f;
            if (blendFactor > 1.0f) {
                blendFactor = 1.0f;
            }

            MR::vecBlend(_368, *pVec, pVec, blendFactor);
            MR::normalize(pVec);
        } else if (mDrawStates._4) {
            MR::vecBlend(_368, *pVec, pVec, 0.2);
            MR::normalize(pVec);
        }
    }
}


void Mario::fixFrontVecByGravity() {
    if (isNonFixHeadVec()) {
        return;
    }

    if (MR::isNearZero(mAirGravityVec)) {
    }

    TVec3f up = -mAirGravityVec;
    MR::normalize(&up);
    if (_10._13) {
        TVec3f side(mPosition);
        side -= _6F4;
        MR::vecKillElement(side, _700, &side);
        MR::normalizeOrZero(&side);
        if (_344.dot(side) >= 0.0f) {
            _344 = side;
        } else {
            _344 = -side;
        }

        if (mSideVec.dot(_344) >= 0.0f) {
            mSideVec = _344;
            MR::normalize(&mSideVec);
        } else {
            // TVec3f stack_14 = -_344;
            mSideVec = -_344;
            MR::normalize(&mSideVec);
        }
    } else if (_1C._2) {
        TVec3f side2 = up.cross(mFrontVec);
        MR::normalizeOrZero(&side2);
        if (MR::isNearZero(side2) == false) {
            _344 = side2;
        }
    }

    TVec3f front = _344.cross(up);
    if (MR::normalizeOrZero(&front) == nullptr) {
        setFrontVec(front);
        _22C = mFrontVec;
        f32 _328mag = _328.length();
        _328 = mFrontVec * _328mag;
    }
}


void Mario::fixFrontVecFromUpSide() {
    TVec3f frontVec = mSideVec.cross(mHeadVec);
    MR::normalizeOrZero(&frontVec);
    if (MR::isNearZero(frontVec) == false) {
        setFrontVec(frontVec);
        _22C = mFrontVec;
        f32 scaleFactor = _328.length();
        _328 = mFrontVec * scaleFactor;
    }
}


void Mario::fixSideVecFromFrontUp() {
    TVec3f side = mHeadVec.cross(mFrontVec);
    MR::normalizeOrZero(&side);
    if (MR::isNearZero(side) == false) {
        mSideVec = side;
    }
}


void Mario::setSideVec(const TVec3f& rSide) {
    mSideVec = rSide;
    MR::normalize(&mSideVec);
}


void Mario::setHeadVec(const TVec3f& rHead) {
    mHeadVec = rHead;
    MR::normalize(&mHeadVec);
}


void Mario::setFrontVec(const TVec3f& rFront) {
    TVec3f plusMinus6A0;
    TVec3f frontWithout6A0;
    if (mMovementStates._37 && !isStatusActive(MarioStatus_Talk)) {
        if (getCamDirZ().dot(_6A0) > 0.0f) {
            plusMinus6A0 = _6A0;
        } else {
            TVec3f neg6A0 = -_6A0;
            plusMinus6A0 = neg6A0;
        }

        if (plusMinus6A0.dot(rFront) > 0.0f) {
            // maybe this reflects rFront around _6A0?? (very unsure)
            f32 f1 = MR::vecKillElement(rFront, plusMinus6A0, &frontWithout6A0);
            frontWithout6A0 -= plusMinus6A0 * f1;
            MR::normalizeOrZero(&frontWithout6A0);
            mFrontVec = frontWithout6A0;
        } else {
            mFrontVec = rFront;
        }
    } else {
        mFrontVec = rFront;
    }

    MR::normalize(&mFrontVec);
}


void Mario::setFrontVecKeepUp(const TVec3f& rFront, f32 blendFactor) {
    TVec3f blendedFront;
    if (MR::isNearZero(rFront) == false) {
        TVec3f normalizedFront(rFront);
        MR::normalize(&normalizedFront);
        s32 blend = 0;
        f32 angleDiff = MR::diffAngleAbs(mFrontVec, normalizedFront);
        if (angleDiff < HALF_PI) {
            blend = MR::vecBlendSphere(mFrontVec, normalizedFront, &blendedFront, blendFactor);
        }

        if (blend == 0) {
            MR::vecRotAxis(mFrontVec, normalizedFront, mHeadVec, &blendedFront, angleDiff * blendFactor);
        }

        setFrontVecKeepUp(blendedFront);
    }
}


void Mario::setFrontVecKeepUpAngle(const TVec3f& rFrontVec, f32 angle) {
    if (MR::isNearZero(rFrontVec) == false) {
        f32 angleDiff = MR::diffAngleAbs(mFrontVec, rFrontVec);
        if (angleDiff != 0.0f) {
            if (angleDiff < angle) {
                setFrontVecKeepUp(rFrontVec);
            } else {
                setFrontVecKeepUp(rFrontVec, angle / angleDiff);
            }
        }
    }
}


void Mario::setFrontVecKeepUp(const TVec3f& rFront, u32 _750_set) {
    Mtx rotMtx;
    TVec3f rotFrontVec;
    TVec3f rotVec2;
    PSMTXRotAxisRad(rotMtx, &mHeadVec, _74C);
    PSMTXMultVec(rotMtx, &mFrontVec, &rotFrontVec);
    f32 angleDiff = MR::diffAngleAbs(rFront, rotFrontVec);
    if (angleDiff > 0.024543693f) {
        PSMTXRotAxisRad(rotMtx, &mHeadVec, PI_180);
        PSMTXMultVec(rotMtx, &rotFrontVec, &rotVec2);
        f32 angleDiff2 = MR::diffAngleAbs(rFront, rotVec2);
        if (angleDiff2 < angleDiff) {
            _74C = -angleDiff;
        } else {
            _74C = angleDiff;
        }
    }

    setFrontVecKeepUp(rFront);
    if (_750_set != 0) {
        _750 = _750_set;
    }
}


void Mario::setFrontVecKeepUp(const TVec3f& rFront) {
    TVec3f front = mFrontVec;
    TVec3f side;
    TVec3f blendedFront;
    if (MR::isNearZero(rFront) == false) {
        setFrontVec(rFront);
    }

    side.cross(mHeadVec, mFrontVec);
    if (MR::isNearZero(side)) {
        MR::vecBlendSphere(front, rFront, &blendedFront, 0.5f);
        side.cross(mHeadVec, blendedFront);
        setFrontVec(blendedFront);
        if (MR::isNearZero(side)) {
            setFrontVecKeepSide(rFront);
            return;
        }
    }

    MR::normalize(&side);
    mSideVec = side;
    MR::normalize(&mSideVec);
    TVec3f stack_14 = mSideVec.cross(mHeadVec);
    setFrontVec(stack_14);
    _22C = mFrontVec;
    f32 _328mag = _328.length();
    _328 = mFrontVec * _328mag;
    _344 = mSideVec;
}


void Mario::setFrontVecKeepSide(const TVec3f& rFront) {
    TVec3f headVec;
    TVec3f front;
    if (MR::isNearZero(rFront) == false) {
        setFrontVec(rFront);
    }

    headVec.cross(mFrontVec, mSideVec);
    if (MR::normalizeOrZero(&headVec) != nullptr) {
        const TVec3f* gravity = getGravityVec();
        TVec3f up = -(*gravity);
        mHeadVec = up;
        MR::normalize(&mHeadVec);
    } else {
        mHeadVec = headVec;
        MR::normalize(&mHeadVec);
    }

    front.cross(mSideVec, mHeadVec);
    setFrontVec(front);
    _22C = mFrontVec;
    f32 _328mag = _328.length();
    _328 = mFrontVec * _328mag;
}


void Mario::setHeadAndFrontVecFromRotate(const TVec3f& rRotate) {
    TVec3f frontVec = TVec3f(0.0f, 0.0f, 1.0f);
    TVec3f headVec = TVec3f(0.0f, 1.0f, 0.0f);
    Mtx mtx;
    MR::makeMtxTR(mtx, 0.0f, 0.0f, 0.0f, rRotate.x, rRotate.y, rRotate.z);
    PSMTXMultVec(mtx, (Vec*)&frontVec, (Vec*)&frontVec);
    PSMTXMultVecSR(mtx, (Vec*)&headVec, (Vec*)&headVec);
    mHeadVec = headVec;
    MR::normalize(&mHeadVec);
    setFrontVecKeepUp(frontVec);
}


void Mario::forceSetHeadVecKeepSide(const TVec3f& rVec) {
    mHeadVec = rVec;
    _1FC = rVec;
    if (MR::isSameDirection(rVec, mSideVec)) {
        fixSideVecFromFrontUp();
    }

    fixFrontVecFromUpSide();
    _43C = mFrontVec;
    _750 = 0;
}


// float regswap + bad stack ordering
void Mario::draw() const {
    if (mDrawStates._1) {
        TDDraw::cameraInit3D();
        TDDraw::setup(0, 1, 0);
        TDDraw::drawCylinder(mPosition, mShadowPos - mActor->_2A0, 32.0f, 0xffff0010, 0xff000004, 8);
        TVec3f diff2A0_31C(mActor->_2A0 - mShadowPos);
        f32 diffMag = diff2A0_31C.length();
        MR::normalize(&diff2A0_31C);
        TVec3f _31CPlusStuff(mShadowPos);
        _31CPlusStuff += diff2A0_31C * diffMag * 0.125f * (mActor->_37C % 0xa) * 0.1f;
        f32 one_eighth = 0.125f;
        for (u32 i = 0; i < 8; i++) {
            TDDraw::drawCircle(_31CPlusStuff, diff2A0_31C, 100.0f, 0xffffff70, 0x10);
            _31CPlusStuff += diff2A0_31C * diffMag * one_eighth;
        }
    }

    if (_97C != nullptr) {
        _97C->draw3D();
    }

    drawTask();
}


void Mario::addTrans(const TVec3f& rShift, const char* pA2) {
    mPosition += rShift;
    TVec3f _148shift(_148);
    _148shift -= rShift;
    if (MR::abs(_148.x) <= MR::abs(_148shift.x)) {
        _148.x = 0.0f;
    }

    if (MR::abs(_148.y) <= MR::abs(_148shift.y)) {
        _148.y = 0.0f;
    }

    if (MR::abs(_148.z) <= MR::abs(_148shift.z)) {
        _148.z = 0.0f;
    }
}


void Mario::setTrans(const TVec3f& rShift, const char* pA2) {
    TVec3f reqShift(rShift);
    reqShift -= mPosition;
    mPosition = rShift;
    TVec3f _148shift(_148);
    _148shift -= reqShift;
    if (MR::abs(_148.x) <= MR::abs(_148shift.x)) {
        _148.x = 0.0f;
    }

    if (MR::abs(_148.y) <= MR::abs(_148shift.y)) {
        _148.y = 0.0f;
    }

    if (MR::abs(_148.z) <= MR::abs(_148shift.z)) {
        _148.z = 0.0f;
    }
}


bool Mario::isEnableRush() const {
    if (isHanging()) {
        return false;
    }

    if (isDamaging()) {
        return false;
    }

    if (isStatusActive(MarioStatus_Flow)) {
        return false;
    }

    if (mMovementStates._B && mMovementStates.jumping) {
        return false;
    }

    return true;
}


bool Mario::isInvincible() const {
    if (isPlayerModeInvincible()) {
        return true;
    }

    if (mMovementStates._1F) {
        return true;
    } else {
        return mActor->_3D8 != false;
    }
}


void Mario::inputStick() {
    mActor->getStickValue(&mStickPos.x, &mStickPos.y);
    if (_10._28) {
        _10._28 = false;
        mStickPos.x = 0.0f;
        mStickPos.y = 0.0f;
    }

    mStickPos.x = 1.5f * mStickPos.x;
    mStickPos.y = 1.5f * mStickPos.y;

    mStickPos.x = MR::clamp(mStickPos.x, -1.0f, 1.0f);
    mStickPos.y = MR::clamp(mStickPos.y, -1.0f, 1.0f);

    _38 = mStickPos.z;

    mStickPos.z = MR::sqrt(mStickPos.x * mStickPos.x + mStickPos.y * mStickPos.y);
    mStickPos.z = MR::clamp(mStickPos.z, 0.0f, 1.0f);

    if (MR::isNearZero(mStickPos.z, 0.01f)) {
        mStickPos.z = 0.0f;
    }

    f32 angle = MR::normalizeAngleAbs(MR::atan2(mStickPos.y, mStickPos.x));
    f32 quarterTurn = HALF_PI;
    f32 stickAngleMargin = mActor->getConst().getTable()->mStickAngleMargin;
    u32 quotient = 0;
    while (angle > quarterTurn) {
        angle -= quarterTurn;
        quotient++;
    }

    f32 resultAngle;

    if (angle >= quarterTurn - stickAngleMargin) {
        resultAngle = quarterTurn;
    } else if (angle <= stickAngleMargin) {
        resultAngle = 0.0f;
    } else {
        resultAngle = (quarterTurn / (quarterTurn - 2.0f * stickAngleMargin)) * (angle - stickAngleMargin);
    }

    resultAngle += quarterTurn * static_cast< f32 >(quotient);

    if (!getPlayer()->_10._11) {
        mStickPos.x = mStickPos.z * MR::cos(resultAngle);
        mStickPos.y = mStickPos.z * MR::sin(resultAngle);
    }

    _524 = _528;
    _528 = resultAngle;

    if (mMovementStates._37) {
        stick2DadjustGround(mStickPos.x, mStickPos.y);
    }

    if (_10._15) {
        stick2Dadjust(mStickPos.x, mStickPos.y);
    }

    if (mMovementStates._3A) {
        update25Dmode();
        updateAxisFromMode(_6AC);
    }

    if (_10._13) {
        mStickPos.y = 0.0f;
    }

    if (mStickPos.z <= 0.01f) {
        mMovementStates._1D = false;
    }

    calcWorldPadDir(&mWorldPadDir, mStickPos.x, mStickPos.y, false);
    MR::normalizeOrZero(&mWorldPadDir);
}


bool Mario::isCeiling() const {
    if (_4C8->isValid() == false) {
        return false;
    } else {
        return !MR::isNearZero(_3A4);
    }
}


void Mario::setGravityVec(const TVec3f& rGravity) {
    if (MR::isSameDirection(mAirGravityVec, rGravity)) {
        if (MR::isOppositeDirection(mAirGravityVec, rGravity)) {
            _10._24 = 1;
            _10._25 = 1;
        }

        _1E4.zero();
    } else {
        _1E4.cross(mAirGravityVec, rGravity);
        MR::normalizeOrZero(&_1E4);
        f32 frontDot = MR::abs(mFrontVec.dot(_1E4));
        f32 sideDot = MR::abs(mSideVec.dot(_1E4));
        f32 headDot = MR::abs(mHeadVec.dot(_1E4));
        if (frontDot > sideDot && frontDot > headDot) {
            _10._A = 0;
            _10._26 = 1;
        }

        _10._24 = 1;
    }

    TVec3f stack_8 = -_368;
    _374 = stack_8;
    mAirGravityVec = rGravity;
}


void Mario::updateSoundCode() {
    s32 soundCode = -1;
    Mario* pPlayer = getPlayer();
    if (pPlayer->mMovementStates._1) {
        if (mGroundPolygon->isValid()) {
            soundCode = MR::getSoundCodeIndex(mGroundPolygon->getAttributes());
        }
    } else if (_45C->isValid()) {
        soundCode = MR::getSoundCodeIndex(_45C->getAttributes());
    }

    if (isStatusActive(MarioStatus_Wall)) {
        if (mMovementStates._8) {
            soundCode = MR::getSoundCodeIndex(mFrontWallTriangle->getAttributes());
        } else if (mMovementStates._19) {
            soundCode = MR::getSoundCodeIndex(mBackWallTriangle->getAttributes());
        } else if (mMovementStates._1A) {
            soundCode = MR::getSoundCodeIndex(mSideWallTriangle->getAttributes());
        }
    }

    MR::setMapSondCodeGravity(mActor, soundCode);
}


bool Mario::isForceStopRush() const {
    if (mVerticalSpeed == 0.0f) {
        switch (_960) {
        case 1:
        case 4:
        case 10:
        case 15:
        case 16:
        case 24:
        case 129:
            return true;
        }
    }

    return false;
}


const TVec3f& Mario::getShadowNorm() const {
    return *_45C->getNormal(0);
}


const TVec3f& Mario::getAirGravityVec() const {
    if (MR::isNearZero(this->mAirGravityVec)) {
    }

    return mAirGravityVec;
}


const TVec3f Mario::getAirFrontVec() const {
    TVec3f stack_8;
    MR::vecKillElement(mFrontVec, mAirGravityVec, &stack_8);
    if (MR::normalizeOrZero(&stack_8)) {
        return mFrontVec;
    } else {
        return stack_8;
    }
}


void Mario::initAfterConst() {
    MarioConst* pConst = mActor->mConst;
    _402 = pConst->getTable()->mAirWalkTime;
    mPosition = mActor->mPosition;
    mShadowPos = mPosition;
    mGroundPos = mPosition;
    mMove->initAfter();
    mFoo->init();
    mSwim->init();
    _7D4 = mPosition;
    _814 = mPosition;
    PSMTXIdentity(_7E4);
    PSMTXIdentity(_824);
    _720 = getAnimationStringPointer("ヒップドロップ開始");
    _724 = getAnimationStringPointer("ヒップドロップ");
    _728 = getAnimationStringPointer("ヒップドロップ着地");
}


void Mario::writeBackPhyisicalVector() {
    f32 slowdown;
    s32 planeCount;
    u32 i;
    if (_1C._10) {
        TVec3f stack_104(mPosition);
        addTrans(mVelocity, nullptr);
        if (calcDistToCeil(false) < 200.0f) {
            mVelocity.zero();
            stopWalk();
        }

        mPosition = stack_104;
    }

    if (mMovementStates._37 || _10._15) {
        TVec3f stack_f8(mPosition - _688);
        MR::vecKillElement(stack_f8, _6A0, &stack_f8);
        mPosition = _688 + stack_f8;
        MR::vecKillElement(mVelocity, _6A0, &mVelocity);
        TVec3f stack_ec;
        f32 f29 = MR::vecKillElement(_694 - _688, _6A0, &stack_ec);
        if (f29 > 1.0f) {
            f29 = 1.0f;
        } else if (f29 < -1.0f) {
            f29 = -1.0f;
        }

        stack_ec = _6A0;
        stack_ec.setLength(f29);
        _688 += stack_ec;
    }

    fixPositionInTower();
    if (mMovementStates.jumping) {
        bool rising = false;
        if (isRising()) {
            rising = true;
        }

        if (mActor->_334 != 0) {
            _578 = 0;
        }

        if (!mMovementStates._22) {
            for (i = 0; i < _578; i++) {
                TVec3f stack_e0;
                if (!(MR::vecKillElement(mVelocity, *_57C[i]->getNormal(0), &stack_e0) < 0.0f)) {
                    continue;
                }

                mVelocity = stack_e0;

                if (!_1C._D) {
                    TVec3f stack_d4;
                    f32 f28 = MR::vecKillElement(mJumpVec, *_57C[i]->getNormal(0), &stack_d4);
                    if (f28 <= 0.0f) {
                        mJumpVec = stack_d4 + *_57C[i]->getNormal(0) * f28 * 0.5f;
                    }
                } else if (MR::diffAngleAbs(*getGravityVec(), mJumpVec) < MR::toRadian(6.0f)) {
                    TVec3f stack_c8;
                    MR::vecKillElement(*_57C[i]->getNormal(0), *getGravityVec(), &stack_c8);
                    f32 _2D4Mag = mJumpVec.length();
                    MR::normalizeOrZero(&stack_c8);
                    mJumpVec += stack_c8;
                    mJumpVec.setLength(_2D4Mag);
                }

                if (rising) {
                    if (_57C[i]->getNormal(0)->dot(*getGravityVec()) > 0.707f) {
                        mActor->sendMsgUpperPunch(_57C[i]->mSensor);
                    }
                }

                if (isPlayerModeBee() && mActor->mBeeWallWalk == 0) {
                    const char* wallCodeString = MR::getWallCodeString(_57C[i]);
                    if (wallCodeString != nullptr && strcmp(wallCodeString, "Fur") == 0) {
                        Triangle stack_110 = Triangle();
                        MarioActor* pActor = mActor;
                        TVec3f stack_bc;
                        if (MR::getFirstPolyOnLineToMap(&stack_bc, &stack_110, pActor->_2A0, -(*_57C[i]->getNormal(0)) * 300.0f)) {
                            wallCodeString = MR::getWallCodeString(&stack_110);
                            if (wallCodeString != nullptr && strcmp(wallCodeString, "Fur") == 0) {
                                MarioActor* pActor2 = getActor();
                                pActor2->entryWallWalkMode(stack_bc, *stack_110.getNormal(0));
                            }
                        }
                    }
                }
            }
        }
    } else if (isSwimming()) {
        if (!mMovementStates._22) {
            for (u32 j = 0; j < _578; j++) {
                TVec3f stack_b0;
                if (MR::vecKillElement(mVelocity, *_57C[j]->getNormal(0), &stack_b0) < 0.0f) {
                    mVelocity = stack_b0;
                    f32 f28 = MR::vecKillElement(mSwim->mKnockbackVel, *_57C[j]->getNormal(0), &mSwim->mKnockbackVel);
                    mSwim->mKnockbackVel += *_57C[j]->getNormal(0) * f28 * 0.5f;
                }
            }
        }
    } else if (!_1C._C && mActor->_288.dot(_368) < 0.707f && !MR::isNearZero(mActor->_288, 1.0f)) {
        TVec3f stack_a4(mActor->_288);
        MR::normalizeOrZero(&stack_a4);
        TVec3f stack_98;
        f32 f2 = MR::vecKillElement(mVelocity, stack_a4, &stack_98);
        if (f2 < 0.0f) {
            mVelocity = stack_98;
            MR::normalizeOrZero(&stack_a4);
            slowdown = 1.0f - MR::clamp((-stack_a4.dot(mFrontVec) - 0.3f) * 1.4285715f, 0.0f, 1.0f);
            if (!mMovementStates._8 && slowdown < 0.1f) {
                if (mMovementStates._1A) {
                    mVelocity += getSideWallNorm() * -f2;
                } else {
                    planeCount = mActor->mBinder->mPlaneNum;
                    if (planeCount != 0) {
                        f32 f28 = 0.0f;
                        u32 index = 0;
                        i = 0;
                        for (; i < planeCount; i++) {
                            f32 dot = mActor->mBinder->getPlane(i)->mParentTriangle.getNormal(0)->dot(mFrontVec);
                            if (dot < f28) {
                                index = i;
                                f28 = dot;
                            }
                        }

                        const TVec3f& hitPos = mActor->mBinder->getPlane(index)->_64;
                        TVec3f stack_8c;
                        MR::vecKillElement(mPosition - hitPos, *getGravityVec(), &stack_8c);
                        MR::vecKillElement(stack_8c, stack_a4, &stack_8c);
                        if (!MR::isNearZero(stack_8c)) {
                            stack_8c.setLength(-f2);
                            mVelocity += stack_8c;
                        }
                    }
                }
            }

            mTargetWalkSpeedIndex = 0;
            mWalkSpeed *= slowdown;
        }
    }

    mActor->mVelocity = mVelocity;
    mActor->mPosition = mPosition;
}


void Mario::update() {
    OSGetTime();
    updateAndClearStrideParameter();
    checkKeyLock();

    if (!(mMovementStates.jumping && mMovementStates._B) && _728 != nullptr && isAnimationRun(_728)) {
        mMovementStates.jumping = true;
        mMovementStates._B = true;
    }

    updateCubeCode();
    mMovementStates._7 = updateBinderInfo();

    checkEnforceMove();

    _A8C[0] = mVelocity;

    if (!isStatusActive(MarioStatus_Stick) && !isStatusActive(MarioStatus_Hang)) {
        checkBaseTransPoint();
        checkHeadPoint();
    }

    if (mMovementStates._F && _544 > 1) {
        createAtField(true, 150.0f);
    } else {
        createAtField(false, 40.0f);
    }

    _72C = calcDistToCeil(true);
    _1C._F = _72C < 160.0f;

    if (_1C._F && _4C8->isValid()) {
        damagePolygonCheck(_4C8);
    }

    if (MR::getKarikariClingNum() != 0) {
        _1C._5 = true;
    }

    OSGetTime();

    f32 f1 = 80.0f;

    if (isSwimming()) {
        f1 = 90.0f;
    }

    checkAllWall(mActor->_2A0, f1);

    OSGetTime();
    updateGroundInfo();
    OSGetTime();

    if (getMovementStates()._1 && !(mMovementStates.jumping && !mMovementStates._B)) {
        if (damageFloorCheck()) {
            writeBackPhyisicalVector();
            return;
        }

        saveLastSafetyTrans();

        if (isCurrentShadowFloorDangerAction()) {
            TVec3f result;
            TVec3f groundToShadow(mShadowPos - mGroundPos);
            MR::isNearZero(mAirGravityVec);
            if (MR::vecKillElement(groundToShadow, mAirGravityVec, &result) >= 5.0f && groundToShadow.dot(mFrontVec) > 0.0f) {
                mDrawStates._A = true;
                groundToShadow.setLength(5.0f);
                addVelocity(groundToShadow);
            }
        }
    }

    mDrawStates._10 = true;

    if (damageWallCheck()) {
        return;
    }

    mDrawStates._10 = false;

    OSGetTime();

    if (checkPressDamage()) {
        return;
    }

    OSGetTime();

    if (checkSliderMode()) {
        startSlider();
    }

    checkAndTryForceJump();

    if (mMovementStates._2E) {
        return;
    }

    if (checkStartSwim()) {
        writeBackPhyisicalVector();
        return;
    }

    if (!mMovementStates._22) {
        check2DMode();
    }

    _898 = false;

    if (!mMovementStates.debugMode && MR::testFpViewStartTrigger()) {
        if (MR::isPossibleToShiftToFirstPersonCamera()) {
            tryFpViewMode();
        } else if (!MR::isDemoActive()) {
            if (!MR::isEqualStageName("EpilogueDemoStage")) {
                MR::startSystemSE("SE_SY_CAMERA_NG");
            }

            _898 = true;
        }
    }

    inputStick();
    checkLockOnHoming();

    _A8C[1] = mVelocity;

    actionMain();

    _A8C[3] = mVelocity;

    calcFrontFloor();

    if (mMovementStates._23 && mMovementStates._1 && mMovementStates._24 && isSlipPolygon(_460)) {
        f32 dot = MR::vecKillElement(mVelocity, *_460->getNormal(0), &mVelocity);
        TVec3f minus((_368 - *_460->getNormal(0)) * 0.5f);
        mVelocity += minus * MR::abs(dot);
    }

    _A8C[4] = mVelocity;

    tryPushToVelocity();
    powerAreaMove();
    powerRailMove();

    _A8C[5] = mVelocity;

    addVelocity(mVelocityAfter);

    _A8C[6] = mVelocity;

    checkForceGrounding();

    _A8C[7] = mVelocity;

    checkStep();
    checkBump();
    doCubeWarp();

    if (isSwimming()) {
        checkBaseTransBall();
    }

    doSpinPunchAroundPolygons();

    _A8C[8] = mVelocity;

    writeBackPhyisicalVector();
    updateTimers();
    doExtraServices();
}


void Mario::actionMain() {
    if (!mMovementStates.debugMode) {
        if (checkDamage() == false) {
            if (_97C) {
                sendStateMsg(MarioStateMsg_Update);
            } else if (mMovementStates.jumping) {
                procJump(false);
                checkTornado();
                checkHang();
                checkWallStick();
            } else {
                mainMove();
                updateWalkSpeed();
            }

            if (mMovementStates._1) {
                mMovementStates._30 = 1;
                mMovementStates._31 = 1;
                doFrontStep();
            }
        }

        if (!mMovementStates._23) {
            _1A8 = mVelocity;
            _3C2 = 0;
            _280 = 0.0f;
            _284.zero();
        } else {
            _3C2++;
        }

        if (_3C4 != 0) {
            _3C4--;
        }

        _A8C[2] = mVelocity;
        if (mMovementStates._1 && !mMovementStates.jumping) {
            if (isEnableSlopeMove()) {
                slopeMove();
            } else {
                clearSlope();
            }
        }

        if (isStickOn() && isAnimationRun("水泳陸うちあげ")) {
            stopAnimation(nullptr);
        }

        if (!mDrawStates._4) {
            _3FE = 0;
        }

        callExtraTasks(0);
        beeMarioOnGround();
        checkSpecialWaitAnimation();
        if (!mDrawStates._16) {
            retainMoveDir(mStickPos.x, mStickPos.y, nullptr);
        }
    }
}


void Mario::updateGroundInfo() {
    checkMap();
    if (isEnableCheckGround()) {
        bool b1 = !mMovementStates.jumping && !mMovementStates._1;

        mMovementStates._1 = checkGround();

        if (mDrawStates._18) {
            _3C6++;
        } else {
            _3C6 = 0;
        }

        if (b1 && mMovementStates._1 && isDefaultAnimationRun("落下")) {
            changeAnimation(nullptr, "基本");
        }

        if (!isStatusActive(MarioStatus_13)) {
            if (!isStatusActive(MarioStatus_13) && !mMovementStates._1 && mMovementStates._3E == 1 && mJumpVec.dot(*getGravityVec()) > 0.0f &&
                mJumpVec.dot(*_45C->getNormal(0)) < 0.0f && _1FC.dot(*getGravityVec()) > 0.0f && mVerticalSpeed < 170.0f) {
                mMovementStates._1 = true;
                setGroundNorm(*_45C->getNormal(0));
                setTrans(mShadowPos, nullptr);
            }
        }
    }

    updateCameraPolygon();

    if (mMovementStates._1) {
        mMovementStates._29 = 0;
        if (_43A == 0 && isSlipPolygon(mGroundPolygon)) {
            mDrawStates._C = 1;
        }
    }

    if (_43A != 0) {
        _43A--;
    }

    updateFloorCode();
    updateSoundCode();
    updateOnSand();
    updateOnWater();
    updateOnPoison();
}


bool Mario::postureCtrl(MtxPtr) {
    TVec3f vec1(_1FC);
    TVec3f vec2(mHeadVec);
    f32 blend;
    f32 f1 = mActor->getConst().getTable()->mHeadRotateRatio;
    if (mMovementStates._1) {
        bool noSlip = !isSlipFloorCode(_960);
        if (noSlip) {
            if (mTargetWalkSpeedIndex < 3) {
                if (calcAngleD(*mGroundPolygon->getNormal(0)) < 45.0f) {
                    MR::isNearZero(mAirGravityVec);
                    vec2 = -mAirGravityVec;
                } else {
                    f32 f2 = calcAngleD(*mGroundPolygon->getNormal(0)) - 45.0f;
                    MR::isNearZero(mAirGravityVec);
                    MR::vecBlendSphere(-mAirGravityVec, vec2, &vec2, f2 / 60.0f);
                }
            } else {
                MR::isNearZero(mAirGravityVec);
                vec2 = -*getGravityVec() - mAirGravityVec;
                MR::normalizeOrZero(&vec2);
            }
        }
    }

    if (isAnimationRun("坂すべり上向きうつぶせ", 2)) {
        vec2 = _368;
    }

    if (isStatusActive(MarioStatus_Wait)) {
        if (isAnimationRun("特殊ウエイト1A")) {
            if (getAnimator()->getFrame() > 290.0f) {
                blend = 1.0f;
            } else if (getAnimator()->getFrame() > 260.0f) {
                blend = 0.5f + 0.5f * ((getAnimator()->getFrame() - 260.0f) / 30.0f);
            } else {
                blend = 0.5f * (getAnimator()->getFrame() / 260.0f);
            }

            MR::vecBlendSphere(vec2, -_374, &vec2, blend);
        } else if (_1C._13 && _1C._14) {
            vec2 = -_374;
        }
    }

    if (!isSwimming() && (isAnimationRun("水泳ジェット") || isAnimationRun("水泳スピン移動") || isAnimationRun("リングダッシュ") ||
                          isAnimationRun("水泳スピンジャンプ"))) {
        MR::vecBlendSphere(mHeadVec, mJumpVec, &vec2, 0.1f);

        if (MR::normalizeOrZero(&vec2)) {
            vec2 = mHeadVec;
        }

        mDrawStates._11 = true;
    }

    calcTornadoTilt();

    slopeTiltHead(&vec2);

    isStatusActive(MarioStatus_Climb);

    if (MR::diffAngleAbs(vec1, vec2) > mActor->getConst().getTable()->mHeadAngleFixMargin && !MR::vecBlendSphere(vec1, vec2, &vec1, f1)) {
        TMtx34f mtx;
        PSMTXRotAxisRad(mtx, &mSideVec, 0.1f);
        PSMTXMultVecSR(mtx, vec1, &vec1);
    }

    if (!MR::normalizeOrZero(&vec1)) {
        _1FC = vec1;
    }

    return false;
}


void Mario::createAngleMtx(MtxPtr pMtx, bool a2) {
    f32 zero;
    TVec3f vec1;
    TVec3f vec2;
    TVec3f vec3;
    TVec3f vec4;
    if (!(isNonFixHeadVec() || a2 || mMovementStates.debugMode)) {
        fixHeadFrontVecByGravity();
    }

    if (updatePosture(pMtx)) {
        return;
    }

    createDirectionMtx(pMtx);

    vec2 = _1FC;
    vec1 = mSideVec;
    vec3 = mFrontVec;

    PSMTXConcat(_C4, pMtx, pMtx);

    HitSensor* sensor = mActor->_470;

    if (isAnimationRun("投げ")) {
        sensor = mActor->_474;
    }

    if (mActor->isPunching()) {
        sensor = mActor->_924;
    }

    if (sensor != nullptr) {
        vec4 = sensor->mPosition;
    }

    if (sensor != nullptr) {
        TVec3f vec5;
        vec5.set(vec4 - mPosition);

        MR::normalizeOrZero(&vec5);

        if (MR::isNearZero(vec5)) {
            return;
        }

        TVec3f vec6(vec2);

        TVec3f vec7;
        MR::vecKillElement(vec5, vec6, &vec7);

        if (MR::normalizeOrZero(&vec7)) {
            return;
        }

        f32 angle = JMAAcosRadian(vec7.dot(vec3));

        TVec3f cross;
        cross.cross(vec3, vec7);

        if (cross.dot(vec2) < 0.0f) {
            angle = -angle;
        }

        f32 pitchAngle = JMAAcosRadian(vec7.dot(vec5));

        bool dotNonNeg = vec5.dot(vec6) >= 0.0f;

        if (MR::abs(angle) < HALF_PI && dotNonNeg) {
            pitchAngle = -pitchAngle;
        } else if (MR::abs(angle) >= HALF_PI && !dotNonNeg) {
            pitchAngle = -pitchAngle;
        }

        f32 f1 = 0.1f * static_cast< f32 >(_412);
        zero = 0.0f;

        if (f1 > MR::toRadian(60.0f)) {
            f1 = MR::toRadian(60.0f);
        }

        _58.x = MR::clamp(angle, -f1, f1);
        _58.z = zero;
        _58.y = 0.0f;

        MR::orderRotateMtx(0, _58, _64);
        MR::orderRotateMtx(0, -_58, _94);
        _12C = 16;
        return;
    } else if (_12C != 0) {
        _12C--;
    }

    _58.x *= 0.8f;
    _58.z *= 0.8f;
    _58.y = 0.0f;

    MR::orderRotateMtx(0, _58, _64);
    MR::orderRotateMtx(0, -_58, _94);
}


void Mario::updateLookOfs() {
    TVec3f vec;
    vec.zero();

    if (isStatusActive(MarioStatus_Blown)) {
        return;
    }

    if (mActor->_934) {
        MR::vecBlend(_13C, vec, &_13C, 0.3f);
    }

    if (_97C == nullptr) {
        if (isAnimationRun("壁押し")) {
            TVec3f vec2;
            MR::vecKillElement(_4E8 - mPosition, mHeadVec, &vec2);
            f32 len = vec2.length();
            vec = *mFrontWallTriangle->getNormal(0) * (len - 35.0f);
        }
    } else if (_97C->mStatusId == MarioStatus_Swim) {
        vec += -*getGravityVec() * mSwim->mSurfaceOffset;
    }

    if (mMovementStates._1 && !mMovementStates.jumping && mVerticalSpeed > 5.0f && mVerticalSpeed < 50.0f && !isCurrentShadowFloorDangerAction()) {
        TVec3f dirShadowToGround(mGroundPos - mShadowPos);
        MR::normalizeOrZero(&dirShadowToGround);
        if (dirShadowToGround.dot(_368) < 0.1f) {
            vec += *getGravityVec() * mVerticalSpeed;
        } else {
            TVec3f vec3;
            f32 f1 = MR::vecKillElement(mGroundPos - mPosition, *getGravityVec(), &vec3);
            vec += *getGravityVec() * f1;
        }
    }

    TVec3f vec4;
    vec4.zero();

    if (mSinkTimer != 0) {
        vec4 = *getGravityVec() * static_cast< f32 >(mSinkTimer) * (1.0f / 256.0f) * 120.0f;
        vec4 = -_1FC * static_cast< f32 >(mSinkTimer) * (1.0f / 256.0f) * 120.0f;
    }

    f32 blendRate = 0.2f;

    if (!isUseSimpleGroundCheck()) {
        if (getAnimator()->isLandingAnimationRun()) {
            blendRate = 0.01f;
        } else if (mTargetWalkSpeedIndex == 0 && mWalkSpeed < 0.2f && isBlendWaitGround()) {
            if (_148.length() > vec.length()) {
                blendRate = 0.5f;
            } else {
                blendRate = 0.02f * static_cast< f32 >(_3CE);

                if (blendRate >= 0.2f) {
                    blendRate = 0.2f;
                }
            }
        }
    }

    MR::vecBlend(_148, vec, &_148, blendRate);
    MR::vecBlend(_154, vec4, &_154, 0.2f);
    _13C = _148 + _154;
}


// conditionals won't behave
const TVec3f* Mario::getGravityVec() const {
    if (isStatusActive(MarioStatus_Bump) || isStatusActive(MarioStatus_Climb)) {
        return &_790;
    }

    if (mMovementStates._1) {
        bool noSlip = !isSlipFloorCode(_960);
        if (noSlip) {
            if (mActor->mBeeWallWalk != 0) {
                return &mAirGravityVec;
            }

            if (isAnimationRun("ハード着地")) {
                return &mAirGravityVec;
            }

            if (_3CE < 0x10 && _3BC > 3 && !mMovementStates._B) {
                return &mAirGravityVec;
            }

            if (MR::isNearZero(_374) == false && _1C._13 && _1C._14) {
                return &_374;
            }
        }
    }

    if (isSwimming()) {
        if (mSwim->mWaterDepth >= -150.0f && mDrawStates._F && MR::isNearZero(_38C) == false) {
            return &_38C;
        }

        return &mAirGravityVec;
    }

    if (MR::isNearZero(mAirGravityVec)) {
    }

    bool b1 = true;
    if (_430 == 0xc) {
        b1 = false;
    } else if (_430 == 0xd) {
        b1 = false;
    }

    bool noSlip = !isSlipFloorCode(_960);
    if (noSlip && b1) {
        bool noHopper = !isPlayerModeHopper();
        if (noHopper) {
            bool noTeresa = !isPlayerModeTeresa();
            if (noTeresa && !isDamaging() && !isStatusActive(MarioStatus_Swim) && !isStatusActive(MarioStatus_Blown) &&
                !isStatusActive(MarioStatus_13) && _430 != 5 && mActor->_334 == 0 && mMovementStates.jumping && !mMovementStates._22 && _3BC < 8) {
                return &_374;
            }
        }
    }

    return &mAirGravityVec;
}


void Mario::touchWater() {
    if (mActor->isActionOk("水解除") && mMorphResetTimer == 0) {
        mMorphResetTimer = 10;
    }
}


XjointTransform* (XanimeCore::*Mario_FORCE_MATCH_JOINT_TRANSFORM())(u32) {
    return &XanimeCore::getJointTransform;
}

// ---- Mario2D.cpp ----

void FORCE_ADD() {
    TVec3f vec;
    TVec3f vec2;
    vec + vec2;
}


void Mario::stick2DadjustGround(f32& rX, f32& rY) {
    if (isStageCameraRotate2D()) {
        return;
    }

    if (_10.turning && mMovementStates._1 && !mMovementStates.jumping) {
        _10.turning = false;
        clear2DStick();
        _66C = false;
    }

    mSide2D.set(1.0f, 0.0f, 0.0f);
    mUp2D.set(0.0f, 1.0f, 0.0f);
    mNormal2D.set(0.0f, 0.0f, -1.0f);

    PSMTXMultVecSR(_F4, mSide2D, mSide2D);
    PSMTXMultVecSR(_F4, mNormal2D, mNormal2D);
    PSMTXMultVecSR(_F4, mUp2D, mUp2D);

    _6A0 = mNormal2D;

    MR::vecKillElement(_368, _6A0, &_660);

    if (MR::normalizeOrZero(&_660)) {
        _660 = _368;
    }

    f32 dot = _660.dot(mUp2D);

    if (!MR::isInRange(dot, -0.707f, 0.707f)) {
        set2Dmode(dot >= 0.0f);
    }

    TVec3f stickDir2D(rX, rY, 0.0f);
    MR::normalizeOrZero(&stickDir2D);

    if (_66C) {
        if (MR::isNearZero(mStickPos.z)) {
            _66C = false;
            return;
        }

        if (MR::diffAngleAbs(stickDir2D, _670) > 0.3f) {
            _66C = false;
            return;
        }

        PSMTXMultVecSR(MR::tmpMtxRotZRad(-MR::diffAngleSignedHorizontal(_67C, _660, _6A0)), stickDir2D, stickDir2D);
        rX = stickDir2D.x;
        rY = stickDir2D.y;
        return;
    }

    if (MR::isNearZero(mStickPos.z)) {
        return;
    }

    TVec3f vec2;
    vec2.x = _660.dot(getCamDirX());
    vec2.y = _660.dot(getCamDirY());
    vec2.z = 0.0f;

    f32 diff = MR::diffAngleAbs(stickDir2D, vec2);

    // explicit conversions to radian instead of calling MR::toRadian()
    if (diff < 30.0f * MR::pi() / 180.0f || diff > 150.0f * MR::pi() / 180.0f) {
        rX = 0.0f;
        rY = 0.0f;
        mStickPos.z = 0.0f;
        return;
    }

    _670 = stickDir2D;
    _67C = _660;
    _66C = true;
}


void Mario::calcDir2D(f32 x, f32 y, TVec3f* pOut) {
    TVec3f xDir;
    TVec3f yDir;
    MR::vecKillElement(getCamDirX(), _6A0, &xDir);
    MR::vecKillElement(getCamDirY(), _6A0, &yDir);

    MR::normalizeOrZero(&xDir);
    MR::normalizeOrZero(&yDir);

    pOut->set(xDir * x);
    pOut->add(yDir * y);
}

// ---- MarioBee.cpp ----

void Mario::beeMarioOnGround() {
    if (getPlayerMode() == PlayerMode_Bee && !mDrawStates._C && mMovementStates._1 && !mMovementStates._23 && !isStatusActive(MarioStatus_Slider)) {
        getPlayer()->incAirWalkTimer();
        getPlayer()->incAirWalkTimer();
        getPlayer()->incAirWalkTimer();
        getPlayer()->incAirWalkTimer();
    }
}


bool Mario::beeMarioOnAir() {
    if (_774) {
        _774--;
        if (!_774) {
            mMovementStates._2F = false;
        }
    }

    if (getPlayerMode() == PlayerMode_Bee) {
        if (_3BC == 1 && _402) {
            _402--;
        }

        if (isAnimationRun("ハチ壁ジャンプ") && isAnimationTerminate(nullptr)) {
            if (checkLvlA()) {
                changeAnimation("ハチ飛行中", "落下");
            } else {
                changeAnimation("ハチ飛行中無入力", "落下");
            }

            changeAnimationInterpoleFrame(30);
        }
    }

    if (getPlayerMode() == PlayerMode_Bee) {
        TVec3f horizontal;
        if (MR::vecKillElement(mJumpVec, getAirGravityVec(), &horizontal) > -5.0f || _76C) {
            if (!_774) {
                f32 verticalSpeed = cutGravityElementFromJumpVec(true);
                f32 speed = mJumpVec.length();
                MR::normalizeOrZero(&mJumpVec);
                if (!MR::vecBlendSphere(mJumpVec, mFrontVec, &mJumpVec, mActor->getConst().getTable()->mBeeSpeedRotateRatio)) {
                    Mtx rotation;
                    PSMTXRotAxisRad(rotation, &mHeadVec, 0.1f);
                    PSMTXMultVecSR(rotation, &mJumpVec, &mJumpVec);
                }

                mJumpVec.setLength(speed);
                mJumpVec += getAirGravityVec() * verticalSpeed;
            }

            if (!mMovementStates._11 && _402) {
                _406 = 16;
                if (_76C < 30) {
                    _76C = 30;
                    _770 = 0.0f;
                }

                mMovementStates._12 = true;
                _4B0 = mPosition;
                mMovementStates._11 = true;
            }

            s16 inhibitTime = mActor->getConst().getTable()->mBeeAirWalkInhibitTime;
            s16 powerTime = mActor->getConst().getTable()->mBeeGravityPowerTime;
            TVec3f horizontalVelocity;
            MR::vecKillElement(mJumpVec, getAirGravityVec(), &horizontalVelocity);
            if (horizontalVelocity.length() < 5.0f) {
                inhibitTime = mActor->getConst().getTable()->mBeeAirWalkInhibitTimeV;
                powerTime = mActor->getConst().getTable()->mBeeGravityPowerTimeV;
            }

            if (checkLvlA() && _402 && _3BC > inhibitTime) {
                playSound("ハチ飛行中");
                if (!MR::isNearZero(mStickPos.z)) {
                    setFrontVecKeepUp(getWorldPadDir(), mActor->getConst().getTable()->mBeeAirWalkTurnSpd);
                }

                u16 previousTime = _402;
                if (_402) {
                    if (!mMovementStates._F && _402 > mActor->getConst().getTable()->mAirWalkTime) {
                        _402 = mActor->getConst().getTable()->mAirWalkTime;
                    }

                    _402--;
                }

                if (!_402) {
                    if (previousTime) {
                        playSound("ハチ体力切れ");
                    }

                    mMovementStates._11 = false;
                    stopAnimation("ハチ飛行中");
                } else {
                    if (!mMovementStates._F) {
                        if (!isAnimationRun("ハチ壁ジャンプ") && !isAnimationRun("ハチスピン空中")) {
                            changeAnimation("ハチ飛行中", "落下");
                        }

                        cancelSquatMode();
                        playSound("空中ふんばり");
                        if (_402 < mActor->getConst().getTable()->mAirWalkTime / 2) {
                            getAnimator()->setSpeed(1.5f);
                        }

                        if (_430 == 4) {
                            setFrontVecKeepUp(-_220);
                            _430 = 0;
                        }

                        if (_430 == 5) {
                            _430 = 0;
                        }
                    }

                    s16 time = static_cast< s16 >(_408);
                    if (time > powerTime) {
                        time = powerTime;
                    }

                    f32 ratio = static_cast< f32 >(time) / powerTime;
                    ratio = 0.9f * (ratio * ratio);
                    f32 acceleration = 15.0f * (MR::getRandom() - ratio);
                    acceleration =
                        acceleration * mActor->getConst().getTable()->mBeeFlyRandomFactor - mActor->getConst().getTable()->mBeeFlyConstantFactor;
                    f32 verticalSpeed = cutGravityElementFromJumpVec(true);
                    TVec3f velocity(mJumpVec);
                    f32 factor = 1.0f;
                    if (!MR::isNearZero(mStickPos.z)) {
                        velocity.dot(getWorldPadDir());
                    }

                    if (factor < 0.0f) {
                        factor *= mActor->getConst().getTable()->mBeeUpAccelRatio;
                    }

                    _770 += factor * (acceleration * mActor->getConst().getTable()->mBeeAccelRatio);
                    if (_770 > 0.0f) {
                        _770 *= mActor->getConst().getTable()->mBeeUpDownKiller;
                    }

                    if (!_774) {
                        if (!getPlayer()->_1C._5) {
                            if (_770 < -mActor->getConst().getTable()->mBeeUpSpeedMax) {
                                _770 = -mActor->getConst().getTable()->mBeeUpSpeedMax;
                            }
                        } else if (_770 < -0.5f) {
                            _770 = -0.5f;
                        }
                    }

                    addVelocity(*getGravityVec(), _770);
                    if (!getPlayer()->_1C._5 && verticalSpeed > 0.0f) {
                        verticalSpeed *= mActor->getConst().getTable()->mBeePushRiseGravityEraser;
                    }

                    mJumpVec += getAirGravityVec() * verticalSpeed;
                    _408++;
                    if (_408 > 120) {
                        _408 = 120;
                    }

                    _4B0 = mPosition;
                    return true;
                }
            } else {
                if (!isAnimationRun("ハチジャンプ") && !isAnimationRun("ハチ壁ジャンプ")) {
                    stopAnimation("ハチ飛行中");
                    if (!isAnimationRun(nullptr) || isAnimationTerminate(nullptr)) {
                        changeAnimation("ハチ飛行中無入力");
                    }
                }

                if (_408) {
                    _408--;
                }

                if (_770 < mActor->getConst().getTable()->mBeeFreeDropMaxSpd) {
                    _770 += mActor->getConst().getTable()->mBeeFreeDropAcc;
                }

                f32 factor = 1.0f;
                addVelocity(*getGravityVec(), _770 * factor);
                if (!MR::isNearZero(mStickPos.z)) {
                    setFrontVecKeepUp(getWorldPadDir(), mActor->getConst().getTable()->mBeeAirWalkTurnSpd);
                }
            }
        }
    }

    return false;
}

// ---- MarioBlown.cpp ----

bool Mario::blown(const TVec3f& rVec) {
    if (getCurrentStatus() == MarioStatus_Blown) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_Damage) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_13) {
        return false;
    }

    stopWalk();

    mBlown->vecKillActor240(rVec);

    mMovementStates._2C = true;
    mMovementStates._A = false;
    _430 = 3;

    return true;
}

// ---- MarioBump.cpp ----

void Mario::checkBump() {
    if (!mMovementStates.jumping && !mMovementStates._A && mMovementStates._1) {
        if (!isStatusActive(MarioStatus_Hang) && !isStatusActive(MarioStatus_Wall) && !isStatusActive(MarioStatus_Swim) &&
            !isStatusActive(MarioStatus_Climb)) {
            if (!isDamaging() && getPlayerMode() != 5 && !isSwimming()) {
                if (isStickOn()) {
                    if (!_750 && (mMovementStates._23 == 0)) {
                        if (_4DC->isValid()) {
                            const char* floor = MR::getFloorCodeString(_4DC);

                            if (!isSlipPolygon(_4DC) || !strcmp(floor, "NoSlip") || !strcmp(floor, "Sand")) {
                                startBump(_518);
                            }
                        }
                    }
                }
            }
        }
    }
}


void Mario::startBump(const TVec3f& rVec) {
    if ((getTrans() - rVec).length() >= 100.0f) {
        return;
    }

    TVec3f v21;
    const TVec3f* grav = &getAirGravityVec();
    const TVec3f* norm = _4DC->getNormal(0);
    MR::vecKillElement(*norm, *grav, &v21);

    if (!MR::normalizeOrZero(&v21) && MR::diffAngleAbsHorizontal(mFrontVec, -v21, getAirGravityVec()) > 0.52359879f) {
        TVec3f v20;
        if (MR::vecKillElement(getTrans() - rVec, getAirGravityVec(), &v20) > 0.0f) {
            return;
        }
    }

    if (getCurrentStatus() == 30) {
        closeStatus(mBump);
    }

    setTrans(rVec, "段差");
    changeStatus(mBump);
    mVelocity.zero();
    _3D0 = 0;
    mMovementStates._10 = 0;
    _790 = -(*_4DC->getNormal(0));
    TVec3f v19;
    MR::vecKillElement(-(*_4DC->getNormal(0)), mSideVec, &v19);

    if (!MR::normalizeOrZero(&v19)) {
        _790 = v19;
    }
}

// ---- MarioClimb.cpp ----

void Mario::connectToClimb() {
    _790 = -getShadowNorm();
    changeStatus(mClimb);
}

// ---- MarioCollision.cpp ----

void MarioCollision_FORCE_MATCH_SDATA2() {
    (void)0.001f;
    (void)3e+01f;
    (void)0.3f;
    (void)0.707f;
    (void)1.01f;
    (void)-0.99f;
    (void)5.0f;
    (void)4e+01f;
    (void)1.1e+02f;
    (void)5e+01f;
    (void)6e+01f;
    (void)-0.707f;
    (void)45.0f;
    (void)0.2f;
    (void)1.2e+02f;
    (void)8e+01f;
    (void)0.7853982f;
    (void)1e+02f;
    (void)4e+02f;
    (void)1e+01f;
    (void)0.1f;
    (void)2.5e+03f;
    (void)1.5e+02f;
    (void)0.10471976f;
    (void)2.3561945f;
    (void)0.17f;
    (void)1.4959966f;
    (void)-3e+01f;
    (void)2e+02f;
    (void)-1e+01f;
    (void)5e+03f;
    (void)25.0f;
    (void)1.8e+02f;
    (void)3.14159f;
    (void)-0.999f;
    (void)-0.9f;
    (void)2.0943952f;
    (void)6.0f;
    (void)-6.0f;
    (void)55.0f;
    (void)0.99f;
    (void)-0.01f;
    (void)8.0f;
    (void)2e+01f;
    (void)2.1e+02f;
    (void)-2607.5945f;
    (void)2607.5945f;
    (void)15.0f;
    (void)2e+03f;
    (void)0.125f;
}


void MarioCollision_FORCE_MATCH_STRINGS(const TVec3f& rPosition) {
    MR::getAreaObj("Fur", rPosition);
    MR::getAreaObj("めりこみ", rPosition);
    MR::getAreaObj("空中一回転", rPosition);
    MR::getAreaObj("Spine1", rPosition);
    MR::getAreaObj("マンホールのふた(クッパ船)", rPosition);
    MR::getAreaObj("GhostThroughCode", rPosition);
    MR::getAreaObj("崖ふんばり", rPosition);
    MR::getAreaObj("壁押し", rPosition);
    MR::getAreaObj("前方WKFALL", rPosition);
    MR::getAreaObj("+逆-左WKFALL", rPosition);
    MR::getAreaObj("+左WKFALL", rPosition);
    MR::getAreaObj("+逆-右WKFALL", rPosition);
    MR::getAreaObj("+右WKFALL", rPosition);
    MR::getAreaObj("+後ろ1WKFALL", rPosition);
    MR::getAreaObj("+後ろ2WKFALL", rPosition);
    MR::getAreaObj("+SIDEFALL", rPosition);
    MR::getAreaObj("-L-SIDEFALL", rPosition);
    MR::getAreaObj("+R-SIDEFALL", rPosition);
    MR::getAreaObj("+F-SIDEFALL", rPosition);
    MR::getAreaObj("force Trans", rPosition);
    MR::getAreaObj("force Trs2", rPosition);
    MR::getAreaObj("force Trs3", rPosition);
    MR::getAreaObj("no-g(back)", rPosition);
    MR::getAreaObj("no-g", rPosition);
    MR::getAreaObj("CubeCamera", rPosition);
    MR::getAreaObj("PullBackCube", rPosition);
    MR::getAreaObj("PullBackCylinder", rPosition);
    MR::getAreaObj("PlaneCollisionCube", rPosition);
    MR::getAreaObj("TowerModeCylinder", rPosition);
    MR::getAreaObj("ForbidTriangleJumpCube", rPosition);
    MR::getAreaObj("GlaringLightArea", rPosition);
    MR::getAreaObj("FallsCube", rPosition);
    MR::getAreaObj("水壁ヒット", rPosition);
    MR::getAreaObj("HeavySteeringCube", rPosition);
    MR::getAreaObj("DashChargeCylinder", rPosition);
    MR::getAreaObj("RasterScrollCube", rPosition);
    MR::getAreaObj("ForbidJumpCube", rPosition);
    MR::getAreaObj("DarkMatterCube", rPosition);
}


bool Mario::isIgnoreTriangle(const Triangle* pTriangle) {
    return MR::isNearZero(pTriangle->getNormal(0)->dot(*getGravityVec()));
}


void Mario::checkBaseTransBall() {
    TVec3f center = mPosition + mVelocity + mHeadVec * 50.0f;
    u32 count = Collision::checkStrikeBallToMap(center, 50.0f, nullptr, nullptr);

    for (u32 i = 0; i < count; i++) {
        doSwimmingHitCheck(Collision::getStrikeInfoMap(i), 0);
    }

    center = mPosition + mVelocity - mHeadVec * 40.0f;
    count = Collision::checkStrikeBallToMap(center, 50.0f, nullptr, nullptr);

    for (u32 i = 0; i < count; i++) {
        doSwimmingHitCheck(Collision::getStrikeInfoMap(i), 1);
    }

    center = mPosition + mVelocity;
    count = Collision::checkStrikeBallToMap(center, 40.0f, nullptr, nullptr);

    for (u32 i = 0; i < count; i++) {
        doSwimmingHitCheck(Collision::getStrikeInfoMap(i), 2);
    }
}


void Mario::createAtField(bool force, f32 radius) {
    u32 hits;
    _578 = 0;
    f32 checkRadius = 50.0f;
    f32 offset = 50.0f;
    u32 count = 2;

    if (isSwimming()) {
        checkRadius = 40.0f;
        offset = 0.0f;
    }

    if (mMovementStates._A) {
        checkRadius = 40.0f;
        count = 1;
        offset = 40.0f;
    }

    if (force) {
        checkRadius = radius;

        if (radius < 40.0f) {
            checkRadius = 40.0f;
        }

        offset = 0.0f;
    }

    f32 minAngle = 60.0f;
    f32 maxAngle = 120.0f;

    for (u32 i = 0; i < count; i++) {
        TVec3f center = mPosition - getAirGravityVec() * offset * (i + 1);

        if (force && mMovementStates._F && _544 > 1) {
            hits = Collision::checkStrikeBallToMap(center, checkRadius, nullptr, nullptr);
        } else {
            hits = Collision::checkStrikeBallToMapWithThickness(center, checkRadius, checkRadius, nullptr, nullptr);
        }

        for (u32 j = 0; j < hits; j++) {
            const HitInfo* hit = Collision::getStrikeInfoMap(j);
            const Triangle* triangle = &hit->mParentTriangle;

            if (force && mMovementStates._F && _544 > 1) {
                if (calcAngleD(*triangle->getNormal(0)) < minAngle) {
                    continue;
                }

                if (calcAngleD(*triangle->getNormal(0)) > maxAngle) {
                    continue;
                }

                TVec3f horizontal;

                if (MR::abs(MR::vecKillElement(hit->_64 - center, getAirGravityVec(), &horizontal)) > 80.0f) {
                    continue;
                }

                if (hit->isCollisionAtEdge() && MR::diffAngleAbsHorizontal(mJumpVec, -*triangle->getNormal(0), getAirGravityVec()) < 0.7853982f) {
                    continue;
                }
            }

            if (!force && !isSwimming()) {
                if (triangle->mSensor != nullptr && mActor->selectPushOff(triangle->mSensor)) {
                    continue;
                }

                if (isThroughWall(triangle)) {
                    continue;
                }

                const TVec3f* normal = triangle->getNormal(0);
                f32 angle = _95C->getCode(triangle) == CollisionFloorCode_NoSlip ? 60.0f : 45.0f;

                if (mMovementStates._37) {
                    if (!MR::isSameMtx(*triangle->getBaseMtx(), *triangle->getPrevBaseMtx()) && mGroundPolygon->mSensor != triangle->mSensor) {
                        angle = -1.0f;
                        push(*normal * hit->_60);
                    }
                }

                if (calcAngleD(*normal) < angle) {
                    continue;
                }
            }

            *_57C[_578] = *triangle;
            _578++;

            if (_578 == 32) {
                break;
            }
        }

        if (_578 == 32) {
            break;
        }
    }
}


void Mario::doSwimmingHitCheck(const HitInfo* pHit, u32 type) {
    const Triangle* triangle = &pHit->mParentTriangle;

    if (MR::isThroughPolygon(triangle)) {
        return;
    }

    if (isThroughWall(triangle)) {
        return;
    }

    if (damagePolygonCheck(triangle)) {
        return;
    }

    TVec3f velocity(mVelocity);
    const TVec3f* normal = triangle->getNormal(0);

    if (velocity.dot(-*normal) < 0.0f) {
        return;
    }

    mSwim->addVelocity(*normal, pHit->_60);

    if (type == 0) {
        _97C->hitWall(*normal, triangle->mSensor);
    } else {
        _97C->hitPoly(type, *normal, triangle->mSensor);
    }
}


void Mario::doSpinPunchAroundPolygons() {
    if (!mActor->isPunching()) {
        return;
    }

    if (!mActor->isInPunchTimerRange()) {
        return;
    }

    TVec3f center = mPosition + mVelocity + mHeadVec * 80.0f;
    Triangle triangles[256];
    u32 count = MR::createAreaPolygonList(triangles, 256, center + mFrontVec * 120.0f + mSideVec * 120.0f + mHeadVec * 120.0f,
                                          center - mFrontVec * 120.0f - mSideVec * 120.0f - mHeadVec * 120.0f);

    for (u32 i = 0; i < count; i++) {
        sendPunch(triangles[i].mSensor, true);
    }
}


void Mario::checkMap() {
    calcShadowPos();

    if (isStatusActive(MarioStatus_Swim)) {
        mGroundPos = mShadowPos;
    }

    TVec3f gravity(*getGravityVec());

    if (mMovementStates._1 && isSlipPolygon(mGroundPolygon)) {
        gravity = _374;
    }

    if (isUseSimpleGroundCheck()) {
        if (!mMovementStates._14) {
            mVerticalSpeed = (mGroundPos - mPosition).dot(gravity);
        } else {
            mVerticalSpeed = (mShadowPos - mPosition).dot(gravity);
        }

        if (!_20._36) {
            _148.zero();
        }

    } else {
        mVerticalSpeed = (mShadowPos - mPosition).dot(gravity);
    }

    if (mVerticalSpeed < 0.0f) {
        mVerticalSpeed = 0.0f;
    }

    if (mMovementStates._23 && calcAngleD(_368) >= 30.0f) {
        TVec3f start = mPosition + _368 * 100.0f;
        mMovementStates._24 = MR::getFirstPolyOnLineToMap(&_498, _460, start, -_904 * 150.0f);
    } else {
        mMovementStates._24 = false;
    }
}


f32 Mario::calcDistToCeil(bool saveSensor) {
    Triangle triangle;
    TVec3f hitPos;
    f32 offset = 30.0f;

    if (saveSensor) {
        _730 = nullptr;
    }

    if (isStatusActive(MarioStatus_Hang)) {
        offset = 0.0f;
    }

    while (true) {
        TVec3f start = mPosition + getAirGravityVec() * offset;
        bool hit = MR::getFirstPolyOnLineToMap(&hitPos, &triangle, start, -getAirGravityVec() * (200.0f + offset));

        if (isThroughWall(&triangle)) {
            hit = false;
        }

        if (!hit) {
            break;
        }

        bool pressGround = false;

        if (mGroundPolygon->mSensor != nullptr && MR::isSensorPressObj(mGroundPolygon->mSensor)) {
            pressGround = true;
        }

        if (!MR::isSensorPressObj(triangle.mSensor) && _95C->getCode(&triangle) != CollisionFloorCode_Press && !pressGround &&
            _960 != CollisionFloorCode_Press) {
            if ((hitPos - mPosition).dot(*getGravityVec()) >= 0.0f) {
                if (offset <= 1.0f) {
                    offset = -10.0f;
                    continue;
                }

                return 200.0f;
            }
        }

        _3B0 = *triangle.getNormal(0);

        if (saveSensor) {
            _730 = triangle.mSensor;
        }

        f32 distance = (hitPos - mPosition).length();

        if (!_4C8->isValid()) {
            *_4C8 = triangle;
        }

        return distance;
    }

    return 200.0f;
}


f32 Mario::calcDistToCeilOnPress() {
    if (!_480->isValid() || !_484->isValid()) {
        return 0.0f;
    }

    TVec3f horizontal;
    f32 distance1 = MR::vecKillElement(mPosition - *_480->calcAndGetPos(0), *_480->calcAndGetNormal(0), &horizontal);
    f32 distance2 = MR::vecKillElement(mPosition - *_484->calcAndGetPos(0), *_484->calcAndGetNormal(0), &horizontal);

    if (_480->getNormal(0)->dot(mAirGravityVec) < 0.0f) {
        f32 distance = MR::vecKillElement(*_480->calcAndGetPos(0) - mPosition, *_480->calcAndGetNormal(0), &horizontal);
        mPosition += *_480->getNormal(0) * distance;
    } else {
        f32 distance = MR::vecKillElement(*_484->calcAndGetPos(0) - mPosition, *_484->calcAndGetNormal(0), &horizontal);
        mPosition += *_484->getNormal(0) * distance;
    }

    mActor->mPosition.set(mPosition);

    if (distance1 > distance2) {
        return distance2;
    }

    return distance1;
}


f32 Mario::calcDistToCeilHead() {
    Triangle triangle;
    TVec3f hitPos;
    TVec3f start = mActor->_2AC - mFrontVec * 40.0f;

    if (MR::getFirstPolyOnLineToMap(&hitPos, &triangle, start, -getAirGravityVec() * 80.0f)) {
        *getTmpPolygon() = triangle;
        return (hitPos - mPosition).length();
    }

    return 80.0f;
}


void Mario::fixTransBetweenWall(const TVec3f& rFirst, const TVec3f& rSecond) {
    TVec3f center = (rFirst + rSecond) * 0.5f;
    MR::vecKillElement(center - mPosition, *getGravityVec(), &center);
    setTrans(mPosition + center, nullptr);
}


f32 Mario::calcDistWidth() {
    Triangle triangle;
    TVec3f hitPos;

    if (mMovementStates._8) {
        if (MR::getFirstPolyOnLineToMap(&hitPos, &triangle, _4E8, *mFrontWallTriangle->getNormal(0) * 100.0f)) {
            fixTransBetweenWall(hitPos, _4E8);
            return (hitPos - _4E8).length();
        }
    }

    if (mMovementStates._19) {
        if (MR::getFirstPolyOnLineToMap(&hitPos, &triangle, _4F4, *mBackWallTriangle->getNormal(0) * 100.0f)) {
            fixTransBetweenWall(hitPos, _4F4);
            return (hitPos - _4F4).length();
        }
    }

    if (mMovementStates._1A) {
        if (MR::getFirstPolyOnLineToMap(&hitPos, &triangle, _500, *mSideWallTriangle->getNormal(0) * 100.0f)) {
            fixTransBetweenWall(hitPos, _500);
            return (hitPos - _500).length();
        }
    }

    TVec3f center = mPosition + mHeadVec * 80.0f;
    TVec3f firstPos;
    TVec3f secondPos;
    u32 count = Collision::checkStrikeBallToMap(center, 30.0f, nullptr, nullptr);

    if (count < 2) {
        return 100.0f;
    }

    f32 width = 100.0f;

    for (u32 i = 0; i < count; i++) {
        const HitInfo* first = Collision::getStrikeInfoMap(i);
        TVec3f firstNormal(*first->mParentTriangle.getNormal(0));

        for (u32 j = i + 1; j < count; j++) {
            const HitInfo* second = Collision::getStrikeInfoMap(j);
            TVec3f secondNormal(*second->mParentTriangle.getNormal(0));

            if (firstNormal.dot(secondNormal) >= -0.707f) {
                continue;
            }

            if ((first->_64 - center).dot(second->_64 - center) > 0.0f) {
                continue;
            }

            f32 distance = (first->_64 - second->_64).length();

            if (distance < width) {
                width = distance;
                firstPos = first->_64;
                secondPos = second->_64;
            }
        }
    }

    if (width < 100.0f) {
        fixTransBetweenWall(firstPos, secondPos);
    }

    return width;
}


void Mario::updateCameraPolygon() {
    HitSensor* sensor;
    const Triangle* triangle;

    if (mMovementStates._1) {
        triangle = mGroundPolygon;
    } else {
        triangle = _45C;
    }

    sensor = triangle->mSensor;

    if (triangle->isValid()) {
        if (sensor == nullptr) {
            setCameraPolygon(triangle);
            return;
        }

        setCameraPolygon(triangle);
        return;
    }

    TVec3f start = mPosition - *getGravityVec() * 100.0f;
    const Triangle* cameraTriangle = MR::getCameraPolyFast(start, *getGravityVec() * 5000.0f, nullptr);

    if (cameraTriangle != nullptr) {
        setCameraPolygon(cameraTriangle);
    }
}


void Mario::setCameraPolygon(const Triangle* pTriangle) {
    *_468 = *pTriangle;

    if (pTriangle->mSensor == nullptr) {
        *_46C = *pTriangle;
        return;
    }

    *_46C = *pTriangle;
}


void Mario::checkAllWall(const TVec3f& rPosition, f32 radius) {
    TVec3f center(rPosition);
    TVec3f front(mFrontVec);
    _4D8->mIdx = -1;
    _4DC->mIdx = -1;

    if (mMovementStates._F) {
        TVec3f direction;
        mActor->getLastMove(&direction);
        MR::vecKillElement(direction, *getGravityVec(), &direction);

        if (!MR::isNearZero(direction)) {
            MR::normalize(&direction);
            front = direction;
        }
    }

    const HitInfo* walls[3];

    for (s32 i = 0; i < 3; i++) {
        walls[i] = nullptr;
    }

    s32 count;
    bool forwardCheck = false;

    if (getPlayerMode() == PlayerMode_Teresa) {
        radius = 110.0f;
        count = Collision::checkStrikeBallToMapWithThickness(center, radius, radius, nullptr, nullptr);

        if (!count) {
            forwardCheck = true;
            center += mFrontVec * 120.0f;
            count = Collision::checkStrikeBallToMapWithThickness(center, 25.0f, 25.0f, nullptr, nullptr);
        }

    } else {
        count = Collision::checkStrikeBallToMap(center, radius, nullptr, nullptr);
    }

    bool through = false;

    for (u32 i = 0; i < count; i++) {
        const HitInfo* hit = Collision::getStrikeInfoMap(i);
        const Triangle* triangle = &hit->mParentTriangle;
        TVec3f normal(*triangle->getNormal(0));
        TVec3f direction(hit->_64);
        direction -= center;
        MR::normalizeOrZero(&direction);

        if (mMovementStates._37 && MR::abs(normal.dot(_6A0)) > 0.707f) {
            continue;
        }

        f32 angle = 180.0f * (marioAcos(-getGravityVec()->dot(normal)) / 3.14159f);
        TVec3f fromBase(hit->_64);
        fromBase -= mPosition;
        MR::normalizeOrZero(&fromBase);

        if (angle >= mActor->getConst().getTable()->mFlatAngle && direction.dot(mFrontVec) > 0.0f && fromBase.dot(mFrontVec) > 0.0f) {
            if (calcAngleD(normal) <= mActor->getConst().getTable()->mSlipAngle && MR::diffAngleAbs(_368, normal) > 30.0f) {
                *_4DC = *triangle;
                _518 = hit->_64;
            }
        }

        if (MR::abs(normal.dot(mHeadVec)) > 0.5f) {
            if (direction.dot(*getGravityVec()) > 0.0f && direction.dot(mFrontVec) > 0.707f) {
                *_4D8 = *triangle;
                _50C = hit->_64;
            }

            continue;
        }

        f32 facing = normal.dot(front);
        s32 side;

        if (facing >= mActor->getConst().getTable()->mWallBackAngleRange) {
            side = 2;
        } else if (facing <= -mActor->getConst().getTable()->mWallFrontAngleRange) {
            side = 0;
        } else {
            side = 1;
        }

        if (walls[side] != nullptr && walls[side]->_60 > hit->_60) {
            continue;
        }

        if (isThroughWall(triangle)) {
            through = true;
            mActor->_F44 = false;
        } else {
            walls[side] = hit;
        }
    }

    if (!through && getPlayerMode() == PlayerMode_Teresa && !isStatusActive(MarioStatus_Recovery)) {
        mActor->_F44 = true;
    }

    if (count >= 1U) {
        f32 minAngle = 180.0f;
        f32 maxAngle = 0.0f;
        u32 minIndex = 0;
        u32 maxIndex = 0;

        for (u32 i = 0; i < count; i++) {
            const HitInfo* hit = Collision::getStrikeInfoMap(i);
            const Triangle* triangle = &hit->mParentTriangle;

            if (mMovementStates._37 && MR::abs(triangle->getNormal(0)->dot(_6A0)) > 0.707f) {
                continue;
            }

            f32 angle = calcAngleD(*triangle->getNormal(0));

            if (angle < minAngle) {
                minAngle = angle;
                minIndex = i;
            }

            if (angle > maxAngle) {
                maxAngle = angle;
                maxIndex = i;
            }
        }

        f32 groundAngle;

        if (mMovementStates._1) {
            groundAngle = calcAngleD(_368);

            if (groundAngle < minAngle) {
                minAngle = groundAngle;
            }

            if (groundAngle > maxAngle) {
                maxAngle = groundAngle;
            }
        }

        if (maxAngle - minAngle < 60.0f && maxAngle - minAngle > 5.0f && maxAngle < 80.0f) {
            mDrawStates._F = true;

            if (maxAngle > groundAngle) {
                _380 = *Collision::getStrikeInfoMap(maxIndex)->mParentTriangle.getNormal(0);
            } else if (minAngle < groundAngle) {
                _380 = *Collision::getStrikeInfoMap(minIndex)->mParentTriangle.getNormal(0);
            }
        }

        if (mMovementStates._1 && maxAngle > 60.0f && maxAngle != groundAngle && maxAngle < 80.0f) {
            TVec3f hitPos(Collision::getStrikeInfoMap(maxIndex)->_64);
            TVec3f direction(center);
            direction -= hitPos;

            if (!MR::normalizeOrZero(&direction)) {
                mDrawStates._17 = true;
                TVec3f side = Collision::getStrikeInfoMap(maxIndex)->mParentTriangle.getNormal(0)->cross(getAirGravityVec());
                MR::normalize(&side);
                TVec3f slope = _368.cross(side);
                MR::normalize(&slope);
                _2C4 = slope;
            }
        }
    }

    HitInfo pointHit;

    if (MR::checkStrikePointToMap(center, &pointHit)) {
        TVec3f normal(*pointHit.mParentTriangle.getNormal(0));

        if (MR::abs(normal.dot(mHeadVec)) <= 0.5f) {
            f32 facing = normal.dot(front);
            s32 side;

            if (facing >= 0.707f) {
                side = 2;
            } else if (facing <= -0.707f) {
                side = 0;
            } else {
                side = 1;
            }

            walls[side] = &pointHit;
        }
    }

    mMovementStates._8 = false;
    mMovementStates._19 = false;
    mMovementStates._1A = false;
    mMovementStates._32 = false;
    mMovementStates._33 = false;

    if (walls[0]) {
        TVec3f direction(walls[0]->_64);
        direction -= center;
        MR::normalizeOrZero(&direction);
        f32 dot = direction.dot(*walls[0]->mParentTriangle.getNormal(0));
        *mFrontWallTriangle = walls[0]->mParentTriangle;
        _4E8 = walls[0]->_64;
        f32 threshold = -0.999f;

        if (isStatusActive(MarioStatus_Wall)) {
            threshold = -0.9f;
        }

        if (dot < threshold) {
            mMovementStates._8 = true;
        } else {
            mMovementStates._32 = true;
        }

        if (mMovementStates._1 && getMovementStates()._15 && mMovementStates._39) {
            if ((_4A4 - _4E8).dot(*getGravityVec()) >= 0.0f) {
                mMovementStates._8 = false;
            }
        }

        if (!_60D && !mMovementStates._8) {
            if (calcAngleD(*mFrontWallTriangle->getNormal(0)) < mActor->getConst().getTable()->mSlipAngle) {
                bool noSlip = !isSlipFloorCode(_95C->getCode(mFrontWallTriangle));

                if (noSlip && walls[0]->mParentTriangle.getNormal(0)->dot(mFrontVec) < 0.0f && direction.dot(mFrontVec) > 0.0f &&
                    direction.dot(*walls[0]->mParentTriangle.getNormal(0)) < 0.0f) {
                    mDrawStates._F = true;
                    _380 = *mFrontWallTriangle->getNormal(0);
                }
            }
        }
    }

    if (!forwardCheck) {
        if (walls[1]) {
            bool valid = true;

            if (isPlayerModeTeresa()) {
                if (walls[1]->mParentTriangle.getNormal(0)->dot(walls[1]->_64 - center) >= 0.0f) {
                    valid = false;
                }
            }

            if (valid) {
                *mSideWallTriangle = walls[1]->mParentTriangle;
                _500 = walls[1]->_64;
                mMovementStates._1A = true;

                if (walls[1]->isCollisionAtCorner() || walls[1]->isCollisionAtEdge()) {
                    mMovementStates._33 = true;
                }
            }
        }

        if (walls[2]) {
            bool valid = true;

            if (mMovementStates._8) {
                TVec3f backDistance = _4F4 - center;
                TVec3f frontDistance = _4E8 - center;

                if (backDistance.dot(frontDistance) >= 0.0f && backDistance.length() >= frontDistance.length()) {
                    valid = false;
                }
            }

            if (isPlayerModeTeresa()) {
                if (walls[2]->mParentTriangle.getNormal(0)->dot(walls[2]->_64 - center) >= 0.0f) {
                    valid = false;
                }
            }

            if (valid) {
                *mBackWallTriangle = walls[2]->mParentTriangle;
                _4F4 = walls[2]->_64;
                mMovementStates._19 = true;
            }
        }
    }

    updateWallFloorCode();
    _38C = -_380;

    if (_400) {
        _400--;
        mMovementStates._8 = true;
    }
}
#pragma push
#pragma opt_propagation off
#pragma opt_lifetimes off
void Mario::calcFrontFloor() {
    Triangle wall;
    Triangle floor;
    TVec3f start;
    TVec3f wallPos;
    TVec3f floorPos;
    TVec3f direction;
    Triangle* pFloor = _47C;
    mMovementStates._15 = false;
    mMovementStates._39 = false;
    pFloor->mIdx = -1;
    _4E0 = 0.0f;
    bool hit;

    if (mMovementStates._8) {
        wallPos = _4E8;
        start = mActor->_2A0;
        hit = true;
    } else {
        direction = mFrontVec * 100.0f;
        start = mPosition - *getGravityVec() * 8.0f;

        if (mMovementStates._F && !MR::isNearZero(_328)) {
            MR::vecKillElement(_328, *getGravityVec(), &direction);
            direction.setLength(200.0f);
        }

        hit = MR::getFirstPolyOnLineToMap(&wallPos, &wall, start, direction);
    }

    if (hit) {
        _4E4 = (wallPos - start).dot(mFrontVec);
        TVec3f normal(*wall.getNormal(0));
        f32 dot = normal.dot(getAirGravityVec());

        if (MR::abs(dot) < 0.1f) {
            f32 angle = marioAcos(-dot);
            start = wallPos - getAirGravityVec() * 200.0f;
            start += mFrontVec * 20.0f;
            direction = getAirGravityVec() * 210.0f;
            hit = MR::getFirstPolyOnLineBFast(start, direction, &floorPos, &floor);

            if (hit && floor.getNormal(0)->dot(getAirGravityVec()) > -0.9f) {
                hit = false;
            }

            if (hit) {
                _4E0 = (wallPos - floorPos).dot(getAirGravityVec());

                if (_4E0 > 0.0f) {
                    mMovementStates._15 = true;
                }

                f32 offset = 0.0f;

                if (angle < 1.5707964f && angle != 0.0f) {
                    offset = _4E0 / (JMASinRadian(angle) / JMACosRadian(angle));
                }

                TVec3f originalPos(floorPos);
                floorPos -= mFrontVec * (15.0f + offset);

                if (!MR::isExistMapCollision(floorPos - getAirGravityVec() * 5.0f, *getGravityVec() * 10.0f)) {
                    floorPos = originalPos;
                } else {
                    _4A4 = floorPos;
                    mMovementStates._39 = true;
                    *_47C = floor;
                }

            } else {
                _4E0 = 400.0f;
            }
        }

    } else {
        _4E0 = 0.0f;
    }

    if (mMovementStates._1 && calcAngleD(_368) >= 25.0f) {
        mMovementStates._39 = false;
    }
}
#pragma pop

const TVec3f& Mario::getWallNorm() const {
    if (mMovementStates._8) {
        return *mFrontWallTriangle->getNormal(0);
    }

    if (mMovementStates._19) {
        return *mBackWallTriangle->getNormal(0);
    }

    if (mMovementStates._1A) {
        return *mSideWallTriangle->getNormal(0);
    }

    if (mFrontWallTriangle->isValid()) {
        return *mFrontWallTriangle->getNormal(0);
    }

    return mFrontVec;
}


const TVec3f& Mario::getSideWallNorm() const {
    if (mMovementStates._1A) {
        return *mSideWallTriangle->getNormal(0);
    }

    return TVec3f(gZeroVec);
}


const TVec3f& Mario::getFrontWallNorm() const {
    if (mMovementStates._8) {
        return *mFrontWallTriangle->getNormal(0);
    }

    return TVec3f(gZeroVec);
}


const TVec3f& Mario::getBackWallNorm() const {
    if (mMovementStates._19) {
        return *mBackWallTriangle->getNormal(0);
    }

    return TVec3f(gZeroVec);
}


const TVec3f& Mario::getWallPos() const {
    if (mMovementStates._8) {
        return _4E8;
    }

    if (mMovementStates._19) {
        return _4F4;
    }

    if (mMovementStates._1A) {
        return _500;
    }

    return mActor->_2A0;
}


const Triangle* Mario::getWallPolygon() const {
    if (mMovementStates._8) {
        return mFrontWallTriangle;
    }

    if (mMovementStates._19) {
        return mBackWallTriangle;
    }

    if (mMovementStates._1A) {
        return mSideWallTriangle;
    }

    if (mFrontWallTriangle->isValid()) {
        return mFrontWallTriangle;
    }

    return nullptr;
}


const Triangle* Mario::getGroundPolygon() const {
    return mGroundPolygon;
}


void Mario::updateFloorCode() {
    s32 code = _95C->getCode(mGroundPolygon);

    if (code != -1) {
        _960 = code;
    }

    code = _95C->getCode(_45C);

    if (code != -1) {
        _962 = code;
    }

    if (mActor->mBeeWallWalk) {
        _41C = 15;
        return;
    }

    f32 angle = calcPolygonAngleD(mGroundPolygon);

    if (angle > mActor->getConst().getTable()->mSlipAngle) {
        if (angle > mActor->getConst().getTable()->mForceWallAngle) {
            _41C = 0;
        }

        if (_41C) {
            _41C--;
        }

        if (!_41C) {
            _960 = 0x80;
        }

    } else {
        bool noSlip = !isSlipFloorCode(_960);

        if (noSlip) {
            _41C = 15;
        }
    }
}


void Mario::updateWallFloorCode() {
    if (mMovementStates._8 || mMovementStates._32) {
        s32 code = _95C->getCode(mFrontWallTriangle);

        if (code != -1) {
            _964[0] = code;
        }
    }

    if (mMovementStates._19) {
        s32 code = _95C->getCode(mBackWallTriangle);

        if (code != -1) {
            _964[1] = code;
        }
    }

    if (mMovementStates._1A) {
        s32 code = _95C->getCode(mSideWallTriangle);

        if (code != -1) {
            _964[2] = code;
        }
    }
}


void Mario::saveLastSafetyTrans() {
    if (isStatusActive(MarioStatus_FireDamage)) {
        return;
    }

    if (isStatusActive(MarioStatus_Recovery)) {
        return;
    }

    if (!_1C._11) {
        return;
    }

    if (_96A) {
        _96A--;
        return;
    }

    switch (_960) {
    case CollisionFloorCode_Normal:
    case CollisionFloorCode_NoSlip:
    case CollisionFloorCode_Sand:
    case CollisionFloorCode_Glass:
    case CollisionFloorCode_WaterBottomH:
    case CollisionFloorCode_WaterBottomM:
    case CollisionFloorCode_WaterBottomL:
    case CollisionFloorCode_Wet:
    case CollisionFloorCode_NoStampSand:
        break;
    default:
        return;
    }

    if (mGroundPolygon->mSensor->isType(0x48)) {
        return;
    }

    if (mMovementStates._8) {
        return;
    }

    if (mMovementStates._32) {
        return;
    }

    if (mMovementStates._19) {
        return;
    }

    if (mMovementStates._1A) {
        return;
    }

    if (mDrawStates._C) {
        return;
    }

    if (_1C._F) {
        return;
    }

    if (!(mVerticalSpeed < 10.0f)) {
        return;
    }

    if (!(calcAngleD(*mGroundPolygon->getNormal(0)) < 15.0f)) {
        return;
    }

    if (!MR::isSameMtx(*mGroundPolygon->getBaseMtx(), *mGroundPolygon->getPrevBaseMtx())) {
        return;
    }

    if (_7E0->isValid() && MR::isSameMtx(*_7E0->getBaseMtx(), _7E4) && MR::isSameMtx(*_7E0->getBaseMtx(), *_7E0->getPrevBaseMtx())) {
        _814 = _7D4;
        *_820 = *_7E0;
        PSMTXCopy(_7E4, _824);
    }

    *_7E0 = *mGroundPolygon;
    PSMTXCopy(*mGroundPolygon->getBaseMtx(), _7E4);
    _7D4 = (mPosition * 5.0f + *mGroundPolygon->getPos(0) + *mGroundPolygon->getPos(1) + *mGroundPolygon->getPos(2)) * 0.125f;

    if ((_7D4 - mPosition).length() > 50.0f) {
        TVec3f direction = _7D4 - mPosition;
        direction.setLength(50.0f);
        _7D4 = mPosition + direction;
    }

    _1C._E = true;
}


void Mario::setNotSafetyTimer() {
    _96A = 2;

    if (_1C._E) {
        _7E0->mIdx = -1;
    }
}


const TVec3f* Mario::getLastSafetyTrans(TVec3f* pNormal) const {
    if (pNormal != nullptr) {
        pNormal->set(-getAirGravityVec());
    }

    if (_7E0->isValid() && MR::isSameMtx(*_7E0->getBaseMtx(), const_cast< TMtx34f& >(_7E4))) {
        if (pNormal != nullptr) {
            pNormal->set(*_7E0->calcAndGetNormal(0));
        }

        return &_7D4;
    }

    if (_820->isValid() && pNormal) {
        pNormal->set(*_820->calcAndGetNormal(0));
    }

    return &_814;
}


bool Mario::checkCurrentFloorCodeSevere(u32 code) const {
    if (_960 != code) {
        return false;
    }

    TVec3f horizontal;

    if (MR::vecKillElement(mShadowPos - mGroundPos, getAirGravityVec(), &horizontal) > 30.0f) {
        return code == _95C->getCode(mGroundPolygon);
    }

    u32 shadowCode = _95C->getCode(_45C);

    if (shadowCode != code) {
        return false;
    }

    return shadowCode == _95C->getCode(mGroundPolygon);
}


bool Mario::isCurrentFloorSink() const {
    return checkCurrentFloorCodeSevere(CollisionFloorCode_Sink) || checkCurrentFloorCodeSevere(CollisionFloorCode_SinkDeathMud) ||
           checkCurrentFloorCodeSevere(CollisionFloorCode_SinkPoison) || checkCurrentFloorCodeSevere(CollisionFloorCode_SinkDeath);
}


bool Mario::isCurrentFloorSand() const {
    if (getPlayer()->mDrawStates.mIsUnderwater) {
        return false;
    }

    if (getPlayer()->mDrawStates._13) {
        return false;
    }

    return checkCurrentFloorCodeSevere(CollisionFloorCode_Sand) || checkCurrentFloorCodeSevere(CollisionFloorCode_NoStampSand);
}


bool Mario::isCurrentShadowFloorDangerAction() const {
    s32 code = _95C->getCode(_45C);

    if (code == _95C->getCode(mGroundPolygon)) {
        return false;
    }

    if (isPlayerModeTeresa()) {
        return false;
    }

    switch (code) {
    case CollisionFloorCode_DamageFire:
        if (isPlayerModeIce()) {
            return false;
        }

    case CollisionFloorCode_DamageNormal:
    case CollisionFloorCode_JumpLow:
    case CollisionFloorCode_JumpMiddle:
    case CollisionFloorCode_JumpHigh:
    case CollisionFloorCode_JumpNormal:
    case CollisionFloorCode_DamageElectric:
    case CollisionFloorCode_Sink:
    case CollisionFloorCode_SinkPoison:
    case CollisionFloorCode_Needle:
    case CollisionFloorCode_SinkDeath:
    case CollisionFloorCode_RailMove:
    case CollisionFloorCode_AreaMove:
    case CollisionFloorCode_SinkDeathMud:
    case CollisionFloorCode_JumpParasol:
        return true;
    default:
        return false;
    }
}


void Mario::checkBaseTransPoint() {
    bool inside = MR::checkStrikePointToMap(mPosition - *getGravityVec() * 30.0f, nullptr);
    u32 count = Collision::checkStrikeBallToMap(mPosition, 1.0f, nullptr, nullptr);

    for (u32 i = 0; i < count; i++) {
        const HitInfo* hit = Collision::getStrikeInfoMap(i);
        const Triangle* triangle = &hit->mParentTriangle;

        if (MR::isThroughPolygon(triangle) || isThroughWall(triangle)) {
            continue;
        }

        if (hit->isCollisionAtEdge() || hit->isCollisionAtCorner()) {
            mDrawStates._9 = true;
            continue;
        }

        const TVec3f* normal = triangle->getNormal(0);

        if (MR::isNearZero(normal->dot(_368), 0.3f) && !inside) {
            continue;
        }

        TVec3f direction(_16C);
        MR::normalizeOrZero(&direction);

        if (_16C.dot(-*normal) < 0.707f) {
            continue;
        }

        if (MR::abs(_16C.dot(*normal)) < hit->_60 || MR::isNearZero(hit->_60, 1.01f)) {
            continue;
        }

        if ((hit->_64 - mPosition).dot(mHeadVec) < 0.0f) {
            continue;
        }

        addTrans(*normal * (hit->_60 - 1.0f), "めりこみ");

        if (!mMovementStates._1 && mMovementStates.jumping && !isRising() && normal->dot(*getGravityVec()) < -0.99f) {
            addVelocity(mFrontVec, 5.0f);
        }
    }
}


void Mario::checkHeadPoint() {
    f32 radius = 40.0f;
    f32 height = 110.0f;
    bool damaging = false;

    if (mMovementStates._A) {
        height = 50.0f;
    }

    TVec3f center = mPosition - *getGravityVec() * height;

    if (isStatusActive(MarioStatus_Damage)) {
        mActor->calcHeadPos();
        center = mActor->_2AC;

        if (!mDamage->_18) {
            radius = 50.0f;
        }

        damaging = true;
    }

    if (mMovementStates._3E) {
        mActor->calcHeadPos();
        center = mActor->_2AC + _16C * 2.0f;
    }

    if (getPlayerMode() == PlayerMode_Bee && !getPlayer()->mMovementStates._23 && !getPlayer()->mMovementStates._A &&
        !isStatusActive(MarioStatus_Wait) && !isStatusActive(MarioStatus_SideStep) && !isStatusActive(MarioStatus_Bury) && !mActor->mBeeWallWalk) {
        if (isStatusActive(MarioStatus_Stick)) {
            return;
        }

        center = mActor->_2AC;
    }

    if (isSwimming()) {
        center = mPosition + mHeadVec * 60.0f;
    }

    u32 count = Collision::checkStrikeBallToMap(center, radius, nullptr, nullptr);
    Triangle triangles[32];
    u32 triangleCount = 0;
    TVec3f reaction;
    reaction.zero();

    for (u32 i = 0; i < count; i++) {
        const HitInfo* hit = Collision::getStrikeInfoMap(i);
        const Triangle* triangle = &hit->mParentTriangle;

        if (MR::isThroughPolygon(triangle) || isThroughWall(triangle)) {
            continue;
        }

        const TVec3f* normal = triangle->getNormal(0);

        if (!isSwimming() && !mMovementStates._3E) {
            if (damaging) {
                if (normal->dot(*getGravityVec()) < -0.707f) {
                    continue;
                }

            } else if (mMovementStates.jumping) {
                if (normal->dot(*getGravityVec()) < 0.0f || normal->dot(*getGravityVec()) < -0.707f) {
                    continue;
                }
            }
        }

        TVec3f horizontal;

        if (MR::vecKillElement(reaction, *normal, &horizontal) < hit->_60) {
            reaction = horizontal + *normal * hit->_60;

            if (isHeadPushEnableArea()) {
                addVelocity(*normal, hit->_60);
            }

            if (isSwimming()) {
                mSwim->hitHead(hit);
                addVelocity(*triangle->getNormal(0), radius - (hit->_64 - center).length());
            }

            const char* wallCode = MR::getWallCodeString(triangle);

            if (wallCode != nullptr && !strcmp(wallCode, "Fur")) {
                mDrawStates._15 = true;
            }

            if (damaging) {
                mDamage->stopHead(*triangle->getNormal(0));
            }

            triangles[triangleCount] = *triangle;
            triangleCount++;
        }
    }

    addVelocity(reaction);

    if (!MR::isNearZero(reaction) && mMovementStates._3E) {
        TVec3f normal(reaction);
        MR::normalizeOrZero(&normal);

        if (calcAngleD(normal) > 45.0f) {
            blown(reaction * 0.2f);
            mMovementStates._2B = true;
            _402 = 0;
            _428 = 60;
            mMovementStates._3E = 0;
        } else if (_1FC.dot(reaction) < 0.0f) {
            mMovementStates._1 = true;
            mJumpVec.zero();
            changeAnimation("空中一回転");
        }
    }

    bool swimSpin = false;

    if (isSwimming()) {
        if (mSwim->check7Aand7C()) {
            swimSpin = true;
        }
    }

    if ((mMovementStates.jumping && isRising()) || swimSpin) {
        for (u32 i = 0; i < triangleCount; i++) {
            if (triangles[i].getNormal(0)->dot(*getGravityVec()) > 0.707f) {
                mActor->sendMsgUpperPunch(triangles[i].mSensor);
            }
        }
    }
}


void Mario::calcShadowPos() {
    f32 offset = 100.0f;
    TVec3f start = mPosition - *getGravityVec() * offset;

    if ((getCurrentStatus() == MarioStatus_Hang && mHang->_12 < 2) || mActor->_EA4) {
        mActor->getRealPos("Spine1", &start);
    }

    f32 previousDistance = (mShadowPos - mPosition).length();
    TVec3f point(start);
    TVec3f direction(*getGravityVec());
    direction.scale(400.0f);
    u32 count = 6;

    if (isStatusActive(MarioStatus_Recovery)) {
        count = 30;
    }

    for (u32 i = 0; i < count; i++) {
        mMovementStates._2 = MR::getFirstPolyOnLineToMap(&mShadowPos, _45C, point, direction, nullptr, _458);

        if (mMovementStates._2) {
            break;
        }

        point += direction;
    }

    f32 distance = 10.0f + (mShadowPos - mPosition).length();
    bool changed = false;

    if (mMovementStates._2 && MR::abs(previousDistance - distance) > 100.0f) {
        changed = true;
    }

    if (!mMovementStates._2 || changed) {
        direction.setLength(100.0f + (50.0f + previousDistance));
        point = start + (mSideVec + mFrontVec) * 0.1f;
        mMovementStates._2 = MR::getFirstPolyOnLineToMap(&mShadowPos, _45C, point, direction, nullptr, _458);

        if (changed) {
            mMovementStates._2 = true;
        }
    }

    if (!mMovementStates._2) {
        mShadowPos = mPosition + *getGravityVec() * 2500.0f;
    }
}


bool Mario::updateBinderInfo() {
    s32 count;
    bool firstCeiling = true;
    bool firstGround = true;
    const TVec3f* pLastMove;
    _3A4.zero();
    _4C8->mIdx = -1;
    Binder* binder = mActor->mBinder;

    if (binder == nullptr) {
        return false;
    }

    count = binder->mPlaneNum;

    if (!count) {
        return false;
    }

    for (u32 i = 0; i < count; i++) {
        const HitInfo* hit = binder->getPlane(i);
        const Triangle* triangle = &hit->mParentTriangle;
        TVec3f normal(*triangle->getNormal(0));

        if (MR::isThroughPolygon(triangle) || isThroughWall(triangle)) {
            continue;
        }

        f32 dot = normal.dot(*getGravityVec());
        f32 ceilingThreshold = 0.1f;

        if (_430 == 10) {
            ceilingThreshold = 0.707f;
        }

        if (dot < _3C) {
            mDrawStates._6 = true;

            if (firstGround && mMovementStates.jumping && mVerticalSpeed > 30.0f) {
                bool oblique = false;
                TVec3f fromHit = mActor->_2A0 - hit->_64;

                if (MR::diffAngleAbs(fromHit, *triangle->getNormal(0)) > 0.10471976f) {
                    oblique = true;
                }

                if (hit->isCollisionAtCorner() || hit->isCollisionAtEdge() || oblique) {
                    TVec3f pushDirection;

                    if (hit->isCollisionAtEdge()) {
                        pushDirection = *triangle->getNormal(hit->_88 - 1);
                    } else if (!hit->isCollisionAtCorner()) {
                        pushDirection = mPosition - hit->_64;
                    } else {
                        pushDirection = mPosition - hit->_64;
                    }

                    TVec3f direction(fromHit);
                    MR::normalizeOrZero(&direction);
                    f32 facing = direction.dot(*triangle->getNormal(0));
                    f32 scale = facing;

                    if (facing >= 0.707f) {
                        if (hit->isCollisionAtEdge()) {
                            MR::vecKillElement(mVelocity, *triangle->getNormal(0), &mVelocity);
                            scale = 1.0f;
                        }

                    } else if (facing <= -0.707f) {
                        scale = 0.0f;
                    } else {
                        scale = (0.707f - MR::abs(facing)) / 0.707f;
                    }

                    MR::vecKillElement(pushDirection, getAirGravityVec(), &pushDirection);

                    if (MR::normalizeOrZero(&pushDirection)) {
                        MR::vecKillElement(fromHit, getAirGravityVec(), &pushDirection);
                        MR::normalizeOrZero(&pushDirection);
                    }

                    if (mMovementStates._14) {
                        pLastMove = &mActor->_288;

                        if (MR::diffAngleAbsHorizontal(*pLastMove, pushDirection, getAirGravityVec()) >= 2.3561945f) {
                            if (_45C->isValid()) {
                                if (triangle->getNormal(0)->dot(*_45C->getNormal(0)) <= 0.17f) {
                                    f32 component = MR::vecKillElement(mJumpVec, *triangle->getNormal(0), &mJumpVec);
                                    mJumpVec += *triangle->getNormal(0) * component * 0.5f;
                                    mJumpVec += *_45C->getNormal(0);
                                } else {
                                    if (mMovementStates._1A && (MR::diffAngleAbs(pushDirection, getAirGravityVec()) >= 1.4959966f ||
                                                                MR::diffAngleAbsHorizontal(mJumpVec, pushDirection, getAirGravityVec()) < 0.0f)) {
                                        pushDirection = -pushDirection;
                                    }

                                    mMovementStates._14 = false;
                                }

                            } else {
                                if (mMovementStates._1A && (MR::diffAngleAbs(pushDirection, getAirGravityVec()) >= 1.4959966f ||
                                                            MR::diffAngleAbsHorizontal(mJumpVec, pushDirection, getAirGravityVec()) < 0.0f)) {
                                    pushDirection = -pushDirection;
                                }

                                mMovementStates._14 = false;
                            }
                        }

                        if (mMovementStates.jumping && !isRising()) {
                            TVec3f horizontal;
                            pLastMove = &mActor->_27C;

                            if (MR::isNearZero(MR::vecKillElement(*pLastMove, getAirGravityVec(), &horizontal)) &&
                                (mMovementStates._8 || mMovementStates._19 || mMovementStates._1A)) {
                                cutGravityElementFromJumpVec(true);
                                mJumpVec += -getAirGravityVec() * 5.0f + getWallNorm() * 2.0f;

                                if (!isCeiling()) {
                                    addTrans(getAirGravityVec() * -30.0f, nullptr);
                                }

                                pushDirection.zero();
                            }
                        }
                    }

                    push(pushDirection * hit->_60 * scale);
                    firstGround = false;

                    if (getPlayerMode() == PlayerMode_Teresa) {
                        _25C = hit->_64;
                        _268 = *triangle->getNormal(0);
                        doTeresaReflection(fromHit, true);
                    }

                    _1C._D = true;
                }
            }

            if (mMovementStates.jumping && mMovementStates._B) {
                TVec3f lastMove;
                mActor->getLastMove(&lastMove);

                if (MR::isNearZero(lastMove.dot(*getGravityVec()))) {
                    mActor->sendMsgToSensor(triangle->mSensor, 0xB4);

                    if (!mActor->sendMsgToSensor(triangle->mSensor, 3)) {
                        TVec3f direction = mPosition - hit->_64;
                        TVec3f originalDirection(direction);
                        TVec3f velocity(_1A8);
                        MR::normalizeOrZero(&velocity);
                        f32 blend = 1.0f;

                        if (!MR::isNearZero(velocity)) {
                            blend = velocity.dot(*triangle->getNormal(0));
                        }

                        if (blend < 0.0f) {
                            blend = 0.0f;
                        } else if (blend > 1.0f) {
                            blend = 1.0f;
                        }

                        MR::vecBlendSphere(_1A8, direction, &direction, blend);
                        MR::vecKillElement(direction, *getGravityVec(), &direction);
                        MR::normalizeOrZero(&direction);

                        if (MR::isNearZero(direction)) {
                            direction = mFrontVec;
                        }

                        f32 distance = hit->_60;

                        if (distance == 0.0f) {
                            distance = 1.0f;
                        }

                        push(direction * distance);
                        firstGround = false;
                    }
                }
            }

        } else if (!(dot < ceilingThreshold)) {
            TVec3f horizontal;

            if (firstCeiling && (MR::vecKillElement(mJumpVec, normal, &horizontal) < 0.0f || mPrevDrawStates._1E) && calcAngleD(normal) > 100.0f) {
                if (strcmp(triangle->mSensor->mHost->mName, "マンホールのふた(クッパ船)")) {
                    mJumpVec = horizontal;
                }

                startPadVib(0UL);
                firstCeiling = false;
                *_4C8 = *triangle;
            }

            _3A4 += normal;
        }
    }

    MR::normalizeOrZero(&_3A4);
    return true;
}


bool Mario::isThroughWall(const Triangle* pTriangle) const {
    if (getPlayerMode() == PlayerMode_Teresa) {
        if (!_418) {
            return false;
        }

        const char* code = MR::getWallCodeString(pTriangle);

        if (code != nullptr && !strcmp(code, "GhostThroughCode")) {
            return true;
        }
    }

    return false;
}


bool Mario::checkGround() {
    if (isStatusActive(MarioStatus_Recovery) || isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    if (mMovementStates._14) {
        if (mMovementStates._36) {
            bool ground = checkGroundOnSlope();

            if (ground) {
                _1C._13 = true;
                _1C._14 = true;
            }

            return ground;
        }

    } else {
        if (isUseSimpleGroundCheck()) {
            if (!mMovementStates._36) {
                if (MR::isNearZero(getGravityVec()->y - 1.0f)) {
                    mActor->setBlendMtxTimer(4);
                }

                mMovementStates._36 = true;
            }

            bool ground = checkGroundOnSlope();

            if (ground) {
                _1C._13 = true;
                _1C._14 = true;
            }

            return ground;
        }

        if (mMovementStates._36) {
            if (MR::isNearZero(getGravityVec()->y - 1.0f)) {
                mActor->setBlendMtxTimer(4);
            }

            mMovementStates._36 = false;
        }
    }

    TVec3f normal;

    if (isAnimationRun("崖ふんばり")) {
        normal = -*getGravityVec();
    } else {
        normal = *_45C->getNormal(0);
    }

    TVec3f offset;
    MR::vecKillElement(mFrontVec, normal, &offset);

    if (MR::isNearZero(offset)) {
        return false;
    }

    offset.setLength(50.0f);
    TVec3f axis;
    axis = normal;

    if (MR::isNearZero(axis)) {
        return false;
    }

    TMtx34f rotation;
    PSMTXRotAxisRad(rotation, &axis, 2.0943952f);
    u8 hit[4];
    u8 tooFar[4] = {0, 0, 0, 0};
    Triangle triangles[4];
    TVec3f positions[4];
    bool noGround = false;
    mMovementStates._14 = false;
    f32 maxDistance;

    if (mMovementStates.jumping && isRising()) {
        maxDistance = 10.0f;
    } else {
        maxDistance = 30.0f;
    }

    if (getCurrentStatus() == MarioStatus_Slider) {
        maxDistance = 100.0f;
    }

    if (getCurrentStatus() == MarioStatus_Freeze) {
        maxDistance = 5.0f;
    }

    s32 groundCount = 0;
    TVec3f sum;
    sum.zero();
    TVec3f selectedPos;
    bool select = true;
    u32 count = 3;

    if (!mTargetWalkSpeedIndex && mMovementStates._1 && !mMovementStates.jumping && !mMovementStates._23) {
        count = 4;
    }

    if (isAnimationRun("壁押し", 0)) {
        count = 4;
    }

    for (u32 i = 0; i < count; i++) {
        TVec3f start = mPosition + offset - *getGravityVec() * 30.0f;

        if (i == 3) {
            start = mPosition - *getGravityVec() * 30.0f;
        }

        hit[i] = MR::getFirstPolyOnLineBFast(start, *getGravityVec() * 100.0f, &positions[i], &triangles[i]);

        if (i == 3) {
            break;
        }

        if (hit[i]) {
            if (_414 && MR::abs((mPosition - positions[i]).dot(*getGravityVec())) > 0.0f) {
                hit[i] = 0;
                tooFar[i] = 1;
            }

            if (MR::abs((mPosition - positions[i]).dot(*getGravityVec())) > maxDistance) {
                hit[i] = 0;
                tooFar[i] = 1;
            }

            if (calcAngleD(*triangles[i].getNormal(0)) >= 80.0f) {
                hit[i] = 0;
            }

            f32 dot = getGravityVec()->dot(*triangles[i].getNormal(0));

            if (getCurrentStatus() != MarioStatus_Slider && dot > _3C) {
                hit[i] = 0;
            }

            if (isStatusActive(MarioStatus_Recovery) && MR::isThroughPolygon(&triangles[i])) {
                hit[i] = 0;
            }

            if (hit[i]) {
                sum += positions[i];
                groundCount++;

                if (mMovementStates._23 && !select) {
                    TVec3f direction(_16C);
                    MR::normalizeOrZero(&direction);

                    if ((positions[i] - mPosition).dot(direction) > (selectedPos - mPosition).dot(direction)) {
                        select = true;
                    }
                }

                if (select) {
                    setGroundNorm(*triangles[i].getNormal(0));
                    *mGroundPolygon = triangles[i];
                    select = false;
                    selectedPos = positions[i];
                    mGroundPos = selectedPos;
                    recordLastGround();
                }

                const char* wallCode = MR::getWallCodeString(&triangles[i]);

                if (wallCode != nullptr && !strcmp(wallCode, "Fur")) {
                    mDrawStates._19 = true;
                }
            }
        }

        PSMTXMultVec(rotation, &offset, &offset);
    }

    u32 i;

    for (i = 0; i < count; i++) {
        if (!hit[i]) {
            break;
        }
    }

    if (i == count) {
        _1C._11 = true;
    }

    if (!isNoWalkFallOnDossun() && !isStatusActive(MarioStatus_Faint) && !isStatusActive(MarioStatus_Damage) && !isStatusActive(MarioStatus_Talk) &&
        !isStatusActive(MarioStatus_Hang) && (!mMovementStates._18 || _10._10)) {
        while ((count == 4 && !hit[3]) || _960 == CollisionFloorCode_Slide) {
            if (!(hit[0] | hit[1] | hit[2])) {
                return false;
            }

            if (!hit[0]) {
                bool moved = false;

                if (!mMovementStates._8) {
                    addTrans(mFrontVec * 6.0f, "前方WKFALL");
                    moved = true;
                }

                if (hit[1] && hit[2]) {
                    if (!moved) {
                        break;
                    }

                } else if (!hit[1]) {
                    if ((mMovementStates._1A || mMovementStates._8 || mMovementStates._19) && mSideVec.dot(getWallNorm()) < 0.0f) {
                        if (moved) {
                            addTrans(-mSideVec * 3.0f - mFrontVec * 6.0f, "+逆-左WKFALL");
                        }

                        break;
                    } else {
                        addTrans(mSideVec * 3.0f, "+左WKFALL");
                        moved = true;
                    }

                } else {
                    if ((mMovementStates._1A || mMovementStates._8 || mMovementStates._19) && -mSideVec.dot(getWallNorm()) < 0.0f) {
                        if (moved) {
                            addTrans(mSideVec * 3.0f - mFrontVec * 6.0f, "+逆-右WKFALL");
                        }

                        break;
                    } else {
                        addTrans(-mSideVec * 3.0f, "+右WKFALL");
                        moved = true;
                    }
                }

                if (moved) {
                    mDrawStates._A = true;
                }

                return true;
            } else if (!hit[1] && !hit[2]) {
                stopWalk();

                if (_3CE < 24 && mJumpVec.dot(mFrontVec) >= 0.0f && _960 != CollisionFloorCode_Slide) {
                    if (!mMovementStates._8 && !mMovementStates._32) {
                        addTrans(mFrontVec * 3.0f, "+後ろ1WKFALL");
                        mDrawStates._A = true;
                    }

                } else {
                    if (!mMovementStates._19) {
                        mDrawStates._18 = true;
                        addTrans(mFrontVec * -6.0f, "+後ろ2WKFALL");
                        mDrawStates._A = true;
                    }

                    if (_3C6 > 8) {
                        addTrans(mSideVec * 5.0f, "+SIDEFALL");
                    }

                    return true;
                }

            } else if (_960 == CollisionFloorCode_Slide) {
                if (!hit[2]) {
                    addTrans(-mSideVec * 5.0f, "-L-SIDEFALL");
                } else if (!hit[1]) {
                    addTrans(mSideVec * 5.0f, "+R-SIDEFALL");
                } else {
                    addTrans(mFrontVec * 5.0f, "+F-SIDEFALL");
                }

                mDrawStates._A = true;
            }

            break;
        }
    }

    if (!groundCount) {
        noGround = true;
    }

    s32 sameSensor = 0;
    s32 sameSlope = 0;
    TVec3f groundHorizontal;
    MR::vecKillElement(*mGroundPolygon->getNormal(0), getAirGravityVec(), &groundHorizontal);

    for (u32 i = 0; i < groundCount; i++) {
        if (hit[i]) {
            if (triangles[i].mSensor == mGroundPolygon->mSensor) {
                sameSensor++;
            } else {
                sameSensor--;
            }

            TVec3f horizontal;
            MR::vecKillElement(*triangles[i].getNormal(0), getAirGravityVec(), &horizontal);

            if (horizontal.dot(groundHorizontal) >= 0.0f) {
                sameSlope++;
            }
        }
    }

    if (sameSensor > 0) {
        _1C._13 = true;
    }

    if (sameSlope >= 3) {
        _1C._14 = true;
    }

    _8EC = sameSensor;

    if (mMovementStates._D) {
        mMovementStates._D = false;
        TVec3f direction = mGroundPos - mPosition;
        f32 distance = MR::vecKillElement(direction, *getGravityVec(), &direction);
        addTrans(*getGravityVec() * distance, "force Trans");
        return true;
    }

    if (groundCount) {
        f32 distance = 0.0f;
        TVec3f horizontal;

        if (calcAngleD(*_45C->getNormal(0)) < 55.0f) {
            distance = MR::vecKillElement(mShadowPos - mPosition, *getGravityVec(), &horizontal);
        }

        if (MR::abs(distance) > 30.0f) {
            distance = MR::vecKillElement(mGroundPos - mPosition, *getGravityVec(), &horizontal);
        }

        if (MR::abs(distance) < 30.0f && MR::abs(distance) > 1.0f && mMovementStates._1) {
            f32 dot = getGravityVec()->dot(-_368);

            if (dot > 0.99f) {
                if (!mDrawStates._9 && !_4D8->isValid()) {
                    if (!isStatusActive(MarioStatus_Talk)) {
                        addTrans(*getGravityVec() * distance, "force Trs2");
                    }

                    return true;
                }

            } else if (dot > 0.0f && !mDrawStates._9 && mMovementStates._23) {
                addTrans(-_368 * distance * dot, "force Trs3");
                return true;
            }
        }
    }

    if (mMovementStates.jumping && isRising()) {
        return false;
    }

    if (!isStatusActive(MarioStatus_Bury) && count == 4 && noGround) {
        if (hit[3] == 1 && MR::abs((mPosition - positions[3]).dot(*getGravityVec())) < maxDistance) {
            noGround = false;
        }

        bool moveFront = true;
        bool moveBack = true;

        if (mMovementStates._8) {
            moveFront = false;
        }

        if (mMovementStates._19) {
            moveBack = false;
        }

        if (mActor->_288.dot(mFrontVec) < 0.0f) {
            if (moveBack) {
                addTrans(-mFrontVec * 6.0f, "no-g(back)");
            }

        } else if (moveFront) {
            addTrans(mFrontVec * 6.0f, "no-g");
        }
    }

    if (noGround) {
        mMovementStates._14 = true;
        return false;
    }

    if (!mActor->_EA4 && getCurrentStatus() != MarioStatus_Hang) {
        if ((mMovementStates.jumping && groundCount) || (mMovementStates._1 && mVerticalSpeed >= 5.0f)) {
            TVec3f direction;
            f32 distance = MR::vecKillElement(mGroundPos - mShadowPos, _368, &direction);
            f32 maxMove = 5.0f;
            direction = mShadowPos - mPosition;

            if (mMovementStates._8 && direction.dot(getFrontWallNorm()) < -0.01f) {
                maxMove = 0.0f;
            }

            if (mMovementStates._1A && direction.dot(getSideWallNorm()) < -0.01f) {
                maxMove = 0.0f;
            }

            if (mMovementStates._8 && direction.dot(getBackWallNorm()) < -0.01f) {
                maxMove = 0.0f;
            }

            if (maxMove != 0.0f) {
                if (direction.length() > maxMove) {
                    mShadowPos = mShadowPos - mPosition;
                    mShadowPos.setLength(maxMove);
                    mShadowPos += mPosition;
                }

                if (distance < maxDistance) {
                    setTrans(mShadowPos, nullptr);
                }
            }
        }

        if (!groundCount && MR::abs((mShadowPos - mPosition).dot(*getGravityVec())) < maxDistance) {
            TVec3f direction = mShadowPos - mPosition;
            f32 distance = MR::vecKillElement(direction, *getGravityVec(), &direction);

            if (!MR::isNearZero(distance, 1.0f) || !mMovementStates._1) {
                mVelocity += *getGravityVec() * distance;
                mDrawStates._0 = true;
            }

            return true;
        }
    }

    if (!groundCount && MR::abs((mGroundPos - mPosition).dot(*getGravityVec())) < maxDistance) {
        TVec3f direction = mGroundPos - mPosition;
        f32 distance = MR::vecKillElement(direction, *getGravityVec(), &direction);

        if (!MR::isNearZero(distance, 1.0f)) {
            mVelocity += *getGravityVec() * distance;
            mDrawStates._0 = true;
        }

        return true;
    }

    if (mVerticalSpeed < 5.0f) {
        return true;
    }

    if (mDrawStates._6) {
        return true;
    }

    if (groundCount) {
        return true;
    }

    return mDrawStates._0;
}


CubeCameraArea* Mario::getCameraCubeCode() const {
    if ((isSwimming() && mSwim->isOnWaterSurface()) || (mMovementStates.jumping && isRising())) {
        return static_cast< CubeCameraArea* >(MR::getAreaObj("CubeCamera", mPosition - *getGravityVec() * 100.0f));
    }

    return static_cast< CubeCameraArea* >(MR::getAreaObj("CubeCamera", mPosition));
}


void Mario::updateCubeCode() {
    AreaObj* tower;
    AreaObj* raster;
    s32 first;
    CubeCameraArea* camera;

    if ((isSwimming() && mSwim->isOnWaterSurface()) || (mMovementStates.jumping && isRising())) {
        camera = static_cast< CubeCameraArea* >(MR::getAreaObj("CubeCamera", mPosition - *getGravityVec() * 100.0f));
    } else {
        camera = static_cast< CubeCameraArea* >(MR::getAreaObj("CubeCamera", mPosition));
    }

    _568 = camera;
    _570 = 0;

    if (camera != nullptr) {
        _564 = MR::getAreaObjArg(camera, 0);
        switch (MR::getAreaObjArg(camera, 1)) {
        case 1:
            mDrawStates._2 = true;
            break;
        case 2:
            mDrawStates._3 = true;
            break;
        }

    } else {
        _564 = -1;
    }

    MR::tryToUpdatePlayerRestartIdInfo(mPosition);
    AreaObj* pullBack = MR::getAreaObj("PullBackCube", mPosition);

    if (pullBack == nullptr) {
        pullBack = MR::getAreaObj("PullBackCylinder", mPosition);

        if (pullBack != nullptr && MR::getAreaObjArg(pullBack, 0) != 1) {
            TVec3f position;
            TVec3f up;
            MR::calcCylinderPos(&position, pullBack);
            MR::calcCylinderUpVec(&up, pullBack);
            MarioRecovery* recovery = mRecovery;
            recovery->_4C = position;
            recovery->_58 = up;
            recovery->_12 = true;
        }
    }

    if (pullBack != nullptr) {
        doRecovery();
    }

    _10._6 = false;

    if (MR::getAreaObj("PlaneCollisionCube", mPosition) != nullptr) {
        _10._6 = true;
    }

    _10._13 = false;
    tower = MR::getAreaObj("TowerModeCylinder", mPosition);

    if (tower != nullptr) {
        _10._13 = true;
        MR::calcCylinderCenterPos(&_6F4, tower);
        MR::calcCylinderUpVec(&_700, tower);
        _718 = MR::getCylinderRadius(tower);
    }

    if (MR::getAreaObj("ForbidTriangleJumpCube", mPosition) != nullptr) {
        mDrawStates._3 = true;
    }

    if (getPlayerMode() == PlayerMode_Teresa) {
        if (MR::getAreaObj("GlaringLightArea", mPosition) != nullptr) {
            mActor->setPlayerMode(PlayerMode_Normal, true);
        }

        if (!MR::isInShadeFromTheSun(mPosition, 2000.0f)) {
            mActor->setPlayerMode(PlayerMode_Normal, true);
        }
    }

    if (MR::getAreaObj("FallsCube", mActor->_2AC) != nullptr) {
        touchWater();
        playEffectRTZ("水壁ヒット", mHeadVec, mActor->_2AC);
    }

    if (MR::getAreaObj("HeavySteeringCube", mPosition) != nullptr) {
        _10._11 = true;
        _10._12 = true;
    } else {
        _10._11 = false;
        _10._12 = false;
    }

    if (MR::getAreaObj("DashChargeCylinder", mPosition) != nullptr) {
        _434 = mActor->getConst().getTable()->mItemDashTimer;
    }

    raster = MR::getAreaObj("RasterScrollCube", mPosition);

    if (raster != nullptr) {
        first = MR::getAreaObjArg(raster, 0);
        s32 second = MR::getAreaObjArg(raster, 1);
        mActor->setRasterScroll(first, second, MR::getAreaObjArg(raster, 2));
    }

    if (MR::getAreaObj("ForbidJumpCube", mPosition) != nullptr) {
        _1C._7 = true;
    }

    if (!isStatusActive(MarioStatus_DarkDamage)) {
        if (MR::getAreaObj("DarkMatterCube", mPosition) != nullptr) {
            mActor->forceKill(4);
        }

        mVelocity.zero();
    }
}

// ---- MarioDamage.cpp ----

namespace {
    const char sDamageAirAnimation[] = "中ダメージ空中";
    const char sDamageLandAnimation[] = "中ダメージ着地";
    const char sBackDamageAirAnimation[] = "中後ダメージ空中";
    const char sBackDamageLandAnimation[] = "中後ダメージ着地";
};
  // namespace

void MarioDamage_FORCE_MATCH_SDATA2() {
    (void)10.0f;
    (void)5.0f;
    (void)0.949999988f;
    (void)30.0f;
    (void)20.0f;
    (void)50.0f;
    (void)-10.0f;
    (void)0.699999988f;
    (void)0.100000001f;
    (void)0.25f;
}


bool Mario::isDamaging() const {
    if (isAnimationRun("水上ダメージ中")) {
        return true;
    }

    if (_41E) {
        return true;
    }

    if (mMovementStates._1B) {
        return true;
    }

    if (mMovementStates._27) {
        return true;
    }

    if (mMovementStates._2C) {
        return true;
    }

    if (_10.jumping) {
        return true;
    }

    if (_10._14) {
        return true;
    }

    if (_10._18) {
        return true;
    }

    switch (getCurrentStatus()) {
    case MarioStatus_FireDamage:
    case MarioStatus_FireDance:
    case MarioStatus_FireRun:
    case MarioStatus_Paralyze:
    case MarioStatus_AbyssDamage:
    case MarioStatus_Freeze:
    case MarioStatus_Crush:
        return true;
    case MarioStatus_Damage:
        return mDamage->_12;
    case MarioStatus_Faint:
        return mFaint->mTookDamage;
    default:
        return false;
    }
}


bool Mario::damageLarge(const TVec3f& rDirection) {
    if (damage(rDirection)) {
        if (isStatusActive(MarioStatus_Swim)) {
            mSwim->_AD = 1;
        } else {
            mDamage->setVecSize(mActor->getConst().getTable()->mJumpDistLargeDamage, mActor->getConst().getTable()->mJumpHeightLargeDamage);
            playSound("投げられ");
        }

        return true;
    }

    return false;
}


void Mario::decDamageAfterTimer() {
    MarioDamage* damage = mDamage;
    if (damage->_16) {
        damage->_16--;
    }

    MarioFaint* faint = mFaint;
    if (faint->_14) {
        faint->_14--;
    }

    MarioParalyze* paralyze = mParalyze;
    if (paralyze->_16) {
        paralyze->_16--;
    }

    MarioFreeze* freeze = mFreeze;
    if (freeze->_1C) {
        freeze->_1C--;
    }

    mFireDamage->decAfterTimer();
    if (_41E) {
        _41E--;
    }
}


bool Mario::checkDamage() {
    decDamageAfterTimer();
    if (mMovementStates._1F) {
        return false;
    }

    if (mActor->_EA4) {
        return false;
    }

    if (MR::isDemoActive()) {
        return false;
    }

    if (isStatusActive(MarioStatus_Talk)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Recovery) || isInvincible()) {
        mMovementStates._1B = false;
        mMovementStates._27 = false;
        mMovementStates._2C = false;
        _10.jumping = false;
        _10._14 = false;
        _10._18 = false;
        mFaint->mNoDamage = false;
        return false;
    }

    if (isStatusActive(MarioStatus_Swim)) {
        checkWaterDamage();
        return false;
    }

    if (mMovementStates._1B) {
        mMovementStates._1B = false;
        changeStatus(mDamage);
        mMovementStates._27 = false;
        mFaint->mNoDamage = false;
        mMovementStates._2C = false;
        return true;
    }

    if (mMovementStates._27) {
        mMovementStates._27 = false;
        mMovementStates._2C = false;
        changeStatus(mFaint);
        return true;
    }

    if (mMovementStates._2C) {
        mMovementStates._2C = false;
        changeStatus(mBlown);
        return true;
    }

    if (_10.jumping) {
        _10.jumping = false;
        doFireDanceWithInitialDamage(1);
        return true;
    }

    if (_10._14) {
        _10._14 = false;
        doParalyze();
        return true;
    }

    if (_10._18) {
        _10._18 = false;
        if (tryCrush()) {
            return true;
        }
    }

    checkKarikariDamage();
    return false;
}


u16 Mario::getDamageAfterTimer() const {
    u16 timer = mDamage->_16;
    if (timer < mFaint->_14) {
        timer = mFaint->_14;
    }

    if (timer < mParalyze->_16) {
        timer = mParalyze->_16;
    }

    if (timer < mFreeze->_1C) {
        timer = mFreeze->_1C;
    }

    if (timer < mFireDamage->_12) {
        timer = mFireDamage->_12;
    }

    if (timer < _41E) {
        timer = _41E;
    }

    return timer;
}


bool Mario::damageFloorCheck() {
    if (mMovementStates._1F) {
        return false;
    }

    if (_1C._16) {
        return false;
    }

    switch (_960) {
    case CollisionFloorCode_Needle:
        if (checkCurrentFloorCodeSevere(CollisionFloorCode_Needle) && doNeedleWithInitialDamage(mGroundPolygon)) {
            return true;
        }

        break;
    case 0x81:
        if (checkCurrentFloorCodeSevere(0x81) && doFireDanceWithInitialDamage(1)) {
            return true;
        }

        break;
    case CollisionFloorCode_Death:
        MarioAccess::forceKill(3, 0);
        return true;
    case CollisionFloorCode_DamageFire:
        if (checkCurrentFloorCodeSevere(CollisionFloorCode_DamageFire) && doFireDanceWithInitialDamage(1)) {
            return true;
        }

        break;
    case CollisionFloorCode_DamageNormal:
        if (isDamaging()) {
            return false;
        }

        if (damage(_368 * 10.0f)) {
            return true;
        }

        break;
    case CollisionFloorCode_DamageElectric:
        if (doParalyze()) {
            return true;
        }

        break;
    case CollisionFloorCode_PullBack:
        if (doRecovery()) {
            return true;
        }

        break;
    }

    return false;
}


bool Mario::damageWallCheck() {
    if (mMovementStates._1F) {
        return false;
    }

    TVec3f normal;
    if (checkWallCodeNorm(CollisionWallCode_Rebound, &normal, false)) {
        return doFlipJump(normal * 5.0f);
    }

    if (checkWallFloorCode(CollisionFloorCode_Death)) {
        mActor->forceKill(3);
        return true;
    }

    if (checkWallFloorCode(CollisionFloorCode_DamageFire) && doFireDanceWithInitialDamage(1)) {
        return true;
    }

    if (checkWallFloorCode(CollisionFloorCode_Needle) && doNeedleWithInitialDamage(1)) {
        return true;
    }

    if (checkWallFloorCode(CollisionFloorCode_DamageElectric) && doParalyze()) {
        return true;
    }

    if (checkWallFloorCode(CollisionFloorCode_PullBack) && doRecovery()) {
        return true;
    }

    if (checkWallFloorCode(CollisionFloorCode_DamageNormal) && damage(getWallNorm() * 10.0f)) {
        return true;
    }

    return false;
}


bool Mario::damagePolygonCheck(const Triangle* pTriangle) {
    if (mMovementStates._1F) {
        return false;
    }

    switch (_95C->getCode(pTriangle)) {
    case 0x81:
        if (doFireDanceWithInitialDamage(1)) {
            return true;
        }

        break;
    case CollisionFloorCode_Death:
        MarioAccess::forceKill(3, 0);
        return true;
    case CollisionFloorCode_DamageFire:
        if (doFireDanceWithInitialDamage(1)) {
            return true;
        }

        break;
    case CollisionFloorCode_DamageNormal:
        mSwim->addDamage(*MR::getNormal(pTriangle) * 10.0f);
        return true;
    case CollisionFloorCode_DamageElectric:
        if (doParalyze()) {
            return true;
        }

        break;
    case CollisionFloorCode_PullBack:
        if (doRecovery()) {
            return true;
        }

        break;
    case CollisionFloorCode_Needle:
        doNeedleWithInitialDamage(pTriangle);
        return true;
    case CollisionFloorCode_SinkDeath:
        return true;
    }

    return false;
}


bool Mario::flipLarge(const TVec3f& rDirection) {
    if (damage(rDirection)) {
        if (isStatusActive(MarioStatus_Swim)) {
            mSwim->_AD = 1;
            mSwim->mDamageType = 1;
        } else {
            mDamage->_11 = 1;
        }

        mDamage->setVecSize(rDirection.length(), 0.0f);
        return true;
    }

    return false;
}


bool Mario::isEnableAddDamage() const {
    if (getCurrentStatus() == MarioStatus_Talk) {
        return false;
    }

    if (isDamaging()) {
        return false;
    }

    if (mActor->_390) {
        return false;
    }

    if (isInvincible()) {
        return false;
    }

    return getDamageAfterTimer() == 0;
}


bool Mario::damage(const TVec3f& rDirection) {
    _7C4 = rDirection;
    if (!isEnableAddDamage()) {
        return false;
    }

    if (mFaint->_14 || mDamage->_16 || mMovementStates._1B) {
        return false;
    }

    if (mMovementStates._F) {
        forceStopTornado();
    }

    mDamage->setVec(rDirection);
    stopWalk();
    forceStopTornado();
    mActor->damageDropThrowMemoSensor();
    if (isStatusActive(MarioStatus_Damage)) {
        closeStatus(mDamage);
    }

    mMovementStates._1B = true;
    return true;
}


bool Mario::doAbyssDamage() {
    if (getCurrentStatus() == MarioStatus_AbyssDamage) {
        return false;
    }

    stopWalk();
    mActor->damageDropThrowMemoSensor();
    MR::removeAllClingingKarikari();
    mActor->_A6E = 0;
    changeStatus(mAbyssDamage);
    return true;
}


const char* MarioDamage_FORCE_MATCH_DATA(u32 index) {
    switch (index) {
    case 0:
        return "炎ダメージ";
    case 1:
        return "炎ダメージ青煙";
    default:
        return "炎ダメージ煙";
    }
}


void Mario::connectToFireRun() {
    if (mActor->mHealth) {
        changeStatus(mFireRun);
        stopJump();
        mFireRun->_12 = 1;
    }
}


bool Mario::doFireDanceWithInitialDamage(u8 amount) {
    if (mMovementStates._1F) {
        return false;
    }

    bool started = doFireDance();
    if (started) {
        for (u32 i = 0; i < amount; i++) {
            mActor->decLife(0);
        }

        if (!mActor->mHealth) {
            mActor->forceGameOverNonStop();
        }
    }

    return started;
}


bool Mario::doFireObjHitWithInitialDamage() {
    if (isEnableAddDamage() == false) {
        return false;
    }

    return doFireDanceWithInitialDamage(1);
}


bool Mario::doNeedleWithInitialDamage(u8 amount) {
    if (mMovementStates._1F) {
        return false;
    }

    if (getPlayerMode() == PlayerMode_Teresa) {
        doTeresaReflection(getWallNorm(), false);
        return false;
    }

    bool started = doNeedle(nullptr);
    if (started) {
        for (u32 i = 0; i < amount; i++) {
            mActor->decLife(0);
        }

        if (!mActor->mHealth) {
            mActor->forceGameOverNonStop();
        }
    }

    return started;
}


bool Mario::doNeedleWithInitialDamage(const Triangle* pTriangle) {
    if (mMovementStates._1F) {
        return false;
    }

    if (getPlayerMode() == PlayerMode_Teresa) {
        doTeresaReflection(*MR::getNormal(pTriangle), false);
        return false;
    }

    bool started = doNeedle(pTriangle);
    if (started) {
        mActor->decLife(0);
        if (!mActor->mHealth) {
            mActor->forceGameOverNonStop();
        }
    }

    return started;
}


bool Mario::doNeedle(const Triangle* pTriangle) {
    if (getCurrentStatus() == MarioStatus_FireDamage) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_FireRun) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_FireDance) {
        return false;
    }

    if (mMovementStates._1B) {
        return false;
    }

    if (getPlayerMode() == PlayerMode_Teresa) {
        if (pTriangle != nullptr) {
            doTeresaReflection(*MR::getNormal(pTriangle), false);
        }

        return false;
    }

    if (isInvincible()) {
        return false;
    }

    mActor->resetPlayerModeOnDamage();
    getPlayer()->mMovementStates._B = false;
    getPlayer()->mMovementStates._A = false;
    mActor->damageDropThrowMemoSensor();
    mFireDance->_29 = 1;
    changeStatus(mFireDance);
    return true;
}


bool Mario::doFireDance() {
    if (getCurrentStatus() == MarioStatus_Paralyze) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_FireDamage) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_FireRun) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_FireDance) {
        return false;
    }

    if (mMovementStates._1B) {
        return false;
    }

    if (isInvincible()) {
        return false;
    }

    if (getPlayerMode() == PlayerMode_Ice) {
        return false;
    }

    mActor->resetPlayerModeOnDamage();
    getPlayer()->mMovementStates._B = false;
    getPlayer()->mMovementStates._A = false;
    mActor->damageDropThrowMemoSensor();
    mFireDance->_29 = 0;
    changeStatus(mFireDance);
    mFireDamage->_12 = 120;
    return true;
}


void Mario::checkKarikariDamage() {
    if (!_1C._5 || isDamaging()) {
        _7D0 = 120;
        return;
    }

    if (mActor->_934) {
        return;
    }

    if (mActor->_EA4) {
        return;
    }

    if (isStatusActive(MarioStatus_Talk)) {
        return;
    }

    if (_1C._5 && _7D0) {
        _7D0--;
        if (!_7D0) {
            if (mActor->mHealth == 1) {
                faint(mHeadVec);
                _7D0 = 120;
                return;
            }

            startPadVib(2);
            playSound("声小ダメージ");
            playSound("ダメージ");
            mActor->decLifeSmall();
            mActor->_BC4 = 16;
            _7D0 = 120;
            if (!mActor->mHealth) {
                mActor->forceGameOver();
            }
        }
    }
}


bool Mario::doDarkDamage() {
    if (getCurrentStatus() == MarioStatus_DarkDamage) {
        return false;
    }

    mActor->_3C0 = true;
    stopWalk();
    mActor->damageDropThrowMemoSensor();
    playSound("声沼沈み");
    playEffect("ダークマター死亡");
    setSeVersion(1);
    changeStatus(mDarkDamage);
    return true;
}

// ---- MarioDamageCrush.cpp ----

bool Mario::requestCrush() {
    _10._18 = true;
    return true;
}


bool Mario::tryCrush() {
    if (mMovementStates._1F) {
        return false;
    }

    if (isInvincible()) {
        return false;
    }

    if (mMovementStates.jumping || !mMovementStates._1) {
        setTrans(mShadowPos, nullptr);
    }

    if (getCurrentStatus() == MarioStatus_Crush) {
        return false;
    }

    mActor->damageDropThrowMemoSensor();
    mActor->resetPlayerModeOnDamage();

    stopJump();
    stopWalk();

    changeStatus(mCrush);
    return true;
}

// ---- MarioDamageFreeze.cpp ----

bool Mario::doFreeze() {
    if (mMovementStates._1F) {
        return false;
    }

    if (isInvincible()) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_Freeze) {
        return false;
    }

    if (mFreeze->_1C != 0) {
        return false;
    }

    mActor->resetPlayerModeOnDamage();

    stopJump();
    stopWalk();

    mActor->damageDropThrowMemoSensor();
    cancelSquatMode();

    stopAnimationUpper(nullptr);
    changeStatus(mFreeze);
    return true;
}

// ---- MarioDamageParalyze.cpp ----

bool Mario::doParalyze() {
    if (mMovementStates._1F) {
        return false;
    }

    if (isInvincible()) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_Paralyze) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_FireDamage) {
        return false;
    }

    if (mMovementStates._1B) {
        return false;
    }

    if (getDamageAfterTimer() != 0) {
        return false;
    }

    if (isDamaging()) {
        return false;
    }

    mActor->damageDropThrowMemoSensor();
    mActor->resetPlayerModeOnDamage();

    stopJump();
    stopWalk();

    changeStatus(mParalyze);
    return true;
}

// ---- MarioEffect.cpp ----

struct MaterialEffectEntry {
    union FlagWord {
        u32 mWord;
        struct {
            u8 mByte0;
            u8 mByte1;
            u8 mByte2;
            u8 mByte3;
        };
    };

    /* 0x00 */ const char* mName;
    /* 0x04 */ FlagWord mFlag;
    /* 0x08 */ const char* mEffects[7];
    /* 0x24 */ const char* mCurrent;
    /* 0x28 */ MultiEmitter* mEmitter;
};


struct SmokeEffectEntry {
    union FlagWord {
        u32 mWord;
        struct {
            u8 mByte0;
            u8 mByte1;
            u8 mByte2;
            u8 mByte3;
        };
    };

    /* 0x00 */ const char* mName;
    /* 0x04 */ FlagWord mType;
    /* 0x08 */ f32 mScale;
    /* 0x0C */ f32 mRate;
    /* 0x10 */ FlagWord mFlags;
    /* 0x14 */ u16 mInterval;
    /* 0x16 */ u16 mTimer;
    /* 0x18 */ u32 mHash;
    /* 0x1C */ MultiEmitter* mEmitter;
    /* 0x20 */ u8 mMaterial;
    /* 0x21 */ u8 _21[3];
};


MaterialEffectEntry cMaterialEffectTable[] = {
    {"属性尻ドロップ",
     0x00000000,
     {"HipDropSmoke", "HipDropWater", "HipDropFlower", "HipDropSand", "HipDropSnow", "MudCircle", "HoneyCircle"},
     nullptr,
     nullptr},
    {"属性ハチ風", 0x03000000, {"Hovering", "HoveringWater", "Hovering", "Hovering", "Hovering", "Hovering", "Hovering"}, nullptr, nullptr},
    {"属性スピン", 0x00000000, {nullptr, "WaterSpin", "FlowerSpin", "SandSpin", "SnowSpin", "MudCircle", "HoneyCircle"}, nullptr, nullptr},
    {"属性ステージイン",
     0x00000000,
     {"StageStartGroundSmoke", nullptr, nullptr, "StageStartGroundSand", nullptr, nullptr, nullptr},
     nullptr,
     nullptr},
    {nullptr, 0, {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}, nullptr, nullptr}};


namespace {

    void addMarioEffectAtJoint(LiveActor* pActor, const char* pEffectName, const char* pJointName, const char* pGroupName) NO_INLINE {
        MtxPtr mtx = pJointName != nullptr ? MR::getJointMtx(pActor, pJointName) : pActor->getBaseMtx();
        pActor->mEffectKeeper->registerEffect(pEffectName, mtx, &pActor->mScale, pGroupName, pJointName);
    }

};


SmokeEffectEntry cSmokeTable[] = {{"共通着地普通", 0x08000000, 1.0f, 1.0f, 0x00000400, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通着地大", 0x03000000, 1.5f, 1.0f, 0x00000400, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通壁ジャンプ", 0x03000000, 1.0f, 1.0f, 0x00000300, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通砂煙レベル", 0x07000000, 0.65f, 1.0f, 0x00000000, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通壁手擦り", 0x01000000, 0.35f, 1.0f, 0x02010000, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通壁上昇", 0x01000000, 1.0f, 1.0f, 0x00000200, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通スリップ坂", 0x02000000, 0.35f, 1.0f, 0x00010000, 2, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通スリップ空転", 0x00000000, 1.0f, 1.0f, 0x00000000, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通スリップ坂制動", 0x02000000, 0.5f, 1.0f, 0x00000000, 2, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通壁ヒット着地", 0x03000000, 1.0f, 1.0f, 0x00000400, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通ダメージ着地", 0x03000000, 1.0f, 1.0f, 0x00000400, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通地上スピン", 0x03000000, 0.9f, 1.0f, 0x00000400, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通ブレーキ", 0x02000000, 0.65f, 1.0f, 0x00010000, 15, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通ハイジャンプ", 0x02000000, 0.65f, 1.0f, 0x00010000, 6, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通ブラックホール", 0x01000000, 0.65f, 1.0f, 0x03030000, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通引き戻し着地", 0x03000000, 1.2f, 1.0f, 0x00000400, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通水底接触", 0x01000000, 6.0f, 0.5f, 0x00000400, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通跳躍", 0x08000000, 1.0f, 1.0f, 0x00000C00, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通ひこうき雲", 0x03000000, 0.3f, 0.2f, 0x00010000, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {"共通ひこうきブースト", 0x00000000, 2.0f, 1.0f, 0x00000000, 0, 0, 0, nullptr, 0, {0, 0, 0}},
                                  {nullptr, 0, 0.0f, 0.0f, 0, 0, 0, 0, nullptr, 0, {0, 0, 0}}};


bool Mario::taskOnEffectCheck(u32 flags) {
    if (flags & 0x2) {
        return mActor->checkEffectWaterColumn();
    }

    return true;
}

// ---- MarioEnforce.cpp ----

void Mario::checkEnforceMove() {
    checkEnforceMoveInner();
    updateOnimasu();
}


void Mario::checkEnforceMoveInner() {
    mMovementStates._18 = 0;
    TVec3f v15(_184);
    if (mMovementStates._1 != 0 || mMovementStates.jumping != 0) {
        _184.zero();
    }

    _8D4 = nullptr;
    u32 relativeMove = moveRelativePosition(0);
    bool same = relativeMove | moveRelativePositionWall();
    if (same) {
        if (MR::isSameMtx(*_45C->getBaseMtx(), *_45C->getPrevBaseMtx())) {
            f32 verticalLimit = getMovementStates()._1 ? 0.0f : mVerticalSpeed;

            if (mVelocity.dot(_368) < 0.0f) {
                f32 element = MR::vecKillElement(mVelocity, _368, &mVelocity);
                f32 v8 = element;

                if (!mMovementStates._1) {
                    if (element < 0.0f) {
                        if (-element > mVerticalSpeed) {
                            v8 = -mVerticalSpeed;
                        }
                    }

                    mVelocity += _368 * v8;
                }
            }
        }

        mDrawStates._B = 1;

        if (!mActor->_F44) {
            addTrans(mVelocity, nullptr);
        } else {
            bool v11 = false;
            TVec3f v14(mPosition);

            if (mMovementStates._A == 0) {
                if (200.0f == _72C) {
                    v11 = true;
                }
            }

            addTrans(mVelocity, nullptr);

            if (v11) {
                if (calcDistToCeil(false) < 200.0f) {
                    mPosition = v14;
                    _1C._10 = 1;
                }
            }

            mActor->mBinder->clear();
        }

        mVelocity.zero();
        MarioActor* actor = mActor;
        actor->mPosition = mPosition;
        actor->calcCenterPos();
    }
}


void Mario::recordRelativePosition() {
    const Triangle* sameSensorFloor = nullptr;
    const TVec3f* pPosition;
    TVec3f strikeCenter(mActor->_2A0);
    _8CC[0]->mIdx = -1;
    _8CC[1]->mIdx = -1;

    f32 strikeRadius = 85.0f;
    if (mMovementStates._1 && _45C->isValid() && MR::isSameMtx(*_45C->getBaseMtx(), *_45C->getPrevBaseMtx())) {
        strikeRadius = 81.0f;
    }

    if (mMovementStates._A) {
        strikeRadius = 51.0f;
    }

    if (mActor->isJumpRising()) {
        invalidateRelativePosition();
        return;
    }

    const u32 strikeNum = static_cast< u32 >(Collision::checkStrikeBallToMap(strikeCenter, strikeRadius, nullptr, nullptr));
    const f32 floorDotThreshold = 0.707f;
    const f32 wallNormalDotThreshold = 0.9f;

    for (u32 i = 0; i < strikeNum; i++) {
        const HitInfo* hitInfo = Collision::getStrikeInfoMap(i);
        const Triangle* tri = &hitInfo->mParentTriangle;

        if (tri->mSensor == nullptr) {
            continue;
        }

        if (MR::isSameMtx(*tri->getBaseMtx(), *tri->getPrevBaseMtx())) {
            continue;
        }

        if (tri->getNormal(0)->dot(-*getGravityVec()) < floorDotThreshold) {
            if (!_8CC[0]->isValid()) {
                *_8CC[0] = *tri;
                continue;
            }

            if (tri->getNormal(0)->dot(*_8CC[0]->getNormal(0)) > wallNormalDotThreshold) {
                continue;
            }

            if (!_8CC[1]->isValid()) {
                *_8CC[1] = *tri;
                continue;
            }

            const f32 candidateDot = tri->getNormal(0)->dot(*_8CC[0]->getNormal(0));
            const f32 currentDot = _8CC[0]->getNormal(0)->dot(*_8CC[1]->getNormal(0));
            if (currentDot > candidateDot) {
                *_8CC[1] = *tri;
            }

            continue;
        }

        sameSensorFloor = tri;
        if (tri->mSensor != mHang->mWallSensor) {
            break;
        }
    }

    if (sameSensorFloor == nullptr) {
        if (isStatusActive(MarioStatus_Wall)) {
            *_8C8 = *mFrontWallTriangle;
        } else if (mMovementStates._1) {
            if (_1C._13) {
                *_8C8 = *mGroundPolygon;
            } else if (isUseAnotherMovingPolygon()) {
                *_8C8 = *mGroundPolygon;
            } else {
                invalidateRelativePosition();
            }
        } else {
            invalidateRelativePosition();
        }
    } else if (_1C._13) {
        if (sameSensorFloor->mSensor == mGroundPolygon->mSensor) {
            *_8C8 = *sameSensorFloor;
        } else if (isUseAnotherMovingPolygon()) {
            *_8C8 = *sameSensorFloor;
        } else {
            invalidateRelativePosition();
        }
    } else if (isUseAnotherMovingPolygon()) {
        *_8C8 = *sameSensorFloor;
    } else {
        invalidateRelativePosition();
    }

    if (_8C8->isValid()) {
        if (mActor->selectInvalidMovingCollision(_8C8->mSensor->mHost->mName)) {
            invalidateRelativePosition();
        } else {
            pPosition = &mActor->mPosition;
            PSMTXMultVec(*_8C8->getBaseInvMtx(), pPosition, &_8A4);
        }
    }

    if (_8CC[0]->isValid()) {
        if (mActor->selectInvalidMovingCollision(_8CC[0]->mSensor->mHost->mName)) {
            _8CC[0]->mIdx = -1;
        } else {
            pPosition = &mActor->mPosition;
            PSMTXMultVec(*_8CC[0]->getBaseInvMtx(), pPosition, &_8B0);
        }
    }

    if (_8CC[1]->isValid()) {
        if (mActor->selectInvalidMovingCollision(_8CC[1]->mSensor->mHost->mName)) {
            _8CC[1]->mIdx = -1;
        } else {
            pPosition = &mActor->mPosition;
            PSMTXMultVec(*_8CC[1]->getBaseInvMtx(), pPosition, &_8BC);
        }
    }
}


void Mario::invalidateRelativePosition() {
    _8C8->mIdx = -1;
    _8CC[0]->mIdx = -1;
    _8CC[1]->mIdx = -1;
}


TPos3f* Mario::getMoveBaseMtx() const {
    if (_8C8->isValid()) {
        return _8C8->getBaseMtx();
    }

    return nullptr;
}


u32 Mario::moveRelativePosition(u32) {
    Triangle* rTri = _8C8;
    if (!rTri->isValid()) {
        return 0;
    }

    if (MR::isSameMtx(*rTri->getBaseMtx(), *rTri->getPrevBaseMtx())) {
        return 0;
    }

    if (mActor->_EA4 && !isStatusActive(MarioStatus_Talk)) {
        invalidateRelativePosition();
        return 0;
    }

    if (rTri->mSensor != nullptr && MR::isDead(rTri->mSensor->mHost)) {
        invalidateRelativePosition();
        return 0;
    }

    TVec3f movePos;
    PSMTXMultVec(*rTri->getBaseMtx(), &_8A4, &movePos);
    _184 = movePos - mPosition;

    if (isSwimming()) {
        TVec3f dist(mActor->_2A0);
        dist -= movePos;

        TVec3f normal(*rTri->getNormal(0));
        if (dist.dot(normal) > 0.0f && dist.dot(_184) > 0.0f) {
            addVelocity(_184);
        }

        return 0;
    }

    addVelocity(_184);

    if (_8D4 == nullptr || rTri->mSensor != mHang->mWallSensor) {
        _8D4 = rTri->mSensor;
    }

    if (!MR::isSameMtxRot(*rTri->getBaseMtx(), *rTri->getPrevBaseMtx())) {
        mMovementStates._18 = 1;

        if (MR::isRotAxisY(*rTri->getBaseMtx(), *rTri->getPrevBaseMtx())) {
            _10._10 = 1;
        }

        TVec3f rotAxis;
        MR::calcMtxRotAxis(&rotAxis, *rTri->getBaseMtx(), *rTri->getPrevBaseMtx());
        if (MR::isSameDirection(rotAxis, getAirGravityVec())) {
            _10._10 = 1;
        }
    }

    return 1;
}


u32 Mario::moveRelativePositionWall() {
    u32 moved = 0;

    for (u32 i = 0; i < 2; i++) {
        Triangle* tri = _8CC[i];
        if (!tri->isValid()) {
            continue;
        }

        if (MR::isSameMtx(*tri->getBaseMtx(), *tri->getPrevBaseMtx())) {
            continue;
        }

        if (tri->mSensor != nullptr && !tri->mSensor->isValid()) {
            tri->mIdx = -1;
            continue;
        }

        const TVec3f* relPos = &mRelativeWallPos[i];
        TVec3f movePos;
        PSMTXMultVec(*tri->getBaseMtx(), relPos, &movePos);

        TVec3f add(movePos);
        add -= mPosition;

        const f32 elem = MR::vecKillElement(add, *tri->getNormal(0), &add);
        if (elem > 0.0f) {
            addVelocity(*tri->getNormal(0) * elem);
        }

        if (!MR::isSameMtxRot(*tri->getBaseMtx(), *tri->getPrevBaseMtx())) {
            mMovementStates._18 = 1;
        }

        if (_8D4 == nullptr || tri->mSensor != mHang->mWallSensor) {
            _8D4 = tri->mSensor;
        }

        moved = 1;
    }

    return moved;
}


void Mario::recordLastGround() {
    *_474 = *mGroundPolygon;
    PSMTXMultVec(*_474->getBaseInvMtx(), &mGroundPos, &_4BC);
}


void Mario::getLastGroundPos(TVec3f* pPos) const {
    if (_474->isValid()) {
        PSMTXMultVec(*_474->getBaseMtx(), &_4BC, pPos);
    } else {
        pPos->zero();
    }
}


const TVec3f* Mario::getLastGroundEdgeNrm(u32 idx) const {
    return _474->getEdgeNormal(idx);
}


u32 Mario::getLastGroundEdgeIndex(const TVec3f& rPos, const TVec3f& rDir) const {
    const TVec3f* edge0 = _474->calcAndGetEdgeNormal(0);
    const TVec3f* edge1 = _474->calcAndGetEdgeNormal(1);
    const TVec3f* edge2 = _474->calcAndGetEdgeNormal(2);

    f32 score0 = rDir.dot(*edge0);
    f32 score1 = rDir.dot(*edge1);
    f32 score2 = rDir.dot(*edge2);

    TVec3f centerAvg(*_474->calcAndGetPos(0) + *_474->calcAndGetPos(1) + *_474->calcAndGetPos(2));

    centerAvg *= 1.0f / 3.0f;

    TVec3f outDir(centerAvg - rPos);
    MR::normalizeOrZero(&outDir);

    const f32 add0 = outDir.dot(*edge0);
    const f32 add1 = outDir.dot(*edge1);
    const f32 add2 = outDir.dot(*edge2);
    if (add0 > 0.0f) {
        score0 += add0;
    }

    if (add1 > 0.0f) {
        score1 += add1;
    }

    if (add2 > 0.0f) {
        score2 += add2;
    }

    if (score0 < score1 && score0 < score2) {
        return 0;
    }

    if (score1 < score0 && score1 < score2) {
        return 1;
    }

    return 2;
}


void Mario::pushedByReaction() {
    addVelocity(_928);
    _928 *= _934;
}


void Mario::addReaction(const TVec3f& rReact) {
    _928 += rReact;
}


void Mario::tryPushToVelocity() {
    _1B4 = mVelocity;
    pushedByWind();
    pushedByReaction();

    if (!MR::isNearZero(_350)) {
        if (isSwimming()) {
            TVec3f pushVec(_350);
            pushVec += _35C;
            _350.zero();
            _35C.zero();
            createAtField(true, pushVec.length());

            for (u32 i = 0; i < _578; i++) {
                TVec3f projected;
                if (MR::vecKillElement(pushVec, *_57C[i]->getNormal(0), &projected) < 0.0f) {
                    pushVec = projected;
                }
            }

            mVelocity += pushVec;
        } else if (!isStatusActive(MarioStatus_Hang)) {
            if (isStatusActive(MarioStatus_Bury)) {
                closeStatus(mBury);
            } else {
                if (mMovementStates.jumping && !mMovementStates._1) {
                    TVec3f pushDir(_350);
                    MR::normalizeOrZero(&pushDir);

                    if (mJumpVec.dot(_350) < 0.0f) {
                        cutVecElementFromJumpVec(pushDir);
                    }

                    if (mVelocity.dot(_350) < 0.0f) {
                        MR::vecKillElement(mVelocity, pushDir, &mVelocity);
                    }
                } else if (isDamaging() && mMovementStates._1) {
                    MR::vecKillElement(_350, *getGravityVec(), &_350);
                }

                mVelocity += _350;
            }
        }

        _1C._15 = 1;
    }

    mVelocity += _35C;
    _350.zero();
    _35C.zero();
}


void Mario::push(const TVec3f& rVec) {
    _350 += rVec;
    _1C._15 = 1;
}


void Mario::push2(const TVec3f& rVec) {
    _35C += rVec;
    _1C._15 = 1;
}


void Mario::powerAreaMove() {
    _190.zero();

    if (mMovementStates._1 && !mMovementStates.jumping && _960 == 28 && MR::calcAreaMoveVelocity(&_190, mPosition)) {
        if (mMovementStates._8 || mMovementStates._19 || mMovementStates._1A) {
            const TVec3f* norm = &getWallNorm();
            f32 dot = norm->dot(_190);

            if (dot < 0.0f) {
                MR::vecKillElement(_190, *norm, &_190);
                MR::vecKillElement(_190, *getGravityVec(), &_190);
            }
        }

        addVelocity(_190);
    }
}


void Mario::powerRailMove() {
    _19C.zero();

    if (mMovementStates._1 && !mMovementStates.jumping && _960 == 0x1B) {
        const LiveActor* host = getGroundPolygon()->mSensor->mHost;
        if (host == nullptr) {
            return;
        }

        TVec3f railDir;
        MR::calcNearestRailDirection(&railDir, host, mPosition);

        s32 arg = -1;
        MR::getRailArg3NoInit(host, &arg);
        s32 speed = arg;
        if (speed == -1) {
            speed = 10;
        }

        MR::normalizeOrZero(&railDir);

        if (mMovementStates._8 || mMovementStates._19 || mMovementStates._1A) {
            const TVec3f* wallNorm = &getWallNorm();
            if (wallNorm->dot(railDir) < 0.0f) {
                MR::vecKillElement(railDir, *wallNorm, &railDir);
                MR::vecKillElement(railDir, *getGravityVec(), &railDir);
            }
        }

        TVec3f velocity(railDir);
        velocity.scale(speed);
        _19C = velocity;
        addVelocity(_19C);
    }
}


void Mario::recordJumpEnforceMove() {
    _10._17 = 1;
    _8D8 = _8D4;
    _8DC = _184;
    *_8E8 = *_8C8;
}


void Mario::doEnforceJump(f32 param) {
    if (mJumpVec.dot(_184) < 0.0f) {
        mDrawStates._1B = 1;
        initActiveJumpVec();
    }

    _8DC = _184 * param;

    f32 element = MR::vecKillElement(_8DC, getAirGravityVec(), &_8DC);

    mJumpVec = _8DC + mJumpVec;

    if (element < 0.0f) {
        mJumpVec += getAirGravityVec() * element;
    }

    mVelocity -= _184;
    invalidateRelativePosition();
}


void Mario::pushedByWind() {
    f32 windMag = _91C.length();

    TVec3f windDir(_91C);
    MR::normalizeOrZero(&windDir);

    if (MR::isNearZero(windDir)) {
        if (isAnimationRun("向かい風ふんばり")) {
            stopAnimation("向かい風ふんばり");
        }

        if (isAnimationRun("向かい風走り")) {
            stopAnimation("向かい風走り");
        }

        if (_10.debugMode) {
            getPlayerMode();
        }

        _10.debugMode = 0;
        return;
    }

    if (mMovementStates.jumping) {
        const MarioConstTable* table = mActor->getConst().getTable();
        mJumpVec += _91C * table->mWindJumpingFriction;
        return;
    }

    if (mTargetWalkSpeedIndex == 0) {
        const MarioConstTable* table = mActor->getConst().getTable();
        if (windMag > table->mWindSlideLimit) {
            windMag -= table->mWindSlideLimit;
            windMag *= table->mWindSlideFriction;
            _350 += windDir * windMag;
            _1C._15 = 1;
            changeAnimation("向かい風ふんばり");
        }
        return;
    }

    TVec3f scaled(windDir * windMag);

    if (mVelocity.dot(windDir) > 0.0f) {
        const MarioConstTable* table = mActor->getConst().getTable();
        _350 += scaled * table->mWindForwardFriction;
        _1C._15 = 1;
        stopAnimation("向かい風ふんばり");
        stopAnimation("向かい風走り");
        return;
    }

    TVec3f killed;
    f32 element = MR::vecKillElement(scaled + mVelocity, windDir, &killed);

    const MarioConstTable* table = mActor->getConst().getTable();
    if (element > table->mWindSlideLimit) {
        element -= table->mWindSlideLimit;
        element *= table->mWindSlideFrictionAgainst;
    } else if (element > 0.0f) {
        element = 0.0f;
    }

    clearVelocity();

    scaled = killed + windDir * element;

    changeAnimation("向かい風走り");
    _350 += scaled;
    _1C._15 = 1;
}

// ---- MarioFaint.cpp ----

bool Mario::doFlipWeak(const TVec3f& rVec) {
    if (mMovementStates._1B) {
        return false;
    }

    mFaint->mNoDamage = true;

    if (faint(rVec)) {
        if (isStatusActive(MarioStatus_Swim)) {
            mSwim->mDamageType = 1;
            mFaint->mNoDamage = false;
        }

        return true;
    }

    mFaint->mNoDamage = false;
    forceStopTornado();
    return false;
}


bool Mario::faint(const TVec3f& rVec) {
    _7C4 = rVec;
    if (!isEnableAddDamage()) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_Damage) {
        return false;
    }

    mFaint->setVec(rVec);
    stopWalk();
    forceStopTornado();
    mActor->damageDropThrowMemoSensor();

    if (getCurrentStatus() != MarioStatus_Faint) {
        mMovementStates._27 = true;
        return true;
    }

    return mMovementStates._27;
}

// ---- MarioFlip.cpp ----

bool Mario::doFlipJump(const TVec3f& rVec) {
    // FIXME: regswap
    if (isAnimationRun("壁はじき")) {
        return false;
    }

    stopWalk();
    TVec3f vec(rVec);
    vec.setLength(mActor->mConst->getTable()->mWallSpinFlipGround);
    vec -= getGravityVec()->scaleInline(mActor->mConst->getTable()->mWallSpinHopGround);
    tryForcePowerJump(vec, true);

    changeAnimation("壁はじき");

    stopPunch();
    mMovementStates._2B = true;
    _430 = 3;
    startPadVib(2);

    return true;
}


bool Mario::doFlipBackRoll(const TVec3f& rVec) {
    TVec3f vec20;
    MR::vecKillElement(rVec, getAirGravityVec(), &vec20);

    if (doFlipLarge(vec20)) {
        checkDamage();
        changeAnimationNonStop("後転ふっとび");

        mDamage->setStrings("後転ふっとび空中", "後転ふっとび着地");

        TVec3f vec28;
        MR::vecKillElement(rVec, getAirGravityVec(), &vec28);
        setFrontVecKeepUp(-vec28, static_cast< u32 >(12l));

        return true;
    }

    return false;
}


bool Mario::doFlipRot(const TVec3f& rVec) {
    getCurrentStatus();
    if (getCurrentStatus() == MarioStatus_FireDamage) {
        return false;
    }

    if (mMovementStates._1B) {
        return false;
    }

    mFlip->setVec(rVec);

    stopJump();
    stopWalk();
    forceStopTornado();

    changeStatus(mFlip);

    return true;
}


bool Mario::doFlipLarge(const TVec3f& rVec) {
    if (getCurrentStatus() == MarioStatus_Damage) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_Flip) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_FireDamage) {
        return false;
    }

    if (mMovementStates._1B) {
        return false;
    }

    return flipLarge(rVec);
}

// ---- MarioFlow.cpp ----

bool Mario::doFlow() {
    if (isStatusActive(MarioStatus_Flow)) {
        return false;
    }

    if (isAnimationRun("空中ひねり")) {
        return false;
    }

    changeStatus(mFlow);

    return true;
}

// ---- MarioFoo.cpp ----

void MarioFoo_FORCE_MATCH_SDATA2() {
    (void)0.52359879f;
    (void)0.100000001f;
    (void)120.0f;
    (void)0.125f;
    (void)10.0f;
    (void)100.0f;
    (void)0.0500000007f;
    (void)5.0f;
    (void)1.04719758f;
    (void)1.10000002f;
    (void)-0.100000001f;
    (void)0.00999999978f;
    (void)25.0f;
    (void)0.980000019f;
    (void)1.01999998f;
    (void)0.899999976f;
    (void)1.20000005f;
    (void)50.0f;
    (void)1.5f;
    (void)12.0f;
    (void)2.09439516f;
    (void)0.314159274f;
    (void)0.785398185f;
    (void)0.25f;
    (void)30.0f;
    (void)0.015625f;
    (void)70.0f;
    (void)2000.0f;
    (void)255.0f;
    (void)1.00010002f;
    (void)6.0f;
}


void MarioFoo_FORCE_MATCH(TVec3f& rVec, f32 a, f32 b, f32 c) {
    rVec.setLength(a);
    rVec.setLength(b);
    rVec.setLength(c);
}


namespace {
    f32 cTurnMotionSpeed = 5.0f;
    f32 cWidth = 70.0f;
    f32 cLimitAngleSink = JGeometry::TUtil< f32 >::PI() / 1.0001f;
    f32 cNeutralAngleWait = JGeometry::TUtil< f32 >::PI() / 2.0f - JGeometry::TUtil< f32 >::PI() / 6.0f;
    f32 cLimitAngleWait = JGeometry::TUtil< f32 >::PI() / 1.0001f;
    f32 cUpperAngleWait = JGeometry::TUtil< f32 >::PI() / 100.0f;
};
  // namespace

void Mario::tryStartFoo() {
    if (MR::getAreaObj("CelestrialSphere", getTrans()) || !mFoo->_48) {
        changeStatus(mFoo);
        mFoo->setStartSpeed(mActor->getConst().getTable()->mSwimFrontMaxSpeed);
    }
}


const char* MarioFoo_FORCE_MATCH_DATA(u32 index) {
    static const char* const strings[] = {"水泳ダメージ", "水中ダメージ", "声小ダメージ", "ダメージ", "水泳ダメージ中"};
    return strings[index];
}

// ---- MarioFpView.cpp ----

namespace {
    static bool sIsDisabled;
};
  // namespace

bool Mario::isDisableStayHere() const {
    if (getMovementStates().jumping) {
        return true;
    }

    if (!getMovementStates()._1) {
        return true;
    }

    if (_3CE < 5) {
        return true;
    }

    if (getMovementStates()._A) {
        return true;
    }

    if (isDamaging()) {
        return true;
    }

    if (getMovementStates()._1B) {
        return true;
    }

    if (MR::getKarikariClingNum() != 0) {
        return true;
    }

    if (getPlayer()->getMovementStates()._23) {
        return true;
    }

    if (mWalkSpeed > 0.01f) {
        return true;
    }

    if (getStickP() != 0.0f) {
        return true;
    }

    if (_10.debugMode) {
        return true;
    }

    if (_960 == 27) {
        return true;
    }

    if (_960 == 28) {
        return true;
    }

    if (getDrawStates()._B) {
        return true;
    }

    if (getMovementStates()._18) {
        return true;
    }

    if (!MR::isNearZero(_928)) {
        return true;
    }

    if (!MR::isNearZero(_350)) {
        return true;
    }

    if (!MR::isNearZero(_35C)) {
        return true;
    }

    if (!MR::isNearZero(_184)) {
        return true;
    }

    if (mActor->mBeeWallWalk != 0) {
        return true;
    }

    if (getMovementStates()._37) {
        return true;
    }

    if (getMovementStates()._3A) {
        return true;
    }

    if (_10._15) {
        return true;
    }

    if (mSinkTimer) {
        return true;
    }

    if (mActor->_EA4) {
        return true;
    }

    return mActor->_3C0;
}


bool Mario::isDisableFpViewMode() const {
    if (_97C) {
        return true;
    }

    return isDisableStayHere();
}


void Mario::tryFpViewMode() {
    if (isStatusActive(MarioStatus_FpView)) {
        return;
    }

    stopWalk();

    changeStatus(mFpView);

    MR::startSystemSE("SE_SY_CAMERA_FP_VIEW");
}

// ---- MarioFrontStep.cpp ----

bool Mario::doFrontStep() {
    if (getCurrentStatus() == MarioStatus_FrontStep) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_SideStep) {
        return false;
    }

    if (checkWallCode("NotSideStep", false)) {
        return false;
    }

    if (checkWallCode("NoAction", false)) {
        return false;
    }

    if (mActor->_468 != 0) {
        return false;
    }

    if (_1C._5) {
        return false;
    }

    if (mMovementStates.jumping) {
        return false;
    }

    if (getPlayerMode() == PlayerMode_Hopper) {
        return false;
    }

    if (_960 == 2) {
        return false;
    }

    if (calcAngleD(_368) >= 30.0f) {
        return false;
    }

    if (mAirGravityVec.dot(_374) >= 0.25f) {
        return false;
    }

    Triangle triangle = Triangle();
    MarioActor* pMario = mActor;
    TVec3f* vec = &pMario->_2A0;
    if (!MR::getFirstPolyOnLineToMap(nullptr, &triangle, *vec, mFrontVec * 200.0f)) {
        return false;
    }

    setFrontVecKeepUp(-*MR::getNormal(&triangle));

    stopWalk();
    forceStopTornado();

    changeStatus(mFrontStep);

    return true;
}

// ---- MarioHang.cpp ----

void MarioHang_FORCE_MATCH_SDATA2() {
    (void)-0.8f;
    (void)50.0f;
    (void)5.0f;
    (void)60.0f;
    (void)160.0f;
    (void)0.1f;
    (void)0.08726647f;
    (void)100.0f;
    (void)150.0f;
    (void)0.2f;
    (void)30.0f;
    (void)-0.707f;
    (void)84.0f;
    (void)-20.0f;
    (void)-100.0f;
    (void)0.707f;
    (void)45.0f;
    (void)80.0f;
    (void)6.0f;
    (void)0.95f;
    (void)20.0f;
    (void)15.0f;
    (void)55.0f;
    (void)-55.0f;
    (void)-15.0f;
    (void)-150.0f;
    (void)0.001f;
    (void)0.01f;
}


bool Mario::isHanging() const {
    return getCurrentStatus() == MarioStatus_Hang;
}


bool Mario::fixHangDir(const TVec3f& rPosition, TVec3f* pDirection) {
    TVec3f horizontal(rPosition - mPosition);
    MR::vecKillElement(horizontal, *getGravityVec(), &horizontal);
    MR::normalizeOrZero(&horizontal);
    Triangle pTriangle;
    if (MR::getFirstPolyOnLineToMap(nullptr, &pTriangle, mPosition, horizontal * 150.0f)) {
        *pDirection = -*pTriangle.getNormal(0);
        mHang->recordWallPolygon(&pTriangle);
        return true;
    }

    if (pDirection->dot(horizontal) < 0.0f) {
        *pDirection = horizontal;
        return false;
    }

    return false;
}


void Mario::checkHang() {
    if (mHang->mHangTimer) {
        return;
    }

    if (mMovementStates.jumping && isRising()) {
        mMovementStates._30 = false;
    }

    if (mMovementStates.jumping && _3BC > 20) {
        mMovementStates._30 = false;
    }

    if (isCeiling()) {
        mMovementStates._31 = false;
    }

    if (!mMovementStates._31) {
        return;
    }

    if (getPlayer()->mDrawStates.mIsUnderwater) {
        return;
    }

    if (isStatusActive(MarioStatus_Rabbit)) {
        return;
    }

    if (mActor->_468) {
        return;
    }

    if (getPlayerMode() == PlayerMode_Bee) {
        return;
    }

    if (getPlayerMode() == PlayerMode_Teresa) {
        return;
    }

    bool front = true;
    bool back = true;
    bool side = true;
    if (mMovementStates._1A) {
        const Triangle* pTriangle = mSideWallTriangle;
        if (!MR::isSameMtx(pTriangle->getBaseMtx()->toMtxPtr(), pTriangle->getPrevBaseMtx()->toMtxPtr())) {
            front = false;
            back = false;
        }
    }

    if (mMovementStates._8) {
        const Triangle* pTriangle = mFrontWallTriangle;
        if (!MR::isSameMtx(pTriangle->getBaseMtx()->toMtxPtr(), pTriangle->getPrevBaseMtx()->toMtxPtr())) {
            side = false;
            back = false;
        }
    }

    if (mMovementStates._19) {
        const Triangle* pTriangle = mBackWallTriangle;
        if (!MR::isSameMtx(pTriangle->getBaseMtx()->toMtxPtr(), pTriangle->getPrevBaseMtx()->toMtxPtr())) {
            front = false;
            side = false;
        }
    }

    TVec3f previousPosition(mPosition);
    if (front && mMovementStates._39 && isEnableHang()) {
        TVec3f direction(_4A4 - mPosition);
        MR::vecKillElement(direction, getAirGravityVec(), &direction);
        MR::normalizeOrZero(&direction);
        if (getFrontWallNorm().dot(direction) >= -0.8f) {
            return;
        }

        if (!MR::isExistMapCollision(_4A4 - mFrontVec * 50.0f - getAirGravityVec() * 5.0f, mFrontVec * 60.0f)) {
            u8 started = 1;
            if (isAnimationRun("落下")) {
                started = 0;
            }

            changeStatus(mHang);
            mVelocity.zero();
            stopWalk();
            stopJump();
            setTrans(_4A4, nullptr);
            setFrontVecKeepUp(-getWallNorm());
            mHang->recordWallPolygon(mFrontWallTriangle);
            mHang->_1E = started;
        }
    } else if (back && isEnableBackHang()) {
        TVec3f position;
        getLastGroundPos(&position);
        if ((mPosition - position).dot(getAirGravityVec()) >= 160.0f) {
            return;
        }

        TVec3f direction(-mFrontVec);
        u32 edge = getLastGroundEdgeIndex(mActor->_2A0, direction);
        direction = _4F4 - mActor->_2A0;
        MR::normalizeOrZero(&direction);
        u32 positionEdge = getLastGroundEdgeIndex(mActor->_2A0, direction);
        direction = -*mBackWallTriangle->getNormal(0);
        u32 normalEdge = getLastGroundEdgeIndex(mActor->_2A0, direction);
        TVec3f normal;
        if (edge == positionEdge && edge == normalEdge) {
            normal = *getLastGroundEdgeNrm(edge);
        } else {
            return;
        }

        if (normal.dot(getAirGravityVec()) >= 0.1f) {
            return;
        }

        if (normal.dot(mHeadVec) >= 0.1f) {
            return;
        }

        direction = -*mBackWallTriangle->getNormal(0);
        if (MR::diffAngleAbsHorizontal(normal, -direction, getAirGravityVec()) > 0.08726647f) {
            return;
        }

        TVec3f horizontal;
        f32 distance = 0.0f;
        switch (edge) {
        case 0:
        case 1:
            distance = MR::vecKillElement(*_474->calcAndGetPos(0) - position, normal, &horizontal);
            position += normal * distance;
            break;
        case 2:
            distance = MR::vecKillElement(*_474->calcAndGetPos(1) - position, normal, &horizontal);
            position += normal * distance;
            break;
        }

        if (distance > 100.0f) {
            return;
        }

        MR::vecKillElement(normal, getAirGravityVec(), &normal);
        MR::normalizeOrZero(&normal);
        if (!MR::isExistMapCollision(position - direction * 50.0f - getAirGravityVec() * 5.0f, direction * 60.0f)) {
            changeStatus(mHang);
            mVelocity.zero();
            stopWalk();
            stopJump();
            TVec3f hangDirection;
            bool found = fixHangDir(position, &hangDirection);
            setTrans(position, nullptr);
            setFrontVecKeepUp(normal);
            mMovementStates._1D = true;
            if (!found) {
                mHang->recordWallPolygon(mBackWallTriangle);
            }

            mHang->recordHangNorm(normal);
            mHang->_1E = 0;
        }
    } else if (side && isEnableSideHang()) {
        TVec3f position;
        getLastGroundPos(&position);
        if ((mPosition - position).dot(getAirGravityVec()) >= 160.0f) {
            return;
        }

        TVec3f direction(_500 - mActor->_2A0);
        MR::normalizeOrZero(&direction);
        u32 edge = getLastGroundEdgeIndex(mActor->_2A0, direction);
        TVec3f sideDirection(mSideVec);
        if (sideDirection.dot(direction) < 0.0f) {
            direction = -sideDirection;
        } else {
            direction = sideDirection;
        }

        u32 sideEdge = getLastGroundEdgeIndex(mActor->_2A0, direction);
        direction = -*mSideWallTriangle->getNormal(0);
        u32 normalEdge = getLastGroundEdgeIndex(mActor->_2A0, direction);
        TVec3f normal;
        if (edge == sideEdge && edge == normalEdge) {
            normal = *getLastGroundEdgeNrm(edge);
        } else {
            return;
        }

        if (normal.dot(getAirGravityVec()) >= 0.1f) {
            return;
        }

        if (normal.dot(mHeadVec) >= 0.1f) {
            return;
        }

        direction = -*mSideWallTriangle->getNormal(0);
        if (MR::diffAngleAbsHorizontal(normal, -direction, getAirGravityVec()) > 0.08726647f) {
            return;
        }

        MR::vecKillElement(normal, getAirGravityVec(), &normal);
        MR::normalizeOrZero(&normal);
        if (!MR::isExistMapCollision(position - direction * 50.0f - getAirGravityVec() * 5.0f, direction * 60.0f)) {
            changeStatus(mHang);
            mVelocity.zero();
            stopWalk();
            stopJump();
            TVec3f hangDirection;
            bool found = fixHangDir(position, &hangDirection);
            setTrans(position, nullptr);
            setFrontVecKeepUp(normal);
            mMovementStates._1D = true;
            if (!found) {
                mHang->recordWallPolygon(mSideWallTriangle);
            }

            mHang->recordHangNorm(normal);
            mHang->_1E = 0;
        }
    }

    if (isStatusActive(MarioStatus_Hang)) {
        _414 = 0;
        for (u32 i = 0; i < 10; i++) {
            updateGroundInfo();
            if (mMovementStates._1) {
                return;
            }

            addTrans(mFrontVec * 1.0f - getAirGravityVec() * 1.0f, nullptr);
        }
    }
}


bool Mario::isEnableHang() {
    if (!mMovementStates._8) {
        return false;
    }

    if (!mMovementStates._15) {
        return false;
    }

    if (mMovementStates._B) {
        return false;
    }

    if (mDrawStates._1E) {
        return false;
    }

    if (isStatusActive(MarioStatus_Recovery)) {
        return false;
    }

    if (checkWallCode("NotGrab", false)) {
        return false;
    }

    if (checkWallCode("NoAction", false)) {
        return false;
    }

    const char* pWallCode = MR::getWallCodeString(_47C);
    if (pWallCode != nullptr) {
        if (strcmp(pWallCode, "NotGrab") == 0) {
            return false;
        }

        if (strcmp(pWallCode, "NoAction") == 0) {
            return false;
        }
    }

    TVec3f shadowPos(mShadowPos);
    TVec3f up(-getAirGravityVec());
    f32 slope = _45C->getNormal(0)->dot(up);
    f32 height = mActor->getConst().getTable()->mWallHangGrHeight;
    if (slope > 0.0f) {
        if (slope > 0.2f) {
            slope = 0.2f;
        }

        height -= 30.0f * (5.0f * slope);
    }

    if (getPlayer()->getShadowNorm().dot(*getGravityVec()) < -0.707f) {
        if ((_4A4 - mShadowPos).dot(-*getGravityVec()) < height) {
            return false;
        }
    }

    f32 heightFromMario = (_4A4 - mPosition).dot(-*getGravityVec());
    if (heightFromMario > mActor->getConst().getTable()->mWallHangMyHeight) {
        return false;
    }

    return !isRising();
}


bool Mario::isEnableBackHang() {
    bool turn = false;
    if (!_474->isValid()) {
        return false;
    }

    if (_430 == 6) {
        if (isRising()) {
            return false;
        }

        turn = true;
    } else if (!mMovementStates._30) {
        return false;
    }

    if (!mMovementStates._19) {
        return false;
    }

    if (mMovementStates._B) {
        return false;
    }

    if (checkWallCode("NotGrab", false)) {
        return false;
    }

    const char* pWallCode = MR::getWallCodeString(_474);
    if (pWallCode != nullptr && strcmp(pWallCode, "NotGrab") == 0) {
        return false;
    }

    if (isRising()) {
        return false;
    }

    if (mStickPos.z > mActor->getConst().getTable()->mWallBackHangStickPower) {
        return false;
    }

    if (mWalkSpeed > mActor->getConst().getTable()->mWallBackHangWalkSpeed) {
        return false;
    }

    TVec3f ground;
    TVec3f horizontal;
    if (getPlayer()->getShadowNorm().dot(*getGravityVec()) < -0.707f) {
        if (turn) {
            ground = mPosition;
        } else {
            getLastGroundPos(&ground);
        }

        f32 height = MR::vecKillElement(mShadowPos - ground, *getGravityVec(), &horizontal);
        if (height < mActor->getConst().getTable()->mWallHangGrHeight) {
            return false;
        }
    }

    if (!turn) {
        if ((_4F4 - mGroundPos).dot(*getGravityVec()) < 0.0f) {
            return false;
        }
    } else {
        setFrontVecKeepUp(-mFrontVec);
        return false;
    }

    TVec3f distance;
    TVec3f position;
    getLastGroundPos(&position);
    MR::vecKillElement(position - mPosition, getAirGravityVec(), &distance);
    return !(distance.length() >= 84.0f);
}


bool Mario::isEnableSideHang() {
    if (_1C._1 || _1C._2) {
        return false;
    }

    if (!mMovementStates._30) {
        return false;
    }

    if (!mMovementStates._1A) {
        return false;
    }

    if (checkWallCode("NotGrab", false)) {
        return false;
    }

    if (checkWallCode("NoAction", false)) {
        return false;
    }

    const char* pWallCode = MR::getWallCodeString(_474);
    if (pWallCode != nullptr) {
        if (strcmp(pWallCode, "NotGrab") == 0) {
            return false;
        }

        if (strcmp(pWallCode, "NoAction") == 0) {
            return false;
        }
    }

    if (isRising()) {
        return false;
    }

    if (mStickPos.z > mActor->getConst().getTable()->mWallBackHangStickPower) {
        return false;
    }

    if (mWalkSpeed > mActor->getConst().getTable()->mWallBackHangWalkSpeed) {
        return false;
    }

    TVec3f ground;
    TVec3f horizontal;
    if (getPlayer()->getShadowNorm().dot(*getGravityVec()) < -0.707f) {
        getLastGroundPos(&ground);
        f32 height = MR::vecKillElement(mShadowPos - ground, *getGravityVec(), &horizontal);
        if (height < mActor->getConst().getTable()->mWallHangGrHeight) {
            return false;
        }
    }

    if (!mMovementStates._33) {
        return false;
    }

    if ((_500 - mGroundPos).dot(*getGravityVec()) < -20.0f) {
        return false;
    }

    TVec3f distance;
    TVec3f position;
    getLastGroundPos(&position);
    MR::vecKillElement(position - mPosition, getAirGravityVec(), &distance);
    return !(distance.length() >= 84.0f);
}

// ---- MarioInit.cpp ----

void Mario::initMember() {
    _40.zero();
    _4C.zero();
    PSMTXIdentity(_F4.toMtxPtr());
    mPosition.zero();
    _16C.zero();
    mVelocityAfter.zero();
    _1B4.zero();
    _274 = 0;
    _280 = 0.0f;
    _284.zero();
    _2B8.zero();
    _2C4.zero();
    _304.zero();
    mShadowPos.zero();

    _340 = 0.0f;
    _3A4.zero();
    _3B0.zero();

    _422 = 0;
    _42A = 0;
    _438 = 0;
    _448.zero();

    _454 = 0.0f;
    mGroundPos.zero();
    _498.zero();
    _4BC.zero();
    _50C.zero();
    _518.zero();
    _600.zero();
    _60C = 0;
    _638 = 0;
    _660.zero();
    _688.zero();
    _694.zero();

    _6C8.y = 0.0f;
    _6C8.x = 0.0f;
    _6D0.zero();
    _6DC.zero();
    _6E8.zero();
    _6F4.zero();
    _700.zero();
    _70C.zero();

    _718 = 0.0f;
    _71D = 0;
    _71F = 0;
    _720 = nullptr;
    _724 = nullptr;
    _728 = nullptr;

    _75C.zero();
    _790.zero();
    _7C4.zero();
    _7D4.zero();
    PSMTXIdentity(_7E4.toMtxPtr());
    _814.zero();
    PSMTXIdentity(_824.toMtxPtr());

    _8B0.zero();
    _8BC.zero();
    _8DC.zero();

    _8EC = 0;
    _8F4 = 0.0f;

    for (int i = 0; i < ARRAY_SIZE(_A08); i++) {
        _A08[i] = 0;
    }

    _A34 = 0;
    _A3C = 0;

    _A40.zero();
    _A4C.zero();
    _A58.zero();
    _A64 = 0.0f;

    for (int i = 0; i < ARRAY_SIZE(_A6C); i++) {
        _A6C[i] = 0;
    }
}

// ---- MarioJump.cpp ----

namespace {
    f32 cDropFrontSpeed = 2.0f;
};
  // namespace

void MarioJump_FORCE_MATCH_SDATA2() {
    (void)0.00100000005f;
    (void)15.0f;
    (void)0.300000012f;
    (void)4.0f;
    (void)10.0f;
    (void)0.899999976f;
    (void)500.0f;
    (void)0.100000001f;
    (void)5.0f;
    (void)100.0f;
    (void)50.0f;
    (void)150.0f;
    (void)0.600000024f;
    (void)80.0f;
    (void)20.0f;
    (void)8.0f;
    (void)0.200000003f;
    (void)0.800000012f;
    (void)0.00390625f;
    (void)45.0f;
    (void)160.0f;
    (void)1.79999995f;
    (void)0.25f;
    (void)300.0f;
    (void)200.0f;
    (void)-0.100000001f;
    (void)-0.800000012f;
    (void)30.0f;
    (void)-0.200000003f;
    (void)0.785398185f;
    (void)0.998000026f;
    (void)0.00999999978f;
    (void)6.0f;
    (void)7.0f;
    (void)90.0f;
}


bool Mario::isRising() const {
    if (getPlayerMode() == 4 || getPlayerMode() == 6) {
        if (_16C.dot(*getGravityVec()) < 0.0f) {
            return true;
        }

        return false;
    }

    return mJumpVec.dot(*getGravityVec()) < 0.0f;
}


void Mario::checkWallRiseAndSlipFront() {
    mMovementStates._6 = false;
    if (mMovementStates._8) {
        mMovementStates._9 = false;
        mWalkSpeed = 0.0f;
        return;
    }

    mMovementStates._9 = true;
}


void Mario::tryJump() {
    bool isMudFloorJump = false;
    bool useStickJumpAnim = false;

    TVec3f stickTotal(_190 + _19C);

    if (mMovementStates.jumping) {
        return;
    }

    if (mSinkTimer != 0) {
        if (checkCurrentFloorCodeSevere(0x19)) {
            if (mSinkTimer > 32) {
                mSinkTimer -= 32;
            }

            if (mSinkTimer < 64) {
                mSinkTimer = 64;
            }

            mMovementStates._38 = false;
            return;
        }

        if (checkCurrentFloorCodeSevere(0x1F)) {
            return;
        }

        if (mSinkTimer > 100) {
            if (mSinkTimer > 200) {
                mSinkTimer = 128;
                changeAnimation("埋まりジャンプA");
            } else {
                mSinkTimer = 32;
                changeAnimation("埋まりジャンプB");
            }

            if (checkCurrentFloorCodeSevere(0x12)) {
                playSound("毒沼脱出");
            } else {
                playSound("砂脱出");
            }

            playSound("声砂脱出");
            return;
        }
    }

    playEffect("共通跳躍");
    stopEffectForce("スピンリング");

    const bool isSquat = checkSquat(true);
    mMovementStates._21 = true;
    mMovementStates._29 = false;
    mMovementStates._38 = false;
    _10._23 = true;
    mMovementStates._B = false;
    _42A = 0;

    if (_3CE > mActor->getConst().getTable()->mJumpConnectTime || !mMovementStates._5) {
        _430 = 0;
    } else {
        _430++;
    }

    if (_430 > 2) {
        _430 = 0;
    }

    if (_430 == 2 && mWalkSpeed < mActor->getConst().getTable()->mJumpConnectSpeed) {
        _430 = 0;
    }

    if (getPlayerMode() == 4 || getPlayerMode() == 6) {
        _430 = 0;
    }

    if (_1C._B) {
        _430 = 2;
        MR::start2PJumpAssistJustSound();
    } else if (_1C._A) {
        MR::start2PJumpAssistSound();
    }

    if (isPlayerModeInvincible() && mWalkSpeed > 1.0f) {
        _430 = 1;
        _42A = 1;
    }

    if (!MR::isNearZero(stickTotal) && mSinkTimer != 0) {
        _430 = 0;
        mMovementStates._4 = false;
        _3D2 = 0;
    }

    forceStopTornado();

    u32 floorCode = getFloorCode();
    if (floorCode != 0x20 && getPlayerMode() != 4 && (((mMovementStates._4) != 0) || (_3D2 != 0 && static_cast< u8 >(checkStickFrontBack()) == 2))) {
        if ((mActor->mConst->getTable()->mTurnSlipTime - _3D0) >= mActor->getConst().getTable()->mTurnJumpInhibitTime) {
            tryTurnJump();
            return;
        }

        _3D0 = 0;
        mMovementStates._4 = false;
    }

    checkWallRiseAndSlipFront();

    const f32 floorAngle = calcPolygonAngleD(mGroundPolygon);
    useStickJumpAnim = false;
    if (!MR::isNearZero(_8F8) && (mMovementStates._23)) {
        useStickJumpAnim = true;
    }

    if (floorAngle < 15.0f) {
        mMovementStates._23 = false;
        _8F8.zero();
    }

    if (getPlayerMode() != 6 && !_10._1A) {
        if (mDrawStates._C && isSlipPolygon(_45C)) {
            const bool dotFront = mFrontVec.dot(_368) < 0.0f;
            const bool dotPad = mWorldPadDir.dot(_368) <= 0.0f;
            const bool diff = MR::diffAngleAbsHorizontal(_16C, _368, getAirGravityVec()) > 1.5707964f;
            const bool movementstate = mMovementStates._A;

            if (dotFront || dotPad || diff || movementstate) {
                if (dotFront && !dotPad) {
                    mMovementStates._2B = true;
                    _430 = 0;
                } else {
                    mWalkSpeed *= 0.5f;
                    if (getPlayerMode() != 4) {
                        _402 = 0;
                        _430 = 3;
                        mMovementStates._2B = true;
                    }

                    useStickJumpAnim = false;
                }
            } else {
                mMovementStates._23 = false;
            }
        }

        if (checkCurrentFloorCodeSevere(0x11)) {
            isMudFloorJump = true;
            _430 = 3;
            mWalkSpeed *= 0.5f;
        }
    }

    floorCode = getFloorCode();
    if (floorCode != 0x20 && getPlayerMode() != 4 && _430 != 3) {
        if (isSquat && mWalkSpeed > 0.3f && ((mMovementStates._23) == 0)) {
            trySquatJump();
            return;
        }

        if ((mMovementStates._A) && ((mMovementStates._23) == 0)) {
            tryBackJump();
            return;
        }
    }

    if (mDrawStates._5) {
        mJumpVec = _334 * mWalkSpeed * mActor->getConst().getTable()->mJumpFrontSpeed;
    } else {
        mJumpVec = mFrontVec * mWalkSpeed * mActor->getConst().getTable()->mJumpFrontSpeed;
    }

    _10._17 = false;
    if (mDrawStates._B) {
        recordJumpEnforceMove();

        f32 enforce;
        if (getStickP() == 0.0f) {
            mJumpVec = _184;
            mJumpVec.zero();
            enforce = 1.0f;
        } else {
            const f32 angle = MR::diffAngleAbsHorizontal(mFrontVec, _184, getAirGravityVec());
            enforce = 1.0f - (angle / 1.5707964f);
            if (enforce < 0.0f) {
                enforce = 0.0f;
            }

            const f32 speed = mJumpVec.length();
            const f32 speedBonus = MR::clamp(1.0f - ((speed - 4.0f) / 10.0f), 0.0f, 1.0f);
            enforce += speedBonus;
            enforce = MR::clamp(enforce, 0.0f, 1.0f);
        }

        doEnforceJump(enforce);
    }

    if (!MR::isNearZero(stickTotal)) {
        addVelocity(-stickTotal);

        MR::vecKillElement(stickTotal, mActor->_240, &stickTotal);
        cutGravityElementFromJumpVec(true);

        if (stickTotal.dot(mJumpVec) > 0.0f) {
            mJumpVec += stickTotal * 1.0f;
        } else {
            mJumpVec += stickTotal * 0.5f;
        }
    }

    if (useStickJumpAnim) {
        mJumpVec += _16C;
        cutGravityElementFromJumpVec(true);
    }

    _340 = 1.0f - mWalkSpeed;
    if (_340 < 0.0f) {
        _340 = 0.0f;
    }

    f32 jumpRatio = 1.0f;
    if (getFloorCode() == 0x20) {
        jumpRatio = mActor->getConst().getTable()->mMudFloorJumpWeakRatio;
    }

    if (_1C._B) {
        jumpRatio = 0.9f;
    }

    mJumpVec += -mActor->_240 * mActor->getConst().getTable()->mJumpHeight[_430] * jumpRatio;

    mMovementStates._E = false;
    mMovementStates._1 = false;
    if (getPlayerMode() != 4 && getFloorCode() == 0x20) {
        _430 = 3;
        isMudFloorJump = true;
        mMovementStates._2B = true;
    }

    if (_10._1A) {
        _430 = 0;
    }

    procJump(true);
    mMovementStates.jumping = true;

    if (_42A == 1 && _430 == 1) {
        _430 = 2;
    }

    switch (_430) {
    case 0:
        if ((mMovementStates._17) != 0) {
            changeAnimation("壁上昇", "落下");
        } else {
            changeAnimation("ジャンプ", "落下");
        }

        if (mActor->mBeeWallWalk != 0) {
            playSound("ハチ壁ジャンプ");
        } else {
            playSound("小ジャンプ");
        }

        if (getPlayerMode() != 6) {
            playSound("声小ジャンプ");
            playSound("ジャンプ踏切");
        }

        if (useStickJumpAnim) {
            changeAnimation("尻滑りジャンプ");
            _428 = 0x1E;
        }

        if (getPlayerMode() == 4) {
            changeAnimation("ハチジャンプ");
            mActor->syncJumpBeeStickMode();
        }

        if (_10._1A) {
            changeAnimation("ショートジャンプ", "落下");
        }

        break;
    case 1:
        changeAnimationNonStop("ジャンプB");
        changeAnimation(nullptr, "落下");
        playSound("中ジャンプ");
        playSound("声中ジャンプ");
        playSound("ジャンプ踏切");
        break;
    case 2:
        playSound("大ジャンプ");
        playSound("声大ジャンプ");
        playSound("ジャンプ踏切");
        playEffect("共通ハイジャンプ");

        if (_1C._B) {
            changeAnimation("スカイラブジャンプ", "落下");
        } else if (getPlayerMode() == 1 && mWalkSpeed > 1.0f) {
            changeAnimation("ダッシュジャンプ", "落下");
        } else {
            changeAnimation("ジャンプC", "落下");
        }

        break;
    case 3:
        if (isMudFloorJump) {
            changeAnimation("埋まり脱出ジャンプ", "落下");
        } else {
            changeAnimation("腹ばいジャンプ", "落下");
            playSound("声小ジャンプ");
            playSound("ジャンプ踏切");
        }

        break;
    default:
        break;
    }

    if (_42A == 1 && _430 == 2) {
        _430 = 1;
    }

    const f32 waterDist = mSwim->checkUnderWaterFull(mFrontVec);
    if (waterDist > 500.0f && mStickPos.z == 0.0f) {
        mMovementStates._E = true;
        changeAnimationNonStop("飛び込みジャンプ");
        playSound("声高飛び込み");
        playSound("ジャンプ踏切");

        if (mStickPos.z < 0.1f) {
            mJumpVec += mFrontVec * 5.0f;
        }
    }

    mMovementStates._5 = true;
    return;
}


void Mario::tryTurnJump() {
    _430 = 0x4;

    if (_3D2 != 0) {
        TVec3f horizontal;
        MR::vecKillElement(_3E4, getAirGravityVec(), &horizontal);
        MR::normalizeOrZero(&horizontal);

        const f32 turnJumpFrontSpeed = mActor->getConst().getTable()->mTurnJumpFrontSpeed;
        mJumpVec = -horizontal * turnJumpFrontSpeed;

        setFrontVecKeepUp(-_3E4);
        _220 = _3E4;
    } else {
        TVec3f horizontal;
        MR::vecKillElement(_220, getAirGravityVec(), &horizontal);
        MR::normalizeOrZero(&horizontal);

        const f32 turnJumpFrontSpeed = mActor->getConst().getTable()->mTurnJumpFrontSpeed;
        mJumpVec = -horizontal * turnJumpFrontSpeed;
    }

    const f32 jumpHeight = mActor->getConst().getTable()->mJumpHeight[_430];
    mJumpVec += -mActor->_240 * jumpHeight;

    _10._17 = false;
    if (mDrawStates._B) {
        recordJumpEnforceMove();
        doEnforceJump(1.0f);
    }

    mMovementStates._E = true;
    procJump(true);
    changeAnimation("ターンジャンプ", "落下");
    playSound("後ジャンプ");
    playSound("声後ジャンプ");
    playSound("ジャンプ踏切");
    playEffect("共通ハイジャンプ");

    _3D0 = 0;
    mMovementStates.jumping = true;
    mMovementStates._1 = false;
    mMovementStates._4 = false;
    mMovementStates._5 = false;
    mWalkSpeed = 0.0f;
    _10._A = false;
    return;
}


void Mario::trySquatJump() {
    mMovementStates._21 = false;
    _430 = 0x5;

    mJumpVec = getAirFrontVec() * mActor->getConst().getTable()->mSquatJumpFrontSpeed;

    TVec3f stickVec(_190 + _19C);
    if (!MR::isNearZero(stickVec)) {
        addVelocity(-stickVec);

        MR::vecKillElement(stickVec, mActor->_240, &stickVec);
        cutGravityElementFromJumpVec(true);

        mJumpVec += stickVec * 1.0f;
    }

    const f32 jumpHeight = mActor->getConst().getTable()->mJumpHeight[_430];
    mJumpVec += -mActor->_240 * jumpHeight;

    _10._17 = false;
    if (mDrawStates._B) {
        recordJumpEnforceMove();

        TVec3f moveKeep;
        f32 gravityMove = MR::vecKillElement(_184, getAirGravityVec(), &moveKeep);

        f32 enforceScale = 1.0f - (MR::diffAngleAbs(mFrontVec, moveKeep) / 1.5707964f);
        if (enforceScale < 0.0f) {
            enforceScale = 0.0f;
        }

        const f32 jumpMag = mJumpVec.length();
        const f32 jumpScale = MR::clamp(1.0f - ((jumpMag - 4.0f) / 10.0f), 0.0f, 1.0f);
        enforceScale += jumpScale;
        enforceScale = MR::clamp(enforceScale, 0.0f, 1.0f);

        if (mJumpVec.dot(moveKeep) < 0.0f) {
            mDrawStates._1B = true;
            initActiveJumpVec();
        }

        mVelocity -= moveKeep;

        mJumpVec += moveKeep * enforceScale;

        if (gravityMove < 0.0f) {
            mJumpVec += getAirGravityVec() * gravityMove;
        }

        invalidateRelativePosition();
    }

    mMovementStates._E = true;
    mMovementStates._1 = false;
    procJump(true);
    changeAnimationNonStop("幅とび");
    changeAnimation(nullptr, "落下");
    playSound("幅ジャンプ");
    playSound("声幅ジャンプ");
    playSound("ジャンプ踏切");

    mMovementStates.jumping = true;
    _3D0 = 0;
    mMovementStates._5 = false;
    mMovementStates._4 = false;
    return;
}


void Mario::tryBackJump() {
    mMovementStates._21 = true;
    mMovementStates._A = false;
    _430 = 0x6;

    const f32 backSpeed = mActor->getConst().getTable()->mSquatJumpBackSpeed;
    mJumpVec = -getAirFrontVec() * backSpeed;

    const f32 jumpHeight = mActor->getConst().getTable()->mJumpHeight[_430];
    mJumpVec += -mActor->_240 * jumpHeight;

    _10._17 = false;
    if (mDrawStates._B) {
        recordJumpEnforceMove();
        doEnforceJump(1.0f);
    }

    mMovementStates._E = true;
    procJump(true);
    changeAnimation("しゃがみジャンプ", "落下");
    playSound("後ジャンプ");
    playSound("声後ジャンプ");
    playSound("ジャンプ踏切");
    playEffect("共通ハイジャンプ");

    if (mSwim->checkUnderWaterFull(-mFrontVec) > 500.0f && mStickPos.z == 0.0f) {
        changeAnimationNonStop("後方飛び込みジャンプ");
        playSound("声高飛び込み");
        playSound("ジャンプ踏切");
    }

    mWalkSpeed = 0.0f;
    mMovementStates.jumping = true;
    _3D0 = 0;
    mMovementStates._1 = false;
    mMovementStates._4 = false;
    mMovementStates._5 = false;
    return;
}


void Mario::tryTornadoJump() {
    _430 = 0x8;
    _42A = 0;

    const f32 frontSpeed = mWalkSpeed;
    const f32 tornadoSpeed = mActor->getConst().getTable()->mJumpTornadoSpeed;

    mJumpVec = mFrontVec * frontSpeed * tornadoSpeed;

    const f32 jumpHeight = mActor->getConst().getTable()->mJumpHeight[_430];
    mJumpVec += -mActor->_240 * jumpHeight;

    mMovementStates._E = false;
    mMovementStates._F = true;
    procJump(true);
    changeAnimation("ジャンプ", "落下");
    playSound("声スピン");
    playSound("トルネードジャンプ");

    mMovementStates.jumping = true;
    _3D0 = 0;
    mMovementStates._4 = false;
    mMovementStates._5 = false;
    mMovementStates._1D = false;
    mWalkSpeed = 0.0f;

    _402 = mActor->getConst().getTable()->mAirWalkTimeTornado;
    _424 = mActor->getConst().getTable()->mTornadoZeroGravityTimer;

    const f32 boosterPower = mActor->getConst().getTable()->mTornadoBoostPower;
    setRocketBooster(-(*getGravityVec()) * boosterPower, mActor->getConst().getTable()->mTornadoBoostAttn,
                     mActor->getConst().getTable()->mTornadoBoostTimer);
}


void Mario::startTornadoCentering(HitSensor* pSensor) {
    pushTask(reinterpret_cast< Task >(&Mario::taskOnTornadoCentering), 0x400);
    _A38 = reinterpret_cast< u32 >(pSensor);
    _A34 = 0x1E;
}


bool Mario::taskOnTornadoCentering(u32 a1) {
    if (_A34 != 0) {
        TVec3f sensorOffset(reinterpret_cast< HitSensor* >(_A38)->mHost->mPosition - mPosition);
        MR::vecKillElement(sensorOffset, *getGravityVec(), &sensorOffset);

        f32 mag = sensorOffset.length();
        if (mag < 10.0f) {
            mag = 10.0f;
        }

        f32 ratio = 10.0f / mag;
        if (ratio < 0.1f) {
            ratio = 0.1f;
        }

        addTrans(sensorOffset * ratio, nullptr);

        _A34--;

        if (_A34 == 0) {
            return false;
        }

        return true;
    }

    return false;
}


void Mario::trySpinJump(u8 a1) {
    if (mMovementStates._B) {
        return;
    }

    if (_430 == 8) {
        return;
    }

    if (isStatusActive(0x17)) {
        return;
    }

    if (getPlayerMode() == 4) {
        return;
    }

    if (getPlayerMode() == 6) {
        return;
    }

    if (getPlayerMode() == 7 && mVerticalSpeed > 100.0f) {
        tryStartFoo();
        return;
    }

    if (isAnimationRun("水泳スピン移動")) {
        return;
    }

    if (isAnimationRun("水泳スピンジャンプ")) {
        return;
    }

    _430 = 0x8;
    _42A = a1 + 1;

    const f32 frontSpeed = mWalkSpeed;
    const f32 tornadoSpeed = mActor->getConst().getTable()->mJumpTornadoSpeed;
    mJumpVec = mFrontVec * frontSpeed * tornadoSpeed;

    const f32 spinJumpHeight = mActor->getConst().getTable()->mSpinJumpHeight;
    mJumpVec += -mActor->_240 * spinJumpHeight;

    mMovementStates._E = false;
    _402 = mActor->getConst().getTable()->mAirWalkTimeSpin;
    procJump(true);

    if (!mMovementStates._B) {
        stopAnimationUpper(nullptr);
        changeAnimation("空中ひねり", "落下");
        playSound("声スピン");
        playSound("スピンジャンプ");
        startPadVib(2);

        mMovementStates.jumping = true;
        mMovementStates._4 = false;
        mMovementStates._5 = false;
        mMovementStates._1D = false;
        _10._19 = true;
        _3D0 = 0;
        mWalkSpeed = 0.0f;
        _10._1 = true;

        if (isStatusActive(MarioStatus_Rabbit)) {
            mRabbit->hop();
        }
    }
}


void Mario::tryForceJumpDelay(const TVec3f& rVec) {
    mMovementStates._2D = true;
    _304 = rVec;
}


void Mario::tryFreeJumpDelay(const TVec3f& rVec) {
    mMovementStates._2E = true;
    _304 = rVec;
}


void Mario::tryForceJump(const TVec3f& rVec, bool a2) {
    if (rVec.dot(*getGravityVec()) > 0.0f) {
        mMovementStates._20 = true;
    }

    mMovementStates.jumping = true;
    mMovementStates._1 = false;
    mMovementStates._5 = false;
    mMovementStates._E = a2;
    mMovementStates._B = false;
    mJumpVec = rVec;

    if (getPlayerMode() == 4 || getPlayerMode() == 6) {
        _16C = mJumpVec;
    }

    resetTornado();

    _3BC = 0;
    _3CA = 0;
    _3CC = 0;
    mMovementStates._13 = true;
    mMovementStates._11 = false;
    _10._8 = false;
    _3BE = 0;
    _344 = mSideVec;
    _4B0 = mPosition;
    _42C = 0;
    _76C = 0;
    procJump(false);
    changeAnimation(nullptr, "落下");
    mRabbit->forceJump();
    return;
}


void Mario::tryForceFreeJump(const TVec3f& rVec) {
    mMovementStates._6 = false;
    _430 = 0xC;
    mMovementStates._2A = true;
    mMovementStates._9 = false;

    tryForceJump(rVec, true);
    _3BC = 0;
    mMovementStates._11 = false;
    _402 = mActor->getConst().getTable()->mAirWalkTime;

    if (getPlayerMode() == 4) {
        playSound("ハチ体力完全回復");
    }

    _76C = 0;
    return;
}


void Mario::tryForcePowerJump(const TVec3f& rVec, bool a2) {
    mMovementStates._6 = false;
    _430 = 0xD;
    mMovementStates._2A = true;
    mMovementStates._9 = false;
    const bool _2FPrev = mMovementStates._2F;
    mMovementStates._2F = true;
    tryForceJump(rVec, true);
    mMovementStates._2F = _2FPrev;
    _3BC = 0;
    mMovementStates._11 = false;

    if (!a2) {
        _402 = mActor->getConst().getTable()->mAirWalkTime;

        if (getPlayerMode() == 4) {
            playSound("ハチ体力完全回復");
        }
    }

    _76C = 0;
    return;
}


void Mario::tryFreeJump(const TVec3f& rVec, bool a2) {
    _430 = 0;

    if (rVec.dot(*getGravityVec()) > 0.0f) {
        mMovementStates._20 = true;
    }

    mMovementStates.jumping = true;
    mMovementStates._1 = false;
    mMovementStates._5 = false;
    mMovementStates._E = a2;
    mMovementStates._B = false;
    mJumpVec = rVec;
    resetTornado();

    _3BC = 0;
    _3CA = 0;
    _3CC = 0;
    mMovementStates._13 = true;
    mMovementStates._11 = false;
    _3BE = 0;
    _76C = 0;
    _344 = mSideVec;
    procJump(false);
    changeAnimation(nullptr, "落下");
    startPadVib(2);
    mRabbit->forceJump();
    return;
}


void Mario::tryWallJump(const TVec3f& rVec, bool a2) {
    _430 = 0x7;

    mMovementStates.jumping = true;
    mMovementStates._5 = false;
    mMovementStates._E = true;
    mMovementStates._B = false;
    mMovementStates._6 = false;
    mMovementStates._28 = false;
    _20._28 = false;
    mMovementStates._29 = false;

    if (a2) {
        mMovementStates._9 = true;
    } else {
        mMovementStates._9 = false;
    }

    mJumpVec = rVec;
    _4B0 = mPosition;
    _3BC = 0;
    mMovementStates._13 = true;
    mMovementStates._11 = false;
    _3CA = 0;
    _3CC = 0;
    _3BE = 0;
    mMovementStates._1D = true;

    if (getPlayerMode() == 5) {
        changeAnimationNonStop("ホッパー壁ジャンプ");
        playSound("ホッパージャンプ");
    } else {
        changeAnimation("壁ジャンプ", "落下");
        playSound("ジャンプ踏切");
    }

    playSound("声小ジャンプ");

    if (a2) {
        setFrontVecKeepUp(rVec);
    }

    stopWalk();
    mRabbit->forceJump();
    return;
}


void Mario::tryStickJump(const TVec3f& rVec) {
    stopJump();
    initJumpParam();

    mMovementStates.jumping = true;
    mMovementStates._5 = false;
    mMovementStates._E = true;
    mMovementStates._B = false;
    _430 = 0;
    _20._28 = false;
    mMovementStates._28 = false;
    mMovementStates._29 = false;
    mMovementStates._6 = false;
    mMovementStates._9 = true;
    mJumpVec = rVec;
    _3CA = 0;
    _3CC = 0;
    _3BE = 0;
    mMovementStates._13 = true;
    mMovementStates._11 = false;
    mMovementStates._1D = true;
    _76C = mActor->getConst().getTable()->mBeeGravityReviveTime;
    _770 = 0.0f;
    _3BC = 0xA;
    playSound("ハチ壁ジャンプ");
    playSound("声小ジャンプ");
    setFrontVecKeepUp(rVec);
    changeAnimationNonStop("ハチ壁ジャンプ");
    return;
}


void Mario::trySlipUpJump() {
    if (mMovementStates._1) {
        mMovementStates._6 = false;
        mMovementStates._28 = false;
        return;
    }

    changeAnimation("スリップアップ");
    playSound("スリップアップ");
    playEffect("スリップアップ");
    startPadVib("マリオ[スリップアップ]");

    mJumpVec = -getAirGravityVec() * mActor->getConst().getTable()->mSlipUpHeight;

    f32 ratio = 1.0f;
    TVec3f predictPos(mPosition - *getGravityVec() * 100.0f);

    TVec3f frontVec;
    MR::vecKillElement(mJumpVec, mFrontVec, &frontVec);
    MR::vecKillElement(frontVec, *getGravityVec(), &frontVec);
    frontVec += mFrontVec * mActor->getConst().getTable()->mSlipUpFront;
    MR::normalizeOrZero(&frontVec);

    f32 step = 50.0f;
    u32 hitCount;
    TVec3f hitPos;
    for (hitCount = 0; hitCount < 10; hitCount++) {
        Triangle* pTmpPolygon = getTmpPolygon();
        predictPos += frontVec * step;

        if (!MR::getFirstPolyOnLineBFast(predictPos, *getGravityVec() * 150.0f, &hitPos, pTmpPolygon)) {
            break;
        }
    }

    ratio *= static_cast< f32 >(hitCount) / 10.0f;
    cutVecElementFromJumpVec(mFrontVec);

    if (mMovementStates._9) {
        mJumpVec += mFrontVec * mActor->getConst().getTable()->mSlipUpFront * ratio;
        mMovementStates._29 = true;
    } else {
        mJumpVec += mFrontVec * mActor->getConst().getTable()->mSlipUpFrontWeak * ratio;
    }

    mMovementStates._6 = false;
    mMovementStates._E = true;
    initJumpParam();
    _430 = 0xA;
    return;
}


void Mario::tryHangSlipUp() {
    mMovementStates._5 = false;
    getPlayer()->tryJump();
    changeAnimation("つかまりスリップアップ");
    playSound("スリップアップ");
    playEffect("スリップアップ");
    startPadVib("マリオ[スリップアップ]");

    const f32 slipUpHeight = mActor->getConst().getTable()->mSlipUpHeightHang;
    mJumpVec = -getAirGravityVec() * slipUpHeight;
    cutVecElementFromJumpVec(mFrontVec);

    const f32 slipUpFront = mActor->getConst().getTable()->mSlipUpFrontHang;
    mJumpVec += mFrontVec * slipUpFront;

    mMovementStates._6 = false;
    mMovementStates._E = true;
    _430 = 0xA;
    mActor->setBlendMtxTimer(0x8);
    mTargetWalkSpeedIndex = 0x7;
    mWalkSpeed = 0.6f;
    return;
}


void Mario::tryDrop() {
    if ((mMovementStates._8) && !mDrawStates._B) {
        TVec3f horizontal;
        MR::vecKillElement(_4E8 - mGroundPos, *getGravityVec(), &horizontal);

        if (horizontal.dot(*mFrontWallTriangle->getNormal(0)) < 0.0f && horizontal.length() < 80.0f) {
            getPlayer()->mMovementStates._1 = true;
            push(*mFrontWallTriangle->getNormal(0) * (80.0f - horizontal.length()) * 0.1f);
            return;
        }
    }

    if ((mMovementStates._1A) && !mDrawStates._B) {
        TVec3f horizontal;
        MR::vecKillElement(_500 - mGroundPos, *getGravityVec(), &horizontal);

        if (horizontal.dot(*mSideWallTriangle->getNormal(0)) < 0.0f && horizontal.length() < 80.0f) {
            push(*mSideWallTriangle->getNormal(0) * (80.0f - horizontal.length()) * 0.1f);
            getPlayer()->mMovementStates._1 = true;
            return;
        }
    }

    mMovementStates._21 = true;
    mMovementStates._B = false;

    mJumpVec = mFrontVec * mWalkSpeed * ::cDropFrontSpeed;

    if (MR::isNearZero(_8F8) && (mMovementStates._23)) {
        TVec3f side;
        side.cross(getAirGravityVec(), _368);
        MR::normalizeOrZero(&side);
        _8F8.cross(_368, side);
        MR::normalizeOrZero(&_8F8);
    }

    if (!MR::isNearZero(_8F8)) {
        mJumpVec = _8F8;
        cutGravityElementFromJumpVec(false);

        if (_16C.length() > 20.0f) {
            mJumpVec += -mActor->getAirGravityVec() * 10.0f;
        } else {
            f32 gravScale = mActor->_288.length();
            if (gravScale > 8.0f) {
                gravScale = 8.0f;
            }

            mJumpVec += -mActor->getAirGravityVec() * gravScale;
            _8F8.zero();
        }
    }

    TVec3f carryVec(_184);
    MR::vecKillElement(carryVec, *getGravityVec(), &carryVec);
    invalidateRelativePosition();

    TVec3f groundDeltaNoGravity;
    MR::vecKillElement(mGroundPos - mPosition, *getGravityVec(), &groundDeltaNoGravity);

    if (carryVec.dot(groundDeltaNoGravity) > 0.0f) {
        carryVec = carryVec * 0.1f;
    }

    mJumpVec += carryVec;

    _340 = 1.0f - mWalkSpeed;
    if (_340 < 0.0f) {
        _340 = 0.0f;
    }

    _430 = 0;
    procJump(true);

    mMovementStates.jumping = true;
    mMovementStates._5 = false;
    mMovementStates._E = true;
    _10._23 = false;
    mMovementStates._B = false;

    if (isAnimationRun("坂すべり上向きうつぶせ", 2) || isAnimationRun("坂すべり下向きあおむけ", 3)) {
        _428 = 0xF;
    }

    changeAnimation(nullptr, "落下");

    if (getPlayerMode() == 4) {
        _408 = mActor->getConst().getTable()->mBeeGravityPowerTimeD;
        _3BC = mActor->getConst().getTable()->mBeeAirWalkInhibitTimeD - 5;
    }

    if (isAnimationRun("ショート着地")) {
        mMovementStates._23 = true;
    }

    if (mMovementStates._8) {
        stopWalk();
    }

    return;
}


bool Mario::isDigitalJump() const NO_INLINE {
    return mMovementStates._E;
}


void Mario::initActiveJumpVec() {
    _2EC = getAirGravityVec();
    MR::vecKillElement(mJumpVec, _2EC, &_2E0);

    if (MR::normalizeOrZero(&_2E0)) {
        _2E0 = mFrontVec;
    }
}


void Mario::initJumpParam() {
    const u32 flags1 = _1C_WORD;
    mMovementStates._13 = true;

    if ((_1C._A) || (_1C._B)) {
        _10._1F = true;
    }

    mMovementStates._38 = false;
    _10._8 = false;
    mMovementStates._2F = false;
    _10._E = false;

    if (!mDrawStates._1B) {
        initActiveJumpVec();
    }

    _42C = 0;
    _3BC = 0;
    _3CA = 0;
    _3CC = 0;
    _774 = 0;
    mMovementStates._11 = false;
    _3BE = 0;
    _344 = mSideVec;

    _408 = 0;
    _76C = 0;
    mMovementStates._C = false;
    mMovementStates._10 = false;
    mMovementStates._4 = false;
    mMovementStates._20 = false;
    _10._C = false;
    _3D2 = 0;
    _3D0 = 0;
    _3CE = 0;
    mMovementStates._2A = mMovementStates._8;

    if (_430 != 5) {
        cancelSquatMode();
    }

    _4B0 = mPosition;
    _426 = 0;

    if (mSinkTimer != 0) {
        mJumpVec.scale(0.2f + (0.8f * ((0x100 - mSinkTimer) / 256.0f)));
        mSinkTimer = 0;
    }
}


bool Mario::isEnableFutureJump() const {
    if (_430 == 0x3) {
        return false;
    }

    if (!isRising()) {
        const f32 dot = mJumpVec.dot(*getGravityVec());
        const s16 limit = mActor->getConst().getTable()->mFutureJumpReqLimitTime;
        if (mVerticalSpeed < dot * static_cast< f32 >(limit)) {
            return true;
        }
    }

    return false;
}


void Mario::procJump(bool a1) {
    f32 jumpGravity;
    f32 gravityScale = 1.0f;
    if (!a1 && !_10._1F && _430 == 0 && _3BC < 6 && mActor->isRequestJump2P()) {
        _1C_WORD |= 0x00100000;
        mMovementStates.jumping = false;
        _3BC = 6;
        _10._1F = true;
        tryJump();
        _3BC = 6;
    }

    if (mMovementStates._B) {
        procHipDrop();
        return;
    }

    if (_424 != 0) {
        _424--;
        return;
    }

    procRocketBooster();

    if (_10._13) {
        fixFrontVecByGravity();
    }

    if (_10._1F) {
        if (!mActor->isKeepJump2P()) {
            mMovementStates._13 = false;
        }
    } else if (!mActor->isKeepJump()) {
        mMovementStates._13 = false;
    }

    if (checkTrgA()) {
        _558 = mActor->_37C;
        if (isEnableFutureJump()) {
            mMovementStates._38 = true;
        }
    }

    if (mActor->isRequestJump2P()) {
        _558 = mActor->_37C;
        if (isEnableFutureJump()) {
            mMovementStates._38 = true;
        }
    }

    bool useConnectGravity = false;
    if (mMovementStates._13 && !isDigitalJump() && _430 == 9 && isRising()) {
        useConnectGravity = true;
    }

    if (a1 || mMovementStates._13 || isDigitalJump() || !isRising() || _430 == 2) {
        useConnectGravity = true;
    }

    if (_430 == 8 && _42A == 0) {
        s32 timer = _426;
        if (timer == 0) {
            useConnectGravity = true;
        } else {
            useConnectGravity = timer > (mActor->getConst().getTable()->mTornadoBoostTimer - 0x10);
        }
    }

    if (useConnectGravity) {
        MarioConstTable* pTable = mActor->getConst().getTable();
        gravityScale = pTable->mGravityRatioA;
        f32 connectRatio = _3BC <= 45 ? 0.0f : 1.0f;
        if (_3BC <= 45) {
            connectRatio = static_cast< f32 >(_3BC) / 45.0f;
        }
    }

    if (a1) {
        initJumpParam();
    } else if (mActor->isRequestHipDrop() && jumpToHipDrop()) {
        return;
    }

    if (!isRising()) {
        if (isAnimationRun("ジャンプB")) {
            stopAnimation(nullptr);
        }

        if (!mSwim->_1B2) {
            if (isAnimationRun("水泳スピン移動")) {
                stopAnimation(nullptr);
            }

            if (isAnimationRun("水泳スピンジャンプ")) {
                stopAnimation(nullptr);
            }

            if (isAnimationRun("水泳ジェット")) {
                changeAnimation("空中一回転");
            }
        }

        _3BE++;
    } else {
        f32 ceilDist = calcDistToCeil(false);
        if (ceilDist < 160.0f) {
            jumpGravity = cutGravityElementFromJumpVec(true);

            playEffectTrans("天井ヒット", mPosition - getAirGravityVec() * ceilDist);

            f32 reduce;
            switch (_430) {
            case 4:
            case 6:
                reduce = 0.9f;
                break;
            case 5:
            default:
                reduce = 0.6f;
                break;
            }

            mJumpVec += getAirGravityVec() * jumpGravity * reduce;
        }
    }

    bool wasAirWalk = false;
    if (_3CC != 0) {
        wasAirWalk = true;
        _3CC--;
    }

    if (_3BC > 10 && checkTrgA() && (mMovementStates._17)) {
        mMovementStates._17 = false;
        if (mWall->startJump()) {
            return;
        }
    }

    mMovementStates._12 = false;
    if (_430 == 8) {
        if (checkTrgZ()) {
            resetTornado();
            cancelTornadoJump();
            stopAnimation(nullptr);
            changeAnimationInterpoleFrame(1);
            mDrawStates._8 = true;
            playSound("声小ジャンプ");

            if (_42A != 0) {
                playEffect("スピンライト消去");
            }
        } else {
            mMovementStates._11 = false;
        }

        if (_42A != 0) {
            if (_402 != 0) {
                _402--;
            }

            if (_402 == 0) {
                cancelTornadoJump();
                playEffect("スピンライト消去");

                if (getPlayerMode() != 4) {
                    _402 = mActor->getConst().getTable()->mAirWalkTime;
                }
            }
        }
    }

    if (beeMarioOnAir()) {
        _76C = mActor->getConst().getTable()->mBeeGravityReviveTime;
    }

    if (_406 != 0) {
        _406--;
    }

    f32 jumpAcceleration;
    if (_430 == 0xC) {
        jumpAcceleration = 1.8f;
    } else if (_430 == 0xD) {
        jumpAcceleration = mActor->getConst().getTable()->mGravityJumping[9];
    } else {
        jumpAcceleration = mActor->getConst().getTable()->mGravityJumping[_430];
    }

    if (_430 == 0xB && (isRising() || _3BE < mActor->getConst().getTable()->mTrampleBegomaOpenTime)) {
        jumpAcceleration = mActor->getConst().getTable()->mGravityJumping[0];
    }

    if (_430 == 8 && _42A == 1) {
        jumpAcceleration = mActor->getConst().getTable()->mSpinJumpGravity;
    }

    if (_430 == 8 && !isRising()) {
        jumpAcceleration *= 0.25f;
    }

    const s32 clingNum = MR::getKarikariClingNum();
    f32 clingScale = 1.0f;
    if (clingNum != 0) {
        clingScale = (2.0f + static_cast< f32 >(clingNum)) / 2.0f;
        if (getPlayerMode() == 4) {
            clingScale = 1.0f;
        }

        jumpAcceleration *= clingScale;
    }

    if (wasAirWalk) {
        jumpAcceleration = mActor->getConst().getTable()->mGravityAirWalk;
    }

    if (_76C != 0) {
        const u16 maxTimer = mActor->mConst->getTable()->mBeeGravityReviveTime;
        const f32 t = static_cast< f32 >(_76C) / static_cast< f32 >(maxTimer);
        jumpAcceleration = ((1.0f - t) + (t * mActor->getConst().getTable()->mGravityAirWalk));
        _76C--;
        jumpAcceleration *= clingScale;
    }

    if (_430 == 0xD && (isDigitalJump() || !isRising())) {
        gravityScale = 1.0f;
    }

    gravityScale *= mActor->getGravityRatio();
    if (mActor->_334 != 0) {
        gravityScale *= 0.3f;
        if (!mActor->isInZeroGravitySpot() && mVerticalSpeed < 300.0f) {
            gravityScale = 0.0f;
            if (mVerticalSpeed < 200.0f) {
                cutGravityElementFromJumpVec(true);
                gravityScale = -0.1f;
            }
        }
    }

    if (getPlayerMode() == 6) {
        gravityScale = 0.0f;
    }

    if (mMovementStates._17) {
        f32 wallRatio = static_cast< f32 >(_3BC) / static_cast< f32 >(mActor->mConst->getTable()->mSlipUpSpdCtrlTimer);
        if (wallRatio > 1.0f) {
            wallRatio = 1.0f;
        }

        const f32 wallScale =
            mActor->getConst().getTable()->mSlipUpSpdRatio + ((1.0f - wallRatio) * (1.0f - mActor->getConst().getTable()->mSlipUpSpdRatio));
        addVelocity(mJumpVec, wallScale);

        mJumpVec += mActor->_240 * jumpAcceleration * gravityScale * wallScale;
        moveWallSlide(1.0f);
    } else if ((mMovementStates._1) == 0) {
        addVelocity(mJumpVec);

        mJumpVec += mActor->_240 * jumpAcceleration * gravityScale;
    }

    checkWallRising();
    checkWallJumpHit();

    if ((mMovementStates._1) == 0) {
        fixFrontVecFromUpSide();
    }

    decideSlipUp();

    if (_430 != 0xD || !mMovementStates._20) {
        TVec3f side;
        const f32 dropSpeed = MR::vecKillElement(mJumpVec, *getGravityVec(), &side);

        f32 maxDrop = mActor->getConst().getTable()->mMaxDropSpeed[mActor->getGravityLevel() & 0xFF];
        if (_430 == 0xB && !isRising() && _3BE >= mActor->mConst->getTable()->mTrampleBegomaOpenTime) {
            maxDrop = mActor->getConst().getTable()->mMaxDropSpeed[3];
        }

        if (getPlayerMode() == 4 && !isRising()) {
            maxDrop = mActor->getConst().getTable()->mMaxDropSpeed[4];
        }

        if (dropSpeed >= maxDrop) {
            mJumpVec = *getGravityVec() * maxDrop + side;
        }
    }

    if ((mMovementStates._1) != 0 && !a1) {
        doLanding();

        if (calcDistToCeil(false) < 80.0f) {
            bool pressedA = false;
            const HitSensor* sensorA = _730;
            if (sensorA != nullptr && MR::isSensorPressObj(sensorA)) {
                pressedA = true;
            }

            bool pressedB = false;
            const HitSensor* sensorB = mGroundPolygon->mSensor;
            if (sensorB != nullptr && MR::isSensorPressObj(sensorB)) {
                pressedB = true;
            }

            if (_730 != mGroundPolygon->mSensor && (pressedA || pressedB)) {
                mActor->setPress(0, 0);
                mActor->_3B4 = *mGroundPolygon->getNormal(0);
            }
        }
    } else {
        doAirWalk();
    }

    _3BC++;
    return;
}


void Mario::checkWallRising() {
}


void Mario::checkWallJumpHit() {
    if (_430 == 0x5) {
        if ((mMovementStates._8) && !checkWallJumpCode()) {
            const f32 wallDot = cutVecElementFromJumpVec(getWallNorm());
            if (checkWallCode("NoAction", true)) {
                return;
            }

            mJumpVec -= getWallNorm() * wallDot;

            blown(mJumpVec * 0.2f);

            TVec3f horizontal;
            MR::vecKillElement(mJumpVec, *getGravityVec(), &horizontal);
            MR::normalizeOrZero(&horizontal);
            if (!MR::isNearZero(horizontal)) {
                setFrontVecKeepUp(-horizontal);
            }

            mMovementStates._2B = true;
            _402 = 0;
            _428 = 0x3C;
        }

        return;
    }

    if (!isRising() && _3BC > 10 && (mMovementStates._8) && calcPolygonAngleD(mFrontWallTriangle) < 80.0f) {
        if ((_10._E) == 0) {
            mJumpVec = *mFrontWallTriangle->getNormal(0) * 5.0f + getAirGravityVec() * cutGravityElementFromJumpVec(true);
            _10._E = true;
            changeAnimation("壁はじき");
        } else {
            const f32 jumpMag = mJumpVec.length();
            cutVecElementFromJumpVec(*mFrontWallTriangle->getNormal(0));
            mJumpVec.setLength(jumpMag);
        }
    }
}


void Mario::decideSlipUp() {
    if (isAnimationRun("壁はじき")) {
        return;
    }

    if (!mMovementStates._9) {
        return;
    }

    if (mMovementStates._F) {
        mMovementStates._6 = false;
        return;
    }

    if (!mMovementStates._6) {
        if (checkWallCode("NotWallSlip", true)) {
            return;
        }

        if (checkWallCode("NoAction", true)) {
            return;
        }

        bool slipFront = false;
        if (isStickOn()) {
            const TVec3f& worldPadDir = getWorldPadDir();
            if (worldPadDir.dot(getWallNorm()) < -0.8f) {
                slipFront = true;
            }
        }

        if (_430 == 0x7) {
            slipFront = true;
        }

        if ((mMovementStates._2A) == 0 && (mMovementStates._8) && isRising() && slipFront) {
            mMovementStates._6 = true;
        }

        if (isRising() && getPlayerMode() != 4 && getPlayerMode() != 6 && getPlayerMode() != 5) {
            if (_4E0 < mActor->getConst().getTable()->mSlipUpContinueHeight) {
                if (mMovementStates._8 && mMovementStates._9 && slipFront && mMovementStates._29) {
                    mMovementStates._6 = true;
                    cutGravityElementFromJumpVec(true);

                    mJumpVec += -getAirGravityVec() * 30.0f;
                }
            }
        }
    } else if (!mMovementStates._7 && isRising() && !mMovementStates._8 && (mMovementStates._15)) {
        trySlipUpJump();
    }
}


void Mario::moveWallSlide(f32 a1) {
    TVec3f crossVec;
    const TVec3f& wallNorm = getWallNorm();
    crossVec.cross(*getGravityVec(), wallNorm);

    if (!MR::normalizeOrZero(&crossVec) && isStickOn()) {
        f32 dot = crossVec.dot(getWorldPadDir());
        const f32 absDot = MR::abs(dot);
        if (absDot > 0.2f) {
            if (dot < -0.2f) {
                dot = (0.2f + dot) / 0.8f;
            } else {
                dot = (dot - 0.2f) / 0.8f;
            }

            addVelocity(crossVec, (10.0f * dot) * a1);
        }
    }
}


bool Mario::jumpToHipDrop() {
    if (mMovementStates._B) {
        return false;
    }

    switch (_430) {
    case 5:
        return false;

    case 9:
    case 11:
    case 12:
    case 13:
        if (getPlayerMode() != 4 && isRising()) {
            return false;
        }
    }

    if (mVerticalSpeed < mActor->getConst().getTable()->mHipDropLimitHeight) {
        return false;
    }

    if (MR::isNearZero(mActor->_240)) {
        return false;
    }

    if (getPlayerMode() == 6) {
        return false;
    }

    bool isNormalDrop = false;
    if (mActor->_3E5) {
        isNormalDrop = true;
    }

    resetTornado();
    cancelTornadoJump();

    mDrawStates._8 = true;
    mMovementStates._21 = false;
    mMovementStates._B = true;
    mJumpVec.zero();
    _422 = 0;

    if (_430 == 4) {
        setFrontVecKeepUp(-_220);
        _430 = 0;
    }

    if (isAnimationRun("カリカリ限界")) {
        stopAnimationUpper(nullptr);
    }

    if (isPlayerModeHopper()) {
        _720 = getAnimationStringPointer("ホッパーヒップドロップ開始");
        _724 = getAnimationStringPointer("ホッパーヒップドロップ");
        _728 = getAnimationStringPointer("ヒップドロップ着地");
        startHipDropBlur();
    } else if (getPlayerMode() == 4) {
        _720 = getAnimationStringPointer("ハチヒップドロップ開始");
        _724 = getAnimationStringPointer("ハチヒップドロップ");

        if (mActor->mBeeWallWalk != 0) {
            _728 = getAnimationStringPointer("ハチヒップドロップ壁着地");
        } else {
            _728 = getAnimationStringPointer("ハチヒップドロップ着地");
        }
    } else if (isNormalDrop) {
        _720 = getAnimationStringPointer("スピンヒップドロップ開始");
        _724 = getAnimationStringPointer("スピンヒップドロップ");
        _728 = getAnimationStringPointer("スピンヒップドロップ着地");
        stopEffect("スピンライト");
        _10._27 = true;
    } else {
        _720 = getAnimationStringPointer("ヒップドロップ開始");
        _724 = getAnimationStringPointer("ヒップドロップ");
        _728 = getAnimationStringPointer("ヒップドロップ着地");
        startHipDropBlur();
    }

    changeAnimation(_720, _724);
    playSound("尻ドロップ回転");

    if (isNormalDrop) {
        playSound("声スピン尻ドロップ");
        playSound("スピン尻ドロップ回転");

    } else {
        playSound("声尻ドロップ");
    }

    _424 = mActor->getConst().getTable()->mHipDropZeroGrTime;
    mRabbit->forceJump();
    mMovementStates._3E = 0;
    return true;
}


void Mario::procHipDrop() {
    f32 gravityHipDrop;
    if (isAnimationRun(_720)) {
        if (!isAnimationTerminate(nullptr)) {
            return;
        }

        if (_424 != 0) {
            _424--;
            return;
        }

        stopAnimation(nullptr);
        return;
    }

    if (_424 != 0) {
        _424--;
        return;
    }

    if (mActor->_38C != 0) {
        return;
    }

    if (MR::isNearZero(mActor->_240)) {
        mMovementStates.digitalJump = true;
    }

    if (mActor->_334 != 0) {
        _2F8.x *= 0.5f;
        _2F8.y *= 0.5f;
        _2F8.z *= 0.5f;
        mJumpVec.x *= 0.5f;
        mJumpVec.y *= 0.5f;
        mJumpVec.z *= 0.5f;
    }

    if ((mMovementStates._1) != 0 || isAnimationRun(_728)) {
        fixFrontVecByGravity();

        if (isAnimationRun(_728)) {
            bool shouldEnd = false;

            if (mActor->_334 != 0) {
                shouldEnd = true;
            } else if ((mMovementStates._1) == 0 && _3BC > 3 && mVerticalSpeed > 10.0f) {
                changeAnimation(_724);
                mJumpVec = _2F8;
                goto PROC_HIP_DROP_MOVE;
            }

            u16 endLimit = 0xF;
            if (isAnimationRun("スピンヒップドロップ着地")) {
                endLimit = 0x2D;
            }

            if (_3CE > endLimit) {
                if (isStickOn()) {
                    shouldEnd = true;
                }

                if (mDrawStates._C) {
                    shouldEnd = true;
                }
            }

            if (_3CE > 5 && checkTrgA()) {
                shouldEnd = true;
            }

            if (isAnimationTerminate(nullptr) || shouldEnd) {
                mMovementStates.jumping = false;
                mMovementStates._B = false;
                stopAnimation(nullptr, "基本");
                stopEffect("属性尻ドロップ");

                if (checkTrgA()) {
                    tryJump();
                    return;
                }
            }

            mWalkSpeed = 0.0f;
        } else {
            _3CE = 0;
            mWalkSpeed = 0.0f;
            _71E = 0;

            if (isCurrentFloorSand()) {
                changeStatus(mBury);
                return;
            }

            if (isCurrentFloorSink()) {
                mMovementStates.jumping = false;
                mMovementStates._B = false;
                stopAnimation(nullptr, "基本");
                mSinkTimer = 200;
                return;
            }

            playSound("尻ドロップ着地");
            playSound("声尻ドロップ着地");
            playEffectRT("属性尻ドロップ", _368, mPosition);
            startPadVib("最強");
            startCamVib(0);
            mDrawStates._14 = true;
            MR::removeAllClingingKarikari();

            if (_960 == 0x1B || _960 == 0x1C || _960 == 9) {
                mMovementStates.jumping = false;
                mMovementStates._B = false;
                stopAnimation(nullptr, "基本");
                return;
            }

            changeAnimationWithAttr(_728, 1);
            _2F8 = mJumpVec;
            clearSlope();

            if (mActor->_B90) {
                mMovementStates.jumping = false;
                mMovementStates._B = false;
            }
        }

        mJumpVec.zero();
    } else {
        _3BC++;

        if (MR::isNearZero(mActor->getLastMove())) {
            _422++;
            if (_422 == 0xF) {
                stopAnimation(nullptr, "基本");
                mMovementStates.jumping = false;
                mMovementStates._B = false;
                mJumpVec.zero();
                mMovementStates._1 = true;
                return;
            }
        }
    }

PROC_HIP_DROP_MOVE:
    TVec3f moveVec(mJumpVec);
    if (moveVec.length() >= mVerticalSpeed) {
        moveVec.setLength(mVerticalSpeed);
    }

    addVelocity(moveVec);

    if (!mMovementStates._1) {
        const f32 speedRate = MR::clamp(0.1f + (1.0f - (mJumpVec.length() / mActor->getConst().getTable()->mLimitSpeedHipDrop)), 0.0f, 1.0f);
        gravityHipDrop = mActor->getConst().getTable()->mGravityHipDrop;

        mJumpVec += *getGravityVec() * gravityHipDrop * speedRate;

        const f32 jumpGravity = cutGravityElementFromJumpVec(true);
        if (mJumpVec.length() > 10.0f) {
            mJumpVec.x *= 0.5f;
            mJumpVec.y *= 0.5f;
            mJumpVec.z *= 0.5f;
        }

        mJumpVec += getAirGravityVec() * jumpGravity;

        if (mJumpVec.length() > mActor->getConst().getTable()->mLimitSpeedHipDrop) {
            mJumpVec.setLength(mActor->getConst().getTable()->mLimitSpeedHipDrop);
        }

        if (_10._27 && isAnimationRun("スピンヒップドロップ")) {
            playSound("スピン尻ドロップ落下");

            Triangle strikeTriangles[0x20];
            HitSensor* homingSensor = nullptr;

            f32 maxAngle = 45.0f;
            f32 speed = mVerticalSpeed;
            if (speed >= 500.0f) {
                speed = 500.0f;
            }

            s32 strikeNum = Collision::checkStrikeBallToMapWithThickness(mShadowPos, speed, speed, nullptr, nullptr);

            for (u32 i = 0; i < strikeNum; i++) {
                const HitInfo* strikeInfo = Collision::getStrikeInfoMap(i);

                if (mActor->selectHomingInSuperHipDrop(strikeInfo->mParentTriangle.mSensor->mHost->mName)) {
                    TVec3f sensorPos(strikeInfo->mParentTriangle.mSensor->mPosition);

                    if (MR::diffAngleAbs(sensorPos - mActor->_2A0, *getGravityVec()) < maxAngle) {
                        homingSensor = strikeInfo->mParentTriangle.mSensor;
                    }
                }
            }

            if (homingSensor == nullptr) {
                homingSensor = reinterpret_cast< HitSensor* >(mActor->_4A8);
            }

            if (homingSensor != nullptr) {
                TVec3f targetJump(homingSensor->mPosition - mActor->_2A0);
                targetJump.setLength(mJumpVec.length());

                TVec3f lastMove;
                mActor->getLastMove(&lastMove);
                if (lastMove.dot(mJumpVec) < 0.0f) {
                    const f32 hitGravity = cutGravityElementFromJumpVec(true);
                    mJumpVec = -mJumpVec + getAirGravityVec() * hitGravity;
                } else if (mJumpVec.dot(targetJump) > 0.0f) {
                    mJumpVec = targetJump;
                }
            }

            if ((mMovementStates._8) || (mMovementStates._1A) || (mMovementStates._19)) {
                _10._27 = false;
            }
        }

        fixFrontVecFromUpSide();
    }
}


void Mario::doAirWalk() {
    if (isAnimationRun("水泳ジェット")) {
        return;
    }

    if (_430 == 3) {
        return;
    }

    if (mMovementStates._2F) {
        return;
    }

    if (isAnimationRun("壁はじき")) {
        return;
    }

    if (mMovementStates._1D) {
        if (_3BC > mActor->getConst().getTable()->mWaitNeutralTimer) {
            mMovementStates._1D = false;
        }

        return;
    }

    if (_428 != 0) {
        _428--;
        if (_428 == 0) {
            stopAnimation("坂すべり上向きうつぶせ", 2);
            stopAnimation("坂すべり下向きあおむけ", 3);
        }

        return;
    }

    f32 speedKiller;
    TVec3f moveDir;
    TVec3f sideMove;
    calcMoveDir(mStickPos.x, mStickPos.y, &moveDir, true);

    if (mActor->_334 != 0) {
        moveDir.zero();
    }

    if (mDrawStates._D) {
        if (MR::diffAngleAbs(getAirGravityVec(), _2EC) > 0.7853982f) {
            TVec3f sideVec;
            sideVec.cross(mFrontVec, getAirGravityVec());
            MR::normalizeOrZero(&sideVec);

            Mtx rotMtx;
            PSMTXRotAxisRad(rotMtx, &sideVec, MR::diffAngleSignedHorizontal(_2EC, getAirGravityVec(), sideVec));

            TVec3f frontRot;
            PSMTXMultVecSR(rotMtx, &_2E0, &frontRot);
            if (frontRot.dot(mFrontVec) < 0.0f) {
                moveDir = -mFrontVec;
            } else {
                moveDir = mFrontVec;
            }
        }
    }

    MR::vecKillElement(moveDir, *getGravityVec(), &moveDir);

    bool inhibitFrontAdjust = false;
    f32 frontDot = MR::vecKillElement(moveDir, mFrontVec, &sideMove);

    if (_430 == 0xB || _430 == 8 || getPlayerMode() == 4 || getPlayerMode() == 6) {
        inhibitFrontAdjust = true;
    } else if (frontDot >= 0.0f) {
        f32 frontReduction = mActor->getConst().getTable()->mJumpFrontReduction;
        s16 reduceBegin = mActor->getConst().getTable()->mJumpFrontReductionBeginTime;
        if (_430 == 5) {
            frontReduction = mActor->getConst().getTable()->mSquatJumpFrontReduction;
            reduceBegin = mActor->getConst().getTable()->mSquatJumpFrontReductionBTime;
        }

        if (_10._17 && _8D8 == _45C->mSensor) {
            reduceBegin = 0xB4;
            if (MR::isSameMtx(_8E8->getBaseMtx()->toMtxPtr(), _8E8->getPrevBaseMtx()->toMtxPtr())) {
                MR::vecKillElement(_8DC, getAirGravityVec(), &_8DC);
                mJumpVec -= _8DC;
                _8DC.zero();
            }
        }

        if ((mMovementStates._11) == 0 && _3BC > reduceBegin) {
            const f32 jumpGravity = cutGravityElementFromJumpVec(true);
            const f32 jumpMag = mJumpVec.length();
            mJumpVec.setLength(frontReduction * jumpMag);

            mJumpVec += getAirGravityVec() * jumpGravity;
        }
    } else {
        if (!isRising() && mVerticalSpeed < mActor->getConst().getTable()->mLandTurnHeight && (mMovementStates._11) == 0) {
            mMovementStates._10 = true;
        } else {
            _3CA++;
        }

        if (isAnimationRun("ターンジャンプ")) {
            _3CA = 0;
            speedKiller = 0.1f;
        } else if (_3CA < mActor->mConst->getTable()->mBackJumpLimitFrame) {
            const f32 remain = static_cast< f32 >(mActor->getConst().getTable()->mBackJumpLimitFrame - _3CA);
            speedKiller =
                mActor->getConst().getTable()->mBackJumpRatio * (remain / static_cast< f32 >(mActor->getConst().getTable()->mBackJumpLimitFrame));
        } else {
            speedKiller = 0.0f;
        }

        if (mMovementStates._11) {
            moveDir = sideMove + mFrontVec * mActor->getConst().getTable()->mAirWalkBackBonus * frontDot;
        } else {
            moveDir = sideMove + mFrontVec * frontDot * speedKiller;
        }

        const f32 backDot = -moveDir.dot(mFrontVec);
        if (backDot > mActor->getConst().getTable()->mMaxBackJumpSpeed) {
            moveDir.setLength(mActor->getConst().getTable()->mMaxBackJumpSpeed);
        } else if (backDot < 0.0f && _3CA < mActor->mConst->getTable()->mBackJumpLimitFrame) {
            _3CC = 0xA;
        }
    }

    TVec3f moveNorm(moveDir);
    MR::normalizeOrZero(&moveNorm);

    TVec3f jumpNorm(mJumpVec);
    MR::normalizeOrZero(&jumpNorm);

    if (!inhibitFrontAdjust) {
        f32 alignment = moveNorm.dot(jumpNorm);
        if (alignment < 0.0f) {
            const f32 jumpGravity = cutGravityElementFromJumpVec(true);
            mJumpVec += mJumpVec * mActor->getConst().getTable()->mAirWalkSpeedKiller * alignment;

            mJumpVec += getAirGravityVec() * jumpGravity;
        }
    }

    if (_430 == 8) {
        mJumpVec += moveDir * mActor->getConst().getTable()->mWalkSpeed * 5.0f / mActor->getConst().getTable()->mAirWalkTimerFact2;
    } else if (_430 == 0xB && !isRising()) {
        mJumpVec += moveDir * mActor->getConst().getTable()->mWalkSpeed * 5.0f / mActor->getConst().getTable()->mAirWalkTimerFact2;
        playSound("滞空中");
    } else if (getPlayerMode() == 4) {
        mJumpVec += moveDir * mActor->getConst().getTable()->mBeeAirWalkAcc;
        playSound("滞空中");
    } else if (getPlayerMode() == 6) {
        if (mJumpVec.dot(moveDir) >= 0.0f) {
            f32 frontGravity = moveDir.dot(mFrontVec);
            if (frontGravity > 0.0f) {
                frontGravity = -frontGravity;
            }

            TVec3f normJump(mJumpVec);
            if (!MR::normalizeOrZero(&normJump)) {
                f32 alignment = normJump.dot(mFrontVec);
                alignment += 1.0f;
                f32 halfAlignment = alignment / 2.0f;
                mJumpVec *= 0.998f - 0.01f * (1.0f - halfAlignment);
            }
        }
    } else {
        mJumpVec += moveDir * mActor->getConst().getTable()->mWalkSpeed * mActor->getConst().getTable()->mAirWalkTimerFact1 /
                    (_3BC + mActor->getConst().getTable()->mAirWalkTimerFact2);
    }

    f32 accelerationRate = 1.0f;
    if (_430 != 0xB && getPlayerMode() != 6 && !isRising()) {
        addVelocity(moveDir, mActor->getConst().getTable()->mWalkSpeed * mActor->getConst().getTable()->mAirWalkTimerFact3 * accelerationRate);
    }

    const f32 jumpGravity = cutGravityElementFromJumpVec(true);
    if (_430 == 8) {
        if (mJumpVec.length() > 6.0f) {
            mJumpVec.setLength(6.0f);
        }
    } else if (_430 == 0xB) {
        if (mJumpVec.length() > 7.0f) {
            mJumpVec.setLength(7.0f);
        }
    } else if (getPlayerMode() == 4) {
        if (_774 == 0) {
            if (mJumpVec.length() > mActor->getConst().getTable()->mBeeAirWalkLimit) {
                mJumpVec.setLength(mActor->getConst().getTable()->mBeeAirWalkLimit);
            }
        } else if (_774 > 0x1E) {
            if (mJumpVec.length() > mActor->getConst().getTable()->mMaxJumpSpeed) {
                mJumpVec.setLength(mActor->getConst().getTable()->mMaxJumpSpeed);
            }
        } else {
            f32 t = static_cast< f32 >(_774) / 30.0f;
            const f32 limit = (t * mActor->mConst->getTable()->mMaxJumpSpeed) + ((1.0f - t) * mActor->getConst().getTable()->mBeeAirWalkLimit);
            if (mJumpVec.length() > limit) {
                mJumpVec.setLength(limit);
            }
        }
    } else if (getPlayerMode() != 6) {
        if (mJumpVec.length() > mActor->getConst().getTable()->mMaxJumpSpeed) {
            mJumpVec.setLength(mActor->getConst().getTable()->mMaxJumpSpeed);
        }
    }

    mJumpVec += getAirGravityVec() * jumpGravity;
}


void Mario::stopJump() {
    fixFrontVecByGravity();
    mJumpVec.zero();

    mMovementStates.jumping = false;
    mMovementStates._B = false;
    mMovementStates._6 = false;

    mMovementStates._28 = false;
    mMovementStates._2B = false;

    _10._17 = false;
    _10._19 = false;
    _10._1F = false;

    if (getPlayerMode() != 4) {
        _402 = mActor->getConst().getTable()->mAirWalkTime;
    }

    _3BC = 0;
    _424 = 0;
    _426 = 0;
    _428 = 0;

    if (isDefaultAnimationRun("落下")) {
        changeAnimation(nullptr, "基本");
    }

    if (isDefaultAnimationRun(_724)) {
        changeAnimation(nullptr, "基本");
    }

    mWall->_1C = 0;
}


void Mario::cancelTornadoJump() {
    _430 = 0;

    if ((mMovementStates._1) == 0) {
        stopAnimation("スピンジャンプ", "落下");
    } else {
        stopAnimation("スピンジャンプ");
    }

    _4B0 = mPosition;
    mMovementStates._2B = true;
    _426 = 0;
}


void Mario::setRocketBooster(const TVec3f& rVec, f32 a2, u16 a3) {
    _426 = a3;
    _454 = a2;
    _448 = rVec;
}


void Mario::procRocketBooster() {
    if (_426 == 0) {
        return;
    }

    _426--;
    mJumpVec += _448;
    _448.scale(_454);
}


bool Mario::isSoftLandingFloor() const {
    switch (_960) {
    case 0x6:
    case 0x7:
    case 0x8:
    case 0xB:
    case 0x22:
        return true;
    default:
        return false;
    }
}


void Mario::checkAndTryForceJump() {
    bool doForceJump = false;
    TVec3f forceJumpVec;

    if ((getMovementStates()._1 && !mMovementStates.jumping) || isAnimationRun(_728)) {
        switch (_960) {
        case 0x6:
        case 0x22: {
            const f32 jumpPower = mActor->getConst().getTable()->mCodeJumpPower[0];
            forceJumpVec = *mGroundPolygon->getNormal(0) * jumpPower;
            doForceJump = true;
            break;
        }

        case 0x7: {
            const f32 jumpPower = mActor->getConst().getTable()->mCodeJumpPower[1];
            forceJumpVec = *mGroundPolygon->getNormal(0) * jumpPower;
            doForceJump = true;
            break;
        }

        case 0x8: {
            const f32 jumpPower = mActor->getConst().getTable()->mCodeJumpPower[2];
            forceJumpVec = *mGroundPolygon->getNormal(0) * jumpPower;
            doForceJump = true;
            break;
        }

        case 0xB: {
            MR::vecKillElement(_16C, *getGravityVec(), &forceJumpVec);
            forceJumpVec.setLength(10.0f);

            const f32 jumpPower = mActor->getConst().getTable()->mCodeJumpPower[3];
            forceJumpVec += -(*getGravityVec()) * jumpPower;
            doForceJump = true;
            break;
        }

        default:
            break;
        }
    }

    if (mMovementStates._2D) {
        doForceJump = false;

        if (mActor->mHealth != 0) {
            if (getCurrentStatus() == 2) {
                closeStatus(nullptr);
            }

            tryForceFreeJump(_304);
        }

        mMovementStates._2D = false;
    }

    if (mMovementStates._2E) {
        doForceJump = false;

        if (mActor->mHealth != 0) {
            if (getCurrentStatus() == 2) {
                closeStatus(nullptr);
            }

            tryFreeJump(_304, true);
        }

        mMovementStates._2E = false;
    }

    if (doForceJump) {
        mActor->sendMsgToSensor(mGroundPolygon->mSensor, 2);

        if (getCurrentStatus() == 2) {
            if (mActor->mHealth == 0) {
                mActor->forceGameOverNonStop();
            }

            closeStatus(nullptr);
        }

        mMovementStates._21 = true;
        mMovementStates._2E = true;
        _304 = forceJumpVec;
        changeAnimation("フリージャンプ");

        if (mActor->isEnableNerveChange()) {
            playSound("トランポリンジャンプ大");
            playSound("声大ジャンプ");
            playSound("ジャンプ踏切");
        }
    }
}


void Mario::doLanding() {
    bool doHardLanding = false;
    bool blendWait = false;
    bool keepFrontSlip = false;

    if (getPlayerMode() == 6) {
        return;
    }

    if (mMovementStates._37) {
        afterLanding2D();
    }

    mMovementStates.jumping = false;
    _10._1F = false;
    _10._21 = false;
    _3CE = 0;
    _1C_WORD |= 0x2000;

    f32 deltadot = MR::abs((mPosition - _4B0).dot(*getGravityVec()));
    if (_3BC <= 3 && deltadot < 1.0f && mJumpVec.length() < 10.0f) {
        stopAnimation(nullptr, "基本");
        return;
    }

    if (mActor->_EEB && (mMovementStates._2B) && _10._19 && _430 != 3) {
        _10._19 = false;
        mActor->_946 = 0x10;
    }

    resetTornado();

    if (mActor->mBeeWallWalk != 0) {
        playSound("ハチ壁くっつき");
    } else {
        playSound("着地");
    }

    if ((isAnimationRun("スケートアクセルジャンプ") || isAnimationRun("スケートジャンプ2") || isAnimationRun("スケートジャンプ3")) &&
        (getPlayerMode() == 3 || getPlayerMode() == 0)) {
        playEffect("スケート着地");
        if (getStickP() == 0.0f) {
            mJumpVec.zero();
            stopWalk();
            changeAnimation("スケート静止着地", "基本");
        } else {
            changeStatus(mSkate);
        }

        return;
    }

    if (_430 == 5 && isSkatableFloor()) {
        if (checkSquat(false)) {
            mMovementStates._A = true;
            mWalkSpeed = 1.0f;
            stopAnimation(nullptr, "しゃがみ基本");
        } else {
            cancelSquatMode();
            stopAnimationUpper(nullptr);
            changeAnimation("スケート静止着地", "基本");
        }

        return;
    }

    if (isAnimationRun("フーファイター着地")) {
        stopAnimation(nullptr, "基本");
        return;
    }

    if (getPlayer()->_10._2) {
        getPlayer()->_10._2 = false;
        _430 = 0xE;
    } else {
        const u32 landingType = mMovementStates._3E;
        if (landingType == 2) {
            getPlayer()->mMovementStates._3E = 0;
            stopWalk();
            playSound("重い着地");
            playSound("声ステージイン着地");
            changeAnimation("ステージインB", "基本");
            startCamVib(4);
            return;
        }

        if (landingType == 1) {
            getPlayer()->mMovementStates._3E = 0;
            playEffect("スーパースピンドライバ終了");
            playEffect("共通着地大");
            if (!isStickFull()) {
                keepFrontSlip = true;
            }

            doHardLanding = true;
            if (_1FC.dot(_368) < 0.0f) {
                if (_960 == 0xD || _960 == 0x1E || _960 == 5 || _960 == 0x17) {
                    changeStatus(mSukekiyo);
                } else {
                    stopWalk();
                    startCamVib(4);
                    changeAnimation("ハード着地", "基本");
                    _3CE += 0x10;
                    forceSetHeadVecKeepSide(_368);
                }

                return;
            }
        } else {
            if (isCurrentFloorSink()) {
                stopAnimation(nullptr, "基本");
                mSinkTimer = 64;
                return;
            }

            if ((_960 == 0x1B || _960 == 0x1C)) {
                if (!strcmp(MR::getSoundCodeString(_45C), "Sand")) {
                    stopAnimation(nullptr, "基本");
                    mSinkTimer = 32;
                    return;
                }
            } else if (_3BC > 10) {
                playEffect("共通着地普通");
            }
        }
    }

    switch (_430) {
    case 0:
    case 0xA:
    case 0xB:
    case 0xE:
        startPadVib(static_cast< u32 >(0));
        break;
    default:
        startPadVib(2);
        break;
    case 2:
    case 4:
    case 6:
        startPadVib(3);
        break;
    }

    getAnimator()->addRumblePower(5.0f, 0x3C);
    mMovementStates._21 = false;

    if (getPlayerMode() != 4) {
        _402 = mActor->getConst().getTable()->mAirWalkTime;
    } else {
        _71E = 0;
    }

    if (!isSoftLandingFloor()) {
        f32 deltadot = (_4B0 - mPosition).dot(-(*getGravityVec()));
        if (deltadot > mActor->getConst().getTable()->mHardLandingHeight) {
            doHardLanding = true;
        }
    }

    if (doHardLanding && isCurrentFloorSand()) {
        changeStatus(mBury);
        return;
    }

    if (isBlendWaitGround()) {
        blendWait = true;
    }

    fixFrontVecByGravity();

    if (!doHardLanding && !mDrawStates._C && _430 != 0xE && !isAnimationRun("飛び込みジャンプ") && !isAnimationRun("後方飛び込みジャンプ") &&
        !_10._8 && !isAnimationRun("水上ダメージ中")) {
        if (_430 == 5 && checkSquat(false)) {
            mMovementStates._A = true;
            mWalkSpeed = 1.0f;
            stopAnimation(nullptr, "しゃがみ基本");
            goto POST_LANDING;
        }

        if (((isStickFull() || _60D) && _3CA == 0) || (((mMovementStates._B) == 0) && checkSquat(false))) {
            stopAnimation(nullptr, "基本");
            if (!_10._A) {
                recordTurnSlipAngle();
            }

            if (_430 == 5) {
                cancelSquatMode();
            }

            goto POST_LANDING;
        }

        switch (_430) {
        case 2:
        case 4:
        case 5:
        case 6:
        case 8:
        case 0xB:
            keepFrontSlip = true;
            break;
        default:
            break;
        }

        if (_10._17 && _8D8 == _45C->mSensor) {
            TVec3f jumpDelta(mJumpVec - _8DC);
            if (jumpDelta.dot(mJumpVec) < 0.0f) {
                mJumpVec.zero();
            } else {
                mJumpVec = jumpDelta;
            }
        }

        const f32 frontDot = mFrontVec.dot(mJumpVec);
        if (frontDot > 6.0f && !keepFrontSlip) {
            mWalkSpeed = frontDot / mActor->getConst().getTable()->mJumpFrontSpeed;
            mWalkSpeed = MR::clamp(mWalkSpeed, 0.0f, 2.0f);
            stopAnimation(nullptr, "基本");
            _3FA = 0;
            _71E = 0;

            if (!_10._A) {
                recordTurnSlipAngle();
            }

            goto POST_LANDING;
        }
    }

    if (_430 == 4) {
        setFrontVecKeepUp(-_220);
    }

    if (isAnimationRun("飛び込みジャンプ") || isAnimationRun("後方飛び込みジャンプ") || _10._8) {
        if (isAnimationRun("後方飛び込みジャンプ")) {
            setFrontVecKeepUp(-mFrontVec);
        }

        if (_10._8) {
            doHardLanding = true;
        }

        stopWalk();
        if (doHardLanding || isAnimationRun("後方飛び込みジャンプ")) {
            if (doHardLanding) {
                changeAnimation("飛び込み失敗回転着地", "基本");
                startSlidingTask(8, 5.0f, 0x19);
                startCamVib(0);
            } else {
                changeAnimation("飛び込み失敗着地", "基本");
            }
        } else {
            changeAnimation("飛び込み失敗着地", "基本");
        }

        goto POST_LANDING;
    }

    if (!mDrawStates._C) {
        clearSlope();
        switch (_430) {
        case 0:
        case 7:
            changeAnimation("着地", "基本");
            break;
        case 1:
            changeAnimation("着地B", "基本");
            break;
        case 2:
            changeAnimation("着地C", "基本");
            break;
        case 4:
            changeAnimation("着地ターン", "基本");
            break;
        case 5:
            if (!checkSquat(false)) {
                cancelSquatMode();
            }

            if (mActor->_468 == 0) {
                stopAnimationUpper(nullptr);
            }

            changeAnimation("着地幅とび", "基本");
            break;
        case 6:
            changeAnimation("しゃがみジャンプ着地", "基本");
            break;
        default:
            if (isAnimationRun("水上ダメージ中")) {
                changeAnimation("中ダメージ着地", "基本");
            } else {
                changeAnimation("着地", "基本");
            }

            break;
        case 3:
        case 0xA:
            stopAnimation(nullptr, "基本");
            break;
        case 0xE:
            playSound("倒れ");
            playSound("着地");
            playSound("声小ダメージ");
            playEffect("共通引き戻し着地");
            changeAnimation("引き戻し着地", "基本");
            doHardLanding = false;
            blendWait = false;
            break;
        }

        stopWalk();
        if (blendWait) {
            changeAnimation("ショート着地", "基本");
        }

        if (doHardLanding) {
            changeAnimation("ハード着地", "基本");
            playSound("重い着地");
            playSound("声着地停止");
            startCamVib(4);
            _3CE += 0x10;
        }

        if (mActor->mBeeWallWalk != 0) {
            changeAnimation("ハチ匍匐着地", "基本");
        }
    } else {
        if (!isAnimationRun("坂すべり上向きうつぶせ", 2) && !isAnimationRun("坂すべり下向きあおむけ", 3)) {
            stopAnimation(nullptr, "基本");
        }

        changeAnimation("すべり着地", "基本");
        _71E = 0;
    }

POST_LANDING:
    if (mDrawStates._C) {
        TVec3f jumpNoGrav;
        MR::vecKillElement(mJumpVec, *getGravityVec(), &jumpNoGrav);

        if (jumpNoGrav.dot(*mGroundPolygon->getNormal(0)) > 0.0f) {
            mMovementStates._23 = true;
            _8F0 = 10.0f;
        } else {
            const f32 ratio = (90.0f - calcPolygonAngleD(mGroundPolygon)) / 90.0f;
            mWalkSpeed = mWalkSpeed * ratio;
            mWalkSpeed = MR::clamp(mWalkSpeed, 0.0f, 2.0f);

            const f32 minVel = 10.0f - (10.0f * ratio);
            if (_8F0 < minVel) {
                _8F0 = minVel;
            }
        }

        if (jumpNoGrav.length() < _8F8.length()) {
            _8F8.setLength(0.5f * (_8F8.length() + jumpNoGrav.length()));
        }
    } else {
        _8F8.zero();
    }

    return;
}


void Mario::startSlidingTask(u32 a1, f32 a2, u16 a3) {
    _A40 = mFrontVec * a2;
    _A3C = a3;
    pushTask(reinterpret_cast< Task >(&Mario::taskOnSlide), a1);
}


bool Mario::taskOnSlide(u32 a1) {
    (void)a1;
    addVelocity(_A40);
    _A3C--;
    return _A3C != 0;
}


void MarioJump_FORCE_MATCH_DATA() {
    Mario::Task task = &Mario::taskOnWallRising;
    (void)task;
}


bool Mario::taskOnWallRising(u32 a1) {
    (void)a1;

    if ((mMovementStates._17) == 0) {
        stopEffect("共通壁上昇");
        return false;
    }

    playSound("スリップ");
    return true;
}


void Mario::incAirWalkTimer() {
    const u16 timer = _402;
    const u16 limit = mActor->getConst().getTable()->mAirWalkTime;

    if (timer >= limit) {
        return;
    }

    _402 = timer + 1;

    if (getPlayerMode() != 4) {
        return;
    }

    const u16 nextTimer = _402;
    const u16 nextLimit = mActor->getConst().getTable()->mAirWalkTime;

    if (nextTimer >= nextLimit) {
        playSound("ハチ体力完全回復");
    } else {
        playSound("ハチ体力回復");
    }
}

// ---- MarioMagic.cpp ----

void Mario::stopPunch() {
    if (isStatusActive(MarioStatus_Magic)) {
        closeStatus(mMagic);
    }

    MarioActor* actor = mActor;

    if (!actor->_944) {
        actor->_945 = 0;
        actor->_974 = 0;
    }

    actor->_944 = 0;
}


void Mario::startMagic() {
    if (!mMovementStates.jumping) {
        if (!mActor->_468) {
            if (!mMovementStates._23) {
                if (!isStatusActive(MarioStatus_Slider)) {
                    if (isSkatableFloor()) {
                        doSkate();
                    } else {
                        clearSlope();
                        changeStatus(mMagic);
                        stopAnimationUpper(nullptr);
                        _10._1 = 1;
                    }
                }
            }
        }
    }
}

// ---- MarioMove.cpp ----

void Mario::mainMove() {
    TVec3f vec1;
    MR::setNan(vec1);

    bool a1 = false;

    if (mDrawStates._5) {
        a1 = true;
    }

    if (mMovementStates._23) {
        mTargetWalkSpeedIndex = 0;
        mWalkSpeed = 0.0f;
        a1 = true;

        playSound("坂滑り");
    }

    if (_754) {
        a1 = true;
    }

    mMovementStates._35 = false;

    if (isSkatableFloor()) {
        mMovementStates._35 = true;
    }

    if (_10._1A) {
        tryJump();
        beforeJumping2D();

        if (isAnimationRun("その場足踏み上半身")) {
            stopAnimationUpper(nullptr);
        }

        _420 = 0;
        mJumpVec = _184;
        _10._1A = false;
        return;
    }

    bool a2 = false;
    bool isRequestJump = mActor->isRequestJump();

    if (!isRequestJump && mActor->isRequestJump2P() && _3C8 == 0) {
        _3C8 = 6;
    }

    if (!isRequestJump) {
        if (_3C8 != 0) {
            _3C8--;
            if (_3C8 == 0) {
                isRequestJump = true;
                _1C._A = true;
            }
        }
    } else if (_3C8 != 0) {
        _1C._B = true;
        _3C8 = 0;
    }

    if (isRequestJump || mMovementStates._38) {
        if (mMovementStates._A && calcDistToCeil(false) < 160.0f) {
            mActor->sendMsgUpperPunch(_730);
            changeAnimation("しゃがみアッパー");
        } else if (isStatusActive(MarioStatus_Skate)) {
            mSkate->exitJump();
            closeStatus(mSkate);
        } else {
            a2 = true;
        }
    }

    if (_436 != 0) {
        _436--;
        if (a2 && checkSquat(false)) {
            if (mStickPos.z >= 0.5f) {
                a2 = false;
                _436 = 0;
                mWalkSpeed = 5.0f;
                _434 = 180;

                playSound("スペシャルダッシュ強");
                playSound("ダッシュ加速強成功");
                playSound("声物ジャンプ");
            }
        } else if (a2 && mStickPos.z >= 0.5f) {
            a2 = false;
            _436 = 0;
            mWalkSpeed = 2.0f;
            _434 = 60;
            playSound("スペシャルダッシュ弱");
            playSound("声物ジャンプ");
        }
    }

    if (a2) {
        saveLastSafetyTrans();
        tryJump();
        beforeJumping2D();

        if (isAnimationRun("その場足踏み上半身")) {
            stopAnimationUpper(nullptr);
        }

        _420 = 0;
        return;
    }

    if (!mMovementStates._1) {
        if (_3C0 != 0 && !mMovementStates._23 && MR::isNearZero(_8F8)) {
            _3C0--;
        } else if (!mActor->_EA4) {
            tryDrop();
            beforeJumping2D();
            _420 = 0;
            return;
        }
    } else if (mStickPos.z < mActor->getConst().getTable()->mWallBackHangStickPower) {
        _3C0 = 0;
    } else if (mDrawStates._A) {
        _3C0 = 0;
    } else if (_3C0 < mActor->getConst().getTable()->mDropWaitTime) {
        _3C0 = mActor->getConst().getTable()->mDropWaitTime;
    }

    if (_420 != 0) {
        _420--;
        _10._28 = true;
        return;
    }

    checkTornado();

    getAnimator()->setWalkMode();

    if (MR::isNearZero(mStickPos.x) && MR::isNearZero(mStickPos.y)) {
        _328 = _334;

        if (MR::isNearZero(_328)) {
            _328 = mFrontVec;
        }

        vec1 = _328;

        calcShadowDir(_328, &_22C);
        _40C = 10;
    } else if (!(mTargetWalkSpeedIndex != 0 && _71D) && (!isAnimationRun("ブレーキ") && !isAnimationRun("ターンブレーキ") && !mMovementStates._10 &&
                                                         !mMovementStates._F && mWalkSpeed < 0.05f && !mDrawStates._5)) {
        bool a3 = true;
        if (_40C != 0) {
            if (mStickPos.z < 0.75f) {
                a3 = false;
                _40C--;
                mStickPos.set(0.0f);
            } else {
                _40C = 0;
            }
        }

        if (a3) {
            calcMoveDir(mStickPos.x, mStickPos.y, &vec1, true);
            calcShadowDir(vec1, &_22C);
            calcShadowDir(mFrontVec, &_214);

            f32 angle = MR::pi() * 10.0f / 180.0f;
            f32 f1;
            if (mMovementStates._37) {
                f1 = MR::diffAngleAbsHorizontal(_214, _22C, _6A0);
                angle = MR::pi() * 90.0f / 180.0f;
            } else {
                f1 = MR::diffAngleAbsHorizontal(_214, _22C, *getGravityVec());
            }

            if (f1 > angle) {
                mDrawStates._E = true;
                u32 turnTime = mActor->getConst().getTable()->mStandingTurnTime;
                if (mActor->isBeeWallWalk() != false) {
                    turnTime = 30;
                }

                if (!a1) {
                    setFrontVecKeepUp(_22C, turnTime);
                }

                if (!isEnableTurn()) {
                    return;
                }

                changeAnimation("その場足踏み");

                if (isAnimationRun("カリカリ限界")) {
                    return;
                }

                changeAnimationUpperWeak("その場足踏み上半身");
                return;
            } else if (!a1) {
                setFrontVecKeepUp(_22C);
            }
        } else {
            vec1 = _328;
        }
    } else {
        f32 stickposX = mStickPos.x;
        f32 stickposY = mStickPos.y;

        if (!getPlayer()->_10._11) {
            if (MR::abs(stickposY) > mActor->getConst().getTable()->mStickMarginYstart) {
                if (MR::abs(stickposX) < mActor->getConst().getTable()->mStickMarginX) {
                    stickposX = 0.0f;
                } else if (stickposX > 0.0f) {
                    stickposX = (stickposX - mActor->getConst().getTable()->mStickMarginX) / (1.0f - mActor->getConst().getTable()->mStickMarginX);
                } else {
                    stickposX = (stickposX + mActor->getConst().getTable()->mStickMarginX) / (1.0f - mActor->getConst().getTable()->mStickMarginX);
                }
            } else if (MR::abs(stickposX) > mActor->getConst().getTable()->mStickMarginXstart) {
                if (MR::abs(stickposY) < mActor->getConst().getTable()->mStickMarginY) {
                    stickposY = 0.0f;
                } else if (stickposY > 0.0f) {
                    stickposY = (stickposY - mActor->getConst().getTable()->mStickMarginY) / (1.0f - mActor->getConst().getTable()->mStickMarginY);
                } else {
                    stickposY = (stickposY + mActor->getConst().getTable()->mStickMarginY) / (1.0f - mActor->getConst().getTable()->mStickMarginY);
                }
            }
        }

        calcMoveDir(stickposX, stickposY, &vec1, true);

        calcShadowDir(vec1, &_22C);
    }

    calcShadowDir(mFrontVec, &_214);

    f32 dot = _214.dot(_22C);
    f32 dot2 = dot;

    if (_3D2 != 0) {
        dot2 = _3E4.dot(_22C);
    }

    if (dot >= MR::cos(mActor->getConst().getTable()->mTurnSlipAngle) && !mMovementStates._4 &&
        mWalkSpeed > mActor->getConst().getTable()->mSlipSpeed && _38 > mStickPos.z && !mDrawStates._D) {
        recordTurnSlipAngle();
    } else if (_3D2 != 0) {
        _3D2--;
    }

    if (mMovementStates._F || mMovementStates._34) {
        _3D2 = 0;
        mMovementStates._10 = false;
    }

    if (_3D2 != 0) {
        if (mStickPos.z < mActor->getConst().getTable()->mTurnSlipNeutral) {
            mMovementStates._10 = true;
        }

        if (mWalkSpeed < 0.1f) {
            _3D2 = 0;
            mWalkSpeed = 0.0f;
        }

        if (isAnimationRun("ブレーキ")) {
            a1 = true;
        }

        if (mMovementStates._10) {
            a1 = true;
        }
    } else {
        if (!mMovementStates._34 && !mMovementStates._A && mMovementStates._10 == true && !mMovementStates._F && !mMovementStates._23 &&
            !isStatusActive(MarioStatus_Skate) && dot2 < MR::cos(mActor->getConst().getTable()->mTurnSlipAngle) && !mMovementStates._4) {
            if (mMovementStates._35) {
                a1 = true;
            }

            _220 = _3E4;

            if (!a1) {
                setFrontVecKeepUp(-_220);
            }

            if (mMovementStates._35) {
                _3D0 = mActor->getConst().getTable()->mTurnSlipTimeB;
                mMovementStates._4 = true;
                changeAnimation("ターンブレーキ滑り床");
                _2B8 = mActor->getLastMove();
                stopWalk();
                _754 = 10;
                pushTask(&Mario::taskOnSlipTurn, 1);
            } else {
                _3D0 = mActor->getConst().getTable()->mTurnSlipTime;
                mMovementStates._4 = true;
                mWalkSpeed = 0.0f;
                changeAnimation("ターンブレーキ");
                playEffect("共通ブレーキ");
            }
        }

        mMovementStates._10 = false;
    }

    if (_3D0 != 0) {
        if (!mMovementStates._35) {
            playSound("スリップ");
        }

        if (_3D0 == static_cast< u16 >(mActor->getConst().getTable()->mTurnSlipTime)) {
            startPadVib(2);
        }

        _3D0--;

        if (!mMovementStates._35) {
            mWalkSpeed = 0.0f;
        }
    }

    if (_3D0 == 0) {
        if (mMovementStates._4) {
            TVec3f* frontVec = &mFrontVec;
            if (MR::diffAngleAbsHorizontal(getWorldPadDir(), *frontVec, *getGravityVec()) > MR::pi() * 90.0f / 180.0f) {
                stopAnimation("ターンブレーキ");
                setFrontVecKeepUp(-*frontVec);
                mMovementStates._4 = false;
            }

            if (isAnimationTerminate("ターンブレーキ")) {
                stopAnimation("ターンブレーキ");
                mMovementStates._4 = false;

                if (mStickPos.z < 0.1f) {
                    stopWalk();
                }
            }
        }

        if (isActiveTask(&Mario::taskOnSlipTurn)) {
            if (isAnimationRun("ターンブレーキ滑り床")) {
                setFrontVecKeepUp(-_220);
                a1 = true;

                stopAnimation("ターンブレーキ滑り床");
                _754 = 0;
                _74C = 0.0f;
                mWalkSpeed = 0.0f;
                popTask(&Mario::taskOnSlipTurn);
            }
        }
    }

    f32 turnAngleSpeed = mActor->getConst().getTable()->mTurnAngleSpeed;

    if (mWalkSpeed > mActor->getConst().getTable()->mFastTurnSpeed) {
        turnAngleSpeed = mActor->getConst().getTable()->mTurnAngleSpeed2;
        if (mTargetWalkSpeedIndex < 5) {
            turnAngleSpeed = mActor->getConst().getTable()->mTurnAngleSpeedSlowWalk;
        }

        if (isStatusActive(MarioStatus_Skate)) {
            turnAngleSpeed *= 0.5f;
        }
    }

    if (mMovementStates._23) {
        turnAngleSpeed = mActor->getConst().getTable()->mTurnAngleSpeed3;
    }

    if (mMovementStates._F) {
        turnAngleSpeed = mActor->getConst().getTable()->mTurnAngleSpeedTornado;
    }

    bool a4;

    if (!mMovementStates._4) {
        TVec3f cross(_238.cross(_22C));
        TVec3f stick;
        mActor->getStickValue(&stick.x, &stick.y);
        stick.z = 0.0f;
        PSVECCrossProduct(&_250, &stick, &cross);

        a4 = true;

        if (mMovementStates._37) {
            a4 = false;
        }

        do {
            if (MR::isNearZero(mStickPos.z)) {
                _10._C = false;
                mMovementStates.turning = false;
                break;
            }

            if (MR::isNearZero(_244) && a4) {
                bool isEqual = false;
                bool equalXY = JGeometry::TUtil< f32 >::epsilonEquals(_250.x, stick.x, JGeometry::TUtil< f32 >::epsilon()) &&
                               JGeometry::TUtil< f32 >::epsilonEquals(_250.y, stick.y, JGeometry::TUtil< f32 >::epsilon());
                if (equalXY && JGeometry::TUtil< f32 >::epsilonEquals(_250.z, stick.z, JGeometry::TUtil< f32 >::epsilon())) {
                    isEqual = true;
                }

                bool isDifferent = !isEqual;
                if (isDifferent) {
                    mMovementStates.turning = true;
                    break;
                }
            } else if (cross.dot(_244) > 0.0f && a4) {
                mMovementStates.turning = true;
                break;
            }

            if (_3D0 != 0) {
                _10._C = false;
                mMovementStates.turning = false;
            }

            if (MR::isInRange(MR::diffAngleAbs(_214, _22C), 0.0f, 0.707f)) {
                TVec3f cross2(_214.cross((_22C)));

                if (_274 != 0) {
                    if (cross2.dot(mHeadVec) > 0.0f) {
                        _10._C = false;
                    }
                } else if (cross2.dot(mHeadVec) < 0.0f) {
                    _10._C = false;
                }
            }
        } while (false);

        _250 = stick;

        if (mMovementStates.turning) {
            TVec3f cross3(_214.cross((_22C)));

            if (cross3.dot(_3D8) < 0.0f) {
                if (!(cross.dot(_244) > 0.0f) && !(_214.dot(_22C) < 0.0f)) {
                    _3D4 = 0;
                }
            } else if (_3D4 < mActor->getConst().getTable()->mWeakTurnTime) {
                _3D4++;
            }

            _3D8 = cross3;
            _244 = cross;
            _238 = _22C;

            f32 f1 = 1.0f;

            if (_3D4 < mActor->getConst().getTable()->mWeakTurnTime && mTargetWalkSpeedIndex > 4) {
                f1 = static_cast< f32 >(_3D4) / static_cast< f32 >(mActor->getConst().getTable()->mWeakTurnTime);
            }

            turnAngleSpeed *= MR::sqrt(f1);
        } else {
            if (_3D4 != 0) {
                _3D4--;
            }

            if (_3D4 == 0) {
                turnAngleSpeed *= 0.01f;
                _244.zero();
                PSVECCrossProduct(&_214, &_22C, &_3D8);
            }

            _238 = _22C;
        }

        bool turnControl = true;

        if (mDrawStates._C) {
            if (MR::diffAngleAbsHorizontal(_8F8, -mFrontVec, *getGravityVec()) < MR::pi() * 45.0f / 180.0f) {
                turnAngleSpeed *= 0.1f;
            } else {
                turnAngleSpeed *= 0.3f;
            }
        }

        TVec3f vec2;

        if (turnControl) {
            if (mMovementStates._37) {
                _40E = 0;
                mDrawStates._D = false;
                _10._C = false;
                vec2 = _22C;
            } else if (_10._C) {
                f32 angle = 0.12f;
                if (_274 == 0) {
                    angle = -angle;
                }

                Mtx mtx;
                PSMTXRotAxisRad(mtx, &mHeadVec, angle);
                PSMTXMultVecSR(mtx, &_214, &vec2);
            } else if (MR::diffAngleAbs(mFrontVec, _22C) < 0.1f) {
                if (_524 == _528) {
                    vec2 = _22C;
                } else {
                    vec2 = mFrontVec;
                }
            } else {
                f32 diffAngleAbs = MR::diffAngleAbs(mFrontVec, _22C);

                if (diffAngleAbs > 0.0f) {
                    f32 f1 = turnAngleSpeed / diffAngleAbs;
                    MR::clamp01(&f1);

                    if (mActor->isBeeWallWalk() != false && f1 > 0.1f) {
                        f1 = 0.1f;
                    }

                    if (getPlayer()->_10._12) {
                        f32 ratio = mActor->getConst().getTable()->mStickHeavyMinRatio;
                        f32 minAngle = mActor->getConst().getTable()->mStickHeavyMinAngle;
                        f32 maxAngle = mActor->getConst().getTable()->mStickHeavyMaxAngle;
                        if (diffAngleAbs >= maxAngle) {
                            ratio = 1.0f;
                        } else if (diffAngleAbs > minAngle) {
                            ratio += (1.0f - ratio) * ((diffAngleAbs - minAngle) / (maxAngle - minAngle));
                        }

                        f1 *= ratio;

                        f32 f2 = 1.0f - ((MR::pi() - diffAngleAbs) / MR::pi());
                        MR::clamp01(&f2);

                        mWalkSpeed *= 1.0f - 0.1f * f2;
                    }

                    if (!MR::vecBlendSphere(mFrontVec, _22C, &vec2, f1)) {
                        _3D4 = 0;
                        MR::vecRotAxis(_214, _22C, mHeadVec, &vec2, 22.5f * MR::pi() / 180.0f);
                    }

                    if (diffAngleAbs > 2.5f && _3D4 == mActor->getConst().getTable()->mWeakTurnTime && !mDrawStates._D) {
                        TVec3f cross3(_214.cross(vec2));
                        _274 = cross3.dot(mHeadVec) > 0.0f;
                        _10._C = true;
                    }
                }
            }

            mMovementStates.turning = true;

            if (!isAnimationRun("その場足踏み") && dot > 0.99f) {
                mMovementStates.turning = false;
            }

            if (mMovementStates._37 && mFrontVec.dot(vec2) < 0.0f && !isAnimationRun("ブレーキ")) {
                _750 = 10;
                _74C = MR::pi();
            }

            if (!a1) {
                setFrontVecKeepUp(vec2);
            }
        }
    } else {
        if (_3D0 == 0) {
            TVec3f blendSphere;
            if (!MR::vecBlendSphere(_214, _22C, &blendSphere, turnAngleSpeed)) {
                MR::vecRotAxis(_214, _22C, mHeadVec, &blendSphere, 22.5f * MR::pi() / 180.0f);
            }

            if (!a1) {
                setFrontVecKeepUp(blendSphere);
            }
        }

        _10._C = false;
        mMovementStates.turning = false;
    }

    if (_750 != 0 && mTargetWalkSpeedIndex == 0 && isEnableTurn()) {
        changeAnimation("その場足踏み");

        if (!isAnimationRun("カリカリ限界")) {
            changeAnimationUpperWeak("その場足踏み上半身");
        }
    } else {
        if (mTargetWalkSpeedIndex != 0) {
            stopAnimation("その場足踏み");
        }

        if (isAnimationRun("その場足踏み上半身")) {
            stopAnimationUpper(nullptr);
        }
    }

    TVec3f vec2;

    if (mMovementStates._34) {
        TVec3f vec3(mFrontVec);
        TVec3f vec4(mFrontVec);
        if (!MR::isNearZero(_16C)) {
            vec4 = _16C;
        }

        MR::normalize(&vec4);

        f32 f1 = (1.1f - mWalkSpeed) * mActor->getConst().getTable()->mInertiaIceTurn;

        MR::clamp01(&f1);

        MR::vecBlendSphere(vec4, vec3, &vec2, f1);

        MR::normalize(&vec2);

        vec2.scale(mWalkSpeed * mActor->getConst().getTable()->mWalkSpeed);

    } else if (mDrawStates._5) {
        vec2 = _22C * mActor->getConst().getTable()->mWalkSpeed * mWalkSpeed;
    } else {
        vec2 = mFrontVec * mActor->getConst().getTable()->mWalkSpeed * mWalkSpeed;
    }

    addVelocity(vec2);

    _328 = vec1;

    if (mMovementStates._A) {
        if (mTargetWalkSpeedIndex != 0) {
            _334 = vec1;
        }
    } else if (_3D2 == 0) {
        _334 = vec1;
    }

    if (mMovementStates._32 && _4E0 > 120.0f) {
        if (mVelocity.dot(*mFrontWallTriangle->getNormal(0)) < 0.0f) {
            TVec3f vec4;
            MR::vecKillElement(mPosition - _4E8, *mFrontWallTriangle->getNormal(0), &vec4);
            MR::vecKillElement(vec4, *getGravityVec(), &vec4);
            MR::vecKillElement(vec4, mFrontVec, &vec4);
            MR::normalizeOrZero(&vec4);

            if (!MR::isNearZero(vec4)) {
                mVelocity = vec4 * mVelocity.length();
            }
        }
    } else if (mMovementStates._8 && mVelocity.dot(*mFrontWallTriangle->getNormal(0)) < 0.0f) {
        f32 pushAngle = -mFrontVec.dot(*mFrontWallTriangle->getNormal(0));
        if (pushAngle < MR::cosDegree(mActor->getConst().getTable()->mWallPushAngleRange)) {
            MR::vecKillElement(mVelocity, *mFrontWallTriangle->getNormal(0), &mVelocity);
        }
    }

    TVec3f newVelocity(mVelocity);
    f32 speed = newVelocity.length();

    MR::normalizeOrZero(&newVelocity);

    TVec3f cross4(_368.cross(newVelocity));
    MR::normalizeOrZero(&cross4);

    PSVECCrossProduct(&cross4, &_368, &newVelocity);
    newVelocity.setLength(speed);

    mVelocity = newVelocity;
}


bool Mario::isEnableTurn() {
    if (!mMovementStates._1) {
        return false;
    }

    if (mMovementStates._4) {
        return false;
    }

    if (mDrawStates._5) {
        return false;
    }

    if (mMovementStates._23) {
        return false;
    }

    if (mMovementStates._34) {
        return false;
    }

    if (mDrawStates._A) {
        return false;
    }

    if (mMovementStates._A) {
        return false;
    }

    if (isStatusActive(MarioStatus_Magic)) {
        return false;
    }

    if (isAnimationRun("坂すべり上向きうつぶせ", 2)) {
        return false;
    }

    if (isAnimationRun("坂すべり下向きあおむけ", 3)) {
        return false;
    }

    if (isAnimationRun("坂すべり下向き終了")) {
        return false;
    }

    if (isAnimationRun("坂すべり上向き終了")) {
        return false;
    }

    if (isAnimationRun("飛び込み失敗着地")) {
        return false;
    }

    if (isAnimationRun("飛び込み失敗回転着地")) {
        return false;
    }

    if (mActor->_480) {
        return false;
    }

    if (mActor->isPunching()) {
        return false;
    }

    if (mActor->isItemSwinging()) {
        return false;
    }

    if (mActor->isBeeWallWalk()) {
        return false;
    }

    if (mActor->_3C0) {
        return false;
    }

    if (mActor->_EA4) {
        return false;
    }

    if (mMovementStates._37 || _10._15) {
        return false;
    }

    return true;
}


void Mario::recordTurnSlipAngle() {
    if (!mActor->isBeeWallWalk()) {
        _3E4 = mFrontVec;
        _3D2 = mActor->getConst().getTable()->mTurnReadyTime;
    }
}


f32 Mario::decideInertia(f32 speed) {
    if (isStatusActive(MarioStatus_Skate)) {
        return decideInertiaOnIce(speed);
    }

    if (mWalkSpeed > 1.0f) {
        return mActor->getConst().getTable()->mInertiaOverSpeed;
    }

    if (mMovementStates._34) {
        return decideInertiaOnIce(speed);
    }

    if (mMovementStates._35) {
        return decideInertiaOnSlip(speed);
    }

    f32 inertia =
        (1.0f - mWalkSpeed) * mActor->getConst().getTable()->mInertiaStandardStop + mWalkSpeed * mActor->getConst().getTable()->mInertiaStandardMax;
    inertia = (1.0f - _3F4) * inertia + _3F4 * mActor->getConst().getTable()->mInertiaStartSpin;
    if (speed == 0.0f) {
        inertia = mActor->getConst().getTable()->mInertiaStop;
        if (mMovementStates._10) {
            inertia = mActor->getConst().getTable()->mInertiaTurnSlip;
        }

        if (mMovementStates._4) {
            inertia = mActor->getConst().getTable()->mInertiaTurning;
        }

        if (_3CE < 30) {
            inertia = mActor->getConst().getTable()->mInertiaJumpFinish;
        }

        if (mMovementStates._A) {
            inertia = mActor->getConst().getTable()->mInertiaSquat;
        }
    }

    if (mMovementStates._F) {
        if (mWalkSpeed >= speed) {
            inertia = mActor->getConst().getTable()->mInertiaTornadoBrake;
        } else {
            inertia = mActor->getConst().getTable()->mInertiaTornadoAccel;
        }
    }

    if (_3F8) {
        _3F8--;
        inertia = mActor->getConst().getTable()->mInertiaReflectSlip;
    }

    if (mWalkSpeed < 0.08f && _3CE > 10 && speed > 0.5f && (mMovementStates._A || mMovementStates._C)) {
        _3FA = mActor->getConst().getTable()->mStartSpinTime;
        mWalkSpeed = 0.08f;
    }

    if (_3FA) {
        _3FA--;
        inertia = mActor->getConst().getTable()->mInertiaStartSpin;
        if (!_3FA) {
            _3FC = 60;
        }
    }

    if (_3FC) {
        _3FC--;
    }

    if (getFloorCode() == CollisionFloorCode_Brake && mWalkSpeed > 0.4f) {
        inertia = 0.5f * inertia;
    }

    return inertia;
}


f32 Mario::decideInertiaOnIce(f32 speed) {
    if (speed > 1.0f) {
        speed = 1.0f;
    }

    f32 inertia =
        (1.0f - speed) * mActor->getConst().getTable()->mInertiaIceStandardStop + speed * mActor->getConst().getTable()->mInertiaIceStandardMax;
    inertia = (1.0f - _3F4) * inertia + _3F4 * mActor->getConst().getTable()->mInertiaIceStartSpin;
    if (speed == 0.0f) {
        inertia = mActor->getConst().getTable()->mInertiaIceStop;
    }

    return inertia;
}


f32 Mario::decideInertiaOnSlip(f32 speed) {
    f32 inertia = (1.0f - mWalkSpeed) * mActor->getConst().getTable()->mInertiaSlipStandardStop +
                  mWalkSpeed * mActor->getConst().getTable()->mInertiaSlipStandardMax;
    inertia = (1.0f - _3F4) * inertia + _3F4 * mActor->getConst().getTable()->mInertiaSlipStartSpin;
    if (speed == 0.0f) {
        inertia = mActor->getConst().getTable()->mInertiaSlipStop;
    }

    return inertia;
}


void Mario::calcShadowDir(const TVec3f& rInput, TVec3f* pShadowDir) {
    TVec3f direction;
    direction = rInput;
    if (!MR::isNormalize(direction)) {
        MR::normalizeOrZero(&direction);
    }

    if (mMovementStates._37 || _10._15) {
        calcShadowDir2D(direction, pShadowDir);
        return;
    }

    TVec3f normal = -*getGravityVec();
    f32 dot = direction.dot(normal);
    direction.dot(mHeadVec);
    if (MR::abs(dot) > MR::abs(direction.dot(_368))) {
        normal = _368;
    }

    TVec3f side = normal.cross(direction);
    pShadowDir->cross(side, normal);
    MR::normalizeOrZero(pShadowDir);
}


bool Mario::retainMoveDir(f32 stickX, f32 stickY, TVec3f* pMoveDir) {
    f32 angle = MR::atan2(stickY, stickX);
    f32 difference = MR::diffAngleAbs(angle, _2B4);
    if (isAnimationRun("その場足踏み")) {
        difference = 1.0f;
    }

    if (isAnimationRun("ターンブレーキ")) {
        difference = 1.0f;
    }

    if (isSwimming()) {
        difference = 1.0f;
    }

    if (mActor->isRequestSpin() && mMovementStates.jumping && mWorldPadDir.dot(_16C) < 0.0f) {
        difference = 1.0f;
    }

    f32 limit = 0.06f;
    if (_3CE < 2) {
        limit = 0.1f;
    }

    if (_10._B) {
        if (isStickOn()) {
            _10._B = false;
        }

        difference = 1.0f;
    }

    bool retain = true;
    mDrawStates._16 = true;
    if (mMovementStates.jumping && !_10._A) {
        retain = false;
    }

    if (mMovementStates.jumping && mMovementStates._8) {
        retain = false;
    }

    if (mMovementStates.jumping && !isAnimationRun(nullptr)) {
        retain = false;
    }

    if (difference < limit && isStickOn() && retain) {
        if (!_40E || (_10._A && mMovementStates.jumping)) {
            mDrawStates._D = true;
            _40E = 0;
        } else {
            if (_40E) {
                _40E--;
            }

            _29C = _368;
            _2A8 = *getGravityVec();
        }

        _2B4 = angle;
    } else {
        if (mMovementStates.jumping) {
            if (isStickOn()) {
                _10._A = false;
            }
        } else {
            _10._A = true;
        }

        if (!_40E) {
            _3D4 = 0;
        }

        _40E = 30;
        if (!isStickOn()) {
            _10._B = true;
        }

        _2B4 = angle;
        _29C = _368;
        _2A8 = *getGravityVec();
    }

    return false;
}


void Mario::calcMoveDir(float stickX, float stickY, TVec3f* pMoveDir, bool isDisableMove) {
    if (mMovementStates._37) {
        calcDir2D(stickX, stickY, pMoveDir);
    } else if (_10._15) {
        calcMoveDir2D(stickX, stickY, pMoveDir);
    } else if (mMovementStates._3A) {
        calcMoveDir25D(stickX, stickY, pMoveDir);
    } else if (!isDisableMove || !retainMoveDir(stickX, stickY, pMoveDir)) {
        const TVec3f& screenXDir = getCamDirX();
        const TVec3f& screenYDir = getCamDirY();
        TVec3f screenZDir = -getCamDirZ();
        TVec3f groundXDir;
        TVec3f groundYDir;
        TVec3f groundYDirOrtho;

        TVec3f marioUpDir = -*getGravityVec();
        if (!MR::isNearZero(marioUpDir)) {
            float dot = _398.dot(marioUpDir);
            if (dot <= 0.0f) {
                _398 = marioUpDir;
            } else {
                MR::vecBlendSphere(_398, marioUpDir, &_398, 0.1f);
            }

            MR::normalizeOrZero(&_398);
        }

        f32 movementDirScreenY = screenYDir.dot(_398);
        f32 movementDirScreenZ = screenZDir.dot(_398);
        f32 absMovementDirScreenY = MR::abs(movementDirScreenY);
        f32 absMovementDirScreenZ = MR::abs(movementDirScreenZ);

        if (absMovementDirScreenZ > absMovementDirScreenY) {
            groundYDirOrtho = screenYDir;
            if (movementDirScreenZ < 0.0f) {
                groundYDirOrtho = -groundYDirOrtho;
            }

            groundXDir.cross(groundYDirOrtho, _398);
        } else {
            groundYDirOrtho = -screenZDir;
            if (movementDirScreenY < 0.0f) {
                groundYDirOrtho = -groundYDirOrtho;
                groundXDir.cross(screenZDir, _398);
            } else {
                groundXDir.cross(-screenZDir, _398);
            }
        }

        groundYDir.cross(screenXDir, _398);

        MR::normalizeOrZero(&groundXDir);
        if (MR::isNearZero(groundXDir)) {
            groundXDir = screenXDir;
        }

        MR::normalizeOrZero(&groundYDir);
        if (MR::isNearZero(groundYDir)) {
            groundYDir = groundYDirOrtho;
        }

        *pMoveDir = groundXDir * stickX - groundYDir * stickY;

        if (_10._13) {
            TVec3f radialDir = mPosition - _6F4;
            MR::vecKillElement(radialDir, _700, &radialDir);
            MR::normalizeOrZero(&radialDir);
            MR::vecKillElement(*pMoveDir, _700, pMoveDir);
            MR::vecKillElement(*pMoveDir, radialDir, pMoveDir);
        }
    }
}


bool Mario::checkLockOnHoming() {
    if (mStickPos.z != 0.0f) {
        return false;
    }

    if (!checkPreLvlZ()) {
        return false;
    }

    if (isSwimming()) {
        return false;
    }

    if (!mActor->_470) {
        return false;
    }

    mMovementStates._A = false;
    mDrawStates._5 = true;
    doLockOnHoming();
    return true;
}


void Mario::doLockOnHoming() {
    TVec3f direction;
    direction.set(mActor->_470->mPosition - mPosition);
    MR::normalize(&direction);
    TVec3f shadowDir;
    calcShadowDir(direction, &shadowDir);
    if (MR::diffAngleAbsHorizontal(mFrontVec, direction, mHeadVec) >= 0.05235988f) {
        if (getAnimator()->isAnimationStop()) {
            changeAnimation("その場足踏み");
            if (!_750) {
                setFrontVecKeepUp(direction, 15UL);
                _334 = direction;
            }
        }
    }
}


void Mario::fixPositionInTower() {
    if (_10._13) {
        TVec3f offset = mPosition - _6F4;
        f32 height = MR::vecKillElement(offset, _700, &offset);
        offset.setLength(_718 - 200.0f);
        offset += _700 * height;
        mPosition = _6F4 + offset;
    }
}

// ---- MarioMove25D.cpp ----

void Mario::set25Dmode(const AreaObj* pArea) {
    Mtx matrix;
    TVec3f rotation;
    MR::calcCubeRotate(pArea, &rotation);
    TVec3f normal(0.0f, 0.0f, 1.0f);
    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::makeMtxTR(matrix, 0.0f, 0.0f, 0.0f, rotation.x, rotation.y, rotation.z);
    PSMTXMultVec(matrix, &normal, &normal);
    PSMTXMultVec(matrix, &up, &up);
    _6D0 = normal;
    _6DC = up;
    _6E8.cross(_6D0, _6DC);
    MR::normalize(&_6E8);
    mMovementStates._3A = true;
}


void Mario::update25Dmode() {
    TVec3f up;
    MR::vecKillElement(-*getGravityVec(), _6D0, &up);
    MR::normalizeOrZero(&up);
    if (MR::isNearZero(up)) {
        _6AC = -getGravityVec()->dot(_6D0) > 0.0f ? 4 : 5;
        return;
    }

    f32 sideDot = up.dot(_6E8);
    f32 upDot = up.dot(_6DC);
    f32 normalDot = up.dot(_6D0);
    f32 sideAbs = MR::abs(sideDot);
    f32 upAbs = MR::abs(upDot);
    f32 normalAbs = MR::abs(normalDot);
    u8 mode;
    if (sideAbs > upAbs && sideAbs > normalAbs) {
        mode = sideDot > 0.0f ? 2 : 3;
    } else if (upAbs > sideAbs && upAbs > normalAbs) {
        if (upDot > 0.0f) {
            mode = 0;
        } else {
            mode = 1;
        }
    } else {
        mode = normalDot > 0.0f ? 4 : 5;
    }

    _6AD = mode;
    if (mode == 2 || mode == 3) {
        if (mode == 2 || mode == 3) {
            f32 threshold = 0.965f;
            if (_6AC == 2 || _6AC == 3) {
                threshold = 0.9397f;
            }

            if (sideAbs < threshold) {
                if (upAbs > normalAbs) {
                    if (upDot > 0.0f) {
                        mode = 0;
                    } else {
                        mode = 1;
                    }
                } else {
                    return;
                }
            }
        }
    }

    if (!isStickOn()) {
        _6AC = mode;
        mMovementStates._3B = false;
    } else if (!mMovementStates._3B && mStickPos.z > 0.7f) {
        _6C8.set(mStickPos.x, mStickPos.y);
        _6AC = mode;
        mMovementStates._3B = true;
    } else {
        TVec2f stick(mStickPos.x, mStickPos.y);
        if (MR::diffAngleAbs(_6C8, stick) >= 0.7853982f) {
            _6AC = mode;
            _6C8.set(mStickPos.x, mStickPos.y);
        }
    }
}


void Mario::updateAxisFromMode(u8 mode) {
    if (mMovementStates.jumping) {
        return;
    }

    TVec3f side;
    TVec3f normal;
    if (mode < 4) {
        TVec3f up;
        TVec3f planeUp;
        MR::vecKillElement(-*getGravityVec(), _6D0, &up);
        MR::vecKillElement(_6DC, _6D0, &planeUp);
        MR::normalizeOrZero(&up);
        MR::normalizeOrZero(&planeUp);
        f32 dot = up.dot(planeUp);
        if (MR::abs(dot) > 0.99f) {
            switch (mode) {
            case 0:
                side = _6E8;
                normal = _6D0;
                if (dot < 0.0f) {
                    side = -side;
                }

                break;
            case 1:
                side = _6E8;
                normal = _6D0;
                if (dot > 0.0f) {
                    side = -side;
                }

                break;
            case 2:
                side = _6D0;
                normal = -_6E8;
                if (dot < 0.0f) {
                    side = -side;
                    normal = -normal;
                }

                break;
            case 3:
                side = -_6D0;
                normal = _6E8;
                if (dot < 0.0f) {
                    side = -side;
                    normal = -normal;
                }

                break;
            }
        } else {
            TVec3f axis;
            axis.cross(up, planeUp);
            MR::normalize(&axis);
            if (axis.dot(_6D0) < 0.0f) {
                axis = -axis;
            }

            TVec3f tangent;
            tangent.cross(axis, up);
            MR::normalize(&tangent);
            switch (mode) {
            case 0:
                side = tangent;
                normal = axis;
                break;
            case 1:
                side = -tangent;
                normal = axis;
                break;
            case 2:
                side = axis;
                normal = -tangent;
                break;
            case 3:
                side = -axis;
                normal = tangent;
                break;
            }
        }
    }

    _6B0 = side;
    _6BC = -normal;
}


void Mario::calcMoveDir25D(f32 x, f32 y, TVec3f* pDirection) {
    *pDirection = _6B0 * x - _6BC * y;
}

// ---- MarioMove2D.cpp ----

void Mario::check2DMode() {
    mMovementStates._37 = false;
    _10._15 = false;

    AreaObj* area = MR::getAreaObj("PlaneModeCube", mPosition);
    if (area != nullptr) {
        mMovementStates._37 = true;

        TVec3f rotate;
        MR::calcCubeRotate(area, &rotate);
        MR::makeMtxTR(_F4, 0.0f, 0.0f, 0.0f, rotate.x, rotate.y, rotate.z);

        if (!(_20._37)) {
            _688 = mPosition;
            MR::getRotatedAxisZ(&_6A0, rotate);
            MR::calcCubePos(area, &_694);
        }
    } else {
        area = MR::getAreaObj("PlaneCircularModeCube", mPosition);
        if (area != nullptr) {
            TVec3f rotate;
            MR::calcCubeRotate(area, &rotate);
            PSMTXCopy(MR::tmpMtxRotYDeg(rotate.y), _F4);
            _10._15 = true;

            if (!((_28 >> 10) & 1)) {
                _688 = mPosition;
                MR::getRotatedAxisZ(&_6A0, rotate);
                MR::calcCubePos(area, &_694);
            }
        }
    }

    mMovementStates._3A = false;
    area = MR::getAreaObj("PipeModeCube", mPosition);
    if (area != nullptr) {
        set25Dmode(area);
    }
}


void Mario::calcMoveDir2D(f32 a1, f32 a2, TVec3f* pOut) {
    if (a1 != 0.0f) {
        a2 = 0.0f;
    }

    TVec3f stack_8C;
    TVec3f stack_80;
    TVec3f stack_74;
    TVec3f stack_68;
    TVec3f stack_5C;

    stack_8C = mSide2D;
    stack_80 = mUp2D;
    stack_74 = mNormal2D;

    TVec3f stack_50(_660);
    stack_68.cross(stack_74, stack_50);
    MR::normalize(&stack_68);

    if (stack_50.dot(stack_80) < 0.0f) {
        stack_68 = -stack_68;
    }

    stack_5C = -stack_80;
    TVec3f stack_44(_398);

    if (_398.dot(stack_50) <= 0.0f) {
        _398 = stack_50;
    } else {
        MR::vecBlendSphere(_398, stack_50, &_398, 0.1f);
    }

    if (MR::isNearZero(_398)) {
        _398 = stack_44;
    } else {
        MR::normalize(&_398);
    }

    *pOut = stack_68 * a1 - stack_5C * a2;
}


void Mario::calcShadowDir2D(const TVec3f& rVec, TVec3f* pOut) {
    *pOut = rVec;
    MR::normalizeOrZero(pOut);
}


void Mario::stick2Dadjust(f32& rStickX, f32& rStickY) {
    bool isNearHalfPi = false;

    if (_10.turning && mMovementStates._1 && !mMovementStates.jumping) {
        _10.turning = false;
        clear2DStick();
    }

    f32 stickXInput = rStickX;
    f32 stickYInput = rStickY;

    mSide2D.set(1.0f, 0.0f, 0.0f);
    mUp2D.set(0.0f, 1.0f, 0.0f);
    mNormal2D.set(0.0f, 0.0f, -1.0f);
    PSMTXMultVecSR(_F4, &mSide2D, &mSide2D);
    PSMTXMultVecSR(_F4, &mNormal2D, &mNormal2D);

    TVec3f stack_38(0.0f, 1.0f, 0.0f);
    TVec3f* camDirZ = &getCamDirZ();
    f32 angle = MR::diffAngleSignedHorizontal(getCamDirY(), stack_38, *camDirZ);

    TVec3f stack_2C(1.0f, 0.0f, 0.0f);
    camDirZ = &getCamDirZ();
    const f32 absHorizontal = MR::diffAngleAbsHorizontal(getCamDirX(), stack_2C, *camDirZ);
    if (absHorizontal >= 3.0415928f && MR::abs(angle) >= 3.0415928f) {
        angle = 0.0f;
        rStickX = -rStickX;
    }

    Mtx rotMtx;
    PSMTXRotAxisRad(rotMtx, &_6A0, angle);
    PSMTXMultVecSR(rotMtx, &mSide2D, &mSide2D);
    PSMTXMultVecSR(rotMtx, &mUp2D, &mUp2D);

    MR::vecKillElement(_368, _6A0, &_660);
    if (MR::normalizeOrZero(&_660)) {
        _660 = _368;
    }

    set2Dmode(_660.dot(mUp2D) >= 0.0f);

    bool isMainY = false;
    bool isMainX = false;

    if (_66C) {
        if (MR::isNearZero(mStickPos.z)) {
            _66C = false;
        } else {
            TVec3f stickVec(rStickX, rStickY, 0.0f);
            const f32 diff = MR::diffAngleAbs(stickVec, _670);

            if (_670.x * rStickX < 0.0f) {
                _66C = false;
            } else if (_670.y * rStickY < 0.0f) {
                _66C = false;
            } else if (diff < 0.3f) {
                rStickX = _67C.x;
                rStickY = _67C.y;
            } else {
                _66C = false;
            }
        }
    }

    if (!_66C && !MR::isNearZero(mStickPos.z) && !_60E && !_60F) {
        TVec3f stickVec(rStickX, rStickY, 0.0f);
        PSMTXMultVecSR(_F4, &stickVec, &stickVec);
        PSMTXMultVecSR(rotMtx, &stickVec, &stickVec);

        TVec3f invMove(-_660);
        const f32 diff = MR::diffAngleAbs(invMove, stickVec);
        f32 clamp = MR::clamp(1.0f - (mStickPos.z * mStickPos.z), 0.0f, 1.0f);

        const f32 threshold = MR::getInterpolateValue(clamp, 0.5f, 1.5f);
        if (diff < threshold) {
            rStickX = 0.0f;
            rStickY = 0.0f;
            mStickPos.z = 0.0f;
        } else if (diff > (3.1415927f - threshold)) {
            rStickX = 0.0f;
            rStickY = 0.0f;
            mStickPos.z = 0.0f;
        } else {
            _670 = stickVec;
            MR::vecKillElement(stickVec, _660, &stickVec);
            rStickX = stickVec.x;
            rStickY = stickVec.y;
            _67C = stickVec;
            _66C = true;
        }
    }

    if (MR::abs(rStickY) > MR::abs(rStickX)) {
        isMainY = true;
        rStickX = 0.0f;
        if (rStickY > 0.0f) {
            rStickY = mStickPos.z;
        } else {
            rStickY = -mStickPos.z;
        }
    } else {
        isMainX = true;
        rStickY = 0.0f;
        if (rStickX > 0.0f) {
            rStickX = mStickPos.z;
        } else {
            rStickX = -mStickPos.z;
        }
    }

    const f32 angleMove = MR::diffAngleAbs(_660, mUp2D);
    u8 lockX = 0;
    u8 lockY = 0;

    if (!((_60E != 0 || _60F != 0) && _610 == 0 && _611 == 0)) {
        bool doSecondBlock = false;

        if ((angleMove < 0.5235988f || angleMove > 2.617994f || _610 != 0) && _611 == 0) {
            if (_620 == 0.0f) {
                rStickY = 0.0f;
                if (isMainY && MR::abs(stickXInput) > 0.3f) {
                    rStickX = mStickPos.z;
                    if (stickXInput < 0.0f) {
                        rStickX = -rStickX;
                    }
                    lockX = 1;
                }
            }

        } else {
            if (!(angleMove > 1.3962635f && angleMove < 1.7453294f) && _611 == 0) {
            } else if (_610 == 0 && _61C == 0.0f) {
                rStickX = 0.0f;
                if (isMainX && MR::abs(stickYInput) > 0.3f) {
                    rStickY = mStickPos.z;
                    if (stickYInput < 0.0f) {
                        rStickY = -rStickY;
                    }
                    lockY = 1;
                }
            }
        }
    }

    _61C = rStickX;
    _620 = rStickY;
    _610 = lockX;
    _611 = lockY;

    if (!_60E && !_60F && rStickY != 0.0f) {
        if (MR::isInRange(angleMove, 0.19634955f, 1.5707964f)) {
            if (_660.dot(mSide2D) < 0.0f) {
                _60F = 1;
            } else {
                _60F = 2;
            }
        } else if (MR::isInRange(angleMove, 1.5707964f, 2.9452431f)) {
            if (_660.dot(mSide2D) < 0.0f) {
                _60F = 2;
            } else {
                _60F = 1;
            }
        }
    }

    switch (_60F) {
    case 1:
        if (rStickY == 0.0f) {
            _60F = 0;
        } else {
            rStickX = rStickY;
        }
        break;
    case 2:
        if (rStickY == 0.0f) {
            _60F = 0;
        } else {
            rStickX = -rStickY;
            rStickY = -rStickY;
        }
        break;
    default:
        _60F = 0;
        break;
    }

    switch (_60E) {
    case 1:
        if (rStickX > 0.0f) {
            rStickX = -rStickX;
        } else {
            _60E = 0;
        }
        break;
    case 2:
        if (rStickX < 0.0f) {
            rStickX = -rStickX;
        } else {
            _60E = 0;
        }
        break;
    default:
        _60E = 0;
        break;
    }

    _614 = rStickX;
    _618 = rStickY;

    if (MR::isNearZero(angleMove - 1.5707964f, 0.0001f)) {
        isNearHalfPi = true;
    }

    if (!isNearHalfPi) {
        rStickY = 0.0f;
        mStickPos.z = MR::abs(rStickX);
    } else {
        rStickX = 0.0f;
        mStickPos.z = MR::abs(rStickY);
    }
}


void Mario::set2Dmode(bool mode) {
    if (mode == _624) {
        return;
    }

    if (_614 > 0.0f) {
        _60E = 1;
    }

    if (_614 < 0.0f) {
        _60E = 2;
    }

    _624 = mode;
}


void Mario::beforeJumping2D() {
    _628 = _660;
}


void Mario::afterLanding2D() {
}


void Mario::clear2DStick() {
    _60E = 0;
    _60F = 0;
    _610 = 0;
    _611 = 0;
    _618 = 0.0f;
    _614 = 0.0f;
    _620 = 0.0f;
    _61C = 0.0f;
    _624 = false;
    _628.zero();
    _66C = false;
    _634 = 0.0f;
    _670.zero();
    _67C.zero();
}

// ---- MarioPress.cpp ----

bool Mario::checkPressDamage() {
    if (_5FC) {
        return false;
    }
    if (isSwimming()) {
        return false;
    }
    if (checkVerticalPress(false)) {
        if (isStatusActive(MarioStatus_Hang)) {
            mHang->forceDrop();
        } else {
            mActor->setPress(0, 0);
            if (mActor->_390) {
                return true;
            }
            _350.zero();
            _35C.zero();
            return true;
        }
    }
    if (checkSidePressPre()) {
        mActor->setPress(1, 0);
        return true;
    }
    if (checkSidePress()) {
        return true;
    }
    if (checkWallFloorCode(0x1D) && mMovementStates._1 && getFloorCode() != 0x1D) {
        mActor->_3B4 = _368;
        mActor->setPress(4, 120);
        return true;
    }
    if (_72C < 200.0f) {
        if (mMovementStates._8 && (!mMovementStates.jumping || !(getFrontWallNorm().dot(getAirGravityVec()) > 0.707f))) {
            if (_72C < 100.0f) {
                _72C = 100.0f;
            }
            addVelocity(getFrontWallNorm() * (_4E4 * (200.0f / _72C) - _4E4));
        }
        if (mMovementStates._1A && (!mMovementStates.jumping || !(getSideWallNorm().dot(getAirGravityVec()) > 0.707f))) {
            if (_72C < 100.0f) {
                _72C = 100.0f;
            }
            addVelocity(getSideWallNorm() * (60.0f * (200.0f / _72C) - 60.0f));
        }
    }
    return false;
}


bool Mario::checkVerticalPress(bool atCurrentPosition) {
    if (mMovementStates.debugMode) {
        return false;
    }
    TVec3f position;
    if (isStatusActive(MarioStatus_Hang)) {
        position = mActor->_2A0;
    } else {
        position = mPosition;
    }
    if (!atCurrentPosition && !mActor->_390) {
        position += mVelocity;
    }
    f32 height;
    if (mMovementStates._A) {
        height = 80.0f;
    } else {
        height = 160.0f;
    }
    bool nearCeiling = false;
    TVec3f up(-*getGravityVec());
    f32 distance = 0.0f;
    Triangle ceiling;
    Triangle floor;
    TVec3f hitPosition;
    bool hit = MR::getFirstPolyOnLineToMap(&hitPosition, &ceiling, position - up * 10.0f, up * height);
    if (hit) {
        distance = up.dot(hitPosition - position);
        if (distance < 150.0f) {
            nearCeiling = true;
        }
    }
    if (hit) {
        TVec3f origin;
        if (nearCeiling) {
            origin = hitPosition;
        } else {
            origin = position;
        }
        if (MR::getFirstPolyOnLineToMap(&hitPosition, &floor, origin - *ceiling.getNormal(0) * 10.0f, *ceiling.getNormal(0) * height)) {
            if (ceiling.mSensor != floor.mSensor && (MR::isSensorPressObj(ceiling.mSensor) || MR::isSensorPressObj(floor.mSensor))) {
                if (!atCurrentPosition && !mActor->_390) {
                    if (!checkVerticalPress(true)) {
                        mVelocity.zero();
                        return false;
                    }
                    return true;
                }
                mActor->_3B4 = _368;
                return true;
            }
        } else if (!atCurrentPosition && MR::isSensorPressObj(ceiling.mSensor)) {
            mDrawStates._1E = true;
            TVec3f displacement;
            if (distance < 0.0f) {
                displacement = hitPosition - position;
                displacement.setLength(height - distance);
            } else {
                displacement = position - hitPosition;
                displacement.setLength(150.0f - distance);
            }
            TVec3f pushDirection;
            bool blended = MR::vecBlendSphere(*ceiling.getNormal(0), *floor.getNormal(0), &pushDirection, 0.5f);
            if (!mActor->_390) {
                if (mMovementStates._1) {
                    if (mGroundPolygon->mSensor == ceiling.mSensor) {
                        if (blended) {
                            push(pushDirection * displacement.length());
                        }
                    } else {
                        mActor->_F48 = ceiling.mSensor;
                        mActor->_3B4 = _368;
                        return true;
                    }
                } else if (blended) {
                    push(pushDirection * displacement.length());
                }
            }
        }
    }
    return false;
}


bool Mario::checkSidePressPre() {
    if (_5FC) {
        return false;
    }
    if (mMovementStates._8 && mMovementStates._19) {
        if ((_4F4 - _4E8).length() < 100.0f && mFrontWallTriangle->mSensor != mBackWallTriangle->mSensor &&
            (MR::isSensorPressObj(mFrontWallTriangle->mSensor) || MR::isSensorPressObj(mBackWallTriangle->mSensor))) {
            if (isDossun(mFrontWallTriangle) || isDossun(mBackWallTriangle)) {
                return false;
            }
            if (mFrontWallTriangle->getNormal(0)->dot(*mBackWallTriangle->getNormal(0)) < -0.5f) {
                mActor->_3B4 = getWallNorm();
                return true;
            }
        }
    } else if (isStatusActive(MarioStatus_Hang) && mMovementStates._19 && _8C8->isValid() && _8C8->mSensor != mBackWallTriangle->mSensor &&
               (MR::isSensorPressObj(_8C8->mSensor) || MR::isSensorPressObj(mBackWallTriangle->mSensor))) {
        if (isDossun(_8C8) || isDossun(mBackWallTriangle)) {
            return false;
        }
        TVec3f horizontal;
        MR::vecKillElement(mPosition - _4F4, getAirGravityVec(), &horizontal);
        if (horizontal.length() < 100.0f) {
            mActor->_3B4 = getWallNorm();
            return true;
        }
    }
    return false;
}
#pragma opt_propagation off
bool Mario::checkSidePress() {
    TVec3f position;
    if (isStatusActive(MarioStatus_Hang)) {
        position = mActor->_2A0;
    } else {
        position = mPosition;
    }
    f32 radius;
    f32 height;
    if (mMovementStates._A) {
        radius = 50.0f;
        height = 40.0f;
    } else {
        radius = 80.0f;
        height = 70.0f;
    }
    f32 ceilingDistance = calcDistToCeil(false);
    TVec3f center(position + mHeadVec * height);
    u32 hitCount = Collision::checkStrikeBallToMap(center, radius, nullptr, nullptr);
    if (hitCount < 2) {
        return false;
    }
    for (u32 i = 0; i < hitCount; i++) {
        const HitInfo* first = Collision::getStrikeInfoMap(i);
        TVec3f firstNormal(*first->mParentTriangle.getNormal(0));
        for (u32 j = i + 1; j < hitCount; j++) {
            const HitInfo* second = Collision::getStrikeInfoMap(j);
            TVec3f secondNormal(*second->mParentTriangle.getNormal(0));
            if (firstNormal.dot(secondNormal) >= -0.707f) {
                continue;
            }
            if ((first->_64 - center).dot(second->_64 - center) > 0.0f) {
                continue;
            }
            f32 separation = (first->_64 - second->_64).length();
            TVec3f firstVelocity;
            TVec3f secondVelocity;
            MR::calcVelocityMovingPoint(&first->mParentTriangle, first->_64, &firstVelocity);
            MR::calcVelocityMovingPoint(&second->mParentTriangle, second->_64, &secondVelocity);
            if (!MR::isNearZero(firstVelocity) || !MR::isNearZero(secondVelocity)) {
                if (!MR::isNearZero(firstVelocity)) {
                    MR::normalize(&firstVelocity);
                    if (firstVelocity.dot(firstNormal) < 0.2f || firstVelocity.dot(secondNormal) > -0.2f) {
                        continue;
                    }
                }
                if (!MR::isNearZero(secondVelocity)) {
                    MR::normalize(&secondVelocity);
                    if (secondVelocity.dot(secondNormal) < 0.2f || secondVelocity.dot(firstNormal) > -0.2f) {
                        continue;
                    }
                }
                if (first->mParentTriangle.getSensor() != second->mParentTriangle.getSensor() &&
                    (MR::isSensorPressObj(first->mParentTriangle.getSensor()) || MR::isSensorPressObj(second->mParentTriangle.getSensor()))) {
                    if (isStatusActive(MarioStatus_Hang)) {
                        mMovementStates._1 = false;
                        closeStatus(mHang);
                    }
                    if (!mMovementStates._1 && MR::abs(firstNormal.dot(*getGravityVec())) < 0.707f) {
                        if (!first->isCollisionAtFace()) {
                            TVec3f horizontal;
                            f32 vertical = MR::vecKillElement(first->_64 - center, *getGravityVec(), &horizontal);
                            if (vertical != 0.0f) {
                                addVelocity(*getGravityVec() * -vertical);
                                mDrawStates._1E = true;
                                continue;
                            }
                        }
                        if (!second->isCollisionAtFace()) {
                            TVec3f horizontal;
                            f32 vertical = MR::vecKillElement(second->_64 - center, *getGravityVec(), &horizontal);
                            if (vertical != 0.0f) {
                                addVelocity(*getGravityVec() * -vertical);
                                mDrawStates._1E = true;
                                continue;
                            }
                        }
                    }
                    if (!isStatusActive(MarioStatus_Bury) && !mSinkTimer && ceilingDistance >= 180.0f) {
                        TVec3f firstOffset(first->_64 - center);
                        TVec3f secondOffset(center - second->_64);
                        if (!MR::isNearZero(MR::diffAngleAbs(firstOffset, secondOffset), 0.017453292f)) {
                            MR::normalizeOrZero(&firstOffset);
                            MR::normalizeOrZero(&secondOffset);
                            addVelocity(-firstOffset * 5.0f);
                            addVelocity(secondOffset * 5.0f);
                            continue;
                        }
                    }
                    f32 angle = JMAAcosRadian(-firstNormal.dot(secondNormal));
                    if (mActor->selectTiltPress(first->mParentTriangle.getSensor()) || mActor->selectTiltPress(second->mParentTriangle.getSensor())) {
                        angle = 0.0f;
                    }
                    if (!isStatusActive(MarioStatus_Bury) && !mSinkTimer && angle > 0.34906587f) {
                        f32 distance = (160.0f - ceilingDistance) / JMASinRadian(angle);
                        if (!(distance < 0.0f)) {
                            TVec3f firstHorizontal;
                            TVec3f secondHorizontal;
                            MR::vecKillElement(firstNormal, *getGravityVec(), &firstHorizontal);
                            MR::vecKillElement(secondNormal, *getGravityVec(), &secondHorizontal);
                            if (firstHorizontal.length() > secondHorizontal.length()) {
                                firstHorizontal.setLength(distance);
                                addVelocity(firstHorizontal);
                            } else {
                                secondHorizontal.setLength(distance);
                                addVelocity(secondHorizontal);
                            }
                        }
                        continue;
                    }
                    s32 previousFloorCode = _95C->getCode(_45C);
                    if (previousFloorCode != _95C->getCode(mGroundPolygon) && MR::isSensorPressObj(mGroundPolygon->mSensor) &&
                        mVerticalSpeed >= 200.0f) {
                        mDrawStates._A = true;
                        TVec3f horizontal;
                        MR::vecKillElement(mShadowPos - mGroundPos, getAirGravityVec(), &horizontal);
                        horizontal.setLength(10.0f);
                        addVelocity(horizontal);
                        return false;
                    }
                    if (MR::abs(firstNormal.dot(mHeadVec)) > 0.707f) {
                        mActor->setPress(0, 0);
                    } else {
                        mActor->setPress(1, 0);
                    }
                    mActor->_3B4 = firstNormal;
                    return true;
                }
            }
            if (getPlayer()->mMovementStates.jumping) {
                MR::diffAngleAbsHorizontal(firstNormal, secondNormal, getAirGravityVec());
                f32 normalDot = firstNormal.dot(secondNormal);
                TVec3f tangent;
                PSVECCrossProduct(&firstNormal, &getAirGravityVec(), &tangent);
                MR::normalizeOrZero(&tangent);
                TVec3f firstUp = tangent.cross(firstNormal);
                MR::normalizeOrZero(&firstUp);
                PSVECCrossProduct(&secondNormal, &getAirGravityVec(), &tangent);
                MR::normalizeOrZero(&tangent);
                TVec3f secondUp = tangent.cross(secondNormal);
                MR::normalizeOrZero(&secondUp);
                TVec3f firstPosition;
                TVec3f secondPosition;
                firstPosition = first->_64 + firstUp * height;
                secondPosition = second->_64 + secondUp * height;
                f32 distance = (firstPosition - secondPosition).length();
                if (isRising()) {
                    if (normalDot < -0.707f && distance < 120.0f && separation < 100.0f) {
                        cutGravityElementFromJumpVec(true);
                    }
                } else if (normalDot < -0.707f && separation < 100.0f && distance < 80.0f) {
                    cutGravityElementFromJumpVec(true);
                    mMovementStates._1 = true;
                    stopJump();
                }
            }
        }
    }
    return false;
}

// ---- MarioRabbit.cpp ----

void Mario::startRabbitMode() {
    if (!isStatusActive(MarioStatus_Rabbit)) {
        changeStatus(mRabbit);
    }
}


void Mario::endRabbitMode() {
    if (isStatusActive(MarioStatus_Rabbit)) {
        closeStatus(mRabbit);
    }
}

// ---- MarioRecovery.cpp ----

bool Mario::doRecovery() {
    if (isStatusActive(MarioStatus_Foo)) {
        return false;
    }
    if (isStatusActive(MarioStatus_Recovery)) {
        return false;
    }
    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }
    if (mActor->_934) {
        return false;
    }
    if (mActor->_EA4) {
        return false;
    }

    cancelSquatMode();
    stopWalk();
    if (mRecovery->_12) {
        changeStatus(mRecovery);
        stopJump();
    } else {
        TVec3f gravity;
        TVec3f position(*getLastSafetyTrans(&gravity));
        position += gravity * 160.0f;
        TVec3f direction(mPosition - position);
        MR::normalizeOrZero(&direction);
        doPointWarpRecovery(position, direction);
    }
    return true;
}

// ---- MarioSideStep.cpp ----

bool Mario::checkWallJumpCode() {
    if (checkWallFloorCode(11) || checkWallFloorCode(6) || checkWallFloorCode(34) || checkWallFloorCode(7) || checkWallFloorCode(8)) {
        if (mMovementStates._1 || isAnimationRun(_728)) {
            stopWalk();
            push(getWallNorm() * 10.0f);
            tryPushToVelocity();
            mJumpVec.set(-getPlayer()->getWallNorm() * 10.0f);
            MR::vecKillElement(mJumpVec, getAirGravityVec(), &mJumpVec);
            mJumpVec -= getAirGravityVec() * 10.0f;
        }
        mWall->initTriangleJump();
        u32 jumpType = -1;
        if (checkWallFloorCode(7)) {
            jumpType = 1;
        }
        if (checkWallFloorCode(6)) {
            jumpType = 0;
        }
        if (checkWallFloorCode(34)) {
            jumpType = 2;
        }
        if (mWall->startBackJump(jumpType)) {
            return true;
        }
    }
    return false;
}


bool Mario::doSideStep() {
    if (getCurrentStatus() == MarioStatus_SideStep) {
        return false;
    }
    if (checkWallCode("NotSideStep", false)) {
        return false;
    }
    if (checkWallCode("NoAction", false)) {
        return false;
    }
    if (mActor->_468) {
        return false;
    }
    if (_1C._5) {
        return false;
    }
    if (_72C < 200.0f) {
        return false;
    }
    if (getPlayerMode() == PlayerMode_Hopper) {
        return false;
    }
    if (calcPolygonAngleD(mFrontWallTriangle) > 91.0f) {
        return false;
    }
    f32 angle = MR::diffAngleAbs(getFrontWallNorm(), _368);
    if (angle >= 1.6534699f || angle < 1.4959966f) {
        return false;
    }
    bool cannotSideStep = !isSlipFloorCode(_960);
    if (cannotSideStep) {
        return false;
    }
    if (_960 == 2) {
        return false;
    }
    if (!getPlayer()->mMovementStates._8) {
        return false;
    }
    const Triangle* wall = getPlayer()->getWallPolygon();
    if (!wall) {
        return false;
    }
    if (!MR::isSameMtx(wall->getBaseMtx()->toMtxPtr(), wall->getPrevBaseMtx()->toMtxPtr())) {
        return false;
    }
    stopWalk();
    forceStopTornado();
    changeStatus(mSideStep);
    return true;
}

// ---- MarioSkate.cpp ----

void MarioSkate_FORCE_MATCH_SDATA2() {
    (void)20.0f;
    (void)50.0f;
    (void)1.25f;
    (void)1.5f;
    (void)1.2f;
    (void)0.04f;
    (void)0.1f;
    (void)-0.1f;
    (void)-0.3926991f;
    (void)0.3926991f;
    (void)-0.5235988f;
    (void)0.5235988f;
    (void)0.8f;
    (void)0.9f;
    (void)0.95f;
    (void)0.05f;
}


bool Mario::isSkatableFloor() const {
    if (_960 == CollisionFloorCode_Ice) {
        return true;
    }

    return _960 == CollisionFloorCode_GlassIce;
}


bool Mario::doSkate() {
    changeStatus(mSkate);
    return true;
}

// ---- MarioSlider.cpp ----

bool Mario::isSliderFloor() const {
    return _960 == 9;
}


bool Mario::checkSliderMode() const {
    if (!mMovementStates.jumping && mMovementStates._1) {
        if (isSliderFloor()) {
            return true;
        }
    }

    return false;
}


void Mario::startSlider() {
    if (!isStatusActive(mSlider->mStatusId)) {
        if (getPlayer()->mMovementStates._B) {
            if (getPlayer()->mMovementStates.jumping) {
                mDrawStates._14 = true;
                playSound("尻ドロップ着地");
                playSound("声尻ドロップ着地");
                playEffectRT("属性尻ドロップ", _368, mPosition);
                startPadVib("最強");
                startCamVib(0);
                MR::removeAllClingingKarikari();
            }
        }

        stopWalk();
        stopJump();
        changeStatus(mSlider);
    }
}

// ---- MarioSlip.cpp ----

void Mario::clearSlope() {
    _8F0 = 0.0f;
    mMovementStates._23 = false;
    _8F8.zero();
    _280 = 0.0f;
    _910.zero();
    _284.zero();
    _3C2 = 0;
    _2C4.zero();
    _3C4 = 0;
    _904.zero();
}


bool Mario::isEnableSlopeMove() const {
    const bool isNotSlipFloor = !isSlipFloorCode(_960);
    if (isNotSlipFloor) {
        if (!mMovementStates._23) {
            if (MR::isNearZero(_8F8)) {
                return false;
            }
        }
    }

    const s32 status = static_cast< s32 >(getCurrentStatus());
    switch (status) {
    case 0:
    case 2:
    case 3:
    case 4:
        return true;

    case 1:
    default:
        return false;
    }
}


void Mario::slopeMove() {
    if (mActor->mHealth == 0) {
        return;
    }

    if (!mMovementStates._1) {
        return;
    }

    f32 blend;
    f32 blendRate = MR::clamp(mWalkSpeed, 0.0f, 1.0f);

    TVec3f stack_188;
    TVec3f stack_17C;
    TVec3f stack_170;
    stack_170.cross(_368, getAirGravityVec());
    MR::normalizeOrZero(&stack_170);

    if (MR::isNearZero(stack_170)) {
        stack_17C.zero();
        _904.zero();
    } else {
        stack_17C.cross(_368, stack_170);
        MR::normalize(&stack_17C);
        _904 = stack_17C;
    }

    if (MR::isSameDirection(stack_17C, getAirGravityVec()) || MR::isOppositeDirection(stack_17C, getAirGravityVec())) {
        mMovementStates._23 = false;
        _904.zero();
        stack_17C.zero();
    }

    f32 slopeAngle = calcAngleD(_368);

    if (mDrawStates._17 && slopeAngle >= 30.0f) {
        if (!mMovementStates._23) {
            _8F8 = -_2C4 * 2.0f;
            mMovementStates._23 = true;
            _3C4 = 30;
            _8F0 = 10.0f;
        } else if (!mMovementStates._23) {
            _8F0 = 10.0f;
        }
    }

    if (_3C4 > 0xF) {
        _8F8 += -_2C4 * 0.5f;
    }

    if (!mDrawStates._C || !isSlipPolygon(_45C)) {
        blendRate = 1.0f;
        if (mMovementStates._23) {
            if (slopeAngle >= 15.0f) {
                _8F0 -= 0.4f;
            } else {
                _8F0 -= 0.8f;
            }

            if (_8F0 < 0.0f) {
                _8F0 = 0.0f;
                mMovementStates._23 = false;
            }

            mVelocity.zero();
        } else {
            _8F0 = 0.0f;
        }
    } else {
        if (!mMovementStates._23) {
            if (slopeAngle >= getActor()->getConst().getTable()->mSlipAngle) {
                TVec3f stack_164;
                if (MR::vecKillElement(mVelocity, stack_17C, &stack_164) < 0.0f) {
                    _8F8 = mVelocity;
                } else {
                    _8F8.zero();
                }

                mVelocity.zero();
                mMovementStates._23 = true;
                _8F0 = 10.0f;
                return;
            }

            TVec3f stack_158;
            const f32 sideSpeed = MR::vecKillElement(mVelocity, stack_17C, &stack_158);

            if (mWalkSpeed < 0.1f) {
                _8F0 = 10.0f;
            }

            if (mTargetWalkSpeedIndex == 0 && !isAnimationRun("すべり着地")) {
                _8F0 = 10.0f;
            }

            if (slopeAngle >= 25.0f && isStickOn()) {
                const TVec3f& worldPadDir = getWorldPadDir();

                TVec3f stack_14C;
                MR::vecKillElement(stack_17C, getCamDirY(), &stack_14C);
                if (MR::isNearZero(stack_14C)) {
                    MR::vecKillElement(stack_17C, getCamDirZ(), &stack_14C);
                }

                MR::normalize(&stack_14C);
                if (stack_14C.dot(worldPadDir) < 0.0f && mFrontVec.dot(stack_14C) < 0.0f) {
                    mDrawStates._4 = true;
                }
            }

            if (!mDrawStates._4) {
                _8F0 += 0.3f * (slopeAngle / 90.0f);
            }

            if (_8F0 >= 10.0f) {
                if (MR::vecKillElement(mActor->getLastMove(), stack_17C, &stack_158) < 0.0f) {
                    _8F8 = mActor->getLastMove();
                } else {
                    _8F8.zero();
                }

                mMovementStates._23 = true;
                _8F0 = 10.0f;
                mVelocity.zero();
                mDrawStates._4 = false;
            } else if (!mDrawStates._4 && sideSpeed > 0.0f) {
                const f32 t = _8F0 / 10.0f;
                const f32 remain = 1.0f - t;

                mVelocity = stack_158 + stack_17C * sideSpeed * remain;
            }

            blendRate *= 1.0f - (_8F0 / 10.0f);

            if (mDrawStates._4) {
                _3FE++;

                if (_3FE > getActor()->getConst().getTable()->mSlopeDashAccelTime) {
                    _3FE = getActor()->getConst().getTable()->mSlopeDashAccelTime;
                }

                _8F4 = (slopeAngle / getActor()->getConst().getTable()->mSlopeDashAngleFactor) *
                       (static_cast< f32 >(_3FE) / static_cast< f32 >(getActor()->getConst().getTable()->mSlopeDashAccelTime));
                _8F4 = 1.0f + (_8F4 * getActor()->getConst().getTable()->mSlopeDashSpeedFactor);

                const f32 accel = 1.0f + ((_8F4 - 1.0f) * getActor()->getConst().getTable()->mSlopeDashAccelFactor);
                mVelocity += -_904 * accel;

                if (isAnimationRun("がんばり走り")) {
                    stopAnimation(nullptr);
                }
            }
        }
    }

    MR::vecKillElement(_8F8, *mGroundPolygon->getNormal(0), &_8F8);

    if (mMovementStates._23) {
        _71E = 0;

        const f32 moveAngle = calcAngleD(_368);
        if (mDrawStates._C) {
            MR::vecKillElement(mVelocity, stack_17C, &mVelocity);
            mVelocity += _8F8;

            _8F8 += -stack_17C * slopeAngle * getActor()->getConst().getTable()->mSlopeAccel;
        } else {
            TVec3f stack_140;
            stack_140 = _8F8;
            if (!MR::normalizeOrZero(&stack_140)) {
                MR::vecKillElement(mVelocity, stack_140, &mVelocity);
            }

            mVelocity += _8F8;
        }

        if (MR::isNearZero(_8F8)) {
            if (!mDrawStates._C) {
                mMovementStates._23 = false;
                return;
            }
        }

        if (moveAngle > 0.0f) {
            const TVec3f& airGravityVec = getAirGravityVec();
            TVec3f stack_134;
            MR::vecKillElement(-stack_17C, airGravityVec, &stack_134);

            if (MR::isNearZero(stack_17C) || (MR::isNearZero(stack_134) && !mDrawStates._C)) {
                mMovementStates._23 = false;
                return;
            }

            if (MR::isNearZero(stack_134)) {
            }

            MR::normalize(&stack_134);

            const f32 speed = _8F8.length();
            MR::normalizeOrZero(&_8F8);

            const f32 ratio = MR::clamp(moveAngle / 30.0f, 0.0f, 1.0f);
            blend = ratio * getActor()->getConst().getTable()->mSlopeCurveAssist;

            MR::vecBlendSphere(_8F8, -stack_17C, &_8F8, blend);
            _8F8.setLength(speed);
        }

        if (MR::isNearZero(_8F8)) {
            mMovementStates._23 = false;
            moveSlopeSlide();
            return;
        }

        if (!(_8F8.dot(stack_17C) > 0.0f)) {
            if (stack_17C.dot(-*getGravityVec()) > 0.05f) {
                TVec3f stack_128;
                MR::vecKillElement(mFrontVec, _368, &stack_128);
                MR::normalize(&stack_128);

                TVec3f stack_11C(_8F8);
                MR::normalize(&stack_11C);

                f32 turnDot = MR::cos(1.0471976f);
                if (isAnimationRun("坂すべり上向きうつぶせ", 2)) {
                    turnDot = MR::cos(1.4959966f);
                }

                if (isAnimationRun("スケーティング") && _910.dot(stack_17C) > 0.0f) {
                    if (_910.length() > 2.0f) {
                        _8F8 = _910;
                    }

                    _910.zero();
                }

                if (stack_128.dot(stack_17C) < turnDot) {
                    setFrontVecKeepUp(stack_11C, 0.1f);
                    if (!isAnimationRun("坂すべり上向きうつぶせ", 2)) {
                        changeAnimation("坂すべり下向きあおむけ", 3);
                    }

                    if (isStickOn() && _16C.dot(stack_11C) > 0.0f &&
                        MR::diffAngleAbsHorizontal(stack_11C, mFrontVec, *getGravityVec()) < 0.5235988f) {
                        setFrontVecKeepUpAngle(_16C, getActor()->getConst().getTable()->mSlipMoveTurnAngleRad);
                    }
                } else {
                    if (!isAnimationRun("坂すべり下向きあおむけ", 3)) {
                        changeAnimation("坂すべり上向きうつぶせ", 2);
                    }

                    if (_3C2 < 5) {
                        setFrontVecKeepUp(-stack_11C, 0.5f);
                    } else {
                        setFrontVecKeepUp(-stack_11C, 0.1f);
                    }

                    if (_3C2 > 10 && isStickOn() && _16C.dot(stack_11C) > 0.0f) {
                        const TVec3f& gravityVec = *getGravityVec();
                        if (MR::diffAngleAbsHorizontal(-stack_11C, mFrontVec, gravityVec) < 0.5235988f) {
                            setFrontVecKeepUpAngle(-_16C, getActor()->getConst().getTable()->mSlipMoveTurnAngleRad);
                        }
                    }
                }
            } else if (mDrawStates._C) {
                if (!isAnimationRun("スケーティング")) {
                    _910 = _8F8;
                    _8F8.zero();
                }

                changeAnimation("スケーティング");
            }
        }

        if (isStickOn() && !MR::isNearZero(stack_17C) && _3C4 == 0) {
            const TVec3f& worldPadDir = getWorldPadDir();

            TVec3f stack_110(stack_17C);
            MR::vecKillElement(stack_17C, getCamDirY(), &stack_17C);
            if (MR::isNearZero(stack_17C)) {
                MR::vecKillElement(stack_110, getCamDirZ(), &stack_17C);
            }

            MR::normalize(&stack_17C);
            if (stack_17C.dot(worldPadDir) > 0.15f) {
                playEffect("共通スリップ坂制動");
                _3D0 = getActor()->getConst().getTable()->mTurnSlipTime;
            }
        }

        _1A8.x *= 0.9f;
        _1A8.y *= 0.9f;
        _1A8.z *= 0.9f;

        TVec3f stack_104;
        if (MR::vecKillElement(_1A8, stack_17C, &stack_104) >= 0.0f) {
            stack_104 = _1A8;
        }

        addVelocity(stack_104);
    } else {
        mVelocity += _8F8;

        if (isStickOn() && _3C4 == 0) {
            _8F8.scale(getActor()->getConst().getTable()->mSlopeCancelInertia);
        } else {
            _8F8.scale(getActor()->getConst().getTable()->mSlopeFinishInertia);
        }

        const MarioConstTable* table = getActor()->getConst().getTable();
        if (_8F8.length() < table->mSlopeAnimeFinishSpeed) {
            if (isAnimationRun("坂すべり上向きうつぶせ", 2)) {
                changeAnimation("坂すべり上向き終了");
            }

            if (isAnimationRun("坂すべり下向きあおむけ", 3)) {
                changeAnimation("坂すべり下向き終了");
            }

            if (isAnimationRun("スケーティング")) {
                stopAnimation("スケーティング");
            }

            if (isStickOn() && _8F0 == 0.0f) {
                if (isAnimationRun("坂すべり上向き終了")) {
                    stopAnimation(nullptr);
                } else if (isAnimationRun("坂すべり下向き終了")) {
                    stopAnimation(nullptr);
                }
            }
        }
    }

    const MarioConstTable* table = getActor()->getConst().getTable();
    f32 slopeSpeedMax = table->mSlopeSpeedMax;
    if (_3D0 != 0) {
        slopeSpeedMax = table->mSlopeSpeedMaxBraking;
    }

    const f32 slopeSpeed = _8F8.length();
    if (slopeSpeed > slopeSpeedMax) {
        _8F8.setLength((0.8f * slopeSpeed) + (0.2f * slopeSpeedMax));
    }

    if (!MR::vecBlendSphere(*getGravityVec(), -_368, &stack_188, blendRate)) {
        mMovementStates._23 = false;
        tryDrop();
        return;
    }

    if (mMovementStates._8) {
        TVec3f stack_F8;
        if (MR::vecKillElement(mVelocity, mFrontVec, &stack_F8) > 0.0f) {
            mVelocity = stack_F8;
        }
    }

    moveSlopeSlide();
}


void Mario::moveSlopeSlide() {
    if (calcAngleD(_368) < 5.8f) {
        if (mDrawStates._C) {
            const TVec3f* pWorldPadDir = &getWorldPadDir();
            f32 speed = _16C.length();
            if (_910.length() < speed) {
                _910 = _16C;
            }

            _910 += *pWorldPadDir * 0.5f;

            if (_910.length() > 15.0f) {
                _910.setLength(15.0f);
            }

            addVelocity(_910);
        }
    } else if (isAnimationRun("スケーティング") || isAnimationRun("坂すべり上向きうつぶせ", 2) || isAnimationRun("坂すべり下向きあおむけ", 3)) {
        TVec3f stack_20;
        stack_20.cross(getAirGravityVec(), _368);
        MR::normalizeOrZero(&stack_20);

        const TVec3f* pWorldPadDir = &getWorldPadDir();
        if (MR::isNearZero(_280)) {
            _284 = stack_20;
        }

        if (isStickOn() && _3C4 == 0) {
            f32 dot = _284.dot(*pWorldPadDir);
            if (MR::abs(dot) > 0.2f) {
                _280 = (_280 * mActor->getConst().getTable()->mSlopeSideMoveInertia) +
                       (dot * (1.0f - mActor->getConst().getTable()->mSlopeSideMoveInertia));
            } else {
                MarioConstTable* pConstTable = mActor->getConst().getTable();
                _280 = _280 * pConstTable->mSlopeSideStopInertia;
            }
        } else {
            MarioConstTable* pConstTable = mActor->getConst().getTable();
            _280 = _280 * pConstTable->mSlopeSideStopInertia;
        }

        TVec3f stack_14;
        stack_14.cross(_284, _368);
        _284.cross(_368, stack_14);

        MarioConstTable* pConstTable = mActor->getConst().getTable();
        addVelocity(_284, _280 * pConstTable->mSlopeSideMoveSpeed);
    }
}


bool Mario::taskOnSlipTurn(u32) {
    if (!isAnimationRun("ターンブレーキ滑り床")) {
        setFrontVecKeepUp(-_220);
        _754 = 0;
        _74C = 0.0f;
        return false;
    }

    _2B8.scale(mActor->mConst->getTable()->mInertiaSlipTurn);
    addVelocity(_2B8);
    return true;
}

// ---- MarioSlope.cpp ----

void Mario::lockGroundCheck(void* owner, bool simple) {
    _574 = owner;
    _10._5 = simple;
    _10._4 = true;
    mMovementStates._36 = simple;
}


void Mario::unlockGroundCheck(void* owner) {
    if (!owner || _574 == owner) {
        _574 = nullptr;
        _10._4 = false;
    }
}


bool Mario::isUseSimpleGroundCheck() const {
    if (_10._4) {
        return _10._5;
    }
    if (mMovementStates._23) {
        return true;
    }
    if (isPlayerModeTeresa()) {
        return true;
    }
    if (!mMovementStates._1) {
        return mMovementStates._23;
    }
    if (calcAngleD(_368) > mActor->getConst().getTable()->mFlatAngle - 5.0f) {
        Triangle ground;
        if (MR::getFirstPolyOnLineBFast(mPosition - *getGravityVec() * 30.0f, *getGravityVec() * 100.0f, nullptr, &ground)) {
            if (calcAngleD(*ground.getNormal(0)) < mActor->getConst().getTable()->mFlatAngle - 5.0f) {
                return false;
            }
        }
        return true;
    }
    getAnimator()->isLandingAnimationRun();
    return false;
}


bool Mario::checkGroundOnSlope() {
    TVec3f normal;
    if (isAnimationRun("崖ふんばり")) {
        normal = -*getGravityVec();
    } else {
        normal = *_45C->getNormal(0);
    }
    if (MR::isNearZero(normal)) {
        return false;
    }
    TVec3f front;
    MR::vecKillElement(mFrontVec, normal, &front);
    if (MR::isNearZero(front)) {
        return false;
    }
    Triangle ground;
    bool hit;
    bool missingGround = false;
    mMovementStates._14 = true;
    f32 distance;
    if (mMovementStates.jumping && isRising()) {
        distance = 10.0f;
    } else if (_414) {
        distance = 20.0f;
    } else {
        distance = 60.0f;
    }
    if (mMovementStates._1) {
        f32 angle = calcAngleD(_368) - 30.0f;
        if (angle < 0.0f) {
            angle = 0.0f;
        }
        if (angle > 40.0f) {
            angle = 40.0f;
        }
        distance += 2.0f * angle;
    }
    if (getCurrentStatus() == MarioStatus_Slider) {
        distance = 100.0f;
    }
    s32 groundCount = 0;
    TVec3f hitPosition;
    TVec3f positionSum;
    positionSum.zero();
    TVec3f groundPosition;
    TVec3f origin(mPosition - *getGravityVec() * 30.0f);
    hit = MR::getFirstPolyOnLineBFast(origin, *getGravityVec() * distance, &hitPosition, &ground);
    if (hit) {
        if (calcAngleD(*ground.getNormal(0)) >= 80.0f) {
            hit = false;
        }
        f32 gravityDot = getGravityVec()->dot(*ground.getNormal(0));
        if (getCurrentStatus() != MarioStatus_Slider && gravityDot > -0.1908f) {
            hit = false;
        }
        if (hit) {
            positionSum += hitPosition;
            groundCount++;
            setGroundNorm(*ground.getNormal(0));
            *mGroundPolygon = ground;
            groundPosition = hitPosition;
            mGroundPos = groundPosition;
            recordLastGround();
        }
    }
    if (!groundCount) {
        missingGround = true;
    }
    if (mMovementStates._D) {
        mMovementStates._D = false;
        TVec3f offset(mGroundPos - mPosition);
        f32 vertical = MR::vecKillElement(offset, *getGravityVec(), &offset);
        addTrans(*getGravityVec() * vertical, "force Trans");
        return true;
    }
    if (mMovementStates.jumping && isRising()) {
        return false;
    }
    if (missingGround) {
        mMovementStates._14 = true;
        return false;
    }
    mMovementStates._14 = false;
    TVec3f down(-_368);
    TVec3f position(mGroundPos);
    if (_10._22) {
        _10._22 = false;
        _1C._C = true;
    } else if ((mMovementStates._23 || mTargetWalkSpeedIndex) && getPlayerMode() != PlayerMode_Teresa) {
        if (MR::abs((position - mPosition).dot(down)) < distance) {
            TVec3f offset(position - mPosition);
            f32 vertical = MR::vecKillElement(offset, down, &offset);
            if (!MR::isNearZero(vertical, 1.0f)) {
                mDrawStates._0 = false;
                TVec3f displacement(down * vertical);
                f32 ratio = MR::clamp(mWalkSpeed, 0.0f, 1.0f);
                if (!mMovementStates._23) {
                    displacement *= ratio;
                }
                bool notSlipping = !isSlipFloorCode(_960);
                if (!notSlipping && !mDrawStates._4 && !mPrevDrawStates._4) {
                    mVelocityAfter += displacement;
                }
            }
            return true;
        }
    }
    if (mVerticalSpeed < 5.0f) {
        return true;
    }
    if (mDrawStates._6) {
        return true;
    }
    if (groundCount) {
        return true;
    }
    return mDrawStates._0;
}

// ---- MarioSound.cpp ----

struct SoundList {
    union SoundFlags {
        u32 _0;
        u8 _4[4];
    };

    const char* name;
    u32 _4;

    SoundFlags _8;
    const char* _C;

    u32 _10;
    u32 _14;
};


SoundList soundlist[] = {
    {
        "声小ジャンプ",  // name
        SE_PV_JUMP_S,    // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声中ジャンプ",  // name
        SE_PV_JUMP_M,    // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声大ジャンプ",  // name
        SE_PV_JUMP_L,    // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声後ジャンプ",   // name
        SE_PV_JUMP_TURN,  // 0x4
        0,                // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "声幅ジャンプ",   // name
        SE_PV_JUMP_LONG,  // 0x4
        0,                // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "声物ジャンプ",  // name
        SE_PV_JUMP_JOY,  // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声高飛び込み",   // name
        SE_PV_HIGH_DIVE,  // 0x4
        0,                // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "声尻ドロップ",  // name
        SE_PV_HIP_DROP,  // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声尻ドロップ着地",   // name
        SE_PV_HIP_DROP_LAND,  // 0x4
        0,                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "声着地停止",  // name
        SE_PV_LAND,    // 0x4
        0,             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "声ステージイン着地",  // name
        SE_PV_LAND_COOL,       // 0x4
        0,                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "声小ダメージ",  // name
        SE_PV_DAMAGE_S,  // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声大ダメージ",  // name
        SE_PV_DAMAGE_L,  // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声最終ダメージ",   // name
        SE_PV_LAST_DAMAGE,  // 0x4
        0,                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "声炎ダメージ",  // name
        SE_PV_BURN,      // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声電気ダメージ",   // name
        SE_PV_ELEC_DAMAGE,  // 0x4
        0,                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "声電気ダメージ終了",       // name
        SE_PV_ELEC_DAMAGE_RECOVER,  // 0x4
        0,                          // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "声倒れ",    // name
        SE_PV_DOWN,  // 0x4
        0,           // 0x8
        0,           // 0xC
        0,           // 0x10
        0,           // 0x14
    },
    {
        "声落下死亡",    // name
        SE_PV_FALL_DIE,  // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声しびれ",      // name
        SE_PV_DAMAGE_S,  // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声氷ダメージ",  // name
        SE_PV_FREEZE,    // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声つぶれダメージ",    // name
        SE_PV_DIE_ROCK_CRASH,  // 0x4
        0,                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "声スピン",  // name
        SE_PV_SPIN,  // 0x4
        0,           // 0x8
        0,           // 0xC
        0,           // 0x10
        0,           // 0x14
    },
    {
        "声スピンキャンセル",  // name
        SE_PV_SPIN_CANCEL,     // 0x4
        0,                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "声パンチ",   // name
        SE_PV_PUNCH,  // 0x4
        0,            // 0x8
        0,            // 0xC
        0,            // 0x10
        0,            // 0x14
    },
    {
        "声踏み",     // name
        SE_PV_STOMP,  // 0x4
        0,            // 0x8
        0,            // 0xC
        0,            // 0x10
        0,            // 0x14
    },
    {
        "声蹴り",    // name
        SE_PV_KICK,  // 0x4
        0,           // 0x8
        0,           // 0xC
        0,           // 0x10
        0,           // 0x14
    },
    {
        "声トルネード",     // name
        SE_PV_TWIST_START,  // 0x4
        0,                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "声壁反射",   // name
        SE_PV_GUARD,  // 0x4
        0,            // 0x8
        0,            // 0xC
        0,            // 0x10
        0,            // 0x14
    },
    {
        "声投げ",     // name
        SE_PV_THROW,  // 0x4
        0,            // 0x8
        0,            // 0xC
        0,            // 0x10
        0,            // 0x14
    },
    {
        "声壁押し",     // name
        SE_PV_LIFT_UP,  // 0x4
        0,              // 0x8
        0,              // 0xC
        0,              // 0x10
        0,              // 0x14
    },
    {
        "声壁体当たり",       // name
        SE_PV_WALL_HIT_BODY,  // 0x4
        0,                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "声スピン尻ドロップ",  // name
        SE_PV_PUNCH,           // 0x4
        0,                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "声軽い力み",  // name
        SE_PV_CATCH,   // 0x4
        0,             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "声崖つかまり",         // name
        SE_PV_CLIFF_FALL_HANG,  // 0x4
        0,                      // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "声崖つかまり終了",  // name
        SE_PV_CLIFF_CLIMB,   // 0x4
        0,                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "声炎ダメージ中",  // name
        SE_PV_BURN_RUN,    // 0x4
        0,                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "声炎ダメージ終了",  // name
        SE_PV_BURN_RECOVER,  // 0x4
        0,                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "声氷ダメージ終了",    // name
        SE_PV_FREEZE_RECOVER,  // 0x4
        0,                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "声針ダメージ",       // name
        SE_PV_NEEDLE_DAMAGE,  // 0x4
        0,                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "声針ダメージ中",         // name
        SE_PV_NEEDLE_DAMAGE_RUN,  // 0x4
        0,                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "声針ダメージ終了",           // name
        SE_PV_NEEDLE_DAMAGE_RECOVER,  // 0x4
        0,                            // 0x8
        0,                            // 0xC
        0,                            // 0x10
        0,                            // 0x14
    },
    {
        "声水中ダメージ",      // name
        SE_PV_DAMAGE_S_WATER,  // 0x4
        0,                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "声水中最終ダメージ",     // name
        SE_PV_LAST_DAMAGE_WATER,  // 0x4
        0,                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "声冷水ダメージ",         // name
        SE_PV_COLD_WATER_DAMAGE,  // 0x4
        0,                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "声スケキヨ開始",  // name
        SE_PV_BURY_HEAD,   // 0x4
        0,                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "声スケキヨ終了",         // name
        SE_PV_BURY_HEAD_RECOVER,  // 0x4
        0,                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "声足埋まり開始",  // name
        SE_PV_BURY_FOOT,   // 0x4
        0,                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "声足埋まり終了",         // name
        SE_PV_BURY_FOOT_RECOVER,  // 0x4
        0,                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "声あくび",  // name
        SE_PV_YAWN,  // 0x4
        0,           // 0x8
        0,           // 0xC
        0,           // 0x10
        0,           // 0x14
    },
    {
        "声いびき１",   // name
        SE_PV_SLEEP_1,  // 0x4
        0,              // 0x8
        0,              // 0xC
        0,              // 0x10
        0,              // 0x14
    },
    {
        "声いびき２",   // name
        SE_PV_SLEEP_2,  // 0x4
        0,              // 0x8
        0,              // 0xC
        0,              // 0x10
        0,              // 0x14
    },
    {
        "声噴水ジャンプ",  // name
        SE_PV_UPSET,       // 0x4
        0,                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "声ランニングキック",  // name
        SE_PV_KICK,            // 0x4
        0,                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "声砂脱出",     // name
        SE_PV_LIFT_UP,  // 0x4
        0,              // 0x8
        0,              // 0xC
        0,              // 0x10
        0,              // 0x14
    },
    {
        "声投げられ",  // name
        SE_PV_THROWN,  // 0x4
        0,             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "声慌て",     // name
        SE_PV_UPSET,  // 0x4
        0,            // 0x8
        0,            // 0xC
        0,            // 0x10
        0,            // 0x14
    },
    {
        "声しゃがむ",  // name
        SE_PV_SQUAT,   // 0x4
        0,             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "声砂沈み",     // name
        SE_PV_DIE_MUD,  // 0x4
        0,              // 0x8
        0,              // 0xC
        0,              // 0x10
        0,              // 0x14
    },
    {
        "声砂沈み死亡",  // name
        SE_PV_MUD_SINK,  // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "声沼沈み",     // name
        SE_PV_DIE_MUD,  // 0x4
        0,              // 0x8
        0,              // 0xC
        0,              // 0x10
        0,              // 0x14
    },
    {
        "声沼沈み死亡",  // name
        SE_PV_MUD_SINK,  // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "足音左",          // name
        SE_PM_FOOTNOTE_L,  // 0x4
        0,                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "足音右",          // name
        SE_PM_FOOTNOTE_R,  // 0x4
        0,                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "ジャンプ踏切",    // name
        SE_PM_JUMP,        // 0x4
        0x4000000,         // 0x8
        "水跳ねジャンプ",  // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "着地",        // name
        SE_PM_LAND,    // 0x4
        0x4000000,     // 0x8
        "水跳ね着地",  // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "重い着地",        // name
        SE_PM_LAND_HEAVY,  // 0x4
        0,                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "掴み",          // name
        SE_PM_GRAB_OBJ,  // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "ぶらさがり",  // name
        SE_PM_HAND,    // 0x4
        0,             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "尻ドロップ回転",   // name
        SE_PM_PRE_HIPDROP,  // 0x4
        0,                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "尻ドロップ着地",  // name
        SE_PM_HIPDROP,     // 0x4
        0x4000000,         // 0x8
        "水跳ね着地",      // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "スピン尻ドロップ回転",    // name
        SE_PM_SPIN_HIP_DROP_TURN,  // 0x4
        0,                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "スピン尻ドロップ落下",       // name
        SE_PM_LV_SPIN_HIP_DROP_FALL,  // 0x4
        0x1000000,                    // 0x8
        0,                            // 0xC
        0,                            // 0x10
        0,                            // 0x14
    },
    {
        "中ジャンプ",  // name
        SE_PM_JUMP_M,  // 0x4
        0,             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "大ジャンプ",  // name
        SE_PM_JUMP_L,  // 0x4
        0,             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "後ジャンプ",     // name
        SE_PM_JUMP_TURN,  // 0x4
        0,                // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "幅ジャンプ",     // name
        SE_PM_JUMP_LONG,  // 0x4
        0,                // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "トルネードジャンプ",  // name
        SE_PM_TWIST_JUMP,      // 0x4
        0,                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "スピンジャンプ",   // name
        SE_PM_SPIN_ATTACK,  // 0x4
        0x4000000,          // 0x8
        "水跳ねジャンプ",   // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "トランポリンジャンプ小",  // name
        SE_OJ_TRAMPOLINE_BOUND_S,  // 0x4
        0,                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "トランポリンジャンプ大",  // name
        SE_OJ_TRAMPOLINE_BOUND_L,  // 0x4
        0,                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "スリップアップ",  // name
        SE_PM_SLIP_UP,     // 0x4
        0,                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "ダメージ",      // name
        SE_PM_DAMAGE_S,  // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "炎ダメージ",  // name
        SE_PM_BURN,    // 0x4
        0,             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "針ダメージ",         // name
        SE_PM_NEEDLE_DAMAGE,  // 0x4
        0,                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "電気ダメージ",     // name
        SE_PM_ELEC_DAMAGE,  // 0x4
        0,                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "壁反射",             // name
        SE_PM_SPIN_HIT_WALL,  // 0x4
        0,                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "プレスダメージ",      // name
        SE_PM_DAMAGE_STOMPED,  // 0x4
        0,                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "氷ダメージ",      // name
        SE_PM_ICE_DAMAGE,  // 0x4
        0,                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "倒れ",            // name
        SE_PM_FALLDOWN_S,  // 0x4
        0x4000000,         // 0x8
        "水跳ね左足",      // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "吹っ飛び倒れ",    // name
        SE_PM_FALLDOWN_M,  // 0x4
        0x4000000,         // 0x8
        "水跳ね着地",      // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "坂滑り",                 // name
        SE_PM_LV_SLIP_SLIP_CODE,  // 0x4
        0x9000000,                // 0x8
        "水面滑り",               // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "ルイージ滑り",            // name
        SE_PM_LV_LUIGI_WALK_SLIP,  // 0x4
        0x9000000,                 // 0x8
        "水面滑り",                // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "パンチ風切り",     // name
        SE_PM_PUNCH_SHOOT,  // 0x4
        0,                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "炎ダメージ復帰バウンド",  // name
        SE_PM_BURN_JUMP,           // 0x4
        0,                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "炎ダメージ炎上中",  // name
        SE_PM_LV_BURNING,    // 0x4
        0x1000000,           // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "氷ダメージ終了",          // name
        SE_PM_ICE_DAMAGE_RECOVER,  // 0x4
        0,                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "引き戻し基本",           // name
        SE_PM_LV_PULL_BACK_BASE,  // 0x4
        0x1000000,                // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "引き戻し浮遊",          // name
        SE_PM_LV_PULL_BACK_FLY,  // 0x4
        0x1000000,               // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "引き戻し泡破裂",        // name
        SE_OJ_GCAPTURE_RELEASE,  // 0x4
        0,                       // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "はねとばされ",             // name
        SE_PM_LV_FLIP_DAMAGE_TURN,  // 0x4
        0x1000000,                  // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "結界ヒット",                // name
        SE_PM_KAMECK_BARRIER_BOUND,  // 0x4
        0,                           // 0x8
        0,                           // 0xC
        0,                           // 0x10
        0,                           // 0x14
    },
    {
        "壁衝突",             // name
        SE_PM_WALL_HIT_BODY,  // 0x4
        0,                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "投げられ",       // name
        SE_PM_FLIP_AWAY,  // 0x4
        0,                // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "ヘリコプタージャンプ",  // name
        SE_PM_HELI_JUMP,         // 0x4
        0,                       // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "滞空開始",                 // name
        SE_PM_HELI_JUMP_AIR_START,  // 0x4
        0,                          // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "滞空中",                // name
        SE_PM_LV_HELI_JUMP_AIR,  // 0x4
        0x1000000,               // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "ホッパー跳ね返り",  // name
        SE_PM_HOPPER_BOUND,  // 0x4
        0,                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "ホッパージャンプ溜め",    // name
        SE_PM_LV_HOPPER_PRE_JUMP,  // 0x4
        0x1000000,                 // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "ホッパージャンプ",  // name
        SE_PM_HOPPER_JUMP,   // 0x4
        0,                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "テレサ浮遊",               // name
        SE_PM_LV_TERESA_MARIO_FLY,  // 0x4
        0x1000000,                  // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "テレサ踏ん張り",   // name
        SE_PM_LV_AIR_WALK,  // 0x4
        0x1000000,          // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "テレサ壁反射",            // name
        SE_PM_TERESA_MARIO_BOUND,  // 0x4
        0,                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "テレサ消える",             // name
        SE_PM_TERESA_MARIO_VANISH,  // 0x4
        0,                          // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "テレサ現れる",             // name
        SE_PM_TERESA_MARIO_APPEAR,  // 0x4
        0,                          // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "テレサ風に乗る",            // name
        SE_PM_LV_T_MARIO_RIDE_WIND,  // 0x4
        0x1000000,                   // 0x8
        0,                           // 0xC
        0,                           // 0x10
        0,                           // 0x14
    },
    {
        "ハチ飛行中",            // name
        SE_PM_LV_BEE_MARIO_FLY,  // 0x4
        0x1000000,               // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "スケキヨ開始",    // name
        SE_PM_BURY_START,  // 0x4
        0,                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "スケキヨ終了",  // name
        SE_PM_BURY_END,  // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "スケキヨ終了スピン",  // name
        SE_PM_BURY_END_SPIN,   // 0x4
        0,                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "スケート足",  // name
        SE_PM_SKATE,   // 0x4
        0,             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "スケート滑り",       // name
        SE_PM_LV_SKATE_SLIP,  // 0x4
        0x1000000,            // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "スケートジャンプ",  // name
        SE_PM_SKATE_JUMP,    // 0x4
        0,                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "スケート着地",    // name
        SE_PM_SKATE_LAND,  // 0x4
        0,                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "スケートスピン",  // name
        SE_PM_SKATE_SPIN,  // 0x4
        0,                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "ハチ壁くっつき",     // name
        SE_PM_BEE_WALL_LAND,  // 0x4
        0,                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "ハチ壁歩き左",         // name
        SE_PM_BEE_WALL_WALK_L,  // 0x4
        0,                      // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "ハチ壁歩き右",         // name
        SE_PM_BEE_WALL_WALK_R,  // 0x4
        0,                      // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "ハチ壁ジャンプ",     // name
        SE_PM_BEE_WALL_JUMP,  // 0x4
        0,                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "砂沈み",            // name
        SE_PM_LV_SAND_SINK,  // 0x4
        0x1000000,           // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "砂脱出",        // name
        SE_PM_SAND_OUT,  // 0x4
        0,               // 0x8
        0,               // 0xC
        0,               // 0x10
        0,               // 0x14
    },
    {
        "砂強制沈み",              // name
        SE_PM_LV_SAND_SINK_FORCE,  // 0x4
        0x1000000,                 // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "砂死亡",           // name
        SE_PM_LV_SAND_DIE,  // 0x4
        0x1000000,          // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "沼強制沈み",             // name
        SE_PM_LV_MUD_SINK_FORCE,  // 0x4
        0x1000000,                // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "沼死亡",          // name
        SE_PM_LV_MUD_DIE,  // 0x4
        0x1000000,         // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "毒沼脱出",            // name
        SE_PM_POISON_MUD_OUT,  // 0x4
        0,                     // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "毒沼ダメージ",           // name
        SE_PM_POISON_MUD_DAMAGE,  // 0x4
        0,                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "ダークマター沈み",       // name
        SE_PM_LV_DARK_MATTER_IN,  // 0x4
        0x1000000,                // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "惑星貫通中",            // name
        SE_PM_LV_WARP_STRAIGHT,  // 0x4
        0x1000000,               // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "惑星貫通終了",           // name
        SE_PM_WARP_STRAIGHT_END,  // 0x4
        0,                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "フー滞空中",           // name
        SE_PM_LV_FOO_FLY_WAIT,  // 0x4
        0x1000000,              // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "フー加速",       // name
        SE_PM_FOO_ACCEL,  // 0x4
        0,                // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "フー飛行中",         // name
        SE_PM_LV_FOO_FLYING,  // 0x4
        0x1000000,            // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "フーブレーキ",   // name
        SE_PM_FOO_BRAKE,  // 0x4
        0,                // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "ワープポッド入り",  // name
        SE_PM_WARP_POD_IN,   // 0x4
        0,                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "ワープポッド出",    // name
        SE_PM_WARP_POD_OUT,  // 0x4
        0,                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "ワープポッド移動",      // name
        SE_PM_LV_WARP_POD_MOVE,  // 0x4
        0x1000000,               // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "スペシャルダッシュ強",   // name
        SE_PM_RACE_START_DASH_L,  // 0x4
        0,                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "スペシャルダッシュ弱",   // name
        SE_PM_RACE_START_DASH_S,  // 0x4
        0,                        // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "ダッシュ加速強成功",  // name
        SE_SY_GET_DASH_RING,   // 0x4
        0x2000000,             // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "ブラックホール吸い込まれ",  // name
        SE_PM_BLACK_HOLE_IN,         // 0x4
        0,                           // 0x8
        0,                           // 0xC
        0,                           // 0x10
        0,                           // 0x14
    },
    {
        "水歩行突入",        // name
        SE_PM_WALK_TO_SWIM,  // 0x4
        0,                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "水歩行脱出",        // name
        SE_PM_SWIM_TO_WALK,  // 0x4
        0,                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "水面ウエイト",       // name
        SE_PM_WAIT_ON_WATER,  // 0x4
        0,                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "水中ウエイト",          // name
        SE_PM_WAIT_UNDER_WATER,  // 0x4
        0,                       // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "水落下突入",         // name
        SE_PM_DIVE_TO_WATER,  // 0x4
        0,                    // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "水ジャンプ脱出",       // name
        SE_PM_JUMP_FROM_WATER,  // 0x4
        0,                      // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "水面一掻き",               // name
        SE_PM_SWIM_ACCEL_ON_WATER,  // 0x4
        0,                          // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "水中一掻き",               // name
        SE_PM_SWIM_ACCEL_IN_WATER,  // 0x4
        0,                          // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "水面バタ足",            // name
        SE_PM_LV_SWIM_ON_WATER,  // 0x4
        0x1000000,               // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "水中バタ足",            // name
        SE_PM_LV_SWIM_IN_WATER,  // 0x4
        0x1000000,               // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "水面潜り",                // name
        SE_PM_DIVE_WATER_ROLLING,  // 0x4
        0,                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "水中潜り",                // name
        SE_PM_DIVE_FAST_IN_WATER,  // 0x4
        0,                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "水面スピン開始",           // name
        SE_PM_TORNADE_ON_WATER_ST,  // 0x4
        0,                          // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "水面スピン",               // name
        SE_PM_LV_TORNADE_ON_WATER,  // 0x4
        0x1000000,                  // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "水中スピン開始",           // name
        SE_PM_TORNADE_IN_WATER_ST,  // 0x4
        0,                          // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "水中スピン",               // name
        SE_PM_LV_TORNADE_IN_WATER,  // 0x4
        0x1000000,                  // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "水面ダメージ",         // name
        SE_PM_DAMAGE_ON_WATER,  // 0x4
        0,                      // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "水中ダメージ",         // name
        SE_PM_DAMAGE_IN_WATER,  // 0x4
        0,                      // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "亀ジェット泳ぎ",          // name
        SE_OJ_LV_TURTLE_JET_SWIM,  // 0x4
        0x1000000,                 // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "水底接触",                 // name
        SE_PM_LV_TURTLE_SEA_SMOKE,  // 0x4
        0x1000000,                  // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "亀壁ヒット",              // name
        SE_OJ_TURTLE_JET_BOUND_W,  // 0x4
        0,                         // 0x8
        0,                         // 0xC
        0,                         // 0x10
        0,                         // 0x14
    },
    {
        "亀ブレーキ",        // name
        SE_PM_TURTLE_BRAKE,  // 0x4
        0,                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "亀加速",            // name
        SE_PM_TURTLE_ACCEL,  // 0x4
        0,                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "水跳ね左足",        // name
        SE_PM_FOOTNOTE_L_W,  // 0x4
        0,                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "水跳ね右足",        // name
        SE_PM_FOOTNOTE_R_W,  // 0x4
        0,                   // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "水跳ね左足小",          // name
        SE_PM_FOOTNOTE_SUB_L_W,  // 0x4
        0,                       // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "水跳ね右足小",          // name
        SE_PM_FOOTNOTE_SUB_R_W,  // 0x4
        0,                       // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "水跳ねジャンプ",  // name
        SE_PM_JUMP_W,      // 0x4
        0,                 // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "水跳ね着地",  // name
        SE_PM_LAND_W,  // 0x4
        0,             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "水跳ね手",    // name
        SE_PM_HAND_W,  // 0x4
        0,             // 0x8
        0,             // 0xC
        0,             // 0x10
        0,             // 0x14
    },
    {
        "水弾かれ",         // name
        SE_PM_WATER_BOUND,  // 0x4
        0,                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "水面滑り",           // name
        SE_PM_LV_SLIP_WATER,  // 0x4
        0x1000000,            // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "トルネード風",       // name
        SE_PM_LV_TWIST_WIND,  // 0x4
        0x1000000,            // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "スリップ",     // name
        SE_PM_LV_SLIP,  // 0x4
        0x9000000,      // 0x8
        "水面滑り",     // 0xC
        0,              // 0x10
        0,              // 0x14
    },
    {
        "最後の一撃",       // name
        SE_PM_LAST_DAMAGE,  // 0x4
        0,                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "空中ふんばり",     // name
        SE_PM_LV_AIR_WALK,  // 0x4
        0x1000000,          // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "スピン許可",       // name
        SE_PM_SPIN_ENABLE,  // 0x4
        0,                  // 0x8
        0,                  // 0xC
        0,                  // 0x10
        0,                  // 0x14
    },
    {
        "スピン回復終了",        // name
        SE_PM_SPIN_RECOVER_END,  // 0x4
        0,                       // 0x8
        0,                       // 0xC
        0,                       // 0x10
        0,                       // 0x14
    },
    {
        "トルネード飛行",         // name
        SE_PM_LV_TORNADE_FLYING,  // 0x4
        0x1000000,                // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "ライフ警告",         // name
        SE_SY_LV_LIFE_ALERT,  // 0x4
        0x3000000,            // 0x8
        0,                    // 0xC
        0,                    // 0x10
        0,                    // 0x14
    },
    {
        "ライフ回復",        // name
        SE_SY_LIFE_RECOVER,  // 0x4
        0x2000000,           // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "水中ライフ減少",      // name
        SE_SY_WATER_LIFE_DEC,  // 0x4
        0x2000000,             // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "酸素減少警告",      // name
        SE_SY_OXYGEN_ALERT,  // 0x4
        0x2000000,           // 0x8
        0,                   // 0xC
        0,                   // 0x10
        0,                   // 0x14
    },
    {
        "無酸素警告",             // name
        SE_SY_OXYGEN_ZERO_ALERT,  // 0x4
        0x2000000,                // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "酸素回復",            // name
        SE_SY_INC_OXYGEN_ONE,  // 0x4
        0x2000000,             // 0x8
        0,                     // 0xC
        0,                     // 0x10
        0,                     // 0x14
    },
    {
        "酸素完全回復",         // name
        SE_SY_INC_OXYGEN_FULL,  // 0x4
        0x2000000,              // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "水面酸素回復",           // name
        SE_SY_LV_RECOVER_OXYGEN,  // 0x4
        0x3000000,                // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "水面酸素完全回復",         // name
        SE_SY_RECOVER_OXYGEN_FULL,  // 0x4
        0x2000000,                  // 0x8
        0,                          // 0xC
        0,                          // 0x10
        0,                          // 0x14
    },
    {
        "無酸素ダメージ",  // name
        SE_SY_NO_OXYGEN,   // 0x4
        0x2000000,         // 0x8
        0,                 // 0xC
        0,                 // 0x10
        0,                 // 0x14
    },
    {
        "ハチ体力回復",               // name
        SE_SY_LV_INC_AIR_WALK_TIMER,  // 0x4
        0x3000000,                    // 0x8
        0,                            // 0xC
        0,                            // 0x10
        0,                            // 0x14
    },
    {
        "ハチ体力完全回復",          // name
        SE_SY_INC_AIR_WALK_TIMER_F,  // 0x4
        0x2000000,                   // 0x8
        0,                           // 0xC
        0,                           // 0x10
        0,                           // 0x14
    },
    {
        "ハチ体力切れ",           // name
        SE_SY_NO_AIR_WALK_TIMER,  // 0x4
        0x2000000,                // 0x8
        0,                        // 0xC
        0,                        // 0x10
        0,                        // 0x14
    },
    {
        "変身解除",       // name
        SE_SY_MORPH_END,  // 0x4
        0x2000000,        // 0x8
        0,                // 0xC
        0,                // 0x10
        0,                // 0x14
    },
    {
        "呪い解除",             // name
        SE_SY_MORPH_TO_NORMAL,  // 0x4
        0x2000000,              // 0x8
        0,                      // 0xC
        0,                      // 0x10
        0,                      // 0x14
    },
    {
        "",  // name
        0,   // 0x4
        0,   // 0x8
        0,   // 0xC
        0,   // 0x10
        0,   // 0x14
    },
};


struct SoundSwapList {
    const char* name;

    u32 offset1;
    u32 offset2;
    u32 offset3;
};


SoundSwapList soundswaplist[] = {{"", 0, 0, 0}};


u32 Mario::initSoundTable(SoundList* pList, u32 globalTablePosition) {
    u32* pSwapOffset = reinterpret_cast< u32* >(soundswaplist) + globalTablePosition;
    SoundList* pEntry;
    u32 count = 0;
    s32 i = 0;
    while (true) {
        pEntry = pList + i;
        if (pEntry->name[0] == '\0') {
            break;
        }

        pEntry->_10 = 0;
        pEntry->_14 = pEntry->_4;

        if (globalTablePosition != 0) {
            s32 swapIndex = 0;
            while (true) {
                const SoundSwapList* pSwapEntry = soundswaplist + swapIndex;
                if (pSwapEntry->name[0] == '\0') {
                    break;
                }

                if (strcmp(pEntry->name, pSwapEntry->name) == 0) {
                    u32 soundID = pSwapOffset[swapIndex * 4];
                    if (soundID != 0) {
                        pEntry->_14 = soundID;
                    }
                    break;
                }

                swapIndex++;
            }
        }

        count++;
        i++;
    }

    return count;
}


void Mario::initSound() {
    u32 count = initSoundTable(soundlist, 0);
    _96C = new HashSortTable(count);
    for (u32 i = 0; i < count; i++) {
        _96C->add(soundlist[i].name, i, false);
    }

    _96C->sort();
    _970 = nullptr;
}


void Mario::playSoundJ(const char* pSoundName, s32 timing) {
    u32 index;
    if (_96C->search(pSoundName, &index)) {
        switch (soundlist[index]._8._4[0] & 0x3) {
        case 0:
            MR::startSound(mActor, soundlist[index]._14, timing);
            break;

        case 2:
            MR::startSystemSE(soundlist[index]._14, timing);
            break;

        case 1:
            MR::startLevelSound(mActor, soundlist[index]._14, timing);
            break;

        case 3:
            MR::startSystemLevelSE(soundlist[index]._14, timing);
            break;
        }

        switch (soundlist[index]._8._4[0] & ~0x3) {
        case 0x4:
        case 0x8:
            if (mDrawStates.mIsUnderwater || mDrawStates._13) {
                playSoundJ(soundlist[index]._C, -1);
            }
            break;
        }
    }

    bool isFound = _96C->search("声", pSoundName, &index);
    if (isFound) {
        MR::startSound(mActor, soundlist[index]._14, timing);
    }
}


void Mario::stopSoundJ(const char* pSoundName, u32 delay) {
    u32 index;
    if (_96C->search(pSoundName, &index)) {
        switch (soundlist[index]._8._4[0] & 0x3) {
        case 0:
            MR::stopSound(mActor, soundlist[index]._14, delay);
            break;

        case 1:
            break;

        case 2:
            MR::stopSystemSE(soundlist[index]._14, delay);
            break;

        case 3:
            break;
        }
    }

    if (_96C->search("声", pSoundName, &index)) {
        JAISoundID soundID(soundlist[index]._14);
        MR::stopSound(mActor, soundID, delay);
    }
}


#if 0  // SMG2: uses LiveActor::mSoundObject (AudAnmSoundObject), gone in SMG2
void Mario::startBas(const char* pAnimName, bool arg2, f32 startFrame, f32 speed) {
    if (mActor->mSoundObject != nullptr) {
        ResourceHolder* pHolder = MR::getResourceHolder(mActor);
        const JAUSoundAnimation* pRes = nullptr;

        if (pAnimName != nullptr && pHolder->mBasResTable->isExistRes(pAnimName)) {
            pRes = static_cast< JAUSoundAnimation* >(pHolder->mBasResTable->getRes(pAnimName));
        }

        if (pRes != nullptr) {
            mActor->mSoundObject->startAnimation(pRes, arg2, startFrame, speed);
        } else {
            mActor->mSoundObject->removeAnimation();
        }

        _970 = pAnimName;
    }
}


bool Mario::isRunningBas(const char* pAnimName) const {
    if (_970 == nullptr) {
        return false;
    }

    if (strcmp(_970, pAnimName) == 0) {
        return true;
    }

    return false;
}


void Mario::skipBas(f32 frame) {
    mActor->mSoundObject->skip(frame);
}
#endif


void Mario::playSoundTeresaFlying() {
    if (getPlayerMode() != 6) {
        return;
    }

    s32 timing = 100;
    if (getCurrentStatus() == 0x1C) {
        if (mWait->_14 == 0) {
            timing = 100 - static_cast< s32 >(mWait->_16);
            if (timing < 0) {
                timing = 0;
            }
        } else {
            timing = 0;
        }
    }

    playSound("テレサ浮遊", timing);
}


void Mario::playSoundTrampleCombo(u8 combo) {
    if (combo >= 7) {
        return;
    }

    MR::startSystemSE("SE_SY_TRAMPLE_COMBO", combo);
}


void Mario::setSeVersion(u32 version) {
    MR::setSeVersion(mActor, version);
}

// ---- MarioSpecial.cpp ----

void Mario::checkOnimasu(const HitSensor* pSensor) {
    if (strstr(pSensor->mHost->mName, "オニマス") == nullptr) {
        return;
    }

    if (_5FC == nullptr) {
        _5FC = pSensor;
        _60C = 1;
    } else if (_5FC != pSensor && (pSensor->mPosition - mPosition).length() < (_5FC->mPosition - mPosition).length()) {
        _5FC = pSensor;
        _60C = 1;
    }
}


bool Mario::isDossun(const Triangle* pTriangle) const {
    if (!pTriangle->isValid()) {
        return false;
    }

    if (strstr(pTriangle->mSensor->mHost->getName(), "ドッスン") != nullptr) {
        return true;
    }

    return false;
}


bool Mario::isStageCameraRotate2D() const {
    return MR::isEqualStageName("HellProminenceGalaxy") && MR::getCurrentScenarioNo() == 3;
}


bool Mario::isNoWalkFallOnDossun() const {
    if (!MR::isEqualStageName("CannonFleetGalaxy")) {
        return false;
    }

    if (isDossun(mGroundPolygon) && isDossun(_45C)) {
        return true;
    }

    return false;
}


bool Mario::isNotReflectGlassGround() const {
    if (!MR::isEqualStageName("AstroGalaxy")) {
        return false;
    }

    TVec3f vec(1627.0f, 783.0f, -2152.0f);
    TVec3f v10 = mPosition - vec;

    f32 v4 = MR::diffAngleAbs(getCamDirZ(), v10);
    f32 v5 = v4;
    if (v4 >= 1.5707964f) {
        v5 = (PI - v4);
    }

    f32 v7 = (400.0f + (300.0f * (MR::sin(v5) * MR::sin(v5))));
    MR::vecKillElement(v10, getAirGravityVec(), &v10);
    f32 length = v10.length();

    if (length >= v7) {
        if (length <= 700.0f) {
            return true;
        }
    }

    return false;
}


bool Mario::isUseAnotherMovingPolygon() const {
    if (MR::isEqualStageName("BattleShipGalaxy")) {
        return true;
    }

    if (MR::isEqualStageName("SandClockGalaxy")) {
        return true;
    }

    if (MR::isEqualStageName("TriLegLv1Galaxy")) {
        return true;
    }

    return false;
}


bool Mario::isUseFooSpecialGravity(const TVec3f& rA1, TVec3f* pA2) const {
    if (!isStatusActive(MarioStatus_Foo)) {
        return false;
    }

    if (MR::isEqualStageName("HeavensDoorGalaxy") && MR::getCurrentScenarioNo() == 2) {
        TVec3f vec(14760.0f, -10676.2f, 6770.0f);
        TVec3f res = vec - rA1;
        pA2->set(res);
        MR::normalizeOrZero(pA2);
        return true;
    }

    return false;
}


void Mario::updateOnimasu() {
    if (!_5FC) {
        return;
    }

    if ((_5FC->mPosition - mPosition).length() >= 1000.0f) {
        _5FC = nullptr;
        return;
    }

    const HitSensor* sensor = _5FC;
    MtxPtr inverse = sensor->mHost->mCollisionParts->mInvBaseMatrix;
    TVec3f localPosition;
    TVec3f previousPosition;
    previousPosition = _600;
    PSMTXMultVec(inverse, &mActor->mPosition, &localPosition);
    if (_60C) {
        _600 = localPosition;
        _60C = 0;
        return;
    }

    if (!_10._1D) {
        if (previousPosition.y < -400.0f && localPosition.y > -400.0f) {
            if (MR::isInRange(localPosition.x, -400.0f, 400.0f) && MR::isInRange(localPosition.z, -400.0f, 400.0f)) {
                localPosition = _600;
                TVec3f position;
                PSMTXMultVec(sensor->mHost->mCollisionParts->mBaseMatrix, &localPosition, &position);
                setTrans(position, nullptr);
                mVelocity.zero();
            }
        } else if (previousPosition.y > 400.0f && localPosition.y <= 400.0f) {
            if (MR::isInRange(localPosition.x, -400.0f, 400.0f) && MR::isInRange(localPosition.z, -400.0f, 400.0f)) {
                _10._1D = true;
                if (previousPosition.x < -320.0f) {
                    previousPosition.x = -320.0f;
                } else if (previousPosition.x > 320.0f) {
                    previousPosition.x = 320.0f;
                }

                if (previousPosition.z < -320.0f) {
                    previousPosition.z = -320.0f;
                } else if (previousPosition.z > 320.0f) {
                    previousPosition.z = 320.0f;
                }

                localPosition = previousPosition;
            }
        } else if (MR::isInRange(localPosition.x, -400.0f, 400.0f) && MR::isInRange(localPosition.y, -400.0f, 400.0f) &&
                   MR::isInRange(localPosition.z, -400.0f, 400.0f)) {
            mVelocity.zero();
            localPosition = _600;
            TVec3f position;
            PSMTXMultVec(sensor->mHost->mCollisionParts->mBaseMatrix, &localPosition, &position);
            TVec3f displacement(position - mPosition);
            f32 height = 10.0f + mVerticalSpeed;
            if (-displacement.dot(getShadowNorm()) > height) {
                MR::vecKillElement(displacement, getShadowNorm(), &displacement);
                displacement += -getShadowNorm() * mVerticalSpeed;
                position = mPosition + displacement;
            }

            setTrans(position, nullptr);
        }

        bool correctX = false;
        bool correctY = false;
        bool correctZ = false;
        TVec3f axisX;
        TVec3f axisY;
        TVec3f axisZ;
        sensor->mHost->mCollisionParts->mBaseMatrix.getXDir(axisX);
        sensor->mHost->mCollisionParts->mBaseMatrix.getYDir(axisY);
        sensor->mHost->mCollisionParts->mBaseMatrix.getZDir(axisZ);
        f32 gravityX = axisX.dot(*getGravityVec());
        f32 absGravityX = MR::abs(gravityX);
        f32 gravityY = axisY.dot(*getGravityVec());
        f32 absGravityY = MR::abs(gravityY);
        f32 gravityZ = axisZ.dot(*getGravityVec());
        f32 absGravityZ = MR::abs(gravityZ);
        f32 lower = -500.0f;
        f32 upper = -300.0f;

        if (absGravityX > absGravityY && absGravityX > absGravityZ) {
            if (gravityX > 0.0f) {
                lower = 300.0f;
                upper = 500.0f;
            }

            if (MR::isInRange(localPosition.x, lower, upper)) {
                bool insideY = MR::isInRange(localPosition.y, -320.0f, 320.0f);
                bool insideZ = MR::isInRange(localPosition.z, -320.0f, 320.0f);
                if (insideY && insideZ) {
                    mActor->_3B4 = axisX;
                    mActor->setPress(0, 0);
                } else {
                    if (insideZ) {
                        correctY = true;
                    }

                    if (insideY) {
                        correctZ = true;
                    }
                }
            }
        } else if (absGravityY > absGravityX && absGravityY > absGravityZ) {
            if (gravityY > 0.0f) {
                lower = 300.0f;
                upper = 500.0f;
            }

            if (MR::isInRange(localPosition.y, lower, upper)) {
                bool insideX = MR::isInRange(localPosition.x, -320.0f, 320.0f);
                bool insideZ = MR::isInRange(localPosition.z, -320.0f, 320.0f);
                if (insideX && insideZ) {
                    mActor->_3B4 = axisY;
                    if (gravityY < 0.0f) {
                        mActor->setPress(0, 0);
                    }
                } else {
                    if (insideZ) {
                        correctX = true;
                    }

                    if (insideX) {
                        correctZ = true;
                    }
                }
            }
        } else {
            if (gravityZ > 0.0f) {
                lower = 300.0f;
                upper = 500.0f;
            }

            if (MR::isInRange(localPosition.z, lower, upper)) {
                bool insideX = MR::isInRange(localPosition.x, -320.0f, 320.0f);
                bool insideY = MR::isInRange(localPosition.y, -320.0f, 320.0f);
                if (insideX && insideY) {
                    mActor->_3B4 = axisZ;
                    mActor->setPress(0, 0);
                } else {
                    if (insideY) {
                        correctX = true;
                    }

                    if (insideX) {
                        correctY = true;
                    }
                }
            }
        }

        if (correctX) {
            if (localPosition.x > -400.0f && localPosition.x < -320.0f) {
                localPosition.x = -400.0f;
            } else if (localPosition.x < 400.0f && localPosition.x > 320.0f) {
                localPosition.x = 400.0f;
            }
        }

        if (correctY) {
            if (localPosition.y > -400.0f && localPosition.y < -320.0f) {
                localPosition.y = -400.0f;
            } else if (localPosition.y < 400.0f && localPosition.y > 320.0f) {
                localPosition.y = 400.0f;
            }
        }

        if (correctZ) {
            if (localPosition.z > -400.0f && localPosition.z < -320.0f) {
                localPosition.z = -400.0f;
            } else if (localPosition.z < 400.0f && localPosition.z > 320.0f) {
                localPosition.z = 400.0f;
            }
        }

        if (correctX | correctY | correctZ) {
            TVec3f position;
            PSMTXMultVec(sensor->mHost->mCollisionParts->mBaseMatrix, &localPosition, &position);
            if (mMovementStates._8 || mMovementStates._19 || mMovementStates._1A) {
                TVec3f displacement(position - mPosition);
                if (getWallNorm().dot(displacement) < 0.0f) {
                    addVelocity(-displacement);
                } else {
                    setTrans(position, nullptr);
                }
            }
        }
    } else if (localPosition.y > 400.0f) {
        _10._1D = false;
    } else {
        bool corrected = false;
        if (previousPosition.y >= -320.0f && localPosition.y < -320.0f) {
            localPosition.y = previousPosition.y;
            corrected = true;
        }

        if (previousPosition.x >= -320.0f && localPosition.x < -320.0f) {
            localPosition.x = previousPosition.x;
            corrected = true;
        }

        if (previousPosition.x <= 320.0f && localPosition.x > 320.0f) {
            localPosition.x = previousPosition.x;
            corrected = true;
        }

        if (previousPosition.z >= -320.0f && localPosition.z < -320.0f) {
            localPosition.z = previousPosition.z;
            corrected = true;
        }

        if (previousPosition.z <= 320.0f && localPosition.z > 320.0f) {
            localPosition.z = previousPosition.z;
            corrected = true;
        }

        if (corrected) {
            TVec3f position;
            PSMTXMultVec(sensor->mHost->mCollisionParts->mBaseMatrix, &_600, &position);
            setTrans(position, nullptr);
            localPosition = _600;
        }
    }

    _600 = localPosition;
}


bool Mario::isHeadPushEnableArea() const {
    if (!mMovementStates._37) {
        return false;
    }

    if (MR::isEqualStageName("SandClockGalaxy")) {
        if (MR::isInRange(mPosition.x, -7020.0f, -6920.0f)) {
            return true;
        }
    }

    return false;
}


bool Mario::isOnimasuBinderPressSkip() const {
    if (isStatusActive(MarioStatus_SideStep)) {
        if (mFrontWallTriangle->mSensor != nullptr) {
            if (strstr(mFrontWallTriangle->mSensor->mHost->mName, "オニマス")) {
                return true;
            }
        }
    }

    return false;
}

// ---- MarioSpin.cpp ----

void Mario::checkTornado() {
    if (mMovementStates._1) {
        mMovementStates._2B = false;
    }
}


void Mario::resetTornado() {
    _530 = 0.0f;
    _534 = 0;
    _538 = 0.0f;
    mMovementStates._F = false;
    _544 = 0;
    mYAngleOffset = 0.0f;
    _3F0 = 1.0f;
}


void Mario::calcTornadoTilt() {
    bool spinning = mMovementStates._F != 0;
    bool flying = false;
    if (getPlayerMode() == PlayerMode_Bee) {
        if (mMovementStates.jumping && mMovementStates._11) {
            flying = true;
        }
    }

    bool tilted = spinning | flying;
    if (!isStickOn() || !tilted) {
        _548 *= mActor->getConst().getTable()->mTornadoTiltCancel;
        bool blended;
        if (tilted) {
            blended = MR::vecBlendSphere(_54C, mHeadVec, &_54C, mActor->getConst().getTable()->mTornadoTiltSpeed);
        } else {
            blended = MR::vecBlendSphere(_54C, mHeadVec, &_54C, mActor->getConst().getTable()->mTornadoTiltOffSpeed);
        }

        if (!blended) {
            _54C = mHeadVec;
        }
    } else {
        TVec3f tilt(getWorldPadDir() * mActor->getConst().getTable()->mTornadoTiltAngle +
                    mHeadVec * (1.0f - mActor->getConst().getTable()->mTornadoTiltAngle));
        MR::normalize(&tilt);
        bool blended = MR::vecBlendSphere(_54C, tilt, &_54C, mActor->getConst().getTable()->mTornadoTiltSpeed);
        MR::normalize(&_54C);
        f32 alignment = MR::abs(getWorldPadDir().dot(mFrontVec));
        _548 = _548 * mActor->getConst().getTable()->mTornadoTiltNear + alignment * (1.0f - mActor->getConst().getTable()->mTornadoTiltNear);
        if (!blended) {
            _54C = tilt;
        }
    }
}


void Mario::reflectWallOnSpinning(const TVec3f& rNormal, u16 time) {
    setFrontVecKeepUp(rNormal);
    _3F8 = time;
    _328 = mFrontVec;
    doSpinWallEffect();
}


void Mario::forceStopTornado() {
    if (mMovementStates._F) {
        _40A = mActor->getConst().getTable()->mTornadoRestartTime;
    }

    resetTornado();
    if (mMovementStates.jumping) {
        cancelTornadoJump();
    }

    mDrawStates._8 = true;
}


void Mario::doSpinWallEffect() {
    if ((!mMovementStates._8 || !mFrontWallTriangle->mSensor->isType(0x55)) && (!mMovementStates._19 || !mBackWallTriangle->mSensor->isType(0x55)) &&
        (!mMovementStates._1A || !mSideWallTriangle->mSensor->isType(0x55))) {
        playSound("壁反射");
        playSound("声スピンキャンセル");
        playEffect("壁スパーク");
    }
}


void Mario::startRotationTask(u32 flags) {
    pushTask(&Mario::taskOnRotation, flags);
}


bool Mario::taskOnRotation(u32 flags) {
    if (flags & 4) {
        if (!isAnimationRun("ヘリコプタージャンプ")) {
            mYAngleOffset = 0.0f;
            return false;
        }

        if (isRising()) {
            mYAngleOffset += mActor->getConst().getTable()->mTrampleBegomaRotRise;
        } else {
            mYAngleOffset += mActor->getConst().getTable()->mTrampleBegomaRotFall;
        }
    }

    return true;
}

// ---- MarioState.cpp ----


void Mario::sendStateMsg(u32 msg) {
    MarioState* pNext;

    for (MarioState* pState = _97C; pState != nullptr;) {
        pNext = pState->_8;

        if (isStatusActive(pState->mStatusId) == MarioStatus_None) {
            pState = pNext;
            continue;
        }

        if (!pState->proc(msg)) {
            if (isStatusActive(pState->mStatusId) != MarioStatus_None) {
                closeStatus(pState);
            }
        } else if (msg == MarioStateMsg_Update) {
            msg = MarioStateMsg_Keep;
        }

        pState = pNext;
    }
}


bool Mario::updatePosture(MtxPtr mtx) {
    if (_97C != nullptr) {
        return _97C->postureCtrl(mtx);
    }

    postureCtrl(mtx);

    return false;
}


void Mario::changeStatus(MarioState* pState) {
    if (isStatusActive(pState->mStatusId)) {
        return;
    }

    _980 = pState;

    sendStateMsg(MarioStateMsg_Notice);

    if (_97C) {
        pState->_8 = _97C;
    }

    _97C = pState;

    if (!pState->proc(MarioStateMsg_Start)) {
        closeStatus(pState);

        _97C = pState->_8;
    }
}


void Mario::closeStatus(MarioState* pState) {
    if (pState == nullptr) {
        while (_97C != nullptr) {
            closeStatus(_97C);
        }

        return;
    }

    MarioState* pCurr = _97C;

    if (pCurr == pState) {
        _97C = pState->_8;
    } else {
        while (pCurr != nullptr) {
            MarioState* pNext = pCurr->_8;

            if (pNext == pState) {
                break;
            }

            pCurr = pNext;
        }

        pCurr->_8 = pState->_8;
    }

    pState->_8 = nullptr;
    pState->proc(MarioStateMsg_Close);
}


u32 Mario::getCurrentStatus() const {
    MarioState* pState = _97C;

    if (pState == nullptr) {
        return MarioStatus_None;
    }

    return pState->mStatusId;
}


bool Mario::isStatusActive(u32 statusId) const {
    MarioState* pState = _97C;

    if (pState == nullptr) {
        return false;
    }

    if (statusId == pState->mStatusId) {
        return true;
    }

    while (pState != nullptr) {
        if (statusId == pState->mStatusId) {
            return true;
        }

        pState = pState->_8;
    }

    return false;
}

// ---- MarioStep.cpp ----

void Mario::checkStep() {
    if (mMovementStates.jumping) {
        return;
    }

    if (mMovementStates._A) {
        return;
    }

    if (!mMovementStates._1) {
        return;
    }

    if (isStatusActive(MarioStatus_Hang)) {
        return;
    }

    if (isStatusActive(MarioStatus_Wall)) {
        return;
    }

    if (isStatusActive(MarioStatus_Swim)) {
        return;
    }

    if (isStatusActive(MarioStatus_Climb)) {
        return;
    }

#if VERSION == RMGK01
    if (isPlayerModeHopper()) {
        return;
    }
#endif

    if (isDamaging()) {
        return;
    }

    if (isSwimming()) {
        return;
    }

    if (!MR::isNearZero(_184)) {
        return;
    }

    if (!isStickOn()) {
        return;
    }

    if (_750) {
        return;
    }

    if (mDrawStates._C) {
        return;
    }

    if (isSlipPolygon(_45C)) {
        return;
    }

    if (_4D8->isValid() && !mMovementStates._8 && mMovementStates._1) {
        const f32 frontDot = mFrontVec.dot(*_4D8->getNormal(0));
        TVec3f horizontal;
        MR::vecKillElement(_50C - mPosition, -*getGravityVec(), &horizontal);

        if (horizontal.length() < 20.0f && MR::abs(frontDot) < 0.3926991f) {
            f32 stepHeight = (_50C - mPosition).dot(-*getGravityVec());
            if (stepHeight < mActor->getConst().getTable()->mWalkStepHeight) {
                startStep(_50C);
            }
        } else if (MR::abs(frontDot) < 1.0471976f) {
            Mtx rotMtx;
            PSMTXRotAxisRad(rotMtx, &mSideVec, frontDot);
            PSMTXMultVec(rotMtx, &mVelocity, &mVelocity);
        }

        return;
    }

    if (!mMovementStates._15) {
        return;
    }

    if (!mMovementStates._39) {
        return;
    }

    const TVec3f* worldPadDir = &getWorldPadDir();
    if (!(worldPadDir->dot(*mFrontWallTriangle->getNormal(0)) < -0.5f)) {
        return;
    }

    f32 stepHeight = (_4A4 - mPosition).dot(-*getGravityVec());
    if (stepHeight < mActor->getConst().getTable()->mWalkStepHeight) {
        startStep(_4A4);
    }
}


void Mario::startStep(const TVec3f& rVec) {
    if (getCurrentStatus() == 0x10) {
        return;
    }

    TVec3f checkPos = rVec - *getGravityVec() * 80.0f;
    HitInfo hitInfo;

    if (MR::checkStrikePointToMap(checkPos, &hitInfo)) {
        return;
    }

    const bool hitNear = Collision::checkStrikeBallToMap(checkPos, 20.0f, nullptr, nullptr);
    if (hitNear) {
        return;
    }

    const bool hitFar = Collision::checkStrikeBallToMap(checkPos, 50.0f, nullptr, nullptr);
    if (hitFar) {
        return;
    }

    TVec3f oldPos(mPosition);
    mPosition = rVec;
    const f32 ceilDist = calcDistToCeil(false);
    mPosition = oldPos;

    if (ceilDist < 200.0f) {
        return;
    }

    if (_4C8->isValid() && MR::isSensorPressObj(_4C8->mSensor)) {
        return;
    }

    if (_4D8->isValid()) {
        if (MR::isSensorPressObj(_4D8->mSensor)) {
            return;
        }

        MtxPtr prevBase = _4D8->getPrevBaseMtx()->toMtxPtr();
        MtxPtr base = _4D8->getBaseMtx()->toMtxPtr();

        if (!MR::isSameMtx(base, prevBase) && _4D8->mSensor != mGroundPolygon->mSensor) {
            return;
        }
    }

    setTrans(rVec, "階段");
    changeStatus(mStep);
    _3D0 = 0;
    const f32 zero = 0.0f;
    mVelocity.z = zero;
    mVelocity.y = zero;
    mVelocity.x = zero;
    mMovementStates._10 = false;
}

// ---- MarioStick.cpp ----

void Mario::checkBeeStick() {
    return;
}


void Mario::tryBeeStick(const HitSensor* pSensor) {
    bool res = mStick->setStickSensor(pSensor);
    if (res) {
        changeStatus(mStick);
    }
}

// ---- MarioSwim.cpp ----

void MarioSwim_FORCE_MATCH_SDATA2() {
    (void)100.0f;
    (void)50.0f;
    (void)0.52359879f;
    (void)-200.0f;
    (void)0.100000001f;
    (void)0.200000003f;
    (void)10.0f;
    (void)1.74532938f;
    (void)-10.0f;
    (void)1.49599659f;
    (void)200.0f;
    (void)8.0f;
    (void)2.09439516f;
    (void)40.0f;
    (void)0.00999999978f;
    (void)0.0299999993f;
    (void)6.28318548f;
    (void)-50.0f;
    (void)0.196349546f;
    (void)30.0f;
    (void)-30.0f;
    (void)-0.100000001f;
    (void)60.0f;
    (void)1000.0f;
    (void)0.899999976f;
    (void)400.0f;
    (void)0.707000017f;
    (void)2.5f;
    (void)-400.0f;
    (void)120.0f;
    (void)0.125f;
    (void)300.0f;
    (void)0.0500000007f;
    (void)5.0f;
    (void)0.150000006f;
    (void)0.87266469f;
    (void)2.26892805f;
    (void)12.0f;
    (void)25.0f;
    (void)0.25f;
    (void)0.980000019f;
    (void)0.00100000005f;
    (void)1.04719758f;
    (void)2000.0f;
    (void)-25.0f;
    (void)0.949999988f;
    (void)0.75f;
    (void)0.300000012f;
    (void)2.61799383f;
    (void)0.99000001f;
    (void)0.995000005f;
    (void)0.959999979f;
    (void)1.65346992f;
    (void)15.0f;
    (void)110.0f;
    (void)80.0f;
    (void)-500.0f;
    (void)210.0f;
    (void)170.0f;
    (void)10000.0f;
    (void)0.970000029f;
    (void)0.800000012f;
    (void)500.0f;
    (void)1.01999998f;
    (void)1.5f;
    (void)6.0f;
    (void)0.699999988f;
    (void)0.314159274f;
    (void)0.785398185f;
    (void)2.61799407f;
    (void)16.0f;
    (void)-0.5f;
    (void)18.0f;
    (void)1.25f;
    (void)1.00010002f;
}


static const f32 cAnimSpeeds[] = {1.0f, 1.0f, 1.0f, 1.0f};

namespace {
    static f32 cFrontAcc[] = {-0.2f, -0.2f, -0.2f, -0.1f, -0.1f, -0.1f, -0.1f, -0.1f, -0.0f, -0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                              0.0f,  0.0f,  0.0f,  0.2f,  0.2f,  0.2f,  0.3f,  0.3f,  0.4f,  0.5f,  0.5f, 0.5f, 0.5f, 0.5f,
                              0.4f,  0.4f,  0.3f,  0.3f,  0.2f,  0.2f,  0.1f,  0.1f,  0.1f,  0.1f,  0.1f, 0.0f};

    static f32 cFrontAccSpin[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 8.0f,  6.0f,  4.0f,  2.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
                                  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f};

    static f32 cFrontAccSpinSurface[] = {0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  8.0f,  6.0f,  4.0f,  2.0f,  1.0f,
                                         -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f};
    static f32 cWeightTable[] = {
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    static f32 cSwimWeightTable[] = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    static f32 cWeightTableSP[] = {0.0f, 0.0f, 0.75f, 0.25f};
    static f32 cWaterSurfaceRange = 160.0f;
};
  // namespace

f32 getSwimValue(f32 stick, u32 index, const MarioConstTable* pTable) {
    if (index == 2) {
        return pTable->mSwimRotXIne;
    }

    if (index == 3) {
        return pTable->mSwimRotZIne;
    }

    return 0.0f;
}


bool Mario::isSwimming() const {
    if (isStatusActive(MarioStatus_Swim)) {
        return true;
    }

    return isStatusActive(MarioStatus_Foo);
}


bool Mario::forceStartSwimAndShoot(const TVec3f& rKnockbackVec) {
    WaterInfo waterInfo;
    if (!MR::getWaterAreaObj(&waterInfo, mActor->mPosition)) {
        return false;
    }

    startSwim();
    mSwim->setDamage(rKnockbackVec, 30);
    return true;
}


bool Mario::forceExitSwim() {
    if (!isStatusActive(MarioStatus_Swim)) {
        return false;
    }

    mSwim->mNextAction = MarioSwim::EXIT_ACTION_SURFACE;
    closeStatus(mSwim);
    return true;
}


bool Mario::checkStartSwim() {
    if (isStatusActive(MarioStatus_Swim)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Climb)) {
        return false;
    }

    if (_10._7 && getMovementStates()._1) {
        getAnimator()->waterToGround();
        _10._7 = false;
    }

    if (mSwim->checkWaterCube(false)) {
        if (getPlayerMode() == 3) {
            addTrans(-*getGravityVec() * 100.0f, nullptr);
            if (getPlayer()->getMovementStates().jumping) {
                stopJump();
            }

            if (getPlayer()->isStatusActive(MarioStatus_Wall)) {
                getPlayer()->closeStatus(0);
            }

            if (getPlayer()->isStatusActive(MarioStatus_Blown)) {
                getPlayer()->closeStatus(0);
            }

            return false;
        }

        startSwim();
        return true;
    }

    mSwim->updateUnderWater();
    return false;
}


void Mario::startSwim() {
    if (!isStatusActive(MarioStatus_Swim)) {
        if (getPlayer()->isDamaging()) {
            mSwim->_9D = 4;
            playSound("水落下突入");
        } else if (getPlayer()->getMovementStates().jumping) {
            mSwim->_9D = 1;
            playSound("水落下突入");
        } else {
            mSwim->_9D = 0;
            playSound("水歩行突入");
        }

        changeStatus(mSwim);
        clearSlope();
        stopWalk();
        stopJump();
        mSinkTimer = 0;

        mMovementStates._3E = 0;
        mMovementStates._12 = false;
        if (mMovementStates._8 || mMovementStates._1A || mMovementStates._19) {
            addVelocity(getWallNorm(), 50.0f);
        }

        getPlayer()->lockGroundCheck(mSwim, true);
        checkBaseTransBall();
        mActor->_F44 = false;
    }
}

// ---- MarioSwimDamage.cpp ----

bool Mario::checkWaterDamage() {
    if (mMovementStates._1B) {
        mMovementStates._1B = 0;
        mSwim->addDamage(_7C4);
        return true;
    } else if (mMovementStates._27) {
        mMovementStates._27 = 0;
        mSwim->addFaint(_7C4);
        return true;
    } else {
        return false;
    }
}

// ---- MarioTalk.cpp ----

void Mario::startTalk(const LiveActor* pActor) {
    stopWalk();

    mTalk->_14 = pActor;

    changeStatus(mTalk);
}


void Mario::endTalk() {
    closeStatus(mTalk);
}


void Mario::setLastNonFixMoveVec(const TVec3f& rVec) {
    _334 = rVec;
}

// ---- MarioTask.cpp ----

static const f32 sOne = 1.0f;

static const f32 sZero = 0.0f;

static const f32 sSameDirEpsilon = 0.01f;

static const f32 sNearZeroEpsilon = 0.001f;

static const f32 sHipDropSlideAngle = 0.7853982f;

static const f32 sHipDropSlideRot = 0.10471976f;

static const f32 sHipDropSlideLen = 10.0f;

static const f32 sJumpDropSlideDotMin = 0.1f;

static const f32 sHopperJumpSlideDotMin = 0.707f;


void Mario::delTask(MarioModuleTask* pTask) {
    MarioModuleTask* next = pTask->mNext;
    if (next == nullptr) {
        return;
    }

    MarioModuleTask* head = _974;
    if (head == pTask) {
        if (next == pTask) {
            _974 = nullptr;
        } else {
            _974 = next;
        }

        pTask->mNext = nullptr;
        pTask->end();
        return;
    }

    while (true) {
        if (head == nullptr) {
            return;
        }

        MarioModuleTask* headNext = head->mNext;
        if (headNext == pTask) {
            if (next == pTask) {
                head->mNext = head;
            } else {
                head->mNext = next;
            }

            pTask->mNext = nullptr;
            pTask->end();
            return;
        }

        if (headNext == head) {
            return;
        }
    }
}


void Mario::execTask() {
    MarioModuleTask* task = _974;

    while (true) {
        if (task == nullptr) {
            return;
        }

        MarioModuleTask* next = task->mNext;
        if (!task->exec()) {
            delTask(task);
        }

        if (next == task) {
            return;
        }

        task = next;
    }
}


void Mario::drawTask() const {
    MarioModuleTask* task = _974;

    while (true) {
        if (task == nullptr) {
            return;
        }

        MarioModuleTask* next = task->mNext;
        task->draw();
        if (next == task) {
            return;
        }

        task = next;
    }
}


void Mario::initTask() {
    for (u32 i = 0; i < ARRAY_SIZE(_984); i++) {
        _984[i] = nullptr;
    }

    _974 = nullptr;
}


bool Mario::isActiveTask(Task task) {
    for (u32 i = 0; i < ARRAY_SIZE(_984); i++) {
        if (_984[i] == task) {
            return true;
        }
    }

    return false;
}


bool Mario::isActiveTaskID(u32 id) {
    for (u32 i = 0; i < ARRAY_SIZE(_984); i++) {
        if (_984[i] != nullptr) {
            if (_A08[i] & id) {
                return true;
            }
        }
    }

    return false;
}


bool Mario::pushTask(Task task, u32 flags) {
    if (isActiveTask(task)) {
        return false;
    }

    u32 index = 0;
    for (; index < ARRAY_SIZE(_984); index++) {
        if (_984[index] == nullptr) {
            break;
        }
    }

    _984[index] = task;
    _A08[index] = flags;
    return true;
}


void Mario::popTask(Task task) {
    for (u32 i = 0; i < ARRAY_SIZE(_984); i++) {
        if (_984[i] == task) {
            _984[i] = nullptr;
            return;
        }
    }
}


void Mario::callExtraTasks(u32 flags) {
    execTask();

    for (u32 i = 0; i < ARRAY_SIZE(_984); i++) {
        if (_984[i] == nullptr) {
            continue;
        }

        if (flags != 0 && !(flags & _A08[i])) {
            continue;
        }

        if (!(this->*_984[i])(_A08[i])) {
            Task task = _984[i];
            popTask(task);
        }
    }
}


void Mario::startHandy() {
    pushTask(&Mario::taskOnHandy, 0x40);
}


bool Mario::taskOnHandy(u32) {
    if (!mActor->_468) {
        stopEffect("いい汗");
        return false;
    }

    if (mTargetWalkSpeedIndex > 2) {
        playEffect("いい汗");
    } else {
        stopEffect("いい汗");
    }

    return true;
}


void Mario::startHipDropBlur() {
    if (isPlayerModeHopper()) {
        if (gIsLuigi) {
            playEffect("ホッパー尻落ルイージ");
        } else {
            playEffect("ホッパー尻落");
        }

        pushTask(&Mario::taskOnHipDropBlurHopper, 0x80);
        return;
    }

    if (gIsLuigi) {
        playEffect("尻落ルイージ");
    } else {
        playEffect("尻落");
    }

    pushTask(&Mario::taskOnHipDropBlur, 0x80);
}


bool Mario::taskOnHipDropBlurHopper(u32) {
    if (!getPlayer()->mMovementStates._B || mMovementStates._1 || isStatusActive(MarioStatus_Swim)) {
        if (gIsLuigi) {
            stopEffect("ホッパー尻落ルイージ");
        } else {
            stopEffect("ホッパー尻落");
        }

        return false;
    }

    return true;
}


bool Mario::taskOnHipDropBlur(u32) {
    if (!getPlayer()->mMovementStates._B || mMovementStates._1 || isStatusActive(MarioStatus_Swim)) {
        if (gIsLuigi) {
            stopEffect("尻落ルイージ");
        } else {
            stopEffect("尻落");
        }

        return false;
    }

    return true;
}


bool Mario::taskOnHipDropSlide(u32 flags) {
    if (flags == 0x100) {
        if (!getPlayer()->mMovementStates._B) {
            return false;
        }
    }

    if (MR::diffAngleAbs(_70C, getAirGravityVec()) > sHipDropSlideAngle) {
        return false;
    }

    if (getPlayer()->mMovementStates._1 || !getPlayer()->mMovementStates.jumping || isStatusActive(MarioStatus_Swim)) {
        return false;
    }

    HitSensor* sensor = _A68;
    bool enabled = sensor->mValidByHost != 0 && sensor->mValidBySystem != 0;

    if (!enabled) {
        return false;
    }

    TVec3f dir(mPosition);
    dir -= _A4C;
    dir.setLength(_A64);

    TVec3f dirCopy(dir);
    MR::normalizeOrZero(&dir);

    TVec3f reverseGravity = -_70C;
    if (dir.dot(reverseGravity) <= sZero) {
        return false;
    }

    Mtx rot;
    PSMTXRotAxisRad(rot, &_A58, sHipDropSlideRot);
    PSMTXMultVecSR(rot, &dir, &dir);
    dir.setLength(_A64);
    dir -= dirCopy;
    dir.setLength(sHipDropSlideLen);

    cutGravityElementFromJumpVec(true);
    cutVecElementFromJumpVec(_70C);
    addVelocity(dir);

    dirCopy.setLength(sOne);
    addVelocity(dirCopy);
    return true;
}


bool Mario::taskOnFreezeEnd(u32) {
    return mActor->finalizeFreezeModel();
}


void Mario::startHipDropSlide(const HitSensor* pSensor) {
    if (isActiveTaskID(0x100)) {
        return;
    }

    if (isActiveTaskID(0x200)) {
        return;
    }

    pushTask(&Mario::taskOnHipDropSlide, 0x100);

    TVec3f dir(mPosition - pSensor->mPosition);

    if (MR::isSameDirection(dir, getAirGravityVec(), sSameDirEpsilon) || MR::isNearZero(dir, sNearZeroEpsilon)) {
        dir = -mFrontVec;
    }

    MR::normalizeOrZero(&dir);

    _A58.cross(-getAirGravityVec(), dir);
    MR::normalizeOrZero(&_A58);

    _A68 = const_cast< HitSensor* >(pSensor);
    _A4C = pSensor->mPosition;
    _A64 = pSensor->mRadius;
    _70C = getAirGravityVec();

    changeAnimation("ヒップドロップ滑り");
}


void Mario::startJumpDropSlide(const HitSensor* pSensor) {
    if (isActiveTaskID(0x100)) {
        return;
    }

    if (isActiveTaskID(0x200)) {
        return;
    }

    TVec3f dir(mPosition - pSensor->mPosition);
    MR::normalizeOrZero(&dir);

    f32 dot = dir.dot(-getAirGravityVec());
    if (dot <= sJumpDropSlideDotMin) {
        return;
    }

    pushTask(&Mario::taskOnHipDropSlide, 0x200);

    TVec3f slideDir(mPosition - pSensor->mPosition);

    if (MR::isSameDirection(slideDir, getAirGravityVec(), sSameDirEpsilon) || MR::isNearZero(slideDir, sNearZeroEpsilon)) {
        slideDir = -mFrontVec;
    }

    MR::normalizeOrZero(&slideDir);

    _A58.cross(-getAirGravityVec(), slideDir);
    MR::normalizeOrZero(&_A58);

    _A68 = const_cast< HitSensor* >(pSensor);
    _A4C = pSensor->mPosition;
    _A64 = pSensor->mRadius;
    _70C = getAirGravityVec();

    bool changeSlideAnimation = !isPlayerModeHopper();
    if (changeSlideAnimation) {
        if (dot > sHopperJumpSlideDotMin) {
            f32 frontDot = mFrontVec.dot(slideDir);
            if (frontDot > sZero) {
                changeAnimation("ジャンプ順滑り");
            } else {
                changeAnimation("ジャンプ逆滑り");
            }
        }
    }

    mJumpVec.zero();
}


void Mario::startFreezeEnd() {
    pushTask(&Mario::taskOnFreezeEnd, 0x800);
}

// ---- MarioTeresa.cpp ----

void MarioTeresa_FORCE_MATCH_SDATA2() {
    (void)100.0f;
    (void)10.0f;
    (void)0.00100000005f;
    (void)0.00999999978f;
    (void)120.0f;
    (void)110.0f;
    (void)-0.100000001f;
    (void)0.100000001f;
    (void)50.0f;
    (void)0.939999998f;
    (void)1.5f;
    (void)0.200000003f;
    (void)200.0f;
    (void)0.99000001f;
    (void)60.0f;
    (void)-2.0f;
    (void)-0.400000006f;
    (void)-0.25f;
    (void)0.25f;
    (void)0.75f;
    (void)-0.5f;
    (void)0.0399999991f;
    (void)0.300000012f;
    (void)20.0f;
    (void)-2607.59448f;
    (void)2607.59448f;
    (void)0.707000017f;
    (void)-10.0f;
}


void Mario::startTeresaMode() {
    _418 = 0;
    _428 = 0;
    getPlayer()->mMovementStates.jumping = true;
    getPlayer()->mMovementStates._1 = false;
    getPlayer()->mMovementStates._22 = false;
    mJumpVec.zero();
    changeStatus(mTeresa);
}


bool Mario::getHitWallNorm(TVec3f* pNormal) {
    if (getPlayer()->mMovementStates._8) {
        if (!isThroughWall(mFrontWallTriangle)) {
            *pNormal = *mFrontWallTriangle->getNormal(0);
            return true;
        }
    } else if (getPlayer()->mMovementStates._19) {
        if (!isThroughWall(mBackWallTriangle)) {
            *pNormal = *mBackWallTriangle->getNormal(0);
            return true;
        }
    } else if (getPlayer()->mMovementStates._1A && !isThroughWall(mSideWallTriangle)) {
        *pNormal = *mSideWallTriangle->getNormal(0);
        return true;
    }

    return false;
}


void Mario::resetTeresaMode() {
    mActor->_F44 = true;
    _418 = 0;
    mTeresa->resetTeresaMode();
}


void Mario::doTeresaReflection(const TVec3f& rNormal, bool effect) {
    mTeresa->doTeresaReflection(rNormal, effect);
}


void Mario::startTeresaDisappear() {
    _418 = mActor->getConst().getTable()->mTeresaWallThroughTime;
    playSound("テレサ消える");
    playSound("声トルネード");
    MR::startCSSound("CS_TERESA", nullptr, 0);
    mTeresa->updateDropFlag();
    resetSleepTimer();
    if (_962 == 5 && mVerticalSpeed < 100.0f) {
        mTeresa->_34 += getAirGravityVec() * 10.0f;
    }
}


XanimeGroupInfo teresaAnimeTable[] = {
    {{"基本"}, 1.0f, 16},
    {{""}},
};


XanimeBckTable2 teresaAnime2[] = {
    {{"基本"}, {{"wait", 1.0f}, {"run", 0.0f}}},
    {{""}, {{"", 0.0f}, {nullptr, 0.0f}}},
};

// ---- MarioWait.cpp ----


bool Mario::isBlendWaitGround() const {
    return calcAngleD(_368) > 8.0f;
}


void Mario::checkSpecialWaitAnimation() {
    if (mWait->checkStart()) {
        changeStatus(mWait);
    }
}


void Mario::resetSleepTimer() {
    mWait->_16 = 0;
    if (isStatusActive(MarioStatus_Wait)) {
        closeStatus(mWait);
    }
}

// ---- MarioWalk.cpp ----

namespace {
    static f32 sSpeedTableA[] = {0.15f, 0.3f, 0.45f, 0.6f, 0.7f, 0.85f, 0.99f};
    static f32 sSpeedTableB[] = {0.02f, 0.2f, 0.4f, 0.5f, 0.65f, 0.75f, 0.98f};
    static f32 sWalkTargetTable[] = {0.0f, 0.15f, 0.25f, 0.4f, 0.5f, 0.6f, 0.8f, 1.0f};
    static f32 sWeightTable[8][4] = {{0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 0.0f},   {0.75f, 0.25f, 0.0f, 0.0f}, {0.25f, 0.75f, 0.0f, 0.0f},
                                     {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.75f, 0.25f, 0.0f}, {0.0f, 0.25f, 0.75f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}};
    static f32 sFootStep[] = {1.2f, 1.5f, 1.3f, 0.0f};
    static f32 sFootStepBeeWallWalk[] = {0.5f, 0.5f, 0.5f, 0.0f};
};
  // namespace

void Mario::stopWalk() {
    mTargetWalkSpeedIndex = 0;
    mWalkSpeed = 0.0f;
    _71E = 0;
    _71F = 0;
    _3D4 = 0;

    getAnimator()->initWalkWeight();
    getAnimator()->resetTilt();

    mMovementStates._10 = false;
    mMovementStates._23 = false;
    cancelSquatMode();

    mMovementStates.turning = false;
    _3D2 = 0;
    _8F0 = 0.0f;
    _3F4 = 0.0f;

    stopEffect("共通スリップ坂");
    stopAnimation("歩行制動ブレーキ", 1);
    stopAnimation("ブレーキ");
}


void Mario::cancelSquatMode() {
    if (!mMovementStates._A) {
        return;
    }

    calcDistToCeil(false);

    if (_20._A || mMovementStates._A) {
        _10._F = true;
    }

    mMovementStates._A = false;
    _20._A = false;
    stopAnimation("しゃがみ基本");

    if (mMovementStates._1 || !mMovementStates.jumping) {
        if (!isSwimming()) {
            changeAnimation(nullptr, "基本");
        }

        if (!isAnimationRun("サマーソルト") && _10._F) {
            if (mWalkSpeed > 0.1f) {
                changeAnimationUpperWeak("しゃがみ終了");
            } else {
                changeAnimation("しゃがみ終了");
            }
        }
    }

    mMovementStates._26 = false;
}


f32 Mario::getTargetWalkSpeed() const {
    if (mMovementStates._A) {
        return 0.0f;
    }

    f32 targetWalkSpeed = ::sWalkTargetTable[mTargetWalkSpeedIndex];

    if (mSinkTimer != 0) {
        if (checkCurrentFloorCodeSevere(25)) {
            targetWalkSpeed = 0.0f;
        } else if (checkCurrentFloorCodeSevere(31)) {
            targetWalkSpeed = 0.0f;
        } else {
            targetWalkSpeed *= 1.0f - static_cast< f32 >(mSinkTimer) / 256.0f;
        }
    }

    targetWalkSpeed *= 1.0f - _2D0;

    if (_434 != 0) {
        targetWalkSpeed *= mActor->getConst().getTable()->mItemDashRatio;
    }

    return targetWalkSpeed;
}


void Mario::decideSquatWalkAnimation() {
    const f32 walkWeights1[] = {1.0f, 0.0f, 0.0f, 0.0f};
    const f32 walkWeights2[] = {0.0f, 1.0f, 0.0f, 0.0f};
    _3F4 = 0.0f;

    if (!mMovementStates._A) {
        if (mWalkSpeed > 0.1f) {
            stopAnimation("しゃがみ", "基本");
            changeAnimationUpperWeak("しゃがみ終了");
        } else {
            changeAnimation("しゃがみ終了", "基本");
        }

        mMovementStates._26 = false;
        return;
    }

    if (isAnimationRun("壁押し", 0)) {
        stopAnimation(nullptr);
    }

    if (!_20._A || !isAnimationRun("しゃがみ基本")) {
        stopAnimation("歩行制動ブレーキ", 1);
        changeAnimation(nullptr, "しゃがみ基本");
        stopAnimation("飛び込み準備", 4);
        getAnimator()->setWalkWeight(&walkWeights1[0]);
        mMovementStates._26 = false;
        if (!mMovementStates.jumping && !isAnimationRun("サマーソルト")) {
            playSound("声しゃがむ");
        }
    }

    if (mWalkSpeed < mActor->getConst().getTable()->mSpeedSquatWalkLower) {
        if (isStickOn()) {
            mWalkSpeed = mActor->getConst().getTable()->mSquatWalkMinSpeed;
            getAnimator()->setWalkWeight(&walkWeights2[0]);
            mTargetWalkSpeedIndex = 1;
            mMovementStates._26 = true;
        } else {
            getAnimator()->setWalkWeight(&walkWeights1[0]);
            mTargetWalkSpeedIndex = 0;
            mMovementStates._26 = false;
        }
    } else if (!isStickOn()) {
        getAnimator()->setWalkWeight(&walkWeights1[0]);
        mTargetWalkSpeedIndex = 0;
    } else if (mMovementStates._26) {
        MarioConstTable* table = mActor->getConst().getTable();
        mWalkSpeed = (table->mSquatWalkMaxSpeed - table->mSquatWalkMinSpeed) * mStickPos.z + table->mSquatWalkMinSpeed;
        getAnimator()->setWalkWeight(&walkWeights2[0]);
        mTargetWalkSpeedIndex = 1;
    }

    if (isAnimationRun("しゃがみ基本")) {
        f32 animspeed = mActor->getConst().getTable()->mSquatWalkStep;
        animspeed = 60.0f / animspeed * mWalkSpeed;
        if (!mMovementStates._26) {
            animspeed = 1.0f;
        }

        getAnimator()->getXanimePlayer()->changeSpeed(animspeed);
    }

    if (getFloorCode() == 32 && mTargetWalkSpeedIndex != 0) {
        _3F4 = 0.2f;
    }

    f32 animFrame = getAnimator()->getFrame();

    if (mTargetWalkSpeedIndex != 0) {
        if (mDrawStates.mIsUnderwater || mDrawStates._13) {
            if (mPrevAnimFrame > animFrame) {
                playSound("水跳ね左足小");
                playEffect("水はね左弱");
                playEffectSRT("水波紋", 0.2f, _73C, (mGroundPos - mSideVec * 20.0f) + _368 * _738);
            }

            if (mPrevAnimFrame < 30.0f && animFrame >= 30.0f) {
                playSound("水跳ね右足小");
                playEffect("水はね右弱");
                playEffectSRT("水波紋", 0.2f, _73C, (mGroundPos + mSideVec * 20.0f) + _368 * _738);
            }
        }
    } else if (mDrawStates.mIsUnderwater && mActor->_37C % 64 == 0) {
        playEffectSRT("水波紋", 0.2f, _73C, (mShadowPos + mSideVec * (MR::getRandom() - 0.5f) * 20.0f) + _368 * _738);
    }

    mPrevAnimFrame = animFrame;
}


void Mario::decideWalkSpeed() {
    u32 i;
    bool canIndexDecrease = false;
    if (mTargetWalkSpeedIndex != 0) {
        f32 threshold = ::sSpeedTableB[mTargetWalkSpeedIndex - 1];
        if (mStickPos.z < threshold) {
            canIndexDecrease = true;
        }
    }

    for (i = 0; i < ARRAY_SIZE(::sSpeedTableA); i++) {
        if (mStickPos.z < ::sSpeedTableA[i]) {
            break;
        }
    }

    if (mTargetWalkSpeedIndex <= i || canIndexDecrease) {
        mTargetWalkSpeedIndex = i;
    }

    s32 clingNum = MR::getKarikariClingNum();
    if (clingNum != 0) {
        if (clingNum > 5) {
            clingNum = 5;
        }

        if (mTargetWalkSpeedIndex > 5 - clingNum) {
            mTargetWalkSpeedIndex = 5 - clingNum;
        }
    }

    if (mDrawStates.mIsUnderwater && mTargetWalkSpeedIndex > 6) {
        mTargetWalkSpeedIndex = 6;
    }

    if (mActor->mBeeWallWalk != 0 && mTargetWalkSpeedIndex > 4) {
        mTargetWalkSpeedIndex = 4;
    }

    if (_960 == 32) {
        if (mTargetWalkSpeedIndex > 3) {
            mTargetWalkSpeedIndex = 3;
        }

        if (mTargetWalkSpeedIndex != 0) {
            startPadVib(1);
        }

        if (mTargetWalkSpeedIndex > 2) {
            getAnimator()->getXanimePlayer()->_0C = 0.5f;
        } else {
            getAnimator()->getXanimePlayer()->_0C = 1.0f;
        }
    } else {
        f32 speed = getAnimator()->getXanimePlayer()->_0C;
        f32 new0C = speed;
        new0C += 0.1f;

        if (new0C > 1.0f) {
            new0C = 1.0f;
        }

        getAnimator()->getXanimePlayer()->_0C = new0C;
    }
}


void Mario::decideWalkAnimation() {
    if (mTargetWalkSpeedIndex == 0 && mWalkSpeed < 0.2f && isBlendWaitGround()) {
        getAnimator()->controlWaitAnimation();
    } else {
        getPlayer()->_10._F = false;

        if (getPlayer()->mTargetWalkSpeedIndex == 0 && mSwim->_1B2) {
            bool prepare = !isPlayerModeBee();
            if (prepare) {
                changeAnimation("飛び込み準備", 4);
                return;
            }
        }

        if (mSinkTimer == 0) {
            if (getPlayer()->mTargetWalkSpeedIndex != 0) {
                getAnimator()->stopWaitAnimation();
            }

            getAnimator()->setWalkWeight(::sWeightTable[mTargetWalkSpeedIndex]);
        } else {
            f32 weights[] = {0, 0, 0, 0};
            if (mTargetWalkSpeedIndex != 0) {
                f32 f1 = 1.0f;
                if (mTargetWalkSpeedIndex <= 3) {
                    if (mTargetWalkSpeedIndex == 2) {
                        f1 = 0.75f;
                    }

                    if (mTargetWalkSpeedIndex == 3) {
                        f1 = 0.5f;
                    }

                    weights[0] = f1;
                    weights[1] = 1.0f - f1;
                } else {
                    if (mTargetWalkSpeedIndex >= 5) {
                        f1 = static_cast< f32 >(mSinkTimer) / 100.0f;
                    }

                    f1 = MR::clamp(f1, 0.0f, 1.0f);
                    weights[1] = f1;
                    weights[2] = 1.0f - f1;
                }
            } else {
                weights[3] = 1.0f;
            }

            getAnimator()->setWalkWeight(&weights[0]);
        }
    }

    stopAnimation("飛び込み準備", 4);

    f32 animFrame = getAnimator()->getFrame();
    if (mTargetWalkSpeedIndex != 0) {
        if (mDrawStates.mIsUnderwater || mDrawStates._13) {
            if (mPrevAnimFrame > animFrame) {
                if (mTargetWalkSpeedIndex >= 2) {
                    if (mTargetWalkSpeedIndex < 6) {
                        playSound("水跳ね左足小");
                        playEffect("水はね左弱");
                    } else {
                        playSound("水跳ね左足");
                        playEffect("水はね左");
                    }
                }

                playEffectSRT("水波紋", 0.2f, _73C, (mGroundPos - mSideVec * 20.0f) + _368 * _738);
            }

            if (mPrevAnimFrame < 30.0f && animFrame >= 30.0f) {
                if (mTargetWalkSpeedIndex >= 2) {
                    if (mTargetWalkSpeedIndex < 6) {
                        playSound("水跳ね右足小");
                        playEffect("水はね右弱");
                    } else {
                        playSound("水跳ね右足");
                        playEffect("水はね右");
                    }
                }

                playEffectSRT("水波紋", 0.2f, _73C, (mGroundPos + mSideVec * 20.0f) + _368 * _738);
            }
        }
    } else if (mDrawStates.mIsUnderwater && mActor->_37C % 64 == 0) {
        playEffectSRT("水波紋", 0.2f, _73C, (mShadowPos + mSideVec * (MR::getRandom() - 0.5f) * 20.0f) + _368 * _738);
    }

    mPrevAnimFrame = animFrame;

    f32* footStep = ::sFootStep;
    f32 f3 = 0.0f;

    if (mActor->mBeeWallWalk != 0) {
        footStep = ::sFootStepBeeWallWalk;
    }

    for (int i = 0; i < ARRAY_SIZE(*::sWeightTable); i++) {
        f3 += footStep[i] * ::sWeightTable[mTargetWalkSpeedIndex][i];
    }

    f32 animationSpeed;

    if (f3 == 0.0f) {
        animationSpeed = 0.33f;
    } else {
        animationSpeed = 0.5f * (60.0f * ((0.01f * (mWalkSpeed * mActor->getConst().getTable()->mWalkSpeed)) / f3));
    }

    if (_8F0 > 0.0f && !mDrawStates._4) {
        animationSpeed *= 1.0f + ((_8F0 / 10.0f) * mActor->getConst().getTable()->mSlopeAnimeRatio);

        if (_8F0 > 5.0f && animationSpeed > 4.0f) {
            animationSpeed *= mActor->getConst().getTable()->mSlopeSpinAnimeRatio;
            changeAnimation("がんばり走り");
            startBas("RunSlope", false, 0.0f, 0.0f);
        }
    } else if (_3FE != 0) {
        animationSpeed *= _8F4;
    }

    f32 diffFromTargetSpeed = getTargetWalkSpeed() - mWalkSpeed;

    if (mWalkSpeed > ::sWalkTargetTable[5]) {
        f32 squared = diffFromTargetSpeed * diffFromTargetSpeed;
        f32 factor = (mWalkSpeed - ::sWalkTargetTable[5]) / (1.0f - ::sWalkTargetTable[5]);
        diffFromTargetSpeed = (diffFromTargetSpeed * (1.0f - factor)) + (squared * factor);
    } else if (mWalkSpeed < ::sWalkTargetTable[3]) {
        f32 sqrt = MR::fastSqrtf(diffFromTargetSpeed);
        f32 factor = (::sWalkTargetTable[3] - mWalkSpeed) / ::sWalkTargetTable[3];
        diffFromTargetSpeed = (diffFromTargetSpeed * factor) + (sqrt * (1.0f - factor));
    }

    f32 f4 = mActor->getConst().getTable()->mStartSpinAnimeRatio;
    f32 f5 = 1.0f + 4.0f * diffFromTargetSpeed;

    if (f5 > 2.0f) {
        f5 = 2.0f;
    }

    if (f5 < 1.0f) {
        f5 = 1.0f;
    }

    if (getTargetWalkSpeed() < ::sWalkTargetTable[6]) {
        f5 = 1.0f;
    }

    if (getFloorCode() == 32 && mTargetWalkSpeedIndex != 0) {
        f5 = 1.2f;
    }

    f32 f6 = f4 * (0.5f * (60.0f * ((getTargetWalkSpeed() * mActor->getConst().getTable()->mWalkSpeed * 0.01f) / footStep[2])));
    f32 factor2 = 2.0f - f5;

    if (animationSpeed < f6) {
        animationSpeed = animationSpeed * factor2 + f6 * (1.0f - factor2);
    }

    if (mSinkTimer != 0 && mTargetWalkSpeedIndex != 0 && animationSpeed < 1.0f) {
        animationSpeed = 1.0f;
    }

    animationSpeed *= (1.0f - _2D0);
    _3F4 = f5 - 1.0f;
    getAnimator()->getXanimePlayer()->changeSpeed(animationSpeed);

    if (!mActor->_EA4 && mTargetWalkSpeedIndex == 0 && mActor->mHealth == 1 && mActor->mMaxHealth > 2 &&
        (_970 == nullptr || strcmp(_970, "DamageWait"))) {
        getAnimator()->getXanimePlayer()->changeTrackAnimation(3, "ダメージウエイト");
        startBas("DamageWait", false, 0.0f, 0.0f);
        mActor->setBlink("DamageWait");
    }

    checkWallPush();

    f32 f7 = 0.9f;

    if (gIsLuigi) {
        f7 = 0.3f;
    }

    if (mTargetWalkSpeedIndex > 5 && mWalkSpeed > f7 && !mDrawStates._4 && !mMovementStates._35) {
        _71E = mActor->getConst().getTable()->mBrakeFirstTimer;
    }

    if (_71E != 0) {
        _71E--;
    }

    if (mTargetWalkSpeedIndex == 0) {
        if (_71E != 0 && !isSlipPolygon(mGroundPolygon) && !mDrawStates._5) {
            doBrakingAnimation();
            _71F = mActor->getConst().getTable()->mBrakeSecondTimer;
        }

        _71E = 0;

        s32 clingNum = MR::getKarikariClingNum();
        if (clingNum >= 1) {
            changeAnimationUpper("カリカリ限界");
            stopAnimation("歩行制動ブレーキ", 1);
        }

        if (clingNum < 1 && isAnimationRun("カリカリ限界")) {
            stopAnimationUpper("カリカリ限界");
        }
    }
}


void Mario::doBrakingAnimation() {
    changeAnimation("歩行制動ブレーキ", 1);
    getAnimator()->getXanimePlayer()->_20->mAttribute = 1;
    if (gIsLuigi) {
        getAnimator()->getXanimePlayer()->changeSpeed(0.5f);
    }

    playEffect("共通ブレーキ");
    _71F = 0;
}


void Mario::checkWallPush() {
    if (mTargetWalkSpeedIndex != 0 && (mMovementStates._8 || mMovementStates._32) && checkWallJumpCode()) {
        return;
    }

    f32 angle = MR::diffAngleAbsHorizontal(mFrontVec, -getWallNorm(), *getGravityVec());
    bool sideStep = false;
    f32 wallPushAngleRange = getActor()->getConst().getTable()->mWallPushAngleRange;

    bool checkAngle = mTargetWalkSpeedIndex != 0 && mMovementStates._8;

    if (checkAngle && angle < MR::toRadian(wallPushAngleRange)) {
        sideStep = true;
    }

    if (mDrawStates._A) {
        sideStep = false;
    }

    if (mDrawStates._C) {
        sideStep = false;
    }

    if (calcAngleD(getWallNorm()) < getActor()->getConst().getTable()->mForceWallAngle) {
        sideStep = false;
        if (mMovementStates._8 && mTargetWalkSpeedIndex != 0) {
            mTargetWalkSpeedIndex = 1;
            mWalkSpeed = 0.0f;
        }
    }

    if (!isAnimationRun("壁押し", 0) && sideStep) {
        doSideStep();
    }
}


void Mario::updateBrakeAnimation() {
    if (_71F != 0) {
        if (!isAnimationRun("歩行制動ブレーキ", 1)) {
            _71F = 0;
        } else {
            _71F--;
            if (!MR::isNearZero(mStickPos.z)) {
                _71F = 0;
            }

            if (_71F == 0) {
                stopAnimation(nullptr);
                stopWalk();
            }
        }
    } else if (isAnimationRun("歩行制動ブレーキ", 1) && (isAnimationTerminate(nullptr) || mTargetWalkSpeedIndex != 0)) {
        stopAnimation(nullptr);
    }

    if (!gIsLuigi) {
        return;
    }

    if (!isAnimationRun("歩行制動ブレーキ", 1) && !isAnimationRun("ブレーキ")) {
        return;
    }

    if (mMovementStates._8 || mMovementStates._32) {
        stopAnimation(nullptr);
        _71F = 0;
        _71E = 0;
        _3D0 = 0;
        _3D2 = 0;
    } else if (!MR::isDemoActive() && mMovementStates._1) {
        playSound("ルイージ滑り");
    }
}
#pragma push
#pragma opt_common_subs off
void Mario::updateWalkSpeed() {
    f32 targetWalkSpeed = getTargetWalkSpeed();
    f32 f2 = 1.0f;

    if (targetWalkSpeed == 0.0f) {
        _404 = getActor()->getConst().getTable()->mSlowStartTime;
    }

    if (_404 != 0) {
        f2 = getActor()->getConst().getTable()->mSlowStartTime - _404;
        f2 /= getActor()->getConst().getTable()->mSlowStartTime;
        _404--;
    }

    targetWalkSpeed *= f2 * f2;
    if (mMovementStates._F || isStatusActive(17)) {
        targetWalkSpeed *= getActor()->getConst().getTable()->mTornadoMultiply;
    }

    bool press = mMovementStates._A;
    if (mMovementStates._A && _1C._F) {
        press = false;
        if (_95C->getCode(_4C8) == 29) {
            press = true;
        } else if (MR::isSensorPressObj(_730)) {
            TVec3f result;
            if (MR::vecKillElement(_184, *getGravityVec(), &result) < -0.5f) {
                press = true;
            } else {
                TVec3f collisionTrans;
                TVec3f collisionPrevTrans;
                MR::extractMtxTrans(_730->mHost->mCollisionParts->mBaseMatrix, &collisionTrans);
                MR::extractMtxTrans(_730->mHost->mCollisionParts->mPrevBaseMatrix, &collisionPrevTrans);
                if (MR::vecKillElement(collisionTrans - collisionPrevTrans, *getGravityVec(), &result) < -0.5f) {
                    press = true;
                }
            }
        }

        if (mMovementStates._1 && (strstr(getGroundPolygon()->mSensor->mHost->mName, "TriPod") != nullptr ||
                                   strstr(getGroundPolygon()->mSensor->mHost->mName, "Tripod") != nullptr)) {
            press = false;
        }

        if (mMovementStates._1 && _730 == getGroundPolygon()->getSensor()) {
            press = false;
        }

        if (_730 != nullptr && press) {
            mActor->_3B4 = _368;
            mActor->setPress(0, 0);
        }
    } else {
        mMovementStates._A = false;

        if (_436 == 0 && _434 == 0 && checkSquat(false) && mSinkTimer <= 32 && !isStatusActive(31)) {
            if (!checkLockOnHoming()) {
                mMovementStates._A = true;
            }

            if (!press && mMovementStates._A && (mMovementStates._8 || mMovementStates._32)) {
                mTargetWalkSpeedIndex = 0;
                mWalkSpeed = 0.0f;
            }
        }

        if (_1C._F && !mMovementStates._A && isAnimationRun("しゃがみ終了")) {
            mMovementStates._A = true;
        }

        if (!mMovementStates._A && press) {
            mMovementStates._A = true;
            cancelSquatMode();
            _71E = 0;
        }
    }

    if (_3D0 != 0) {
        targetWalkSpeed = 0.0f;
    }

    if (mMovementStates._10) {
        targetWalkSpeed = 0.0f;
    }

    f32 inertia = decideInertia(targetWalkSpeed);

    if (!mMovementStates._A && getPlayerMode() == 1) {
        if (mWalkSpeed >= 0.9999f) {
            targetWalkSpeed *= getActor()->getConst().getTable()->mDashMultiply;
            if (targetWalkSpeed > mWalkSpeed) {
                inertia = 0.99f;
            }

            if (getPlayer()->mWalkSpeed >= 1.5f) {
                getAnimator()->getXanimePlayer()->changeTrackAnimation(2, "メタルダッシュ");
            }
        } else {
            getAnimator()->stopWaitAnimation();
        }
    }

    targetWalkSpeed = targetWalkSpeed * static_cast< f32 >(256 - mSinkTimer) / 256.0f;
    mWalkSpeed = mWalkSpeed * inertia + targetWalkSpeed * (1.0f - inertia);
}
#pragma pop

void Mario::decideOnIceAnimation() {
    if (mTargetWalkSpeedIndex == 0) {
        if (mWalkSpeed > 0.2f && !isAnimationRun("氷上慣性走行")) {
            changeAnimationWithAttr("氷上慣性走行", 1);
            mIceAnimFoot = 1 - mIceAnimFoot;
        }
    } else {
        decideWalkAnimation();
        if (mWalkSpeed > 0.7f) {
            switch (mIceAnimFoot) {
            case 0:
                getAnimator()->getXanimePlayer()->changeTrackAnimation(2, "氷上力行左");
                break;
            default:
                getAnimator()->getXanimePlayer()->changeTrackAnimation(2, "氷上力行右");
            }
        }
    }

    if (mTargetWalkSpeedIndex != 0 || mWalkSpeed <= 0.2f) {
        stopAnimation("氷上慣性走行");
    }
}


void Mario::updateOnSand() {
    if (mMovementStates._1F) {
        return;
    }

    if (mMovementStates._1) {
        if (_960 == 27 || _960 == 28) {
            if (!strcmp(MR::getSoundCodeString(_45C), "Sand") && mSinkTimer < 64) {
                mSinkTimer++;
            }
        } else if (isCurrentFloorSink()) {
            if (mSinkTimer < 255) {
                mSinkTimer++;
                if (_960 == 25 || _960 == 31) {
                    if (_960 == 31) {
                        if (mSinkTimer == 1) {
                            playSound("声沼沈み");
                        }

                        playSound("沼強制沈み");
                    } else {
                        if (mSinkTimer == 1) {
                            playSound("声砂沈み");
                        }

                        playSound("砂強制沈み");
                    }

                    stopWalk();
                    mSinkTimer = MR::clamp(static_cast< s32 >(mSinkTimer) + 3, 0, 255);

                    if (getAirGravityVec().dot(_368) > -0.99f) {
                        TVec3f vec1;
                        MR::vecKillElement(_368, getAirGravityVec(), &vec1);
                        TVec3f vec2;
                        vec2.cross(vec1, _368);
                        MR::normalize(&vec2);
                        vec1.cross(_368, vec2);
                        addVelocity(vec1 * 6.0f);
                    }
                } else {
                    playSound("砂沈み");
                }
            } else {
                mActor->forceGameOverSink();
                return;
            }

            if (!isAnimationRun(nullptr)) {
                getAnimator()->getXanimePlayer()->changeTrackAnimation(1, "埋まり歩行");
            }
        } else {
            if (mSinkTimer != 0 && !isAnimationRun(nullptr)) {
                getAnimator()->getXanimePlayer()->changeTrackAnimation(1, "歩行");
            }

            mSinkTimer = 0;
        }
    }

    if (mMovementStates.jumping) {
        mSinkTimer = 0;
    }

    if (isStatusActive(6)) {
        mSinkTimer = 0;
    }
}


void Mario::updateOnPoison() {
    if (mMovementStates._1) {
        if (checkCurrentFloorCodeSevere(18)) {
            if (mPoisonTimer == 0) {
                mActor->decLife(0);
                playSound("毒沼ダメージ");
                playSound("ダメージ");
                playSound("声小ダメージ");
                if (mActor->mHealth == 0) {
                    mActor->forceGameOver();
                }

                startCamVib(0);
                mActor->_BC4 = 1;
            }

            if (mPoisonTimer < 255) {
                mPoisonTimer++;
            } else {
                mPoisonTimer = 0;
            }
        } else {
            mPoisonTimer = 0;
        }
    } else if (mMovementStates.jumping && _3BC > 10) {
        mPoisonTimer = 0;
    }

    if (isStatusActive(6)) {
        mPoisonTimer = 0;
    }
}


void Mario::updateOnWater() {
    if (mMovementStates._1) {
        switch (_960) {
        case 20:
        case 21:
        case 22:
            touchWater();
            _738 = 20.0f;
            _73C = _368;

            switch (_962) {
            case 20:
                _738 += 20.0f;
            case 21:
                _738 += 20.0f;
            case 22:
                mDrawStates.mIsUnderwater = true;
            }
        }

        if (_960 == 23 && _962 == 23) {
            touchWater();
            mDrawStates._13 = true;
            _738 = 3.0f;
            _73C = _368;
        }
    }

    switch (_962) {
    case 20:
    case 21:
    case 22:
    case 23:
        mDrawStates._1D = true;
    }
}

// ---- MarioWall.cpp ----

bool Mario::isWalling() const {
    return getCurrentStatus() == MarioStatus_Wall;
}


void Mario::checkWallStick() {
    if (getPlayerMode() == PlayerMode_Bee) {
        checkBeeStick();
        return;
    }

    if (!checkWallJumpCode() && isEnableStickWall()) {
        if (mActor->_334) {
            TVec3f horizontal;
            f32 vertical = MR::vecKillElement(mJumpVec, mActor->_240, &horizontal);
            horizontal.setLength(-0.25f);
            mJumpVec = mActor->_240 * vertical + horizontal;
            return;
        }

        changeStatus(mWall);
        stopWalk();
        mMovementStates.jumping = false;
        mMovementStates._B = false;
        mMovementStates._6 = false;
        mMovementStates._17 = false;
        fixWallingPosition(true);
        resetTornado();
    }
}


u8 Mario::checkStickWallSide() {
    if (isStickOn()) {
        const TVec3f& direction = getWorldPadDir();
        if (direction.dot(getWallNorm()) < -0.5f) {
            return 1;
        }

        if (direction.dot(getWallNorm()) > 0.5f && !mMovementStates._1D) {
            return 2;
        }
    }

    return 0;
}


s32 Mario::checkStickFrontBack() {
    if (isStickOn()) {
        const TVec3f& direction = getWorldPadDir();
        if (direction.dot(-mFrontVec) < -0.5f) {
            return 1;
        }

        if (direction.dot(-mFrontVec) > 0.5f && !mMovementStates._1D) {
            return 2;
        }
    }

    return 0;
}


bool Mario::fixWallingPosition(bool immediate) {
    if (!fixWallingDist()) {
        return false;
    }

    fixWallingDir(immediate);
    return true;
}


void Mario::fixWallingDir(bool immediate) {
    bool back = true;
    if (isStatusActive(MarioStatus_SideStep)) {
        back = false;
    }

    if (mMovementStates._19 && back) {
        if (immediate) {
            setFrontVecKeepUp(-*mBackWallTriangle->getNormal(0));
            return;
        }

        setFrontVecKeepUp(-*mBackWallTriangle->getNormal(0), 0.1f);
        return;
    }

    if (mMovementStates._8) {
        if (immediate) {
            setFrontVecKeepUp(-*mFrontWallTriangle->getNormal(0));
            return;
        }

        setFrontVecKeepUp(-*mFrontWallTriangle->getNormal(0), 0.1f);
    }
}


bool Mario::fixWallingTop() {
    TVec3f side = getWallNorm().cross(getAirGravityVec());
    MR::normalizeOrZero(&side);
    if (MR::isNearZero(side)) {
        return false;
    }

    _75C.cross(getWallNorm(), side);
    MR::normalizeOrZero(&_75C);
    if (MR::isNearZero(_75C)) {
        return false;
    }

    getWallNorm().dot(getAirGravityVec());
    getPlayer()->forceSetHeadVecKeepSide(_75C);
    return true;
}


bool Mario::checkWallFloorCode(u16 code) const {
    if ((mMovementStates._8 || mMovementStates._32) && _964[0] == code) {
        return true;
    }

    if (mMovementStates._19 && _964[1] == code) {
        return true;
    }

    if (mMovementStates._1A && _964[2] == code) {
        return true;
    }

    return false;
}


bool Mario::checkWallCode(const char* pCode, bool frontBackOnly) const {
    if (mMovementStates._19) {
        const char* wallCode = MR::getWallCodeString(mBackWallTriangle);
        if (wallCode && strcmp(wallCode, pCode) == 0) {
            return true;
        }
    }

    if (mMovementStates._8 || mMovementStates._32) {
        const char* wallCode = MR::getWallCodeString(mFrontWallTriangle);
        if (wallCode && strcmp(wallCode, pCode) == 0) {
            return true;
        }
    }

    if (frontBackOnly) {
        return false;
    }

    if (mMovementStates._1A) {
        const char* wallCode = MR::getWallCodeString(mSideWallTriangle);
        if (wallCode && strcmp(wallCode, pCode) == 0) {
            return true;
        }
    }

    return false;
}


bool Mario::checkWallCodeNorm(u16 code, TVec3f* pNormal, bool frontBackOnly) const {
    if (mMovementStates._19) {
        u16 wallCode = MR::getWallCodeIndex(mBackWallTriangle);
        if (wallCode == code) {
            if (pNormal != nullptr) {
                *pNormal = *mBackWallTriangle->getNormal(0);
            }

            return true;
        }
    }

    if (mMovementStates._8 || mMovementStates._32) {
        u16 wallCode = MR::getWallCodeIndex(mFrontWallTriangle);
        if (wallCode == code) {
            if (pNormal != nullptr) {
                *pNormal = *mFrontWallTriangle->getNormal(0);
            }

            return true;
        }
    }

    if (frontBackOnly) {
        return false;
    }

    if (mMovementStates._1A) {
        u16 wallCode = MR::getWallCodeIndex(mSideWallTriangle);
        if (wallCode == code) {
            if (pNormal != nullptr) {
                *pNormal = *mSideWallTriangle->getNormal(0);
            }

            return true;
        }
    }

    return false;
}


void Mario::setWallCancel() {
    mWall->_1C = 1;
    mWall->_24 = getWallNorm();
}


void Mario::keepDistFrontWall() {
    if (mMovementStates._8) {
        TVec3f horizontal;
        MR::vecKillElement(_4E8 - mGroundPos, *getGravityVec(), &horizontal);
        if (horizontal.length() < 80.0f) {
            push(*mFrontWallTriangle->getNormal(0) * (80.0f - horizontal.length()));
        }
    }
}


bool Mario::isEnableStickWall() {
    switch (getPlayerMode()) {
    case PlayerMode_Teresa:
        return false;
    case PlayerMode_Foo:
        if (isStatusActive(MarioStatus_Foo)) {
            return false;
        }

        break;
    }

    if (mWall->isCancel()) {
        return false;
    }

    if (!mMovementStates.jumping || mMovementStates._1) {
        return false;
    }

    if (mMovementStates._B) {
        return false;
    }

    if (mMovementStates._F) {
        return false;
    }

    if (mDrawStates._1E) {
        return false;
    }

    if (!mMovementStates._8 && !mMovementStates._19) {
        return false;
    }

    if (mMovementStates._2 && mVerticalSpeed < mActor->getConst().getTable()->mWallStickGrHeight) {
        return false;
    }

    if (mMovementStates._15 && _4E0 < mActor->getConst().getTable()->mWallStickFrHeight && !getWallPolygon()->mSensor->isType(0x57)) {
        return false;
    }

    if (isAnimationRun("空中ひねり")) {
        if (mActor->_945 < 25) {
            return false;
        }
    } else if (mActor->isPunching() && mActor->_945 < 15) {
        return false;
    }

    if (mMovementStates._15 && mMovementStates._2 && mMovementStates._39) {
        if ((_4A4 - mShadowPos).dot(-*getGravityVec()) < mActor->getConst().getTable()->mWallStickStepHeight) {
            return false;
        }
    }

    TVec3f horizontal;
    if (mMovementStates._19) {
        MR::vecKillElement(mJumpVec, mActor->_240, &horizontal);
        if (mBackWallTriangle->getNormal(0)->dot(horizontal) > 0.0f) {
            return false;
        }
    }

    if (isInhibitWall()) {
        return false;
    }

    if (mMovementStates._8 && calcPolygonAngleD(mFrontWallTriangle) < 80.0f) {
        return false;
    }

    if (mMovementStates._19 && calcPolygonAngleD(mBackWallTriangle) < 80.0f) {
        return false;
    }

    TVec3f side = getWallNorm().cross(getAirGravityVec());
    if (MR::normalizeOrZero(&side)) {
        return false;
    }

    if (!checkWallCode("NotWallSlip", true)) {
        mMovementStates._28 = true;
    }

    if (checkWallCode("NoAction", true)) {
        return false;
    }

    if ((getPlayerMode() != PlayerMode_Ice || !getWallPolygon()->mSensor->isType(0x57)) && isRising()) {
        return false;
    }

    if (mMovementStates._19) {
        if (!isAnimationRun("壁ジャンプ") && getPlayer()->checkStickWallSide() != 1) {
            return false;
        }
    } else if (!isAnimationRun("壁ジャンプ") && !mMovementStates._9 && getPlayer()->checkStickWallSide() != 1) {
        return false;
    }

    if (mMovementStates._30) {
        return false;
    }

    if (mActor->_468) {
        if (!checkTrgA()) {
            return false;
        }

        mWall->initTriangleJump();
        mWall->startJump();
        return false;
    }

    TVec3f position(mActor->_2AC);
    f32 distance = 120.0f;
    if (mMovementStates._19) {
        distance = -120.0f;
    }

    if (!MR::isExistMapCollision(position, mFrontVec * distance)) {
        return false;
    }

    return MR::getKarikariClingNum() == 0;
}


bool Mario::fixWallingDist() {
    if (mMovementStates._19 & mMovementStates._8) {
        return true;
    }

    if (mMovementStates._19) {
        TVec3f position(_4F4 + *mBackWallTriangle->getNormal(0) * 79.0f);
        f32 distance = MR::vecKillElement(position - mPosition, *mBackWallTriangle->getNormal(0), &position);
        mPosition += *mBackWallTriangle->getNormal(0) * distance;
        if (!fixWallingTop()) {
            return false;
        }

        mPosition = _4F4 + *mBackWallTriangle->getNormal(0) * 60.0f - _75C * 80.0f;
    } else if (mMovementStates._8) {
        if (!fixWallingTop()) {
            return false;
        }

        if (MR::isExistMapCollision(mPosition, mFrontVec * 100.0f) && mFrontVec.dot(*mFrontWallTriangle->getNormal(0)) < -0.999f) {
            TVec3f position(_4E8 + *mFrontWallTriangle->getNormal(0) * 60.0f - _75C * 80.0f);
            TVec3f correction;
            MR::vecKillElement(position - mPosition, *getGravityVec(), &correction);
            if (!MR::isNearZero(correction)) {
                TVec3f relative(correction + mActor->_288);
                if (MR::abs(relative.x) < MR::abs(correction.x)) {
                    correction.x = relative.x;
                }

                if (MR::abs(relative.y) < MR::abs(correction.y)) {
                    correction.y = relative.y;
                }

                if (MR::abs(relative.z) < MR::abs(correction.z)) {
                    correction.z = relative.z;
                }

                addTrans(correction, "壁補正");
            }
        }
    }

    return true;
}


bool Mario::isInhibitWall() const {
    if (mDrawStates._2) {
        return true;
    }

    if (checkWallCode("NotWallJump", true)) {
        return true;
    }

    return checkWallCode("NoAction", true);
}


void Mario::tryWallPunch() {
    if (!mMovementStates._2B && !checkWallCodeNorm(8, nullptr, false) && (!isStatusActive(MarioStatus_Skate) || !checkWallFloorCode(0x21))) {
        if (isSwimming()) {
            if (getPlayer()->mMovementStates._8) {
                mSwim->hitWall(*mFrontWallTriangle->getNormal(0), mFrontWallTriangle->mSensor);
            }

            if (getPlayer()->mMovementStates._19) {
                mSwim->hitWall(*mBackWallTriangle->getNormal(0), mBackWallTriangle->mSensor);
            }

            if (getPlayer()->mMovementStates._1A) {
                mSwim->hitWall(*mSideWallTriangle->getNormal(0), mSideWallTriangle->mSensor);
            }
        } else {
            stopWalk();
            TVec3f velocity(getWallNorm() * mActor->getConst().getTable()->mWallSpinFlipGround);
            if (mMovementStates.jumping) {
                velocity.scale(mActor->getConst().getTable()->mWallSpinFlipAirRatio);
            }

            velocity -= *getGravityVec() * mActor->getConst().getTable()->mWallSpinHopGround;
            tryForcePowerJump(velocity, true);
            mMovementStates._2B = true;
            startPadVib(2);
            playSound("壁反射");
            playSound("声スピンキャンセル");
            playEffectTrans("壁ヒット", getWallPos());
            if (getPlayer()->mMovementStates._8) {
                sendPunch(mFrontWallTriangle->mSensor, true);
            }

            if (getPlayer()->mMovementStates._19) {
                sendPunch(mBackWallTriangle->mSensor, true);
            }

            if (getPlayer()->mMovementStates._1A) {
                sendPunch(mSideWallTriangle->mSensor, true);
            }

            mActor->_EF6 = 30;
        }
    }
}

// ---- MarioWarp.cpp ----

namespace JGeometry {
};
  // namespace JGeometry

bool Mario::doObjWarp(LiveActor* pActor) {
    if (getPlayer()->getMovementStates().debugMode) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    WarpPod* pPod = MR::getWarpPodManager()->getPairPod(pActor);

    if (pPod != nullptr) {
        mWarp->_4C = pActor;

        mVelocity.zero();

        stopWalk();

        mActor->setBlendMtxTimer(16);

        TVec3f axisY;
        TVec3f pairAxisY;

        MR::calcActorAxisY(&axisY, pActor);
        MR::calcActorAxisY(&pairAxisY, pPod);

        mWarp->_14 = pPod->mPosition + pairAxisY * 160.0f;
        mWarp->_20 = pActor->mPosition + axisY * 160.0f;

        MR::getRotatedAxisZ(&mWarp->_38, pPod->mRotation);
        MR::getRotatedAxisY(&mWarp->_2C, pPod->mRotation);

        mWarp->_45 = pPod->mVisibilityState;
        mWarp->_44 = pPod->mArg2;

        stopWalk();

        changeStatus(mWarp);

        stopJump();

        return true;
    }

    return false;
}


bool Mario::doPointWarp(const TVec3f& rVec1, const TVec3f& rVec2, s32 myInt) {
    if (getPlayer()->getMovementStates().debugMode) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    mVelocity.zero();

    stopWalk();

    mActor->setBlendMtxTimer(16);

    mWarp->_14 = rVec1;
    mWarp->_20 = mPosition;
    mWarp->_38 = rVec2;
    mWarp->_45 = 2;
    mWarp->_44 = false;

    if (myInt != 0) {
        mWarp->_48 = myInt;
    } else {
        mWarp->_48 = 90;
    }

    changeStatus(mWarp);

    stopJump();

    return true;
}


bool Mario::isVisibleRecoveryWarpBubble() const {
    if (isStatusActive(MarioStatus_Warp) && mWarp->_45 == 3) {
        return true;
    }

    return false;
}


void Mario::doCubeWarp() {
    if (getPlayer()->getMovementStates().debugMode) {
        return;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return;
    }

    WarpCube* pWarpCube = static_cast< WarpCube* >(MR::getAreaObj("WarpCube", mActor->_2A0));
    WarpCubeMgr* pWarpCubeMgr = static_cast< WarpCubeMgr* >(MR::getAreaObjManager("WarpCube"));

    if (pWarpCube != nullptr) {
        if (_56C == pWarpCube) {
            return;
        }

        WarpCube* pPairCube = pWarpCubeMgr->getPairCube(pWarpCube);
        if (pPairCube == nullptr) {
            return;
        }

        pWarpCubeMgr->startEventCamera(pWarpCube);

        TVec3f cubePos;
        MR::calcCubePos(pPairCube, &cubePos);

        mVelocity.zero();
        stopWalk();
        mActor->_38C = 10;

        _56C = pPairCube;
        mWarp->_14 = cubePos;

        MR::calcCubePos(pWarpCube, &cubePos);
        mWarp->_20 = cubePos;
        mWarp->_38.zero();
        mWarp->_2C.zero();
        mWarp->_45 = 0;
        mWarp->_44 = 0;

        stopWalk();

        changeStatus(mWarp);

        stopJump();

        return;
    }

    if (_56C != nullptr) {
        pWarpCubeMgr->setInvalidateTimer(_56C, 300);
    }

    _56C = nullptr;
}


bool Mario::doPointWarpRecovery(const TVec3f& rVec1, const TVec3f& rVec2) {
    if (getPlayer()->getMovementStates().debugMode) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    mVelocity.zero();

    stopWalk();

    mActor->setBlendMtxTimer(16);

    mWarp->_14 = rVec1;
    mWarp->_20 = mPosition;
    mWarp->_38 = rVec2;
    mWarp->_2C.zero();
    mWarp->_45 = 3;
    mWarp->_44 = false;

    changeStatus(mWarp);

    stopJump();

    MR::startGlobalEventCameraNoTarget("引き戻し");

    return true;
}
