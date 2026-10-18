#include "Game/MapObj/ManholeCover.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/MapObjConnector.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"

namespace NrvManholeCover {
    class HostTypeWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static HostTypeWait sInstance;
    };
    HostTypeWait HostTypeWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(HostTypeOpen, ManholeCover, Open);
    NEW_NERVE(HostTypeRattle, ManholeCover, Rattle);
};  // namespace NrvManholeCover

ManholeCover::ManholeCover(const char* pName) : LiveActor(pName), mConnector(nullptr), _94(0), _95(0) {
    mConnector = new MapObjConnector(this);
}

// NONMATCHING: target stores the negated axis through a stack temp (psq_st to 0x8, then field copies); MWCC folds the temp away in every form tried
void ManholeCover::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    initNerve(GET_NERVE(ManholeCover, HostTypeWait), 0);

    TVec3f axisY;
    MR::calcActorAxisY(&axisY, this);
    mGravity = -axisY;

    makeActorAppeared();
}

void ManholeCover::initAfterPlacement() {
    mConnector->attachToBack();
}

void ManholeCover::control() {
    _94 = _95;
    _95 = 0;
}

void ManholeCover::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    mConnector->connect();
}

bool ManholeCover::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDropFloor(msg) || MR::isMsgPlayerUpperPunch(msg)) {
        if (isNerve(GET_NERVE(ManholeCover, HostTypeWait)) || isNerve(GET_NERVE(ManholeCover, HostTypeRattle)) ||
            (isNerve(GET_NERVE(ManholeCover, HostTypeOpen)) && MR::isGreaterEqualStep(this, 20))) {
            f32 distance, dot;
            MR::calcSensorDistanceGravity(&distance, &dot, pSender, pReceiver, mGravity);
            if (distance < 230.0f) {
                if (_94 != 0) {
                    setNerve(GET_NERVE(ManholeCover, HostTypeRattle));
                } else {
                    setNerve(GET_NERVE(ManholeCover, HostTypeOpen));
                }

                return true;
            }
        }
    }

    return false;
}

bool ManholeCover::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0xD1 && _94 == 0) {
        return true;
    }

    if (msg == 0xD2) {
        _95 = 1;
        return true;
    }

    if (MR::isMsgSpinStormRange(msg) && isNerve(GET_NERVE(ManholeCover, HostTypeWait))) {
        setNerve(GET_NERVE(ManholeCover, HostTypeRattle));
        return true;
    }

    return false;
}

void ManholeCover::exeOpen() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Rotation");
        MR::invalidateCollisionParts(this);
    }

    if (MR::isValidSwitchA(this)) {
        TVec3f dirToPlayer;
        MR::calcDirToPlayer(&dirToPlayer, this);

        TVec3f up;
        MR::calcUpVec(&up, this);

        if (dirToPlayer.dot(up) > 0.0f) {
            if (MR::isOnSwitchA(this)) {
                MR::offSwitchA(this);
            }
        } else if (!MR::isOnSwitchA(this)) {
            MR::onSwitchA(this);
        }
    }

    if (MR::isStep(this, 20)) {
        MR::validateCollisionParts(this);
    }

    if (MR::isActionEnd(this)) {
        MR::validateCollisionParts(this);
        setNerve(GET_NERVE(ManholeCover, HostTypeWait));
    }
}

void ManholeCover::exeRattle() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Vibration");
    }

    if (MR::isGreaterStep(this, 45) && MR::isActionEnd(this)) {
        setNerve(GET_NERVE(ManholeCover, HostTypeWait));
    }
}

ManholeCover::~ManholeCover() {
}
