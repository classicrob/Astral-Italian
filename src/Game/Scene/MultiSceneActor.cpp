#include "Game/Scene/MultiSceneActor.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Scene/MultiSceneEffectKeeper.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/System/ResourceInfo.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include <JSystem/J3DGraphAnimator/J3DModel.hpp>

MultiSceneActor::MultiSceneActor(const char* pName, const char* pModelName, const f32 (*pProjection)[4], bool param4)
    : NameObj(pName), mTranslation(0.0f, 0.0f, 0.0f), mRotation(0.0f, 0.0f, 0.0f), mScale(1.0f, 1.0f, 1.0f), _30(true), mModelManager(nullptr),
      mEffectSystem(nullptr), mEffectKeeper(nullptr), mSpine(nullptr) {
    mModelManager = new ModelManager(pProjection);
    mModelManager->init(pModelName, nullptr, param4);
}

void MultiSceneActor::init(const JMapInfoIter& rIter) {
}

void MultiSceneActor::movement() {
    if (_30) {
        return;
    }

    mModelManager->update();

    if (mSpine != nullptr) {
        mSpine->update();
    }

    if (_30) {
        return;
    }

    control();
}

void MultiSceneActor::calcAnim() {
    if (_30) {
        return;
    }

    TMtx34f mtx;
    mtx.identity();

    MR::makeMtxTR(mtx, mTranslation, mRotation);
    PSMTXCopy(mtx, mModelManager->getJ3DModel()->mBaseTransformMtx);

    J3DModel* pModel = mModelManager->getJ3DModel();
    pModel->mBaseScale.x = mScale.x;
    pModel->mBaseScale.y = mScale.y;
    pModel->mBaseScale.z = mScale.z;

    mModelManager->calcAnim();
}

void MultiSceneActor::calcViewAndEntry() {
    if (_30) {
        return;
    }

    mModelManager->calcView();
    mModelManager->entry();
}

void MultiSceneActor::appear() {
    _30 = false;

    calcAnim();
}

void MultiSceneActor::kill() {
    if (mEffectKeeper != nullptr) {
        mEffectKeeper->clear();
    }

    _30 = true;
}

ModelManager* MultiSceneActor::getModelManager() const {
    return mModelManager;
}

EffectSystem* MultiSceneActor::getEffectSystem() const {
    return mEffectSystem;
}

MultiSceneEffectKeeper* MultiSceneActor::getEffectKeeper() const {
    return mEffectKeeper;
}

void MultiSceneActor::initEffect(EffectSystem* pEffectSystem, int param2, const char* pName) {
    mEffectSystem = pEffectSystem;

    mEffectKeeper = new MultiSceneEffectKeeper(mName, mModelManager, param2, pName);
    mEffectKeeper->init(this, pEffectSystem);
}

void MultiSceneActor::setNerve(const Nerve* pNerve) {
    mSpine->setNerve(pNerve);
}

bool MultiSceneActor::isNerve(const Nerve* pNerve) const {
    return mSpine->getCurrentNerve() == pNerve;
}

void MultiSceneActor::initNerve(const Nerve* pNerve) {
    mSpine = new Spine(this, pNerve);
}

s32 MultiSceneActor::getNerveStep() const {
    return mSpine->mStep;
}

namespace MultiScene {
    ResourceHolder* getResourceHolder(MultiSceneActor* pActor) {
        ModelManager* pModelManager = pActor->getModelManager();
        if (pModelManager != nullptr) {
            return pModelManager->getResourceHolder();
        }

        return nullptr;
    }

    void startBck(MultiSceneActor* pActor, const char* pBckName) {
        pActor->getModelManager()->startBck(pBckName, nullptr);
    }

    void startBtk(MultiSceneActor* pActor, const char* pBtkName) {
        pActor->getModelManager()->startBtk(pBtkName);
    }

    void startBrk(MultiSceneActor* pActor, const char* pBrkName) {
        pActor->getModelManager()->startBrk(pBrkName);
    }

    void startBtp(MultiSceneActor* pActor, const char* pBtpName) {
        pActor->getModelManager()->startBtp(pBtpName);
    }

    void startBva(MultiSceneActor* pActor, const char* pBvaName) {
        pActor->getModelManager()->startBva(pBvaName);
    }

    void setBckFrameAndStop(MultiSceneActor* pActor, f32 frame) {
        // NOTE: the target body drives the BTK frame control (see report: name/body mismatch)
        pActor->getModelManager()->getBtkCtrl()->setFrame(frame);
        pActor->getModelManager()->getBtkCtrl()->setRate(0.0f);
    }

    void setBrkFrameAndStop(MultiSceneActor* pActor, f32 frame) {
        pActor->getModelManager()->getBrkCtrl()->setFrame(frame);
        pActor->getModelManager()->getBrkCtrl()->setRate(0.0f);
    }

