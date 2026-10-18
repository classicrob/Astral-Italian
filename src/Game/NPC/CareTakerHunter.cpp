#include "Game/NPC/CareTakerHunter.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"

extern u8 lbl_806B8010[];
extern const char lbl_806B8170[];
extern const char lbl_806B8288[];
extern const char lbl_806B8098[];
extern const char lbl_806B8210[];
extern s32 lbl_807D4868;
extern s32 lbl_807D486C;
extern s32 lbl_807D4874;
extern s32 lbl_807D4878;
extern s32 lbl_807D487C;
extern s32 lbl_807D4880;
extern s32 lbl_807D4884;
extern s32 lbl_807D4888;
extern s32 lbl_807D488C;
extern s32 lbl_807D4890;
extern s32 lbl_807D4894;
extern s32 lbl_807D4898;
extern s32 lbl_807D489C;
extern s32 lbl_807D48A0;
extern s32 lbl_807D48A4;
extern s32 lbl_807D48A8;

// Raw string/pointer slots inside lbl_806B8010 (the unit's rodata table).
#define LBL(off) (reinterpret_cast< const char* >(lbl_806B8010 + (off)))
#define NRV(sym) (reinterpret_cast< Nerve* >(&(sym)))
#define RAW(T, this_, off) (*reinterpret_cast< T* >(reinterpret_cast< u8* >(this_) + (off)))

extern "C" void fn_8001AF50(s32, s32);
extern "C" void fn_80051140();
extern "C" void** fn_80344770(void*);
extern "C" f32 fn_80344620(void*, TVec3f*, void*);

CareTakerHunter::CareTakerHunter(const char* pName) : NPCActor(pName) {
    mTakeOutStar = NULL;
    mObjArg0 = -1;
    mObjArg4 = 0;
    mBodyColorFrame = 0;
    mCameraInfo0 = NULL;
    mCameraInfo1 = NULL;
    mCameraInfo2 = NULL;
    mCameraInfo3 = NULL;
    mBringEnemy = NULL;
    mChildObjs = NULL;
    mChildCount = 0;
    mActorGroup = NULL;
    mRotAngle = 0.0f;
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    mTimer = 0.0f;
}

