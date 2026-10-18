#include "Game/Enemy/AttackRock.hpp"
f32 fn_801510C0();
bool fn_801510D0(LiveActor* pActor);
bool fn_801517C0(LiveActor* pActor, s32 arg);
void fn_801552A0(LiveActor* pActor);
f32 fn_801552E0(LiveActor* pActor, TQuat4f* pQuat);
void fn_80155360(TVec3f* pOut, TQuat4f* pQuat, HitSensor* pSender, HitSensor* pReceiver, f32 speed);
void fn_801553D0(TVec3f* pOut, TVec3f* pDir, HitSensor* pSender, HitSensor* pReceiver, f32 speed);
void fn_801554D0(TPos3f* pMtx, LiveActor* pActor);
void fn_80155540(ProjmapEffectMtxSetter* pSetter, TQuat4f* pQuat, TVec3f* pPos);
ProjmapEffectMtxSetter* fn_80155660(LiveActor* pActor);
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

class BaseMatrixFollowValidater;
class KoopaBattleAccessor {
public:
    void addAttackRockFinal(AttackRockFinal*);
};

namespace MR {
    bool isMsgKoopaCastAttackRockStart(u32);
    bool isMsgKoopaCastAttackRockEnd(u32);
    bool isMsgKoopaPunchAttack(u32);
    bool isMsgKoopaReleaseAttackRock(u32);
    bool isMsgKoopaFinalBattleStart(u32);
    bool isMsgAutoRushBegin(u32);
    bool isMsgUpdateBaseMtx(u32);
    bool isMsgRushCancel(u32);
    bool isMsgFloorTouch(u32);
    f32 calcNerveEaseOutValue(const NerveExecutor*, s32, s32, f32, f32);
    void createKoopaBattleAccessor();
    KoopaBattleAccessor* getKoopaBattleAccessor();
};  // namespace MR

// Unnamed SMG2 helpers outside this unit (Koopa battle functions).

// Unit-local helpers (unnamed in the target).

AttackRockRide* AttackRockRide::fn_801528E0(LiveActor* pHost) {
    this->mHost = pHost;
    this->mQuat0.set(0.0f, 0.0f, 0.0f, 1.0f);
    this->mQuat1.set(0.0f, 0.0f, 0.0f, 1.0f);
    this->mQuat2.set(0.0f, 0.0f, 0.0f, 1.0f);
    this->_34.set(0.0f, 0.0f, 1.0f);
    this->_40.set(0.0f, 0.0f, 0.0f);
    this->_4C.set(0.0f, 0.0f, 0.0f);
    this->_58.set(0.0f, 0.0f, 0.0f);
    this->_64.set(0.0f, 0.0f, 1.0f);
    this->_70.set(0.0f, 0.0f, 1.0f);
    this->_7C.set(0.0f, 0.0f, 1.0f);
    this->_88 = 0.0f;
    this->_8C = 0.0f;
    this->_90 = 0.0f;
    this->_94 = 0;
    this->_98 = 0;
    this->mIsRush = false;
    this->mIsEnable = false;
    return this;
}

// NONMATCHING: TVec3f operator= is expanded inline (psq copy) in the target.
void AttackRockRide::fn_801529E0(const TVec3f* pVec) {
    this->_34 = *pVec;
}

// NONMATCHING: vector math written from the m2c output; operand order / temporaries not verified.
void AttackRockRide::fn_80152A00() {
    TVec3f last = *MR::getPlayerLastMove();
    TVec3f tmp(last);
    tmp.scale(0.9f);
    this->_4C.set(tmp);
    this->_58.set(this->_4C);
    MR::calcDirToPlayer(&this->_64, this->mHost);
    TVec3f playerMove = *MR::getPlayerLastMove();
    TVec3f sum(this->_40);
    sum.add(playerMove);
    TVec3f dir = sum - this->mHost->mPosition;
    MR::normalizeOrZero(&dir);
    this->_88 = MR::diffAngleAbs(this->_64, dir);
    this->_8C = 0.0f;
    TVec3f cross;
    PSVECCrossProduct(&this->_64, &dir, &this->_7C);
    if (MR::normalizeOrZero(&this->_7C)) {
        MR::makeAxisVerticalZX(&this->_7C, this->_64);
    }
    this->_98 = 0;
    this->mQuat1.set(this->mQuat0);
    TVec3f rotated;
    MR::rotateVecRadian(&rotated, this->_64, this->_7C, 10.0f * this->_88);
    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::turnVecToVecDegree(&this->_70, this->_34, rotated, 45.0f, up);
    TVec3f side = this->_70 - this->_70;
    if (MR::isNearZero(side, 0.001f)) {
        MR::makeAxisVerticalZX(&side, this->_70);
    }
    MR::makeQuatUpFront(&this->mQuat2, this->_70, side);
    f32 angle = 57.29578f * MR::diffAngleAbs(this->_64, this->_70);
    s32 frames = (s32)(angle / 3.0f);
    this->_94 = frames + 15;
    this->_90 = PSVECDistance(&this->_40, &this->mHost->mPosition);
    MR::startBckPlayer("Fall", "SpinDriverFall");
}

