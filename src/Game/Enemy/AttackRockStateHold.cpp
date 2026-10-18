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
extern "C" void fn_800CB710(HitSensor*);
extern "C" void fn_800CB090(TVec3f*);
extern "C" void fn_800CB6D0(TVec3f*);
extern "C" f32 fn_800CB410(const TVec3f*);
extern "C" void fn_800CAF80(TVec3f*, const TVec3f*);
extern "C" void fn_800CAFE0(TVec3f*, const TVec3f*);
extern "C" void fn_800CB460(TVec3f*);
extern "C" void fn_800CC210(HitSensor*, HitSensor*);
extern "C" bool fn_800CC1F0(HitSensor*, HitSensor*);
extern "C" void fn_800CC220(LiveActor*, HitSensor*);
extern "C" bool fn_800CC230(LiveActor*, HitSensor*);
extern "C" bool fn_800CC270(u32);
extern "C" bool fn_800CC280(u32);
extern "C" bool fn_800CC290(u32);
extern "C" bool fn_800CC2B0(u32);
extern "C" void fn_800E4420(void*);
extern "C" void fn_800E4700(const TVec3f*);
extern "C" void fn_800513C0(TQuat4f*, TVec3f*);
extern "C" TVec3f* fn_800514B0(TQuat4f*, TVec3f*);

// Unit-local helpers (unnamed in the target).
extern "C" {
f32 fn_801510C0() NO_INLINE;
bool fn_801510D0(LiveActor*) NO_INLINE;
void fn_80151160(AttackRock*) NO_INLINE;
bool fn_801517C0(LiveActor*, s32) NO_INLINE;
void fn_80151850(void*) NO_INLINE;
void fn_801552A0(LiveActor*) NO_INLINE;
void fn_80155360(TVec3f*, TQuat4f*, HitSensor*, HitSensor*, f32) NO_INLINE;
void fn_801553D0(TVec3f*, TVec3f*, HitSensor*, HitSensor*, f32) NO_INLINE;
void fn_801554D0(TPos3f*, LiveActor*) NO_INLINE;
void fn_80155540(ProjmapEffectMtxSetter*, TQuat4f*, TVec3f*) NO_INLINE;
ProjmapEffectMtxSetter* fn_80155660(LiveActor*) NO_INLINE;
void fn_80152F10(AttackRockRide*) NO_INLINE;
void fn_80152F20(AttackRockRide*) NO_INLINE;
bool fn_80152E20(AttackRockRide*, u32, HitSensor*, HitSensor*) NO_INLINE;
void fn_80152A00(AttackRockRide*) NO_INLINE;
void fn_80152C00(AttackRockRide*) NO_INLINE;
void fn_801529E0(AttackRockRide*, const TVec3f*) NO_INLINE;
AttackRockRide* fn_801528E0(AttackRockRide*, LiveActor*) NO_INLINE;
bool fn_80153400(AttackRockStateHold*) NO_INLINE;
bool fn_80153460(AttackRockStateHold*) NO_INLINE;
f32 fn_801552E0(LiveActor*, TQuat4f*) NO_INLINE;
}

extern "C" AttackRockRide* fn_801528E0(AttackRockRide* pRide, LiveActor* pHost) {
    pRide->mHost = pHost;
    pRide->mQuat0.set(0.0f, 0.0f, 0.0f, 1.0f);
    pRide->mQuat1.set(0.0f, 0.0f, 0.0f, 1.0f);
    pRide->mQuat2.set(0.0f, 0.0f, 0.0f, 1.0f);
    pRide->_34.set(0.0f, 0.0f, 1.0f);
    pRide->_40.set(0.0f, 0.0f, 0.0f);
    pRide->_4C.set(0.0f, 0.0f, 0.0f);
    pRide->_58.set(0.0f, 0.0f, 0.0f);
    pRide->_64.set(0.0f, 0.0f, 1.0f);
    pRide->_70.set(0.0f, 0.0f, 1.0f);
    pRide->_7C.set(0.0f, 0.0f, 1.0f);
    pRide->_88 = 0.0f;
    pRide->_8C = 0.0f;
    pRide->_90 = 0.0f;
    pRide->_94 = 0;
    pRide->_98 = 0;
    pRide->mIsRush = false;
    pRide->mIsEnable = false;
    return pRide;
}

