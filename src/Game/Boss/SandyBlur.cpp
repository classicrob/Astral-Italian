#include "Game/Boss/Sandy.hpp"
// SMG2: MR::appearCoinPop returns bool (ObjUtil.hpp declares it void), declare the real one here.
#define appearCoinPop appearCoinPopUnused
#include "Game/Util.hpp"
#undef appearCoinPop
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Util/FixedPosition.hpp"
#include <cstdio>

namespace NrvSandyBlur {
    class SandyBlurNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
        }
        static SandyBlurNrvWait sInstance;
    };
    SandyBlurNrvWait SandyBlurNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(SandyBlurNrvBlur, SandyBlur, Blur);
};  // namespace NrvSandyBlur

// 800FFDA0
SandyBlur::SandyBlur(const char* pName) : NerveExecutor(pName) {
    initNerve(&NrvSandyBlur::SandyBlurNrvWait::sInstance, 0);
    MR::createCenterScreenBlur();
}

// 800FFDF0
bool SandyBlur::tryStart() {
    if (!isNerve(&NrvSandyBlur::SandyBlurNrvWait::sInstance)) {
        return false;
    }
    MR::startCenterScreenBlur(45, 10.0f, 80, 5, 30);
    setNerve(&NrvSandyBlur::SandyBlurNrvBlur::sInstance);
    return true;
}

// 800FFE60
SandyBlur::~SandyBlur() {
}

// 800FFF10
void SandyBlur::exeBlur() {
    if (MR::isGreaterStep(this, 45)) {
        setNerve(&NrvSandyBlur::SandyBlurNrvWait::sInstance);
    }
}
