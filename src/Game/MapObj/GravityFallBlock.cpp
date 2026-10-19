#include "Game/MapObj/GravityFallBlock.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

namespace NrvGravityFallBlock {
    NEW_NERVE(GravityFallBlockNrvWait, GravityFallBlock, Wait);
    class GravityFallBlockNrvStartFall : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;

        static GravityFallBlockNrvStartFall sInstance;
    };
    GravityFallBlockNrvStartFall GravityFallBlockNrvStartFall::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(GravityFallBlockNrvFall, GravityFallBlock, Fall);

    inline void GravityFallBlockNrvStartFall::execute(Spine* pSpine) const {
        GravityFallBlock* actor = reinterpret_cast< GravityFallBlock* >(pSpine->mExecutor);
        if (MR::isGreaterStep(actor, 15)) {
            actor->setNerve(&NrvGravityFallBlock::GravityFallBlockNrvFall::sInstance);
        }
    }

    NEW_NERVE(GravityFallBlockNrvBreak, GravityFallBlock, Break);
};  // namespace NrvGravityFallBlock

GravityFallBlock::GravityFallBlock(const char* pName) : LiveActor(pName) {
    mBreakModel = nullptr;
    mBasePos.x = 0.0f;
    mBasePos.y = 0.0f;
    mBasePos.z = 0.0f;
    mUpDir.set(0.0f, 1.0f, 0.0f);
}

void GravityFallBlock::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    mBasePos.set< f32 >(mPosition);
    MR::calcActorAxisY(&mUpDir, this);
    MR::addEffectHitNormal(this, "Hit");
    initNerve(&NrvGravityFallBlock::GravityFallBlockNrvWait::sInstance, 0);
    mBreakModel = MR::tryCreateBreakModel(this, nullptr);
    makeActorAppeared();
}

void GravityFallBlock::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::onCalcGravity(this);
        MR::offBind(this);
    }
    TVec3f up = mUpDir * 150.0f;
    TVec3f pos = mPosition + up;
    TVec3f dir = mGravity * 155.0f;
    if (!MR::isExistMapCollisionExceptSensor(pos, dir, reinterpret_cast< const LiveActor* >(getSensor("Body")))) {
        setNerve(&NrvGravityFallBlock::GravityFallBlockNrvStartFall::sInstance);
    }
}

void GravityFallBlock::exeFall() {
    if (MR::isFirstStep(this)) {
        MR::onBind(this);
    }
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    MR::addVelocityToGravity(this, 0.4f);
    if (MR::isGreaterStep(this, 30) && MR::isBindedGround(this) && MR::isNearZero(MR::getGroundActor(this)->mVelocity, 0.001f)) {
        TVec3f diff(mPosition);
        diff -= mBasePos;
        TVec3f proj = mGravity * diff.dot(mGravity);
        TVec3f pos = proj + mBasePos;
        mPosition.set(pos);
        MR::zeroVelocity(this);
        MR::offBind(this);
        MR::offCalcGravity(this);
        MR::shakeCameraNormalWeak();
        MR::tryRumblePadWeak(this, 0);
        MR::emitEffectHit(this, MR::getGroundHitPos(this), "Hit");
        mBasePos.set< f32 >(mPosition);
        setNerve(&NrvGravityFallBlock::GravityFallBlockNrvBreak::sInstance);
    }
}

void GravityFallBlock::exeBreak() {
    if (MR::isFirstStep(this)) {
        mBreakModel->appear();
        MR::startAction(mBreakModel, "Break");
        MR::invalidateClipping(mBreakModel);
        MR::invalidateCollisionParts(this);
        MR::hideModel(this);
    }
    if (MR::isActionEnd(mBreakModel)) {
        mBreakModel->kill();
        kill();
    }
}

bool GravityFallBlock::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvGravityFallBlock::GravityFallBlockNrvBreak::sInstance)) {
        return false;
    }
    if (MR::isMsgFireBallAttack(msg)) {
        MR::emitEffectHitBetweenSensors(this, pReceiver, pSender, 0.0f, "Hit");
        setNerve(&NrvGravityFallBlock::GravityFallBlockNrvBreak::sInstance);
        return true;
    }
    return false;
}

GravityFallBlock::~GravityFallBlock() {
}
