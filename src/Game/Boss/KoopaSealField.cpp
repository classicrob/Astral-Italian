#include "Game/Boss/KoopaSealField.hpp"
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Enemy/AttackRock.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/GravityLightRoad.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

// PLACEHOLDER(objects): extern "C" block of unnamed/odd target symbols (relocs must match); resolve when they get real names in symbols.txt
extern "C" {
void fn_800346F0(LiveActor*, const char*, const TPos3f*);
extern const char* lbl_807CF5F8;
}

static const char* const sSealFixPosNames[] = {
    "SealFixPosMulti0", "SealFixPosMulti1", "SealFixPosMulti2", "SealFixPosMulti3", "SealFixPosMulti4",
};

namespace NrvKoopaSealField {
    NEW_NERVE(KoopaSealFieldNrvWait, KoopaSealField, Wait);
    NEW_NERVE(KoopaSealFieldNrvOpen, KoopaSealField, Open);
    NEW_NERVE(KoopaSealFieldNrvOpenEnd, KoopaSealField, OpenEnd);
    NEW_NERVE(KoopaSealFieldNrvDemoCamera, KoopaSealField, DemoCamera);

    class KoopaSealFieldNrvDemoPuppet : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static KoopaSealFieldNrvDemoPuppet sInstance;
    };
    KoopaSealFieldNrvDemoPuppet KoopaSealFieldNrvDemoPuppet::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(KoopaSealFieldNrvDemoIn, KoopaSealField, DemoIn);
    NEW_NERVE(KoopaSealFieldNrvDemoEnd, KoopaSealField, DemoEnd);
};  // namespace NrvKoopaSealField

KoopaSealField::KoopaSealField(const char* pName) : LiveActor(pName) {
    mGravityLightRoad = nullptr;
    mAttackRockTutorial = nullptr;
    mSealGroup = nullptr;
    mCameraInfo = nullptr;
    mAlreadyDoneFlag = -1;
    mIsSealField = true;
}

void KoopaSealField::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);

    s32 childNum = MR::getChildObjNum(rIter);
    for (s32 i = 0; i < childNum; i++) {
        const char* pName = nullptr;
        MR::getChildObjName(&pName, rIter, i);
        if (MR::isEqualString(pName, "GravityLightRoad")) {
            if (mGravityLightRoad == nullptr) {
                mGravityLightRoad = new GravityLightRoad("\x8F\x64\x97\xCD\x83\x89\x83\x43\x83\x67\x83\x8D\x81\x5B\x83\x68");
                MR::initChildObj(mGravityLightRoad, rIter, i);
                mGravityLightRoad->makeActorDead();
            }
        } else if (MR::isEqualString(pName, "AttackRockTutorial")) {
            if (mAttackRockTutorial == nullptr) {
                mAttackRockTutorial = new AttackRockTutorial("\x8D\x55\x8C\x82\x8A\xE2\x83\x60\x83\x85\x81\x5B\x83\x67\x83\x8A\x83\x41\x83\x8B");
                MR::initChildObj(mAttackRockTutorial, rIter, i);
            }
        }
    }

    f32 radius = 0.0f;
    MR::calcModelBoundingRadius(&radius, this);
    MR::setClippingTypeSphere(this, radius);

    mIsSealField = MR::isObjectName(rIter, "KoopaSealField");
    if (mIsSealField) {
        mSealGroup = new DeriveActorGroup< KoopaJrRobotSeal >("\x8F\xE9\x96\xE5\x95\x95\x88\xF3\x83\x8A\x83\x58\x83\x67", 3);
        for (s32 i = 0; i < 3; i++) {
            KoopaJrRobotSeal* pSeal = new KoopaJrRobotSeal("\x8F\xE9\x96\xE5\x95\x95\x88\xF3");
            pSeal->initWithoutIter();
            pSeal->setup(this, sSealFixPosNames[i], 1);
            pSeal->setClipping(&mPosition, radius);
            mSealGroup->registerActor(pSeal);
        }
    } else {
        mSealGroup = new DeriveActorGroup< KoopaJrRobotSeal >("\x8F\xE9\x96\xE5\x95\x95\x88\xF3\x83\x8A\x83\x58\x83\x67", 1);
        KoopaJrRobotSeal* pSeal = new KoopaJrRobotSeal("\x8F\xE9\x96\xE5\x95\x95\x88\xF3");
        pSeal->initWithoutIter();
        pSeal->setup(this, lbl_807CF5F8, 3);
        pSeal->setClipping(&mPosition, radius);
        mSealGroup->registerActor(pSeal);
    }

    MR::initMultiActorCamera(this, rIter, &mCameraInfo, "\x94\xE0\x83\x43\x83\x93");
    initNerve(GET_NERVE(KoopaSealField, KoopaSealFieldNrvWait), 0);
    makeActorAppeared();

    s32 sealNum = mSealGroup->getObjNum();
    for (s32 i = 0; i < sealNum; i++) {
        mSealGroup->getMember(i)->makeChainsAppeared();
    }

    if (mIsSealField) {
        if (MR::setupAlreadyDoneFlag(&mAlreadyDoneFlag, "\x83\x4E\x83\x62\x83\x70\x8F\xE9\x96\xE5\x8A\x4A\x95\xFA\x8D\xCF\x82\xDD", rIter, 1)) {
            setNerve(GET_NERVE(KoopaSealField, KoopaSealFieldNrvOpenEnd));
            if (mGravityLightRoad != nullptr) {
                mGravityLightRoad->appear();
            }
            if (mAttackRockTutorial != nullptr) {
                mAttackRockTutorial->startDead();
            }
            breakAllSeals();
        }
    }
}

