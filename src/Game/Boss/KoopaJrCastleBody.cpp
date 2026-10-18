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

    typedef JointControlDelegator< KoopaJrCastleBody > BodyDelegator;

    static JointController* createBodyJoint(KoopaJrCastleBody* pHost, const char* pName, bool (KoopaJrCastleBody::*pFunc)(TPos3f*, const JointControllerInfo&)) {
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

void KoopaJrCastleBody::control() {
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
    const char* pBck = MR::getPlayingBckName(this);
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

void KoopaJrCastleBody::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::offBind(mHost);
        _F0 = -1600.0f;
    }
}

void KoopaJrCastleBody::exeFall() {
    MR::offBind(mHost);
    _F0 = MR::calcNerveValue(this, 120, -1600.0f, 0.0f);
}

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
