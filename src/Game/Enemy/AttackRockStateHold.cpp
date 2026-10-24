// SMG2 calls TVec3 operator+/operator*/add/scale out of line in this TU (appear); the define does that.
#define JGEOMETRY_VEC3_WMR_OUTLINE
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Enemy/AttackRock.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/Boss/KoopaFinalBattlePos.hpp"

#include "Game/Boss/KoopaBattleAccessor.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"

// ---- AttackRockStateHold ----

namespace NrvAttackRockStateHold {
    class AttackRockStateHoldNrvFloat : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockStateHoldNrvFloat sInstance;
    };
    class AttackRockStateHoldNrvPower : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockStateHoldNrvPower sInstance;
    };
    class AttackRockStateHoldNrvFloatEnd : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockStateHoldNrvFloatEnd sInstance;
    };
    class AttackRockStateHoldNrvFallBreak : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static AttackRockStateHoldNrvFallBreak sInstance;
    };
    AttackRockStateHoldNrvFloat AttackRockStateHoldNrvFloat::sInstance ATTRIBUTE_WEAK;
    AttackRockStateHoldNrvPower AttackRockStateHoldNrvPower::sInstance ATTRIBUTE_WEAK;
    AttackRockStateHoldNrvFloatEnd AttackRockStateHoldNrvFloatEnd::sInstance ATTRIBUTE_WEAK;
    AttackRockStateHoldNrvFallBreak AttackRockStateHoldNrvFallBreak::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvAttackRockStateHold


// NONMATCHING: Target addresses the four ctor strings as one 0x40-byte object lbl_806825F0 (base+0/0x18/0x24/0x38); ours uses the anonymous .data base (+0x18 start) and pRock/rockNum callee-saved registers are swapped.
AttackRockStateHold::AttackRockStateHold(AttackRock* pRock, s32 rockNum)
    : ActorStateBase< AttackRock >("\215U\214\202\212\342\225\202\202\251\202\361\202\305\202\242\202\351\217\363\221\324", pRock), mRide(nullptr), mBreakModel(nullptr) {
    mStartPos.x = 0.0f;
    mStartPos.y = 0.0f;
    mStartPos.z = 0.0f;
    mUpVec.x = 0.0f;
    mUpVec.y = 1.0f;
    mUpVec.z = 0.0f;
    mRate = 0.0f;
    mRockNum = rockNum;
    initNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFloat::sInstance, 0);
    mRide = new AttackRockRide(pRock);
    mBreakModel = new BreakModel(mHost, "\225\366\202\352\203\202\203f\203\213", "AttackRockFallBreak", mHost->getBaseMtx(), "Break");
    mBreakModel->initWithoutIter();
}

AttackRockStateHold::~AttackRockStateHold() {
}

#pragma auto_inline off
// NONMATCHING: Only vector temporary stack slot offsets differ (calls, operands and registers match).
void AttackRockStateHold::appear() {
    mIsDead = false;
    TVec3f up;
    MR::calcUpVec(&up, mHost);
    TVec3f dir;
    MR::getKoopaBattlePos(&dir);
    const TVec3f& hostPos = mHost->mPosition;
    TVec3f target;
    target.set(hostPos + up * 700.0f);
    TVec3f toTarget = target - dir;
    MR::normalizeOrZero(&dir, toTarget);
    TVec3f scaled = dir * (0.5f * up.dot(dir));
    TVec3f side = up - scaled;
    mUpVec.set(side);
    MR::normalizeOrZero(&mUpVec);
    mRate = 0.0f;
    mStartPos.set(mHost->mPosition);
    setNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFloat::sInstance);
    mHost->mDemoObj->kill();
}
#pragma auto_inline on

void AttackRockStateHold::kill() {
    mIsDead = true;
    mRide->endRide();
}

// NONMATCHING: Quaternion-copy temporary placement and floating-point scheduling differ.
void AttackRockStateHold::control() {
    TVec3f dir;
    MR::getKoopaBattleTargetPos(&dir);
    TVec3f toPlayer = mHost->mPosition - dir;
    MR::normalizeOrZero(&dir, toPlayer);
    MR::turnQuatZDirRate(&mHost->mQuat100, mHost->mQuat100, dir, 0.3f);
    TVec3f zdir;
    TQuat4f quat(mHost->mQuat100);
    quat.getZDir(zdir);
    mRide->setHostZDir(&zdir);
    mRide->updateRush();
}