void CareTakerHunter::init(const JMapInfoIter& rIter) {
    s32 arg0 = -1;
    MR::getJMapInfoArg0NoInit(rIter, &arg0);
    mObjArg0 = arg0;

    NPCActorCaps caps(LBL(0x18));
    caps.setDefault();
    initialize(rIter, caps, LBL(0x18), NULL, nullptr);

    s32 arg3 = 0;
    MR::getJMapInfoArg3NoInit(rIter, &arg3);
    mBodyColorFrame = arg3;

    MR::startBrk(this, LBL(0x28));
    MR::setBrkFrameAndStop(this, (f32)mBodyColorFrame);

    RAW(const char*, this, 0x138) = LBL(0x34);
    RAW(const char*, this, 0x134) = LBL(0x40);
    RAW(const char*, this, 0x140) = LBL(0x40);
    RAW(const char*, this, 0x13C) = LBL(0x4C);
    RAW(const char*, this, 0x100) = LBL(0x58);
    RAW(const char*, this, 0x104) = LBL(0x58);
    RAW(u8, this, 0xEC) = 0;
    RAW(u8, this, 0xED) = 1;
    RAW(f32, this, 0xF4) = 3.0f;
    RAW(f32, this, 0x110) = 2.0f;
    RAW(f32, this, 0x114) = 0.1f;
    RAW(f32, this, 0x118) = 0.05f;
    RAW(u8, this, 0x128) = 1;

    MR::startBckNoInterpole(this, LBL(0x58));
    MR::calcAnimDirect(this);

    s32 arg4 = 0;
    MR::getJMapInfoArg4NoInit(rIter, &arg4);
    mObjArg4 = arg4;
    setTalkStrings(mObjArg4);

    if (getMsgCtrl() != NULL) {
        MR::registerBranchFunc(getMsgCtrl(), TalkMessageFunc(this, &CareTakerHunter::branchFunc));
        MR::registerEventFunc(getMsgCtrl(), TalkMessageFunc(this, &CareTakerHunter::eventFunc));
    }

    if (mObjArg0 == 0) {
        if (!MR::isScenarioOpenInCurrentStageWithDeclarer(LBL(0x64))) {
            makeActorDead();
        }
        MR::initMultiActorCamera(this, rIter, &mCameraInfo0, LBL(0x7C));
        MR::initMultiActorCamera(this, rIter, &mCameraInfo1, LBL(0x88));
        MR::initMultiActorCamera(this, rIter, &mCameraInfo2, LBL(0x94));
        MR::initMultiActorCamera(this, rIter, &mCameraInfo3, LBL(0xA8));
        MR::declarePowerStar(this);

        TakeOutStar* pTakeOut = new TakeOutStar(this, LBL(0xB4), LBL(0xB4), NRV(lbl_807D4868));  // NONMATCHING: target calls the 5-arg ctor (extra s32 0)
        mTakeOutStar = pTakeOut;

        mChildCount = MR::getChildObjNum(rIter) - 1;
        mChildObjs = new LiveActor*[mChildCount];

        s32 numChild = MR::getChildObjNum(rIter);
        s32 idx = 0;
        for (s32 i = 0; i < numChild; i++) {
            const char* pName = NULL;
            MR::getChildObjName(&pName, rIter, i);
            if (MR::isEqualString(pName, LBL(0xD0))) {
                NameObj* pObj = NameObjFactory::initChildObj(rIter, i);
                mBringEnemy = reinterpret_cast< BringEnemy* >(pObj);
                MR::setPosition(reinterpret_cast< LiveActor* >(pObj), mPosition);
                mBringEnemy->makeActorDead();
            } else {
                mChildObjs[idx] = reinterpret_cast< LiveActor* >(NameObjFactory::initChildObj(rIter, i));
                mChildObjs[idx]->makeActorDead();
                idx++;
            }
        }

        switch (mBringEnemy->getEnemyType()) {
        case 1: {
            mActorGroup = new LiveActorGroup(LBL(0xDC), 1);
            LiveActor* pModel = reinterpret_cast< LiveActor* >(MR::createModelObjEnemy(LBL(0xEC), LBL(0xFC), NULL));
            MR::invalidateClipping(pModel);
            mActorGroup->registerActor(pModel);
            break;
        }
        case 2: {
            mActorGroup = new LiveActorGroup(LBL(0xDC), 2);
            LiveActor* pModel1 = reinterpret_cast< LiveActor* >(MR::createModelObjEnemy(LBL(0x104), LBL(0x118), NULL));
            LiveActor* pModel2 = reinterpret_cast< LiveActor* >(MR::createModelObjEnemy(LBL(0x128), LBL(0x138), MR::getJointMtx(pModel1, LBL(0x14C))));
            mActorGroup->registerActor(pModel1);
            mActorGroup->registerActor(pModel2);
            MR::initLightCtrl(pModel1);
            MR::initLightCtrl(pModel2);
            MR::invalidateClipping(pModel1);
            MR::invalidateClipping(pModel2);
            break;
        }
        }

        MR::setEffectHostMtx(this, LBL(0x154), mActorGroup->getActor(0)->getBaseMtx());

        for (s32 i = 0; i < mActorGroup->getObjNum(); i++) {
            LiveActor* pActor = mActorGroup->getActor(i);
            MR::registerDemoSimpleCastAll(pActor);
            MR::setScale(mActorGroup->getActor(i), mScale);
            mActorGroup->getActor(i)->makeActorDead();
        }

        setNerve(NRV(lbl_807D4874));
    }
}

void CareTakerHunter::control() {
    MR::startActionSound(this, lbl_806B8170, -1, -1, -1);
    NPCActor::control();
}

bool CareTakerHunter::branchFunc(u32 msg) {
    if (msg == 0) {
        if (fn_80344770(mBringEnemy) != NULL) {
            if (isNerve(NRV(lbl_807D4880))) {
                setNerve(NRV(lbl_807D4884));
            }
            return true;
        }
        return false;
    }
    return false;
}

bool CareTakerHunter::eventFunc(u32 msg) {
    if (mObjArg0 == -1) {
        if (msg == 0xA) {
            MR::addPlayerLeft(1);
            return true;
        }
        return true;
    }
    if (msg == 0) {
        return mTakeOutStar->takeOut();
    }
    if (msg == 1) {
        if (isNerve(NRV(lbl_807D4874))) {
            setNerve(NRV(lbl_807D4878));
            return false;
        }
        return isNerve(NRV(lbl_807D487C));
    } else if (msg == 2) {
        return isNerve(NRV(lbl_807D4898));
    }
    return true;
}

