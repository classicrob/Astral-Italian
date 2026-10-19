#include "Game/Enemy/RingBeam.hpp"
#include "Game/LiveActor/ActorStateParamScale.hpp"
#include <cstdio>
#include "Game/Util.hpp"
#include "Game/AudioLib/AudAnmSoundObject.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <JSystem/JGeometry/TUtil.hpp>

namespace NrvRingBeam {
    NEW_NERVE(RingBeamNrvSpread, RingBeam, Spread);
};  // namespace NrvRingBeam

RingBeamShadowDrawer::RingBeamShadowDrawer(const LiveActor* unk0) : ShadowVolumeDrawer("影描画[リングビーム]") {
    mHost = unk0;
    mRadius = 0.0f;
}

void RingBeamShadowDrawer::drawShape() const {
    TPos3f MtxList[32];
    TPos3f baseMtxCopy;
    Mtx mtx;
    TVec3f temp;
    TVec3f temp2;
    TVec3f temp3;
    TVec3f listXDirScaledPos;
    TVec3f XDirScaled;
    TVec3f listXDirScaledNeg;
    TVec3f negYDirScaled;
    TVec3f XDir;
    TVec3f YDir;
    TVec3f Trans;
    TVec3f listXDir;
    TVec3f listYDir;
    f32 f1 = 0.2026834f;

    baseMtxCopy.setInline(mHost->getBaseMtx());
    baseMtxCopy.getXDir(XDir);
    baseMtxCopy.getYDir(YDir);
    baseMtxCopy.getTrans(Trans);
    PSMTXRotAxisRad(mtx, &YDir, f1);
    XDirScaled.scale(mRadius, XDir);
    f32 f2 = 0.0f;
    for (int i = 0; i < 32; i++) {
        temp.add(Trans, XDirScaled);
        MR::calcGravityVectorOrZero(mHost, temp, &temp2, nullptr, 0);
        if (MR::normalizeOrZero(XDirScaled, &temp3)) {
            MtxList[i].setInline(baseMtxCopy);
        } else if (MR::isSameDirection(-temp2, temp3)) {
            MR::makeMtxUpSide(&MtxList[i], -temp2, -YDir);
        } else if (temp2.dot(YDir) < f2) {
            MR::makeMtxUpSide(&MtxList[i], -temp2, temp3);
        } else {
            MR::makeMtxUpSide(&MtxList[i], -temp2, -temp3);
        }
        // almost certainly an inline
        MtxList[i][0][3] = temp.x;
        MtxList[i][1][3] = temp.y;
        MtxList[i][2][3] = temp.z;

        PSMTXMultVec(mtx, &XDirScaled, &XDirScaled);
    }

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 64);
    for (int i = 0; i < 32; i++) {
        MtxList[i].getXDir(listXDir);
        MtxList[i].getTrans(Trans);
        listXDirScaledNeg.scale(-5.0f, listXDir);
        listXDirScaledPos.scale(5.0f, listXDir);
        temp.add(Trans, listXDirScaledNeg);
        temp2.add(Trans, listXDirScaledPos);
        GXPosition3f32(temp2.x, temp2.y, temp2.z);
        GXPosition3f32(temp.x, temp.y, temp.z);
    }
    GXEnd();

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 64);
    for (int i = 0; i < 32; i++) {
        MtxList[i].getXDir(listXDir);
        MtxList[i].getYDir(listYDir);
        MtxList[i].getTrans(Trans);

        listXDirScaledNeg.scale(-5.0f, listXDir);
        listXDirScaledPos.scale(5.0f, listXDir);
        negYDirScaled.scale(500.0f, -listYDir);
        temp.add(Trans, listXDirScaledNeg);
        temp2.add(Trans, listXDirScaledPos);
        temp.add(negYDirScaled);
        temp2.add(negYDirScaled);
        GXPosition3f32(temp.x, temp.y, temp.z);
        GXPosition3f32(temp2.x, temp2.y, temp2.z);
    }
    GXEnd();

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 64);
    for (int i = 0; i < 32; i++) {
        MtxList[i].getXDir(listXDir);
        MtxList[i].getYDir(listYDir);
        MtxList[i].getTrans(Trans);

        listXDirScaledNeg.scale(-5.0f, listXDir);
        negYDirScaled.scale(500.0f, -listYDir);
        temp.add(Trans, listXDirScaledNeg);
        temp2.add(Trans, listXDirScaledNeg);
        temp2.add(negYDirScaled);
        GXPosition3f32(temp.x, temp.y, temp.z);
        GXPosition3f32(temp2.x, temp2.y, temp2.z);
    }
    GXEnd();

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 64);
    for (int i = 0; i < 32; i++) {
        MtxList[i].getXDir(listXDir);
        MtxList[i].getYDir(listYDir);
        MtxList[i].getTrans(Trans);

        listXDirScaledPos.scale(5.0f, listXDir);
        negYDirScaled.scale(500.0f, -listYDir);
        temp.add(Trans, listXDirScaledPos);
        temp2.add(Trans, listXDirScaledPos);
        temp2.add(negYDirScaled);
        GXPosition3f32(temp2.x, temp2.y, temp2.z);
        GXPosition3f32(temp.x, temp.y, temp.z);
    }
    GXEnd();
}

