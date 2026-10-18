#include "Game/Enemy/BoomerangBros.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"

extern "C" void fn_8022BC70(LiveActor*);
extern "C" void fn_800348C0(LiveActor*);

BoomerangBrosBoomerang::BoomerangBrosBoomerang(const char* pName) : BrosProjectileBase(pName), _BC(10.0f), mStep(0), mBind(nullptr) {
}

namespace NrvBoomerangBrosBoomerang {
    NEW_NERVE(BoomerangBrosBoomerangNrvFly, BoomerangBrosBoomerang, Fly);
    NEW_NERVE(BoomerangBrosBoomerangNrvOut, BoomerangBrosBoomerang, Out);
    NEW_NERVE(BoomerangBrosBoomerangNrvBack, BoomerangBrosBoomerang, Back);
    NEW_NERVE(BoomerangBrosBoomerangNrvTongue, BoomerangBrosBoomerang, Tongue);
    NEW_NERVE(BoomerangBrosBoomerangNrvEaten, BoomerangBrosBoomerang, Eaten2);
    NEW_NERVE(BoomerangBrosBoomerangNrvThrown, BoomerangBrosBoomerang, Thrown);
    NEW_NERVE(BoomerangBrosBoomerangNrvX, BoomerangBrosBoomerang, X);

    class BoomerangBrosBoomerangNrvFreeze : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            BoomerangBrosBoomerang* pActor = reinterpret_cast< BoomerangBrosBoomerang* >(pSpine->mExecutor);
            pActor->_B4 = true;
            MR::updateActorStateAndNextNerve(pActor, &BoomerangBrosBoomerangNrvOut::sInstance);
        }
        static BoomerangBrosBoomerangNrvFreeze sInstance;
    };
    BoomerangBrosBoomerangNrvFreeze BoomerangBrosBoomerangNrvFreeze::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvBoomerangBrosBoomerang

#define BBNRV(n) (&NrvBoomerangBrosBoomerang::BoomerangBrosBoomerangNrv##n::sInstance)

void BoomerangBrosBoomerang::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "BoomerangBrosBoomerang", false);
    BrosProjectileBase::init(rIter);
    initNerve(BBNRV(Fly), 1);
    mBind = new WalkerStateBindStarPointer(this, mScaleCtrl);
    MR::initActorState(this, mBind, BBNRV(Freeze), "SupportFreeze");
    makeActorDead();
}

void BoomerangBrosBoomerang::appear() {
    BrosProjectileBase::appear();
    mStep = 0;
    MR::startAction(this, "Normal");
    setNerve(BBNRV(Fly));
    MR::showModel(this);
}

void BoomerangBrosBoomerang::kill() {
    LiveActor::kill();
    if (!isNerve(BBNRV(Tongue))) {
        fn_800348C0(this);
    }
}

void BoomerangBrosBoomerang::control() {
    BrosProjectileBase::control();
    if (isNerve(BBNRV(Out)) && mBind->tryStartPointBind()) {
        setNerve(BBNRV(Freeze));
    }
}

void BoomerangBrosBoomerang::attachToBros(BrosBase* pBros) {
    BrosProjectileBase::attachToBros(pBros);
    setNerve(BBNRV(Fly));
}

BoomerangBrosBoomerang::~BoomerangBrosBoomerang() {
}