bool CareTakerHunter::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        RAW(u8, this, 0xE7) = 1;
        return false;
    }
    return NPCActor::receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

// NONMATCHING: rotation/matrix math is a best-effort reconstruction (fmod operands unclear).
LiveActor* CareTakerHunter::updateRotation() {
    if (mActorGroup != NULL) {
        LiveActor* pActor = mActorGroup->getActor(0);
        mRotAngle += mTimer;
        mRotAngle = fmod(360.0f + mRotAngle, 360.0f);
        Mtx mtx;
        PSMTXCopy(pActor->getBaseMtx(), mtx);
        MR::addTransMtxLocal(mtx, TVec3f(0.0f));
        MR::rotateMtxLocalYDegree(mtx, mRotAngle);
        TVec3f dir;
        MR::extractMtxZDir(mtx, &dir);
        f32 x = dir.x;
        (void)x;
        return pActor;
    }
    return NULL;
}

void CareTakerHunter::setTalkStrings(s32 arg) {
    if (arg == 0) {
        RAW(const char*, this, 0x108) = LBL(0x174 + 0x10);
        RAW(const char*, this, 0x10C) = LBL(0x174 + 0x10);
    }
}

void CareTakerHunter::exeReaction() {
    if (RAW(u8, this, 0xDC) != 0) {
        MR::startActionSound(this, LBL(0x180), -1, -1, -1);
        MR::startActionSound(this, LBL(0x194), -1, -1, -1);
    }
    if (RAW(u8, this, 0xDD) != 0) {
        MR::startActionSound(this, LBL(0x1A8), -1, -1, -1);
        MR::startActionSound(this, LBL(0x1BC), -1, -1, -1);
    }
    if (isPointingSe()) {
        MR::startActionSound(this, LBL(0x1D0), -1, -1, -1);
        MR::startDPDHitSound();
    }
    if (RAW(u8, this, 0xDF) != 0) {
        MR::startActionSound(this, LBL(0x1A8), -1, -1, -1);
        MR::startActionSound(this, LBL(0x1E8), -1, -1, -1);
    }
    MR::tryStartReactionAndPopNerve(this);
}

void CareTakerHunter::exeTalk() {
    if (!MR::tryStartReactionAndPushNerve(this, NRV(lbl_807D486C))) {
        if (MR::isShortTalk(getMsgCtrl())) {
            RAW(const char*, this, 0x108) = LBL(0x4C);
        } else {
            RAW(const char*, this, 0x108) = LBL(0x174 + 0x10);
        }
        if (MR::tryTalkNearPlayerAndStartMoveTalkAction(this)) {
            RAW(const char*, this, 0x108) = LBL(0x174 + 0x10);
        }
    }
}

void CareTakerHunter::exeTalkBegin() {
    if (MR::isFirstStep(this)) {
        mBringEnemy->appear();
        for (s32 i = 0; i < mChildCount; i++) {
            if (mChildObjs[i] != NULL) {
                mChildObjs[i]->appear();
            }
        }
        MR::startMultiActorCameraTargetSelf(this, mCameraInfo0, LBL(0x7C), -1);
    }
    if (MR::isGreaterStep(this, 120)) {
        setNerve(NRV(lbl_807D487C));
    }
}

void CareTakerHunter::exeTalkEnd() {
    if (MR::tryTalkForceWithoutDemoAtEnd(getMsgCtrl())) {
        MR::endMultiActorCamera(this, mCameraInfo0, LBL(0x7C), false, -1);
        setNerve(NRV(lbl_807D4880));
    }
}

