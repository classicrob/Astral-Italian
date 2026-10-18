#pragma auto_inline off
#include "Game/Boss/KoopaJrCastleNerves.hpp"
#include <cmath>
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/PartsModelLodCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Ride/FluffWind.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"
#include <JSystem/J3DGraphAnimator/J3DJoint.hpp>

namespace NrvKoopaJrCastleBody { KoopaJrCastleBodyNrvWait KoopaJrCastleBodyNrvWait::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleBody { KoopaJrCastleBodyNrvStop KoopaJrCastleBodyNrvStop::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleBody { KoopaJrCastleBodyNrvMove KoopaJrCastleBodyNrvMove::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleBody { KoopaJrCastleBodyNrvDummy1 KoopaJrCastleBodyNrvDummy1::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleBody { KoopaJrCastleBodyNrvDummy2 KoopaJrCastleBodyNrvDummy2::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleBody { KoopaJrCastleBodyNrvFall KoopaJrCastleBodyNrvFall::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleBody { KoopaJrCastleBodyNrvDead KoopaJrCastleBodyNrvDead::sInstance ATTRIBUTE_WEAK; }

namespace {
    struct GearInfo {
        const char* mName;
        int mType;
    };

    static const GearInfo sGearTable[28] = {
        {"GearL1", 1},  {"GearL2", -2}, {"GearL3", 2},  {"GearL4", -2}, {"GearL5", 2},  {"GearL6", 2},  {"GearL7", 2},
        {"GearL8", 2},  {"GearL9", 2},  {"GearL10", 2}, {"GearL11", 2}, {"GearL12", 2}, {"GearL13", 2}, {"GearL14", 1},
        {"GearR1", 1},  {"GearR2", -2}, {"GearR3", 2},  {"GearR4", -2}, {"GearR5", 2},  {"GearR6", 2},  {"GearR7", 2},
        {"GearR8", 2},  {"GearR9", 2},  {"GearR10", 2}, {"GearR11", 2}, {"GearR12", 2}, {"GearR13", 2}, {"GearR14", 1},
    };

    static const char* sGearNames[11] = {"BodyGear1", "BodyGear2", "BodyGear3", "Propeller1", "Propeller2", "Propeller3",
                                         "Propeller4", "Windmill1", "Windmill2", "Windmill3", "Windmill4"};

    TVec3f sAxisY(0.0f, 1.0f, 0.0f);
    TVec3f sAxisBack(0.0f, 0.0f, -1.0f);
    TVec3f sAxisFront(0.0f, 0.0f, 1.0f);

    typedef bool (KoopaJrCastleBody::*GearCalcFunc)(TPos3f*, const JointControllerInfo&);

    typedef JointControlDelegator< KoopaJrCastleBody > BodyDelegator;

