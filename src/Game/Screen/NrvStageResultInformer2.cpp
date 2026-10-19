#include "Game/Screen/StageResultInformer.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/NerveUtil.hpp"

namespace NrvStageResultInformer {
    void StageResultInformerNrvDisappearBestTime::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeDisappearBestTime();
    }

    void StageResultInformerNrvDisplayBestTime::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeDisplayBestTime();
    }

    void StageResultInformerNrvWaitBeforeDisplayBestTime::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->tryWaitIntervalBeforeKeyWait(&StageResultInformerNrvDisplayBestTime::sInstance, 30);
    }

    void StageResultInformerNrvAppearBestTime::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeAppearBestTime();
    }

    void StageResultInformerNrvDisappearGetCoin::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeDisappearGetCoin();
    }

    void StageResultInformerNrvCoinCountWait::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeShowGalaxyMap();
    }

    void StageResultInformerNrvCoinCountEnd::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        MR::setNerveAtStep(actor, &StageResultInformerNrvDisappearGetCoin::sInstance, 45);
    }

    void StageResultInformerNrvWaitBeforeDisappearGetCoin::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeWaitBeforeDisappearGetCoin();
    }

    void StageResultInformerNrvDisplayGetCoin::executeOnEnd(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        MR::deleteEffect(actor, "ResultCoinCounter");
    }

    void StageResultInformerNrvDisplayGetCoin::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeDisplayGetCoin();
    }

    void StageResultInformerNrvAppearGetCoin::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeAppearGetCoin();
    }

    void StageResultInformerNrvDisappearGetStarPiece::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeDisappearGetStarPiece();
    }

    void StageResultInformerNrvWaitBeforeDisappearGetStarPiece::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeWaitBeforeDisappearGetStarPiece();
    }

    void StageResultInformerNrvDisplayGetStarPiece::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeDisplayGetStarPiece();
    }

    void StageResultInformerNrvAppearGetStarPiece::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeAppearGetStarPiece();
    }

    void StageResultInformerNrvGhostAppear::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        const wchar_t* galaxyName = MR::getGalaxyNameOnCurrentLanguage(GameSequenceFunction::getClearedStageName());
        actor->tryShowAndKeyWaitInformationWindow(cMessageGhostAppear, galaxyName);
    }

    void StageResultInformerNrvComplete::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeComplete();
    }

    void StageResultInformerNrvDisappearGetCometMedal::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeDisappearGetCometMedal();
    }

    void StageResultInformerNrvDisplayGetCometMedal::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeDisplayGetCometMedal();
    }

    void StageResultInformerNrvCountUpGetCometMedal::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeCountUpGetCometMedal();
    }

    void StageResultInformerNrvWaitBeforeCountUpCometMedal::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->tryWaitIntervalBeforeKeyWait(&StageResultInformerNrvCountUpGetCometMedal::sInstance, 45);
    }

    void StageResultInformerNrvAppearGetCometMedal::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeAppearGetCometMedal();
    }

    void StageResultInformerNrvDisappearGetPowerStar::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeDisappearGetPowerStar();
    }

    void StageResultInformerNrvDisplayGetPowerStar::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeDisplayGetPowerStar();
    }

    void StageResultInformerNrvCountUpPowerStar::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeCountUpPowerStar();
    }

    void StageResultInformerNrvWaitBeforeCountUpPowerStar::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        bool hasPowerStar = GameSequenceFunction::hasResultPowerStar();
        const Nerve* next = &StageResultInformerNrvDisplayGetPowerStar::sInstance;
        if (hasPowerStar) next = &StageResultInformerNrvCountUpPowerStar::sInstance;
        actor->tryWaitIntervalBeforeKeyWait(next, 85);
    }

    void StageResultInformerNrvAppearGetPowerStar::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        actor->exeAppearGetPowerStar();
    }

}
