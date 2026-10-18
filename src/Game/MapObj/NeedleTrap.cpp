#pragma auto_inline off
#include "Game/MapObj/NeedleTrap.hpp"
#include "Game/LiveActor/CollisionObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
namespace NrvNeedleTrap {
    NEW_NERVE(NeedleTrapNrvWaitFirst, NeedleTrap, WaitFirst);
    NEW_NERVE(NeedleTrapNrvWait, NeedleTrap, Wait);
    NEW_NERVE(NeedleTrapNrvAttackSign, NeedleTrap, AttackSign);
    NEW_NERVE(NeedleTrapNrvAttack, NeedleTrap, Attack);
    NEW_NERVE(NeedleTrapNrvAttackEnd, NeedleTrap, AttackEnd);
    NEW_NERVE(NeedleTrapNrvFreeze, NeedleTrap, Freeze);
}
NeedleTrap::NeedleTrap(const char* name) : LiveActor(name), mNeedleCollision(nullptr), mFirstWait(0) {}
NeedleTrap::~NeedleTrap() {}
void NeedleTrap::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    initNerve(&NrvNeedleTrap::NeedleTrapNrvWaitFirst::sInstance);
    MtxPtr mtx = MR::getJointMtx(this, "ColPos");
    mNeedleCollision = MR::createCollisionObj(this, "Needle", getSensor(nullptr), mtx);
    MR::invalidateCollisionParts(mNeedleCollision);
    MR::getJMapInfoArg0NoInit(iter, &mFirstWait);
    makeActorAppeared();
}
void NeedleTrap::control() {
    if (isNotNerveFreeze() && MR::attachSupportTicoToTarget(this)) setNerve(&NrvNeedleTrap::NeedleTrapNrvFreeze::sInstance);
}
void NeedleTrap::exeWaitFirst() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Wait");
    if (MR::isGreaterStep(this, mFirstWait)) setNerve(&NrvNeedleTrap::NeedleTrapNrvWait::sInstance);
}
void NeedleTrap::exeWait() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Wait");
    if (MR::isGreaterStep(this, 150)) setNerve(&NrvNeedleTrap::NeedleTrapNrvAttackSign::sInstance);
}
void NeedleTrap::exeAttackSign() {
    if (MR::isFirstStep(this)) MR::startAction(this, "AttackSign");
    if (MR::isActionEnd(this)) setNerve(&NrvNeedleTrap::NeedleTrapNrvAttack::sInstance);
}
void NeedleTrap::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        MR::validateCollisionParts(mNeedleCollision);
    }
    if (MR::isGreaterStep(this, 90)) {
        MR::invalidateCollisionParts(mNeedleCollision);
        setNerve(&NrvNeedleTrap::NeedleTrapNrvAttackEnd::sInstance);
    }
}
void NeedleTrap::exeAttackEnd() {
    if (MR::isFirstStep(this)) MR::startAction(this, "AttackEnd");
    if (MR::isActionEnd(this)) setNerve(&NrvNeedleTrap::NeedleTrapNrvWait::sInstance);
}
void NeedleTrap::exeFreeze() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SupportFreeze");
        MR::invalidateCollisionParts(mNeedleCollision);
    }
    if (!MR::attachSupportTicoToTarget(this)) setNerve(&NrvNeedleTrap::NeedleTrapNrvWait::sInstance);
}
bool NeedleTrap::isNotNerveFreeze() const { return !isNerve(&NrvNeedleTrap::NeedleTrapNrvFreeze::sInstance); }