// NONMATCHING: TVec3f operator= is expanded inline (psq copy) in the target.
extern "C" void fn_801529E0(AttackRockRide* pRide, const TVec3f* pVec) {
    pRide->_34 = *pVec;
}

// NONMATCHING: vector math written from the m2c output; operand order / temporaries not verified.
extern "C" void fn_80152A00(AttackRockRide* pRide) {
    TVec3f last = *MR::getPlayerLastMove();
    TVec3f tmp(last);
    tmp.scale(0.9f);
    pRide->_4C.set(tmp);
    pRide->_58.set(pRide->_4C);
    MR::calcDirToPlayer(&pRide->_64, pRide->mHost);
    TVec3f playerMove = *MR::getPlayerLastMove();
    TVec3f sum(pRide->_40);
    sum.add(playerMove);
    TVec3f dir = sum - pRide->mHost->mPosition;
    MR::normalizeOrZero(&dir);
    pRide->_88 = MR::diffAngleAbs(pRide->_64, dir);
    pRide->_8C = 0.0f;
    TVec3f cross;
    PSVECCrossProduct(&pRide->_64, &dir, &pRide->_7C);
    if (MR::normalizeOrZero(&pRide->_7C)) {
        MR::makeAxisVerticalZX(&pRide->_7C, pRide->_64);
    }
    pRide->_98 = 0;
    pRide->mQuat1.set(pRide->mQuat0);
    TVec3f rotated;
    MR::rotateVecRadian(&rotated, pRide->_64, pRide->_7C, 10.0f * pRide->_88);
    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::turnVecToVecDegree(&pRide->_70, pRide->_34, rotated, 45.0f, up);
    TVec3f side = pRide->_70 - pRide->_70;
    if (MR::isNearZero(side, 0.001f)) {
        MR::makeAxisVerticalZX(&side, pRide->_70);
    }
    MR::makeQuatUpFront(&pRide->mQuat2, pRide->_70, side);
    f32 angle = 57.29578f * MR::diffAngleAbs(pRide->_64, pRide->_70);
    s32 frames = (s32)(angle / 3.0f);
    pRide->_94 = frames + 15;
    pRide->_90 = PSVECDistance(&pRide->_40, &pRide->mHost->mPosition);
    MR::startBckPlayer("Fall", "SpinDriverFall");
}

// NONMATCHING: vector math written from the m2c output; operand order / temporaries not verified.
extern "C" void fn_80152C00(AttackRockRide* pRide) {
    if (pRide->mIsRush) {
        f32 a = pRide->_88;
        f32 sum = pRide->_8C + a;
        pRide->_8C = sum;
        f32 t = (f32)pRide->_98 / (f32)pRide->_94;
        pRide->_88 = a * 0.95f;
        f32 invT = 1.0f - t;
        TVec3f rotated;
        MR::rotateVecRadian(&rotated, pRide->_64, pRide->_7C, sum * invT);
        TVec3f blend;
        MR::vecBlendSphere(&blend, pRide->_64, pRide->_70, t);
        MR::normalizeOrZero(&blend);
        f32 len = MR::lerp(pRide->_90, 300.0f, t);
        TVec3f pos(blend);
        pos.x *= len;
        pos.y *= len;
        pos.z *= len;
        pRide->_40.set(pos);
        pRide->_40.add(pRide->mHost->mPosition);
        TVec3f drift(pRide->_58);
        drift.scale(invT);
        pRide->_40.add(drift);
        pRide->mQuat0.set(pRide->mQuat1);
        pRide->mQuat0.slerp(pRide->mQuat2, t);
        pRide->_4C.x *= 0.9f;
        pRide->_98 += 1;
        pRide->_4C.y *= 0.9f;
        pRide->_4C.z *= 0.9f;
        pRide->_58.add(pRide->_4C);
        if (pRide->_98 > pRide->_94) {
            TVec3f jump(pRide->_70);
            jump.scale(-20.0f);
            MR::endBindAndPlayerJump(pRide->mHost, jump, 0);
            pRide->mIsRush = false;
            pRide->mIsEnable = false;
        }
    }
}

