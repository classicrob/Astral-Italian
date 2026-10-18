#include "Game/Util.hpp"
#include "Game/Enemy/UnbabaSlim.hpp"
#include "Game/MapObj/MapPartsRotator.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Util/MapPartsUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"

namespace NrvUnbabaSlim {
    UnbabaSlimNrvWait UnbabaSlimNrvWait::sInstance ATTRIBUTE_WEAK;
    UnbabaSlimNrvEatStart UnbabaSlimNrvEatStart::sInstance ATTRIBUTE_WEAK;
    UnbabaSlimNrvEat UnbabaSlimNrvEat::sInstance ATTRIBUTE_WEAK;
    UnbabaSlimNrvEatEnd UnbabaSlimNrvEatEnd::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvUnbabaSlim

// NONMATCHING: static initialization differs in scheduling and literal-base register setup.
TVec3f sUnbabaVec[4] = {TVec3f(0.0f, 4500.0f, 600.0f), TVec3f(1.1f, 1.25f, 1.1f), TVec3f(100.0f, -150.0f, 0.0f), TVec3f(0.08f, 0.08f, 0.08f)};

void NrvUnbabaSlim::UnbabaSlimNrvWait::execute(Spine* pSpine) const {
    static_cast< UnbabaSlim* >(pSpine->mExecutor)->exeWait();
}
void NrvUnbabaSlim::UnbabaSlimNrvEatStart::execute(Spine* pSpine) const {
    static_cast< UnbabaSlim* >(pSpine->mExecutor)->exeEatStart();
}
void NrvUnbabaSlim::UnbabaSlimNrvEat::execute(Spine* pSpine) const {
    static_cast< UnbabaSlim* >(pSpine->mExecutor)->exeEat();
}
void NrvUnbabaSlim::UnbabaSlimNrvEatEnd::execute(Spine* pSpine) const {
    static_cast< UnbabaSlim* >(pSpine->mExecutor)->exeEatEnd();
}

void UnbabaSlim::initRotator(const JMapInfoIter& rIter) {
    mRotator = new MapPartsRotator(this);
    mRotator->init(rIter);
}

void UnbabaSlim::startRotator() {
    if (mRotator != nullptr) {
        mRotator->start();
    }
}

void UnbabaSlim::endRotator() {
    if (mRotator != nullptr) {
        mRotator->end();
    }
}


UnbabaSlim::~UnbabaSlim() {}