// NONMATCHING: facing/quaternion math is a best-effort reconstruction.
void CareTakerHunter::exeWait() {
    if (MR::isFirstStep(this)) {
        RAW(f32, this, 0xF4) = 0.0f;
    }
    TVec3f dir(0.0f);
    f32 dist = fn_80344620(mBringEnemy, &dir, this);
    if (!MR::isNearZero(dist, 0.001f)) {
        if (dist < 1500.0f) {
            RAW(f32, this, 0xF4) = 0.0f;
            TVec3f up(0.0f, 0.0f, 1.0f);
            (void)up;
            if (!MR::isOppositeDirection(-dir, dir, 0.01f)) {
                MR::faceToVector(&RAW(TQuat4f, this, 0xA0), dir, 4.0f);
            }
        } else {
            RAW(f32, this, 0xF4) = 4.0f;
        }
    } else {
        RAW(f32, this, 0xF4) = 4.0f;
    }
    if (MR::tryTalkNearPlayerAtEndAndStartTalkAction(this)) {
        RAW(f32, this, 0xF4) = 4.0f;
    }
    if (MR::tryStartReactionAndPushNerve(this, NRV(lbl_807D486C))) {
        RAW(f32, this, 0xF4) = 4.0f;
    }
}

void CareTakerHunter::exeGrab() {
    if (MR::tryStartDemoMarioPuppetable(this, lbl_806B8210)) {  // NONMATCHING: carry-sensor kill path simplified
        if (MR::isPlayerCarryAny()) {
            MR::tryPlayerKillTakingActor();
        }
        setNerve(NRV(lbl_807D4888));
    }
}

void CareTakerHunter::exeGrabWait() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, lbl_806B8170, -1, -1, -1);
    }
    fn_8001AF50(0, 0);
    if (MR::isGreaterStep(this, 90)) {
        setNerve(NRV(lbl_807D488C));
    }
}

void CareTakerHunter::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::endDemo(this, lbl_806B8098);
    }
    fn_8001AF50(0, 0);
    if (MR::tryStartDemoMarioPuppetable(this, lbl_806B8098)) {
        setNerve(NRV(lbl_807D4890));
    }
}

void CareTakerHunter::exeHideWait() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeFade(-1);
        fn_80051140();
    }
    if (!MR::isWipeActive()) {
        fn_80051140();
        setNerve(NRV(lbl_807D4894));
    }
}

void CareTakerHunter::exeAppear() {
    if (MR::isFirstStep(this)) {
        fn_80344770(mBringEnemy);
        MR::openWipeFade(-1);
        MR::startSubBGM(LBL(0x220), false);
        MR::startMultiActorCameraTargetSelf(this, mCameraInfo1, LBL(0x88), -1);
        MR::setPlayerPosOnGroundAndWait(LBL(0x234));
        MR::startBckPlayer(LBL(0x250));
        MR::startAction(this, LBL(0x258));
        mRotAngle = 0.0f;
    }
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    for (s32 i = 0; i < mActorGroup->getObjNum(); i++) {
        MR::setScale(mActorGroup->getActor(i), mScale);
    }
    if (MR::isGreaterStep(this, 120)) {
        mTimer = 4.0f;
    }
    updateRotation();
    if (!MR::isWipeActive() && MR::isActionEnd(this)) {
        setNerve(NRV(lbl_807D4898));
    }
}

void CareTakerHunter::exeAppeared() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, lbl_806B8288);
    }
    fn_8001AF50(2, 0);
    if (MR::tryTalkForceWithoutDemoAtEnd(getMsgCtrl())) {
        setNerve(NRV(lbl_807D489C));
    }
}

void CareTakerHunter::exeWipeIn() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeFade(-1);
    }
    fn_8001AF50(2, 0);
    if (!MR::isWipeActive()) {
        setNerve(NRV(lbl_807D48A0));
    }
}

void CareTakerHunter::exeLeave() {
    if (MR::isFirstStep(this)) {
        MR::openWipeFade(-1);
        MR::startBckPlayer(LBL(0x250));
        MR::endMultiActorCamera(this, mCameraInfo1, LBL(0x88), false, -1);
        mActorGroup->makeAllDead();
        MR::startMultiActorCameraTargetSelf(this, mCameraInfo2, LBL(0x94), -1);
        MR::startAction(this, LBL(0x174));
        MR::forceDeleteEffect(this, LBL(0x154));
    }
    fn_8001AF50(2, 0);
    if (!MR::isWipeActive()) {
        setNerve(NRV(lbl_807D48A4));
    }
}

void CareTakerHunter::exeEnd() {
    if (MR::tryTalkForceWithoutDemoAtEnd(getMsgCtrl())) {
        MR::endDemo(this, lbl_806B8098);
        setNerve(NRV(lbl_807D48A8));
    }
}
