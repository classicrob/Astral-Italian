#include "Game/Enemy/Nyoropon.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

// Nerve instances: a Nerve subclass whose execute dispatches to a free exe function (vtable mapping above).
#define NYORO_NERVE(name, exefn)                                                                              \
    class name : public Nerve {                                                                               \
    public:                                                                                                    \
        virtual void execute(Spine* pSpine) const {                                                           \
            exefn(reinterpret_cast<Nyoropon*>(pSpine->mExecutor));                                            \
        }                                                                                                     \
        static name sInstance;                                                                                \
    };                                                                                                        \
    name name::sInstance ATTRIBUTE_WEAK;

extern "C" void fn_801D8370(Nyoropon*);
extern "C" void fn_801D8460(Nyoropon*);
extern "C" void fn_801D8520(Nyoropon*);
extern "C" void fn_801D8640(Nyoropon*);
extern "C" void fn_801D8730(Nyoropon*);
extern "C" void fn_801D8830(Nyoropon*);
extern "C" void fn_801D8910(Nyoropon*);
extern "C" void fn_801D89F0(Nyoropon*);
extern "C" void fn_801D8AD0(Nyoropon*);
extern "C" void fn_801D8BB0(Nyoropon*);

namespace NrvNyoropon {
NYORO_NERVE(NyoroponNrv00, fn_801D8370);
NYORO_NERVE(NyoroponNrv01, fn_801D8460);
NYORO_NERVE(NyoroponNrv02, fn_801D8520);
NYORO_NERVE(NyoroponNrv03, fn_801D8640);
NYORO_NERVE(NyoroponNrv04, fn_801D8730);
NYORO_NERVE(NyoroponNrv05, fn_801D8830);
NYORO_NERVE(NyoroponNrv06, fn_801D8910);
NYORO_NERVE(NyoroponNrv07, fn_801D89F0);
NYORO_NERVE(NyoroponNrv08, fn_801D8AD0);
NYORO_NERVE(NyoroponNrv09, fn_801D8BB0);
};  // namespace NrvNyoropon

#define GET_NYORO_NERVE(n) (&NrvNyoropon::NyoroponNrv##n::sInstance)

Nyoropon::Nyoropon(const char* pName) : LiveActor(pName) {
    _90 = 0;
    _94 = 0;
    _98 = 0;
    _9C = 0;
    mArrayA0 = nullptr;
    mArrayB = nullptr;
    _114 = 0;
    _118 = 0;
    _168 = false;
    _169 = false;
    _160 = 0;
    _164 = 0;
}

Nyoropon::~Nyoropon() {
}

void Nyoropon::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Nyoropon", nullptr, false);
    MR::calcFrontVec(&mFront, this);
    MR::calcUpVec(&mUp, this);
    initNerve(GET_NYORO_NERVE(00), 0);
    makeActorAppeared();
}

void Nyoropon::makeActorAppeared() {
    MR::startAction(this, "Wait");
    LiveActor::makeActorAppeared();
}

void Nyoropon::control() {
    _11C = MR::clamp(_11C, 0.0f, 1.0f);
}

void Nyoropon::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        MR::sendMsgEnemyAttackStrong(pReceiver, pSender);
        return;
    }
    MR::sendMsgPush(pReceiver, pSender);
}

// NONMATCHING: message filter approximated from the MR-call summary.
bool Nyoropon::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg) || MR::isMsgStarPieceReflect(msg)) {
        return true;
    }
    if (MR::isMsgStarPieceAttack(msg) || MR::isMsgPlayerSpinAttackOrSupportTico(msg) || MR::isMsgJetTurtleAttack(msg)) {
        setNerve(GET_NYORO_NERVE(05));
        return true;
    }
    return false;
}

void Nyoropon::calcAndSetBaseMtx() {
    MR::updateBaseScale(this, nullptr);
}

// NONMATCHING: exe bodies below are approximations of the SMG2 state machine (MR-call summary).
extern "C" void fn_801D8370(Nyoropon* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::tryStartActionNotPlaying(pActor, "Wait");
    }
    if (MR::calcDistanceToPlayer(pActor) < 1000.0f) {
        pActor->setNerve(GET_NYORO_NERVE(01));
    }
}

extern "C" void fn_801D8460(Nyoropon* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::tryStartActionNotPlaying(pActor, "Wait");
    }
    if (MR::isBckOneTimeAndStopped(pActor)) {
        pActor->setNerve(GET_NYORO_NERVE(02));
    }
}

extern "C" void fn_801D8520(Nyoropon* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startActionSound(pActor, "Light", -1, -1, -1);
        MR::tryStartActionNotPlaying(pActor, "Light");
    }
    if (MR::isGreaterStep(pActor, 60)) {
        pActor->setNerve(GET_NYORO_NERVE(03));
    }
}

extern "C" void fn_801D8640(Nyoropon* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pActor, "Recover");
    }
    if (MR::isActionEnd(pActor)) {
        pActor->setNerve(GET_NYORO_NERVE(04));
    }
}

