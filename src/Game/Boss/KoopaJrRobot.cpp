#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/MapObj/StarPieceGroup.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include <algorithm>
#include <functional.hpp>

namespace NrvKoopaJrRobot {
    // the six phases chain through updateActorStateAndNextNerve (see execute bodies)
    class KoopaJrRobotNrvIntro : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaJrRobotNrvIntro sInstance;
    };
    class KoopaJrRobotNrvPhase1 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaJrRobotNrvPhase1 sInstance;
    };
    class KoopaJrRobotNrvPhase2 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaJrRobotNrvPhase2 sInstance;
    };
    class KoopaJrRobotNrvPhase3 : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaJrRobotNrvPhase3 sInstance;
    };
    class KoopaJrRobotNrvDemo : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaJrRobotNrvDemo sInstance;
    };
    class KoopaJrRobotNrvDead : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KoopaJrRobotNrvDead sInstance;
    };

    // 16C0 -> 16C4 -> 16D0 -> 16C8 -> 16D4 -> kill; 16CC (Intro) -> 16C0
    inline void KoopaJrRobotNrvPhase1::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< LiveActor* >(pSpine->mExecutor), &KoopaJrRobotNrvPhase2::sInstance);
    }
    inline void KoopaJrRobotNrvPhase2::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< LiveActor* >(pSpine->mExecutor), &KoopaJrRobotNrvDemo::sInstance);
    }
    inline void KoopaJrRobotNrvPhase3::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< LiveActor* >(pSpine->mExecutor), &KoopaJrRobotNrvDead::sInstance);
    }
    inline void KoopaJrRobotNrvIntro::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< LiveActor* >(pSpine->mExecutor), &KoopaJrRobotNrvPhase1::sInstance);
    }
    inline void KoopaJrRobotNrvDemo::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< LiveActor* >(pSpine->mExecutor), &KoopaJrRobotNrvPhase3::sInstance);
    }
    inline void KoopaJrRobotNrvDead::execute(Spine* pSpine) const {
        LiveActor* actor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
        if (MR::updateActorState(actor)) {
            actor->kill();
        }
    }
};  // namespace NrvKoopaJrRobot