extern "C" bool fn_80152E20(AttackRockRide* pRide, u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgAutoRushBegin(msg) && !MR::isOnGroundPlayer() && pRide->mIsEnable) {
        fn_80152A00(pRide);
        pRide->mIsRush = true;
        pRide->mIsEnable = false;
        return true;
    }
    if (MR::isMsgUpdateBaseMtx(msg) && pRide->mIsRush) {
        fn_800513C0(&pRide->mQuat0, &pRide->_40);
        return true;
    }
    if (MR::isMsgRushCancel(msg)) {
        pRide->mIsRush = false;
        return true;
    }
    if (MR::isMsgFloorTouch(msg)) {
        pRide->mIsEnable = false;
    }
    return false;
}

extern "C" void fn_80152F10(AttackRockRide* pRide) {
    pRide->mIsEnable = true;
}

extern "C" void fn_80152F20(AttackRockRide* pRide) {
    if (pRide->mIsRush) {
        MR::endBindAndPlayerWait(pRide->mHost);
        pRide->mIsRush = false;
    }
    pRide->mIsEnable = false;
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

extern "C" {
void fn_80153530(AttackRockStateHold*) NO_INLINE;
void fn_80153620(AttackRockStateHold*) NO_INLINE;
void fn_80153720(AttackRockStateHold*) NO_INLINE;
void fn_801537D0(AttackRockStateHold*) NO_INLINE;
void fn_801534D0(AttackRockStateHold*) NO_INLINE;
}

AttackRockStateHold::AttackRockStateHold(AttackRock* pRock, s32 rockNum)
    : ActorStateBase< AttackRock >("\215U\214\202\212\342\225\202\202\251\202\361\202\305\202\242\202\351\217\363\221\324", pRock), mRide(nullptr), mBreakModel(nullptr), mStartPos(0.0f, 0.0f, 0.0f), mUpVec(0.0f, 1.0f, 0.0f),
      mRate(0.0f), mRockNum(rockNum) {
    initNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFloat::sInstance, 0);
    mRide = new AttackRockRide;
    fn_801528E0(mRide, pRock);
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
    fn_800CB090(&dir);
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
    fn_80152F20(mRide);
}

void AttackRockStateHold::control() {
    TVec3f dir;
    fn_800CB460(&dir);
    TVec3f toPlayer = dir - mHost->mPosition;
    MR::normalizeOrZero(&dir, toPlayer);
    MR::turnQuatZDirRate(&mHost->mQuat100, mHost->mQuat100, dir, 0.3f);
    TVec3f zdir;
    mHost->mQuat100.getZDir(zdir);
    fn_801529E0(mRide, &zdir);
    fn_80152C00(mRide);
}

bool AttackRockStateHold::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (fn_800CC280(msg)) {
        mBreakModel->makeActorDead();
        kill();
        return true;
    }
    if (fn_800CC290(msg) && !isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFallBreak::sInstance)) {
        setNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFallBreak::sInstance);
        return true;
    }
    if (fn_80152E20(mRide, msg, pSender, pReceiver)) {
        return true;
    }
    return false;
}

extern "C" bool fn_80153400(AttackRockStateHold* pState) {
    if (pState->isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvPower::sInstance) ||
        pState->isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFloatEnd::sInstance)) {
        return true;
    }
    return false;
}

extern "C" bool fn_80153460(AttackRockStateHold* pState) {
    if (pState->isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFloat::sInstance) ||
        pState->isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvPower::sInstance) ||
        pState->isNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFloatEnd::sInstance)) {
        return true;
    }
    return false;
}

// NONMATCHING: operator*/operator+ out-of-line vs inline decisions differ (target was a separate TU).
extern "C" void fn_801534D0(AttackRockStateHold* pState) {
    AttackRock* pHost = pState->mHost;
    TVec3f offset = pState->mUpVec * pState->mRate;
    TVec3f pos(pState->mStartPos + offset);
    MR::setPosition(pHost, pos);
}