    static JointController* createBodyJoint(KoopaJrCastleBody* pHost, const char* pName, GearCalcFunc pFunc) {
        BodyDelegator* pCtrl = new BodyDelegator(pHost, pFunc, nullptr);
        MR::setJointControllerParam(pCtrl, pHost, pName);
        return pCtrl;
    }
};  // namespace

KoopaJrCastleBody::KoopaJrCastleBody(LiveActor* pHost, const char* pName)
    : PartsModel(pHost, pName, "KoopaJrCastleBody", mMtx, 0x13, false), mLowModel(nullptr), mGearCtrl(nullptr), mGearCtrlLR(nullptr),
      mCollision(nullptr), mState(nullptr), _B4(true), mGearMtx(nullptr), mJointIdx(nullptr), _F4(false), _F5(false), _F6(false),
      _F8(0.0f), _FC(0.0f), mRate(0.0f), mGearStep(0), mPropStep(0), mWindStep(0) {
    PSMTXCopy(pHost->getBaseMtx(), mMtx);
}

KoopaJrCastleBody::~KoopaJrCastleBody() {}

namespace {
    static GearCalcFunc sGearFunc0 = &KoopaJrCastleBody::calcBodyGear1;
    static GearCalcFunc sGearFunc1 = &KoopaJrCastleBody::calcBodyGear2;
    static GearCalcFunc sGearFunc2 = &KoopaJrCastleBody::calcBodyGear3;
    static GearCalcFunc sGearFunc3 = &KoopaJrCastleBody::calcPropeller;
    static GearCalcFunc sGearFunc4 = &KoopaJrCastleBody::calcPropeller;
    static GearCalcFunc sGearFunc5 = &KoopaJrCastleBody::calcPropeller;
    static GearCalcFunc sGearFunc6 = &KoopaJrCastleBody::calcPropeller;
    static GearCalcFunc sGearFunc7 = &KoopaJrCastleBody::calcWindmill;
    static GearCalcFunc sGearFunc8 = &KoopaJrCastleBody::calcWindmill;
    static GearCalcFunc sGearFunc9 = &KoopaJrCastleBody::calcWindmill;
    static GearCalcFunc sGearFunc10 = &KoopaJrCastleBody::calcWindmill;
    static GearCalcFunc sGearFunc11 = &KoopaJrCastleBody::calcGearL;
    static GearCalcFunc sGearFunc12 = &KoopaJrCastleBody::calcGearR;
}

void KoopaJrCastleBody::init(const JMapInfoIter& rInfo) {
    mGearCtrl = new JointController*[11];
    for (s32 i = 0; i < 11; i++) {
        mGearCtrl[i] = nullptr;
    }
    mGearCtrl[0] = createBodyJoint(this, sGearNames[0], sGearFunc0);
    mGearCtrl[1] = createBodyJoint(this, sGearNames[1], sGearFunc1);
    mGearCtrl[2] = createBodyJoint(this, sGearNames[2], sGearFunc2);
    mGearCtrl[3] = createBodyJoint(this, sGearNames[3], sGearFunc3);
    mGearCtrl[4] = createBodyJoint(this, sGearNames[4], sGearFunc4);
    mGearCtrl[5] = createBodyJoint(this, sGearNames[5], sGearFunc5);
    mGearCtrl[6] = createBodyJoint(this, sGearNames[6], sGearFunc6);
    mGearCtrl[7] = createBodyJoint(this, sGearNames[7], sGearFunc7);
    mGearCtrl[8] = createBodyJoint(this, sGearNames[8], sGearFunc8);
    mGearCtrl[9] = createBodyJoint(this, sGearNames[9], sGearFunc9);
    mGearCtrl[10] = createBodyJoint(this, sGearNames[10], sGearFunc10);

    mGearCtrlLR = new JointController*[28];
    for (s32 i = 0; i < 14; i++) {
        mGearCtrlLR[i] = createBodyJoint(this, sGearTable[i].mName, sGearFunc11);
    }
    for (s32 i = 14; i < 28; i++) {
        mGearCtrlLR[i] = createBodyJoint(this, sGearTable[i].mName, sGearFunc12);
    }

    MR::initJointTransform(this);
    mLowModel = MR::createModelObjMapObj("LOW\x83L\x83\x83\x83" "b\x83X\x83\x8B", "KoopaJrCastleBodyLow", nullptr);
    MR::setClippingFarMax(mLowModel);
    PartsModel::init(rInfo);

    mCollision = new CollisionParts*[2];
    mCollision[0] = MR::createCollisionPartsFromLiveActor(this, "KoopaJrCastleBody", getSensor("Collision"), getBaseMtx(),
                                                          MR::CollisionScaleType_Default);
    mCollision[1] = MR::createCollisionPartsFromLiveActor(this, "KoopaJrCastleBodyMove", getSensor("Collision"), getBaseMtx(),
                                                          MR::CollisionScaleType_Default);

    mState = new KoopaJrCastleStateBodyMove(this);

    s32 jointNum = MR::getJointNum(this);
    mJointIdx = new s32[jointNum];
    for (s32 i = 0; i < jointNum; i++) {
        mJointIdx[i] = -1;
    }
    for (s32 i = 0; i < 28; i++) {
        mJointIdx[MR::getJointIndex(this, sGearTable[i].mName)] = i;
    }

    mGearMtx = new TPos3f[28];
    for (s32 i = 0; i < 28; i++) {
        PSMTXIdentity(reinterpret_cast< MtxPtr >(&mGearMtx[i]));
    }

    initNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvWait::sInstance, 1);
    MR::initActorState(this, mState, &NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvMove::sInstance, "Move");
    makeActorAppeared();
    validateCollisionB();
    _F0 = -1600.0f;
    _B4 = false;
    _F6 = false;
}

void KoopaJrCastleBody::makeActorAppeared() {
    PartsModel::makeActorAppeared();
    MR::tryStartActionNotPlaying(this, "Wait");
    mGearStep = 0;
    mPropStep = 0;
    mWindStep = 0;
    _F8 = 0.0f;
    _FC = 0.0f;
    mRate = 0.0f;
}

// NONMATCHING: early-return branch layout
void KoopaJrCastleBody::calcAnim() {
    PartsModel::calcAnim();
    if (!MR::isDead(this) && !MR::isNoCalcAnim(this)) {
        if (!MR::isClipped(this)) {
            if (mCollision != nullptr) {
                for (int i = 0; i < 2; i++) {
                    mCollision[i]->setMtx();
                }
            }
        }
    }
}

// NONMATCHING: literal pool offsets and callee-saved register assignment
void KoopaJrCastleBody::control() {
    const char* pBck;
    if (_F4) {
        if (!MR::isDead(mLowModel)) {
            mLowModel->kill();
        }
        MR::showModelIfHidden(this);
    } else {
        if (MR::isDead(mLowModel)) {
            mLowModel->appear();
        }
        MR::hideModelAndOnCalcAnimIfShown(this);
    }
    PSMTXCopy(mHost->getBaseMtx(), mMtx);
    if (!_F5) {
        MR::addTransMtxLocalY(mMtx, _F0);
    }
    if (MR::isBindedWall(this)) {
        HitSensor* pSensor = getSensor("Collision");
        MR::sendArbitraryMsg(0xF4, MR::getWallSensor(this), pSensor);
    }
    pBck = MR::getPlayingBckName(this);
    if (MR::isEqualString(pBck, "MoveR")) {
        _F8 -= mRate;
        _FC += mRate;
        MR::repeat(_F8, 0.0f, MR::getBckFrameMax(this));
        MR::repeat(_FC, 0.0f, MR::getBckFrameMax(this));
    } else if (MR::isEqualString(pBck, "MoveL")) {
        _F8 += mRate;
        _FC -= mRate;
        MR::repeat(_F8, 0.0f, MR::getBckFrameMax(this));
        MR::repeat(_FC, 0.0f, MR::getBckFrameMax(this));
    } else if (MR::isEqualString(pBck, "MoveLR")) {
        _F8 += mRate;
        _FC += mRate;
        MR::repeat(_F8, 0.0f, MR::getBckFrameMax(this));
        MR::repeat(_FC, 0.0f, MR::getBckFrameMax(this));
    } else if (MR::isEqualString(pBck, "MoveBack")) {
        _F8 -= mRate;
        _FC -= mRate;
        MR::repeat(_F8, 0.0f, MR::getBckFrameMax(this));
        MR::repeat(_FC, 0.0f, MR::getBckFrameMax(this));
    }
    if (_F6) {
        if (++mGearStep > 500) {
            mGearStep = 0;
        }
        if (++mPropStep > 20) {
            mPropStep = 0;
        }
        if (++mWindStep > 20) {
            mWindStep = 0;
        }
        MR::startActionSound(this, "BodyGear", -1, -1, -1);
    }
}

void KoopaJrCastleBody::calcAndSetBaseMtx() {
    calcGearMtx();
    for (s32 i = 0; i < 11; i++) {
        mGearCtrl[i]->registerCallBack();
    }
    for (s32 i = 0; i < 28; i++) {
        mGearCtrlLR[i]->registerCallBack();
    }
    PartsModel::calcAndSetBaseMtx();
    if (!MR::isDead(mLowModel)) {
        MR::copyTransRotate(this, mLowModel);
    }
}

void KoopaJrCastleBody::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::offBind(mHost);
        _F0 = -1600.0f;
    }
}

#pragma dont_inline on
void KoopaJrCastleBody::exeFall() {
    MR::offBind(mHost);
    _F0 = MR::calcNerveValue(this, 120, -1600.0f, 0.0f);
}
#pragma dont_inline off

void KoopaJrCastleBody::exeDead() {
    if (MR::isFirstStep(this)) {
        for (int i = 0; i < 2; i++) {
            MR::invalidateCollisionParts(mCollision[i]);
        }
    }
}

void KoopaJrCastleBody::setNerveDummy1() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvDummy1::sInstance);
}

void KoopaJrCastleBody::setNerveDummy2() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvDummy2::sInstance);
}