// NONMATCHING: vector math written from the m2c output; operand order / temporaries not verified.
void AttackRockRide::fn_80152C00() {
    if (this->mIsRush) {
        f32 a = this->_88;
        f32 sum = this->_8C + a;
        this->_8C = sum;
        f32 t = (f32)this->_98 / (f32)this->_94;
        this->_88 = a * 0.95f;
        f32 invT = 1.0f - t;
        TVec3f rotated;
        MR::rotateVecRadian(&rotated, this->_64, this->_7C, sum * invT);
        TVec3f blend;
        MR::vecBlendSphere(&blend, this->_64, this->_70, t);
        MR::normalizeOrZero(&blend);
        f32 len = MR::lerp(this->_90, 300.0f, t);
        TVec3f pos(blend);
        pos.x *= len;
        pos.y *= len;
        pos.z *= len;
        this->_40.set(pos);
        this->_40.add(this->mHost->mPosition);
        TVec3f drift(this->_58);
        drift.scale(invT);
        this->_40.add(drift);
        this->mQuat0.set(this->mQuat1);
        this->mQuat0.slerp(this->mQuat2, t);
        this->_4C.x *= 0.9f;
        this->_98 += 1;
        this->_4C.y *= 0.9f;
        this->_4C.z *= 0.9f;
        this->_58.add(this->_4C);
        if (this->_98 > this->_94) {
            TVec3f jump(this->_70);
            jump.scale(-20.0f);
            MR::endBindAndPlayerJump(this->mHost, jump, 0);
            this->mIsRush = false;
            this->mIsEnable = false;
        }
    }
}

bool AttackRockRide::fn_80152E20(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgAutoRushBegin(msg) && !MR::isOnGroundPlayer() && this->mIsEnable) {
        this->fn_80152A00();
        this->mIsRush = true;
        this->mIsEnable = false;
        return true;
    }
    if (MR::isMsgUpdateBaseMtx(msg) && this->mIsRush) {
        MR::fn_800513C0(&this->mQuat0, &this->_40);
        return true;
    }
    if (MR::isMsgRushCancel(msg)) {
        this->mIsRush = false;
        return true;
    }
    if (MR::isMsgFloorTouch(msg)) {
        this->mIsEnable = false;
    }
    return false;
}

void AttackRockRide::fn_80152F10() {
    this->mIsEnable = true;
}

void AttackRockRide::fn_80152F20() {
    if (this->mIsRush) {
        MR::endBindAndPlayerWait(this->mHost);
        this->mIsRush = false;
    }
    this->mIsEnable = false;
}

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


AttackRockStateHold::AttackRockStateHold(AttackRock* pRock, s32 rockNum)
    : ActorStateBase< AttackRock >("\215U\214\202\212\342\225\202\202\251\202\361\202\305\202\242\202\351\217\363\221\324", pRock), mRide(nullptr), mBreakModel(nullptr), mStartPos(0.0f, 0.0f, 0.0f), mUpVec(0.0f, 1.0f, 0.0f),
      mRate(0.0f), mRockNum(rockNum) {
    initNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFloat::sInstance, 0);
    mRide = new AttackRockRide;
    mRide->fn_801528E0(pRock);
    mBreakModel = new BreakModel(mHost, "\225\366\202\352\203\202\203f\203\213", "AttackRockFallBreak", mHost->getBaseMtx(), "Break");
    mBreakModel->initWithoutIter();
}

AttackRockStateHold::~AttackRockStateHold() {
}