void RingBeamShadowDrawer::loadModelDrawMtx() const {
    GXLoadPosMtxImm(MR::getCameraViewMtx(), 0);
    GXSetCurrentMtx(0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
};

bool RingBeamShadowDrawer::isDraw() const {
    return MR::isValidDraw(mHost);
}

RingBeam::RingBeam(const char* pName, LiveActor* arg1, bool arg2, bool arg3, bool arg4, bool arg5, const char* arg6) : LiveActor(pName) {
    mHost = arg1;
    mBloomModel = nullptr;
    mBeamName = arg6 ? arg6 : "JumpBeamerBeam";
    mSpeed = 20.0f;
    mLife = 100;
    mFollowGravity = arg2;
    mUseShadowModel = arg3;
    mUseBloomModel = arg4;
    mUseColorBtp = arg5;
    mShadowDrawer = nullptr;
    mShadowModel = nullptr;
    mColor = 0.0f;
    mStartRadius = 0.0f;
    mMaxRadius = 2000.0f;
}

void RingBeam::setSpeed(f32 speed) {
    mSpeed = speed;
}

void RingBeam::setLife(s32 life) {
    mLife = life;
}

void RingBeam::init(const JMapInfoIter& rIter) {
    TPos3f mtx;
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, mBeamName, false);
    MR::connectToSceneIndirectEnemy(this);
    initHitSensor(1);
    MR::addHitSensorEye(this, "Beam", 8, 0.0f, TVec3f(0.0f, 0.0f, 0.0f));
    initEffectKeeper(1, nullptr, false);
    initSound(2, "RingBeam", &mSoundPos, TVec3f(0.0f));
    MR::invalidateClipping(this);
    initNerve(GET_NERVE(RingBeam, RingBeamNrvSpread));

    initPos(this);
    mtx.setInline(this->getBaseMtx());
    mtx.getZDir(mFront);
    mtx.getTrans(mBeamPos);
    mtx.getTrans(mSoundPos);
    if (mUseShadowModel) {
        char name[256];
        snprintf(name, sizeof(name), "%sShadow", mBeamName);
        mShadowModel = new ModelObj("影モデル", name, nullptr, 29, 41, 8, false);
        mShadowModel->initWithoutIter();
        MR::copyTransRotateScale(mShadowModel, this);
    } else {
        mShadowDrawer = new RingBeamShadowDrawer(this);
    }
    if (mUseBloomModel) {
        char name[256];
        snprintf(name, sizeof(name), "%sBloom", mBeamName);
        mBloomModel = MR::createModelObjBloomModel("リングビームブルーム", name, getBaseMtx(), true);
        MR::invalidateClipping(mBloomModel);
    }
    mParamScale = new ActorStateParamScale(this);
    makeActorDead();
}

