#include "Game/System/GameSequenceInGame.hpp"

#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Scene/GameSceneFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"

namespace NrvGameSequenceInGame {
    void GameSequenceInGameNrvBeforeKoopaVs3::execute(Spine*) const {
        if (GameSequenceFunction::isGameSequenceWipeStarted() != 0) {
            MR::openWipeFade(-1);
        }
    }
    void GameSequenceInGameNrvInStage::execute(Spine* pSpine) const {
        if (MR::updateActorState(static_cast< NerveExecutor* >(pSpine->mExecutor))) {
            GameSequenceFunction::requestChangeStageWorldMap();
        }
    }
    void GameSequenceInGameNrvWorldMap::execute(Spine* pSpine) const {
        MR::updateActorState(static_cast< NerveExecutor* >(pSpine->mExecutor));
    }
    void GameSequenceInGameNrvMarioFace::execute(Spine* pSpine) const {
        MR::updateActorState(static_cast< NerveExecutor* >(pSpine->mExecutor));
    }
    void GameSequenceInGameNrvOpening::execute(Spine* pSpine) const {
        MR::updateActorState(static_cast< NerveExecutor* >(pSpine->mExecutor));
    }
};  // namespace NrvGameSequenceInGame