namespace NrvKoopaJrRobot { KoopaJrRobotNrvPhase1 KoopaJrRobotNrvPhase1::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrRobot { KoopaJrRobotNrvPhase2 KoopaJrRobotNrvPhase2::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrRobot { KoopaJrRobotNrvPhase3 KoopaJrRobotNrvPhase3::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrRobot { KoopaJrRobotNrvIntro KoopaJrRobotNrvIntro::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrRobot { KoopaJrRobotNrvDemo KoopaJrRobotNrvDemo::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrRobot { KoopaJrRobotNrvDead KoopaJrRobotNrvDead::sInstance ATTRIBUTE_WEAK; }

KoopaJrRobot::KoopaJrRobot(const char* pName)
    : LiveActor(pName), mStateA(nullptr), mStateB(nullptr), mStateC(nullptr), mSeq(nullptr), mCameraInfo(nullptr),
      mHeadModel(nullptr), mBody(nullptr), mZAxis(0.0f, 0.0f, 1.0f), mZAxis2(0.0f, 0.0f, 1.0f), _10C(0.0f) {
}

void KoopaJrRobot::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "KoopaJrRobot", false);
    mSeq = new KoopaJrRobotSeq(this);
    mSeq->initLaunchers();
    mSeq->killLaunchers();
    MR::declarePowerStar(this);
    MR::declareStarPiece(this, 24);
    mParts[0] = KoopaJrRobotFunction::createPart(this, 0);
    mParts[1] = KoopaJrRobotFunction::createPart(this, 1);
    mParts[2] = KoopaJrRobotFunction::createPart(this, 2);
    mParts[3] = KoopaJrRobotFunction::createPart(this, 3);
    for (s32 i = 0; i < 5; i++) {
        mFollowers[i] = KoopaJrRobotFunction::createFollower(this, i);
    }
    mHandL = KoopaJrRobotFunction::createHand(this, "HandL");
    mHandR = KoopaJrRobotFunction::createHand(this, "HandR");
    mCollisionA = KoopaJrRobotFunction::createCollision(this, rIter, 0);
    mCollisionB = KoopaJrRobotFunction::createCollision(this, rIter, 1);
    mCollisionC = KoopaJrRobotFunction::createCollision(this, rIter, 2);
    mBody = KoopaJrRobotFunction::createBody(this, MR::getJointMtx(this, "Head"));
    mHeadModel = KoopaJrRobotFunction::createHeadModel(mBody);
    initChildren(rIter);
    initNerve(&NrvKoopaJrRobot::KoopaJrRobotNrvIntro::sInstance, 6);
    mStateA = new KoopaJrRobotStateA(this);
    mStateB = new KoopaJrRobotStateB(this);
    mStateC = new KoopaJrRobotStateC(this);
    MR::initActorState(this, mStateA, &NrvKoopaJrRobot::KoopaJrRobotNrvPhase1::sInstance, nullptr);
    MR::initActorState(this, mStateB, &NrvKoopaJrRobot::KoopaJrRobotNrvPhase2::sInstance, nullptr);
    MR::initActorState(this, mStateC, &NrvKoopaJrRobot::KoopaJrRobotNrvPhase3::sInstance, nullptr);
    MR::initActorState(this, new KoopaJrRobotStateIntro(this, rIter), &NrvKoopaJrRobot::KoopaJrRobotNrvIntro::sInstance, nullptr);
    MR::initActorState(this, new KoopaJrRobotStateTU08(this), &NrvKoopaJrRobot::KoopaJrRobotNrvDemo::sInstance, nullptr);
    MR::initActorState(this, new KoopaJrRobotStateTU07(this), &NrvKoopaJrRobot::KoopaJrRobotNrvDead::sInstance, nullptr);
    mCameraInfo = MR::createActorCameraInfo(rIter);
    MR::initAnimCamera(this, mCameraInfo, KoopaJrRobotStateIntro::getCameraName());
    MR::initAnimCamera(this, mCameraInfo, KoopaJrRobotStateTU08::getCameraName());
    MR::initAnimCamera(this, mCameraInfo, KoopaJrRobotStateTU07::getCameraName());
    MR::useStageSwitchSyncAppear(this, rIter);
}

void KoopaJrRobot::appear() {
    LiveActor::appear();
    MR::invalidateClipping(this);
    if (MR::isDead(mHeadModel)) {
        mHeadModel->appear();
    }
    if (MR::isDead(mBody)) {
        mBody->appear();
    }
    MR::hideModel(this);
    MR::hideModel(mHeadModel);
    MR::hideModel(mBody);
}

// NONMATCHING: iterator/end pointer register allocation differs in the member-function loops.
void KoopaJrRobot::kill() {
    if (!MR::isDead(mBody)) {
        mBody->kill();
    }
    if (!MR::isDead(mHeadModel)) {
        mHeadModel->kill();
    }
    std::for_each(&mHandL, &mHandL + 2, std::mem_fun(&LiveActor::kill));
    std::for_each(mFollowers, mFollowers + 5, std::mem_fun(&LiveActor::kill));
    LiveActor::kill();
}

// NONMATCHING: iterator/end pointer register allocation differs.
void KoopaJrRobot::killParts() {
    MR::deleteEffectAll(this);
    std::for_each(mParts, mParts + 4, std::mem_fun(&KoopaJrRobotPart::kill));
}

bool KoopaJrRobot::isStateNerve() const {
    if (isNerve(&NrvKoopaJrRobot::KoopaJrRobotNrvIntro::sInstance)) {
        return true;
    }
    if (isNerve(&NrvKoopaJrRobot::KoopaJrRobotNrvDemo::sInstance)) {
        return true;
    }
    return isNerve(&NrvKoopaJrRobot::KoopaJrRobotNrvDead::sInstance);
}

// NONMATCHING: bool-negation lowering and iterator/end pointer register allocation differ.
void KoopaJrRobot::control() {
    mSeq->movementKillerPool();
    if (!isStateNerve()) {
        mSeq->updateNerve();
        mSeq->movementLaunchers();
    }
    std::for_each(mParts, mParts + 4, std::mem_fun(&NerveExecutor::updateNerve));
}

void KoopaJrRobot::movement() {
    LiveActor::movement();
    mBody->movement();
    calcAnim();
}

void KoopaJrRobot::calcAnim() {
    LiveActor::calcAnim();
    if (MR::isValidCalcAnim(this)) {
        mSeq->updateLauncherTransforms();
        mHandL->calcAnim();
        mHandR->calcAnim();
        mBody->calcAnim();
    }
}

void KoopaJrRobot::calcAndSetBaseMtx() {
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mZAxis);
}

void KoopaJrRobot::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrRobot::KoopaJrRobotNrvPhase1::sInstance)) {
        mStateA->attackSensor(pSender, pReceiver);
        return;
    }
    if (isNerve(&NrvKoopaJrRobot::KoopaJrRobotNrvPhase2::sInstance)) {
        mStateA->attackSensor(pSender, pReceiver);
        return;
    }
    if (isNerve(&NrvKoopaJrRobot::KoopaJrRobotNrvPhase3::sInstance)) {
        mStateA->attackSensor(pSender, pReceiver);
    }
}

bool KoopaJrRobot::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrRobot::KoopaJrRobotNrvPhase1::sInstance)) {
        return mStateA->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&NrvKoopaJrRobot::KoopaJrRobotNrvPhase2::sInstance)) {
        return mStateB->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&NrvKoopaJrRobot::KoopaJrRobotNrvPhase3::sInstance)) {
        return mStateC->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    return false;
}

bool KoopaJrRobot::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrRobot::KoopaJrRobotNrvPhase1::sInstance)) {
        return mStateA->receiveMsgEnemyAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&NrvKoopaJrRobot::KoopaJrRobotNrvPhase2::sInstance)) {
        return mStateB->receiveMsgEnemyAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&NrvKoopaJrRobot::KoopaJrRobotNrvPhase3::sInstance)) {
        return mStateC->receiveMsgEnemyAttack(msg, pSender, pReceiver);
    }
    return false;
}

// NONMATCHING: child-array/count load register allocation differs.
void KoopaJrRobot::initChildren(const JMapInfoIter& rIter) {
    s32 num = MR::getChildObjNum(rIter);
    mChildren.mArray = new NameObj*[num - 1];
    mChildren.mCapacity = num - 1;
    for (s32 i = 0; i < num; i++) {
        const char* pName = nullptr;
        MR::getChildObjName(&pName, rIter, i);
        if (MR::isEqualString(pName, "StarPieceGroup")) {
            mStarPieceGroup = static_cast< StarPieceGroup* >(NameObjFactory::initChildObj(rIter, i));
        } else {
            mChildren.mArray[mChildren.mCount++] = NameObjFactory::initChildObj(rIter, i);
        }
    }
}

KoopaJrRobotChildren::~KoopaJrRobotChildren() {
    if (mArray != nullptr) {
        delete[] mArray;
    }
}

// NONMATCHING: compiler omits the redundant null guard for the embedded child-array object.
KoopaJrRobot::~KoopaJrRobot() {}
