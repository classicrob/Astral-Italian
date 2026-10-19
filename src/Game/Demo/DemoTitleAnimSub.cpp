#include "Game/Demo/DemoTitleAnim.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace {
    class DemoTitleAnimModelNrvAction : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast< DemoTitleAnimModel* >(pSpine->mExecutor)->exeAction();
        }
        static DemoTitleAnimModelNrvAction sInstance;
    };
#ifdef GALAXY_NATIVE
    DemoTitleAnimModelNrvAction DemoTitleAnimModelNrvAction::sInstance;
#else
    DemoTitleAnimModelNrvAction DemoTitleAnimModelNrvAction::sInstance ATTRIBUTE_WEAK;
#endif
};  // namespace

DemoTitleAnimParam::DemoTitleAnimParam() : mArcName(nullptr), mJointName(nullptr), mActionName(nullptr) {
}

DemoTitleAnimModel::DemoTitleAnimModel(DemoTitleAnim* pParent, DemoTitleAnimParam* pParam)
    : LiveActor(pParam->mArcName), mParent(pParent), mParam(pParam), mBaseMtx(nullptr) {
}

void DemoTitleAnimModel::init(const JMapInfoIter&) {
    MR::initActor(this, mParam->mArcName, "DemoTitle", false);
    if (mParam->mJointName != nullptr) {
        mBaseMtx = MR::getJointMtx(mParent, mParam->mJointName);
    } else {
        mBaseMtx = mParent->getBaseMtx();
    }
    MR::extractMtxTrans(mBaseMtx, &mPosition);
    initNerve(GET_NERVE_ANON(DemoTitleAnimModelNrvAction), 0);
    MR::invalidateClipping(this);
    makeActorDead();
}

void DemoTitleAnimModel::calcAndSetBaseMtx() {
    if (mBaseMtx != nullptr) {
        MR::extractMtxTrans(mBaseMtx, &mPosition);
        MR::setBaseTRMtx(this, mBaseMtx);
    } else {
        LiveActor::calcAndSetBaseMtx();
    }
}

void DemoTitleAnimModel::startSub() {
    if (mBaseMtx != nullptr) {
        MR::extractMtxTrans(mBaseMtx, &mPosition);
    }
    setNerve(GET_NERVE_ANON(DemoTitleAnimModelNrvAction));
    appear();
}

void DemoTitleAnimModel::exeAction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, mParam->mActionName);
    }
}

void DemoTitleAnimModel::loopSub() {
    if (mParam->mLoopActionName != nullptr) {
        MR::startAction(this, mParam->mLoopActionName);
    }
}

void DemoTitleAnimModel::selectSub() {
    if (mParam->mSelectingActionName != nullptr) {
        MR::startAction(this, mParam->mSelectingActionName);
    }
}

void DemoTitleAnimModel::endSub() {
    kill();
}

DemoTitleAnimPlayer::DemoTitleAnimPlayer(DemoTitleAnim* pParent, DemoTitleAnimParam* pParam) : DemoTitleAnimModel(pParent, pParam) {
}

void DemoTitleAnimPlayer::init(const JMapInfoIter&) {
    if (mParam->mJointName != nullptr) {
        mBaseMtx = MR::getJointMtx(mParent, mParam->mJointName);
    } else {
        mBaseMtx = mParent->getBaseMtx();
    }
    MR::extractMtxTrans(mBaseMtx, &mPosition);
    MR::connectToSceneNpcMovement(this);
    MR::invalidateClipping(this);
    makeActorDead();
}

void DemoTitleAnimPlayer::control() {
    MR::setPlayerBaseMtx(mBaseMtx);
}

void DemoTitleAnimPlayer::startSub() {
    MR::showPlayer();
    MR::startBckPlayer(mParam->mActionName, nullptr);
    appear();
}

void DemoTitleAnimPlayer::loopSub() {
    if (mParam->mLoopActionName != nullptr) {
        MR::startBckPlayer(mParam->mLoopActionName, nullptr);
    }
}

void DemoTitleAnimPlayer::selectSub() {
    if (mParam->mSelectingActionName != nullptr) {
        MR::startBckPlayer(mParam->mSelectingActionName, nullptr);
    }
}

void DemoTitleAnimPlayer::endSub() {
    MR::startBckPlayer("BattleWait", nullptr);
    MR::hidePlayer();
    kill();
}