void KoopaJrCastleBody::setNerveFall() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvFall::sInstance);
}

void KoopaJrCastleBody::setNerveDead() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvDead::sInstance);
}

void KoopaJrCastleBody::setNerveWait() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvWait::sInstance);
}

void KoopaJrCastleBody::setNerveStop() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvStop::sInstance);
}

void KoopaJrCastleBody::startMove0() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvMove::sInstance);
    mState->setMode0();
}

void KoopaJrCastleBody::startMove1() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvMove::sInstance);
    mState->setMode1();
}

void KoopaJrCastleBody::startMove2() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvMove::sInstance);
    mState->setMode2();
}

void KoopaJrCastleBody::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvDead::sInstance)) {
        if (MR::isSensorType(pSender, 0x36)) {
            if (MR::isSensorPlayerOrRide(pReceiver)) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        } else {
            mHost->attackSensor(pSender, pReceiver);
        }
    }
}

bool KoopaJrCastleBody::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvDead::sInstance)) {
        return false;
    }
    return mHost->receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

bool KoopaJrCastleBody::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvDead::sInstance)) {
        return false;
    }
    return mHost->receiveOtherMsg(msg, pSender, pReceiver);
}

void KoopaJrCastleBody::validateCollisionA() {
    if (!MR::isOnPlayer(getSensor("Collision"))) {
        MR::invalidateCollisionParts(mCollision[0]);
        MR::validateCollisionParts(mCollision[1]);
        mCollision[0]->resetAllMtx();
        mCollision[1]->resetAllMtx();
    }
}

