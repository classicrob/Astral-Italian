#include "Game/Boss/DinoPackunBaby.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

// Nerve classes are declared first so exeWait/exeBattle1st (inlined into their Nrv::execute in the target)
// can be defined before the executes while still naming the later nerves.
#define DECL_NRV(name)                                                                                                                               \
    class name : public Nerve {                                                                                                                      \
    public:                                                                                                                                          \
        virtual void execute(Spine* pSpine) const;                                                                                                   \
        static name sInstance;                                                                                                                       \
    }
#define DEF_NRV(name, exe_name)                                                                                                                      \
    inline void name::execute(Spine* pSpine) const {                                                                                                 \
        reinterpret_cast< DinoPackunBaby* >(pSpine->mExecutor)->exe##exe_name();                                                                     \
    }

namespace NrvDinoPackunBaby {
    DECL_NRV(DinoPackunBabyNrvWait);
    DECL_NRV(DinoPackunBabyNrvDemoBattleStart);
    DECL_NRV(DinoPackunBabyNrvBattle1st);
    DECL_NRV(DinoPackunBabyNrvDemoPowerUp);
    DECL_NRV(DinoPackunBabyNrvBattle2nd);
    DECL_NRV(DinoPackunBabyNrvDemoDeath);
};  // namespace NrvDinoPackunBaby

namespace {
    class DinoPackunBabyParam : public AnimScaleParam {
    public:
        DinoPackunBabyParam() {
            mSpringStrength = 0.1f;
            mDpdVibrationTime = 30.0f;
            mDpdVibrationBase = 0.60f;
            mDpdVibrationAmplitude = 0.06f;
            mDpdVibrationFrequency = 3.0f;
            mHitReactionVelocityY = 0.07f;
            mHitReactionSpringStrength = 0.12f;
            mHitReactionDamping = 0.92f;
        }
    };
};  // namespace

inline void DinoPackunBaby::exeWait() {
    if (MR::isOnSwitchA(this)) {
        MR::invalidateClipping(this);
        setNerve(&NrvDinoPackunBaby::DinoPackunBabyNrvDemoBattleStart::sInstance);
    }
}

inline void DinoPackunBaby::exeBattle1st() {
    if (MR::updateActorState(this)) {
        setNerve(&NrvDinoPackunBaby::DinoPackunBabyNrvDemoPowerUp::sInstance);
    }
}

namespace NrvDinoPackunBaby {
    DEF_NRV(DinoPackunBabyNrvWait, Wait)
    DEF_NRV(DinoPackunBabyNrvDemoBattleStart, DemoBattleStart)
    DEF_NRV(DinoPackunBabyNrvBattle1st, Battle1st)
    DEF_NRV(DinoPackunBabyNrvDemoPowerUp, DemoPowerUp)
    DEF_NRV(DinoPackunBabyNrvBattle2nd, Battle2nd)
    DEF_NRV(DinoPackunBabyNrvDemoDeath, DemoDeath)

