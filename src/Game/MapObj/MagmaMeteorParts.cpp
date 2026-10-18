#include "Game/MapObj/MagmaMeteorParts.hpp"
#include "Game/MapObj/MagmaMeteor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
namespace {
    const char* sJointName = "IceSurface";
    const char* sModelName = "MagmaMeteorBall";
}
namespace NrvMagmaMeteorParts {
    NEW_NERVE(MagmaMeteorPartsNrvFall, MagmaMeteorParts, Fall);
}
MagmaMeteorParts::MagmaMeteorParts(MagmaMeteor* host, const char* name)
    : PartsModel(host, name, sModelName, nullptr, -1, false), mOffset(0.0f, 0.0f, 0.0f) {}
MagmaMeteorParts::~MagmaMeteorParts() {}
void MagmaMeteorParts::init(const JMapInfoIter& iter) {
    PartsModel::init(iter);
    initFixedPosition(sJointName);
    initNerve(&NrvMagmaMeteorParts::MagmaMeteorPartsNrvFall::sInstance);
}
void MagmaMeteorParts::setOffset(const TVec3f& offset) { mOffset = offset; }
const TVec3f& MagmaMeteorParts::getOffset() const { return mOffset; }
void MagmaMeteorParts::calcAndSetBaseMtx() {
    PartsModel::calcAndSetBaseMtx();
    TPos3f mtx;
    PSMTXIdentity(mtx);
    PSMTXCopy(getBaseMtx(), mtx);
    TPos3f rotation;
    rotation.makeRotate(TVec3f(1.0f, 0.0f, 0.0f), MR::toRadian(mRotation.x));
    mtx.concat(rotation);
    MR::addTransMtx(mtx, mOffset);
    mPosition += mOffset;
    MR::setBaseTRMtx(this, mtx);
}
void MagmaMeteorParts::exeFall() { mRotation.x = MR::repeatDegree(-7.5f + mRotation.x); }