void KoopaJrCastleBody::validateCollisionB() {
    if (!MR::isOnPlayer(getSensor("Collision"))) {
        MR::validateCollisionParts(mCollision[0]);
        MR::invalidateCollisionParts(mCollision[1]);
        mCollision[0]->resetAllMtx();
        mCollision[1]->resetAllMtx();
    }
}

void KoopaJrCastleBody::onFlagF4() {
    _F4 = true;
}

void KoopaJrCastleBody::offFlagF4() {
    _F4 = false;
}

void KoopaJrCastleBody::onFlagF5() {
    _F5 = true;
    MR::setBckRate(this, 1.0f);
}

void KoopaJrCastleBody::offFlagF5() {
    _F5 = false;
    MR::startAction(this, "Wait");
}

void KoopaJrCastleBody::onFlagF6() {
    _F6 = true;
}

void KoopaJrCastleBody::resetF0() {
    _F0 = 0.0f;
}

bool KoopaJrCastleBody::isMove() const {
    return isNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvMove::sInstance);
}

bool KoopaJrCastleBody::isMoveState() const {
    bool ret = false;
    if (isNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvMove::sInstance) && mState->isMoving()) {
        ret = true;
    }
    return ret;
}

bool KoopaJrCastleBody::getStateFlag() const {
    return mState->getFlag();
}

