#pragma auto_inline off
#include "Game/MapObj/MorphItemCollection.hpp"
#include "Game/MapObj/MorphItemObjNeo.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/NPCUtil.hpp"
namespace NrvMorphItemCollection {
NEW_NERVE(MorphItemCollectionNrvWait, MorphItemCollection, Wait);
}
MorphItemCollection::MorphItemCollection(const char* name) : LiveActor(name), mPowerUp(-1), mAngle(0.0f), mIsDrill(false) {}
void MorphItemCollection::init(const JMapInfoIter& iter) {
    initPowerUpModel(iter);
    if (mIsDrill) MR::initActor(this, iter, "ItemDrill", "Collection", false);
    else MR::initActor(this, iter, MorphItemObjNeo::getMorphModelName(mPowerUp), "Collection", false);
    initNerve(&NrvMorphItemCollection::MorphItemCollectionNrvWait::sInstance);
    MR::registerMarioFaceShipNpc(this, iter);
    makeActorAppeared();
}
void MorphItemCollection::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    MtxPtr base = getBaseMtx();
    MtxPtr rotation = MR::tmpMtxRotYDeg(mAngle);
    PSMTXConcat(getBaseMtx(), rotation, base);
}
void MorphItemCollection::exeWait() {
    if (mIsDrill) { if (MR::isFirstStep(this)) MR::startAction(this, "ItemWait"); }
    else {
        if (MR::isFirstStep(this)) MorphItemObjNeo::glowEffect(this, mPowerUp);
        mAngle += 2.0f;
        mAngle = MR::repeat(mAngle, 0.0f, 360.0f);
    }
}
void MorphItemCollection::initPowerUpModel(const JMapInfoIter& iter) {
    const char* name = 0;
    MR::getObjectName(&name, iter);
    if (MR::isEqualString(name, "MorphItemCollectionHopper")) mPowerUp = 1;
    else if (MR::isEqualString(name, "MorphItemCollectionBee")) mPowerUp = 2;
    else if (MR::isEqualString(name, "MorphItemCollectionTeresa")) mPowerUp = 3;
    else if (MR::isEqualString(name, "MorphItemCollectionFire")) mPowerUp = 5;
    else if (MR::isEqualString(name, "MorphItemCollectionCloud")) mPowerUp = 7;
    else if (MR::isEqualString(name, "MorphItemCollectionRock")) mPowerUp = 8;
    else if (MR::isEqualString(name, "MorphItemCollectionDrill")) { mPowerUp = -1; mIsDrill = true; }
}
MorphItemCollection::~MorphItemCollection() {}
