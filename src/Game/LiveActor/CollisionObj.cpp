#include "Game/LiveActor/CollisionObj.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

CollisionObj::CollisionObj(LiveActor* pHost, const char* pName, const char* pArcName, HitSensor* pSensor, MtxPtr pBaseMtx, int drawType)
    : LiveActor(pName), mBaseMtx(nullptr) {
    if (drawType < 0) {
        drawType = 0x1d;
    }

    MR::connectToScene(this, drawType, -1, -1, -1);

    if (pSensor == nullptr) {
        initHitSensor(1);
        MR::addBodyMessageSensorMapObj(this);
        pSensor = getSensor(nullptr);
    }

    mBaseMtx = pBaseMtx;
    if (pBaseMtx == nullptr) {
        mBaseMtx = pHost->getBaseMtx();
    }

    MR::invalidateClipping(this);
    MR::initCollisionPartsFromResourceHolder(this, pArcName, pSensor, MR::getResourceHolder(pHost), pBaseMtx);
}

void CollisionObj::init(const JMapInfoIter& rIter) {
    makeActorAppeared();
}

void CollisionObj::movement() {
    LiveActor::movement();
    calcAnim();
}

MtxPtr CollisionObj::getBaseMtx() const {
    return mBaseMtx;
}
