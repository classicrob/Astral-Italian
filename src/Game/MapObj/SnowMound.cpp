// NONMATCHING: skeleton. fn_80303CD0 (0x1a0 bytes), the SnowBall spawn loop, the WhiteSnowTree/csv branches in init,
// and the nerve bodies are not ported yet; those parts are approximations or omitted.
#include "Game/MapObj/SnowMound.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

namespace NrvSnowMound {
    class SnowMoundNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }

        static SnowMoundNrvWait sInstance;
    };
    SnowMoundNrvWait SnowMoundNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(SnowMoundNrvMelt, SnowMound, Melt);
    NEW_NERVE(SnowMoundNrvHide, SnowMound, Hide);
    NEW_NERVE(SnowMoundNrvHidden, SnowMound, Hidden);
};  // namespace NrvSnowMound

SnowMound::SnowMound(const char* pName) : LiveActor(pName) {
    mBreakModel = nullptr;
    mBreakDustDemo = nullptr;
    mSnowBalls = nullptr;
    mSnowBallNum = 1;
    mCsvValue = 1;
    mIsCsvFlag = true;
    mSnowTree = nullptr;
    mPowerStarId = -1;
    mIsMelt = false;
}

void SnowMound::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    MR::initMapPartsClipping(this, rIter, nullptr, false);
    initNerve(&NrvSnowMound::SnowMoundNrvWait::sInstance, 0);
    makeActorAppeared();
}

void SnowMound::kill() {
    LiveActor::kill();
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
    afterKill();
}

void SnowMound::afterKill() {
}

bool SnowMound::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHitProjectiles(msg)) {
        startMelt();
        return true;
    }
    return false;
}

bool SnowMound::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgToEnemyAttackRoll(msg)) {
        startMelt();
        return true;
    }
    if (mIsMelt && MR::isMsgEnemyAttack(msg)) {
        startMelt();
        return true;
    }
    return false;
}

void SnowMound::startMelt() {
    setNerve(&NrvSnowMound::SnowMoundNrvMelt::sInstance);
}

void SnowMound::exeMelt() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Damage");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvSnowMound::SnowMoundNrvHide::sInstance);
    }
}

void SnowMound::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
    }
    if (MR::isGreaterStep(this, 30)) {
        setNerve(&NrvSnowMound::SnowMoundNrvHidden::sInstance);
    }
}

void SnowMound::exeHidden() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
    }
}

SnowMound::~SnowMound() {
}
