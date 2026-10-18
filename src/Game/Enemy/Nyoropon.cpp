#include "Game/Enemy/Nyoropon.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

// Nerve instances: a Nerve subclass whose execute dispatches to a free exe function (vtable mapping above).
#define NYORO_NERVE(name, exefn)                                                                              \
    class name : public Nerve {                                                                               \
    public:                                                                                                    \
        virtual void execute(Spine* pSpine) const {                                                           \
            reinterpret_cast<Nyoropon*>(pSpine->mExecutor)->exefn();                                            \
        }                                                                                                     \
        static name sInstance;                                                                                \
    };                                                                                                        \
    name name::sInstance ATTRIBUTE_WEAK;


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
void Nyoropon::fn_801D8370() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
    }
    if (MR::calcDistanceToPlayer(this) < 1000.0f) {
        this->setNerve(GET_NYORO_NERVE(01));
    }
}

void Nyoropon::fn_801D8460() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
    }
    if (MR::isBckOneTimeAndStopped(this)) {
        this->setNerve(GET_NYORO_NERVE(02));
    }
}

void Nyoropon::fn_801D8520() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "Light", -1, -1, -1);
        MR::tryStartActionNotPlaying(this, "Light");
    }
    if (MR::isGreaterStep(this, 60)) {
        this->setNerve(GET_NYORO_NERVE(03));
    }
}

void Nyoropon::fn_801D8640() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Recover");
    }
    if (MR::isActionEnd(this)) {
        this->setNerve(GET_NYORO_NERVE(04));
    }
}

void Nyoropon::fn_801D8730() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Starpiece");
    }
    if (MR::isActionEnd(this)) {
        this->setNerve(GET_NYORO_NERVE(05));
    }
}

void Nyoropon::fn_801D8830() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Down");
    }
    if (MR::isActionEnd(this)) {
        MR::appearCoinPop(this, this->mPosition, 1);
        this->setNerve(GET_NYORO_NERVE(06));
    }
}

void Nyoropon::fn_801D8910() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Spin");
    }
    if (MR::isActionEnd(this)) {
        this->setNerve(GET_NYORO_NERVE(07));
    }
}

void Nyoropon::fn_801D89F0() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SearchLight");
    }
    if (MR::isActionEnd(this)) {
        this->setNerve(GET_NYORO_NERVE(08));
    }
}

void Nyoropon::fn_801D8AD0() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Down");
    }
    if (MR::isActionEnd(this)) {
        this->setNerve(GET_NYORO_NERVE(09));
    }
}

// NONMATCHING: the SMG2 body calls updateActorState on a state object not recovered yet.
void Nyoropon::fn_801D8BB0() {
    (void)this;
}

// NONMATCHING: the helpers below are approximations; they keep the SMG2 call set and nerve flow.
bool Nyoropon::fn_801D8130() {
    return this->isNerve(GET_NYORO_NERVE(00)) || this->isNerve(GET_NYORO_NERVE(01)) ||
           this->isNerve(GET_NYORO_NERVE(02)) || this->isNerve(GET_NYORO_NERVE(03));
}

bool Nyoropon::fn_801D81E0() {
    return this->isNerve(GET_NYORO_NERVE(00)) || this->isNerve(GET_NYORO_NERVE(01)) ||
           this->isNerve(GET_NYORO_NERVE(02)) || this->isNerve(GET_NYORO_NERVE(03));
}

bool Nyoropon::fn_801D8260() {
    return this->isNerve(GET_NYORO_NERVE(00)) || this->isNerve(GET_NYORO_NERVE(01)) ||
           this->isNerve(GET_NYORO_NERVE(02));
}

bool Nyoropon::fn_801D82D0() {
    return this->isNerve(GET_NYORO_NERVE(03)) || this->isNerve(GET_NYORO_NERVE(04)) ||
           this->isNerve(GET_NYORO_NERVE(09));
}

f32 Nyoropon::fn_801D8350() {
    return this->isNerve(GET_NYORO_NERVE(09)) ? 1.0f : 0.0f;
}

void Nyoropon::fn_801D8C80() {
    if (!MR::isBtpPlaying(this, "Sleep")) {
        MR::startBtp(this, "Sleep");
    }
}

void Nyoropon::fn_801D8CF0() {
    if (!MR::isBtpPlaying(this, "Normal")) {
        MR::startBtp(this, "Normal");
    }
}

void Nyoropon::fn_801D8D60() {
    if (!MR::isBtpPlaying(this, "Search")) {
        MR::startBtp(this, "Search");
    }
}

void Nyoropon::fn_801D8DD0() {
    // NONMATCHING: joint controller setup (HeadPos / NyoroponBody) not recovered.
    (void)this;
}

void Nyoropon::fn_801D8EE0() {
    MR::setMtxTrans(nullptr, this->_11C, this->_120, this->_124);
}

void Nyoropon::fn_801D9070() {
    this->_128 = MR::clamp(this->_128, 0.0f, 1.0f);
}

void Nyoropon::fn_801D91A0() {
    TVec3f pos(*MR::getPlayerPos());
    MR::normalize(&pos);
    this->_158 = pos.x;
}

void Nyoropon::fn_801D93A0() {
    this->_15C = MR::clamp(this->_15C, -1.0f, 1.0f) * MR::sign(this->_15C);
}

void Nyoropon::fn_801D9C60() {
    this->_14C = MR::clamp(this->_14C, 0.0f, 1.0f);
}

bool Nyoropon::fn_801DA150() {
    return this->isNerve(GET_NYORO_NERVE(00));
}

bool Nyoropon::fn_801DA300() {
    return this->isNerve(GET_NYORO_NERVE(09));
}

void Nyoropon::fn_801DA370() {
    MR::startActionSound(this, "WaitA", -1, -1, -1);
}
