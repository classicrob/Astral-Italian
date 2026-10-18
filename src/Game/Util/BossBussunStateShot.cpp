#include "Game/Util/BossBussun.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvBossBussunStateShot {
    NEW_NERVE(BossBussunStateShotNrvShotSign, BossBussunStateShot, ShotSign);
    NEW_NERVE(BossBussunStateShotNrvShot, BossBussunStateShot, Shot);
};  // namespace NrvBossBussunStateShot

BossBussunStateShot::BossBussunStateShot(BossBussun* pHost) : ActorStateBase< BossBussun >("ボスブッスン、ショット！", pHost), mShotGroup(nullptr), mFixedPos(nullptr), mShotCount(3), mShotCountMax(3) {
    mFixedPos = new FixedPosition(mHost, "ShotPos", nullptr);
    mShotGroup = new LiveActorGroup("弾リスト", 0x20);
    mShotGroup->initWithoutIter();
    for (s32 i = 0; i < 0x20; i++) {
        BossBussunShot* pShot = new BossBussunShot("ボスブッスン弾");
        pShot->mPosition.set(mHost->mPosition);
        pShot->initWithoutIter();
        mShotGroup->registerActor(pShot);
    }
    initNerve(GET_NERVE(BossBussunStateShot, BossBussunStateShotNrvShotSign), 0);
}

void BossBussunStateShot::appear() {
    mIsDead = false;
    setNerve(GET_NERVE(BossBussunStateShot, BossBussunStateShotNrvShotSign));
    mShotCount = mShotCountMax;
}

void BossBussunStateShot::setShotCountMax(s32 count) {
    mShotCountMax = count;
}

void BossBussunStateShot::killAll() {
    mShotGroup->makeAllDead();
}

BossBussunStateShot::~BossBussunStateShot() {
}