    bool startBckIfExist(MultiSceneActor* pActor, const char* pBckName) {
        if (pActor->getModelManager() != nullptr && pActor->getModelManager()->mXanimePlayer != nullptr && pActor->getModelManager()->mXanimePlayer->isExistAnimation(pBckName)) {
            pActor->getModelManager()->startBck(pBckName, nullptr);
            return true;
        }

        return false;
    }

    bool startBtkIfExist(MultiSceneActor* pActor, const char* pBtkName) {
        if (getResourceHolder(pActor)->mBtkResTable->isExistRes(pBtkName)) {
            pActor->getModelManager()->startBtk(pBtkName);
            return true;
        }

        return false;
    }

    bool startBrkIfExist(MultiSceneActor* pActor, const char* pBrkName) {
        if (getResourceHolder(pActor)->mBrkResTable->isExistRes(pBrkName)) {
            pActor->getModelManager()->startBrk(pBrkName);
            return true;
        }

        return false;
    }

    bool startBtpIfExist(MultiSceneActor* pActor, const char* pBtpName) {
        if (getResourceHolder(pActor)->mBtpResTable->isExistRes(pBtpName)) {
            pActor->getModelManager()->startBtp(pBtpName);
            return true;
        }

        return false;
    }

    bool startBvaIfExist(MultiSceneActor* pActor, const char* pBvaName) {
        if (getResourceHolder(pActor)->mBvaResTable->isExistRes(pBvaName)) {
            pActor->getModelManager()->startBva(pBvaName);
            return true;
        }

        return false;
    }

    void setBtpFrameAndStop(MultiSceneActor* pActor, f32 frame) {
        pActor->getModelManager()->getBtpCtrl()->setFrame(frame);
        pActor->getModelManager()->getBtpCtrl()->setRate(0.0f);
    }

    void setBvaFrameAndStop(MultiSceneActor* pActor, f32 frame) {
        pActor->getModelManager()->getBvaCtrl()->setFrame(frame);
        pActor->getModelManager()->getBvaCtrl()->setRate(0.0f);
    }

    bool isBckStopped(MultiSceneActor* pActor) {
        return pActor->getModelManager()->isBckStopped();
    }

    bool isBckEnd(const MultiSceneActor* pActor) {
        return (pActor->getModelManager()->getBckCtrl()->getState() & 0x2) != 0;
    }

    bool isStep(const MultiSceneActor* pActor, s32 step) {
        return pActor->getNerveStep() == step;
    }

    bool isFirstStep(const MultiSceneActor* pActor) {
        return pActor->getNerveStep() == 0;
    }

    bool isGreaterStep(const MultiSceneActor* pActor, s32 step) {
        return pActor->getNerveStep() > step;
    }

    bool isLessStep(const MultiSceneActor* pActor, s32 step) {
        return pActor->getNerveStep() < step;
    }

    bool isLessEqualStep(const MultiSceneActor* pActor, s32 step) {
        return pActor->getNerveStep() <= step;
    }

    void setNerveAtStep(MultiSceneActor* pActor, const Nerve* pNerve, s32 step) {
        if (isStep(pActor, step)) {
            pActor->setNerve(pNerve);
        }
    }

    void setNerveAtBckStopped(MultiSceneActor* pActor, const Nerve* pNerve) {
        if (isBckStopped(pActor)) {
            pActor->setNerve(pNerve);
        }
    }

    f32 calcNerveRate(const MultiSceneActor* pActor, s32 stepMax) {
        return stepMax <= 0 ? 1.0f : MR::clamp(static_cast< f32 >(pActor->getNerveStep()) / stepMax, 0.0f, 1.0f);
    }

    f32 calcNerveRate(const MultiSceneActor* pActor, s32 stepMin, s32 stepMax) {
        return MR::clamp(MR::normalize(pActor->getNerveStep(), stepMin, stepMax), 0.0f, 1.0f);
    }

    f32 calcNerveEaseOutRate(const MultiSceneActor* pActor, s32 stepMax) {
        return MR::getEaseOutValue(calcNerveRate(pActor, stepMax), 0.0f, 1.0f, 1.0f);
    }

    f32 calcNerveValue(const MultiSceneActor* pActor, s32 stepMax, f32 valueStart, f32 valueEnd) {
        return MR::getLinerValue(calcNerveRate(pActor, stepMax), valueStart, valueEnd, 1.0f);
    }

    void emitEffect(const MultiSceneActor* pActor, const char* pParam2) {
        pActor->getEffectKeeper()->create(pParam2, pActor->getEffectSystem());
    }

    void deleteEffectAll(const MultiSceneActor* pActor) {
        pActor->getEffectKeeper()->deleteAll();
    }

    void forceDeleteEffectAll(const MultiSceneActor* pActor) {
        pActor->getEffectKeeper()->forceDeleteAll(pActor->getEffectSystem());
    }

    MtxPtr getJointMtx(const MultiSceneActor* pActor, const char* pParam2) {
        return MR::getJointMtx(pActor->getModelManager()->getJ3DModel(), (const char*)pParam2);
    }
};  // namespace MultiScene

void MultiSceneActor::control() {
}