void RingBeam::appear() {
    initPos(mHost);
    LiveActor::appear();
    if (mBloomModel) mBloomModel->appear();
}


void RingBeam::kill() {
    setNerve(GET_NERVE(RingBeam, RingBeamNrvSpread));
    initPos(mHost);
    setRadius(0.0f);
    LiveActor::kill();
}

void RingBeam::startBrk(const char* pchar) {
    MR::startBrk(this, pchar);
    if (mShadowModel) {
        MR::startBrk(mShadowModel, pchar);
    }
}

void RingBeam::startSpreadBck(const char* name) {
    MR::startBckNoInterpole(this, name);
    if (mBloomModel) MR::startBckNoInterpole(mBloomModel, name);
}
void RingBeam::setRingBckFrame(f32 frame) {
    MR::setBckFrameAndStop(this, frame);
    if (mBloomModel) MR::setBckFrameAndStop(mBloomModel, frame);
}
void RingBeam::setColorFrame(f32 color) {
    if (mUseColorBtp) {
        MR::startBtp(this, "ColorChange");
        MR::setBtpFrameAndStop(this, color);
    }
    if (mBloomModel) {
        MR::startBpk(mBloomModel, "ColorChange");
        MR::setBpkFrameAndStop(mBloomModel, color);
    }
}
void RingBeam::setRadius(f32 radius) {
    TVec3f Ydir;
    TVec3f temp2;
    TVec3f temp6;

    if (radius >= mMaxRadius) {
        radius = mMaxRadius;
    }
    if (mShadowDrawer) {
        mShadowDrawer->mRadius = radius;
    }
    MR::setSensorRadius(this, "Beam", radius);
    startSpreadBck("Spread");
    setRingBckFrame((radius / mMaxRadius) * MR::getBckFrameMax(this));
    setColorFrame(mColor);
    if (mShadowModel) {
        Triangle triangle;
        MR::extractMtxYDir(mHost->getBaseMtx(), &Ydir);
        Ydir.scale(1000.0f);

        MR::getFirstPolyOnLineToMap(&temp2, &triangle, mBeamPos, mGravityDirection * 500.0f);

        temp2 -= mGravityDirection * 10.0f;
        MR::calcPerpendicFootToLine(&temp6, temp2, mHost->mPosition + Ydir, mHost->mPosition - Ydir);
        MR::setPosition(mShadowModel, temp6);
        MR::startBckNoInterpole(mShadowModel, "Spread");
        f32 length = (temp2 - temp6).length();
        if (0.0f <= length && length < mMaxRadius) {
            MR::setBckFrameAndStop(mShadowModel, (length / mMaxRadius) * MR::getBckFrameMax(mShadowModel));
        }
    }
}

void RingBeam::initPos(const LiveActor* actor) {
    TRot3f actorBaseMtxCopy;
    TVec3f baseMtxYdir;
    actorBaseMtxCopy.setInline(actor->getBaseMtx());
    actorBaseMtxCopy.getYDir(baseMtxYdir);
    mPosition.set(actor->mPosition + baseMtxYdir * 75.0f);
}

void RingBeam::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    TRot3f mtx;
    TVec3f temp;
    TVec3f temp2;
    const TVec3f* senderPos = MR::getSensorPos(pSender);
    TVec3f sensordiff(*MR::getSensorPos(pReceiver));
    sensordiff -= *senderPos;
    MR::makeMtxTR((MtxPtr)&mtx, this);
    // this is 100% certainly an inline
    f32 x = mtx.mMtx[0][1];
    f32 y = mtx.mMtx[1][1];
    f32 z = mtx.mMtx[2][1];
    temp.set< f32 >(x, y, z);
    MR::vecKillElement(sensordiff, temp, &temp2);
    temp2.setLength(MR::getSensorRadius(pSender));

    TVec3f temp3 = temp2;
    temp3 -= sensordiff;
    f32 length = temp3.length();
    f32 receiverRadius = MR::getSensorRadius(pReceiver);

    if (!(length > 5.0f + receiverRadius) && MR::isSensorPlayerOrRide(pReceiver)) {
        MR::sendMsgEnemyAttackElectric(pReceiver, pSender);
    }
}

