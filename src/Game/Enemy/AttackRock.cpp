#include "Game/Enemy/AttackRock.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

extern "C" const char lbl_806822F8[];
extern "C" const char lbl_80682318[];
extern "C" const char lbl_8068235C[];
extern "C" const char lbl_80682354[];
extern "C" const char lbl_80682388[];
extern "C" const char lbl_80682364[];
extern "C" const char lbl_8068236C[];

AttackRock::AttackRock(const char* pName)
    : LiveActor(pName), mQuatF0(0.0f, 0.0f, 0.0f, 1.0f), mQuat100(0.0f, 0.0f, 0.0f, 1.0f) {
    mBreakModel = nullptr;
    mDemoObj = nullptr;
    mProjmapEffectMtxSetter = nullptr;
    mRockNum = 1;
    mIsOnPlayer = false;
    mMtx90.identity();
    mMtxC0.identity();
    mQuat100.set(0.0f, 0.0f, 0.0f, 1.0f);
}

extern "C" u32 lbl_807D1F50;
extern "C" u32 lbl_807D1F54;
extern "C" u32 lbl_807D1F58;
extern "C" u32 lbl_807D1F5C;
extern "C" u32 lbl_807D1F60;

extern "C" void fn_801554D0(TPos3f*, AttackRock*);
extern "C" void fn_80155540(ProjmapEffectMtxSetter*, TQuat4f*, TVec3f*);
extern "C" void fn_80155360(TVec3f*, TQuat4f*, HitSensor*, HitSensor*, f32);
extern "C" void fn_800CC210(HitSensor*, HitSensor*);
extern "C" bool fn_800CC1F0(HitSensor*, HitSensor*);
extern "C" void fn_80151160(AttackRock*);

void AttackRock::makeActorAppeared() {
    LiveActor::makeActorAppeared();
}

void AttackRock::control() {
    if (MR::isOnGroundPlayer()) {
        if (MR::isActorOnPlayer(this)) {
            mIsOnPlayer = true;
        } else {
            mIsOnPlayer = false;
        }
    }
    fn_801554D0(&mMtxC0, this);
}

void AttackRock::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuatF0);
    fn_80155540(mProjmapEffectMtxSetter, &mQuat100, &mPosition);
}

void AttackRock::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pReceiver) && MR::isEqualSensor(pSender, this, lbl_80682318) && isNerve(reinterpret_cast<const Nerve*>(&lbl_807D1F5C))) {
        fn_800CC210(pReceiver, pSender);
        if (fn_800CC1F0(pReceiver, pSender)) {
            fn_80151160(this);
            return;
        }
    }
    if (isNerve(reinterpret_cast<const Nerve*>(&lbl_807D1F50))) {
        mStateFall->attackSensor(pSender, pReceiver);
    }
}

extern "C" const f32 lbl_807D9A0C;
extern "C" s32 fn_80153400(LiveActor*);

bool AttackRock::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDropFloor(msg) && isNerve(reinterpret_cast<const Nerve*>(&lbl_807D1F58)) && fn_80153400(mStateHold)) {
        fn_80155360(&mVelocity, &mQuat100, pSender, pReceiver, lbl_807D9A0C);
        setNerve(reinterpret_cast<const Nerve*>(&lbl_807D1F5C));
        return true;
    }
    return false;
}

void AttackRock::makeActorDead() {
    LiveActor::makeActorDead();
    mDemoObj->makeActorDead();
}

AttackRock::~AttackRock() {
}
