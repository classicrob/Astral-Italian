#include "Game/LiveActor/ActorActionKeeper.hpp"
#include "Game/LiveActor/ActionBgmCtrl.hpp"
#include "Game/LiveActor/ActionFlagCtrl.hpp"
#include "Game/LiveActor/ActionFootPrintCtrl.hpp"
#include "Game/LiveActor/ActionScreenBlurCtrl.hpp"
#include "Game/LiveActor/ActorAnimKeeper.hpp"
#include "Game/LiveActor/ActorHitReactionCtrl.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/LiveActor/ActorPadAndCameraCtrl.hpp"
#include "Game/LiveActor/ActorParam.hpp"
#include "Game/LiveActor/LiveActor.hpp"

ActorActionKeeper* ActorActionKeeper::tryCreate(LiveActor* pActor, const char* pName) {
    ActorAnimKeeper* pAnimKeeper = ActorAnimKeeper::tryCreate(pActor);
    ActionFlagCtrl* pFlagCtrl = ActionFlagCtrl::tryCreate(pActor);
    ActorPadAndCameraCtrl* pPadAndCameraCtrl = ActorPadAndCameraCtrl::tryCreate(pActor->mModelManager, &pActor->mPosition);
    ActorHitReactionCtrl* pHitReactionCtrl = ActorHitReactionCtrl::tryCreate(pActor);
    ActorItemGenerator* pItemGenerator = ActorItemGenerator::tryCreate(pActor, pName);
    ActorParam* pParam = ActorParam::tryCreate(pActor);
    ActionScreenBlurCtrl* pScreenBlurCtrl = ActionScreenBlurCtrl::tryCreate(pActor->mModelManager);
    ActionFootPrintCtrl* pFootPrintCtrl = ActionFootPrintCtrl::tryCreate(pActor);
    ActionBgmCtrl* pBgmCtrl = ActionBgmCtrl::tryCreate(pActor->mModelManager);

    if (pAnimKeeper == NULL && pFlagCtrl == NULL && pPadAndCameraCtrl == NULL && pHitReactionCtrl == NULL && pItemGenerator == NULL &&
        pParam == NULL && pScreenBlurCtrl == NULL && pFootPrintCtrl == NULL && pBgmCtrl == NULL) {
        return NULL;
    }

    return new ActorActionKeeper(pActor, pAnimKeeper, pFlagCtrl, pPadAndCameraCtrl, pHitReactionCtrl, pItemGenerator, pParam, pScreenBlurCtrl,
                                 pFootPrintCtrl, pBgmCtrl);
}

void ActorActionKeeper::initFlagCtrl() {
    if (mFlagCtrl != NULL) {
        mFlagCtrl->init();
    }
}

bool ActorActionKeeper::startAction(const char* pName) {
    if (mFlagCtrl != NULL) {
        mFlagCtrl->startAction(pName);
    }

    if (mAnimKeeper != NULL) {
        return mAnimKeeper->start(pName);
    }

    return false;
}

void ActorActionKeeper::updateAnimKeeper() {
    if (mAnimKeeper != NULL) {
        mAnimKeeper->update();
    }
}

void ActorActionKeeper::update() {
    if (mFlagCtrl != NULL) {
        mFlagCtrl->update();
    }

    if (mPadAndCameraCtrl != NULL) {
        mPadAndCameraCtrl->update();
    }

    if (mScreenBlurCtrl != NULL) {
        mScreenBlurCtrl->update();
    }

    if (mFootPrintCtrl != NULL) {
        mFootPrintCtrl->update();
    }

    if (mBgmCtrl != NULL) {
        mBgmCtrl->update();
    }
}

ActorActionKeeper::ActorActionKeeper(LiveActor* pActor, ActorAnimKeeper* pAnimKeeper, ActionFlagCtrl* pFlagCtrl,
                                     ActorPadAndCameraCtrl* pPadAndCameraCtrl, ActorHitReactionCtrl* pHitReactionCtrl,
                                     ActorItemGenerator* pItemGenerator, ActorParam* pParam, ActionScreenBlurCtrl* pScreenBlurCtrl,
                                     ActionFootPrintCtrl* pFootPrintCtrl, ActionBgmCtrl* pBgmCtrl)
    : mActor(pActor), mAnimKeeper(pAnimKeeper), mFlagCtrl(pFlagCtrl), mPadAndCameraCtrl(pPadAndCameraCtrl), mHitReactionCtrl(pHitReactionCtrl),
      mItemGenerator(pItemGenerator), mActorParam(pParam), mScreenBlurCtrl(pScreenBlurCtrl), mFootPrintCtrl(pFootPrintCtrl), mBgmCtrl(pBgmCtrl) {}
