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
extern "C" void fn_80052450(LiveActor*, s32, f32, f32, f32);
extern "C" void fn_80181C80(Foomin*);
extern "C" void fn_80181940(Foomin*);
extern "C" void fn_801819D0(Foomin*);
extern "C" void fn_80181A80(Foomin*);
extern "C" void fn_80181B00(Foomin*);

Foomin::Foomin(const char* pName) : LiveActor(pName) {
    mSupportTico = nullptr;
    mFront.x = 0.0f;
    mFront.y = 0.0f;
    mFront.z = 1.0f;
    mArg1 = 250;
    mArg0 = 0;
}

void Foomin::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Foomin", nullptr, false);
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

    initNerve(GET_FOOMIN_NERVE(FoominNrvWait), 0);
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

extern "C" void fn_80181940(Foomin* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::validateClipping(pActor);
        MR::startAction(pActor, "Wait");
    }

    if ((!MR::isValidSwitchA(pActor) || MR::isOnSwitchA(pActor)) && MR::isGreaterStep(pActor, pActor->mArg0)) {
        pActor->setNerve(GET_FOOMIN_NERVE(FoominNrvAttackSign));
    }}

extern "C" void fn_801819D0(Foomin* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::validateClipping(pActor);
        MR::startAction(pActor, "Wait");
    }

    if (pActor->mSupportTico->tryAttachTouch() == 0) {
        if ((!MR::isValidSwitchA(pActor) || MR::isOnSwitchA(pActor)) && MR::isGreaterStep(pActor, 120)) {
            pActor->setNerve(GET_FOOMIN_NERVE(FoominNrvAttackSign));
        }
        fn_80181C80(pActor);
    }}

extern "C" void fn_80181A80(Foomin* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::invalidateClipping(pActor);
        MR::startAction(pActor, "AttackSign");
    }

    if (pActor->mSupportTico->tryAttachTouch() == 0) {
        fn_80181C80(pActor);
        if (MR::isActionEnd(pActor)) {
            pActor->setNerve(GET_FOOMIN_NERVE(FoominNrvAttack));
        }
    }}

extern "C" void fn_80181B00(Foomin* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pActor, "Attack");
    }

    if (MR::isLessStep(pActor, 25) && MR::isActorOnPlayer(pActor)) {
        TVec3f front(0.0f);
        MR::calcFrontVec(&front, pActor);
        TVec3f jump = front * -0.5f;
        jump.normalize();
        MR::forceJumpPlayer(jump * 35.0f);
        MR::startBckPlayer("Blow", nullptr);
        MR::startLevelSoundPlayer("Thrown", -1, -1);
        MR::emitEffect(pActor, "Shrink");
    }

    fn_80181C80(pActor);
    if (MR::isActionEnd(pActor)) {
        MR::validateClipping(pActor);
        pActor->setNerve(GET_FOOMIN_NERVE(FoominNrvWaitTouch));
    }}

extern "C" void fn_80181C80(Foomin* pActor) {
    if (MR::isExistRail(pActor)) {
        s32 speed = pActor->mArg1;
        f32 rate = 0.009999999776482582f * (f32)speed;
        fn_80052450(pActor, speed, rate, 0.20000000298023224f * rate, 100.0f);
    }
}

Foomin::~Foomin() {
}

void NrvFoomin::FoominNrvWait::execute(Spine* pSpine) const {
    fn_80181940(reinterpret_cast<Foomin*>(pSpine->mExecutor));
}

void NrvFoomin::FoominNrvWaitTouch::execute(Spine* pSpine) const {
    fn_801819D0(reinterpret_cast<Foomin*>(pSpine->mExecutor));
}

void NrvFoomin::FoominNrvAttackSign::execute(Spine* pSpine) const {
    fn_80181A80(reinterpret_cast<Foomin*>(pSpine->mExecutor));
}

void NrvFoomin::FoominNrvAttack::execute(Spine* pSpine) const {
    fn_80181B00(reinterpret_cast<Foomin*>(pSpine->mExecutor));
}
