#include "Game/NPC/SignBoard.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/NPCUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"

extern "C" void fn_800345E0(LiveActor*, const void*);
extern "C" const u32 lbl_807CFF10;

namespace NrvSignBoard {
    NEW_NERVE(SignBoardNrvWait, SignBoard, Wait);
};  // namespace NrvSignBoard

SignBoard::SignBoard(const char* pName) : NPCActor(pName) {
    _164 = -1;
    _168 = 0;
}

void SignBoard::init(const JMapInfoIter& rIter) {
    const char* name;
    MR::getObjectName(&name, rIter);
    NPCActorCaps caps(name);
    caps.setDefault();
    caps.mWaitNerve = GET_NERVE(SignBoard, SignBoardNrvWait);
    caps._95 = false;
    caps._10 = "SignBoard";

    MR::getJMapInfoArg0NoInit(rIter, &_164);
    if (_164 == 0) {
        MR::joinSupportPlayGroup(this);
        MR::isExistStageSwitchAppear(rIter);
    }

    initialize(rIter, caps, name, nullptr, nullptr);

    TVec3f up;
    f32 ux = 1.0f - 2.0f * (_A0.y * _A0.y) - 2.0f * (_A0.w * _A0.w);
    f32 uy = 2.0f * _A0.y * _A0.z - 2.0f * _B0.x * _A0.w;
    f32 uz = 2.0f * _A0.z * _A0.w + 2.0f * _B0.x * _A0.y;
    up.set(ux, uy, uz);

    MR::setShadowProjection(this, nullptr, mPosition, up, true);
    MR::offCalcShadow(this, nullptr);

    if (_164 == 0 || MR::isValidSwitchAppear(this)) {
        makeActorDead();
    } else {
        makeActorAppeared();
    }
}

void SignBoard::exeWait() {
    MR::tryTalkNearPlayer(getMsgCtrl());
}

void SignBoard::appear() {
    if (_164 == 0) {
        if (_168 == 0) {
            return;
        }
        if (MR::isDead(this)) {
            fn_800345E0(this, &lbl_807CFF10);
        }
    }
    LiveActor::appear();
}

void SignBoard::kill() {
    fn_800345E0(this, &lbl_807CFF10);
    NPCActor::kill();
}

void SignBoard::makeActorDead() {
    if (_164 == 0 && MR::isValidSwitchAppear(this)) {
        if (!MR::isDead(this) && _168 != 0) {
            fn_800345E0(this, &lbl_807CFF10);
        }
    }
    NPCActor::makeActorDead();
}

bool SignBoard::receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*) {
    return false;
}

bool SignBoard::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (_164 != 0) {
        return false;
    }

    if (MR::isMsgSupportPlayGroupStart(msg)) {
        _168 = 1;
        if (!MR::isValidSwitchAppear(this) || MR::isOnSwitchAppear(this)) {
            kill();
        }
        return true;
    }

    if (MR::isMsgSupportPlayGroupStop(msg)) {
        _168 = 0;
        if (!MR::isDead(this)) {
            makeActorDead();
        }
        return true;
    }
    return false;
}
