#pragma auto_inline off
#include "Game/MapObj/LavaSeparateWaterfall.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/RailObj.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/MapObj/MapPartsRailMover.hpp"
#include "Game/Util.hpp"
namespace { const char* sRightJoint = "LavaSeparateWaterfallA"; const char* sLeftJoint = "LavaSeparateWaterfallB"; }
namespace NrvLavaSeparateWaterfall {
NEW_NERVE(LavaSeparateWaterfallNrvWait, LavaSeparateWaterfall, Wait);
NEW_NERVE(LavaSeparateWaterfallNrvMove, LavaSeparateWaterfall, Move);
}
LavaSeparateWaterfall::LavaSeparateWaterfall(const char* name) : LiveActor(name), mLeftCollision(0), mRightCollision(0), mLeftRail(0), mRightRail(0), mLeftMover(0), mRightMover(0), mLeftController(0), mRightController(0), mReactionDistance(14000.0f), mIsMoving(false) {}
// NONMATCHING: nonvolatile register allocation differs throughout initialization.
void LavaSeparateWaterfall::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    initNerve(&NrvLavaSeparateWaterfall::LavaSeparateWaterfallNrvWait::sInstance);
    MR::setClippingTypeSphere(this, mReactionDistance);
    MR::setClippingFarMax(this);
    MtxPtr leftMtx = MR::getJointMtx(this, sLeftJoint);
    mLeftCollision = MR::createCollisionPartsFromLiveActor(this, "MoveLeft", getSensor("Body"), leftMtx, MR::CollisionScaleType_Default);
    MR::validateCollisionParts(mLeftCollision);
    MtxPtr rightMtx = MR::getJointMtx(this, sRightJoint);
    mRightCollision = MR::createCollisionPartsFromLiveActor(this, "MoveRight", getSensor("Body"), rightMtx, MR::CollisionScaleType_Default);
    MR::validateCollisionParts(mRightCollision);
    mLeftController = createJointController(sLeftJoint, &LavaSeparateWaterfall::calcLeftMtx);
    mRightController = createJointController(sRightJoint, &LavaSeparateWaterfall::calcRightMtx);
    s32 childCount = MR::getChildObjNum(iter);
    for (s32 i = 0; i < childCount; ++i) { const char* name = 0; MR::getChildObjName(&name, iter, i); }
    mLeftRail = MR::createRailObj("左溶岩レール", this, iter, 0);
    mLeftMover = new MapPartsRailMover(mLeftRail);
    mLeftMover->init(iter);
    mLeftMover->setSignMotionTypeShake();
    mLeftMover->mDisableSignMotionAnimation = true;
    mRightRail = MR::createRailObj("右溶岩レール", this, iter, 1);
    mRightMover = new MapPartsRailMover(mRightRail);
    mRightMover->init(iter);
    mRightMover->setSignMotionTypeShake();
    mRightMover->mDisableSignMotionAnimation = true;
    MR::useStageSwitchAwake(this, iter);
    mLeftRail->initSound(4, "LavaSeparateWaterfallA", 0, TVec3f(0.0f));
    mRightRail->initSound(4, "LavaSeparateWaterfallB", 0, TVec3f(0.0f));
    makeActorAppeared();
}
JointController* LavaSeparateWaterfall::createJointController(const char* name, JointControlDelegator<LavaSeparateWaterfall>::func calcFunc) {
    JointControlDelegator<LavaSeparateWaterfall>* controller = new JointControlDelegator<LavaSeparateWaterfall>(this, calcFunc, 0);
    MR::setJointControllerParam(controller, this, name);
    return controller;
}
void LavaSeparateWaterfall::control() {
    if (!MR::isEffectValid(this, "TorchA")) MR::emitEffect(this, "TorchA");
    if (!MR::isEffectValid(this, "TorchB")) MR::emitEffect(this, "TorchB");
    mLeftMover->movement();
    mRightMover->movement();
}
void LavaSeparateWaterfall::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    mLeftController->registerCallBack();
    mRightController->registerCallBack();
}
void LavaSeparateWaterfall::calcAnim() { LiveActor::calcAnim(); mRightCollision->setMtx(); mLeftCollision->setMtx(); }
void LavaSeparateWaterfall::exeWait() {
    if (MR::isValidSwitchB(this) && MR::isOnSwitchB(this)) setNerve(&NrvLavaSeparateWaterfall::LavaSeparateWaterfallNrvMove::sInstance);
}
void LavaSeparateWaterfall::exeMove() {
    if (MR::isFirstStep(this)) { mRightMover->start(); mLeftMover->start(); }
    if (mLeftMover->isWorking()) MR::emitEffect(this, "LavaLeft");
    if (mRightMover->isWorking()) MR::emitEffect(this, "LavaRight");
    if (MR::isNearPlayer(this, mReactionDistance)) {
        if (mLeftMover->isWorking()) {
            if (mLeftMover->isStarting()) MR::tryHitReaction(this, "予兆");
            else { if (!mIsMoving) MR::tryHitReaction(this, "移動開始"); else MR::tryHitReaction(this, "移動中"); mIsMoving = true; }
        }
        if (mLeftMover->isReachedEnd()) {
            mIsMoving = false;
            if (MR::isRailGoingToEnd(mLeftRail)) MR::tryHitReaction(this, "開き終わり[左]");
            else MR::tryHitReaction(this, "閉じ終わり[左]");
        }
        if (mRightMover->isReachedEnd()) {
            if (MR::isRailGoingToEnd(mRightRail)) { MR::tryHitReaction(this, "開き終わり[右]"); return; }
            MR::tryHitReaction(this, "閉じ終わり[右]");
        }
    }
}
bool LavaSeparateWaterfall::calcLeftMtx(TPos3f* mtx, const JointControllerInfo&) { MR::setMtxTrans(mtx->toMtxPtr(), mLeftMover->mPosition.x, mLeftMover->mPosition.y, mLeftMover->mPosition.z); return true; }
bool LavaSeparateWaterfall::calcRightMtx(TPos3f* mtx, const JointControllerInfo&) { MR::setMtxTrans(mtx->toMtxPtr(), mRightMover->mPosition.x, mRightMover->mPosition.y, mRightMover->mPosition.z); return true; }
LavaSeparateWaterfall::~LavaSeparateWaterfall() {}