bool AttackRockStateHold::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgKoopaBA(msg)) {
        mBreakModel->makeActorDead();
        kill();
        return true;
    }
    if (MR::isMsgKoopaBB(msg) && !isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFallBreak::sInstance)) {
        setNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFallBreak::sInstance);
        return true;
    }
    if (mRide->tryReceiveRideMsg(msg, pSender, pReceiver)) {
        return true;
    }
    return false;
}

bool AttackRockStateHold::isPowerOrFloatEnd() {
    if (this->isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvPower::sInstance) ||
        this->isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFloatEnd::sInstance)) {
        return true;
    }
    return false;
}

bool AttackRockStateHold::isFloating() {
    if (this->isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFloat::sInstance) ||
        this->isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvPower::sInstance) ||
        this->isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFloatEnd::sInstance)) {
        return true;
    }
    return false;
}

#pragma dont_inline on
void AttackRockStateHold::updateHoldPosition() {
    AttackRock* pHost = this->mHost;
    TVec3f pos(this->mStartPos + this->mUpVec * this->mRate);
    MR::setPosition(pHost, pos);
}
#pragma dont_inline off

void AttackRockStateHold::exeHoldFloat() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this->mHost, "Float");
        MR::validateShadow(this->mHost, nullptr);
        this->mRide->enableRide();
    }
    MR::startActionSound(this->mHost, "Power", -1, -1, -1);
    this->mRate = MR::calcNerveEaseOutValue(this, 0, 40, 0.0f, 700.0f);
    this->updateHoldPosition();
    AttackRock* pHost = this->mHost;
    f32 max = MR::getBrkFrameMax(pHost);
    MR::setBrkFrameAndStop(pHost, this->mRate * max / 700.0f);
    MR::setStageBGMStateBit(0x80);
    if (MR::isGreaterStep(this, 40)) {
        this->setNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvPower::sInstance);
    }
}

void AttackRockStateHold::exeHoldPower() {
    if (MR::isFirstStep(this)) {
        MR::validateShadow(this->mHost, nullptr);
        this->mRate = 700.0f;
        this->mRide->enableRide();
    }
    MR::startActionSound(this->mHost, "Power", -1, -1, -1);
    this->updateHoldPosition();
    if (MR::isOnGroundPlayer() && !MR::isActorOnPlayer(this->mHost)) {
        this->mRide->enableRide();
    }
    MR::setStageBGMStateBit(0x80);
    s32 step;
    switch (this->mRockNum) {
    case 1:
        step = 240;
        break;
    case 2:
        step = 210;
        break;
    case 3:
        step = 180;
        break;
    default:
        step = 240;
        break;
    }
    if (MR::isGreaterStep(this, step)) {
        this->setNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFloatEnd::sInstance);
    }
}

void AttackRockStateHold::exeHoldFloatEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this->mHost, "FloatEnd");
    }
    AttackRock* pHost = this->mHost;
    f32 rate = MR::calcNerveEaseInOutValue(this, 30, 1.0f, 0.0f);
    f32 max = MR::getBrkFrameMax(pHost);
    MR::setBrkFrameAndStop(pHost, max * rate);
    if (MR::isGreaterStep(this, 30)) {
        this->setNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFallBreak::sInstance);
    }
}

void AttackRockStateHold::exeHoldFallBreak() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this->mHost, "FallBreak");
        this->mBreakModel->appear();
        MR::invalidateCollisionParts(this->mHost);
        MR::invalidateHitSensors(this->mHost);
        MR::zeroVelocity(this->mHost);
        this->mRide->endRide();
    }
    AttackRock* pHost = this->mHost;
    f32 base = pHost->getRockBaseRadius();
    base *= pHost->mScale.y;
    MR::setShadowVolumeSphereRadius(this->mHost, nullptr, MR::calcNerveValue(this, 30, 0.95f * base, 0.05f * base));
    if (MR::isStep(this, 1)) {
        MR::hideModelAndOnCalcAnim(this->mHost);
    }
    if (MR::isGreaterStep(this, 30)) {
        MR::invalidateShadow(this->mHost, nullptr);
        this->kill();
    }
}

namespace NrvAttackRockStateHold {
    inline void AttackRockStateHoldNrvFloat::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockStateHold* >(pSpine->mExecutor))->exeHoldFloat();
    }
    inline void AttackRockStateHoldNrvPower::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockStateHold* >(pSpine->mExecutor))->exeHoldPower();
    }
    inline void AttackRockStateHoldNrvFloatEnd::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockStateHold* >(pSpine->mExecutor))->exeHoldFloatEnd();
    }
    inline void AttackRockStateHoldNrvFallBreak::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockStateHold* >(pSpine->mExecutor))->exeHoldFallBreak();
    }
};  // namespace NrvAttackRockStateHold

// ---- AttackRockStateFall ----

