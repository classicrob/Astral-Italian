#include "Game/Boss/DinoPackunBaby.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

#define DECL_NRV(name)                                                                                                                               \
    class name : public Nerve {                                                                                                                      \
    public:                                                                                                                                          \
        virtual void execute(Spine* pSpine) const;                                                                                                   \
        static name sInstance;                                                                                                                       \
    }
#define DEF_NRV(name, exe_name)                                                                                                                      \
    inline void name::execute(Spine* pSpine) const {                                                                                                 \
        reinterpret_cast< DinoPackunBabyDemo* >(pSpine->mExecutor)->exe##exe_name();                                                                 \
    }

namespace NrvDinoPackunBabyDemo {
    DECL_NRV(DinoPackunBabyDemoNrvWait);
    DECL_NRV(DinoPackunBabyDemoNrvDemo);
};  // namespace NrvDinoPackunBabyDemo

namespace NrvDinoPackunBabyDemo {
    DEF_NRV(DinoPackunBabyDemoNrvWait, Wait)
    DEF_NRV(DinoPackunBabyDemoNrvDemo, Demo)

    DinoPackunBabyDemoNrvWait DinoPackunBabyDemoNrvWait::sInstance ATTRIBUTE_WEAK;
    DinoPackunBabyDemoNrvDemo DinoPackunBabyDemoNrvDemo::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvDinoPackunBabyDemo

DinoPackunBabyDemo::DinoPackunBabyDemo(DinoPackunBaby* pHost, const char* pName)
    : ActorStateBase< DinoPackunBaby >(pName, pHost), mDemoName(pName), mPlayerBck(nullptr), mModel(nullptr), mIsRestoreShells(false),
      mIsPuppetEnd(true) {
}

void DinoPackunBabyDemo::setupDemo(const char* pPlayerBck, const char* pName, const char* pModelName) {
    mPlayerBck = pPlayerBck;
    mModel = MR::createModelObjMapObjDecoration(pName, pModelName, nullptr);
    mModel->initWithoutIter();
    mModel->makeActorDead();
    MR::invalidateClipping(mModel);
}

void DinoPackunBabyDemo::setRestoreShells() {
    mIsRestoreShells = true;
}

void DinoPackunBabyDemo::clearPuppetEnd() {
    mIsPuppetEnd = false;
}

void DinoPackunBabyDemo::init() {
    initNerve(&NrvDinoPackunBabyDemo::DinoPackunBabyDemoNrvWait::sInstance, 0);
    mHost->initDemoCamera(mDemoName);
    kill();
}

void DinoPackunBabyDemo::appear() {
    mIsDead = false;
    setNerve(&NrvDinoPackunBabyDemo::DinoPackunBabyDemoNrvWait::sInstance);
}

void DinoPackunBabyDemo::exeWait() {
    if (mHost->tryStartDemoWithCamera(mDemoName, mPlayerBck, 0)) {
        if (mModel != nullptr) {
            MR::showModel(mHost);
            MR::resetPosition(mModel, "ベビーディノパックンデモ位置");
            mModel->makeActorAppeared();
            MR::requestMovementOn(mModel);
        }
        mHost->requestMovementOnParts();
        MR::overlayWithPreviousScreen(2);
        setNerve(&NrvDinoPackunBabyDemo::DinoPackunBabyDemoNrvDemo::sInstance);
    }
}

void DinoPackunBabyDemo::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, mDemoName);
        if (mPlayerBck != nullptr) {
            MR::startBckPlayer(mPlayerBck, nullptr);
        }
        if (mModel != nullptr) {
            MR::startAction(mModel, mDemoName);
            mHost->startActionShells(mDemoName);
            mHost->mEgg->makeActorDead();
        }
        if (mIsRestoreShells) {
            mHost->startBreakShells(mDemoName);
        }
    }
    if (mIsRestoreShells) {
        mHost->updateShellsMtx();
        if (MR::isStep(this, 40)) {
            mHost->restoreShells();
        }
    }
    if (mHost->tryEndDemoWithCamera(mDemoName, mIsPuppetEnd)) {
        if (mPlayerBck != nullptr) {
            MR::setPlayerPos("プレイヤーデモ位置（ベビーディノパックン戦）");
        }
        if (mModel != nullptr) {
            mModel->makeActorDead();
            mHost->startActionShells("Wait");
        }
        kill();
    }
}

DinoPackunBabyDemo::~DinoPackunBabyDemo() {
}

DinoPackunBabyShell::DinoPackunBabyShell(LiveActor* pHost, const char* pName, const char* pModelName, const char* pJointName)
    : PartsModel(pHost, pName, pModelName, nullptr, -1, false), mModel(nullptr) {
    mModelMtx.identity();
    loadFixedPosition(pJointName, nullptr);
}

inline PartsModel::~PartsModel() {}

// NONMATCHING: string-address lifetime and register allocation differ.
void DinoPackunBabyShell::init(const JMapInfoIter& rIter) {
    PartsModel::init(rIter);
    mModelMtx.set(getBaseMtx());
    mModel = new BreakModel(this, "シェル壊れ", "DinoPackunBabyShellBreak", mModelMtx, "Break");
    mModel->initWithoutIter();
}

void DinoPackunBabyShell::kill() {
    LiveActor::kill();
    MR::calcGravity(this);
    MR::makeMtxUpNoSupportPos(&mModelMtx, -mGravity, mPosition);
    mModel->mAnimName = "Break";
    mModel->appear();
}

void DinoPackunBabyShell::startBreak(const char* pActionName) {
    mModelMtx.set(mHost->getBaseMtx());
    mModel->mAnimName = pActionName;
    mModel->appear();
}

void DinoPackunBabyShell::updateModelMtx() {
    mModelMtx.set(mHost->getBaseMtx());
}
void DinoPackunBabyShell::requestMovementOnModel() {
    MR::requestMovementOn(mModel);
}
void DinoPackunBabyShell::killModel() {
    MR::tryKillAndForceDeleteEffectAll(mModel);
}
DinoPackunBabyShell::~DinoPackunBabyShell() {}

