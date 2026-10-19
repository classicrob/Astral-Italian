#include "Game/Boss/KoopaDemoChamber.hpp"
#include "Game/Boss/KoopaBattleAccessor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/DemoObj.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"
namespace NrvKoopaDemoChamber {
NEW_NERVE(KoopaDemoChamberNrvWait, KoopaDemoChamber, Wait);
NEW_NERVE(KoopaDemoChamberNrvBreak, KoopaDemoChamber, Break);
}
KoopaDemoChamber::KoopaDemoChamber(const char* name) : LiveActor(name), mChildGroup(0), mBreakObj(0), mPlayerPosition(0) {}
void KoopaDemoChamber::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    if (mChildGroup) mChildGroup->appearAll();
}
void KoopaDemoChamber::makeActorDead() {
    LiveActor::makeActorDead();
    if (mBreakObj) mBreakObj->makeActorDead();
    if (mChildGroup) mChildGroup->makeAllDead();
}
void KoopaDemoChamber::init(const JMapInfoIter& iter) {
    MR::createKoopaBattleAccessor();
    MR::getKoopaBattleAccessor()->registerKoopaDemoChamber(this);
    MR::initActor(this, iter, false);
    MR::addBaseMatrixFollowTarget(this, iter, 0, 0);
    initNerve(&NrvKoopaDemoChamber::KoopaDemoChamberNrvWait::sInstance);
    MR::invalidateClipping(this);
    mChildGroup = MR::initChildObjGroup(this, "子供管理", iter);
    bool hasBreak = false;
    MR::getJMapInfoArg0WithInit(iter, &hasBreak);
    if (hasBreak) {
        mBreakObj = MR::createDemoObj("玉座壊れモデル", "KoopaDemoChamberBreak", getBaseMtx());
        MR::invalidateClipping(mBreakObj);
        mBreakObj->makeActorDead();
    }
    mPlayerPosition = new FixedPosition(this, "DemoEndPlayerPos", 0);
    makeActorDead();
}
void KoopaDemoChamber::getDemoEndPlayerPos(MtxPtr out) const { reinterpret_cast<TPos3f*>(out)->set(mPlayerPosition->mMtx); }
void KoopaDemoChamber::control() { mPlayerPosition->calc(); }
void KoopaDemoChamber::startBreak() { setNerve(&NrvKoopaDemoChamber::KoopaDemoChamberNrvBreak::sInstance); }
void KoopaDemoChamber::exeBreak() {
    if (MR::isFirstStep(this)) {
        if (mChildGroup) mChildGroup->makeAllDead();
        if (mBreakObj) { mBreakObj->appear(); MR::startAction(mBreakObj, "Break"); }
        MR::startAction(this, "Break");
    }
}
KoopaDemoChamber::~KoopaDemoChamber() {}
void KoopaDemoChamber::exeWait() {}
