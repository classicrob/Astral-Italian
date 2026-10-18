#include "Game/Screen/StageResultInformer.hpp"
#include "Game/System/WorldMapResultCommand.hpp"
#include "Game/Scene/SceneObjHolder.hpp"

namespace NrvStageResultInformer {
    StageResultInformerNrvAppearGetPowerStar::StageResultInformerNrvAppearGetPowerStar() {}
    StageResultInformerNrvWaitBeforeCountUpPowerStar::StageResultInformerNrvWaitBeforeCountUpPowerStar() {}
    StageResultInformerNrvCountUpPowerStar::StageResultInformerNrvCountUpPowerStar() {}
    StageResultInformerNrvDisplayGetPowerStar::StageResultInformerNrvDisplayGetPowerStar() {}
    StageResultInformerNrvDisappearGetPowerStar::StageResultInformerNrvDisappearGetPowerStar() {}
    StageResultInformerNrvAppearGetCometMedal::StageResultInformerNrvAppearGetCometMedal() {}
    StageResultInformerNrvWaitBeforeCountUpCometMedal::StageResultInformerNrvWaitBeforeCountUpCometMedal() {}
    StageResultInformerNrvCountUpGetCometMedal::StageResultInformerNrvCountUpGetCometMedal() {}
    StageResultInformerNrvDisplayGetCometMedal::StageResultInformerNrvDisplayGetCometMedal() {}
    StageResultInformerNrvDisappearGetCometMedal::StageResultInformerNrvDisappearGetCometMedal() {}
    StageResultInformerNrvComplete::StageResultInformerNrvComplete() {}
    StageResultInformerNrvGhostAppear::StageResultInformerNrvGhostAppear() {}
    StageResultInformerNrvAppearGetStarPiece::StageResultInformerNrvAppearGetStarPiece() {}
    StageResultInformerNrvDisplayGetStarPiece::StageResultInformerNrvDisplayGetStarPiece() {}
    StageResultInformerNrvWaitBeforeDisappearGetStarPiece::StageResultInformerNrvWaitBeforeDisappearGetStarPiece() {}
    StageResultInformerNrvDisappearGetStarPiece::StageResultInformerNrvDisappearGetStarPiece() {}
    StageResultInformerNrvAppearGetCoin::StageResultInformerNrvAppearGetCoin() {}
    StageResultInformerNrvDisplayGetCoin::StageResultInformerNrvDisplayGetCoin() {}
    StageResultInformerNrvWaitBeforeDisappearGetCoin::StageResultInformerNrvWaitBeforeDisappearGetCoin() {}
    StageResultInformerNrvCoinCountEnd::StageResultInformerNrvCoinCountEnd() {}
    StageResultInformerNrvCoinCountWait::StageResultInformerNrvCoinCountWait() {}
    StageResultInformerNrvDisappearGetCoin::StageResultInformerNrvDisappearGetCoin() {}
    StageResultInformerNrvAppearBestTime::StageResultInformerNrvAppearBestTime() {}
    StageResultInformerNrvWaitBeforeDisplayBestTime::StageResultInformerNrvWaitBeforeDisplayBestTime() {}
    StageResultInformerNrvDisplayBestTime::StageResultInformerNrvDisplayBestTime() {}
    StageResultInformerNrvDisappearBestTime::StageResultInformerNrvDisappearBestTime() {}
    StageResultInformerNrvHideClearTime::StageResultInformerNrvHideClearTime() {}
    StageResultInformerNrvSendCompleteMail::StageResultInformerNrvSendCompleteMail() {}
    StageResultInformerNrvMapStart::StageResultInformerNrvMapStart() {}
    StageResultInformerNrvRouteOpen::StageResultInformerNrvRouteOpen() {}
    StageResultInformerNrvCometAppear::StageResultInformerNrvCometAppear() {}
    StageResultInformerNrvMapEnd::StageResultInformerNrvMapEnd() {}
}

namespace NrvDisplayInformationForResult {
    DisplayInformationForResultNrvAppear::DisplayInformationForResultNrvAppear() {}
    DisplayInformationForResultNrvIdle::DisplayInformationForResultNrvIdle() {}
    void DisplayInformationForResultNrvIdle::execute(Spine*) const {}
    void DisplayInformationForResultNrvAppear::execute(Spine* spine) const {
        static_cast<DisplayInformationForResult*>(spine->mExecutor)->exeAppear();
    }
}

namespace NrvStageResultInformer {
    void StageResultInformerNrvMapEnd::execute(Spine* spine) const {
        static_cast<StageResultInformer*>(spine->mExecutor)->exeMapEnd();
    }
    void StageResultInformerNrvCometAppear::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        if (MR::getSceneObj<WorldMapResultCommand>(0x34)->updateController()) actor->decideNextNerve();
    }
    void StageResultInformerNrvRouteOpen::execute(Spine* spine) const {
        StageResultInformer* actor = static_cast<StageResultInformer*>(spine->mExecutor);
        if (MR::getSceneObj<WorldMapResultCommand>(0x34)->updateController()) actor->decideNextNerve();
    }
    void StageResultInformerNrvMapStart::execute(Spine* spine) const {
        static_cast<StageResultInformer*>(spine->mExecutor)->exeMapStart();
    }
    void StageResultInformerNrvSendCompleteMail::execute(Spine* spine) const {
        static_cast<StageResultInformer*>(spine->mExecutor)->exeSendCompleteMail();
    }
    void StageResultInformerNrvHideClearTime::execute(Spine* spine) const {
        static_cast<StageResultInformer*>(spine->mExecutor)->exeHideClearTime();
    }
}