    DinoPackunBabyNrvWait DinoPackunBabyNrvWait::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyNrvDemoBattleStart DinoPackunBabyNrvDemoBattleStart::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyNrvBattle1st DinoPackunBabyNrvBattle1st::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyNrvDemoPowerUp DinoPackunBabyNrvDemoPowerUp::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyNrvBattle2nd DinoPackunBabyNrvBattle2nd::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyNrvDemoDeath DinoPackunBabyNrvDemoDeath::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvDinoPackunBaby

static DinoPackunBabyParam sParam;

DinoPackunBaby::DinoPackunBaby(const char* pName)
    : LiveActor(pName), mFront(0.0f, 0.0f, 1.0f), mCameraInfo(nullptr), mDemoStart(nullptr), mBattle1st(nullptr), mDemoPowerUp(nullptr),
      mBattle2nd(nullptr), mDemoDeath(nullptr), mShellLeft(nullptr), mShellRight(nullptr), mShellHip(nullptr), mShellCenter(nullptr),
      mPetal(nullptr), mTail(nullptr), mEgg(nullptr), mScaleController(nullptr) {
}

void DinoPackunBaby::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "DinoPackunBaby", false);
    mCameraInfo = new ActorCameraInfo(rIter);
    MR::declareCameraRegisterVec(this, 0, &mPosition);
    initNerve(&NrvDinoPackunBaby::DinoPackunBabyNrvWait::sInstance, 8);
    mDemoStart = new DinoPackunBabyDemo(this, "DemoBattleStart");
    mDemoStart->setupDemo("DemoBattleStartDinoPackunBaby", "デモ用卵", "DinoPackunBabyShellDemo");
    mBattle1st = new DinoPackunBabyBattle(this, true);
    mDemoPowerUp = new DinoPackunBabyDemo(this, "DemoPowerUp");
    mDemoPowerUp->setRestoreShells();
    mBattle2nd = new DinoPackunBabyBattle(this, false);
    mDemoDeath = new DinoPackunBabyDemo(this, "DemoDeath");
    mDemoDeath->clearPuppetEnd();
    MR::initActorState(this, mDemoStart, &NrvDinoPackunBaby::DinoPackunBabyNrvDemoBattleStart::sInstance, "DemoBattleStart");
    MR::initActorState(this, mBattle1st, &NrvDinoPackunBaby::DinoPackunBabyNrvBattle1st::sInstance, "Battle1st");
    MR::initActorState(this, mDemoPowerUp, &NrvDinoPackunBaby::DinoPackunBabyNrvDemoPowerUp::sInstance, "DemoPowerUp");
    MR::initActorState(this, mBattle2nd, &NrvDinoPackunBaby::DinoPackunBabyNrvBattle2nd::sInstance, "Battle2nd");
    MR::initActorState(this, mDemoDeath, &NrvDinoPackunBaby::DinoPackunBabyNrvDemoDeath::sInstance, "DemoDeath");
    mShellLeft = new DinoPackunBabyShell(this, "シェル（左）", "DinoPackunBabyShellLeft", "PosShellLeft");
    mShellLeft->initWithoutIter();
    mShellRight = new DinoPackunBabyShell(this, "シェル（右）", "DinoPackunBabyShellRight", "PosShellRight");
    mShellRight->initWithoutIter();
    mShellHip = new DinoPackunBabyShell(this, "シェル（尻）", "DinoPackunBabyShellHip", "PosShellHip");
    mShellHip->initWithoutIter();
    mShellCenter = new DinoPackunBabyShell(this, "シェル（中）", "DinoPackunBabyShellCenter", "PosShellCenter");
    mShellCenter->initWithoutIter();
    mPetal = reinterpret_cast< LiveActor* >(MR::createDemoObj("花びら", "DinoPackunBabyPetal", getBaseMtx()));
    mPetal->makeActorDead();
    mTail = reinterpret_cast< LiveActor* >(MR::createDemoObj("尻尾", "DinoPackunBabyShellTail", MR::getJointMtx(this, "Shell")));
    mTail->makeActorDead();
    mEgg = reinterpret_cast< LiveActor* >(MR::createDemoObj("卵", "DinoPackunBabyEgg", getBaseMtx()));
    MR::hideModel(this);
    mScaleController = new AnimScaleController(&sParam);
    MR::declarePowerStar(this);
    makeActorAppeared();
}

void DinoPackunBaby::exeDemoBattleStart() {
    if (MR::isFirstStep(this)) {
        MR::stopStageBGM(0);
    }
    if (MR::updateActorState(this)) {
        setNerve(&NrvDinoPackunBaby::DinoPackunBabyNrvBattle1st::sInstance);
    }
}

void DinoPackunBaby::exeDemoPowerUp() {
    if (MR::isFirstStep(this)) {
        MR::forceClearFootPrint(this);
        MR::offSwitchB(this);
    }
    if (MR::updateActorState(this)) {
        setNerve(&NrvDinoPackunBaby::DinoPackunBabyNrvBattle2nd::sInstance);
    }
}

void DinoPackunBaby::exeBattle2nd() {
    if (MR::isFirstStep(this)) {
        restoreShells();
        MR::emitItemBossDamage(this);
    }
    if (MR::updateActorState(this)) {
        setNerve(&NrvDinoPackunBaby::DinoPackunBabyNrvDemoDeath::sInstance);
    }
}

void DinoPackunBaby::exeDemoDeath() {
    if (MR::isFirstStep(this)) {
        appearPetalDeath();
        MR::forceClearFootPrint(this);
    }
    if (MR::updateActorState(this)) {
        MR::requestAppearPowerStarFromBelow(this);
        MR::offSwitchB(this);
        mPetal->makeActorDead();
        kill();
    }
}

void DinoPackunBaby::control() {
    mScaleController->updateNerve();
    mScale.set(mScaleController->mScale);
}

void DinoPackunBaby::calcAndSetBaseMtx() {
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFront);
}

void DinoPackunBaby::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvDinoPackunBaby::DinoPackunBabyNrvBattle1st::sInstance)) {
        mBattle1st->attackSensor(pSender, pReceiver);
    }
    if (isNerve(&NrvDinoPackunBaby::DinoPackunBabyNrvBattle2nd::sInstance)) {
        mBattle2nd->attackSensor(pSender, pReceiver);
    }
}

bool DinoPackunBaby::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvDinoPackunBaby::DinoPackunBabyNrvBattle1st::sInstance)) {
        return mBattle1st->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    if (isNerve(&NrvDinoPackunBaby::DinoPackunBabyNrvBattle2nd::sInstance)) {
        return mBattle2nd->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    return false;
}

DinoPackunBaby::~DinoPackunBaby() {
}
