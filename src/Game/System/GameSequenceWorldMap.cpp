#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSequenceWorldMap.hpp"

#include "Game/LiveActor/Nerve.hpp"
#include "Game/NameObj/NameObj.hpp"
#include "Game/System/GameSequenceInGame.hpp"
#include "Game/System/PlayResultWorldMapHolder.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"

namespace NrvGameSequenceWorldMap {
    void GameSequenceWorldMapNrvClose::execute(Spine* pSpine) const {
        reinterpret_cast< GameSequenceWorldMap* >(pSpine->mExecutor)->exeClose();
    }
    void GameSequenceWorldMapNrvWait::execute(Spine* pSpine) const {
        reinterpret_cast< GameSequenceWorldMap* >(pSpine->mExecutor)->exeWait();
    }
    void GameSequenceWorldMapNrvEnd::execute(Spine* pSpine) const {
        reinterpret_cast< GameSequenceWorldMap* >(pSpine->mExecutor)->exeEnd();
    }

    GameSequenceWorldMapNrvClose GameSequenceWorldMapNrvClose::sInstance ATTRIBUTE_WEAK;
    GameSequenceWorldMapNrvWait GameSequenceWorldMapNrvWait::sInstance ATTRIBUTE_WEAK;
    GameSequenceWorldMapNrvEnd GameSequenceWorldMapNrvEnd::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvGameSequenceWorldMap

GameSequenceWorldMap::GameSequenceWorldMap(GameSequenceInGame* pHost) : ActorStateBase< GameSequenceInGame >("[シーケンス]ワールドマップ", pHost), mPlayResultHolder(nullptr), mNameObj(nullptr) {
    mPlayResultHolder = new PlayResultWorldMapHolder();
    mNameObj = new NameObj("\x83\x8f\x81[\x83\x8b\x83h\x83}\x83""b\x83v\x83""f\x83\x82\x8aJ\x8en\x8e\xd2");
}

GameSequenceWorldMap::~GameSequenceWorldMap() {
}

void GameSequenceWorldMap::init() {
    initNerve(GET_NERVE_DIRECT(NrvGameSequenceWorldMap, GameSequenceWorldMapNrvClose), 0);
}

void GameSequenceWorldMap::appear() {
    mIsDead = false;
    setNerve(GET_NERVE_DIRECT(NrvGameSequenceWorldMap, GameSequenceWorldMapNrvClose));
}

PlayResultWorldMapHolder* GameSequenceWorldMap::getPlayResultHolder() const {
    return mPlayResultHolder;
}

void GameSequenceWorldMap::exeClose() {
    if (GameSequenceFunction::isGameSequenceWipeStarted() != 0) {
        if (GameSequenceFunction::isWorldMapEnterType5() != 0) {
            MR::forceCloseSystemWipeFade();
        } else {
            MR::forceCloseSystemWipeWhiteFade();
        }
        setNerve(GET_NERVE_DIRECT(NrvGameSequenceWorldMap, GameSequenceWorldMapNrvWait));
    }
}

void GameSequenceWorldMap::exeWait() {
    if (MR::tryEndModelWipe()) {
        setNerve(GET_NERVE_DIRECT(NrvGameSequenceWorldMap, GameSequenceWorldMapNrvEnd));
    }
}

void GameSequenceWorldMap::exeEnd() {
}
