#include "Game/MapObj/IceMeteorParts.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"

namespace { const char* sJointName = "IceSurface"; const char* sModelName = "IceMeteorBall"; }
namespace NrvIceMeteorParts { NEW_NERVE(IceMeteorPartsNrvFall, IceMeteorParts, Fall); }
IceMeteorParts::IceMeteorParts(LiveActor* host, const char* name) : PartsModel(host, name, sModelName, 0, -1, false), mOffset(0.0f, 0.0f, 0.0f) {}
void IceMeteorParts::init(const JMapInfoIter& iter) {
    PartsModel::init(iter);
    initFixedPosition(sJointName);
    initNerve(&NrvIceMeteorParts::IceMeteorPartsNrvFall::sInstance);
}
void IceMeteorParts::setOffset(const TVec3f& offset) { mOffset = offset; }
const TVec3f& IceMeteorParts::getOffset() const { return mOffset; }
void IceMeteorParts::calcAndSetBaseMtx() {
    PartsModel::calcAndSetBaseMtx();
    Mtx mtx;
    PSMTXIdentity(mtx);
    PSMTXCopy(getBaseMtx(), mtx);
    MR::addTransMtx(mtx, mOffset);
    mPosition += mOffset;
    MR::setBaseTRMtx(this, mtx);
}
void IceMeteorParts::exeFall() {}
IceMeteorParts::~IceMeteorParts() {}
IceMeteorHolder::IceMeteorHolder(const char* name) : NameObj(name), mRequest(0), mSender(0), mReceiver(0) {}
void IceMeteorHolder::init(const JMapInfoIter&) { MR::connectToSceneMapObjDecorationMovement(this); }
void IceMeteorHolder::movement() {
    if (mRequest == 1) MR::sendMsgFireDance(mSender, mReceiver);
    clearRequest();
}
void IceMeteorHolder::requestFireDance(HitSensor* sender, HitSensor* receiver) {
    if (mRequest == 2) return;
    mRequest = 1;
    mSender = sender;
    mReceiver = receiver;
}
void IceMeteorHolder::cancelFireDance(HitSensor*, HitSensor*) {
    if (mRequest == 2) return;
    mRequest = 2;
    mSender = 0;
    mReceiver = 0;
}
void IceMeteorHolder::clearRequest() { mRequest = 0; mSender = 0; mReceiver = 0; }
namespace MR {
void createIceMeteorHolder() { createSceneObj(0x88); }
void requestIceMeteorFireDance(HitSensor* sender, HitSensor* receiver) { getSceneObj<IceMeteorHolder*>(0x88)->requestFireDance(sender, receiver); }
void cancelIceMeteorFireDance(HitSensor* sender, HitSensor* receiver) { getSceneObj<IceMeteorHolder*>(0x88)->cancelFireDance(sender, receiver); }
}
IceMeteorHolder::~IceMeteorHolder() {}