// NONMATCHING: vector temporary stack slots
bool KoopaJrCastleBody::calcBodyGear1(TPos3f* pMtx, const JointControllerInfo&) {
    MtxPtr pDst = reinterpret_cast< MtxPtr >(pMtx);
    f32 step = (f32)mGearStep;
    TVec3f sp8(sAxisY);
    sp8.scale(360.0f);
    TVec3f sp14(sp8);
    sp14.scale(step);
    TVec3f sp20(sp14);
    s32 gearMax = 500;
    sp20.scale(1.0f / (f32)gearMax);
    Mtx rot;
    PSMTXIdentity(rot);
    MR::makeMtxRotate(rot, sp20);
    PSMTXConcat(pDst, rot, pDst);
    return true;
}

// NONMATCHING: vector temporary stack slots and load scheduling
bool KoopaJrCastleBody::calcBodyGear2(TPos3f* pMtx, const JointControllerInfo&) {
    MtxPtr pDst = reinterpret_cast< MtxPtr >(pMtx);
    TVec3f sp8 = -sAxisY;
    f32 step = (f32)mGearStep;
    TVec3f sp14(sp8);
    sp14.scale(360.0f);
    TVec3f sp20(sp14);
    sp20.scale(step);
    TVec3f sp2C(sp20);
    s32 gearMax = 500;
    sp2C.scale(1.0f / (f32)gearMax);
    Mtx rot;
    PSMTXIdentity(rot);
    MR::makeMtxRotate(rot, sp2C);
    PSMTXConcat(pDst, rot, pDst);
    return true;
}

bool KoopaJrCastleBody::calcBodyGear3(TPos3f* pMtx, const JointControllerInfo&) {
    MtxPtr pDst = reinterpret_cast< MtxPtr >(pMtx);
    f32 angle = -KoopaJrCastleFunction::getFireBarB0();
    TVec3f sp8(sAxisY);
    sp8.scale(angle);
    Mtx rot;
    PSMTXIdentity(rot);
    MR::makeMtxRotate(rot, sp8);
    PSMTXConcat(pDst, rot, pDst);
    return true;
}

// NONMATCHING: vector temporary stack slots
bool KoopaJrCastleBody::calcPropeller(TPos3f* pMtx, const JointControllerInfo&) {
    MtxPtr pDst = reinterpret_cast< MtxPtr >(pMtx);
    f32 step = (f32)mPropStep;
    TVec3f sp8(sAxisBack);
    sp8.scale(360.0f);
    TVec3f sp14(sp8);
    sp14.scale(step);
    TVec3f sp20(sp14);
    s32 propMax = 20;
    sp20.scale(1.0f / (f32)propMax);
    Mtx rot;
    PSMTXIdentity(rot);
    MR::makeMtxRotate(rot, sp20);
    PSMTXConcat(pDst, rot, pDst);
    return true;
}

// NONMATCHING: vector temporary stack slots
bool KoopaJrCastleBody::calcWindmill(TPos3f* pMtx, const JointControllerInfo&) {
    MtxPtr pDst = reinterpret_cast< MtxPtr >(pMtx);
    f32 step = (f32)mWindStep;
    TVec3f sp8(sAxisFront);
    sp8.scale(360.0f);
    TVec3f sp14(sp8);
    sp14.scale(step);
    TVec3f sp20(sp14);
    s32 propMax = 20;
    sp20.scale(1.0f / (f32)propMax);
    Mtx rot;
    PSMTXIdentity(rot);
    MR::makeMtxRotate(rot, sp20);
    PSMTXConcat(pDst, rot, pDst);
    return true;
}

