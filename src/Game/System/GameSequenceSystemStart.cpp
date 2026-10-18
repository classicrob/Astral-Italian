#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSequenceSystemStart.hpp"

#include "Game/LiveActor/Nerve.hpp"
#include "Game/System/GameSequence.hpp"
#include "Game/System/GameSystemFunction.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/NerveUtil.hpp"

namespace {
    class GameSequenceSystemStartNrvStart : public Nerve {
    public:
        virtual void execute(Spine*) const {
            if (GameSequenceFunction::isGameSequenceWipeStarted() != 0) {
                GameSystemFunction::setPermissionToCheckWiiRemoteConnect(false);
                GameSystemFunction::tryToLoadSystemArchive();
            }
        }
        static GameSequenceSystemStartNrvStart sInstance;
    };
    GameSequenceSystemStartNrvStart GameSequenceSystemStartNrvStart::sInstance ATTRIBUTE_WEAK;
};  // namespace

GameSequenceSystemStart::GameSequenceSystemStart(GameSequence* pHost) : ActorStateBase< GameSequence >("[シーケンス]起動〜ロゴ", pHost) {
}

GameSequenceSystemStart::~GameSequenceSystemStart() {
}

void GameSequenceSystemStart::init() {
    initNerve(GET_NERVE_ANON(GameSequenceSystemStartNrvStart), 0);
}

void GameSequenceSystemStart::appear() {
    mIsDead = false;
    setNerve(GET_NERVE_ANON(GameSequenceSystemStartNrvStart));
}
