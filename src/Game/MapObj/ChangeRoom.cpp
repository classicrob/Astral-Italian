#include "Game/MapObj/ChangeRoom.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

namespace GameSequenceFunction {
    void prepareStageResetChangeRoomMario(s32);
    void prepareStageResetChangeRoomLuigi(s32);
};  // namespace GameSequenceFunction

extern s32 lbl_807E3088;

namespace NrvChangeRoom {
    NEW_NERVE(ChangeRoomNrvWait, ChangeRoom, Wait);
    NEW_NERVE(ChangeRoomNrvOpen, ChangeRoom, Open);
    NEW_NERVE(ChangeRoomNrvOpenWait, ChangeRoom, OpenWait);

    class ChangeRoomNrvEnter : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            ChangeRoom* actor = reinterpret_cast< ChangeRoom* >(pSpine->mExecutor);
            if (MR::isFirstStep(actor)) {
                actor->startEnterSoundAndPrepareReset();
            }
        };

        static ChangeRoomNrvEnter sInstance;
    };
    ChangeRoomNrvEnter ChangeRoomNrvEnter::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(ChangeRoomNrvEnterDemo, ChangeRoom, EnterDemo);
};  // namespace NrvChangeRoom

namespace {
    static TVec3f sJumpVelocity(0.0f, 25.0f, 0.0f);
}

ChangeRoom::ChangeRoom(const char* pName) : LiveActor(pName), mCollisionParts() {
}

void ChangeRoom::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    if (MR::tryRegisterDemoCast(this, "\x83\x60\x83\x46\x83\x93\x83\x57\x83\x8B\x81\x5B\x83\x80\x82\xA9\x82\xE7\x83\x58\x83\x5E\x81\x5B\x83\x67", rIter)) {
        MR::registerDemoActionNerve(this, &NrvChangeRoom::ChangeRoomNrvEnterDemo::sInstance, nullptr);
    }
    MR::tryRegisterDemoCast(this, rIter);

    mCollisionParts = new CollisionParts*[2];
    mCollisionParts[0] = MR::createCollisionPartsFromLiveActor(this, "ChangeRoom", getSensor("Collision"), getBaseMtx(), MR::CollisionScaleType_Default);
    mCollisionParts[1] = MR::createCollisionPartsFromLiveActor(this, "ChangeRoomOpen", getSensor("Collision"), getBaseMtx(), MR::CollisionScaleType_Default);

    TPos3f mtx;
    mtx.identity();
    MR::makeMtxTR(mtx, this);
    TVec3f front;
    front.set< f32 >(mtx.mMtx[0][2], mtx.mMtx[1][2], mtx.mMtx[2][2]);
    TVec3f up;
    up.x = mGravity.x * -1.0f;
    up.y = mGravity.y * -1.0f;
    up.z = mGravity.z * -1.0f;
    TPos3f base;
    base.identity();
    MR::makeMtxUpFrontPos(&base, up, front, mPosition);
    MR::setBaseTRMtx(this, base);

    initNerve(&NrvChangeRoom::ChangeRoomNrvWait::sInstance, 0);
    if (MR::isOnGameEventFlagNormalEnding()) {
        setNerve(&NrvChangeRoom::ChangeRoomNrvOpen::sInstance);
    } else {
        setNerve(&NrvChangeRoom::ChangeRoomNrvWait::sInstance);
    }
    MR::validateCollisionParts(mCollisionParts[0]);
    MR::invalidateCollisionParts(mCollisionParts[1]);
    resetCollisionPartsMtx();
    MR::useStageSwitchSyncAppear(this, rIter);
    MR::startAction(this, "Wait");
    MR::setBckFrameEndAndStop(this);

    if (MR::isOnGameEventFlagNormalEnding()) {
        if (MR::isPlayerLuigi()) {
            MR::startBrkAndSetFrameAndStop(this, "ChangeColors", 1.0f);
            MR::startBtkAndSetFrameAndStop(this, "ChangeMarks", 1.0f);
        } else {
            MR::startBrkAndSetFrameAndStop(this, "ChangeColors", 2.0f);
            MR::startBtkAndSetFrameAndStop(this, "ChangeMarks", 2.0f);
        }
    } else {
        MR::startBrkAndSetFrameAndStop(this, "ChangeColors", 0.0f);
        MR::startBtkAndSetFrameAndStop(this, "ChangeMarks", 0.0f);
    }
}

