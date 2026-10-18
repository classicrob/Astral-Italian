#include "Game/Util/BellyDragon.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

extern "C" void* __nw__FUl(u32);
extern "C" void* fn_800919E0(void*, BellyDragon*, TPos3f*, bool);
extern "C" void* fn_800942E0(void*, BellyDragon*, const JMapInfoIter*);
extern "C" void* fn_80094D90(void*, BellyDragon*, s32);
extern "C" void* fn_80095530(void*, BellyDragon*, const char*, const f32*);
extern "C" void fn_800955B0(void*);
extern "C" void fn_800955C0(void*);
extern "C" const char lbl_8066C858[];
extern "C" const f32 lbl_807D76F4;
extern "C" const f32 lbl_807D76F8;
extern "C" const f32 lbl_807D76FC;
extern "C" int snprintf(char*, unsigned long, const char*, ...);
extern "C" void fn_80092170(BellyDragonBody*);
extern "C" void fn_80092500(BellyDragonBody*);
extern "C" void fn_80092230(BellyDragonBody*);
extern "C" void fn_80092540(BellyDragonBody*);
extern "C" void fn_80092670(BellyDragonBody*);
extern "C" void fn_80093810(BellyDragon*);
extern "C" void fn_80093B10(void);
extern "C" void fn_80093B20(BellyDragon*);
extern "C" void fn_800938E0(BellyDragon*, s32);
extern "C" void fn_800951D0(BellyDragonArm*, HitSensor*, HitSensor*);
extern "C" bool fn_80095260(BellyDragonArm*, u32, HitSensor*, HitSensor*);

namespace NrvBellyDragon {
    NEW_NERVE(BellyDragonNrvStart, BellyDragon, Start);
    NEW_NERVE(BellyDragonNrvFire, BellyDragon, Fire);
    NEW_NERVE(BellyDragonNrvShot, BellyDragon, Shot);
    NEW_NERVE(BellyDragonNrvAttack, BellyDragon, Attack);

    // Last state: runs the state update, then kills the actor (vtable slot 0x34 = kill).
    class BellyDragonNrvEnd : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BellyDragonNrvEnd sInstance;
    };
    BellyDragonNrvEnd BellyDragonNrvEnd::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvBellyDragon

bool fn_8008EDA0(LiveActor* pActor) {
    return pActor->isNerve(&NrvBellyDragon::BellyDragonNrvAttack::sInstance);
}

void BellyDragon::init(const JMapInfoIter& rIter) {
    mIsLv2 = MR::isObjectName(rIter, lbl_8066C858);
    MR::initDefaultPos(this, rIter);
    MR::makeMtxTR(mBaseMtx.mMtx, this);
    mFront.set(mBaseMtx.mMtx[0][2], mBaseMtx.mMtx[1][2], mBaseMtx.mMtx[2][2]);
    MR::initActor(this, rIter, false);

    mJointCtrl = new ActorJointCtrl(this);

    void* pMem = __nw__FUl(0x100);
    mBody = pMem ? (BellyDragonBody*)fn_800919E0(pMem, this, &mBaseMtx, mIsLv2) : NULL;
    pMem = __nw__FUl(0x80);
    mPod = pMem ? (BellyDragonPod*)fn_800942E0(pMem, this, &rIter) : NULL;
    initNerve(&NrvBellyDragon::BellyDragonNrvStart::sInstance, 0x10);

    pMem = __nw__FUl(0x24);
    mArmA = pMem ? (BellyDragonArm*)fn_80094D90(pMem, this, 1) : NULL;
    pMem = __nw__FUl(0x24);
    mArmB = pMem ? (BellyDragonArm*)fn_80094D90(pMem, this, 0) : NULL;

    pMem = __nw__FUl(0x1C);
    void* pStateA = pMem ? fn_80095530(pMem, this, lbl_8066C858 + 0x10, &lbl_807D76F4) : NULL;
    fn_800955B0(pStateA);
    pMem = __nw__FUl(0x1C);
    void* pStateB = pMem ? fn_80095530(pMem, this, lbl_8066C858 + 0x20, &lbl_807D76F8) : NULL;
    pMem = __nw__FUl(0x1C);
    void* pStateC = pMem ? fn_80095530(pMem, this, lbl_8066C858 + 0x2C, &lbl_807D76F8) : NULL;
    fn_800955C0(pStateC);

    MR::initActorState(this, (ActorStateBaseInterface*)pStateA, &NrvBellyDragon::BellyDragonNrvStart::sInstance, lbl_8066C858 + 0x10);
    MR::initActorState(this, (ActorStateBaseInterface*)mArmA, &NrvBellyDragon::BellyDragonNrvFire::sInstance, lbl_8066C858 + 0x3C);
    MR::initActorState(this, (ActorStateBaseInterface*)pStateB, &NrvBellyDragon::BellyDragonNrvShot::sInstance, lbl_8066C858 + 0x20);
    MR::initActorState(this, (ActorStateBaseInterface*)mArmB, &NrvBellyDragon::BellyDragonNrvAttack::sInstance, lbl_8066C858 + 0x48);
    MR::initActorState(this, (ActorStateBaseInterface*)pStateC, &NrvBellyDragon::BellyDragonNrvEnd::sInstance, lbl_8066C858 + 0x2C);
    MR::declarePowerStar(this);

    if (MR::tryRegisterDemoCast(this, rIter)) {
        mDemoA = (LiveActor*)MR::createDemoObj(lbl_8066C858 + 0x54, lbl_8066C858 + 0x68, lbl_8066C858 + 0x78, NULL);
        mDemoB = (LiveActor*)MR::createDemoObj(lbl_8066C858 + 0x88, lbl_8066C858 + 0x98, lbl_8066C858 + 0x78, NULL);
        ((s16*)mDemoB)[7] = ((s16*)this)[7];
        MR::resetPosition(mDemoA, lbl_8066C858 + 0xA0);
        MR::resetPosition(mDemoB, lbl_8066C858 + 0xA0);
        mDemoA->makeActorDead();
        mDemoB->makeActorDead();
        MtxPtr jointMtx = MR::getJointMtx(mDemoB, lbl_8066C858 + 0x108);
        TVec3f talkOffset(0.0f, lbl_807D76FC, 0.0f);
        MR::initDemoSheetTalkAnim(mDemoB, rIter, lbl_8066C858 + 0xC0, lbl_8066C858 + 0xE8, NULL, talkOffset, jointMtx);
        MR::initTalkAnimDemoCast(mDemoA, rIter, lbl_8066C858 + 0xC0, lbl_8066C858 + 0xE8);
        makeActorDead();
    } else {
        MR::useStageSwitchSyncAppear(this, rIter);
    }
}