void KoopaSealField::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    s32 sealNum = mSealGroup->getObjNum();
    for (s32 i = 0; i < sealNum; i++) {
        mSealGroup->getActor(i)->makeActorAppeared();
    }
}

void KoopaSealField::makeActorDead() {
    LiveActor::makeActorDead();
    s32 sealNum = mSealGroup->getObjNum();
    for (s32 i = 0; i < sealNum; i++) {
        mSealGroup->getActor(i)->makeActorDead();
    }
}

void KoopaSealField::control() {}

bool KoopaSealField::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return MR::isMsgAttackRockReflect(msg);
}

bool KoopaSealField::tryStartPuppet() {
    if (mSealGroup->getObjNum() > 1 && MR::isNearPlayerVerticalGravity(this, 500.0f)) {
        TVec3f front;
        MR::calcFrontVec(&front, this);
        TPos3f mtx;
        MR::makeMtxUpNoSupportPos(&mtx, front, *MR::getPlayerPos());
        fn_800346F0(this, "\x83\x7D\x83\x8A\x83\x49\x83\x43\x83\x93", &mtx);
        setNerve(GET_NERVE(KoopaSealField, KoopaSealFieldNrvDemoPuppet));
        return true;
    }
    return false;
}

void KoopaSealField::breakAllSeals() {
    s32 sealNum = mSealGroup->getObjNum();
    for (s32 i = 0; i < sealNum; i++) {
        mSealGroup->getMember(i)->breakImmediately();
    }
}

void KoopaSealField::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    s32 sealNum = mSealGroup->getObjNum();
    bool isAllBroken = true;
    for (s32 i = 0; i < sealNum; i++) {
        if (!mSealGroup->getMember(i)->isBroken()) {
            isAllBroken = false;
            break;
        }
    }

    if (isAllBroken) {
        setNerve(GET_NERVE(KoopaSealField, KoopaSealFieldNrvOpen));
    }
}

void KoopaSealField::exeOpen() {
    if (MR::isFirstStep(this)) {
        if (mIsSealField) {
            MR::updateAlreadyDoneFlag(mAlreadyDoneFlag);
        }
        if (mAttackRockTutorial != nullptr) {
            mAttackRockTutorial->startDead();
        }
        MR::startAction(this, "Open");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(KoopaSealField, KoopaSealFieldNrvOpenEnd));
    }
}

void KoopaSealField::exeOpenEnd() {
    if (MR::isFirstStep(this)) {
        if (MR::isValidSwitchB(this)) {
            MR::onSwitchB(this);
        }
        MR::startAction(this, "OpenEnd");
        if (mGravityLightRoad != nullptr) {
            mGravityLightRoad->appear();
        }
        MR::calcFrontVec(&mGravity, this);
        MR::invalidateCollisionParts(this);
    }

    if (mGravityLightRoad != nullptr && mGravityLightRoad->isPlayerFall()) {
        setNerve(GET_NERVE(KoopaSealField, KoopaSealFieldNrvDemoCamera));
    }
}

void KoopaSealField::exeDemoCamera() {
    if (MR::isFirstStep(this)) {
        if (mCameraInfo != nullptr) {
            MR::startMultiActorCameraTargetSelf(this, mCameraInfo, "\x94\xE0\x83\x43\x83\x93", -1);
        }
    }
    tryStartPuppet();
}

void KoopaSealField::exeDemoIn() {
    if (MR::isFirstStep(this)) {
        mGravityLightRoad->endPlayerFall();
        MR::requestMovementOn(mGravityLightRoad);
        MR::startAction(this, "DemoIn");
        MR::hidePlayer();
    }

    if (MR::isGreaterStep(this, 180)) {
        setNerve(GET_NERVE(KoopaSealField, KoopaSealFieldNrvDemoEnd));
    }
}

void KoopaSealField::exeDemoEnd() {
    if (MR::isFirstStep(this)) {
        MR::overlayWithPreviousScreen(4);
        if (mCameraInfo != nullptr) {
            MR::endMultiActorCamera(this, mCameraInfo, "\x94\xE0\x83\x43\x83\x93", false, -1);
        }
        MR::endDemo(this, "\x94\xE0\x83\x43\x83\x93");
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
    }
}

// Demo puppet nerve: the target execute is the body itself (fn_800E6770), so it is written inline here.
void NrvKoopaSealField::KoopaSealFieldNrvDemoPuppet::execute(Spine* pSpine) const {
    KoopaSealField* pActor = reinterpret_cast< KoopaSealField* >(pSpine->mExecutor);
    if (MR::tryStartDemoMarioPuppetableWithoutCinemaFrame(pActor, "\x94\xE0\x83\x43\x83\x93")) {
        pActor->setNerve(GET_NERVE(KoopaSealField, KoopaSealFieldNrvDemoIn));
    }
}

KoopaSealField::~KoopaSealField() {}