extern "C" void fn_80153530(AttackRockStateHold* pState) {
    if (MR::isFirstStep(pState)) {
        MR::startAction(pState->mHost, "Float");
        MR::validateShadow(pState->mHost, nullptr);
        fn_80152F10(pState->mRide);
    }
    MR::startActionSound(pState->mHost, "Power", -1, -1, -1);
    pState->mRate = MR::calcNerveEaseOutValue(pState, 0, 40, 0.0f, 700.0f);
    fn_801534D0(pState);
    AttackRock* pHost = pState->mHost;
    f32 max = MR::getBrkFrameMax(pHost);
    MR::setBrkFrameAndStop(pHost, pState->mRate * max / 700.0f);
    MR::setStageBGMStateBit(0x80);
    if (MR::isGreaterStep(pState, 40)) {
        pState->setNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvPower::sInstance);
    }
}

extern "C" void fn_80153620(AttackRockStateHold* pState) {
    if (MR::isFirstStep(pState)) {
        MR::validateShadow(pState->mHost, nullptr);
        pState->mRate = 700.0f;
        fn_80152F10(pState->mRide);
    }
    MR::startActionSound(pState->mHost, "Power", -1, -1, -1);
    fn_801534D0(pState);
    if (MR::isOnGroundPlayer() && !MR::isActorOnPlayer(pState->mHost)) {
        fn_80152F10(pState->mRide);
    }
    MR::setStageBGMStateBit(0x80);
    s32 step;
    switch (pState->mRockNum) {
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
    if (MR::isGreaterStep(pState, step)) {
        pState->setNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFloatEnd::sInstance);
    }
}

extern "C" void fn_80153720(AttackRockStateHold* pState) {
    if (MR::isFirstStep(pState)) {
        MR::startAction(pState->mHost, "FloatEnd");
    }
    AttackRock* pHost = pState->mHost;
    f32 rate = MR::calcNerveEaseInOutValue(pState, 30, 1.0f, 0.0f);
    f32 max = MR::getBrkFrameMax(pHost);
    MR::setBrkFrameAndStop(pHost, max * rate);
    if (MR::isGreaterStep(pState, 30)) {
        pState->setNerve(&NrvAttackRockStateHold::AttackRockStateHoldNrvFallBreak::sInstance);
    }
}

extern "C" void fn_801537D0(AttackRockStateHold* pState) {
    if (MR::isFirstStep(pState)) {
        MR::startAction(pState->mHost, "FallBreak");
        pState->mBreakModel->appear();
        MR::invalidateCollisionParts(pState->mHost);
        MR::invalidateHitSensors(pState->mHost);
        MR::zeroVelocity(pState->mHost);
        fn_80152F20(pState->mRide);
    }
    AttackRock* pHost = pState->mHost;
    f32 base = fn_801510C0() * pHost->mScale.x;
    MR::setShadowVolumeSphereRadius(pState->mHost, nullptr, MR::calcNerveValue(pState, 30, 0.95f * base, 0.05f * base));
    if (MR::isStep(pState, 1)) {
        MR::hideModelAndOnCalcAnim(pState->mHost);
    }
    if (MR::isGreaterStep(pState, 30)) {
        MR::invalidateShadow(pState->mHost, nullptr);
        pState->kill();
    }
}

namespace NrvAttackRockStateHold {
    inline void AttackRockStateHoldNrvFloat::execute(Spine* pSpine) const {
        fn_80153530(reinterpret_cast< AttackRockStateHold* >(pSpine->mExecutor));
    }
    inline void AttackRockStateHoldNrvPower::execute(Spine* pSpine) const {
        fn_80153620(reinterpret_cast< AttackRockStateHold* >(pSpine->mExecutor));
    }
    inline void AttackRockStateHoldNrvFloatEnd::execute(Spine* pSpine) const {
        fn_80153720(reinterpret_cast< AttackRockStateHold* >(pSpine->mExecutor));
    }
    inline void AttackRockStateHoldNrvFallBreak::execute(Spine* pSpine) const {
        fn_801537D0(reinterpret_cast< AttackRockStateHold* >(pSpine->mExecutor));
    }
};  // namespace NrvAttackRockStateHold

// ---- AttackRockStateFall ----

