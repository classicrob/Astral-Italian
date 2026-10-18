#include "Game/Enemy/RingBeamer.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/JumpGuarder.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/RingBeam.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace NrvRingBeamer {
    NEW_NERVE(RingBeamerNrvWait, RingBeamer, Wait);
    NEW_NERVE(RingBeamerNrvAttack, RingBeamer, Attack);
    NEW_NERVE(RingBeamerNrvInter, RingBeamer, Inter);

    class RingBeamerNrvFreeze : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static RingBeamerNrvFreeze sInstance;
    };
    RingBeamerNrvFreeze RingBeamerNrvFreeze::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvRingBeamer

RingBeamer::RingBeamer(const char* pName) : LiveActor(pName), mBeams(), mScaleCtrl(), mBindStarPointer() {
}

void RingBeamer::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "RingBeamer", false);
    initNerve(GET_NERVE(RingBeamer, RingBeamerNrvWait), 1);
    mScaleCtrl = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleCtrl);
    MR::initActorState(this, mBindStarPointer, GET_NERVE(RingBeamer, RingBeamerNrvFreeze), "SupportFreeze");
    makeActorAppeared();
    if (MR::isValidSwitchB(this)) {
        MR::listenStageSwitchOffB(this, MR::FunctorV0M< RingBeamer*, void (RingBeamer::*)() >(this, &RingBeamer::syncSwitchOffB));
    }
    MR::joinToGroupArray(this, rIter, mName, 32);
    // initializes to 5 long, but only uses 3?
    mBeams = new RingBeam*[5];
    for (int i = 0; i < 5; i++) {
        mBeams[i] = nullptr;
    }
    f32 arg0 = 20.0f;
    s32 arg1 = 100;
    f32 arg2 = 0.0f;
    MR::getJMapInfoArg0NoInit(rIter, &arg0);
    MR::getJMapInfoArg1NoInit(rIter, &arg1);
    MR::getJMapInfoArg2NoInit(rIter, &arg2);
    for (int i = 0; i < 3; i++) {
        mBeams[i] = new RingBeam("リングビーム", this, false, false, true, true, nullptr);
        mBeams[i]->init(rIter);
        mBeams[i]->setSpeed(arg0);
        mBeams[i]->setLife(arg1);
        mBeams[i]->setColor(arg2);
    }
    MR::startBckWithInterpole(this, "Open", 0);
    MR::setBckFrameAndStop(this, 0.0);
    MR::calcAnimDirect(this);
}

void RingBeamer::control() {
    mScaleCtrl->updateNerve();
    if (isNerveTypeActive() && mBindStarPointer->tryStartPointBind()) {
        setNerve(GET_NERVE(RingBeamer, RingBeamerNrvFreeze));
    }
}

bool RingBeamer::isNerveTypeActive() {
    if (isNerve(GET_NERVE(RingBeamer, RingBeamerNrvWait)) || isNerve(GET_NERVE(RingBeamer, RingBeamerNrvAttack)) || isNerve(GET_NERVE(RingBeamer, RingBeamerNrvInter))) {
        return true;
    }
    return false;
}

void RingBeamer::syncSwitchOffB() {
    setNerve(GET_NERVE(RingBeamer, RingBeamerNrvInter));

    for (int i = 0; i < 3; i++) {
        mBeams[i]->kill();
    }
}

void RingBeamer::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
    }

    if (MR::enableGroupAttack(this, 3000.0f, 500.0f)) {
        MR::sendMsgToGroupMember(0x7C, this, getSensor("Body"), "Body");
    }
}

void RingBeamer::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Open");
    }

    if (MR::isGreaterEqualStep(this, 240)) {
        setNerve(GET_NERVE(RingBeamer, RingBeamerNrvInter));
        return;
    }

    if (getNerveStep() % 80 >= 0 && getNerveStep() % 80 < 79) {
        MR::startActionSound(this, "EmLvRingbeamCharge");
    }

    if (getNerveStep() % 80 == 0) {
        MR::emitEffect(this, "Charge");
    }

    if (getNerveStep() % 80 == 79) {
        MR::deleteEffect(this, "Charge");

        if (!MR::enableGroupAttack(this, 3200.0f, 500.0f)) {
            MR::sendMsgToGroupMember(0x7E, this, getSensor("Body"), "Body");
        } else if (mBeams[getNerveStep() / 80]) {
            mBeams[getNerveStep() / 80]->appear();
        }
    }
}

void RingBeamer::exeInter() {
    if (MR::isFirstStep(this)) {
    }

    if (MR::isGreaterEqualStep(this, 80)) {
        for (int i = 0; i < 3; i++) {
            if (!MR::isDead(mBeams[i])) {
                return;
            }
        }

        setNerve(GET_NERVE(RingBeamer, RingBeamerNrvWait));
    }
}

void RingBeamer::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver) && MR::isSensorEnemyAttack(pSender)) {
        MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
    } else {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool RingBeamer::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(RingBeamer, RingBeamerNrvFreeze))) {
        return false;
    }

    if (msg == 0x7C) {
        MR::invalidateClipping(this);
        setNerve(GET_NERVE(RingBeamer, RingBeamerNrvAttack));

        return true;
    }

    if (msg == 0x7E) {
        setNerve(GET_NERVE(RingBeamer, RingBeamerNrvInter));

        return true;
    }

    return false;
}

bool RingBeamer::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgJetTurtleAttack(msg)) {
        return true;
    }

    return MR::isMsgStarPieceReflect(msg);
}

void NrvRingBeamer::RingBeamerNrvFreeze::execute(Spine* pSpine) const {
    LiveActor* actor = static_cast<LiveActor*>(pSpine->mExecutor);
    if (MR::isFirstStep(actor)) {
        MR::tryDeleteEffect(actor, "Charge");
    }
    MR::updateActorStateAndNextNerve(actor, &RingBeamerNrvWait::sInstance);
}