extern "C" void fn_801D8730(Nyoropon* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pActor, "Starpiece");
    }
    if (MR::isActionEnd(pActor)) {
        pActor->setNerve(GET_NYORO_NERVE(05));
    }
}

extern "C" void fn_801D8830(Nyoropon* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pActor, "Down");
    }
    if (MR::isActionEnd(pActor)) {
        MR::appearCoinPop(pActor, pActor->mPosition, 1);
        pActor->setNerve(GET_NYORO_NERVE(06));
    }
}

extern "C" void fn_801D8910(Nyoropon* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pActor, "Spin");
    }
    if (MR::isActionEnd(pActor)) {
        pActor->setNerve(GET_NYORO_NERVE(07));
    }
}

extern "C" void fn_801D89F0(Nyoropon* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pActor, "SearchLight");
    }
    if (MR::isActionEnd(pActor)) {
        pActor->setNerve(GET_NYORO_NERVE(08));
    }
}

extern "C" void fn_801D8AD0(Nyoropon* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pActor, "Down");
    }
    if (MR::isActionEnd(pActor)) {
        pActor->setNerve(GET_NYORO_NERVE(09));
    }
}

// NONMATCHING: the SMG2 body calls updateActorState on a state object not recovered yet.
extern "C" void fn_801D8BB0(Nyoropon* pActor) {
    (void)pActor;
}

// NONMATCHING: the helpers below are approximations; they keep the SMG2 call set and nerve flow.
extern "C" bool fn_801D8130(Nyoropon* pActor) {
    return pActor->isNerve(GET_NYORO_NERVE(00)) || pActor->isNerve(GET_NYORO_NERVE(01)) ||
           pActor->isNerve(GET_NYORO_NERVE(02)) || pActor->isNerve(GET_NYORO_NERVE(03));
}

extern "C" bool fn_801D81E0(Nyoropon* pActor) {
    return pActor->isNerve(GET_NYORO_NERVE(00)) || pActor->isNerve(GET_NYORO_NERVE(01)) ||
           pActor->isNerve(GET_NYORO_NERVE(02)) || pActor->isNerve(GET_NYORO_NERVE(03));
}

extern "C" bool fn_801D8260(Nyoropon* pActor) {
    return pActor->isNerve(GET_NYORO_NERVE(00)) || pActor->isNerve(GET_NYORO_NERVE(01)) ||
           pActor->isNerve(GET_NYORO_NERVE(02));
}

extern "C" bool fn_801D82D0(Nyoropon* pActor) {
    return pActor->isNerve(GET_NYORO_NERVE(03)) || pActor->isNerve(GET_NYORO_NERVE(04)) ||
           pActor->isNerve(GET_NYORO_NERVE(09));
}

extern "C" f32 fn_801D8350(Nyoropon* pActor) {
    return pActor->isNerve(GET_NYORO_NERVE(09)) ? 1.0f : 0.0f;
}

extern "C" void fn_801D8C80(Nyoropon* pActor) {
    if (!MR::isBtpPlaying(pActor, "Sleep")) {
        MR::startBtp(pActor, "Sleep");
    }
}

extern "C" void fn_801D8CF0(Nyoropon* pActor) {
    if (!MR::isBtpPlaying(pActor, "Normal")) {
        MR::startBtp(pActor, "Normal");
    }
}

extern "C" void fn_801D8D60(Nyoropon* pActor) {
    if (!MR::isBtpPlaying(pActor, "Search")) {
        MR::startBtp(pActor, "Search");
    }
}

extern "C" void fn_801D8DD0(Nyoropon* pActor) {
    // NONMATCHING: joint controller setup (HeadPos / NyoroponBody) not recovered.
    (void)pActor;
}

extern "C" void fn_801D8EE0(Nyoropon* pActor) {
    MR::setMtxTrans(nullptr, pActor->_11C, pActor->_120, pActor->_124);
}

extern "C" void fn_801D9070(Nyoropon* pActor) {
    pActor->_128 = MR::clamp(pActor->_128, 0.0f, 1.0f);
}

extern "C" void fn_801D91A0(Nyoropon* pActor) {
    TVec3f pos(*MR::getPlayerPos());
    MR::normalize(&pos);
    pActor->_158 = pos.x;
}

extern "C" void fn_801D93A0(Nyoropon* pActor) {
    pActor->_15C = MR::clamp(pActor->_15C, -1.0f, 1.0f) * MR::sign(pActor->_15C);
}

extern "C" void fn_801D9C60(Nyoropon* pActor) {
    pActor->_14C = MR::clamp(pActor->_14C, 0.0f, 1.0f);
}

extern "C" bool fn_801DA150(Nyoropon* pActor) {
    return pActor->isNerve(GET_NYORO_NERVE(00));
}

extern "C" bool fn_801DA300(Nyoropon* pActor) {
    return pActor->isNerve(GET_NYORO_NERVE(09));
}

extern "C" void fn_801DA370(Nyoropon* pActor) {
    MR::startActionSound(pActor, "WaitA", -1, -1, -1);
}