bool KoopaJrCastleBody::calcGearL(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    MtxPtr pDst = reinterpret_cast< MtxPtr >(pMtx);
    if (_B4) {
        return false;
    }
    u16 jntNo = rInfo.mJoint->getJntNo();
    if (mJointIdx[jntNo] < 0) {
        return false;
    }
    TVec3f trans;
    MR::extractMtxTrans(pDst, &trans);
    MtxPtr jointMtx = MR::getJointMtx(this, "GearL");
    PSMTXConcat(jointMtx, reinterpret_cast< MtxPtr >(&mGearMtx[mJointIdx[jntNo]]), pDst);
    MR::setMtxTrans(pDst, trans.x, trans.y, trans.z);
    return true;
}

bool KoopaJrCastleBody::calcGearR(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    MtxPtr pDst = reinterpret_cast< MtxPtr >(pMtx);
    if (_B4) {
        return false;
    }
    u16 jntNo = rInfo.mJoint->getJntNo();
    if (mJointIdx[jntNo] < 0) {
        return false;
    }
    TVec3f trans;
    MR::extractMtxTrans(pDst, &trans);
    MtxPtr jointMtx = MR::getJointMtx(this, "GearR");
    PSMTXConcat(jointMtx, reinterpret_cast< MtxPtr >(&mGearMtx[mJointIdx[jntNo]]), pDst);
    MR::setMtxTrans(pDst, trans.x, trans.y, trans.z);
    return true;
}

void KoopaJrCastleBody::calcGearMtx() {
    if (MR::getPlayingBckName(this) != nullptr) {
        f32 f30 = _F8 / 480.0f;
        f32 f31 = _FC / 480.0f;
        Mtx sp70;
        Mtx sp40;
        Mtx sp10;

        PSMTXIdentity(sp70);
        PSMTXIdentity(sp40);
        PSMTXIdentity(sp10);

        f32 angle = 360.0f * f30;
        MR::repeatDegree(&angle);
        MR::makeMtxRotate(sp70, angle, 0.0f, 0.0f);
        angle = 2.0f * (360.0f * f30);
        MR::repeatDegree(&angle);
        MR::makeMtxRotate(sp40, angle, 0.0f, 0.0f);
        angle = -2.0f * (360.0f * f30);
        MR::repeatDegree(&angle);
        MR::makeMtxRotate(sp10, angle, 0.0f, 0.0f);

        for (s32 i = 0; i < 14; i++) {
            switch (sGearTable[i].mType) {
            case 1:
                PSMTXCopy(sp70, reinterpret_cast< MtxPtr >(&mGearMtx[i]));
                break;
            case 2:
                PSMTXCopy(sp40, reinterpret_cast< MtxPtr >(&mGearMtx[i]));
                break;
            case -2:
                PSMTXCopy(sp10, reinterpret_cast< MtxPtr >(&mGearMtx[i]));
                break;
            }
        }

        angle = 360.0f * f31;
        MR::repeatDegree(&angle);
        MR::makeMtxRotate(sp70, angle, 0.0f, 0.0f);
        angle = 2.0f * (360.0f * f31);
        MR::repeatDegree(&angle);
        MR::makeMtxRotate(sp40, angle, 0.0f, 0.0f);
        angle = -2.0f * (360.0f * f31);
        MR::repeatDegree(&angle);
        MR::makeMtxRotate(sp10, angle, 0.0f, 0.0f);

        for (s32 i = 14; i < 28; i++) {
            switch (sGearTable[i].mType) {
            case 1:
                PSMTXCopy(sp70, reinterpret_cast< MtxPtr >(&mGearMtx[i]));
                break;
            case 2:
                PSMTXCopy(sp40, reinterpret_cast< MtxPtr >(&mGearMtx[i]));
                break;
            case -2:
                PSMTXCopy(sp10, reinterpret_cast< MtxPtr >(&mGearMtx[i]));
                break;
            }
        }
    }
}

void KoopaJrCastleBody::startMoveSound() {
    MR::startActionSound(this, "MoveLR", (s32)(100.0f * mRate), -1, -1);
}
