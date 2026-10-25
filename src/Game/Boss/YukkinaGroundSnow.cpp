#include "Game/Boss/YukkinaGroundSnow.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SupportPlayUtil.hpp"

namespace NrvYukkinaGroundSnow {
    class YukkinaGroundSnowNrvHidden : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static YukkinaGroundSnowNrvHidden sInstance;
    };
    YukkinaGroundSnowNrvHidden YukkinaGroundSnowNrvHidden::sInstance;

    NEW_NERVE(YukkinaGroundSnowNrvAppear, YukkinaGroundSnow, Appear);
    NEW_NERVE(YukkinaGroundSnowNrvWait, YukkinaGroundSnow, Wait);
    NEW_NERVE(YukkinaGroundSnowNrvBreak, YukkinaGroundSnow, Break);

    inline void YukkinaGroundSnowNrvHidden::execute(Spine* pSpine) const {
        YukkinaGroundSnow* pActor = reinterpret_cast< YukkinaGroundSnow* >(pSpine->mExecutor);
        if (MR::isGreaterStep(pActor, 0x78)) {
            bool far = !MR::isNearPlayer(pActor, 1000.0f);
            if (far) {
                pActor->setNerve(&YukkinaGroundSnowNrvAppear::sInstance);
            }
        }
    }
}  // namespace NrvYukkinaGroundSnow

YukkinaGroundSnow::YukkinaGroundSnow(const char* pName) : LiveActor(pName), mBreakModel(NULL), mBreakCount(0), mItemIndex(0) {
}

void YukkinaGroundSnow::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "YukkinaGroundSnow", false);
    initNerve(GET_NERVE(YukkinaGroundSnow, YukkinaGroundSnowNrvWait), 0);
    mBreakModel = MR::createBreakModel(this, NULL);
    makeActorAppeared();
    MR::getJMapInfoArg0NoInit(rIter, &mItemIndex);
    mItemIndex = (mItemIndex + 4) % 4;
}

void YukkinaGroundSnow::control() {
}

bool YukkinaGroundSnow::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHitAll(msg)) {
        return tryBreak();
    }
    return false;
}

bool YukkinaGroundSnow::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgEnemyAttack(msg) || MR::isMsgExplosionAttack(msg)) {
        return tryBreak();
    }
    return false;
}

bool YukkinaGroundSnow::tryBreak() {
    if (isNerve(GET_NERVE(YukkinaGroundSnow, YukkinaGroundSnowNrvWait))) {
        setNerve(GET_NERVE(YukkinaGroundSnow, YukkinaGroundSnowNrvBreak));
        return true;
    }
    return false;
}

void YukkinaGroundSnow::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::startAction(this, "Appear");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(YukkinaGroundSnow, YukkinaGroundSnowNrvWait));
    }
}

void YukkinaGroundSnow::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::validateCollisionParts(this);
    }
    MR::attachSupportTicoToTarget(this);
}

void YukkinaGroundSnow::exeBreak() {
    if (MR::isFirstStep(this)) {
        if (mItemIndex == mBreakCount % 4) {
            MR::emitItemHit(this);
        }
        mBreakCount++;
        mBreakModel->appear();
        MR::hideModel(this);
        MR::invalidateCollisionParts(this);
        setNerve(GET_NERVE(YukkinaGroundSnow, YukkinaGroundSnowNrvHidden));
        MR::tryHitReactionBreak(this);
    }
}
