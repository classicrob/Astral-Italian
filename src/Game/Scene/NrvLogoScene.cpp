#include "Game/Scene/NrvLogoScene.hpp"
#include "Game/Scene/LogoScene.hpp"
#include "Game/Screen/SimpleLayout.hpp"
#include "Game/Scene/ModelWipeScene.hpp"
#include <JSystem/J3DGraphBase/J3DDrawBuffer.hpp>
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSystemFunction.hpp"
#include "Game/System/NerveExecutor.hpp"
#include "Game/Util/NerveUtil.hpp"

namespace NrvLogoScene {
    LogoSceneNrvStrapFadein::LogoSceneNrvStrapFadein() {
    }

    void LogoSceneNrvStrapFadein::execute(Spine* pSpine) const {
        LogoScene* actor = reinterpret_cast< LogoScene* >(pSpine->mExecutor);
        actor->exeStrapFadein();
    }

    LogoSceneNrvStrapDisplay::LogoSceneNrvStrapDisplay() {
    }

    void LogoSceneNrvStrapDisplay::execute(Spine* pSpine) const {
        LogoScene* actor = reinterpret_cast< LogoScene* >(pSpine->mExecutor);
        actor->exeStrapDisplay();
    }

    LogoSceneNrvStrapFadeout::LogoSceneNrvStrapFadeout() {
    }

    void LogoSceneNrvStrapFadeout::execute(Spine* pSpine) const {
        LogoScene* actor = reinterpret_cast< LogoScene* >(pSpine->mExecutor);
        if (actor->tryFadeoutLayout(actor->mStrapLayout)) {
            actor->setNerve(&LogoSceneNrvWaitReadDoneSystemArchive::sInstance);
        }
    }

    LogoSceneNrvMountGameData::LogoSceneNrvMountGameData() {
    }

    void LogoSceneNrvMountGameData::execute(Spine* pSpine) const {
        LogoScene* actor = reinterpret_cast< LogoScene* >(pSpine->mExecutor);
        actor->exeMountGameData();
    }

    LogoSceneNrvWaitReadDoneSystemArchive::LogoSceneNrvWaitReadDoneSystemArchive() {
    }

    void LogoSceneNrvWaitReadDoneSystemArchive::execute(Spine* pSpine) const {
        NerveExecutor* pExecutor = reinterpret_cast< NerveExecutor* >(pSpine->mExecutor);
        if (GameSystemFunction::isDoneLoadSystemArchive()) {
            pExecutor->setNerve(&LogoSceneNrvMountGameData::sInstance);
        }
    }

    LogoSceneNrvDeactive::LogoSceneNrvDeactive() {
    }

    void LogoSceneNrvDeactive::execute(Spine* pSpine) const {
        if (MR::isFirstStep(reinterpret_cast< const NerveExecutor* >(pSpine->mExecutor))) {
            GameSequenceFunction::notifyToGameSequenceProgressToEndScene();
        }
    }
};  // namespace NrvLogoScene

J3DDrawBuffer* ModelWipeScene::createDrawBuffer() {
    J3DDrawBuffer* pBuffer = new J3DDrawBuffer(0x20);
    pBuffer->mSortMode = 5;
    return pBuffer;
}