void ChangeRoom::makeActorDead() {
    LiveActor::makeActorDead();
    MR::invalidateCollisionParts(mCollisionParts[0]);
    MR::invalidateCollisionParts(mCollisionParts[1]);
}

void ChangeRoom::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateCollisionParts(mCollisionParts[0]);
        MR::invalidateCollisionParts(mCollisionParts[1]);
        MR::invalidateHitSensors(this);
        resetCollisionPartsMtx();
    }
}

void ChangeRoom::exeOpen() {
    if (MR::isFirstStep(this)) {
        MR::validateCollisionParts(mCollisionParts[0]);
        MR::invalidateCollisionParts(mCollisionParts[1]);
        MR::invalidateHitSensors(this);
        MR::validateHitSensor(this, "Push");
        resetCollisionPartsMtx();
    }
    if (MR::isOnSwitchA(this) && MR::isActionEnd(this)) {
        setNerve(&NrvChangeRoom::ChangeRoomNrvOpenWait::sInstance);
    }
}

void ChangeRoom::exeOpenWait() {
    if (MR::isFirstStep(this)) {
        MR::invalidateCollisionParts(mCollisionParts[0]);
        MR::validateCollisionParts(mCollisionParts[1]);
        MR::validateHitSensors(this);
        MR::validateHitSensor(this, "Push");
        resetCollisionPartsMtx();
        f32 frame = MR::getBckFrame(this);
        MR::startAction(this, "DoorOpen");
        MR::setBckFrame(this, MR::getBckFrameMax(this) - frame);
    }
    if (!MR::isOnSwitchA(this) && MR::isActionEnd(this)) {
        f32 frame = MR::getBckFrame(this);
        MR::startAction(this, "DoorClose");
        MR::setBckFrame(this, MR::getBckFrameMax(this) - frame);
        setNerve(&NrvChangeRoom::ChangeRoomNrvOpen::sInstance);
    }
}

void ChangeRoom::exeEnterDemo() {
    s32 step = lbl_807E3088;
    if (MR::isStep(this, step)) {
        if (MR::isPlayerLuigi()) {
            MR::startActionSound(this, "LuigiOut", -1, -1, -1);
        } else {
            MR::startActionSound(this, "MarioOut", -1, -1, -1);
        }
        MR::jumpPlayer(sJumpVelocity);
        MR::invalidateCollisionParts(mCollisionParts[0]);
        MR::invalidateCollisionParts(mCollisionParts[1]);
        MR::startBckPlayerJ("\x83\x57\x83\x83\x83\x93\x83\x76");
    }
    if (MR::isStep(this, step + 10)) {
        setNerve(&NrvChangeRoom::ChangeRoomNrvOpen::sInstance);
    }
}

void ChangeRoom::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "Push")) {
        if (MR::isSensorYoshi(pReceiver)) {
            TVec3f diff;
            diff = *MR::getSensorPos(pSender);
            diff -= *MR::getSensorPos(pReceiver);
            TVec3f up;
            up.negate(mGravity);
            diff.scale(up.dot(diff), up);
            if (PSVECMag(&diff) < 300.0f) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
    } else if (MR::isSensorPlayerOrRide(pReceiver)) {
        if (isNerve(&NrvChangeRoom::ChangeRoomNrvOpenWait::sInstance)) {
            setNerve(&NrvChangeRoom::ChangeRoomNrvEnter::sInstance);
        }
    }
}

void ChangeRoom::calcAnim() {
    LiveActor::calcAnim();
    if (MR::isDead(this) || MR::isNoCalcAnim(this) || MR::isClipped(this)) {
        return;
    }
    if (mCollisionParts) {
        for (s32 i = 0; i < 2; i++) {
            mCollisionParts[i]->setMtx();
        }
    }
}

void ChangeRoom::startEnterSoundAndPrepareReset() {
    if (MR::isPlayerLuigi()) {
        MR::startActionSound(this, "LuigiIn", -1, -1, -1);
        GameSequenceFunction::prepareStageResetChangeRoomMario(2);
    } else {
        MR::startActionSound(this, "MarioIn", -1, -1, -1);
        GameSequenceFunction::prepareStageResetChangeRoomLuigi(2);
    }
}

void ChangeRoom::resetCollisionPartsMtx() {
    for (s32 i = 0; i < 2; i++) {
        CollisionParts* parts = mCollisionParts[i];
        if (parts->_0) {
            parts->resetAllMtx();
        } else {
            TPos3f mtx(getBaseMtx());
            parts->resetAllMtx(mtx);
        }
    }
}

ChangeRoom::~ChangeRoom() {
}