bool RingBeam::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return false;
}

// NONMATCHING: A temporary scalar uses f31 instead of f30.
void RingBeam::exeSpread() {
    TPos3f mtx;

    if (mParamScale->isFirstStep()) {
        if (!MR::isPowerStarGetDemoActive()) {
            MR::startActionSound(this, "EmJumpringAppear");
        }
        char* str = "Spread";
        startSpreadBck(str);
        setColorFrame(mColor);
        startBrk("Erase");
        MR::setBrkFrameAndStop(this, 0.0f);
        if (mShadowModel) {
            MR::setBrkFrameAndStop(mShadowModel, 0.0f);
        }
        TVec3f temp;
        MR::extractMtxZDir(mHost->getBaseMtx(), &mFront);
        MR::extractMtxYDir(mHost->getBaseMtx(), &temp);
        mBeamPos.set(mHost->mPosition + temp * 75.0f);
        MR::calcGravityVector(this, mBeamPos, &mGravityDirection, nullptr, 0);
        mBeamPos += mFront * mStartRadius;
    }

    TVec3f ec;
    if (mFollowGravity) {
        MR::calcGravityVector(this, mBeamPos + mFront * mSpeed, &mGravityDirection, nullptr, 0);
        MR::makeMtxUpFront(&mtx, -mGravityDirection, mFront);
        mtx.getZDir(mFront);
        TVec3f temp7;
        MR::extractMtxYDir(mHost->getBaseMtx(), &temp7);
        MR::calcPerpendicFootToLine(&ec, mBeamPos, mHost->mPosition + temp7, mHost->mPosition - temp7);
        mPosition = ec.copy();
    } else {
        ec = mPosition;
    }

    f32 rate = mParamScale->getSpeed();
    mBeamPos += (mFront * mSpeed) * rate;
    setRadius((mBeamPos - ec).length());

    if (!MR::isPowerStarGetDemoActive()) {
        TVec3f temp12 = MR::getCamPos() - ec;
        if (!MR::isNearZero(temp12)) {
            f32 radius;
            f32 float2;
            TVec3f temp13;
            MR::extractMtxYDir(mHost->getBaseMtx(), &temp13);
            TVec3f temp15 = temp12 - temp13 * (temp12.dot(temp13));
            MR::normalizeOrZero(&temp15);
            MR::normalizeOrZero(&temp12);
            float2 = 1.0f - MR::abs(temp13.dot(temp12));
            radius = MR::getSensorRadius(this, "Beam");
            mSoundPos = ec + temp15 * radius * float2;
        } else {
            mSoundPos = ec;
        }
        MR::startActionSound(this, "EmLvRingbeamMove");
    }
    if (mParamScale->isStep(mLife - (s32)MR::getBrkFrameMax(this))) {
        startBrk("Erase");
    }
    if (mParamScale->isGreaterEqualStep(mLife)) {
        kill();
    }
}

void RingBeam::setColor(f32 color) { mColor = color; }
void RingBeam::initWithParamScale(const JMapInfoIter& iter, const ActorStateParamScale* param) {
    init(iter);
    mParamScale->init(param);
}
void RingBeam::movement() {
    mParamScale->update();
    LiveActor::movement();
    mParamScale->resetHostVelocity();
}
void RingBeam::control() { mParamScale->calcHostVelocity(); }
void RingBeam::makeActorDead() {
    LiveActor::makeActorDead();
    if (mBloomModel) mBloomModel->makeActorDead();
}
void RingBeam::setStartRadius(f32 radius) { mStartRadius = radius; }
// NONMATCHING: Remaining FP register allocation and store scheduling differ.
void RingBeam::setMaxRadius(f64 radius) {
    f32 scale = (f32)radius / 2000.0f;
    mMaxRadius = radius;
    mScale.x *= scale;
    mScale.z *= scale;
    if (mBloomModel) {
        mBloomModel->mScale.x *= scale;
        mBloomModel->mScale.z *= mMaxRadius / 2000.0f;
    }
}