// NONMATCHING: vector math written from the m2c output; operand order / temporaries not verified.
void AttackRockStateHold::appear() {
    mIsDead = false;
    TVec3f up;
    MR::calcUpVec(&up, mHost);
    TVec3f dir;
    TVec3f offset = up * 700.0f;
    TVec3f target(mHost->mPosition + offset);
    TVec3f toTarget = dir - target;
    MR::normalizeOrZero(&dir, toTarget);
    TVec3f scaled = up * (0.5f * up.dot(dir));
    TVec3f side = scaled - up;
    mUpVec.set(side);
    MR::normalizeOrZero(&mUpVec);
    mRate = 0.0f;
    mStartPos.set(mHost->mPosition);
    setNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFloat::sInstance);
    mHost->mDemoObj->kill();
}

void AttackRockStateHold::kill() {
    mIsDead = true;
    mRide->fn_80152F20();
}

void AttackRockStateHold::control() {
    TVec3f dir;
    TVec3f toPlayer = dir - mHost->mPosition;
    MR::normalizeOrZero(&dir, toPlayer);
    MR::turnQuatZDirRate(&mHost->mQuat100, mHost->mQuat100, dir, 0.3f);
    TVec3f zdir;
    mHost->mQuat100.getZDir(zdir);
    mRide->fn_801529E0(&zdir);
    mRide->fn_80152C00();
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
    if (mRide->fn_80152E20(msg, pSender, pReceiver)) {
        return true;
    }
    return false;
}

bool AttackRockStateHold::fn_80153400() {
    if (this->isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvPower::sInstance) ||
        this->isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFloatEnd::sInstance)) {
        return true;
    }
    return false;
}

bool AttackRockStateHold::fn_80153460() {
    if (this->isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFloat::sInstance) ||
        this->isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvPower::sInstance) ||
        this->isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFloatEnd::sInstance)) {
        return true;
    }
    return false;
}

// NONMATCHING: operator*/operator+ out-of-line vs inline decisions differ (target was a separate TU).
void AttackRockStateHold::fn_801534D0() {
    AttackRock* pHost = this->mHost;
    TVec3f offset = this->mUpVec * this->mRate;
    TVec3f pos(this->mStartPos + offset);
    MR::setPosition(pHost, pos);
}

void AttackRockStateHold::fn_80153530() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this->mHost, "Float");
        MR::validateShadow(this->mHost, nullptr);
        this->mRide->fn_80152F10();
    }
    MR::startActionSound(this->mHost, "Power", -1, -1, -1);
    this->mRate = MR::calcNerveEaseOutValue(this, 0, 40, 0.0f, 700.0f);
    this->fn_801534D0();
    AttackRock* pHost = this->mHost;
    f32 max = MR::getBrkFrameMax(pHost);
    MR::setBrkFrameAndStop(pHost, this->mRate * max / 700.0f);
    MR::setStageBGMStateBit(0x80);
    if (MR::isGreaterStep(this, 40)) {
        this->setNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvPower::sInstance);
    }
}

void AttackRockStateHold::fn_80153620() {
    if (MR::isFirstStep(this)) {
        MR::validateShadow(this->mHost, nullptr);
        this->mRate = 700.0f;
        this->mRide->fn_80152F10();
    }
    MR::startActionSound(this->mHost, "Power", -1, -1, -1);
    this->fn_801534D0();
    if (MR::isOnGroundPlayer() && !MR::isActorOnPlayer(this->mHost)) {
        this->mRide->fn_80152F10();
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

void AttackRockStateHold::fn_80153720() {
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

void AttackRockStateHold::fn_801537D0() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this->mHost, "FallBreak");
        this->mBreakModel->appear();
        MR::invalidateCollisionParts(this->mHost);
        MR::invalidateHitSensors(this->mHost);
        MR::zeroVelocity(this->mHost);
        this->mRide->fn_80152F20();
    }
    AttackRock* pHost = this->mHost;
    f32 base = fn_801510C0() * pHost->mScale.x;
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
        (reinterpret_cast< AttackRockStateHold* >(pSpine->mExecutor))->fn_80153530();
    }
    inline void AttackRockStateHoldNrvPower::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockStateHold* >(pSpine->mExecutor))->fn_80153620();
    }
    inline void AttackRockStateHoldNrvFloatEnd::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockStateHold* >(pSpine->mExecutor))->fn_80153720();
    }
    inline void AttackRockStateHoldNrvFallBreak::execute(Spine* pSpine) const {
        (reinterpret_cast< AttackRockStateHold* >(pSpine->mExecutor))->fn_801537D0();
    }
};  // namespace NrvAttackRockStateHold

// ---- AttackRockStateFall ----

