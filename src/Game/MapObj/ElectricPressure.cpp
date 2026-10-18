#pragma auto_inline off
#include "Game/MapObj/ElectricPressure.hpp"
#include "Game/Enemy/ElectricPressureBullet.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <JSystem/JMath.hpp>
#include <revolution/types.h>


namespace NrvElectricPressure {
    // ShotStart and Shot are declared first so the inline executes of WaitFirst and Wait can reference them.
    // Their sInstance definitions are placed below in sinit order.
    class ElectricPressureNrvShotStart : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            ElectricPressure* actor = reinterpret_cast< ElectricPressure* >(pSpine->mExecutor);
            actor->exeShotStart();
        };

        static ElectricPressureNrvShotStart sInstance;
    };

    class ElectricPressureNrvShot : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            ElectricPressure* actor = reinterpret_cast< ElectricPressure* >(pSpine->mExecutor);
            actor->exeShot();
        };

        static ElectricPressureNrvShot sInstance;
    };

    class ElectricPressureNrvWaitFirst : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            ElectricPressure* actor = reinterpret_cast< ElectricPressure* >(pSpine->mExecutor);
            if (MR::isOnSwitchB(actor)) {
                actor->setNerve(&ElectricPressureNrvShotStart::sInstance);
            }
        };

        static ElectricPressureNrvWaitFirst sInstance;
    };

    class ElectricPressureNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            ElectricPressure* actor = reinterpret_cast< ElectricPressure* >(pSpine->mExecutor);
            if (MR::isStep(actor, actor->mWaitFrames)) {
                actor->setNerve(&ElectricPressureNrvShotStart::sInstance);
            }
        };

        static ElectricPressureNrvWait sInstance;
    };

    ElectricPressureNrvWaitFirst ElectricPressureNrvWaitFirst::sInstance ATTRIBUTE_WEAK;
    ElectricPressureNrvWait ElectricPressureNrvWait::sInstance ATTRIBUTE_WEAK;
    ElectricPressureNrvShotStart ElectricPressureNrvShotStart::sInstance ATTRIBUTE_WEAK;
    ElectricPressureNrvShot ElectricPressureNrvShot::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvElectricPressure

ElectricPressure::ElectricPressure(const char* pName) : LiveActor(pName) {
    mBullets = nullptr;
    mWaitFrames = 90;
    mIsBulletFlag = false;
}

void ElectricPressure::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "ElectricPressure", false);
    MR::getJMapInfoArg1NoInit(rIter, &mWaitFrames);
    MR::getJMapInfoArg7NoInit(rIter, &mIsBulletFlag);

    mBullets = new ElectricPressureBullet*[5];
    for (s32 i = 0; i < 5; i++) {
        mBullets[i] = new ElectricPressureBullet("\203G\203\214\203N\203g\203\212\203b\203N\203o\203Y\201[\203J\202\314\222e");
        mBullets[i]->initWithoutIter();
    }

    if (MR::isValidSwitchB(this)) {
        initNerve(&NrvElectricPressure::ElectricPressureNrvWaitFirst::sInstance, 0);
    } else {
        initNerve(&NrvElectricPressure::ElectricPressureNrvWait::sInstance, 0);
    }
    makeActorAppeared();
}

void ElectricPressure::exeShotStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ShotStart");
    }

    if (MR::isActionEnd(this)) {
        setNerve(&NrvElectricPressure::ElectricPressureNrvShot::sInstance);
    }
}

void ElectricPressure::exeShot() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shot");
        shoot();
    }

    if (MR::isActionEnd(this)) {
        if (MR::isValidSwitchB(this) && !MR::isOnSwitchB(this)) {
            setNerve(&NrvElectricPressure::ElectricPressureNrvWaitFirst::sInstance);
        } else {
            setNerve(&NrvElectricPressure::ElectricPressureNrvWait::sInstance);
        }
    }
}

void ElectricPressure::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorMapObj(pSender)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool ElectricPressure::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    return MR::isMsgStarPieceReflect(msg);
}

bool ElectricPressure::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) {
    return msg == 0xCF;
}

// NONMATCHING: 99.3%; all calls/constants/offsets match, only the float load order inside the axisX/axisY/axisZ set<f32>() arg setup differs (target loads the 3rd component first).
void ElectricPressure::shoot() {
    ElectricPressureBullet* pBullet = nullptr;
    for (s32 i = 0; i < 5; i++) {
        if (MR::isDead(mBullets[i])) {
            pBullet = mBullets[i];
            break;
        }
    }

    if (pBullet != nullptr) {
        TPos3f mtx(MR::getJointMtx(this, "Cannon1"));

        TVec3f axisX(0.0f, 0.0f, 0.0f);
        axisX.set<f32>(mtx.mMtx[0][0], mtx.mMtx[1][0], mtx.mMtx[2][0]);
        TVec3f pos(0.0f, 0.0f, 0.0f);
        pos.set<f32>(mtx.mMtx[0][3], mtx.mMtx[1][3], mtx.mMtx[2][3]);
        JMAVECScaleAdd(&axisX, &pos, &pos, 200.0f);

        TVec3f axisY(0.0f, 0.0f, 0.0f);
        TVec3f axisZ(0.0f, 0.0f, 0.0f);
        axisY.set<f32>(mtx.mMtx[0][1], mtx.mMtx[1][1], mtx.mMtx[2][1]);
        axisZ.set<f32>(mtx.mMtx[0][2], mtx.mMtx[1][2], mtx.mMtx[2][2]);

        MR::makeMtxFrontUpPos(&mtx, axisX, axisY, pos);
        pBullet->shotElectricBullet(this, mtx, 12.0f, mIsBulletFlag);
    }
}

ElectricPressure::~ElectricPressure() {}
