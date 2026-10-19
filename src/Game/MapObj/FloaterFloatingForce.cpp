#include "Game/MapObj/FloaterFloatingForce.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/MapPartsUtil.hpp"
#include "Game/Util/MathUtil.hpp"

FloaterFloatingForce::~FloaterFloatingForce() {
}

FloaterFloatingForce::FloaterFloatingForce(LiveActor* pHost, const char* pName) : MapPartsFunction(pHost, "浮力"), mFloaterName(pName), mStartPosition(gZeroVec) {
    mUpDir = TVec3f(0.0f, 1.0f, 0.0f);
    mMoveConditionType = 0.0f;
}

void FloaterFloatingForce::init(const JMapInfoIter& rIter) {
    s32 moveConditionType = 0;
    MR::getMapPartsArgMoveConditionType(&moveConditionType, rIter);
    mMoveConditionType = moveConditionType;
}

void FloaterFloatingForce::start() {
    mStartPosition.set(mHost->mPosition);

    TPos3f mtx;
    mtx.set(mHost->getBaseMtx());
    mtx.getYDir(mUpDir);
    MR::normalize(&mUpDir);
}
