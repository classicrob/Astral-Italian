#include "Game/Util.hpp"
#include "Game/Enemy/Garigari.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/ActorPoseTQ.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapPartsRailMover.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"

namespace NrvGarigari {
    NEW_NERVE(GarigariNrvWait, Garigari, Wait);
    NEW_NERVE(GarigariNrvRotateSign, Garigari, RotateSign);
    NEW_NERVE(GarigariNrvRotate, Garigari, Rotate);
    NEW_NERVE(GarigariNrvFall, Garigari, Fall);
    NEW_NERVE(GarigariNrvSupportFreeze, Garigari, SupportFreeze);
};  // namespace NrvGarigari

Garigari::Garigari(const char* pName) : LiveActor(pName) {
    mPose = new ActorPoseTQ();
    mRailMover = nullptr;
    mUnk9C.x = 0.0f;
    mUnk9C.y = 0.0f;
    mUnk9C.z = 0.0f;
    mType = 1;
}

// NONMATCHING: pose quaternion products scheduled/allocated in a different register order (98.9%)
void Garigari::init(const JMapInfoIter& rIter) {
    const char* pObjectName = nullptr;
    MR::getObjectName(&pObjectName, rIter);
    MR::initActor(this, rIter, pObjectName, false);

    s32 arg1 = -1;
    MR::getJMapInfoArg1WithInit(rIter, &arg1);
    bool hasGravity = arg1 != 0;
    if (hasGravity) {
        MR::onCalcGravity(this);
        MR::calcGravity(this);
        MR::initActorPoseTQ(mPose, this);
    } else {
        MR::fn_80010130(mPose, this);
        f32 x = mPose->x;
        f32 y = mPose->y;
        f32 z = mPose->z;
        f32 w = mPose->w;
        TVec3f sp1C;
        sp1C.set<f32>(2.0f * (x * y) - 2.0f * (w * z), 1.0f - 2.0f * (x * x) - 2.0f * (z * z), 2.0f * (y * z) + 2.0f * (w * x));
        TVec3f sp10 = -sp1C;
        mGravity.set<f32>(sp10);
    }

    mUnk9C.set<f32>(mPosition);
    if (MR::isConnectedWithRail(rIter)) {
        mRailMover = new MapPartsRailMover(this);
        mRailMover->init(rIter);
        fn_80332BB0(mRailMover);
        mRailMover->setStopTypeStopAtEnd();
        MR::initAndSetRailClipping(&mUnk9C, this, 100.0f, 500.0f);
        if (hasGravity) {
            MR::calcGravity(this);
        }
        fn_801821A0();
    }

    MR::getJMapInfoArg0WithInit(rIter, &mType);
    initNerve(&NrvGarigari::GarigariNrvWait::sInstance, 1);
    mBindState = new WalkerStateBindStarPointer(this, nullptr);
    MR::initActorState(this, mBindState, &NrvGarigari::GarigariNrvSupportFreeze::sInstance, "SupportFreeze");
    MR::initUseStageSwitchWriteA(this, rIter);
    if (MR::initUseStageSwitchReadB(this, rIter)) {
        MR::listenStageSwitchOnB(this, MR::FunctorV0M< Garigari*, void (Garigari::*)() >(this, &Garigari::fn_80182140));
    } else {
        if (mRailMover != nullptr) {
            mRailMover->start();
        }
        setNerve(&NrvGarigari::GarigariNrvRotate::sInstance);
    }
    MR::useStageSwitchSyncAppear(this, rIter);
}

void Garigari::control() {}

void Garigari::fn_80182140() {
    if (isNerve(&NrvGarigari::GarigariNrvWait::sInstance)) {
        MR::invalidateClipping(this);
        setNerve(&NrvGarigari::GarigariNrvRotateSign::sInstance);
    }
}

void Garigari::fn_801821A0() {
    TVec3f sp14(MR::getRailDirection(this));
    TVec3f sp8 = -mGravity;
    if (MR::isSameDirection(sp14, sp8, 0.01f)) {
        MR::calcUpVec(&sp8, this);
        if (MR::isSameDirection(sp14, sp8, 0.01f)) {
            MR::makeAxisVerticalZX(&sp8, sp14);
        }
    }
    MR::makeQuatFrontUp(mPose, sp14, sp8);
}

void Garigari::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mPose);
}

// NONMATCHING: sendMsgEnemyAttackExplosion result branch emitted as beq+b instead of bne+b, and one extra b in the blow block (98.4%)
void Garigari::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    TVec3f sp14;
    MR::calcSensorDirection(&sp14, pSender, pReceiver);
    TVec3f sp8;
    MR::calcSideVec(&sp8, this);
    f32 dot = sp8.dot(sp14);
    if (!(MR::abs(dot) > 40.0f)) {
        if (MR::isSensorPlayer(pReceiver)) {
            MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
            return;
        }
        if (MR::isSensorType(pReceiver, 0x1D)) {
            if (isNerve(&NrvGarigari::GarigariNrvRotate::sInstance)) {
                if (!MR::sendMsgEnemyAttackExplosion(pReceiver, pSender)) {
                    return;
                }
            } else {
                MR::sendMsgPush(pReceiver, pSender);
            }
        } else if (MR::isSensorEnemy(pReceiver)) {
            if (isNerve(&NrvGarigari::GarigariNrvRotate::sInstance)) {
                if (MR::sendMsgToEnemyAttackBlow(pReceiver, pSender)) {
                    MR::fn_80034820(this);
                    return;
                }
                return;
            } else {
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
    }
}

// NONMATCHING: MapPartsRailMover vtable slot 0x30 approximated as start()
void Garigari::exeRotateSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "RotateSign");
        if (mType == 0) {
            MR::startAction(this, "RotateSign");
        } else {
            MR::startAction(this, "RotateSignWood");
        }
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvGarigari::GarigariNrvRotate::sInstance);
        if (mRailMover != nullptr) {
            mRailMover->start();
        }
    }
}

// NONMATCHING: MapPartsRailMover vtable slot 0x14 approximated as isWorking(); block order guessed
void Garigari::exeRotate() {
    if (MR::isFirstStep(this)) {
        if (mType == 0) {
            MR::startAction(this, "Rotate");
        } else {
            MR::startAction(this, "RotateWood");
        }
    }
    if (mBindState->tryStartPointBind()) {
        setNerve(&NrvGarigari::GarigariNrvSupportFreeze::sInstance);
        return;
    }
    if (mRailMover != nullptr) {
        TVec3f sp14(MR::getRailDirection(this));
        fn_801821A0();
        mRailMover->movement();
        MR::setPosition(this, mRailMover->_28);
        if (MR::isRailReachedGoal(this)) {
            f32 speed = MR::getRailCoordSpeed(this);
            TVec3f sp8(sp14);
            sp8 *= speed;
            MR::setVelocity(this, sp8);
            setNerve(&NrvGarigari::GarigariNrvFall::sInstance);
            MR::offCalcGravity(this);
            MR::calcGravity(this);
            MR::invalidateClipping(this);
        }
    }
}

void Garigari::exeFall() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Fall");
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
    }
    MR::addVelocityToGravity(this, 1.0f);
    MR::attenuateVelocity(this, 0.99f);
    if (MR::isGreaterStep(this, 360)) {
        kill();
    }
}

void Garigari::exeSupportFreeze() {
    MR::updateActorStateAndNextNerve(this, &NrvGarigari::GarigariNrvRotate::sInstance);
}

inline void Garigari::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
}
