#include "Game/Player/InfernoMarioManekuri.hpp"
#include "Game/Player/InfernoMario.hpp"
#include "Game/Player/InfernoMarioManager.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"
namespace NrvInfernoMarioManekuri {
    NEW_NERVE(InfernoMarioManekuriNrvWait, InfernoMarioManekuri, Wait);
    NEW_NERVE(InfernoMarioManekuriNrvFind, InfernoMarioManekuri, Find);
    NEW_NERVE(InfernoMarioManekuriNrvFollowStart, InfernoMarioManekuri, FollowStart);
    NEW_NERVE(InfernoMarioManekuriNrvFollow, InfernoMarioManekuri, Follow);
    NEW_NERVE(InfernoMarioManekuriNrvJump, InfernoMarioManekuri, Jump);
    NEW_NERVE(InfernoMarioManekuriNrvTrampleDown, InfernoMarioManekuri, TrampleDown);
    NEW_NERVE(InfernoMarioManekuriNrvBlowDownPlayer, InfernoMarioManekuri, BlowDownPlayer);
    NEW_NERVE(InfernoMarioManekuriNrvBlowDownEnemy, InfernoMarioManekuri, BlowDownEnemy);
}
ManekuriModel::ManekuriModel(const char* name,MtxPtr mtx) : LiveActor(name),mParentMtx(mtx) {
    if(mtx) mPosition.set<f32>(mtx[0][3],mtx[1][3],mtx[2][3]);
}
ManekuriModel::~ManekuriModel() {}
void ManekuriModel::calcAndSetBaseMtx() {
    if(mParentMtx) { mPosition.set<f32>(mParentMtx[0][3],mParentMtx[1][3],mParentMtx[2][3]);MR::setBaseTRMtx(this,mParentMtx);return; }
    LiveActor::calcAndSetBaseMtx();
}
InfernoMarioManekuri::InfernoMarioManekuri(InfernoMario* host) : InfernoMarioStateBase("インフェルノマリオ[マネクリ]",host),mModel(nullptr),mFront(0.0f,0.0f,1.0f),mStartPos(0.0f,0.0f,0.0f),mEndPos(0.0f,0.0f,0.0f),mFrame(0),mCombo(0) {
    initNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvWait::sInstance);
    mModel=new ManekuriModel("マネクリモデル",host->getBaseMtx());
    MR::initActor(mModel,"Manekuri","Inferno",false);
    mModel->makeActorDead();
}
InfernoMarioManekuri::~InfernoMarioManekuri() {}
void InfernoMarioManekuri::appear() { mIsDead=false;mHost->mCurrentQuat.getZDir(mFront); }
void InfernoMarioManekuri::kill() { mIsDead=true;mModel->makeActorDead(); }
void InfernoMarioManekuri::attackSensor(HitSensor* sender,HitSensor* receiver) {
    if(isFollowing() && MR::isSensorPlayerOrRide(receiver) && MR::isEqualSensor(sender,mHost,"Attack") && MR::sendMsgEnemyAttack(receiver,sender)) {
        MR::emitEffect(mHost,"Explosion");kill();return;
    }
    isJumping();
    if(isNerveBlowDownPlayer() && MR::isSensorEnemy(receiver) && MR::sendMsgToEnemyAttackBlowToDir(receiver,sender,mHost->mVelocity)) {
        u32 combo=mCombo++;
        ScoreAttackFunction::scoreCombo(mHost,combo);
    }
}
bool InfernoMarioManekuri::receiveMsgPlayerAttack(u32 msg,HitSensor* sender,HitSensor* receiver) {
    if(MR::isMsgPlayerTrample(msg)||MR::isMsgPlayerHipDrop(msg)) return trample(msg,sender);
    if(MR::isMsgPlayerHitAll(msg)) return blowDownPlayer(sender,receiver);
    return false;
}
bool InfernoMarioManekuri::receiveMsgEnemyAttack(u32 msg,HitSensor* sender,HitSensor* receiver) { if(MR::isMsgToEnemyAttackBlow(msg)) return blowDownEnemy(sender,receiver);return false; }
bool InfernoMarioManekuri::receiveOtherMsg(u32,HitSensor*,HitSensor*) { return false; }
bool InfernoMarioManekuri::trample(u32 msg,HitSensor* sensor) {
    if (canDamage()) {
        mModel->appear();MR::zeroVelocity(mHost);ScoreAttackFunction::scoreByAttack(mHost,msg,sensor);
        setNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvTrampleDown::sInstance);return true;
    }
    return false;
}
bool InfernoMarioManekuri::blowDownPlayer(HitSensor* sender,HitSensor* receiver) {
    if (canDamage()) {
        mCombo=1;ScoreAttackFunction::scoreCombo(mHost,0);mModel->appear();
        MR::setVelocityBlowAttack(mHost, sender, receiver, 40.0f, 0.0f, 4);
        MR::calcSensorHorizonNormalize(&mFront,mHost->mGravity,receiver,sender);
        InfernoMario* host = mHost;
        MR::makeQuatUpFront(&host->mCurrentQuat,-host->mGravity,mFront);
        setNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvBlowDownPlayer::sInstance);MR::onBind(mHost);MR::onCalcGravity(mHost);return true;
    }
    return false;
}
bool InfernoMarioManekuri::blowDownEnemy(HitSensor* sender,HitSensor* receiver) {
    if (canDamage()) {
        mModel->appear();MR::setVelocityJump(mHost,40.0f);
        MR::calcSensorHorizonNormalize(&mFront,mHost->mGravity,receiver,sender);
        InfernoMario* host = mHost;
        MR::makeQuatUpFront(&host->mCurrentQuat,-host->mGravity,mFront);
        setNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvBlowDownEnemy::sInstance);MR::calcGravity(mHost);return true;
    }
    return false;
}
bool InfernoMarioManekuri::startFrame(s32 frame) {
    mFrame=frame;mHost->setFrameDelay(frame);mHost->readPacket(mFrame);MR::resetPosition(mHost);MR::invalidateClipping(mHost);
    setNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvJump::sInstance);return true;
}
void InfernoMarioManekuri::fn_80381750() {}
void InfernoMarioManekuri::fn_80381780() {
    if(isNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvFind::sInstance)||isNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvFollowStart::sInstance)||isNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvFollow::sInstance)) {
        mHost->fn_80381790();setNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvWait::sInstance);
        mModel->makeActorAppeared();MR::showModel(mModel);MR::hideModelAndOnCalcAnim(mHost);MR::onBind(mHost);MR::zeroVelocity(mHost);
        mHost->mCurrentQuat.getZDir(mFront);MR::onShadowVisibleSyncHostAll(mModel);MR::removeInfernoMario(mHost);
    }
}
bool InfernoMarioManekuri::isFollowing() const { return isNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvFollow::sInstance); }
bool InfernoMarioManekuri::isNerveBlowDownPlayer() const { return isNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvBlowDownPlayer::sInstance); }
bool InfernoMarioManekuri::isJumping() const { return isNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvJump::sInstance); }
bool InfernoMarioManekuri::canDamage() const { return isNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvFollow::sInstance); }
void InfernoMarioManekuri::control() {}
// NONMATCHING: epilogue register-load order and verified literal labels differ.
void InfernoMarioManekuri::exeWait() {
    if(MR::isFirstStep(this)) { MR::validateClipping(mHost);MR::startAction(mModel,"Wait");mModel->makeActorAppeared();MR::hideModelAndOnCalcAnim(mHost);MR::onBind(mHost); }
    if(!MR::isBindedGround(mHost)) MR::addVelocityToGravity(mHost,1.0f);
    MR::attenuateVelocity(mHost,0.95f);tryFind();
}
void InfernoMarioManekuri::exeFind() {
    if(MR::isFirstStep(this)) MR::startAction(mModel,"Find");
    if(!MR::isBindedGround(mHost)) MR::addVelocityToGravity(mHost,1.0f);
    MR::attenuateVelocity(mHost,0.95f);MR::turnDirectionToPlayerDegreeHorizon(mHost,&mFront,6.0f);
    MR::blendQuatFromGroundAndFront(&mHost->mCurrentQuat,mHost,mFront,0.2f,0.2f);
    if(MR::isActionEnd(mModel)&&MR::addInfernoMarioFirst(mHost)) {
        MR::zeroVelocity(mHost);mStartPos.set(mHost->mPosition);mEndPos.set(*MR::getPlayerPos());
        mFrame=mHost->mFrameSpacing;mHost->setFrameDelay(mFrame);MR::invalidateClipping(mHost);
        setNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvFollowStart::sInstance);
    }
}
void InfernoMarioManekuri::exeFollowStart() {
    if(MR::isFirstStep(this)) {
        MR::startAction(mModel,"FollowStart");MR::tryHitReaction(mHost,"変身開始");MR::hideModelAndOnCalcAnim(mModel);
        MR::offShadowVisibleSyncHostAll(mModel);MR::validateShadowAll(mModel);MR::offBind(mHost);
    }
    MR::turnDirectionToPlayerDegreeHorizon(mHost,&mFront,6.0f);MR::blendQuatFromGroundAndFront(&mHost->mCurrentQuat,mHost,mFront,0.2f,0.2f);
    TVec3f* position = &mHost->mPosition;
    MR::vecBlend(position,mStartPos,mEndPos,MR::calcNerveEaseInOutRate(this,mFrame));
    if(MR::isGreaterEqualStep(this,mFrame)) { MR::onShadowVisibleSyncHostAll(mModel);mHost->readTransform(mFrame);setNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvJump::sInstance); }
}
// NONMATCHING: vector temporary address lifetime, registers and instruction scheduling differ.
void InfernoMarioManekuri::exeJump() {
    if(MR::isFirstStep(this)) { MR::startAction(mHost,"Jump");MR::showModel(mHost);MR::tryHitReaction(mHost,"変身完了");mStartPos.set(mHost->mPosition); }
    f32 height=150.0f*MR::calcNerveParabolaValue(this,30);
    MR::setPosition(mHost, mStartPos + mHost->mGravity.scaleInline(-height));
    mFrame++;
    if(MR::isGreaterStep(this,30)) setNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvFollow::sInstance);
}
// NONMATCHING: temporary register choice and verified literal labels differ.
void InfernoMarioManekuri::exeFollow() {
    if(MR::isFirstStep(this)) mModel->makeActorDead();
    MR::startActionSound(mHost,"BmLvGhostMarioAmbient",-1,-1,-1);mHost->readPacket(mFrame);
    mHost->mCurrentQuat.slerp(mHost->mTargetQuat,0.25f);mHost->mCurrentQuat.normalize();
    if(mFrame>mHost->mFrameDelay) { if(MR::isGreaterStep(this,60)&&MR::isIntervalStep(this,4)) mFrame--; }
    else if(mFrame<mHost->mFrameDelay) { if(MR::isIntervalStep(this,2)) mFrame++; }
    if(MR::isPowerStarGetDemoActive()&&MR::isNearPlayer(mHost,500.0f)) kill();
}
void InfernoMarioManekuri::exeTrampleDown() {
    if(MR::isFirstStep(this)) { MR::showModel(mModel);MR::startAction(mModel,"TrampleDown");MR::hideModelAndOnCalcAnim(mHost); }
    if(MR::isActionEnd(mModel)) { MR::tryHitReactionDead(mHost);kill(); }
}
void InfernoMarioManekuri::exeBlowDownPlayer() {
    if(MR::isFirstStep(this)) { MR::startBlowHitReaction(mHost);MR::showModel(mModel);MR::startAction(mModel,"BlowDown");MR::hideModelAndOnCalcAnim(mHost); }
    if(MR::isGreaterStep(this,5)&&MR::isBindedWall(mHost)) {
        InfernoMario* host=mHost;
        MR::sendMsgEnemyAttackExplosionToBindedSensor(host,host->getSensor("Body"));
        host=mHost;
        MR::sendMsgEnemyAttackToBindedSensor(host,host->getSensor("Body"));MR::tryHitReactionDead(mHost);kill();return;
    }
    if(MR::isGreaterStep(this,180)) { MR::tryHitReactionDead(mHost);kill(); }
}
void InfernoMarioManekuri::exeBlowDownEnemy() {
    if(MR::isFirstStep(this)) { MR::startBlowHitReaction(mHost);MR::showModel(mModel);MR::startAction(mModel,"BlowDown");MR::hideModelAndOnCalcAnim(mHost); }
    MR::attenuateVelocity(mHost,0.9f);
    if(MR::isGreaterStep(this,15)) { MR::tryHitReactionDead(mHost);kill(); }
}
bool InfernoMarioManekuri::tryFind() { if(MR::isNearPlayer(mHost,900.0f)) { setNerve(&NrvInfernoMarioManekuri::InfernoMarioManekuriNrvFind::sInstance);return true; }return false; }