BellyDragon::BellyDragon(const char* pName) : LiveActor(pName) {
    mIsLv2 = false;
    mFront.set(0.0f, 0.0f, 1.0f);
    mBaseMtx.identity();
    mBody = NULL;
    mPod = NULL;
    mJointCtrl = NULL;
    mArmA = NULL;
    mArmB = NULL;
    mDemoB = NULL;
    mDemoA = NULL;
}

void BellyDragon::emitFireBodyEffects() {
    char name[0x40];
    for (s32 i = 0; i < 0x20; i++) {
        snprintf(name, 0x40, "FireBody%02d", i);
        MR::emitEffect(this, name);
    }
}

void BellyDragon::appear() {
    LiveActor::appear();
    fn_80092170(mBody);
    if (mIsLv2) {
        emitFireBodyEffects();
    }
}

void BellyDragon::kill() {
    fn_80092230(mBody);
    fn_80093810(this);
    TVec3f pos(0.0f);
    if (MR::tryFindNamePos("グランドスター出現位置", &pos, NULL)) {
        MR::requestAppearPowerStar(this, pos);
    } else {
        MR::requestAppearPowerStar(this);
    }
    LiveActor::kill();
}

void BellyDragon::updateFrontVec(f32 speed) {
    f32 turn = speed;
    if (speed < 0.0f) {
        turn = 3.0f;
    }
    fn_80092540(mBody);
    TVec3f up(mBaseMtx.mMtx[0][1], mBaseMtx.mMtx[1][1], mBaseMtx.mMtx[2][1]);
    MR::normalize(&up);
    TVec3f turned = -mGravity;
    TVec3f axis(0.0f, 1.0f, 0.0f);
    TVec3f from = -mGravity;
    MR::turnVecToVecDegree(&turned, up, from, turn, axis);
    MR::makeMtxFrontUpPos(&mBaseMtx, mFront, turned, mPosition);
}

void BellyDragon::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::stopStageBGM(0x1E);
    }
    if (MR::updateActorState(this)) {
        setNerve(&NrvBellyDragon::BellyDragonNrvFire::sInstance);
    }
}

void BellyDragon::exeFire() {
    if (MR::isFirstStep(this) && mIsLv2) {
        emitFireBodyEffects();
    }
    if (MR::updateActorState(this)) {
        fn_80092500(mBody);
        setNerve(&NrvBellyDragon::BellyDragonNrvShot::sInstance);
    }
}

void BellyDragon::exeShot() {
    if (mIsLv2) {
        if (MR::isStep(this, 0x11D)) {
            fn_800938E0(this, 1);
        }
    }
    if (MR::updateActorState(this)) {
        setNerve(&NrvBellyDragon::BellyDragonNrvAttack::sInstance);
    }
}

void BellyDragon::exeAttack() {
    if (MR::isFirstStep(this) && mIsLv2) {
        emitFireBodyEffects();
    }
    if (MR::updateActorState(this)) {
        setNerve(&NrvBellyDragon::BellyDragonNrvEnd::sInstance);
    }
}

void NrvBellyDragon::BellyDragonNrvEnd::execute(Spine* pSpine) const {
    BellyDragon* actor = reinterpret_cast< BellyDragon* >(pSpine->mExecutor);
    if (MR::updateActorState(actor)) {
        actor->kill();
    }
}

void BellyDragon::control() {
    fn_80093B10();
    mJointCtrl->update();
}

void BellyDragon::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mBaseMtx);
    fn_80092670(mBody);
    mJointCtrl->setCallBackFunction();
    fn_80093B20(this);
}

void BellyDragon::attackSensor(HitSensor* pSensorA, HitSensor* pSensorB) {
    if (isNerve(&NrvBellyDragon::BellyDragonNrvFire::sInstance)) {
        fn_800951D0(mArmA, pSensorA, pSensorB);
        return;
    }
    if (isNerve(&NrvBellyDragon::BellyDragonNrvAttack::sInstance)) {
        fn_800951D0(mArmB, pSensorA, pSensorB);
    }
}

bool BellyDragon::receiveMsgPlayerAttack(u32 msg, HitSensor* pSensorA, HitSensor* pSensorB) {
    if (isNerve(&NrvBellyDragon::BellyDragonNrvFire::sInstance)) {
        return fn_80095260(mArmA, msg, pSensorA, pSensorB);
    }
    if (isNerve(&NrvBellyDragon::BellyDragonNrvAttack::sInstance)) {
        return fn_80095260(mArmB, msg, pSensorA, pSensorB);
    }
    return false;
}

MtxPtr BellyDragon::getBaseMtx() const {
    return const_cast<MtxPtr>(mBaseMtx.mMtx);
}

BellyDragon::~BellyDragon() {
}
