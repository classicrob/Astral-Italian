#include "Game/Enemy/Foomin.hpp"
#include "Game/Enemy/SupportTicoStop.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvFoomin {
    class FoominNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static FoominNrvWait sInstance;
    };
    FoominNrvWait FoominNrvWait::sInstance ATTRIBUTE_WEAK;

    class FoominNrvWaitTouch : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static FoominNrvWaitTouch sInstance;
    };
    FoominNrvWaitTouch FoominNrvWaitTouch::sInstance ATTRIBUTE_WEAK;

    class FoominNrvAttackSign : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static FoominNrvAttackSign sInstance;
    };
    FoominNrvAttackSign FoominNrvAttackSign::sInstance ATTRIBUTE_WEAK;

    class FoominNrvAttack : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static FoominNrvAttack sInstance;
    };
    FoominNrvAttack FoominNrvAttack::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvFoomin

#define GET_FOOMIN_NERVE(name) (&NrvFoomin::name::sInstance)

// Unnamed SMG2 helpers.

Foomin::Foomin(const char* pName) : LiveActor(pName) {
    mSupportTico = nullptr;
    mFront.x = 0.0f;
    mFront.y = 0.0f;
    mFront.z = 1.0f;
    mArg1 = 250;
    mArg0 = 0;
}

void Foomin::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Foomin", false);
    if (MR::isExistRail(this)) {
        MR::moveCoordAndTransToNearestRailPos(this);
    }

    MR::calcFrontVec(&mFront, this);
    mSupportTico = new SupportTicoStop(this, nullptr);

    s32 arg0 = 0;
    MR::getJMapInfoArg0NoInit(rIter, &arg0);
    if (arg0 >= 0) {
        mArg0 = arg0;
    }

    s32 arg1 = -1;
    MR::getJMapInfoArg1NoInit(rIter, &arg1);
    if (arg1 >= 0) {
        mArg1 = arg1;
    }

    initNerve(GET_FOOMIN_NERVE(FoominNrvWaitTouch), 0);
    makeActorAppeared();
}

void Foomin::makeActorAppeared() {
    if (mArg0 > 0) {
        setNerve(GET_FOOMIN_NERVE(FoominNrvWaitTouch));
    } else {
        setNerve(GET_FOOMIN_NERVE(FoominNrvWait));
    }
    LiveActor::makeActorAppeared();
}

void Foomin::control() {
    mSupportTico->updateNerve();
}

void Foomin::calcAndSetBaseMtx() {
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFront);
}

void Foomin::fn_80181940() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::validateClipping(this);
    }

    if ((!MR::isValidSwitchA(this) || MR::isOnSwitchA(this)) && MR::isGreaterStep(this, this->mArg0)) {
        this->setNerve(GET_FOOMIN_NERVE(FoominNrvAttackSign));
    }
}

void Foomin::fn_801819D0() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::validateClipping(this);
    }

    if (this->mSupportTico->tryAttachTouch() == 0) {
        if ((!MR::isValidSwitchA(this) || MR::isOnSwitchA(this)) && MR::isGreaterStep(this, 120)) {
            this->setNerve(GET_FOOMIN_NERVE(FoominNrvAttackSign));
        }
        this->fn_80181C80();
    }
}

void Foomin::fn_80181A80() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSign");
        MR::invalidateClipping(this);
    }

    if (this->mSupportTico->tryAttachTouch() == 0) {
        this->fn_80181C80();
        if (MR::isActionEnd(this)) {
            this->setNerve(GET_FOOMIN_NERVE(FoominNrvAttack));
        }
    }
}

void Foomin::fn_80181B00() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
    }

    if (MR::isLessStep(this, 25) && MR::isActorOnPlayer(this)) {
        TVec3f front(0.0f);
        MR::calcFrontVec(&front, this);
        TVec3f jump = -mGravity + (-front) * -0.5f;
        MR::normalize(&jump);
        MR::forceJumpPlayer(jump * 35.0f);
        MR::startBckPlayer("Blow", nullptr);
        MR::startLevelSoundPlayer("Thrown", -1, -1);
        MR::emitEffect(this, "Shrink");
        MR::invalidateCollisionParts(this);
    }

    this->fn_80181C80();
    if (MR::isActionEnd(this)) {
        MR::validateCollisionParts(this);
        this->setNerve(GET_FOOMIN_NERVE(FoominNrvWaitTouch));
    }
}

void Foomin::fn_80181C80() {
    if (MR::isExistRail(this)) {
        s32 speed = this->mArg1;
        f32 rate = 0.009999999776482582f * (f32)speed;
        MR::fn_80052450(this, rate, 0.20000000298023224f * rate, 100.0f);
    }
}

Foomin::~Foomin() {
}

void NrvFoomin::FoominNrvWait::execute(Spine* pSpine) const {
    (reinterpret_cast<Foomin*>(pSpine->mExecutor))->fn_80181940();
}

void NrvFoomin::FoominNrvWaitTouch::execute(Spine* pSpine) const {
    (reinterpret_cast<Foomin*>(pSpine->mExecutor))->fn_801819D0();
}

void NrvFoomin::FoominNrvAttackSign::execute(Spine* pSpine) const {
    (reinterpret_cast<Foomin*>(pSpine->mExecutor))->fn_80181A80();
}

void NrvFoomin::FoominNrvAttack::execute(Spine* pSpine) const {
    (reinterpret_cast<Foomin*>(pSpine->mExecutor))->fn_80181B00();
}
