#include "Game/MapObj/CymbalsStep.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/JointController.hpp"
namespace NrvCymbalsStep {
    NEW_NERVE(CymbalsStepNrvWait, CymbalsStep, Wait);
    NEW_NERVE(CymbalsStepNrvMove, CymbalsStep, Move);
    NEW_NERVE(CymbalsStepNrvClose, CymbalsStep, Close);
}
namespace { const char* sJointName = "Down"; }
CymbalsStep::CymbalsStep(const char* name) : SimpleSeesawObj(name), mJointController(nullptr), mClosedCollision(nullptr), mCloseRate(0.0f) {
    PSMTXIdentity(mInitialMtx);
}
// NONMATCHING: target inlines the empty SimpleSeesawObj destructor.
CymbalsStep::~CymbalsStep() {}
void CymbalsStep::init(const JMapInfoIter& iter) {
    MapObjActor::init(iter);
    MapObjActorInitInfo info;
    MapObjActorUtil::setupInitInfoSimpleMapObj(&info);
    MapObjActorUtil::setupInitInfoSeesaw(&info, iter, nullptr, 0.0f);
    MapObjActorUtil::setupInitInfoTypical(&info, mObjectName);
    MapObjActorUtil::setupInitInfoColorChangeArg0(&info, iter);
    MapObjActorUtil::setupInitInfoTextureChangeArg1(&info, iter);
    MapObjActorUtil::setupInitInfoShadowLengthArg2(&info, iter);
    info.setupNerve(&NrvCymbalsStep::CymbalsStepNrvWait::sInstance);
    info.setupClippingRadius(2000.0f);
    initialize(iter, info);
    mJointController = MR::createJointController(this, sJointName, &CymbalsStep::move);
    mClosedCollision = MR::createCollisionPartsFromLiveActor(this, "CymbalsStepClose", getSensor("Body"), MR::CollisionScaleType(2));
    MR::invalidateCollisionParts(mClosedCollision);
    MR::makeMtxTR(mInitialMtx, this);
}
void CymbalsStep::calcAndSetBaseMtx() {
    if (isNerve(&NrvCymbalsStep::CymbalsStepNrvClose::sInstance)) {
        updateProjmapMtx();
        TMtx34f mtx;
        PSMTXCopy(mInitialMtx, mtx);
        if (mCloseRate < 0.99f) MR::blendMtxRotate(getBaseMtx(), mtx, mCloseRate, mtx);
        MR::setMtxTrans(mtx, mPosition.x, mPosition.y, mPosition.z);
        MR::setBaseTRMtx(this, mtx);
    } else MapObjActor::calcAndSetBaseMtx();
}
void CymbalsStep::calcAnim() {
    if (mJointController) mJointController->registerCallBack();
    LiveActor::calcAnim();
}
bool CymbalsStep::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgPlayerHipDropFloor(msg)) {
        if (isNerve(&NrvCymbalsStep::CymbalsStepNrvClose::sInstance)) return false;
        MR::invalidateCollisionParts(this);
        MR::validateCollisionParts(mClosedCollision);
        setNerve(&NrvCymbalsStep::CymbalsStepNrvClose::sInstance);
        mCloseRate = 0.0f;
        return true;
    }
    isNerve(&NrvCymbalsStep::CymbalsStepNrvClose::sInstance);
    return false;
}
bool CymbalsStep::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) {
    if (isNerve(&NrvCymbalsStep::CymbalsStepNrvClose::sInstance)) {
        MR::startActionSound(this, "RideOnClosed");
        return false;
    }
    if (MR::isMsgFloorTouch(msg) || MR::isMsgWallTouch(msg) || MR::isMsgCeilTouch(msg)) {
        MR::startActionSound(this, "RideOn");
        return false;
    }
    return false;
}
void CymbalsStep::exeClose() {
    if (MR::isFirstStep(this)) {
        MapObjActorUtil::pauseRotator(this);
        MR::startAction(this, "Close");
        if (MR::isValidSwitchA(this)) MR::onSwitchA(this);
    }
    mCloseRate += 0.1f;
    MR::clamp(mCloseRate, 0.0f, 1.0f);
}
namespace {
    inline void setCymbalScale(TPos3f* mtx, f32 x, f32 y, f32 z) {
        mtx->zeroTrans();
        mtx->mMtx[0][0] = x;
        mtx->mMtx[0][1] = 0.0f;
        mtx->mMtx[0][2] = 0.0f;
        mtx->mMtx[1][0] = 0.0f;
        mtx->mMtx[1][1] = y;
        mtx->mMtx[1][2] = 0.0f;
        mtx->mMtx[2][0] = 0.0f;
        mtx->mMtx[2][1] = 0.0f;
        mtx->mMtx[2][2] = z;
    }
}

bool CymbalsStep::move(TPos3f* mtx, const JointControllerInfo&) {
    if (isNerve(&NrvCymbalsStep::CymbalsStepNrvClose::sInstance) && mCloseRate >= 0.99f) return true;
    TVec3f scale;
    mtx->getScale(scale);
    TVec3f trans;
    mtx->getTrans(trans);
    setCymbalScale(mtx, scale.x, scale.y, scale.z);
    mtx->setTrans(trans);
    return true;
}
void CymbalsStep::initCaseUseSwitchA(const MapObjActorInitInfo&) {}
